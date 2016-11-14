#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include "buffer.h"
#include "buffer_list.h"
#include "fixed_size_allocator.h"

#include <assert.h>

namespace util
{

class IOBuffer
{
    public:
        IOBuffer():
            _buffer_entry_cache(256)
        {
        }

        ~IOBuffer()
        {
            release();
        }

        int read(int fd, int max = -1);
        int write(int fd, bool& null_loop);
        int write(int fd, BufferList::BufferEntry* entry);

        inline void release()
        {
            while (!_buffer_list.empty())
            {
                BufferList::BufferEntry* entry = _buffer_list.front();
                _buffer_list.pop_front();
                delete entry;
            }
        }

        inline void append(util::Buffer& buffer)
        {
            int send_sz = buffer.getAvailableDataSize();
            if (_buffer_list.empty())
            {
                int new_sz = send_sz > 65536 ? (send_sz+4095)&~4095 : 65536;
                util::Buffer new_buffer(new_sz);
                BufferList::BufferEntry* entry = _buffer_entry_cache.get();
                entry->buffer = new_buffer;
                _buffer_list.push_back(entry);
            }
            else if (_buffer_list.back()->buffer.getAvailableSpaceSize() < send_sz)
            {
                util::Buffer& pre_buffer = _buffer_list.back()->buffer;
                int cp_sz = pre_buffer.getAvailableSpaceSize();
                ::memcpy(pre_buffer.producer(), buffer.consumer(), cp_sz);
                pre_buffer.produce_unsafe(cp_sz);
                buffer.consume_unsafe(cp_sz);
                send_sz -= cp_sz;

                util::Buffer new_buffer(2*pre_buffer.capacity());
                BufferList::BufferEntry* entry = _buffer_entry_cache.get();
                entry->buffer = new_buffer;
                _buffer_list.push_back(entry);
            }

            BufferList::BufferEntry* back = _buffer_list.back();
            ::memcpy(back->buffer.producer(), buffer.consumer(), send_sz);
            back->buffer.produce_unsafe(send_sz);
            buffer.consume_unsafe(send_sz);
            /*
               fprintf(stderr, "++++++++++++++++++:%d:%d\n",
               _buffer_list.size(),
               _buffer_list.prev(end)->buffer.getAvailableDataSize());
               */
            return;
        }

        inline void append(BufferList::BufferEntry* entry)
        {
            util::Buffer& buffer = entry->buffer;
            int send_sz = buffer.getAvailableDataSize();
            if (_buffer_list.empty())
            {
                int new_sz = send_sz > 65536 ? (send_sz+4095)&~4095 : 65536;
                util::Buffer new_buffer(new_sz);
                BufferList::BufferEntry* new_entry = _buffer_entry_cache.get();
                new_entry->buffer = new_buffer;
                _buffer_list.push_back(new_entry);
            }
            else if (_buffer_list.back()->buffer.getAvailableSpaceSize() < send_sz)
            {
                util::Buffer& pre_buffer = _buffer_list.back()->buffer;
                int cp_sz = pre_buffer.getAvailableSpaceSize();
                ::memcpy(pre_buffer.producer(), buffer.consumer(), cp_sz);
                pre_buffer.produce_unsafe(cp_sz);
                buffer.consume_unsafe(cp_sz);
                send_sz -= cp_sz;

                util::Buffer new_buffer(2*pre_buffer.capacity());
                BufferList::BufferEntry* new_entry = _buffer_entry_cache.get();
                new_entry->buffer = new_buffer;
                _buffer_list.push_back(new_entry);
            }

            BufferList::BufferEntry* back = _buffer_list.back();
            ::memcpy(back->buffer.producer(), buffer.consumer(), send_sz);
            back->buffer.produce_unsafe(send_sz);
            buffer.consume_unsafe(send_sz);

            delete entry;
            /*
               fprintf(stderr, "++++++++++++++++++:%d:%d\n",
               _buffer_list.size(),
               _buffer_list.prev(end)->buffer.getAvailableDataSize());
               */
            return;
        }

        // 读buffer只有一个
        inline Buffer& getReadBuffer()
        {
            assert(_buffer_list.size() == 1);
            return _buffer_list.front()->buffer;
        }

        inline bool empty() const { return (_buffer_list.size() == 0); }
        inline int size() const { return _buffer_list.size(); }

    private:
        int read(int fd, int max_read, Buffer& buffer);

    private:
        BufferList::TList _buffer_list;
        BufferList::BufferEntryCache _buffer_entry_cache;

    public:
        static void small_buffer_pool_init(int block_size, int block_count);
        static void large_buffer_pool_init(int block_size, int block_count);
        static void small_buffer_deallocator(char* ptr);
        static void large_buffer_deallocator(char* ptr);
        static Buffer getSmallBuffer(int size);
        static Buffer getLargeBuffer(int size);
        static int getSmallBufferSize();
        static int getLargeBufferSize();

    private:
        static FixedSizeAllocator _small_buffer_allocator;
        static FixedSizeAllocator _large_buffer_allocator;

        static bool _small_buffer_init;
        static int  _small_buffer_size;

        static bool _large_buffer_init;
        static int  _large_buffer_size;
};

}

#endif

