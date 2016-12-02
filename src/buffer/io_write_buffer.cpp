#include "io_write_buffer.h"
#include "global.h"

#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <limits.h>

namespace knet { namespace util {

int IOWriteBuffer::write(int fd, bool& null_loop)
{
    const int size = _buffer_list.size();
    if (unlikely(size == 0))
    {
        null_loop = true;
        return 0;
    }

    const int writev_size = mmin(IOV_MAX, 1024);
    const int sendv_size = size > writev_size ? writev_size : size;
    struct iovec write_vec[sendv_size];

    int will_send_size = 0;
    int vec_idx = 0;
    BufferList::BufferEntry* entry = _buffer_list.front();
    while (vec_idx < sendv_size)
    {
        const int nbytes = entry->buffer.getAvailableDataSize();
        write_vec[vec_idx].iov_base = entry->buffer.consumer();
        write_vec[vec_idx].iov_len = nbytes;
        will_send_size += nbytes;
        entry = _buffer_list.next(entry);
        ++vec_idx;
    }

    assert(will_send_size > 0);

    const int nwrite = ::writev(fd, write_vec, vec_idx);
    if (nwrite == will_send_size)
    {
        int i = 0;
        while (i < sendv_size)
        {
            BufferList::BufferEntry* entry = _buffer_list.front();
            _buffer_list.pop_front();
            _buffer_entry_cache.put(entry);
            ++i;
        }
        return nwrite;
    }
    else if (nwrite > 0) // 发送了部分数据
    {
        int nbytes = nwrite;
        BufferList::BufferEntry* entry = _buffer_list.front();
        int have_data;
        while ((have_data = entry->buffer.getAvailableDataSize()) <= nbytes)
        {
            nbytes -= have_data;
            _buffer_list.pop_front();
            _buffer_entry_cache.put(entry);
            entry = _buffer_list.front();
        }

        entry->buffer.consume_unsafe(nbytes);
        /*
        if (_buffer_list.size() == 1)
            entry->buffer.compact();
        */

        /*
        if (_buffer_list.size() > 0)
            fprintf(stderr, "writev: %d:%d------------------------------------%d:%d:%d\n",
                    sendv_size,
                    vec_idx,
                    _buffer_list.size(),
                    nwrite,
                    will_send_size);
                    */

        return nwrite;
    }

    return nwrite >= 0 ? nwrite : (errno > 0 ? -errno : errno);
}

/*
 * 首先尝试立刻发送
 * 这个方案，影响吞吐量
 */
int IOWriteBuffer::write(int fd, BufferList::BufferEntry* entry)
{
    const int nbytes = entry->buffer.getAvailableDataSize();
    const int size = _buffer_list.size();
    if (size == 0)
    {
        int ret = ::write(fd, entry->buffer.consumer(), nbytes);
        if (ret == nbytes)
        {
            delete entry;
            return ret;
        }
        else if (ret >= 0)
        {
            entry->buffer.consume_unsafe(ret);
            _buffer_list.push_back(entry);
            return ret;
        }
        else if (errno == EAGAIN || errno == EINTR)
        {
            _buffer_list.push_back(entry);
            return nbytes;
        }
        else
        {
            delete entry;
            return -1;
        }
    }
    else
    {
        _buffer_list.push_back(entry);
        return nbytes;
    }
}

void IOWriteBuffer::append(util::Buffer& buffer, WriteBufferAllocator& allocator)
{
    int send_sz = buffer.getAvailableDataSize();
    if (unlikely(send_sz <= 0))
        return;

    if (send_sz <= BIG_PACKET_THRESHHOLD)
    {
        if (_buffer_list.empty())
        {
            int real_sz = 0;
            char* ptr = allocator.allocate(send_sz, real_sz);
            util::Buffer new_buffer(ptr, real_sz, allocator.getDeallocator());

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

            int real_sz = 0;
            int expect_sz = pre_buffer.capacity();
            expect_sz = (expect_sz >= 1048576 ? expect_sz : 2*expect_sz);
            char* ptr = allocator.allocate(expect_sz, real_sz);
            util::Buffer new_buffer(ptr, real_sz, allocator.getDeallocator());

            BufferList::BufferEntry* entry = _buffer_entry_cache.get();
            entry->buffer = new_buffer;
            _buffer_list.push_back(entry);
        }

        BufferList::BufferEntry* back = _buffer_list.back();
        ::memcpy(back->buffer.producer(), buffer.consumer(), send_sz);
        back->buffer.produce_unsafe(send_sz);
        buffer.consume_unsafe(send_sz);
    }
    else
    {
        BufferList::BufferEntry* entry = _buffer_entry_cache.get();
        entry->buffer = buffer;
        _buffer_list.push_back(entry);
    }
}

void IOWriteBuffer::append(BufferList::BufferEntry* entry, WriteBufferAllocator& allocator)
{
    util::Buffer& data_buffer = entry->buffer;
    int send_sz = data_buffer.getAvailableDataSize();
    if (unlikely(send_sz <= 0))
    {
        delete entry;
        return;
    }

    if (send_sz <= BIG_PACKET_THRESHHOLD)
    {
        if (_buffer_list.empty())
        {
            int real_sz = 0;
            char* ptr = allocator.allocate(send_sz, real_sz);
            util::Buffer new_buffer(ptr, real_sz, allocator.getDeallocator());

            BufferList::BufferEntry* new_entry = _buffer_entry_cache.get();
            new_entry->buffer = new_buffer;
            _buffer_list.push_back(new_entry);
        }
        else if (_buffer_list.back()->buffer.getAvailableSpaceSize() < send_sz)
        {
            util::Buffer& pre_buffer = _buffer_list.back()->buffer;
            int cp_sz = pre_buffer.getAvailableSpaceSize();
            ::memcpy(pre_buffer.producer(), data_buffer.consumer(), cp_sz);
            pre_buffer.produce_unsafe(cp_sz);
            data_buffer.consume_unsafe(cp_sz);
            send_sz -= cp_sz;

            int real_sz = 0;
            int expect_sz = pre_buffer.capacity();
            expect_sz = (expect_sz >= 1048576 ? expect_sz : 2*expect_sz);
            char* ptr = allocator.allocate(expect_sz, real_sz);
            util::Buffer new_buffer(ptr, real_sz, allocator.getDeallocator());

            BufferList::BufferEntry* new_entry = _buffer_entry_cache.get();
            new_entry->buffer = new_buffer;
            _buffer_list.push_back(new_entry);
        }

        BufferList::BufferEntry* back = _buffer_list.back();
        ::memcpy(back->buffer.producer(), data_buffer.consumer(), send_sz);
        back->buffer.produce_unsafe(send_sz);
        data_buffer.consume_unsafe(send_sz);

        delete entry;
        return;
    }
    else
    {
        BufferList::BufferEntry* new_entry = _buffer_entry_cache.get();
        new_entry->buffer = data_buffer;
        _buffer_list.push_back(new_entry);

        delete entry;
        return;
    }
}

}}

