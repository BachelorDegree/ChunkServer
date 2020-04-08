#include <list>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <colib/co_aio.h>
#include <colib/co_mutex.h>
#include <spdlog/spdlog.h>

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
    spdlog::trace("LRU.CalculateFirstSliceIn4kBlock - in_sid={}, ret_sid={}", uSliceId, oRetSid.UInt());
    return oRetSid.UInt();
}

static ssize_t Read4KInodes(uint64_t iStartSlice, void *pDest)
{
    Storage::SliceId oSid(iStartSlice);
    auto oChunkInfo = g_apDiskInfo[oSid.Disk()].Chunks[oSid.Chunk()];
    auto uOffset = oChunkInfo.GetInodeOffset(oSid.Slice());
    // std::lock_guard<libco::CoMutex> oGuard(*(oChunkInfo.DiskInfoPtr->Mutex));
    std::lock_guard<libco::CoMutex> oGuard(*oChunkInfo.Mutex);
    auto iRet = co_pread(oChunkInfo.DiskInfoPtr->Fd, pDest, FourKiB, uOffset);
    spdlog::trace("LRU.Read4KInodes - fd={}, pDest={}, nsize={}, offset={}", 
        oChunkInfo.DiskInfoPtr->Fd, reinterpret_cast<uint64_t>(pDest), FourKiB, uOffset);
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
    std::lock_guard<libco::CoMutex> oGuard(PImpl->Mutex);
    auto iFirstIn4K = CalculateFirstSliceIn4kBlock(uSliceId);
    auto pBlock = static_cast<char*>(this->Get4kBlockPointer(uSliceId));
    auto uOffsetDiff = (uSliceId - iFirstIn4K) * sizeof(Inode);
    spdlog::trace("LRU.Get - pBlock={:016x}, uOffsetDiff={}",
        reinterpret_cast<uint64_t>(pBlock), uOffsetDiff);
    memcpy(&oRet, pBlock + uOffsetDiff, sizeof(oRet));
    spdlog::trace("LRU.Get memcpy src={}, size={}",
        reinterpret_cast<uint64_t>(pBlock + uOffsetDiff), sizeof(oRet));
    spdlog::trace("LRU.Get - {}", oRet.ShortDebugString());
    return oRet;
}

void InodeLruCache::Put(uint64_t uSliceId, const Inode& oInode)
{
    std::lock_guard<libco::CoMutex> oGuard(PImpl->Mutex);
    auto iFirstIn4K = CalculateFirstSliceIn4kBlock(uSliceId);
    auto pBlock = static_cast<char*>(this->Get4kBlockPointer(uSliceId));
    auto uOffsetDiff = (uSliceId - iFirstIn4K) * sizeof(oInode);
    spdlog::trace("LRU.Put - {}", oInode.ShortDebugString());
    spdlog::trace("LRU.Put - pBlock={:016x}, uOffsetDiff={}", 
        reinterpret_cast<uint64_t>(pBlock), uOffsetDiff);
    memcpy(pBlock + uOffsetDiff, &oInode, sizeof(oInode));
    spdlog::trace("LRU.Put memcpy dest={}, size={}",
        reinterpret_cast<uint64_t>(pBlock + uOffsetDiff), sizeof(oInode));
}
