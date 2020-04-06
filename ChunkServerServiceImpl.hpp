#pragma once
#include "Proto/chunkserver.pb.h"
class ChunkServerServiceImpl
{
public:
    static ChunkServerServiceImpl *GetInstance();
    static void SetInstance(ChunkServerServiceImpl *);
    static int BeforeServerStart(const char * czConf) {
        return 0;
    }
    int BeforeWorkerStart() {
        return 0;
    }
    int SetChunkState(const ::chunkserver::SetChunkStateReq & oReq, ::chunkserver::SetChunkStateRsp & oResp);
    int AllocateInode(const ::chunkserver::AllocateInodeReq & oReq, ::chunkserver::AllocateInodeRsp & oResp);
    int ReadSlice(const ::chunkserver::ReadSliceReq & oReq, ::chunkserver::ReadSliceRsp & oResp);
    int WriteSlice(const ::chunkserver::WriteSliceReq & oReq, ::chunkserver::WriteSliceRsp & oResp);
};
