#include "net_connection.h"
#include "net_manager.h"

#include <errno.h>

namespace knet
{

void NetConnection::handleReadEvent()
{
    // 防止即可读又可写的情况,在写事件中关闭了connection
    if (unlikely(!isGood()))
    {
        fprintf(stderr, "read event ignored: fd closed\n");
        return;
    }

    assert(_cb != 0);

    switch (_state)
    {
        case LISTEN:
            recv_connection();
            break;
        case NORMAL:
            recv_data();
            break;
        case SEND_NOTIFY:
            recv_notify();
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

    bool null_loop = false;
    int nwrite = _out_buffer.write(_sock->fd(), null_loop);
    if (nwrite < 0 &&
            (unsigned int)nwrite != (unsigned int)EAGAIN &&
            (unsigned int)nwrite != (unsigned int)EINTR)
    {
        Input input(this, 0);
        _cb->handleEvent(EVENT_NET_ERROR, (void*)&input);
        return;
    }

    //if (_out_buffer.size() == 0)
    if (null_loop)
        _io->modEvFlag(evnet::EV_IO_READ);
}

void NetConnection::handleTimeoutEvent()
{
}

void NetConnection::recv_connection()
{
    TcpSocket* const sock = _sock->accept();
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
        Input input(this, 0);
        _cb->handleEvent(EVENT_NET_EOF, (void*)&input);
    }
    // EAGAIN, EINTR这类导致的未读取到数据
    else if ((unsigned int)nread != (unsigned int)EAGAIN &&
            (unsigned int)nread != (unsigned int)EINTR)
    {
        Input input(this, 0);
        _cb->handleEvent(EVENT_NET_ERROR, (void*)&input);
    }
}

void NetConnection::recv_notify()
{
    _cb->handleEvent(0, 0);
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

    if (event & evnet::EV_IO_READ)
        conn->handleReadEvent();

    if (event & evnet::EV_IO_WRITE)
        conn->handleWriteEvent();
}

}

