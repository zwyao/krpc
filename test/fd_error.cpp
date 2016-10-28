#include "ev.h"
#include "ev_io.h"
#include "ev_timer.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

using namespace evnet;

struct IoArg
{
    EvLoop* loop;
    EvIo* e;
    char buf[1024];
};

void wakeup(void* data)
{
    fprintf(stderr, "wakeup\n");
}

void test_read(int event, void* data)
{
    IoArg* arg = (IoArg*)data;

    EvIo* e = arg->e;
    int fd = e->fd();

    int rcnt = read(fd, arg->buf, 1024);
    fprintf(stderr, "ok? %d\n", rcnt);
    if (rcnt == 0)
    {
        e->delEvent();
        ev_break(arg->loop);
    }

    if (rcnt == 1)
        ev_wakeup(arg->loop);
}

void test_read_none(int event, void* data)
{
    EvIo* e = (EvIo*)data;
    int fd = e->fd();

    if (event & EV_ERROR)
        fprintf(stderr, "error? %d\n", fd);

    fprintf(stderr, "ok? just call %d\n", fd);
}

int main(int argc, char** argv)
{
    EvLoop* reactor = ev_init(EV_REACTOR_EPOLL);
    ev_set_wakeup(reactor, wakeup, 0);

    struct IoArg* arg = new IoArg;
    arg->loop = reactor;

    EvIo* ev = EvIo::create();
    arg->e = ev;
    ev->setEvent(0, EV_IO_READ, test_read, arg);
    ev->addEvent(reactor);

    EvIo* ev1 = EvIo::create();
    ev1->setEvent(10, EV_IO_READ, test_read_none, ev1);
    ev1->addEvent(reactor);

    ev_loop(reactor);
    ev_destroy(reactor);

    EvIo::destroy(ev);
    EvIo::destroy(ev1);
}
