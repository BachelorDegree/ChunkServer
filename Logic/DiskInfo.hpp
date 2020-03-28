#pragma once

#include <fcntl.h>

namespace libco
{
    class CoMutex;
}

class CoMutexGuard
{
public:
    CoMutexGuard(libco::CoMutex&);
    ~CoMutexGuard(void);
private:
    libco::CoMutex& _CoMutex;
    CoMutexGuard(CoMutexGuard&) = delete;
    CoMutexGuard& operator= (CoMutexGuard&) = delete;
};

struct DiskInfo
{
    int Fd;
    int ChunkCount;
    libco::CoMutex *Mutex;
    DiskInfo(void);
    ~DiskInfo(void);
};