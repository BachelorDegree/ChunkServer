#include <cstring>
#include <spdlog/spdlog.h>
#include "define.hpp"
#include "Logic.hpp"
#include "DiskInfo.hpp"
#include "InodeLruCache.hpp"
#include "colib/co_mutex.h"
#include "colib/co_aio.h"
#include "coredeps/SliceId.hpp"

DiskInfo::DiskInfo(void):
    Fd(0), ChunkCount(0),
    Chunks(nullptr),
    Mutex(new libco::CoMutex)
{

}

DiskInfo::~DiskInfo(void)
{
    if (Mutex != nullptr)
        delete Mutex;
    if (Fd != 0)
        close(Fd);
    if (Chunks != nullptr)
        delete[] Chunks;
}

ChunkInfo::ChunkInfo(DiskInfo* pParent):
    Mutex(new libco::CoMutex), DiskInfoPtr(pParent)
{
    spdlog::debug("ChunkInfo constructor, Mutex={:016x}", reinterpret_cast<uint64_t>(Mutex));
}

ChunkInfo::~ChunkInfo(void)
{
    if (Mutex != nullptr)
        delete Mutex;
}

void ChunkInfo::SetDiskInfoPtr(DiskInfo *pParent)
{
    DiskInfoPtr = pParent;
}

off_t ChunkInfo::GetBaseOffset(void) const
{
    return ChunkId.Chunk() * ChunkLength + ChunkHeaderOffset;
}

off_t ChunkInfo::GetInodeOffset(uint64_t iSliceNumber) const
{
    return this->GetInodeSectionOffset() + sizeof(Inode) * iSliceNumber;
}

off_t ChunkInfo::GetDataSectionOffset(void) const
{
    return this->GetBaseOffset() + ChunkDataSectionOffset;
}

off_t ChunkInfo::GetInodeSectionOffset(void) const
{
    return this->GetBaseOffset() + ChunkInodeSectionOffset;
}

ssize_t ChunkInfo::FlushToDisk(bool UseCoroutine)
{
    auto oChunkHeader = this->GetChunkHeader();
    if (UseCoroutine)
    {
        return co_pwrite(DiskInfoPtr->Fd, &oChunkHeader, sizeof(oChunkHeader), this->GetBaseOffset());
    }
    else
    {
        return pwrite64(DiskInfoPtr->Fd, &oChunkHeader, sizeof(oChunkHeader), this->GetBaseOffset());
    }
}

ChunkHeader ChunkInfo::GetChunkHeader(void) const
{
    ChunkHeader oRet(this->ChunkId.UInt());
    oRet.NextInode = this->NextInode;
    oRet.ActualUsedSpace = this->ActualUsedSpace;
    oRet.LogicalUsedSpace = this->LogicalUsedSpace;
    oRet.State = this->State;
    return oRet;
}

void Inode::FlushLruCache(uint64_t sliceId)
{
    spdlog::trace("Inode::FlushLruCache");
    InodeLruCache::GetInstance().Put(sliceId, *this);
}

ssize_t Inode::FlushToDisk(uint64_t sliceId, bool UseCoroutine)
{
    ssize_t iRet = 0;
    spdlog::trace("Inode::FlushToDisk");
    Storage::SliceId sid(sliceId);
    auto &di = g_apDiskInfo[sid.Disk()];
    auto offset = di.Chunks[sid.Chunk()].GetInodeOffset(sid.Slice());
    if (UseCoroutine)
    {
        iRet = co_pwrite(di.Fd, this, sizeof(*this), offset);
    }
    else
    {
        iRet = pwrite64(di.Fd, this, sizeof(*this), offset);
    }
    this->FlushLruCache(sliceId);
    return iRet;
}

std::string Inode::ShortDebugString(void) const
{
    return fmt::format(FMT_STRING("Inode: Offset={},LogicalLength={},RefCount={}"), Offset, LogicalLength, RefCount);
}
