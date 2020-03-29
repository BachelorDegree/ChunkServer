#include <colib/co_routine_specific.h>
#include "ChunkServerServiceImpl.hpp"
struct __ChunkServerServiceImplWrapper{
    ChunkServerServiceImpl * pImpl;
};
CO_ROUTINE_SPECIFIC(__ChunkServerServiceImplWrapper, g_coChunkServerServiceImplWrapper)
ChunkServerServiceImpl *ChunkServerServiceImpl::GetInstance()
{
    return g_coChunkServerServiceImplWrapper->pImpl;
}
void ChunkServerServiceImpl::SetInstance(ChunkServerServiceImpl *pImpl)
{
    g_coChunkServerServiceImplWrapper->pImpl = pImpl;
}
