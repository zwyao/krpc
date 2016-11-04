#include "current_thread.h"

#include <sys/syscall.h>
#include <sys/types.h>

namespace util
{

namespace detail
{

__thread pid_t cached_tid = 0;

pid_t cacheTid()
{
    if (cached_tid == 0)
        cached_tid = ::syscall(SYS_gettid);
    return cached_tid;
}

}

}

