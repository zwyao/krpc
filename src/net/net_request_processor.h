#ifndef __NET_REQUEST_PROCESSOR_H__
#define __NET_REQUEST_PROCESSOR_H__

#include "net_pipe.h"
#include "buffer.h"

namespace knet
{

// 无状态类
class NetRequestProcessor
{
    public:
        NetRequestProcessor() { }
        virtual ~NetRequestProcessor() { }

        // 请把数据拿走
        virtual int process(NetPipe& pipe, util::Buffer& pack) = 0;
};

}

#endif

