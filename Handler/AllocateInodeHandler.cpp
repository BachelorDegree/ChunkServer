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
        int iRet = ChunkServerServiceImpl::GetInstance()->AllocateInode(request, response);
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

