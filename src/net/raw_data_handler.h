#ifndef __RAW_DATA_HANDLER_H__
#define __RAW_DATA_HANDLER_H__

#include "net_pipe.h"
#include "buffer.h"

namespace knet { namespace net {

// 无状态类
class RawDataHandler
{
    public:
        RawDataHandler() { }
        virtual ~RawDataHandler() { }

        // 请把数据拿走
        virtual int handle(NetPipe& pipe, util::Buffer& pack) = 0;
};

}}

#endif

