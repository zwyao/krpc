#ifndef __NET_PROTOCOL_H__
#define __NET_PROTOCOL_H__

namespace krpc
{

class NetProtocol
{
    public:
        NetProtocol() { }
        virtual ~NetProtocol() { }

        virtual int encode(Packet* pack, Buffer& buffer);
        virtual Packet* decode(Buffer& buffer);
};

}

#endif

