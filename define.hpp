#pragma once

#include <cmath>
#include <cstdint>

constexpr uint32_t TwoKiB = 2048; // in Bytes
constexpr uint32_t FourKiB = 4096; // in Bytes
constexpr uint64_t AioMinimumLength = 4096;
constexpr auto ChunkLength = 1LU << 31; // 2 GiB

constexpr uint32_t ChunkHeaderOffset = 0;
constexpr uint32_t ChunkInodeSectionOffset = 4096;
constexpr uint32_t ChunkDataSectionOffset = 16781312;

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

struct ChunkHeader
{
    char     Magic[16];
    int32_t  Version;
    uint64_t ChunkId;
    uint32_t NextInode;
    uint32_t LogicalUsedSpace;
    uint32_t ActualUsedSpace;
    uint32_t State;
    ChunkHeader(void):
        Version(1), ChunkId(0),
        NextInode(0), LogicalUsedSpace(0), ActualUsedSpace(0),
        State(4)
    {
        Magic[0] = 'A';
        Magic[1] = 'L';
        Magic[2] = 'O';
        Magic[3] = 'H';
        Magic[4] = 'A';
        Magic[5] = '\0';
    }
    ChunkHeader(uint64_t ChunkId):
        Version(1), ChunkId(ChunkId),
        NextInode(0), LogicalUsedSpace(0), ActualUsedSpace(0),
        State(4)
    {
        Magic[0] = 'A';
        Magic[1] = 'L';
        Magic[2] = 'O';
        Magic[3] = 'H';
        Magic[4] = 'A';
        Magic[5] = '\0';
    }
};

struct Inode
{
    uint32_t Offset; // 相对data段起点的偏移
    uint32_t LogicalLength;
    uint32_t RefCount;
    uint32_t Padding;
    uint32_t ActualLength(void) const
    {
        return static_cast<uint32_t>(ceil((LogicalLength / static_cast<double>(FourKiB))) * FourKiB);
    }
    size_t SizeOfInode(void) const { return sizeof(Inode); }
    void FlushLruCache(uint64_t sliceId);
    ssize_t FlushToDisk(uint64_t sliceId, bool UseCoroutine = true);
};