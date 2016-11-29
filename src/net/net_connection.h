#ifndef __NET_CONNECTION_H__
#define __NET_CONNECTION_H__

#include "net_event.h"
#include "tcp_socket.h"
#include "callback_object.h"
#include "io_buffer.h"
#include "write_buffer_allocator.h"
#include "list.h"
#include "ev.h"
#include "ev_io.h"

namespace knet { namespace net {

/*
 * connection 完全由 NetProcessor 管理
 */
class NetConnection
{
    public:
        enum State
        {
            LISTEN = 1,
            NORMAL,
            NOTIFIER,
            CONNECTING,
        };

    public:
        NetConnection():
            _reactor(0),
            _io(evnet::EvIo::create()),
            _sock(0),
            _cb(0),
            _id(-1),
            _mask(-1),
            _state(NetConnection::NORMAL),
            _error(NetConnection::NET_NONE)
        {
            assert(_io != 0);
        }

        NetConnection(TcpSocket* sock,
                net::CallbackObj* cb,
                NetConnection::State state):
            _reactor(0),
            _io(evnet::EvIo::create()),
            _sock(sock),
            _cb(cb),
            _id(-1),
            _mask(-1),
            _state(state),
            _error(NetConnection::NET_NONE)
        {
            assert(sock && cb && _io);
        }

        ~NetConnection()
        {
            NetConnection::close();
            if (_io)
            {
                evnet::EvIo::destroy(_io);
                _io = 0;
            }
        }

        void handleReadEvent();
        void handleWriteEvent();
        void handleErrorEvent();
        void handleTimeoutEvent();

        inline void init(net::TcpSocket* sock,
                net::CallbackObj* cb,
                int id,
                int64_t mask,
                NetConnection::State state)
        {
            _sock = sock;
            _cb   = cb;
            _id   = id;
            _mask = mask;
            _state = state;
            _error = NetConnection::NET_NONE;
        }

        // 加入事件循环
        inline void join(evnet::EvLoop* reactor, int event)
        {
            _reactor = reactor;
            _io->setEvent(_sock->fd(), event, processor, this);
            _io->addEvent(_reactor, evnet::EV_HIGH_PRI);

            if (_state == NetConnection::NORMAL)
            {
                Input input(this, 0);
                _cb->handleEvent(EVENT_NEW_CONNECTION, (void*)&input);
            }
            else if (_state == NetConnection::CONNECTING)
            {
                Input input(this, 0);
                _cb->handleEvent(EVENT_CONNECTING, (void*)&input);
            }
        }

        inline void close()
        {
            if (_sock == 0) return;
            if (_reactor && _io)
            {
                _io->delEvent();
                _reactor = 0;
            }
            //防止竞态条件: os可能重用正在关闭的fd
            TcpSocket* const sock = _sock;
            _sock = 0;

            sock->close();
            delete sock;

            _id = -1;
            _mask = -1;

            _in_buffer.release();
            _out_buffer.release();
        }

        inline bool isGood() const { return (_sock && _sock->isGood()); }
        inline bool isListen() const { return _state == NetConnection::LISTEN; }
        inline int myID() const { return _id; }
        inline int64_t myMask() const { return _mask; }

        inline void setSocket(net::TcpSocket* sock) { _sock = sock; }
        inline void setCallbackObj(net::CallbackObj* cb) { _cb = cb; }
        inline void setID(int id) { _id = id; }
        inline void setMask(int64_t mask) { _mask = mask; }

        inline int send(util::Buffer& buffer, util::WriteBufferAllocator& allocator)
        {
            _out_buffer.append(buffer, allocator);
            if ((_io->events() & evnet::EV_IO_WRITE) == 0)
                _io->modEvFlag(evnet::EV_IO_READ|evnet::EV_IO_WRITE);
            return 0;
        }

        /*
         * 把buffer添加到发送队列
         * @param[in] buffer 的内存会被夺走
         */
        inline int send(util::BufferList::BufferEntry* entry, util::WriteBufferAllocator& allocator)
        {
            /*
            int ret = _out_buffer.write(_sock->fd(), entry);
            if (ret >= 0)
            {
                if (_out_buffer.size() == 1)
                    _io->modEvFlag(evnet::EV_IO_READ|evnet::EV_IO_WRITE);
                return 0;
            }
            else
            {
                Input input(this, 0);
                _cb->handleEvent(EVENT_NET_ERROR, (void*)&input);
                return -1;
            }
            */
            _out_buffer.append(entry, allocator);
            if ((_io->events() & evnet::EV_IO_WRITE) == 0)
                _io->modEvFlag(evnet::EV_IO_READ|evnet::EV_IO_WRITE);
            return 0;
        }

    public:
        struct Input
        {
            NetConnection* the_conn;
            util::Buffer* the_buffer;

            Input(NetConnection* conn, util::Buffer* buffer):
                the_conn(conn),
                the_buffer(buffer)
            {
            }
        };

    private:
        void recv_connection();
        void recv_data();
        void notified();
        void send_data();
        void connecting();

    private:
        static void processor(int event, void* data);

    private:
        enum Error
        {
            NET_NONE = 1,
            NET_ERROR,
            NET_EOF,
        };

    private:
        evnet::EvLoop* _reactor;
        evnet::EvIo* _io;

        //own _sock
        TcpSocket* _sock;
        CallbackObj* _cb;
        int _id;  // 约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
        int64_t _mask;

        NetConnection::State _state;
        NetConnection::Error _error;

        // read buffer,初始大小global::g_read_io_buffer_init,按需增长
        util::IOBuffer _in_buffer;
        // write buffer
        util::IOBuffer _out_buffer;

    public:
        util::ListOp::ListNode list_node;
};

}}

#endif

