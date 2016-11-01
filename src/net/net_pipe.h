#ifndef __NET_PIPE_H__
#define __NET_PIPE_H__

#include "buffer.h"

namespace knet
{

class NetPipe
{
    public:
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
         * 线程不安全接口
         * 在io线程的事件循环中,获取到一个pack之后,可以调用这个不安全接口
         * @param[in] pack, 发送成功pack的内存会被夺走
         * @return 0: 成功加入发送队列
         *        -1: 失败
         */
        int sendUnsafe(util::Buffer& pack);

        /*
         * 线程安全接口
         * 异步发送数据
         * @param[in] pack, 发送成功pack的内存会被夺走
         * @return 0: 成功加入发送队列
         *        -1: 失败
         */
        int send(util::Buffer& pack);

    private:
        int _processor_id;
        int _conn_id;
        int _mask;
        unsigned int _channel_id;
};

}

#endif

