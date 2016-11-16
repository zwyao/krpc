#ifndef __WRITE_BUFFER_ALLOCATOR_H__
#define __WRITE_BUFFER_ALLOCATOR_H__

#include "list.h"
#include "bits.h"
#include "buffer.h"

#include <assert.h>

namespace util
{

class WriteBufferAllocator
{
    private:
        struct BufferEntry
        {
            char* buffer;
            util::ListOp::ListNode list_node;
        };

        typedef util::List<BufferEntry, &BufferEntry::list_node> TList;

        class BufferEntryCache
        {
            public:
                BufferEntryCache(int size):
                    _size(size)
                {
                    for (int i = 0; i < _size; ++i)
                    {
                        BufferEntry* entry = new BufferEntry();
                        _list.push_back(entry);
                    }
                }

                ~BufferEntryCache() { }

                BufferEntry* get()
                {
                    if (likely(_list.empty() == false))
                    {
                        BufferEntry* entry = _list.front();
                        _list.pop_front();
                        return entry;
                    }
                    else
                    {
                        return new BufferEntry();
                    }
                }

                void put(BufferEntry* entry)
                {
                    _list.push_back(entry);
                }

            private:
                int _size;
                TList _list;
        };

        class Deallocator : public BufferDeallocator
        {
            public:
                Deallocator(WriteBufferAllocator* allocator):_allocator(allocator) { }
                virtual ~Deallocator() { }
                virtual void deallocate(char* ptr)
                {
                    _allocator->deallocate(ptr);
                }

            private:
                WriteBufferAllocator* _allocator;
        };


    public:
        explicit WriteBufferAllocator(int base_size):
            _base_size(base_size),
            _buffer_entry_cache(1024),
            _deallocator(this)
        {
        }

        // TODO
        ~WriteBufferAllocator()
        {
        }

        char* allocate(int size, int& real_size)
        {
            assert(size > 0);

            int multi_base = bits::change2Pow((size-1)/_base_size + 1);
            int bucket_id = 0;
            int v = 1;
            while (v !=  multi_base)
            {
                v <<= 1;
                ++bucket_id;
            }

            if (unlikely((unsigned int)bucket_id >= BUCKET_SIZE))
                abort();

            if (likely(_bucket[bucket_id].empty() == false))
            {
                BufferEntry* entry = _bucket[bucket_id].front();
                _bucket[bucket_id].pop_front();
                char* buffer = entry->buffer;
                entry->buffer = 0;
                _buffer_entry_cache.put(entry);
                int new_size = *((int*)(buffer-4)) - 8;
                if (unlikely(new_size != (multi_base*_base_size)))
                    abort();
                real_size = new_size;
                return buffer;
            }
            else
            {
                int new_size = multi_base*_base_size + 8;
                char* buffer = (char*)::malloc(new_size);
                if (likely(buffer != 0))
                {
                    real_size = new_size - 8;
                    *((int*)buffer) = bucket_id;
                    *((int*)(buffer+4)) = new_size;
                    return buffer+8;
                }
            }

            abort();

            return 0;
        }

        void deallocate(char* ptr)
        {
            int bucket_id = *((int*)(ptr-8));
            int buffer_size = *((int*)(ptr-4));
            if (likely(((unsigned int)bucket_id < BUCKET_SIZE) && (buffer_size == ((1 << bucket_id)*_base_size + 8))))
            {
                BufferEntry* entry = _buffer_entry_cache.get();
                entry->buffer = ptr;
                _bucket[bucket_id].push_front(entry);
                return;
            }

            abort();
        }

        int getBaseSize() const { return _base_size; }

        WriteBufferAllocator::Deallocator* getDeallocator() { return &_deallocator; }

        void printInfo()
        {
            for (unsigned int i = 0; i < BUCKET_SIZE; ++i)
            {
                fprintf(stderr, "bucket[%d]: base(%d), size(%d)\n", i, _base_size*(1<<i), _bucket[i].size());
            }
        }

    private:
        static const unsigned int BUCKET_SIZE = 32;

    private:
        int _base_size;
        TList _bucket[BUCKET_SIZE];
        BufferEntryCache _buffer_entry_cache;
        Deallocator _deallocator;
};

}

#endif

