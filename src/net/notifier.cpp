#include "notifier.h"
#include "net_processor.h"
#include "net_connection.h"

#include <sys/eventfd.h>
#include <stdlib.h>

namespace knet { namespace net {

Notifier::Notifier(NetProcessor* processor):
    _net_processor(processor)
{
    SET_HANDLE(this, &Notifier::notified);
    this->start();
}

Notifier::~Notifier()
{
}

void Notifier::start()
{
    _fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (_fd < 0)
    {
        fprintf(stderr, "Failed create eventfd\n");
        abort();
    }

    TcpSocket* const sock = new TcpSocket(_fd);
    if (sock)
    {
        _net_processor->addStaticConnection(new NetConnection(sock, this, NetConnection::NOTIFIER));
    }
    else
    {
        delete sock;
        fprintf(stderr, "Unable to start notify socket\n");
        abort();
    }
    fprintf(stderr, "notifier start ok\n");
}

int Notifier::notified(int code, void* data)
{
    (void)code;
    (void)data;
    uint64_t one = 0;
    ::read(_fd, &one, sizeof(one));
    return 0;
}

}}

