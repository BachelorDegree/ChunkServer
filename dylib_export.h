#pragma once

namespace grpc
{
    class Service;
    class ServerCompletionQueue;
}
class SatelliteClient;

extern "C" 
{

const char *    EXPORT_Description(void);
void            EXPORT_DylibInit(const char *);
grpc::Service * EXPORT_GetGrpcServiceInstance(void);
void            EXPORT_OnWorkerThreadStart(grpc::ServerCompletionQueue*);

} 
