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
        int iRet = ChunkServerServiceImpl::GetInstance()->ReadSlice(request, response);
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

