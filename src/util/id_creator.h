#ifndef __ID_CREATOR_H__
#define __ID_CREATOR_H__

#include "locker.h"

#include <assert.h>

namespace util
{

class IDCreator
{
    public:
        explicit IDCreator(int init_id = 0):
            _id(init_id),
            _locker()
        {
            assert(_id >= 0);
        }

        ~IDCreator() { }

        int nextID()
        {
            Guard<SpinLocker> m(_locker);
            return _id++;
        }

    private:
        int _id;
        SpinLocker _locker;
};

class IDCreatorUnsafe
{
    public:
        explicit IDCreatorUnsafe(int init_id = 0):
            _id(init_id)
        {
            assert(_id >= 0);
        }

        ~IDCreatorUnsafe() { }

        int nextID()
        {
            return _id++;
        }

    private:
        int _id;
};
};

#endif

