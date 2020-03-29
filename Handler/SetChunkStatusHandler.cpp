#include "SetChunkStatusHandler.hpp"
#include "ChunkServerServiceImpl.hpp"
void SetChunkStatusHandler::SetInterfaceName(void)
{
    interfaceName = "ChunkServerService.SetChunkStatus";
}

void SetChunkStatusHandler::Proceed(void)
{
    switch (status)
    {
    case Status::CREATE:
        this->SetStatusProcess();
        service->RequestSetChunkStatus(&ctx, &request, &responder, cq, cq, this);
        break;
    case Status::PROCESS:
    {
        // Firstly, spawn a new handler for next incoming RPC call
        new SetChunkStatusHandler(service, cq);
        this->BeforeProcess();
        // Implement your logic here
        int iRet = ChunkServerServiceImpl::GetInstance()->SetChunkStatus(request, response);
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

