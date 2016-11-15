#ifndef __NET_PROCESSOR_H__
#define __NET_PROCESSOR_H__

#include "net_connection.h"
#include "id_creator.h"
#include "conn_id_generator.h"
#include "callback_object.h"
#include "defines.h"
#include "current_thread.h"
#include "locker.h"
#include "write_buffer_allocator.h"
#include "ev_timer.h"
#include "ev.h"

#include <string.h>

namespace knet
{

class WhenReceivePacket;
class NetManager;
// 网络请求处理的入口
class NetProcessor : public CallbackObj
{
    private:
        class Session
        {
            public:
                enum State
                {
                    IDLE,
                    CONNECTING,
                    FRAME_HEAD,
                    DATA,
                    INVALID,
                };

            public:
                Session():
                    _state(IDLE),
                    _conn_id(-1),
                    _mask(-1),
                    _frame_size(0),
                    _channel_id(-1)
                {
                }

                ~Session() { }

                void close()
                {
                    _state = IDLE;
                    _mask = -1;
                }

            public:
                State _state;
                int _conn_id;  // 约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
                int _mask;
                int _frame_size;
                unsigned int _channel_id;
        };

        class TimeWheel
        {
            private:
                typedef util::List<NetConnection, &NetConnection::list_node> TimeWheelList;

            public:
                TimeWheel() { }
                ~TimeWheel() { } 

                void init(NetProcessor* net_processor, int size)
                {
                    _net_processor = net_processor;
                    _size = size+1;
                    _current_idx = 0;
                    _timedout_idx = 1;

                    assert(_size > _timedout_idx);
                    _wheel = new TimeWheelList[_size];
                    assert(_wheel != 0);
                }

                void add(NetConnection* conn)
                {
                    _wheel[_current_idx].push_back(conn);
                }

                void remove(NetConnection* conn)
                {
                    util::ListOp::remove(&(conn->list_node));
                }

                void update(NetConnection* conn)
                {
                    util::ListOp::remove(&(conn->list_node));
                    _wheel[_current_idx].push_back(conn);
                }

                void check();

            private:
                NetProcessor* _net_processor;
                TimeWheelList* _wheel;
                int _current_idx;
                int _timedout_idx;
                int _size;
        };

    private:
        typedef util::SpinLocker PendingDataLocker;
        typedef util::SpinLocker PendingConnectingLocker;

    public:
        NetProcessor(WhenReceivePacket* processor,
                int write_buffer_size,
                int idle_timeout);
        virtual ~NetProcessor();

        void run();

        int process(int code, void* data);

        // 主动发起链接
        inline int newConnection(TcpSocket* sock)
        {
            NetConnection* const conn =
                new (std::nothrow) NetConnection(sock, this, NetConnection::CONNECTING);
            if (conn == 0)
            {
                delete sock;
                return -1;
            }

            const int id = _conn_id_gen.get();
            if (likely(id >= 0))
            {
                conn->setID(id);
                conn->setMask(_mask_generator.nextID());

                {
                    util::Guard<PendingConnectingLocker> m(_pending_conn_locker);
                    _pending_conn_list.push_back(conn);
                }
                //evnet::ev_wakeup(_reactor);

                return 0;
            }
            else
            {
                delete conn;
                return -1;
            }
        }

        // thread not safe, but in loop
        inline void addDynamicConnection(TcpSocket* sock)
        {
            NetConnection* const conn = acquireConnection();
            if (unlikely(conn == 0))
            {
                delete sock;
                return;
            }

            assert(conn->myID() == -1);
            assert(conn->myMask() == -1);

            const int id = _conn_id_gen.get();
            if (likely(id >= 0))
            {
                conn->init(sock,
                        this,
                        id,
                        _mask_generator.nextID(),
                        NetConnection::NORMAL);
                attachConnection(conn, evnet::EV_IO_READ);
            }
            else
            {
                releaseConnection(conn);
                delete sock;
            }
        }

        // 外部传进来的conn
        // thread not safe, but in loop
        inline int addStaticConnection(NetConnection* conn, int event = evnet::EV_IO_READ)
        {
            const int id = _conn_id_gen.get();
            if (likely(id >= 0))
            {
                conn->setID(id);
                conn->setMask(_mask_generator.nextID());
                attachConnection(conn, event);
                return 0;
            }
            return -1;
        }

        // thread not safe, but in loop
        inline void delConnection(NetConnection* conn)
        {
            // id >= 0,说明是调用的add接口
            // 在close之前获取id
            const int id = conn->myID();
            if (likely(id >= 0))
            {
                detachConnection(conn);
                _conn_id_gen.put(id);
                releaseConnection(conn);
            }
            else
            {
                conn->close();
                delete conn;
                fprintf(stderr, "ok???????\n");
            }
        }

