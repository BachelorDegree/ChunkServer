#include <spdlog/spdlog.h>
#include "coredeps/SliceId.hpp"

#include "errcode.h"
#include "Logic/Logic.hpp"

#include "SetChunkStateHandler.hpp"
#include "ChunkServerServiceImpl.hpp"

void SetChunkStateHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.SetChunkState";
}

void SetChunkStateHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestSetChunkState(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new SetChunkStateHandler(service, cq);
        this->BeforeProcess();
        // Implement your logic here
        // int iRet = ChunkServerServiceImpl::GetInstance()->SetChunkState(request, response);
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

int SetChunkStateHandler::Implementation(void)
{
    int iRet = 0;
    Storage::SliceId oSliceId(request.chunk_id());
    do
    {
        if (oSliceId.Cluster() != g_iClusterId 
            || oSliceId.Machine() != g_iMachineId
            || oSliceId.Disk() >= g_iDiskCount
        )
        {
            iRet = E_DISK_NOT_ON_THIS_MACHINE;
            break;
        }
        auto &oDiskInfo = g_apDiskInfo[oSliceId.Disk()];
        if (oSliceId.Chunk() >= static_cast<uint64_t>(oDiskInfo.ChunkCount))
        {
            iRet = E_CHUNK_ID_OUT_OF_RANGE;
            break;
        }
        auto &oChunkInfo = oDiskInfo.Chunks[oSliceId.Chunk()];
        CoMutexGuard oGuard(oChunkInfo.Mutex);
        oChunkInfo.State = request.state_to_set();
        auto iPwriteRet = oChunkInfo.FlushToDisk();
        if (iPwriteRet < 0)
        {
            spdlog::error("SetChunkState - flush to disk error, errno: {}", iPwriteRet);
            return E_FLUSH_CHUNK_HEADER_FAILED;
        }
    } while (false);
    spdlog::info("SetChunkState - chunk_id: 0x{:016x}, state: {}", request.chunk_id(), request.state_to_set());
    return iRet;
}
