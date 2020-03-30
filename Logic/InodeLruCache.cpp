#include <list>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <colib/co_aio.h>
#include <colib/co_mutex.h>

#include "Logic.hpp"
#include "InodeLruCache.hpp"
#include "coredeps/SliceId.hpp"

class InodeLruCacheImpl
{
    friend class InodeLruCache;
    char *MemPool;
    libco::CoMutex Mutex;
    size_t Capacity; // 4K blocks count
    int NextAllocatableIndex;
    std::list<std::pair<uint64_t, char*>> List; // Pointers to block
    std::unordered_map<uint64_t, std::list<std::pair<uint64_t, char*>>::iterator> Map; // 4K块的起始分片号到迭代器映射
    
    InodeLruCacheImpl(void):
        MemPool(nullptr), NextAllocatableIndex(0) { }
    ~InodeLruCacheImpl(void)
    {
        if (MemPool != nullptr)
            delete[] MemPool;
    }
};

InodeLruCache& InodeLruCache::GetInstance(void)
{
    static InodeLruCache oInstance;
    return oInstance;
}

InodeLruCache::InodeLruCache(void):
    PImpl(nullptr)
{
    
}

InodeLruCache::~InodeLruCache(void)
{
    if (PImpl != nullptr)
        delete PImpl;
}

static uint64_t CalculateFirstSliceIn4kBlock(uint64_t uSliceId)
{
    constexpr int InodeNumberIn4K = FourKiB / sizeof(Inode);
    Storage::SliceId oSid(uSliceId);
    auto oRetSid = oSid;
    auto iSliceNumber = oSid.Slice();
    oRetSid.SetSlice((iSliceNumber / InodeNumberIn4K) * InodeNumberIn4K);
    return oRetSid.UInt();
}

static ssize_t Read4KInodes(uint64_t iStartSlice, void *pDest)
{
    Storage::SliceId oSid(iStartSlice);
    auto oChunkInfo = GDiskInfo[oSid.Disk()].Chunks[oSid.Chunk()];
    auto uOffset = oChunkInfo.GetInodeOffset(oSid.Slice());
    CoMutexGuard guard(oChunkInfo.DiskInfoPtr->Mutex);
    auto iRet = co_pread(oChunkInfo.DiskInfoPtr->Fd, pDest, FourKiB, uOffset);
    return iRet;
}

void InodeLruCache::InitializeMemoryPool(uint64_t size)
{
    PImpl = new InodeLruCacheImpl;
    PImpl->MemPool = new char[size];
    if (PImpl->MemPool == nullptr)
        throw std::runtime_error("Cannot allocate memory for InodeLruCache");
    PImpl->Capacity = size / FourKiB;
}

void* InodeLruCache::Get4kBlockPointer(uint64_t uSliceId)
{
    CoMutexGuard guard(PImpl->Mutex);
    void *pRet = nullptr;
    auto iFirstIn4K = CalculateFirstSliceIn4kBlock(uSliceId); // 用这个作为key
    auto it = PImpl->Map.find(iFirstIn4K);
    if (it == PImpl->Map.end())
    {
        // Cache miss, 读取
        if (PImpl->List.size() >= PImpl->Capacity)
        {
            // 空间已满，移除一个
            auto itElementToRemove = PImpl->List.back();
            pRet = itElementToRemove.second;
            auto iKeyToRemove = itElementToRemove.first;
            PImpl->Map.erase(iKeyToRemove);
            PImpl->List.pop_back();
        }
        else
        {
            pRet = PImpl->MemPool + FourKiB * PImpl->NextAllocatableIndex;
            ++PImpl->NextAllocatableIndex;
        }
        // 添加新的
        PImpl->List.push_front(std::make_pair(iFirstIn4K, static_cast<char*>(pRet)));
        PImpl->Map[iFirstIn4K] = PImpl->List.begin();
        it = PImpl->Map.find(iFirstIn4K);
        Read4KInodes(iFirstIn4K, pRet);
    }
    else
    {
        // 命中，进行LRU操作
        pRet = it->second->second; // 到内存池的指针
        // 清掉链表中的对应元素，并在头部插入新的
        PImpl->List.erase(it->second);
        PImpl->List.push_front(std::make_pair(iFirstIn4K, static_cast<char*>(pRet)));
        it->second = PImpl->List.begin();
    }
    return pRet;
}

Inode InodeLruCache::Get(uint64_t uSliceId)
{
    Inode oRet;
    auto iFirstIn4K = CalculateFirstSliceIn4kBlock(uSliceId);
    auto pBlock = static_cast<char*>(this->Get4kBlockPointer(uSliceId));
    auto uOffsetDiff = (uSliceId - iFirstIn4K) * sizeof(Inode);
    CoMutexGuard guard(PImpl->Mutex);
    memcpy(&oRet, pBlock + uOffsetDiff, sizeof(Inode));
    return oRet;
}

void InodeLruCache::Put(uint64_t uSliceId, const Inode& oInode)
{
    auto iFirstIn4K = CalculateFirstSliceIn4kBlock(uSliceId);
    auto pBlock = static_cast<char*>(this->Get4kBlockPointer(uSliceId));
    auto uOffsetDiff = (uSliceId - iFirstIn4K) * sizeof(oInode);
    CoMutexGuard guard(PImpl->Mutex);
    memcpy(pBlock + uOffsetDiff, &oInode, sizeof(oInode));
}
