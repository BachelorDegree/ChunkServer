#include "dylib_export.h"
#include "Proto/chunkserver.grpc.pb.h"
#include "ChunkServerServiceImpl.hpp"
#include "Handler/SetChunkStateHandler.hpp"
#include "Handler/AllocateInodeHandler.hpp"
#include "Handler/ReadSliceHandler.hpp"
#include "Handler/WriteSliceHandler.hpp"
#include "Handler/ManipulateReferenceCountHandler.hpp"
#include "Logic/Logic.hpp"

::chunkserver::ChunkServerService::AsyncService service;

const char * EXPORT_Description(void)
{
    return "chunkserver";
}

void EXPORT_DylibInit(const char *conf_file)
{
    DoInitialize(conf_file);
    ChunkServerServiceImpl::BeforeServerStart(conf_file);
}

grpc::Service * EXPORT_GetGrpcServiceInstance(void)
{
    return &service;
}

void EXPORT_OnWorkerThreadStart(grpc::ServerCompletionQueue *cq)
{
    // Bind handlers
    new SetChunkStateHandler(&service, cq);
    new AllocateInodeHandler(&service, cq);
    new ReadSliceHandler(&service, cq);
    new WriteSliceHandler(&service, cq);
    new ManipulateReferenceCountHandler(&service, cq);
}

void EXPORT_OnCoroutineWorkerStart(void)
{
    ChunkServerServiceImpl::SetInstance(new ChunkServerServiceImpl);
    ChunkServerServiceImpl::GetInstance()->BeforeWorkerStart();
}