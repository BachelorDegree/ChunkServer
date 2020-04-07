#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class ManipulateReferenceCountHandler final : public AsyncRpcHandler
{
public:
    ManipulateReferenceCountHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;
    int  Implementation(void);

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::ManipulateReferenceCountReq                                    request;
    ::chunkserver::ManipulateReferenceCountRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::ManipulateReferenceCountRsp>  responder;
};
