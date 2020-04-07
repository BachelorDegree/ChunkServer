#include <colib/co_aio.h>
#include <spdlog/spdlog.h>
#include "coredeps/SliceId.hpp"

#include "errcode.h"
#include "../Logic/Logic.hpp"
#include "../Logic/InodeLruCache.hpp"

#include "WriteSliceHandler.hpp"
#include "ChunkServerServiceImpl.hpp"

void WriteSliceHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.WriteSlice";
}

void WriteSliceHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestWriteSlice(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new WriteSliceHandler(service, cq);
        this->BeforeProcess();
        // Implement your logic here
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

int WriteSliceHandler::Implementation(void)
{
    int iRet = 0;
    ssize_t iPwriteRet;
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
        auto oInode = InodeLruCache::GetInstance().Get(oSliceId.UInt());
        if (request.offset() >= oInode.LogicalLength)
        {
            iRet = E_OFFSET_OUT_OF_RANGE;
            break;
        }
        if (request.offset() + request.data().length() >= oInode.LogicalLength)
        {
            iRet = E_DATA_LENGTH_OUT_OF_RANGE;
            break;
        }
        CoMutexGuard oGuard(oChunkInfo.Mutex);
        auto iOffset = oChunkInfo.GetDataSectionOffset() + oInode.Offset + request.offset();
        iPwriteRet = co_pwrite(oChunkInfo.DiskInfoPtr->Fd, (void*)request.data().data(), request.data().size(), iOffset);
        if (iPwriteRet < 0)
        {
            iRet = E_PWRITE_FAILED;
            spdlog::error("WriteSliceHandler - co_pwrite() failed, errno: {}", iPwriteRet);
            break;
        }
    } while (false);
    spdlog::info("WriteSliceHandler - slice_id: 0x{:016x}, data_length: {}, ret: {}", request.slice_id(), request.data().length(), iRet);
    return iRet;
}
