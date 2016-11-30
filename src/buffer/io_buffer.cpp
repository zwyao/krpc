#include "io_buffer.h"
#include "global.h"

#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <limits.h>

namespace knet { namespace global {

int g_read_io_buffer_init = 1048576;

void buffer_pool_init(int block_size, int block_count)
{
    util::IOBuffer::buffer_pool_init(block_size, block_count);
}

util::Buffer getBuffer(int size)
{
    util::Buffer buffer = util::IOBuffer::getBuffer(size+8);
    buffer.produce_unsafe(8);
    buffer.consume_unsafe(8);
    return buffer;
}

}}

namespace 
{

inline int read_data(int fd, int max_read, knet::util::Buffer& buffer)
{
    char extrabuf[65536];
    struct iovec read_vec[2];
    read_vec[0].iov_base = buffer.producer();
    read_vec[0].iov_len = max_read;
    read_vec[1].iov_base = extrabuf;
    read_vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (max_read < (int)sizeof(extrabuf)) ? 2 : 1;
    const int nread = ::readv(fd, read_vec, iovcnt);
    if (nread < 0)
    {
        return (errno > 0 ? -errno : errno);
    }
    else if (nread <= max_read)
    {
        buffer.produce_unsafe(nread);
    }
    else
    {
        fprintf(stderr, "+++++++++++++++++++++++++++++++++++++++++++++++%d: %d\n", max_read, nread);
        int total_size = buffer.capacity() + nread;
        int mcp_size1 = buffer.getAvailableDataSize() + max_read;

        knet::util::Buffer new_buffer(total_size);

        memcpy(new_buffer.getBuffer(), buffer.consumer(), mcp_size1);
        memcpy(new_buffer.getBuffer()+mcp_size1, extrabuf, nread-max_read);
        new_buffer.produce_unsafe(mcp_size1+nread-max_read);

        buffer = new_buffer;
    }

    return nread;
}

}

namespace knet { namespace util {

FixedSizeAllocator IOBuffer::_buffer_allocator;
bool IOBuffer::_buffer_init = false;
int  IOBuffer::_buffer_size = 4096;

void IOBuffer::buffer_pool_init(int block_size, int block_count)
{
    if (_buffer_init == false)
    {
        _buffer_allocator.init(block_size, block_count);
        _buffer_init = true;
        _buffer_size = block_size;
    }
}

Buffer IOBuffer::getBuffer(int size)
{
    if (likely(_buffer_init && size <= _buffer_size))
    {
        char* mem = _buffer_allocator.allocate();
        if (likely(mem != 0))
            return Buffer(mem, _buffer_size, _buffer_allocator.getDeallocator());
        else
            return size > 0 ? Buffer(size) : Buffer(_buffer_size);
    }
    else
    {
        return size > 0 ? Buffer(size) : Buffer(_buffer_size);
    }
}

int IOBuffer::getBufferSize() { return _buffer_size; }

int IOBuffer::read(int fd, int max)
{
    if (unlikely(_buffer_list.empty() == true))
    {
        util::Buffer buffer(global::g_read_io_buffer_init);
        _buffer_list.push_back(new BufferList::BufferEntry(buffer, 0, 0));
    }
    assert(_buffer_list.size() == 1);
    max = _buffer_list.front()->buffer.getAvailableSpaceSize();
    return read_data(fd, max, _buffer_list.front()->buffer);
}

int IOBuffer::write(int fd, bool& null_loop)
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
int IOBuffer::write(int fd, BufferList::BufferEntry* entry)
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


void IOBuffer::append(util::Buffer& buffer, WriteBufferAllocator& allocator)
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

void IOBuffer::append(BufferList::BufferEntry* entry, WriteBufferAllocator& allocator)
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

int IOBuffer::read(int fd, int max_read, Buffer& buffer)
{
    char* ptr = buffer.producer();
    int nread = ::read(fd, ptr, max_read);
    if (nread > 0)
        buffer.produce_unsafe(nread);
    return nread >= 0 ? nread : (errno > 0 ? -errno : errno);
}

}}

