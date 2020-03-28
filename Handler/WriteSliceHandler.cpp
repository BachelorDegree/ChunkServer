#include "WriteSliceHandler.hpp"

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
        // Firstly, spawn a new handler for next incoming RPC call
        new WriteSliceHandler(service, cq);
        // Implement your logic here
        // response.set_reply(request.greeting());
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

