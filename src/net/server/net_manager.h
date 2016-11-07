#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include "ev.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace knet { namespace server {

class WhenReceivePacket;
class NetProcessor;
class Acceptor;
class NetManager
{
    public:
        NetManager(WhenReceivePacket* processor, int idle_timeout = 0):
            _reactor(evnet::ev_init(evnet::EV_REACTOR_EPOLL)),
            _acceptor(0),
            _net_processor(0),
            _request_processor(processor),
            _idle_timeout(idle_timeout)
        {
            if (_reactor == 0)
            {
                fprintf(stderr, "Net driver start error\n");
                abort();
            }
        }

        ~NetManager();

        /*
         * 启动监听
         */
        void startAcceptor(int port);

        /*
         *
         */
        void startNetProcessor();

        void run();

        inline evnet::EvLoop* getReactor() { return _reactor; }
        inline int getIdleTimeout() const { return _idle_timeout; }

    private:
        evnet::EvLoop* _reactor;
        Acceptor* _acceptor;
        NetProcessor* _net_processor;
        WhenReceivePacket* _request_processor;
        // 空闲链接的timeout，不是收发数据的timeout
        // 秒级
        int _idle_timeout;
};

}}

#endif

