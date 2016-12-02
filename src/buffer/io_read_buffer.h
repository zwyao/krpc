#ifndef __IO_READ_BUFFER_H__
#define __IO_READ_BUFFER_H__

#include "buffer.h"

namespace knet { namespace util {

/*
 * 读取数据，存放在Buffer中
 * buffer大小初始值设置为_io_buffer_size
 * 每次读操作，都会额外读取65536字节的数据，buffer会按需增长
 * 参考read函数
 */
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

