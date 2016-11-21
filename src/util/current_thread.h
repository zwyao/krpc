#ifndef __CURRENT_THREAD_H__
#define __CURRENT_THREAD_H__

#include "defines.h"

#include <unistd.h>

namespace knet { namespace util {


namespace detail
{

extern __thread pid_t cached_tid;

pid_t cacheTid();

}

namespace CurrentThread
{

inline pid_t getTid()
{
    if (likely(knet::util::detail::cached_tid != 0))
        return knet::util::detail::cached_tid;

    return knet::util::detail::cacheTid();
}

}

}}

#endif

