#include "dylib_export.h"
#include "Proto/chunkserver.grpc.pb.h"
#include "Handler/SetChunkStatusHandler.hpp"
#include "Handler/AllocateInodeHandler.hpp"
#include "Handler/ReadSliceHandler.hpp"
#include "Handler/WriteSliceHandler.hpp"
#include "coredeps/SatelliteClient.hpp"
#include "Logic/Logic.hpp"

::chunkserver::ChunkServerService::AsyncService service;

const char * EXPORT_Description(void)
{
    return "chunkserver";
}

void EXPORT_DylibInit(const char *conf_file)
{
    DoInitialize(conf_file);
}

grpc::Service * EXPORT_GetGrpcServiceInstance(void)
{
    return &service;
}

void EXPORT_OnWorkerThreadStart(grpc::ServerCompletionQueue *cq)
{
    // Bind handlers

    new SetChunkStatusHandler(&service, cq);
    new AllocateInodeHandler(&service, cq);
    new ReadSliceHandler(&service, cq);
    new WriteSliceHandler(&service, cq);
}
