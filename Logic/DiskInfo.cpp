#include <cstring>
#include "define.hpp"
#include "Logic.hpp"
#include "DiskInfo.hpp"
#include "colib/co_mutex.h"
#include "colib/co_aio.h"
#include "../CoreDeps/include/SliceId.hpp"

CoMutexGuard::CoMutexGuard(libco::CoMutex &m):
    _CoMutex(m)
{
    _CoMutex.lock();
}

CoMutexGuard::CoMutexGuard(libco::CoMutex *m):
    _CoMutex(*m)
{
    _CoMutex.lock();
}

CoMutexGuard::~CoMutexGuard(void)
{
    _CoMutex.unlock();
}

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

ChunkInfo::ChunkInfo(void):
    Mutex(new libco::CoMutex)
{

}

ChunkInfo::~ChunkInfo(void)
{
    if (Mutex != nullptr)
        delete Mutex;
}

ssize_t ChunkInfo::FlushToDisk(bool UseCoroutine)
{
    ChunkHeader ch;
    ch.ChunkId = ChunkId.UInt();
    ch.NextInode = NextInode;
    ch.ActualUsedSpace = ActualUsedSpace;
    ch.LogicalUsedSpace = LogicalUsedSpace;
    auto base_offset = ChunkId.Chunk() * ChunkLength + ChunkHeaderOffset;
    auto &di = GDiskInfo[ChunkId.Disk()];
    if (UseCoroutine)
    {
        return co_pwrite(di.Fd, &ch, sizeof(ch), base_offset);
    }
    else
    {
        return pwrite64(di.Fd, &ch, sizeof(ch), base_offset);
    }
}

void Inode::FlushLruCache(uint64_t sliceId)
{
    // Todo
}

ssize_t Inode::FlushToDisk(uint64_t sliceId, bool UseCoroutine)
{
    Storage::SliceId sid(sliceId);
    this->FlushLruCache(sliceId);
    loff_t inode_base_offset = 4096;
    loff_t offset = ChunkLength * sid.Chunk() + 
        inode_base_offset + sid.Slice() * sizeof(*this);
    auto &di = GDiskInfo[sid.Disk()];
    if (UseCoroutine)
    {
        return co_pwrite(di.Fd, this, sizeof(*this), offset);
    }
    else
    {
        return pwrite64(di.Fd, this, sizeof(*this), offset);
    }
}