#pragma once

#include "../define.hpp"

class InodeLruCacheImpl;

// 读取操作需要取得chunk锁！
class InodeLruCache
{
public:
    Inode   Get(uint64_t sliceId);
    void    Put(uint64_t sliceId, const Inode& inode);
    void    InitializeMemoryPool(uint64_t size);

    static  InodeLruCache& GetInstance(void);
private:
    InodeLruCacheImpl* PImpl;
    InodeLruCache(void);
    ~InodeLruCache(void);
    InodeLruCache(InodeLruCache&) = delete;
    void* Get4kBlockPointer(uint64_t uSliceId);
};