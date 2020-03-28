#pragma once

#include <cstdio>
#include <fcntl.h>
#include "../CoreDeps/include/SliceId.hpp"

namespace libco
{
    class CoMutex;
}

class CoMutexGuard
{
public:
    CoMutexGuard(libco::CoMutex&);
    CoMutexGuard(libco::CoMutex*);
    ~CoMutexGuard(void);
private:
    libco::CoMutex& _CoMutex;
    CoMutexGuard(CoMutexGuard&) = delete;
    CoMutexGuard& operator= (CoMutexGuard&) = delete;
};

struct ChunkInfo
{
    Storage::SliceId ChunkId;
    uint32_t NextInode;
    uint32_t LogicalUsedSpace;
    uint32_t ActualUsedSpace;
    libco::CoMutex *Mutex;
    ChunkInfo(void);
    ~ChunkInfo(void);
    ssize_t FlushToDisk(bool UseCoroutine = true);
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