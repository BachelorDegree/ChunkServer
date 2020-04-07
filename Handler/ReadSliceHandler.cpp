#include <colib/co_aio.h>
#include <spdlog/spdlog.h>
#include "coredeps/SliceId.hpp"

#include "errcode.h"
#include "Logic/Logic.hpp"
#include "Logic/InodeLruCache.hpp"

#include "ReadSliceHandler.hpp"
#include "ChunkServerServiceImpl.hpp"

void ReadSliceHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.ReadSlice";
}

void ReadSliceHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestReadSlice(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new ReadSliceHandler(service, cq);
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

int ReadSliceHandler::Implementation(void)
{
    int iRet = 0;
    ssize_t iPreadRet;
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
        if (request.offset() + request.length_to_read() >= oInode.LogicalLength)
        {
            iRet = E_DATA_LENGTH_OUT_OF_RANGE;
            break;
        }
        response.mutable_data()->reserve(request.length_to_read());
        CoMutexGuard oGuard(oChunkInfo.Mutex);
        auto iOffset = oChunkInfo.GetDataSectionOffset() + oInode.Offset + request.offset();
        iPreadRet = co_pread(oChunkInfo.DiskInfoPtr->Fd, (void*)response.mutable_data()->data(), request.length_to_read(), iOffset);
        if (iPreadRet < 0)
        {
            iRet = E_PREAD_FAILED;
            spdlog::error("ReadSliceHandler - co_pread() failed, errno: {}", iPreadRet);
            break;
        }
    } while (false);
    spdlog::info("ReadSliceHandler - slice_id: 0x{:016x}, data_length: {}, ret: {}", request.slice_id(), iPreadRet, iRet);
    return iRet;
}