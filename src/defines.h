#ifndef __DEFINES_H__
#define __DEFINES_H__

#if defined(__GNUC__) && __GNUC__ >= 4
        #define likely(x)   (__builtin_expect((x), 1))
        #define unlikely(x) (__builtin_expect((x), 0))
#else
        #define likely(x)   (x)
        #define unlikely(x) (x)
#endif

#ifndef mmax
#define mmax(a,b) (a)>(b)?(a):(b)
#endif

#ifndef mmin
#define mmin(a,b) (a)<(b)?(a):(b)
#endif

#ifndef NET_MANAGER_NUM
#define NET_MANAGER_NUM 64
#endif

#ifndef MAX_CONNECTION_EACH_MANAGER
#define MAX_CONNECTION_EACH_MANAGER 100000
#endif

#endif

