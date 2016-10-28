#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "callback_object.h"

namespace knet
{

class NetManager;
class Acceptor : public CallbackObj
{
    public:
        Acceptor(NetManager* manager, int port);

        virtual ~Acceptor();

        int receiveConnection(int code, void* data);

        bool isReady() const { return (_ready == 1); }

    private:
        void listen();

    private:
        NetManager* const _net_manager;
        int _port;
        int _ready; // 1: ready, 0:not ready
};

}

#endif

