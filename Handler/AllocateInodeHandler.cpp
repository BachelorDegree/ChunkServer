#include "AllocateInodeHandler.hpp"
#include "ChunkServerServiceImpl.hpp"
#include "coredeps/SliceId.hpp"
#include "../define.hpp"
#include "../Logic/Logic.hpp"
#include "spdlog/spdlog.h"

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
        int iRet = 0;
    {
        Storage::SliceId sid(request.slice_id());
        do
        {
            if (sid.Cluster() != GClusterId 
                || sid.Machine() != GMachineId
                || sid.Disk() >= GDiskCount
            )
            {
                iRet = E_DISK_NOT_ON_THIS_MACHINE;
                break;
            }
            auto &oDiskInfo = GDiskInfo[sid.Disk()];
            if (sid.Chunk() >= static_cast<uint64_t>(oDiskInfo.ChunkCount))
            {
                iRet = E_CHUNK_ID_OUT_OF_RANGE;
                break;
            }
            auto &oChunkInfo = oDiskInfo.Chunks[sid.Chunk()];
            // 分配分片后该chunk转移到writing，如果两者不同则请求无效
            if (oChunkInfo.NextInode != sid.Slice())
            {
                iRet = E_SLICE_NUMBER_DOES_NOT_MATCH_CHUNK_STATUS;
                spdlog::error("AllocateInode - request slice number: {}, should be: {}", request.slice_id(), oChunkInfo.NextInode);
                break;
            }
            // Lock this chunk
            CoMutexGuard guard(oChunkInfo.Mutex);
            Inode oInode;
            oInode.RefCount = 1;
            oInode.LogicalLength = request.data_length();
            oInode.Offset = ChunkInodeSectionOffset + oChunkInfo.ActualUsedSpace;
            oChunkInfo.ActualUsedSpace += oInode.ActualLength();
            oChunkInfo.LogicalUsedSpace += oInode.LogicalLength;
            ++oChunkInfo.NextInode;
            auto iInodeFlushRet = oInode.FlushToDisk(request.slice_id());
            if (iInodeFlushRet < 0)
            {
                iRet = E_FLUSH_INODE_FAILED;
                spdlog::error("AllocateInode - Flush inode to disk failed, syscall ret: {}, slice id: {:016x}", iInodeFlushRet, request.slice_id());
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
        spdlog::info("AllocateInode - slice id: {:016x}, length: {}", request.slice_id(), request.data_length());
    }
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

