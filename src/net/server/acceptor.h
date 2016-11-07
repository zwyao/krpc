#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "callback_object.h"

namespace knet { namespace server {

class NetProcessor;
class Acceptor : public CallbackObj
{
    public:
        Acceptor(NetProcessor* processor, int port);

        virtual ~Acceptor();

        int receiveConnection(int code, void* data);

        bool isReady() const { return (_ready == 1); }

    private:
        void listen();

    private:
        NetProcessor* const _net_processor;
        int _port;
        int _ready; // 1: ready, 0:not ready
};

}}

#endif

