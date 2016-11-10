#include "accept_adaptor.h"
#include "net_processor.h"
#include "net_connection.h"

namespace knet { namespace server {

int AcceptAdaptor::start(int code, void* data)
{
    (void)code;
    TcpSocket* const sock = (TcpSocket*)data;
    // TODO  this 又被NetConnection引用
    NetConnection* const conn = new (std::nothrow) NetConnection(sock, this, NetConnection::LISTEN);
    if (conn == 0)
    {
        delete sock;
        return -1;
    }

    int ret = _net_processor->addStaticConnection(conn);
    if (ret != 0)
    {
        delete conn;
        return -1;
    }

    SET_HANDLE(this, &AcceptAdaptor::when);

    return 0;
}

int AcceptAdaptor::when(int code, void* data)
{
    (void)code;
    _net_processor->addDynamicConnection((TcpSocket*)data);
    return 0;
}

}}

