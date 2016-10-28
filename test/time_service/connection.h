#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "ev.h"
#include "ev_io.h"
#include "ev_timer.h"

#include "message.h"

class TimeServer;
class Connection
{
    public:
        explicit Connection(TimeServer* s):
            next(0),
            pre(0),
            _owner(s),
            _sending(empty_list),
            _fd(-1),
            _reactor(0),
            _io(0),
            _timer(0)
        {
        }

        ~Connection();

        void init(int fd, int timeout, evnet::EvLoop* reactor);
        void send(char* data, int size);

    private:
        static void read_data_cb(int event, void* data);
        static void write_data_cb(int event, void* data);
        static void timeout_cb(int event, void* data);

    public:
        Connection* next;
        Connection* pre;

    private:
        TimeServer* _owner;
        SendList _sending;

        int _fd;
        evnet::ev_tstamp _timeout;

        evnet::EvLoop* _reactor;
        evnet::EvIo* _io;
        evnet::EvTimer* _timer;

        char* _recv_buf;
        int _recv_buf_sz;
        int _recv_buf_pos;
        int _curpack_sz;

    private:
        static SendList empty_list;

        friend class TimeServer;
};

#endif
