#ifndef __IO_WRITE_BUFFER_H__
#define __IO_WRITE_BUFFER_H__

#include "buffer.h"
#include "buffer_list.h"
#include "write_buffer_allocator.h"

#include <assert.h>

namespace knet { namespace util {

class IOWriteBuffer
{
    public:
        IOWriteBuffer():
            _buffer_entry_cache(256)
        {
        }

        ~IOWriteBuffer()
        {
            release();
        }

        int write(int fd, bool& null_loop);
        int write(int fd, BufferList::BufferEntry* entry);

        void append(util::Buffer& buffer, WriteBufferAllocator& allocator);
        void append(BufferList::BufferEntry* entry, WriteBufferAllocator& allocator);

        inline void release()
        {
            while (!_buffer_list.empty())
            {
                BufferList::BufferEntry* entry = _buffer_list.front();
                _buffer_list.pop_front();
                delete entry;
            }
        }

        inline bool empty() const { return (_buffer_list.size() == 0); }
        inline int size() const { return _buffer_list.size(); }

    private:
        BufferList::TList _buffer_list;
        BufferList::BufferEntryCache _buffer_entry_cache;
};

}}

#endif

