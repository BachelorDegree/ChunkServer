#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <coredeps/SatelliteClient.hpp>
#include <coredeps/ContextHelper.hpp>
#include "ChunkMasterServiceClient.hpp"
ChunkMasterServiceClient::ChunkMasterServiceClient():
  m_strServiceName("ChunkMasterService")
{
  m_pChannel = GetChannel();
}
ChunkMasterServiceClient::ChunkMasterServiceClient(const std::string &strIpPortOrCanonicalName)
{
  if (strIpPortOrCanonicalName.find(':') == std::string::npos)
  {
    m_strServiceName = strIpPortOrCanonicalName;
    m_pChannel = this->GetChannel();
  }
  else
  {
    m_pChannel = grpc::CreateChannel(strIpPortOrCanonicalName, grpc::InsecureChannelCredentials());
  }
}
std::shared_ptr<grpc::Channel> ChunkMasterServiceClient::GetChannel()
{
  std::string strServer = SatelliteClient::GetInstance().GetNode(m_strServiceName);
  return grpc::CreateChannel(strServer, grpc::InsecureChannelCredentials());
}
int ChunkMasterServiceClient::CalculateUploadSliceLengths(const ::chunkmaster::CalculateUploadSliceLengthsReq & oReq, ::chunkmaster::CalculateUploadSliceLengthsRsp & oResp)
{
  ::chunkmaster::ChunkMasterService::Stub oStub{m_pChannel};
  grpc::ClientContext oContext;
  auto oStatus = oStub.CalculateUploadSliceLengths(&oContext, oReq, &oResp);
  if (oStatus.ok() == false)
  {
    return -1;
  }
  return ClientContextHelper(oContext).GetReturnCode();
}
int ChunkMasterServiceClient::AllocateUploadSlice(const ::chunkmaster::AllocateUploadSliceReq & oReq, ::chunkmaster::AllocateUploadSliceRsp & oResp)
{
  ::chunkmaster::ChunkMasterService::Stub oStub{m_pChannel};
  grpc::ClientContext oContext;
  auto oStatus = oStub.AllocateUploadSlice(&oContext, oReq, &oResp);
  if (oStatus.ok() == false)
  {
    return -1;
  }
  return ClientContextHelper(oContext).GetReturnCode();
}
int ChunkMasterServiceClient::FinishUploadSlice(const ::chunkmaster::FinishUploadSliceReq & oReq, ::chunkmaster::FinishUploadSliceRsp & oResp)
{
  ::chunkmaster::ChunkMasterService::Stub oStub{m_pChannel};
  grpc::ClientContext oContext;
  auto oStatus = oStub.FinishUploadSlice(&oContext, oReq, &oResp);
  if (oStatus.ok() == false)
  {
    return -1;
  }
  return ClientContextHelper(oContext).GetReturnCode();
}
int ChunkMasterServiceClient::ReportChunkInformation(const ::chunkmaster::ReportChunkInformationReq & oReq, ::chunkmaster::ReportChunkInformationRsp & oResp)
{
  ::chunkmaster::ChunkMasterService::Stub oStub{m_pChannel};
  grpc::ClientContext oContext;
  auto oStatus = oStub.ReportChunkInformation(&oContext, oReq, &oResp);
  if (oStatus.ok() == false)
  {
    return -1;
  }
  return ClientContextHelper(oContext).GetReturnCode();
}
