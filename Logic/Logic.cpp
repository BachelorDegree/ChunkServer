#include "errcode.h"
#include "Logic.hpp"
#include "coredeps/SliceId.hpp"

int IsChunkIdValid(uint64_t iChunkId)
{
    int iRet = 0;
    Storage::SliceId oChunkId(iChunkId);
    do
    {
        if (oChunkId.Cluster() != g_iClusterId 
            || oChunkId.Machine() != g_iMachineId
            || oChunkId.Disk() >= g_iDiskCount
        )
        {
            iRet = E_DISK_NOT_ON_THIS_MACHINE;
            break;
        }
        auto &oDiskInfo = g_apDiskInfo[oChunkId.Disk()];
        if (oChunkId.Chunk() >= static_cast<uint64_t>(oDiskInfo.ChunkCount))
        {
            iRet = E_CHUNK_ID_OUT_OF_RANGE;
            break;
        }
    } while (false);
    return iRet;
}