#pragma once

#include <cstdio>
#include <fcntl.h>
#include "coredeps/SliceId.hpp"

#include "../define.hpp"

namespace libco
{
    class CoMutex;
}

struct DiskInfo;

struct ChunkInfo
{
    Storage::SliceId ChunkId;
    uint32_t NextInode;
    uint32_t LogicalUsedSpace;
    uint32_t ActualUsedSpace;
    libco::CoMutex *Mutex;
    DiskInfo* DiskInfoPtr;
    explicit ChunkInfo(DiskInfo* pParent = nullptr);
    ~ChunkInfo(void);
    void SetDiskInfoPtr(DiskInfo *pParent);
    ssize_t FlushToDisk(bool UseCoroutine = true);
    ChunkHeader GetChunkHeader(void) const;
    off_t GetBaseOffset(void) const;
    off_t GetInodeOffset(uint64_t iSliceNumber) const;
    off_t GetDataSectionOffset(void) const;
    off_t GetInodeSectionOffset(void) const;
};

struct DiskInfo
{
    int Fd;
    int ChunkCount;
    ChunkInfo* Chunks;
    libco::CoMutex *Mutex;
    DiskInfo(void);
    ~DiskInfo(void);
};