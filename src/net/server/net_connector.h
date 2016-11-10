#ifndef __NET_CONNECTOR_H__
#define __NET_CONNECTOR_H__

#include "tcp_socket.h"

namespace knet { namespace server {

class NetConnector
{
    public:
        NetConnector() { }
        ~NetConnector() { }

        TcpSocket* connect(const ServerLocation& location);
};

}}

#endif

