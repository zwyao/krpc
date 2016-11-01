#include "notifier.h"
#include "net_processor.h"
#include "net_connection.h"

#include <sys/eventfd.h>
#include <stdlib.h>

namespace knet
{

Notifier::Notifier(NetProcessor* processor):
    _net_processor(processor)
{
    SET_HANDLE(this, &Notifier::notified);
    _fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (_fd < 0)
    {
        fprintf(stderr, "Failed create eventfd\n");
        abort();
    }
    this->start();
}

Notifier::~Notifier()
{
}

void Notifier::start()
{
    TcpSocket* const sock = new TcpSocket(_fd);
    if (sock)
    {
        _net_processor->addConnection(new NetConnection(sock, this, NetConnection::SEND_NOTIFY));
    }
    else
    {
        delete sock;
        fprintf(stderr, "Unable to start notify socket\n");
        abort();
    }
}

int Notifier::notified(int code, void* data)
{
    return 0;
}

}

