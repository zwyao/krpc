#ifndef __NET_PROCESSOR_H__
#define __NET_PROCESSOR_H__

#include "net_connection.h"
#include "callback_object.h"
#include "buffer.h"
#include "defines.h"
#include "ev_timer.h"

#include <string.h>

namespace knet
{

class NetManager;
class NetRequestProcessor;
// 网络请求处理的入口
class NetProcessor : public CallbackObj
{
    private:
        class Session
        {
            private:
                enum State
                {
                    IDLE,
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

            private:
                State _state;
                int _conn_id;  // 约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
                int _mask;
                int _frame_size;
                unsigned int _channel_id;

                friend class NetProcessor;
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

    public:
        NetProcessor(NetManager* net_manager, NetRequestProcessor* processor);
        virtual ~NetProcessor();

        int process(int code, void* data);

    private:
        int process_read(NetProcessor::Session& session, util::Buffer& buffer);
        int check_data(NetProcessor::Session& session, util::Buffer& buffer);
        void close_session(NetConnection* conn)
        {
            int id = conn->myID();
            assert((unsigned int)id < MAX_CONNECTION_EACH_MANAGER);
            NetProcessor::Session& session = _session_set[id];
            assert(session._mask == conn->myMask());
            session.close();
        }

    private:
        static void on_timer(int event, void* data);

    private:
        NetManager* _net_manager;
        NetRequestProcessor* _processor;
        TimeWheel _timer_queue;
        evnet::EvTimer* _timer;

        int _frame_limit;
        NetProcessor::Session _session_set[MAX_CONNECTION_EACH_MANAGER];

        friend class TimeWheel;
};

}

#endif

