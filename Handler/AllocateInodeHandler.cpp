#include "AllocateInodeHandler.hpp"
#include "../define.hpp"
#include "../Logic/Logic.hpp"
#include "../CoreDeps/include/SliceId.hpp"

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
        // Firstly, spawn a new handler for next incoming RPC call
        new AllocateInodeHandler(service, cq);
        // Implement your logic here

    {
        Storage::SliceId sid(request.slice_id());
        do
        {
            if (sid.Cluster() != GClusterId 
                || sid.Machine() != GMachineId
                || sid.Disk() >= GDiskCount
            )
            {
                // Todo: SET ERROR CODE
                break;
            }
            auto &di = GDiskInfo[sid.Disk()];
            if (sid.Chunk() >= static_cast<uint64_t>(di.ChunkCount))
            {
                // Todo: SET ERROR CODE
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
            inode.FlushToDisk(request.slice_id());
            int ret = ci.FlushToDisk();
        } while (false);
        // Todo: log
    }

        this->SetStatusFinish();
        responder.Finish(response, grpc::Status::OK, this);
        break;
    case Status::FINISH:
        delete this;
        break;
    default:
        // throw exception
        ;
    }
}

