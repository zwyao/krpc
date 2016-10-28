#ifndef __KRPC_PROTOCOL_H__
#define __KRPC_PROTOCOL_H__

#include "buffer.h"
#include "protocol_exception.h"

namespace krpc
{

#define ntohll(n) ( (((uint64_t)ntohl(n)) << 32) + ntohl(n >> 32) )
#define htonll(n) ( (((uint64_t)htonl(n)) << 32) + htonl(n >> 32) )

class Stream;
/* TODO: 未来支持自定义结构协议 */
/* 目前只有一个空类             */
class Protocol
{
    public:
        virtual ~Protocol() {}

        virtual unsigned int readByte_virt(int8_t& byte) = 0;
        virtual unsigned int readI16_virt(int16_t& i16) = 0;
        virtual unsigned int readI32_virt(int& i32) = 0;
        virtual unsigned int readI64_virt(int64_t& i64) = 0;
        virtual unsigned int readDouble_virt(double& dub) = 0;
        virtual unsigned int readBytes_virt(Buffer* buf) = 0;

        virtual unsigned int writeByte_virt(const int8_t byte) = 0;
        virtual unsigned int writeI16_virt(const int16_t i16) = 0;
        virtual unsigned int writeI32_virt(const int i32) = 0;
        virtual unsigned int writeI64_virt(const int64_t i64) = 0;
        virtual unsigned int writeDouble_virt(const double dub) = 0;
        virtual unsigned int writeBytes_virt(const Buffer* buf) = 0;

        unsigned int readByte(int8_t& byte)
        {
            return readByte_virt(byte);
        }

        unsigned int readI16(int16_t& i16)
        {
            return readI16_virt(i16);
        }

        unsigned int readI32(int& i32)
        {
            return readI32_virt(i32);
        }

        unsigned int readI64(int64_t& i64)
        {
            return readI64_virt(i64);
        }

        unsigned int readDouble(double& dub)
        {
            return readDouble_virt(dub);
        }

        unsigned int readBytes(Buffer* buf)
        {
            return readBytes_virt(buf);
        }

        unsigned int writeByte(const int8_t byte)
        {
            return writeByte_virt(byte);
        }

        unsigned int writeI16(const int16_t i16)
        {
            return writeI16_virt(i16);
        }

        unsigned int writeI32(const int i32)
        {
            return writeI32_virt(i32);
        }

        unsigned int writeI64(const int64_t i64)
        {
            return writeI64_virt(i64);
        }

        unsigned int writeDouble(const double dub)
        {
            return writeDouble_virt(dub);
        }

        unsigned int writeBytes(const Buffer* buf)
        {
            return writeBytes_virt(buf);
        }

        Stream* getStream() { return _stream; }

    protected:
        Protocol(Stream* stream):
            _stream(stream)
        {
        }

    protected:
        Stream* _stream;
};

class DefaultProtocol : public Protocol
{
    public:
        virtual ~DefaultProtocol() {}

        unsigned int readByte(int8_t& byte)
        {
            (void)byte;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support reading(yet).");
        }

        unsigned int readI16(int16_t& i16)
        {
            (void)i16;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support reading(yet).");
        }

        unsigned int readI32(int& i32)
        {
            (void)i32;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support reading(yet).");
        }

        unsigned int readI64(int64_t& i64)
        {
            (void)i64;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support reading(yet).");
        }

        unsigned int readDouble(double& dub)
        {
            (void)dub;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support reading(yet).");
        }

        unsigned int readBytes(Buffer* buf)
        {
            (void)buf;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeByte(const int8_t byte)
        {
            (void)byte;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeI16(const int16_t i16)
        {
            (void)i16;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeI32(const int i32)
        {
            (void)i32;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeI64(const int64_t i64)
        {
            (void)i64;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeDouble(const double dub)
        {
            (void)dub;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

        unsigned int writeBytes(const Buffer* buf)
        {
            (void)buf;
            throw ProtocolException(ProtocolException::NOT_IMPLEMENTED,
                    "this protocol does not support writing (yet).");
        }

    protected:
        DefaultProtocol(Stream* stream):
            Protocol(stream)
        {
        }
};

template <typename Self, typename Super = DefaultProtocol>
class VirtualProtocol : public Super
{
    public:
        virtual ~VirtualProtocol() {}

        virtual unsigned int readByte_virt(int8_t& byte)
        {
            return static_cast<Self*>(this)->readByte(byte);
        }

        virtual unsigned int readI16_virt(int16_t& i16)
        {
            return static_cast<Self*>(this)->readI16(i16);
        }

        virtual unsigned int readI32_virt(int& i32)
        {
            return static_cast<Self*>(this)->readI32(i32);
        }

        virtual unsigned int readI64_virt(int64_t& i64)
        {
            return static_cast<Self*>(this)->readI64(i64);
        }

        virtual unsigned int readDouble_virt(double& dub)
        {
            return static_cast<Self*>(this)->readDouble(dub);
        }

        virtual unsigned int readBytes_virt(Buffer* buf)
        {
            return static_cast<Self*>(this)->readBytes(buf);
        }

        virtual unsigned int writeByte_virt(const int8_t byte)
        {
            return static_cast<Self*>(this)->writeByte(byte);
        }

        virtual unsigned int writeI16_virt(const int16_t i16)
        {
            return static_cast<Self*>(this)->writeI16(i16);
        }

        virtual unsigned int writeI32_virt(const int i32)
        {
            return static_cast<Self*>(this)->writeI32(i32);
        }

        virtual unsigned int writeI64_virt(const int64_t i64)
        {
            return static_cast<Self*>(this)->writeI64(i64);
        }

        virtual unsigned int writeDouble_virt(const double dub)
        {
            return static_cast<Self*>(this)->writeDouble(dub);
        }

        virtual unsigned int writeBytes_virt(const Buffer* buf)
        {
            return static_cast<Self*>(this)->writeBytes(buf);
        }

    protected:
        VirtualProtocol(Stream* stream):
            Super(stream)
        {
        }
};

}

#endif

