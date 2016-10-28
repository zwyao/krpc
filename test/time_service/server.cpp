#include "server.h"
#include "connection.h"

#include "ev.h"
#include "ev_io.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

SendList empty_list = {0, 0, 0};

namespace
{

inline int set_nonblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;
    return 0;
}

inline void add_queue(ConnQueue* q, Connection* c)
{
    c->next = q->next;
    c->pre  = 0;
    if (q->next)
        q->next->pre = c;
    q->next = c;
    ++q->total;
}

inline void del_queue(ConnQueue* q, Connection* c)
{
    if (c->pre)
        c->pre->next = c->next;
    if (c->next)
        c->next->pre = c->pre;

    c->next = 0;
    c->pre  = 0;

    --q->total;
}

int create_listen_socket(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        fprintf(stderr, "socket() error\n");
        return -1;
    }

    set_nonblocking(fd);

    int flags = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*)&sin, sizeof(sin)) != 0)
    {
        fprintf(stderr, "bind() error\n");
        return -2;
    }

    if (listen(fd, 128) != 0)
    {
        fprintf(stderr, "listen() error\n");
        return -3;
    }

    return fd;
}

}

void TimeServer::accept_cb(int event, void* data)
{
    TimeServer* s = (TimeServer*)data;
    int fd = -1;
    while (true)
    {
        struct sockaddr_in sock;
        socklen_t sock_len = sizeof(sock);
        fd = accept(s->_listen_fd, (struct sockaddr*)&sock, &sock_len);
        if (fd == -1)
        {
            if (errno == EINTR)
                continue;
            else 
                return;
        }

        break;
    }

    set_nonblocking(fd);

    int flags = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));

    struct linger ling = {0, 0};
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));

    Connection* conn = new Connection(s);
    conn->init(fd, 0, s->_reactor);

    //TODO获得conn
    add_queue(&(s->_conn_queue), conn);

    ++s->_total_cnt;
    ++s->_current_cnt;

    fprintf(stderr, "%ld\n", s->_current_cnt);

    return;
}

void TimeServer::wakeup(void* data)
{
    TimeServer* s = (TimeServer*)data;
    Package* item = 0;
    pthread_mutex_lock(&s->_list_lock);
    item = s->_send_list.head;
    s->_send_list = empty_list;
    pthread_mutex_unlock(&s->_list_lock);

    while (item)
    {
        SendList* list = &(item->owner->_sending);
        if (list->tail)
            list->tail->next = item;
        else
            list->head = item;
        list->tail = item;
        ++list->total;
        item = item->next;
        list->tail->next = 0;

        item->owner->_io->modEvFlag(evnet::EV_IO_RDWR);
    }
}

void TimeServer::removeConn(Connection* conn)
{
    del_queue(&_conn_queue, conn);
}

void TimeServer::sendPackage(Package* pack)
{
    pthread_mutex_lock(&_list_lock);
    if (_send_list.tail)
        _send_list.tail->next = pack;
    else
        _send_list.head = pack;
    _send_list.tail = pack;
    ++_send_list.total;
    pthread_mutex_unlock(&_list_lock);
}

int TimeServer::run(int port)
{
    int fd = -1;
    if ((fd = create_listen_socket(port)) < 0)
        return -1;

    _listen_fd = fd;
    _reactor = evnet::ev_init(evnet::EV_REACTOR_EPOLL);
    _listen_event = evnet::EvIo::create();
    _listen_event->setEvent(fd, evnet::EV_IO_READ, TimeServer::accept_cb, this);
    _listen_event->addEvent(_reactor);

    ev_set_wakeup(_reactor, wakeup, this);

    _conn_queue.next  = 0;
    _conn_queue.total = 0;

    evnet::ev_loop(_reactor);
    evnet::ev_destroy(_reactor);
    evnet::EvIo::destroy(_listen_event);
    close(_listen_fd);

    return 0;
}

TimeServer::TimeServer():
    _listen_fd(-1),
    _total_cnt(0),
    _current_cnt(0),
    _send_list(empty_list),
    _reactor(0),
    _listen_event(0)
{
    pthread_mutex_init(&_list_lock, 0);
}

TimeServer::~TimeServer()
{
    close(_listen_fd);
    evnet::EvIo::destroy(_listen_event);
    ev_destroy(_reactor);
    pthread_mutex_destroy(&_list_lock);
}

