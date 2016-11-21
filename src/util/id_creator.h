#ifndef __ID_CREATOR_H__
#define __ID_CREATOR_H__

#include "atomic.h"

namespace knet { namespace util {

class IDCreator
{
    public:
        IDCreator() { }
        ~IDCreator() { }

        int64_t nextID()
        {
            return _id.getAndAdd(1);
        }

    private:
        AtomicInt64 _id;
};

}}

#endif

