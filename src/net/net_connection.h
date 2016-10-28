#ifndef __NET_CONNECTION_H__
#define __NET_CONNECTION_H__

#include "net_event.h"
#include "tcp_socket.h"
#include "io_buffer.h"
#include "list.h"
#include "id_creator.h"
#include "callback_object.h"
#include "ev.h"
#include "ev_io.h"

namespace knet
{

class CallbackObj;
class NetManager;
class NetConnection
{
    public:
        enum State
        {
            LISTEN = 1,
            NORMAL,
            SEND_NOTIFY,
        };

    public:
        NetConnection():
            _net_manager(0),
            _reactor(0),
            _io(evnet::EvIo::create()),
            _sock(0),
            _cb(0),
            _state(NetConnection::NORMAL),
            _id(-1),
            _mask(-1),
            _net_code(EVENT_NONE)
        {
        }

        NetConnection(TcpSocket* sock,
                CallbackObj* cb,
                NetConnection::State state):
            _net_manager(0),
            _reactor(0),
            _io(evnet::EvIo::create()),
            _sock(sock),
            _cb(cb),
            _state(state),
            _id(-1),
            _mask(-1),
            _net_code(EVENT_NONE)
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

        // 加入事件循环
        inline void join(NetManager* manager,
                evnet::EvLoop* reactor,
                const int id,
                const int mask)
        {
            _net_manager = manager;
            _reactor = reactor;
            _id = id;
            _mask = mask;

            assert(_io != 0);
            _io->setEvent(_sock->fd(), evnet::EV_IO_READ, processor, this);
            _io->addEvent(_reactor, evnet::EV_HIGH_PRI);

            // 新连接到来
            if (likely(_state == NetConnection::NORMAL))
            {
                Input input(this, 0);
                _cb->handleEvent(EVENT_NEW_CONNECTION, (void*)&input);
            }
        }

        inline void close()
        {
            if (_sock == 0) return;
            if (_io) _io->delEvent();

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
        inline int myMask() const { return _mask; }

        inline void setNetCode(EventCode code) { _net_code = code; }
        inline void setSocket(TcpSocket* sock) { _sock = sock; }
        inline void setCallbackObj(CallbackObj* cb) { _cb = cb; }

        /*
         * 把buffer添加到发送队列
         * @param[in] buffer 的内存会被夺走
         */
        inline int send(util::Buffer& buffer)
        {
            // 至此，buffer被夺走
            _out_buffer.append(buffer);
            if (_out_buffer.size() == 1)
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

        int sendCount()
        {
            return _out_buffer.size();
        }

    private:
        void recv_connection();
        void recv_data();

    private:
        static void processor(int event, void* data);

    private:
        NetManager* _net_manager;
        evnet::EvLoop* _reactor;
        evnet::EvIo* _io;

        //own _sock
        TcpSocket* _sock;
        CallbackObj* _cb;
        NetConnection::State _state;
        int _id;  // 约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
        int _mask;
        int _net_code;

        util::IOBuffer _in_buffer;
        util::IOBuffer _out_buffer;

    public:
        util::ListOp::ListNode list_node;
};

}

#endif

