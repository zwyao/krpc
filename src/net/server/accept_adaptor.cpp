#include "accept_adaptor.h"
#include "net_processor.h"
#include "net_connection.h"

namespace knet { namespace server {

int AcceptAdaptor::start(int code, void* data)
{
    (void)code;
    SET_HANDLE(this, &AcceptAdaptor::when);

    // TODO  this 又被NetConnection引用
    _net_processor->addConnection(
            new NetConnection((TcpSocket*)data, this, NetConnection::LISTEN));
    return 0;
}

int AcceptAdaptor::when(int code, void* data)
{
    (void)code;
    _net_processor->newConnection((TcpSocket*)data);
    return 0;
}

}}

