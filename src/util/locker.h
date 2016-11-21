#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

namespace knet { namespace util {

class MutexLocker
{
    public:
        MutexLocker()
        {
            pthread_mutex_init(&_locker, 0);
        }

        ~MutexLocker()
        {
            pthread_mutex_destroy(&_locker);
        }

        void lock()
        {
            pthread_mutex_lock(&_locker);
        }

        void unlock()
        {
            pthread_mutex_unlock(&_locker);
        }

    private:
        pthread_mutex_t _locker;
};

class SpinLocker
{
    public:
        SpinLocker()
        {
            int ret = pthread_spin_init(&_locker, PTHREAD_PROCESS_PRIVATE);
            if (ret != 0)
            {
                fprintf(stderr, "SpinLocker init error\n");
                abort();
            }
        }

        ~SpinLocker()
        {
            int ret = pthread_spin_destroy(&_locker);
            if (ret != 0)
            {
                fprintf(stderr, "SpinLocker destroy error\n");
                abort();
            }
        }

        void lock()
        {
            pthread_spin_lock(&_locker);
        }

        void unlock()
        {
            pthread_spin_unlock(&_locker);
        }

    private:
        pthread_spinlock_t _locker;
};

template <typename T>
class Guard
{
    public:
        Guard(T& locker):
            _locker(locker)
        {
            _locker.lock();
        }

        ~Guard()
        {
            _locker.unlock();
        }

    private:
        T& _locker;
};

}}

#endif

