#include <spdlog/spdlog.h>

#include "Logic.hpp"
#include "ChunkMasterServiceClient/ChunkMasterServiceClient.hpp"

#include "ReportToMasterLoop.hpp"

static void Impl(void)
{
    ::chunkmaster::ReportChunkInformationReq oReq;
    ::chunkmaster::ReportChunkInformationRsp oRsp;
    ChunkMasterServiceClient oClient;
    for (uint32_t i = 0; i < g_iDiskCount; ++i)
    {
        auto &oDiskInfo = g_apDiskInfo[i];
        for (int j = 0; j < oDiskInfo.ChunkCount; ++j)
        {
            auto &oChunkInfo = oDiskInfo.Chunks[j];
            auto pCI = oReq.add_chunk_info();
            pCI->set_chunk_id(oChunkInfo.ChunkId.UInt());
            pCI->set_logical_used_space(oChunkInfo.LogicalUsedSpace);
            pCI->set_actual_used_space(oChunkInfo.ActualUsedSpace);
            pCI->set_state(static_cast<chunkmaster::ChunkState>(oChunkInfo.State));
        }
    }
    int iRpcRet = oClient.ReportChunkInformation(oReq, oRsp);
    if (iRpcRet != 0)
    {
        spdlog::error("ReportToMasterLoop - ChunkMaster.ReportChunkInformation failed, retcode: {}", iRpcRet);
    }
}

void ReportToMasterLoop(void)
{
    for ( ; ;)
    {
        spdlog::info("ReportToMasterLoop running");
        Impl();
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
}