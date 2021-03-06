#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class SetChunkStateHandler final : public AsyncRpcHandler
{
public:
    SetChunkStateHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;
    int  Implementation(void);

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::SetChunkStateReq                                    request;
    ::chunkserver::SetChunkStateRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::SetChunkStateRsp>  responder;
};
