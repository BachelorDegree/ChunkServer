#pragma once
#include <memory>
#include <string>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include "chunkmaster.grpc.pb.h"
#include "chunkmaster.pb.h"
class ChunkMasterServiceClient
{
private:
  std::shared_ptr<grpc::Channel> m_pChannel;
  std::string m_strServiceName;
public:
  ChunkMasterServiceClient();
  // User specified IpPort or CanonicalName
  ChunkMasterServiceClient(const std::string &strIpPortOrCanonicalName);
  std::shared_ptr<grpc::Channel> GetChannel();
    int CalculateUploadSliceLengths(const ::chunkmaster::CalculateUploadSliceLengthsReq & oReq, ::chunkmaster::CalculateUploadSliceLengthsRsp & oResp);
    int AllocateUploadSlice(const ::chunkmaster::AllocateUploadSliceReq & oReq, ::chunkmaster::AllocateUploadSliceRsp & oResp);
    int FinishUploadSlice(const ::chunkmaster::FinishUploadSliceReq & oReq, ::chunkmaster::FinishUploadSliceRsp & oResp);
    int BatchGetPhysicalSlices(const ::chunkmaster::BatchGetPhysicalSlicesReq & oReq, ::chunkmaster::BatchGetPhysicalSlicesRsp & oResp);
    int ReportChunkInformation(const ::chunkmaster::ReportChunkInformationReq & oReq, ::chunkmaster::ReportChunkInformationRsp & oResp);
};
