#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include "buffer.h"
#include "buffer_list.h"
#include "write_buffer_allocator.h"
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

        void append(util::Buffer& buffer, WriteBufferAllocator& allocator);
        void append(BufferList::BufferEntry* entry, WriteBufferAllocator& allocator);

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

