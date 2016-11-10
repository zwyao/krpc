#ifndef __WHEN_RECEIVE_PACKET_H__
#define __WHEN_RECEIVE_PACKET_H__

#include "net_pipe.h"
#include "buffer.h"

namespace knet
{

// 无状态类
class WhenReceivePacket
{
    public:
        WhenReceivePacket() { }
        virtual ~WhenReceivePacket() { }

        // 请把数据拿走
        virtual int process(NetPipe& pipe, util::Buffer& pack) = 0;
};

}

#endif

