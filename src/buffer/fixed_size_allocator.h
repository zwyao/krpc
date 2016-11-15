#ifndef __FIXED_SIZE_ALLOCATOR_H__
#define __FIXED_SIZE_ALLOCATOR_H__

#include "allocator.h"
#include "buffer.h"
#include "locker.h"

namespace util
{

class FixedSizeAllocator : public Allocator
{
    private:
        class Deallocator : public BufferDeallocator
        {
            public:
                Deallocator(FixedSizeAllocator* allocator):_allocator(allocator) { }
                virtual ~Deallocator() { }
                virtual void deallocate(char* ptr)
                {
                    _allocator->deallocate(ptr);
                }

            private:
                FixedSizeAllocator* _allocator;
        };

    public:
        FixedSizeAllocator();
        explicit FixedSizeAllocator(int block_size);
        FixedSizeAllocator(int block_size, int block_count);

        virtual ~FixedSizeAllocator();

        virtual char* allocate();
        virtual void  deallocate(char* ptr);
        virtual int   getBufferSize() { return _block_size; }

        FixedSizeAllocator::Deallocator* getDeallocator() { return &_deallocator; }

        void init(int block_size, int block_count);
        int getBlockCount() const { return _block_count; }

    private:
        FixedSizeAllocator(const FixedSizeAllocator&);
        FixedSizeAllocator& operator=(const FixedSizeAllocator&);

        class ChunkAllocator;

    private:
        ChunkAllocator* findVicinity(char* ptr);
        char* tryAllocate();

    private:
        ChunkAllocator* _chunks;
        ChunkAllocator* _last_chunk;
        ChunkAllocator* _alloc_chunk;
        ChunkAllocator* _dealloc_chunk;

        int _block_size;
        int _block_count;
        int _chunk_size;
        int _chunk_count;

        Deallocator _deallocator;

        util::MutexLocker _mutex_locker;
};

}

#endif
