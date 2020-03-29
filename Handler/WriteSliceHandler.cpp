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
        int iRet = ChunkServerServiceImpl::GetInstance()->WriteSlice(request, response);
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

