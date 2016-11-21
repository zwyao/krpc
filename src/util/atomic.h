#ifndef __ATOMIC_H__
#define __ATOMIC_H__

namespace knet { namespace util {

template <typename T>
class AtomicInteger
{
    public:
        AtomicInteger():
            _value(0)
        {
        }

        T getAndAdd(T v)
        {
            return __sync_fetch_and_add(&_value, v);
        }

        T addAndGet(T v)
        {
            return getAndAdd(v) + v;
        }

        T inc()
        {
            return addAndGet(1);
        }

        T dec()
        {
            return addAndGet(-1);
        }

    private:
        volatile T _value;
};

typedef AtomicInteger<int>     AtomicInt32;
typedef AtomicInteger<int64_t> AtomicInt64;

}}

#endif

