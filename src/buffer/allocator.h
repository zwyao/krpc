#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

namespace util
{

class Allocator
{
    public:
        Allocator() {}
        virtual ~Allocator() {}

        virtual char* allocate() = 0;
        virtual void  deallocate(char* ptr) = 0;
        virtual int   getBufferSize()  = 0;
};

}

#endif

