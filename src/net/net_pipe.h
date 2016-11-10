#ifndef __NET_PIPE_H__
#define __NET_PIPE_H__

#include "buffer.h"
#include "net_processor.h"

namespace knet
{

class NetPipe
{
    public:
        NetPipe():
            _processor_id(-1),
            _conn_id(-1),
            _mask(-1),
            _channel_id(-1)
        {
        }

        NetPipe(int processor_id,
                int conn_id,
                int mask,
                unsigned int channel_id):
            _processor_id(processor_id),
            _conn_id(conn_id),
            _mask(mask),
            _channel_id(channel_id)
        { }

        ~NetPipe() { }

        /*
         * 线程安全接口
         * 异步发送数据
         * @param[in] pack, 发送成功pack的内存会被夺走
         * @return 0: 成功加入发送队列
         *        -1: 失败
         */
        int send(util::Buffer& pack)
        {
            NetProcessor* processor = detail::g_net_processors[_processor_id];
            assert(processor != 0 && processor->myID() == _processor_id);
            int ret = processor->send(_conn_id, _mask, _channel_id, pack);
            return ret >= 0 ? 0 : -1;
        }

        int sendForceAsyn(util::Buffer& pack)
        {
            NetProcessor* processor = detail::g_net_processors[_processor_id];
            assert(processor != 0 && processor->myID() == _processor_id);
            int ret = processor->sendAsyn(_conn_id, _mask, _channel_id, pack);
            return ret >= 0 ? 0 : -1;
        }

    private:
        int _processor_id;
        int _conn_id;
        int _mask;
        unsigned int _channel_id;
};

}

#endif

