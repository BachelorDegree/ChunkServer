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
            auto &di = GDiskInfo[sid.Disk()];
            if (sid.Chunk() >= static_cast<uint64_t>(di.ChunkCount))
            {
                iRet = E_CHUNK_ID_OUT_OF_RANGE;
                break;
            }
            auto &ci = di.Chunks[sid.Chunk()];
            // Lock this chunk
            CoMutexGuard guard(ci.Mutex);
            Inode inode;
            inode.RefCount = 1;
            inode.LogicalLength = request.data_length();
            inode.Offset = ChunkInodeSectionOffset + ci.ActualUsedSpace;
            ci.ActualUsedSpace += inode.ActualLength();
            ci.LogicalUsedSpace += inode.LogicalLength;
            ++ci.NextInode;
            auto inodeFlushRet = inode.FlushToDisk(request.slice_id());
            if (inodeFlushRet < 0)
            {
                iRet = E_FLUSH_INODE_FAILED;
                spdlog::error("AllocateInode - Flush inode to disk failed, syscall ret: {}, slice id: {:x}", inodeFlushRet, request.slice_id());
                break;
            }
            auto ciFlushRet = ci.FlushToDisk();
            if (ciFlushRet < 0)
            {
                iRet = E_FLUSH_CHUNK_HEADER_FAILED;
                spdlog::error("AllocateInode - Flush chunk header to disk failed, syscall ret: {}, slice id: {:x}", ciFlushRet, ci.ChunkId.UInt());
                break;
            }
        } while (false);
        spdlog::info("AllocateInode - slice id: {:x}, length: {}", request.slice_id(), request.data_length());
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

