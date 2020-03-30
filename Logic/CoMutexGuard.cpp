#include "../define.hpp"
#include "colib/co_mutex.h"

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