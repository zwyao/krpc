#include "struct.h"

#include <arpa/inet.h>

namespace krpc
{

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readByte(int8_t& byte)
{
    char b;
    this->_stream->readAll(&b, 1);
    byte = (int8_t)b;
    return 1;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readI16(int16_t& i16)
{
    union bytes
    {
        char b[2];
        int16_t all;
    } thisByte;
    this->_stream->readAll(thisByte.b, 2);
    i16 = (int16_t)ntohs(thisByte.all);
    return 2;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readI32(int& i32)
{
    union bytes
    {
        char b[4];
        int   all;
    } thisByte;
    this->_stream->readAll(thisByte.b, 4);
    i32 = (int)ntohl(thisByte.all);
    return 4;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readI64(int64_t& i64)
{
    union bytes
    {
        char b[8];
        int64_t all;
    } thisByte;
    this->_stream->readAll(thisByte.b, 8);
    i64 = (int64_t)ntohll(thisByte.all);
    return 8;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readDouble(double& dub)
{
    //TODO
    return 0;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readBytes(Buffer* buf)
{
    union bytes
    {
        char b[4];
        int all;
    } thisByte;
    this->_stream->readAll(thisByte.b, 4);
    int size = (int)ntohl(thisByte.all);
    return readBytesBody(buf, size)+sizeof(size);
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::readBytesBody(Buffer* buf, int size)
{
    if (unlikely(size < 0))
        throw ProtocolException(ProtocolException::NEGATIVE_SIZE);

    if (size == 0) return 0;

    int space = buf->getAvailableSpaceSize();
    assert(space >= size);

    this->_stream->readAll(buf->producer(), size);
    buf->produce(size);

    return size;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeByte(const int8_t byte)
{
    this->_stream->write((char*)&byte, 1);
    return 1;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeI16(const int16_t i16)
{
    int16_t v = (int16_t)htons(i16);
    this->_stream->write((char*)&v, 2);
    return 2;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeI32(const int i32)
{
    int v = (int)htonl(i32);
    this->_stream->write((char*)&v, 4);
    return 4;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeI64(const int64_t i64)
{
    int64_t v = (int64_t)htonll(i64);
    this->_stream->write((char*)&v, 8);
    return 8;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeDouble(const double dub)
{
    //TODO
    return 0;
}

template <typename TStream>
unsigned int BinaryProtocolT<TStream>::writeBytes(const Buffer* buf)
{
    unsigned int size = buf->size();
    unsigned int result = writeI32(size);
    if (size > 0)
        this->_stream->write(buf->data(), size);

    return result + size;
}

}

