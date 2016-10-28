#include "connection.h"
#include "server.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SendList Connection::empty_list = {0, 0, 0};

Connection::~Connection()
{
    evnet::EvIo::destroy(_io);
    evnet::EvTimer::destroy(_timer);
}

void Connection::read_data_cb(int event, void* data)
{
    Connection* c = (Connection*)data;
    if (event & evnet::EV_ERROR)
    {
        close(c->_fd);
        c->_io->delEvent();
        c->_timer->delTimer();
        c->_owner->removeConn(c);
        //TODO 可以让单独线程负责删除
        delete c;
        return;
    }

    while (true)
    {
        int ret = read(c->_fd,
                c->_recv_buf    + c->_recv_buf_pos,
                c->_recv_buf_sz - c->_recv_buf_pos);
        if (ret > 0)
        {
            c->_recv_buf_pos += ret;
            break;
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                break;
        }

        close(c->_fd);
        c->_io->delEvent();
        c->_timer->delTimer();
        c->_owner->removeConn(c);
        //TODO 可以让单独线程负责删除
        delete c;
        return;
    }

    if (c->_curpack_sz == 0 && c->_recv_buf_pos >= (int)sizeof(int))
    {
        c->_curpack_sz = ntohl(*((int*)(c->_recv_buf))) + sizeof(int);
        assert(c->_curpack_sz > (int)sizeof(int));
        //TODO 超限制如何处理
        fprintf(stderr, "%d-%d\n", c->_curpack_sz, c->_recv_buf_sz);
        assert(c->_curpack_sz <= c->_recv_buf_sz);
    }

    while (c->_curpack_sz > 0 && c->_curpack_sz <= c->_recv_buf_pos)
    {
        //TODO 接收到完整的package
        //TODO 处理package
        fprintf(stderr, "%d\n", c->_curpack_sz);
        c->_recv_buf_pos -= c->_curpack_sz;
        memmove(c->_recv_buf, c->_recv_buf+c->_curpack_sz, c->_recv_buf_pos);

        if (c->_recv_buf_pos >= (int)sizeof(int))
        {
            c->_curpack_sz = ntohl(*((int*)(c->_recv_buf))) + sizeof(int);
            assert(c->_curpack_sz > (int)sizeof(int));
            //TODO 超限制如何处理
            assert(c->_curpack_sz <= c->_recv_buf_sz);
        }
        else
        {
            c->_curpack_sz = 0;
        }
    }

    return;
}

void Connection::write_data_cb(int event, void* data)
{
    Connection* c = (Connection*)data;
    if (event & evnet::EV_ERROR)
    {
        close(c->_fd);
        c->_io->delEvent();
        c->_timer->delTimer();
        c->_owner->removeConn(c);
        //TODO 可以让单独线程负责删除
        delete c;
        return;
    }

    Package* q = c->_sending.head;
    while (true)
    {
        int ret = write(c->_fd, q->data+q->skip, q->size-q->skip);
        if (ret > 0)
        {
            q->skip += ret;
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                break;
            else
            {
                close(c->_fd);
                c->_io->delEvent();
                c->_timer->delTimer();
                c->_owner->removeConn(c);
                //TODO 可以让单独线程负责删除
                delete c;
                return;
            }
        }

        if (q->skip == q->size)
        {
            --c->_sending.total;
            //TODO 出队列
            c->_sending.head = q->next;
            if (q->next == 0)
            {
                c->_sending.tail = 0;
                assert(c->_sending.total == 0);
            }
        }

        break;
    }

    if (c->_sending.total == 0)
    {
        c->_io->modEvFlag(evnet::EV_IO_READ);
    }
}

void Connection::timeout_cb(int event, void* data)
{
    Connection* c = (Connection*)data;
    close(c->_fd);
    c->_io->delEvent();
    c->_timer->delTimer();
    c->_owner->removeConn(c);
    //TODO 可以让单独线程负责删除
    delete c;
}

void Connection::init(int fd,
        int timeout,
        evnet::EvLoop* reactor)
{
    _recv_buf = (char*)malloc(1024);
    _recv_buf_sz = 1024;
    _recv_buf_pos = 0;
    _curpack_sz = 0;

    _fd = fd;
    _timeout = timeout;
    _reactor = reactor;

    _io = evnet::EvIo::create();
    _io->setEvent(_fd, evnet::EV_IO_READ, read_data_cb, this);
    _io->addEvent(_reactor);

    if (timeout > 0)
    {
        _timer = evnet::EvTimer::create();
        _timer->set(timeout, 0, timeout_cb, this);
        _timer->addTimer(_reactor);
    }
}

void Connection::send(char* data, int size)
{
    Package* item = (Package*)malloc(sizeof(Package));
    item->next = 0;
    item->owner = this;
    item->data = data;
    item->size = size; 
    item->skip = 0; 

    _owner->sendPackage(item);
    ev_wakeup(_reactor);
}

