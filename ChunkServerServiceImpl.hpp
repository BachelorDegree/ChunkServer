#pragma once
#include "Proto/chunkserver.pb.h"
class ChunkServerServiceImpl
{
public:
    static ChunkServerServiceImpl *GetInstance();
    static void SetInstance(ChunkServerServiceImpl *);
    int OnServerStart(){
        return 0;
    }
    int SetChunkStatus(const ::chunkserver::SetChunkStatusReq & oReq, ::chunkserver::SetChunkStatusRsp & oResp);
    int AllocateInode(const ::chunkserver::AllocateInodeReq & oReq, ::chunkserver::AllocateInodeRsp & oResp);
    int ReadSlice(const ::chunkserver::ReadSliceReq & oReq, ::chunkserver::ReadSliceRsp & oResp);
    int WriteSlice(const ::chunkserver::WriteSliceReq & oReq, ::chunkserver::WriteSliceRsp & oResp);
};
