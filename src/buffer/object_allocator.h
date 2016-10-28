#ifndef __OBJECT_ALLOCATOR_H__
#define __OBJECT_ALLOCATOR_H__

#include "fixed_size_allocator.h"

#include <stdio.h>

#include <new>

namespace util
{

template <typename T>
class ObjectAllocator
{
    public:
        ObjectAllocator();
        ObjectAllocator(int nobj);
        explicit ObjectAllocator(FixedSizeAllocator* allocator);
        ~ObjectAllocator();

        void init(int nobj);
        T* allocate();
        void  deallocate(T* ptr);

    private:
        void construtor(void* p)
        {
            new (p) T();
        }

        void destructor(T* p)
        {
            p->~T();
        }

    private:
        ObjectAllocator(const ObjectAllocator&);
        ObjectAllocator& operator=(const ObjectAllocator&);

    private:
        FixedSizeAllocator* _allocator;
        bool _allocator_own;
};

template <typename T>
ObjectAllocator<T>::ObjectAllocator():
    _allocator(0),
    _allocator_own(false)
{
}

template <typename T>
ObjectAllocator<T>::ObjectAllocator(int nobj)
{
    assert(nobj > 0);

    _allocator = new FixedSizeAllocator(sizeof(T), nobj);
    _allocator_own = true;

    assert(_allocator != 0);
}

template <typename T>
ObjectAllocator<T>::ObjectAllocator(FixedSizeAllocator* allocator):
    _allocator(allocator),
    _allocator_own(false)
{
    assert(allocator != NULL);
}

template <typename T>
ObjectAllocator<T>::~ObjectAllocator()
{
    if (_allocator_own == true)
    {
        delete _allocator;
        _allocator = NULL;
    }
}

template <typename T>
void ObjectAllocator<T>::init(int nobj)
{
    assert(_allocator == 0);
    assert(nobj > 0);

    _allocator = new FixedSizeAllocator(sizeof(T), nobj);
    _allocator_own = true;

    assert(_allocator != 0);
}

template <typename T>
T* ObjectAllocator<T>::allocate()
{
    void* p = _allocator->allocate();
    construtor(p);
    return (T*)p;
}

template <typename T>
void ObjectAllocator<T>::deallocate(T* p)
{
    destructor(p);
    _allocator->deallocate((char*)p);
}

}

#endif

