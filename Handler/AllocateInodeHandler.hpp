#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class AllocateInodeHandler final : public AsyncRpcHandler
{
public:
    AllocateInodeHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;
    int  Implementation(void);

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::AllocateInodeReq                                    request;
    ::chunkserver::AllocateInodeRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::AllocateInodeRsp>  responder;
};
