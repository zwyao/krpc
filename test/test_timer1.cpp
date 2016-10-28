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

struct TimerArg
{
    EvTimer* e;
    ev_tstamp t;
    ev_tstamp r;
    int idx;
    int64_t c;
};

ev_tstamp repeat1 = 0.01;
ev_tstamp repeat2 = 0.01;

EvTimer** timers  = 0;
int timers_cnt = 0;

void test_timeout(int event, void* data)
{
    TimerArg* arg = (TimerArg*)(data);
    ++arg->c;
    if (arg->c % 1000 == 0)
        fprintf(stderr, "%d: %ld(%f:%f)\n",
                arg->idx,
                arg->c,
                arg->t,
                arg->r);
}

void setTimers(EvLoop* reactor, int count)
{
    timers_cnt = count;
    timers = (EvTimer**)malloc(count * sizeof(EvTimer*));
    int j = 0;
    for (int i = 0; i < count; ++i, ++j)
    {
        TimerArg* arg = (TimerArg*)malloc(sizeof(TimerArg));
        ev_tstamp t = (j+1)*0.01;
        if (t >= 3.0)
        {
            j = 0;
            t = (j+1)*0.01;
        }
        arg->t = t;
        arg->r = t;
        arg->idx = i;
        arg->c = 0;

        timers[i] = EvTimer::create();
        arg->e = timers[i];
        timers[i]->set(arg->t, arg->r, test_timeout, arg);
        timers[i]->addTimer(reactor);
    }
}

void clearTimers()
{
    for (int i = 0; i < timers_cnt; ++i)
    {
        TimerArg* arg = (TimerArg*)(timers[i]->data());
        free(arg);
        EvTimer::destroy(timers[i]);
    }

    timers_cnt = 0;
    free(timers);
    timers= 0;
}

void test_timeout1(int event, void* data)
{
    EvTimer* e = (EvTimer*)data;
    fprintf(stderr, "in timeout 1 %f\n", e->repeat());
    repeat1 += 0.01;
    e->setRepeatTime(0);
    if (!e->isValid())
    {
        fprintf(stderr, "in timeout 1 quit %f\n", e->repeat());
    }
}

void test_timeout2(int event, void* data);
void test_timeout_ok(int event, void* data)
{
    EvTimer* e = (EvTimer*)data;
    fprintf(stderr, "in timeout ok %f\n", e->repeat());
    repeat2 += 0.1;
    if ((int)repeat2 % 2 == 1)
        e->setCb(test_timeout2);

    e->setRepeatTime(repeat2);
}

void test_timeout2(int event, void* data)
{
    EvTimer* e = (EvTimer*)data;
    fprintf(stderr, "in timeout 2 %f\n", e->repeat());
    repeat2 += 0.1;
    if ((int)repeat2 % 2 == 0)
        e->setCb(test_timeout_ok);

    e->setRepeatTime(repeat2);
}

void test_read(int event, void* data)
{
    IoArg* arg = (IoArg*)data;
    EvIo* e = arg->e;
    char buf[1024];
    int fd = e->fd();
    int rcnt = read(fd, buf, 1024);
    fprintf(stderr, "ok? %d\n", rcnt);
    if (rcnt == 0)
    {
        e->delEvent();
        ev_break(arg->loop);
    }
}

int main(int argc, char** argv)
{
    EvLoop* reactor = ev_init(EV_REACTOR_EPOLL);

    struct IoArg* arg = new IoArg;
    arg->loop = reactor;
    EvIo* ev = EvIo::create();
    arg->e = ev;
    ev->setEvent(0, EV_IO_READ, test_read, arg);
    ev->addEvent(reactor);

    EvTimer* t1 = EvTimer::create();
    t1->set(1, repeat1, test_timeout1, t1);
    t1->addTimer(reactor);

    EvTimer* t2 = EvTimer::create();
    t2->set(1, repeat2, test_timeout2, t2);
    t2->addTimer(reactor);

    setTimers(reactor, 100000);

    ev_loop(reactor);
    ev_destroy(reactor);

    delete arg;
    EvIo::destroy(ev);
    EvTimer::destroy(t1);
    EvTimer::destroy(t2);
    clearTimers();
}
