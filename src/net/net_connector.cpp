#include "net_connector.h"

namespace knet { namespace net {

TcpSocket* NetConnector::connect(const ServerLocation& location)
{
    TcpSocket* const sock = new (std::nothrow) TcpSocket();
    int ret = sock->connect(location, true);
    if (ret != -1)
    {
        return sock;
    }
    else
    {
        delete sock;
        return 0;
    }
}

}}

