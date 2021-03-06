#include <spdlog/spdlog.h>
#include <coredeps/SliceId.hpp>

#include "errcode.h"
#include "define.hpp"
#include "Logic/Logic.hpp"
#include "Logic/ChunkFSM.hpp"

#include "AllocateInodeHandler.hpp"
#include "ChunkServerServiceImpl.hpp"

void AllocateInodeHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.AllocateInode";
}

void AllocateInodeHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestAllocateInode(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new AllocateInodeHandler(service, cq);
        this->BeforeProcess();
        // Implement your logic here
        auto iRet = this->Implementation();
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

int AllocateInodeHandler::Implementation(void)
{
    int iRet = 0;
    Storage::SliceId oSliceId(request.chunk_id());
    auto oRetSliceId = oSliceId;
    do
    {
        iRet = IsChunkIdValid(oSliceId.UInt());
        if (iRet != 0)
        {
            break;
        }
        auto &oDiskInfo = g_apDiskInfo[oSliceId.Disk()];
        auto &oChunkInfo = oDiskInfo.Chunks[oSliceId.Chunk()];
        oRetSliceId.SetSlice(oChunkInfo.NextInode);

        // // 分配分片后该chunk转移到writing，如果两者不同则请求无效
        // if (oChunkInfo.NextInode != oSliceId.Slice())
        // {
        //     iRet = E_SLICE_NUMBER_DOES_NOT_MATCH_CHUNK_STATUS;
        //     spdlog::error("AllocateInode - request slice number: {}, should be: {}", oSliceId.Slice(), oChunkInfo.NextInode);
        //     break;
        // }

        // Lock this chunk
        // std::lock_guard<libco::CoMutex> oGuard(*oChunkInfo.Mutex); // TODO: fix coroutine mutex
        Inode oInode;
        oInode.RefCount = 1;
        oInode.LogicalLength = request.data_length();
        oInode.Offset = oChunkInfo.ActualUsedSpace;
        oChunkInfo.ActualUsedSpace += oInode.ActualLength();
        oChunkInfo.LogicalUsedSpace += oInode.LogicalLength;
        ++oChunkInfo.NextInode;
        oChunkInfo.State = static_cast<uint32_t>(ChunkFSM::WRITING); // 状态转移到 WRITING
        spdlog::debug(oInode.ShortDebugString());
        auto iInodeFlushRet = oInode.FlushToDisk(oRetSliceId.UInt());
        if (iInodeFlushRet < 0)
        {
            iRet = E_FLUSH_INODE_FAILED;
            spdlog::error("AllocateInode - Flush inode to disk failed, syscall ret: {}, slice id: {:016x}", iInodeFlushRet, request.chunk_id());
            break;
        }
        auto iChunkInfoFlushRet = oChunkInfo.FlushToDisk();
        if (iChunkInfoFlushRet < 0)
        {
            iRet = E_FLUSH_CHUNK_HEADER_FAILED;
            spdlog::error("AllocateInode - Flush chunk header to disk failed, syscall ret: {}, slice id: {:016x}", iChunkInfoFlushRet, oChunkInfo.ChunkId.UInt());
            break;
        }
    } while (false);
    response.set_slice_id(oRetSliceId.UInt());
    spdlog::info("AllocateInode - slice id: {:016x}, length: {}, ret: {}", response.slice_id(), request.data_length(), iRet);
    return iRet;
}