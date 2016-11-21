#include "net_connection.h"

namespace knet { namespace net {

void NetConnection::handleReadEvent()
{
    // 防止即可读又可写的情况,在写事件中关闭了connection
    if (unlikely(!isGood())) return;

    assert(_cb != 0);

    switch (_state)
    {
        case LISTEN:
            recv_connection();
            break;
        case NORMAL:
            recv_data();
            break;
        default:
            assert(0);
            break;
    }

    //TODO
    //update();
}

void NetConnection::handleWriteEvent()
{
    // 防止即可读又可写的情况,在读事件中关闭了connection
    if (unlikely(!isGood())) return; 

    assert(_cb != 0);

    switch (_state)
    {
        case NetConnection::NORMAL:
            send_data();
            break;
        case NetConnection::CONNECTING:
            connecting();
            break;
        default:
            break;
    }
}

void NetConnection::handleTimeoutEvent()
{
}

void NetConnection::recv_connection()
{
    net::TcpSocket* const sock = _sock->accept();
    if (likely(sock != 0))
    {
        _cb->handleEvent(EVENT_NEW_CONNECTION, (void*)sock);
    }
    else
    {
        fprintf(stderr, "accept failure\n");
    }
}

void NetConnection::recv_data()
{
    //状态无关,只管读取数据,尽量多读取
    int nread = _in_buffer.read(_sock->fd());
    if (nread > 0)
    {
        Input input(this, &(_in_buffer.getReadBuffer()));
        _cb->handleEvent(EVENT_NET_READ, (void*)&input);
    }
    else if (nread == 0)
    {
        _error = NetConnection::NET_EOF;
    }
    // EAGAIN, EINTR这类导致的未读取到数据
    else if ((unsigned int)nread != (unsigned int)EAGAIN &&
            (unsigned int)nread != (unsigned int)EINTR)
    {
        _error = NetConnection::NET_ERROR;
    }
}

void NetConnection::send_data()
{
    bool null_loop = false;
    int nwrite = _out_buffer.write(_sock->fd(), null_loop);
    if (nwrite < 0 &&
            (unsigned int)nwrite != (unsigned int)EAGAIN &&
            (unsigned int)nwrite != (unsigned int)EINTR)
    {
        _error = NetConnection::NET_ERROR;
        return;
    }

    if (null_loop)
        _io->modEvFlag(evnet::EV_IO_READ);
}

void NetConnection::connecting()
{
    int err = _sock->getSocketError();
    if (err || _sock->isSelfConnect())
    {
        _error = NetConnection::NET_ERROR;
    }
    else
    {
        _state = NORMAL;
        Input input(this, 0);
        _cb->handleEvent(EVENT_CONNECTED, (void*)&input);
        _io->modEvFlag(evnet::EV_IO_READ);
    }
}

void NetConnection::processor(int event, void* data)
{
    NetConnection* conn = (NetConnection*)data;
    /*
    if (event & evnet::EV_TIMER)
    {
        conn->handleTimeoutEvent();
        return;
    }
    */

    // 先执行写操作，优先发送数据
    if (event & evnet::EV_IO_WRITE)
    {
        conn->handleWriteEvent();
        if (conn->_error != NET_NONE)
        {
            Input input(conn, 0);
            conn->_cb->handleEvent(EVENT_NET_ERROR, (void*)&input);
            return;
        }
    }

    if (event & evnet::EV_IO_READ)
    {
        conn->handleReadEvent();
        if (conn->_error == NetConnection::NET_EOF)
        {
            Input input(conn, 0);
            conn->_cb->handleEvent(EVENT_NET_EOF, (void*)&input);
            return;
        }
        else if (conn->_error == NetConnection::NET_ERROR)
        {
            Input input(conn, 0);
            conn->_cb->handleEvent(EVENT_NET_ERROR, (void*)&input);
            return;
        }
    }
}

}}

