#include "acceptor.h"
#include "tcp_socket.h"
#include "net_processor.h"
#include "net_connection.h"
#include "defines.h"

#include <assert.h>
#include <stdlib.h>

namespace knet { namespace server {

Acceptor::Acceptor(NetProcessor* processor, int port):
    _net_processor(processor),
    _port(port),
    _ready(0)
{
    assert(_net_processor != 0);

    SET_HANDLE(this, &Acceptor::receiveConnection);
    this->listen();
}

Acceptor::~Acceptor()
{
}

void Acceptor::listen()
{
    if (_ready == 1) return;

    TcpSocket* const sock = new TcpSocket();
    int ret = sock->listen(_port);
    if (ret < 0)
    {
        fprintf(stderr, "Unable to bind to port: %d\n", _port);
        delete sock;
        return;
    }

    _net_processor->addConnection(new NetConnection(sock, this, NetConnection::LISTEN));
    _ready = 1;
}

int Acceptor::receiveConnection(int code, void* data)
{
    switch (code)
    {
        case EVENT_NEW_CONNECTION:
            _net_processor->newConnection((TcpSocket*)data);
            break;
        case EVENT_NET_EOF:
            break;
        default:
            assert(0);
    }

    return 0;
}

}}
