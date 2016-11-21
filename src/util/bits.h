#ifndef __BITS_H__
#define __BITS_H__

namespace knet { namespace util { namespace bits {

inline int change2Pow(unsigned int n)
{
    if(!(n & (n-1))) return n;
    n |= n >> 1; n |= n >> 2;
    n |= n >> 4; n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

}}}

#endif

