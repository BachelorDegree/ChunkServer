#pragma once

#include "AsyncRpcHandler.hpp"
#include "Proto/chunkserver.grpc.pb.h"

class WriteSliceHandler final : public AsyncRpcHandler
{
public:
    WriteSliceHandler(::chunkserver::ChunkServerService::AsyncService *service, grpc::ServerCompletionQueue *cq):
        AsyncRpcHandler(cq), service(service), responder(&ctx)
    {
        this->Proceed();
    }
    void Proceed(void) override;
    void SetInterfaceName(void) override;
    int  Implementation(void);

private:
    ::chunkserver::ChunkServerService::AsyncService*                     service;
    ::chunkserver::WriteSliceReq                                    request;
    ::chunkserver::WriteSliceRsp                                   response;
    grpc::ServerAsyncResponseWriter<::chunkserver::WriteSliceRsp>  responder;
};
