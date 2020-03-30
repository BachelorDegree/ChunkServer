#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class ReadSliceHandler final : public AsyncRpcHandler
{
public:
    ReadSliceHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;
    int  Implementation(void);

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::ReadSliceReq                                    request;
    ::chunkserver::ReadSliceRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::ReadSliceRsp>  responder;
};
