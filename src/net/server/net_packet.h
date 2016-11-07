#ifndef __NET_PACKET_H__
#define __NET_PACKET_H__

#include "io_buffer.h"

namespace knet
{

class NetPacket
{
    public:
        NetPacket() { }

        NetPacket(Buffer& buffer):
            _buffer(buffer)
        { }

        ~NetPacket() { }

        inline void setBuffer(Buffer& buffer) { _buffer = buffer; }
        inline Buffer& buffer() { return _buffer; }

    private:
        struct NetPacketHead
        {
            int frame_size;
            int channel_id;
        };

    private:
        NetPacketHead _head;
        Buffer _buffer;
};

}

#endif

