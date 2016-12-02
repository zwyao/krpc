#include "io_read_buffer.h"
#include "buffer.h"

#include <sys/uio.h>
#include <errno.h>

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

int IOReadBuffer::read(int fd)
{
    if (unlikely(_buffer.capacity() == 0))
    {
        util::Buffer buffer(_io_buffer_size);
        _buffer = buffer;
    }
    int max = _buffer.getAvailableSpaceSize();
    return read_data(fd, max, _buffer);
}

}}

