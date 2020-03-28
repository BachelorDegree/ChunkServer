#pragma once

#include <cmath>
#include <cstdint>

constexpr uint32_t TwoKiB = 2048; // in Bytes
constexpr uint32_t FourKiB = 4096; // in Bytes

struct ChunkHeader
{
    char     Magic[16];
    int32_t  Version;
    uint64_t ChunkId;
    uint32_t NextInode;
    uint32_t LogicalUsedSpace;
    uint32_t ActualUsedSpace;
    ChunkHeader(void) = default;
    ChunkHeader(uint64_t ChunkId):
        Version(1), ChunkId(ChunkId),
        NextInode(0), LogicalUsedSpace(0), ActualUsedSpace(0)
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
    uint32_t Offset;
    uint32_t LogicalLength;
    uint32_t RefCount;
    uint32_t ActualLength(void) const
    {
        return static_cast<uint32_t>(ceil((LogicalLength / static_cast<double>(FourKiB))) * FourKiB);
    }
};