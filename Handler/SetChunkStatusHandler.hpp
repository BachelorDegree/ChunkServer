#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class SetChunkStatusHandler final : public AsyncRpcHandler
{
public:
    SetChunkStatusHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::SetChunkStatusReq                                    request;
    ::chunkserver::SetChunkStatusRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::SetChunkStatusRsp>  responder;
};