        inline int send(int conn_id, int mask, int channel_id, util::Buffer& buffer)
        {
            if (_thread_id == util::CurrentThread::getTid())
                return send_by_me(conn_id, mask, channel_id, buffer);
            else
                return send_by_queue(conn_id, mask, channel_id, buffer);
        }

        // for test
        inline int sendAsyn(int conn_id, int mask, int channel_id, util::Buffer& buffer)
        {
            return send_by_queue(conn_id, mask, channel_id, buffer);
        }

        inline int myID() const { return _id; }

    private:
        void init_conn_list();
        void init_session();

        int process_read(NetProcessor::Session& session, util::Buffer& buffer);
        int check_data(NetProcessor::Session& session, util::Buffer& buffer);
        void send_pending_data();
        void process_pending_connection();
        void setup_timer(int timeout);

        inline NetConnection* acquireConnection()
        {
            NetConnection* conn = 0;
            if (likely(_conn_empty_list_num > 0))
                conn = _conn_empty_list[--_conn_empty_list_num];
            else
                conn = new (std::nothrow) NetConnection();
            return conn;
        }

        inline void releaseConnection(NetConnection* conn)
        {
            if (_conn_empty_list_num < _conn_empty_list_size)
                _conn_empty_list[_conn_empty_list_num++] = conn;
            else
                delete conn;
        }

        inline void attachConnection(NetConnection* conn, int event)
        {
            int id = conn->myID();
            assert(_connections[id] == 0);
            _connections[id] = conn;
            conn->join(_reactor, event);
        }

        inline void detachConnection(NetConnection* conn)
        {
            int id = conn->myID();
            assert(conn == _connections[id]);
            _connections[id] = 0;
            conn->close();
        }

        inline void close_session(NetConnection* conn)
        {
            int id = conn->myID();
            assert((unsigned int)id < MAX_CONNECTION_EACH_MANAGER);
            NetProcessor::Session& session = _session_set[id];
            assert(session._mask == conn->myMask());
            session.close();
        }

        inline int send_by_me(int conn_id, int mask, int channel_id, util::Buffer& buffer)
        {
            // 包含负值的检查
            assert((unsigned int)conn_id < MAX_CONNECTION_EACH_MANAGER);

            NetConnection* conn = _connections[conn_id];
            if (unlikely(conn == 0 || conn->myMask() != mask)) return -1;

            char* ptr = buffer.getBuffer();
            assert(buffer.consumer() - ptr == 8);

            *((unsigned int*)ptr) = htonl(buffer.getAvailableDataSize());
            *((unsigned int*)(ptr+4)) = htonl(channel_id);
            buffer.consume_unsafe(-8);

            // 至此，buffer被夺走
            return conn->send(buffer, _write_buffer_allocator);
        }

        inline int send_by_queue(int conn_id, int mask, int channel_id, util::Buffer& buffer)
        {
            char* ptr = buffer.getBuffer();
            assert(buffer.consumer() - ptr == 8);

            *((unsigned int*)ptr) = htonl(buffer.getAvailableDataSize());
            *((unsigned int*)(ptr+4)) = htonl(channel_id);
            buffer.consume_unsafe(-8);

            // 至此，buffer被夺走
            util::BufferList::BufferEntry* entry =
                new (std::nothrow) util::BufferList::BufferEntry(buffer, conn_id, mask);
            if (likely(entry != 0))
            {
                util::Guard<PendingDataLocker> m(_pending_data_locker);
                _pending_data_list.push_back(entry);
                return 0;
            }

            return -1;
        }

    private:
        static void on_idle_timer(int event, void* data);
        static void do_pending(void* data);

    private:
        evnet::EvLoop* const _reactor;
        evnet::EvTimer* _idle_timer;
        WhenReceivePacket* const _processor;
        TimeWheel _idle_queue;
        util::IDCreator _mask_generator;
        ConnIdGenerator _conn_id_gen;
        util::WriteBufferAllocator _write_buffer_allocator;

        const pid_t _thread_id;
        const int _id;
        const int _frame_limit;

        NetConnection** _conn_empty_list;
        int _conn_empty_list_num;
        int _conn_empty_list_size;

        // connection的id到NetConnection的映射表
        NetConnection* _connections[MAX_CONNECTION_EACH_MANAGER];
        NetProcessor::Session _session_set[MAX_CONNECTION_EACH_MANAGER];

        PendingDataLocker _pending_data_locker;
        util::BufferList::TList _pending_data_list;

        typedef util::List<NetConnection, &NetConnection::list_node> PendingConnectingList;
        PendingConnectingLocker _pending_conn_locker;
        PendingConnectingList _pending_conn_list;

        friend class TimeWheel;
};

namespace detail
{

extern util::IDCreator g_processor_id_creator;
extern knet::NetProcessor* g_net_processors[NET_MANAGER_NUM];

}

}

#endif

