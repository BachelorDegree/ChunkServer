#include <spdlog/spdlog.h>
#include <coredeps/SliceId.hpp>

#include "errcode.h"
#include "define.hpp"
#include "Logic/Logic.hpp"
#include "Logic/InodeLruCache.hpp"

#include "ManipulateReferenceCountHandler.hpp"
#include "ChunkServerServiceImpl.hpp"

void ManipulateReferenceCountHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.ManipulateReferenceCount";
}

void ManipulateReferenceCountHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestManipulateReferenceCount(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new ManipulateReferenceCountHandler(service, cq);
        this->BeforeProcess();
        // Implement your logic here
        // int iRet = ChunkServerServiceImpl::GetInstance()->ManipulateReferenceCount(request, response);
        int iRet = this->Implementation();
        this->SetReturnCode(iRet);
        this->SetStatusFinish();
        responder.Finish(response, grpc::Status::OK, this);
        break;
    }
    case Status::FINISH:
        delete this;
        break;
    default:
        // throw exception
        ;
    }
}

int ManipulateReferenceCountHandler::Implementation(void)
{
    int iRet = 0;
    Storage::SliceId oSliceId(request.slice_id());
    do
    {
        iRet = IsChunkIdValid(oSliceId.UInt());
        if (iRet != 0)
        {
            break;
        }
        auto &oDiskInfo = g_apDiskInfo[oSliceId.Disk()];
        auto &oChunkInfo = oDiskInfo.Chunks[oSliceId.Chunk()];

        do
        {
            // 控制锁的生命周期
            std::lock_guard<libco::CoMutex> oGuard(*oChunkInfo.Mutex);
            auto oInode = InodeLruCache::GetInstance().Get(oSliceId.UInt());
            switch (request.operation())
            {
            case ::chunkserver::ManipulateReferenceCountReq::INCREASE:
                ++oInode.RefCount;
                break;
            case ::chunkserver::ManipulateReferenceCountReq::DECREASE:
                if (oInode.RefCount == 0)
                {
                    spdlog::error("ManipulateReferenceCount - refcount is zero, slice_id: 0x{:016x}", request.slice_id());
                    return E_REFCOUNT_IS_ZERO;
                }
                --oInode.RefCount;
                break;
            default:
                spdlog::error("ManipulateReferenceCount - operation unknown, slice_id: 0x{:016x}", request.slice_id());
                return E_MANIP_REFCOUNT_OP_UNKNOWN;
            }
            auto iFlushRet = oInode.FlushToDisk(oSliceId.UInt());
            if (iFlushRet < 0)
            {
                iRet = E_FLUSH_INODE_FAILED;
                spdlog::error("ManipulateReferenceCount - Flush inode to disk failed, syscall ret: {}, slice id: {:016x}", iFlushRet, request.slice_id());
            }
        } while (false);
    } while (false);
    spdlog::info("ManipulateReferenceCount - slice_id: 0x{:016x}", request.slice_id());
    return iRet;
}
