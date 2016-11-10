#include "acceptor.h"
#include "tcp_socket.h"
#include "net_event.h"
#include "defines.h"

#include <stdio.h>

namespace knet
{

int Acceptor::listen()
{
    if (_port < 0 || _cb == 0)
    {
        fprintf(stderr, "Acceptor need port/callback\n");
        return -1;
    }

    TcpSocket* const sock = new TcpSocket();
    int ret = sock->listen(_port);
    if (ret < 0)
    {
        fprintf(stderr, "Unable to bind to port: %d\n", _port);
        delete sock;
        return -1;
    }

    ret = _cb->handleEvent(knet::EVENT_LISTEN, (void*)sock);
    return ret == 0 ? 0 : -1;
}

}

