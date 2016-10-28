#include "buffer_stream.h"

#include <arpa/inet.h>

namespace krpc
{

unsigned int BufferStream::readSlow(char* buf, unsigned int len)
{
    unsigned int payload = _rbound - _rbase;
    if (payload > 0)
    {
        ::memcpy(buf, _rbase, payload);
        setReadBuffer(_rbufBase, 0);
        return payload;
    }
    unsigned int read_size = _stream->read(_rbufBase, _rbufSize);
    if (read_size > 0)
    {
        setReadBuffer(_rbufBase, read_size);
        int give = mmin(len, read_size);
        ::memcpy(buf, _rbase, give);
        _rbase += give;
        return give;
    }

    return read_size;
}

void BufferStream::writeSlow(const char* buf, unsigned int len)
{
    unsigned int payload = _wbase - _wbufBase;
    if (payload + len >= 2*(unsigned int)_wbufSize || payload == 0)
    {
        if (payload > 0)
        {
            _stream->write(_wbufBase, payload);
            _wbase = _wbufBase;
        }
        _stream->write(buf, len);
    }

    unsigned int write_len = len;
    unsigned int space = _wbound - _wbase;
    if (space > 0)
    {
        ::memcpy(_wbase, buf, space);
        buf += space;
        write_len -= space;
    }
    _stream->write(_wbufBase, _wbufSize);

    ::memcpy(_wbufBase, buf, write_len);
    _wbase = _wbufBase + write_len;
}

void BufferStream::flush()
{
    unsigned int payload = _wbase - _wbufBase;
    if (likely(payload > 0))
    {
        /* 先修改_wbase，保证内部指针状态的一致性，防止下面write调用抛异常 */
        _wbase = _wbufBase;
        _stream->write(_wbufBase, payload);
    }
    _stream->flush();
}

/***********************************************************/
/***********************************************************/

/***********************************************************/
/***********************************************************/
unsigned int FramedStream::readSlow(char* buf, unsigned int len)
{
    unsigned int payload = _rbound - _rbase;
    if (payload > 0)
    {
        ::memcpy(buf, _rbase, payload);
        setReadBuffer(_rbufBase, 0);
        return payload;
    }

    if (!readFrame())
    {
        /* read EOF */
        return 0;
    }

    unsigned int want = len;
    unsigned int give = mmin(want, (unsigned int)(_rbound-_rbase));
    ::memcpy(buf, _rbase, give);
    _rbase += give;
    want -= give;

    return (len - want);
}

unsigned int FramedStream::readFrame()
{
    int sz = 0;
    unsigned int frame_bytes = 0;
    while (frame_bytes < sizeof(sz))
    {
        char* b = (char*)(&sz) + frame_bytes;
        unsigned int rsz = _stream->read(b, (unsigned int)sizeof(sz)-frame_bytes);
        if (likely(rsz > 0))
        {
            frame_bytes += rsz;
        }
        else
        {
            if (frame_bytes == 0)
                return 0;
            else
                throw StreamException(StreamException::END_OF_FILE,
                        "no more data to read after "
                        "partial frame header.");
        }
    }

    sz = ntohl(sz);
    if (unlikely(sz < 0))
        throw StreamException("frame size has negative value");

    if (sz > _rbufSize)
    {
        char* new_buf = new char[sz];
        delete [] _rbufBase;
        _rbufBase = new_buf;
        _rbufSize = sz;
    }

    _stream->readAll(_rbufBase, sz);
    setReadBuffer(_rbufBase, sz);
    return 1;
}

void FramedStream::writeSlow(const char* buf, unsigned int len)
{
    unsigned int payload = _wbase - _wbufBase;
    unsigned int need_size = len + payload;
    if (unlikely(need_size < payload || need_size > 0x7fffffff))
    {
        throw StreamException(StreamException::BAD_ARGS,
                "attempted to write over 2 GB to FramedStream.");
    }

    unsigned int new_size = _wbufSize;
    while (new_size < need_size) new_size *= 2; 

    char* new_buf = new char[new_size];
    ::memcpy(new_buf, _wbufBase, payload);
    delete [] _wbufBase;
    _wbufSize = new_size;
    _wbufBase = new_buf;

    _wbase = _wbufBase + payload;
    _wbound = _wbufBase + new_size;
    ::memcpy(_wbase, buf, len);
    _wbase += len;
}

void FramedStream::flush()
{
    unsigned int payload = _wbase - _wbufBase;
    int frame_nbo = 0;
    if (likely(payload > sizeof(frame_nbo)))
    {
        int frame_nbo = htonl(payload-sizeof(frame_nbo));
        ::memcpy(_wbufBase, &frame_nbo, sizeof(frame_nbo));

        /* 先修改_wbase，保证内部指针状态的一致性，防止下面write调用抛异常 */
        _wbase = _wbufBase + sizeof(frame_nbo);
        _stream->write(_wbufBase, payload);
    }
    _stream->flush();
}

unsigned int FramedStream::readEnd()
{
    return _rbound - _rbufBase + sizeof(int);
}

unsigned int FramedStream::writeEnd()
{
    return _wbase - _wbufBase;
}

}

