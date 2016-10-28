#ifndef __SERVER_H__
#define __SERVER_H__

#include "ev.h"
#include "ev_io.h"

#include "message.h"

#include <pthread.h>
#include <stdint.h>

class Connection;
struct ConnQueue
{
    Connection* next;
    int total;
};

struct Package;
class TimeServer
{
    public:
        TimeServer();
        ~TimeServer();

        int run(int port);
        void removeConn(Connection* conn);
        void sendPackage(Package* pack);

    private:
        static void accept_cb(int event, void* data);
        static void wakeup(void* data);

    private:
        int _listen_fd;
        int64_t _total_cnt;
        int64_t _current_cnt;

        ConnQueue _conn_queue;
        SendList _send_list;
        pthread_mutex_t _list_lock;

        evnet::EvLoop* _reactor;
        evnet::EvIo* _listen_event;
};

#endif

