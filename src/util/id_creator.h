#ifndef __ID_CREATOR_H__
#define __ID_CREATOR_H__

#include "atomic.h"

namespace util
{

class IDCreator
{
    public:
        explicit IDCreator(int init_id = -1)
        {
            atomic_set(&_id, init_id);
        }

        ~IDCreator() { }

        int nextID()
        {
            return atomic_inc_return(&_id);
        }

    private:
        atomic_t _id;
};

}

#endif

