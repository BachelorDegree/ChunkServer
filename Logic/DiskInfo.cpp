#include "DiskInfo.hpp"
#include "colib/co_mutex.h"

CoMutexGuard::CoMutexGuard(libco::CoMutex &m):
    _CoMutex(m)
{
    _CoMutex.lock();
}

CoMutexGuard::~CoMutexGuard(void)
{
    _CoMutex.unlock();
}

DiskInfo::DiskInfo(void):
    Fd(0), ChunkCount(0),
    Mutex(new libco::CoMutex)
{

}

DiskInfo::~DiskInfo(void)
{
    if (Mutex != nullptr)
        delete Mutex;
    if (Fd != 0)
        close(Fd);
}