#ifndef __NET_PIPE_H__
#define __NET_PIPE_H__

#include "buffer.h"
#include "net_processor.h"

namespace knet { namespace net {

class NetPipe
{
    public:
        NetPipe():
            _processor_id(-1),
            _conn_id(-1),
            _mask(-1),
            _req_id(0)
        { }

        NetPipe(int processor_id,
                int conn_id,
                int64_t mask,
                unsigned int req_id):
            _processor_id(processor_id),
            _conn_id(conn_id),
            _mask(mask),
            _req_id(req_id)
        { }

        NetPipe(const NetPipe& other):
            _processor_id(other._processor_id),
            _conn_id(other._conn_id),
            _mask(other._mask),
            _req_id(other._req_id)
        { }

        NetPipe& operator=(const NetPipe& other)
        {
            _processor_id = other._processor_id;
            _conn_id      = other._conn_id;
            _mask         = other._mask;
            _req_id   = other._req_id;
            return *this;
        }

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
            int ret = processor->send(_conn_id, _mask, _req_id, pack);
            return ret >= 0 ? 0 : -1;
        }

        // for test
        int sendForceAsyn(util::Buffer& pack)
        {
            NetProcessor* processor = detail::g_net_processors[_processor_id];
            assert(processor != 0 && processor->myID() == _processor_id);
            int ret = processor->sendAsyn(_conn_id, _mask, _req_id, pack);
            return ret >= 0 ? 0 : -1;
        }

    private:
        int _processor_id;
        int _conn_id;
        int64_t _mask;
        unsigned int _req_id;
};

}}

#endif

