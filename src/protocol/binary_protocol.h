#ifndef __KRPC_BINARY_PROTOCOL_H__
#define __KRPC_BINARY_PROTOCOL_H__

#include "protocol.h"
#include "stream.h"

#include <stdint.h>
#include <stdio.h>

namespace krpc
{

template <typename TStream>
class BinaryProtocolT : public VirtualProtocol<BinaryProtocolT<TStream> >
{
    public:
        explicit BinaryProtocolT(TStream* stream):
            VirtualProtocol<BinaryProtocolT<TStream> >(stream)
        {
        }

        virtual ~BinaryProtocolT() {}

        inline unsigned int readByte(int8_t& byte);
        inline unsigned int readI16(int16_t& i16);
        inline unsigned int readI32(int& i32);
        inline unsigned int readI64(int64_t& i64);
        inline unsigned int readDouble(double& dub);
        inline unsigned int readBytes(Buffer* buf);

        inline unsigned int writeByte(const int8_t byte);
        inline unsigned int writeI16(const int16_t i16);
        inline unsigned int writeI32(const int i32);
        inline unsigned int writeI64(const int64_t i64);
        inline unsigned int writeDouble(const double dub);
        inline unsigned int writeBytes(const Buffer* buf);

    protected:
        inline unsigned int readBytesBody(Buffer* buf, int size);
};

typedef BinaryProtocolT<Stream> BinaryProtocol;

}

#include "binary_protocol.inl"

#endif

