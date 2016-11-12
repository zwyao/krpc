#include "io_buffer.h"

#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <limits.h>

namespace global
{

int g_read_io_buffer_init = 1024000;

void small_buffer_pool_init(int block_size, int block_count)
{
    util::IOBuffer::small_buffer_pool_init(block_size, block_count);
}

void large_buffer_pool_init(int block_size, int block_count)
{
    util::IOBuffer::large_buffer_pool_init(block_size, block_count);
}

util::Buffer getSmallBuffer(int size)
{
    util::Buffer buffer = util::IOBuffer::getSmallBuffer(size+8);
    buffer.produce_unsafe(8);
    buffer.consume_unsafe(8);
    return buffer;
}

util::Buffer getLargeBuffer(int size)
{
    util::Buffer buffer = util::IOBuffer::getLargeBuffer(size+8);
    buffer.produce_unsafe(8);
    buffer.consume_unsafe(8);
    return buffer;
}

}

namespace 
{

inline int read_data(int fd, int max_read, util::Buffer& buffer)
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
        int total_size = buffer.capacity() + nread;
        int mcp_size1 = buffer.getAvailableDataSize() + max_read;

        util::Buffer new_buffer(total_size);

        memcpy(new_buffer.getBuffer(), buffer.consumer(), mcp_size1);
        memcpy(new_buffer.getBuffer()+mcp_size1, extrabuf, nread-max_read);
        new_buffer.produce_unsafe(mcp_size1+nread-max_read);

        buffer = new_buffer;
    }

    return nread;
}

}

namespace util
{

FixedSizeAllocator IOBuffer::_small_buffer_allocator;
FixedSizeAllocator IOBuffer::_large_buffer_allocator;

bool IOBuffer::_small_buffer_init = false;
bool IOBuffer::_large_buffer_init = false;

int  IOBuffer::_small_buffer_size = 4096;
int  IOBuffer::_large_buffer_size = 4096;

void IOBuffer::small_buffer_pool_init(int block_size, int block_count)
{
    _small_buffer_allocator.init(block_size, block_count);
    _small_buffer_init = true;
    _small_buffer_size = block_size;
}

void IOBuffer::large_buffer_pool_init(int block_size, int block_count)
{
    _large_buffer_allocator.init(block_size, block_count);
    _large_buffer_init = true;
    _large_buffer_size = block_size;
}

void IOBuffer::small_buffer_deallocator(char* ptr)
{
    _small_buffer_allocator.deallocate(ptr);
}

void IOBuffer::large_buffer_deallocator(char* ptr)
{
    _large_buffer_allocator.deallocate(ptr);
}

Buffer IOBuffer::getSmallBuffer(int size)
{
    if (likely(_small_buffer_init && size <= _small_buffer_size))
    {
        char* mem = _small_buffer_allocator.allocate();
        if (likely(mem != 0))
            return Buffer(mem, _small_buffer_size, small_buffer_deallocator);
        else
            return size > 0 ? Buffer(size) : Buffer(_small_buffer_size);
    }
    else
    {
        return size > 0 ? Buffer(size) : Buffer(_small_buffer_size);
    }
}

Buffer IOBuffer::getLargeBuffer(int size)
{
    if (likely(_large_buffer_init && size <= _large_buffer_size))
    {
        char* mem = _large_buffer_allocator.allocate();
        if (likely(mem != 0))
            return Buffer(mem, _large_buffer_size, large_buffer_deallocator);
        else
            return size > 0 ? Buffer(size) : Buffer(_large_buffer_size);
    }
    else
    {
        return size > 0 ? Buffer(size) : Buffer(_large_buffer_size);
    }
}

int IOBuffer::getSmallBufferSize() { return _small_buffer_size; }
int IOBuffer::getLargeBufferSize() { return _large_buffer_size; }

int IOBuffer::read(int fd, int max)
{
    if (unlikely(_buffer_list.empty()))
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
            delete entry;
            ++i;
        }
        if (_buffer_list.size() > 100)
            fprintf(stderr, "+%d:%d\n", sendv_size, _buffer_list.size());

        return nwrite;
    }
    else
    {
        int nbytes = nwrite;
        BufferList::BufferEntry* entry = _buffer_list.front();
        int have_data;
        while ((have_data = entry->buffer.getAvailableDataSize()) <= nbytes)
        {
            nbytes -= have_data;
            _buffer_list.pop_front();
            delete entry;
            entry = _buffer_list.front();
        }
        if (_buffer_list.size() > 100)
            fprintf(stderr, "-%d:%d:%d(%d:%d)\n", sendv_size,
                    _buffer_list.size(),
                    size,
                    nwrite,
                    will_send_size);

        if (nbytes > 0)
            entry->buffer.consume_unsafe(nbytes);

        return nwrite >= 0 ? nwrite : (errno > 0 ? -errno : errno);
    }
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

int IOBuffer::read(int fd, int max_read, Buffer& buffer)
{
    char* ptr = buffer.producer();
    int nread = ::read(fd, ptr, max_read);
    if (nread > 0)
        buffer.produce_unsafe(nread);
    return nread >= 0 ? nread : (errno > 0 ? -errno : errno);
}

}

