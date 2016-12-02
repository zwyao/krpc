#ifndef __IO_READ_BUFFER_H__
#define __IO_READ_BUFFER_H__

#include "buffer_list.h"

namespace knet { namespace util {

class IOReadBuffer
{
    public:
        IOReadBuffer():
            _io_buffer_size(65536)
        {
        }

        explicit IOReadBuffer(int buffer_size):
            _io_buffer_size(buffer_size<65536 ? 65536 : buffer_size)
        {
        }

        int read(int fd);

        inline void setBufferSize(int buffer_size)
        {
            _io_buffer_size = buffer_size<65536 ? 65536 : buffer_size;
        }

        inline Buffer& getReadBuffer() { return _buffer; }
        inline void release() { _buffer.release(); }

        /*
        inline bool empty() const;
        inline int size() const;
        */

    private:
        int _io_buffer_size;
        Buffer _buffer;
};

}}

#endif

