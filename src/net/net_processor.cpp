#include "net_processor.h"
#include "net_manager.h"
#include "net_request_processor.h"
#include "net_pipe.h"
#include "net_event.h"

#include <stdio.h>
#include <stdlib.h>

namespace detail 
{

util::IDCreator g_processor_id_creator(0);
knet::NetProcessor* g_net_processors[NET_MANAGER_NUM] = {0};

}

namespace knet
{

NetProcessor::NetProcessor(NetManager* net_manager, NetRequestProcessor* processor):
    _reactor(net_manager->getReactor()),
    _processor(processor),
    _timer_queue(),
    _timer(0),
    _mask_generator(0),
    _id(detail::g_processor_id_creator.nextID()),
    _frame_limit(util::IOBuffer::getSmallBufferSize() - 8),
    _conn_empty_list(0)
{
    SET_HANDLE(this, &NetProcessor::process);

    if (_id >= NET_MANAGER_NUM)
    {
        fprintf(stderr, "Too many net processor\n");
        abort();
    }

    detail::g_net_processors[_id] = this;
    fprintf(stderr, "net processor %d start\n", _id);

    assert(_processor != 0);
    assert(MAX_CONNECTION_EACH_MANAGER > 0);

    init_empty_conn_list();
    init_conn_ids();

    for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
        _session_set[i]._conn_id = i;

    if (net_manager->getIdleTimeout() > 0)
    {
        _timer = evnet::EvTimer::create();
        _timer_queue.init(this, net_manager->getIdleTimeout()*2);
        assert(_timer != 0);

        _timer->set(0.5, 0.5, NetProcessor::on_timer, this);
        _timer->addTimer(_reactor);
    }
}

NetProcessor::~NetProcessor()
{
    if (_timer)
        evnet::EvTimer::destroy(_timer);

    for (int i = 0; i < _conn_empty_list_num; ++i)
        delete _conn_empty_list[i];
    delete _conn_empty_list;
}

int NetProcessor::process(int code, void* data)
{
    NetConnection::Input* input = (NetConnection::Input*)data;
    int id = input->the_conn->myID();
    assert((unsigned int)id < MAX_CONNECTION_EACH_MANAGER);

    NetProcessor::Session& session = _session_set[id];
    switch (code)
    {
        case EVENT_NEW_CONNECTION:
            assert(session._state == NetProcessor::Session::IDLE);
            assert(session._mask == -1);

            session._state = NetProcessor::Session::FRAME_HEAD;
            session._mask = input->the_conn->myMask();
            if (_timer) _timer_queue.add(input->the_conn);
            break;

        case EVENT_NET_READ:
            assert(session._mask == input->the_conn->myMask());

            if (_timer) _timer_queue.update(input->the_conn);
            if (process_read(session, *(input->the_buffer)) != 0)
            {
                session.close();
                if (_timer) _timer_queue.remove(input->the_conn);
                delConnection(input->the_conn);
                return -1;
            }
            break;

        case EVENT_NET_EOF:
            assert(session._mask == input->the_conn->myMask());

            session.close();
            if (_timer) _timer_queue.remove(input->the_conn);
            delConnection(input->the_conn);
            break;

        case EVENT_NET_ERROR:
            assert(session._mask == input->the_conn->myMask());

            session.close();
            if (_timer) _timer_queue.remove(input->the_conn);
            delConnection(input->the_conn);
            break;

        default:
            assert(0);
            break;
    }

    return 0;
}

int NetProcessor::process_read(NetProcessor::Session& session, util::Buffer& buffer)
{
    int pack_num = 0;
    while (check_data(session, buffer))
        ++pack_num;

    if (session._state == NetProcessor::Session::INVALID)
        return -1;

    if (pack_num > 0)
        buffer.compact();

    return 0;
}

int NetProcessor::check_data(NetProcessor::Session& session, util::Buffer& buffer)
{
    switch (session._state)
    {
        /*
        case NetProcessor::Session::IDLE:
        {
            assert(session._mask == -1);

            session._state = NetProcessor::Session::FRAME_HEAD;
            session._mask = mask;
        }
        */
        case NetProcessor::Session::FRAME_HEAD:
        {
            if (buffer.getAvailableDataSize() < 8)
                return 0;

            char* frame = buffer.consumer();
            session._frame_size = ntohl(*((unsigned int*)frame));
            session._channel_id = ntohl(*((unsigned int*)(frame+4)));
            // 跳过8字节
            buffer.consume(8);

            // 包含负值的检查
            if (unlikely((unsigned int)session._frame_size > (unsigned int)_frame_limit))
            {
                fprintf(stderr, "frame size: %d too large, channel id: %u<%d>\n",
                        session._frame_size,
                        session._channel_id,
                        buffer.getAvailableDataSize());

                session._state = NetProcessor::Session::INVALID;
                return 0;
            }

            session._state = NetProcessor::Session::DATA;
        }
        case NetProcessor::Session::DATA:
        {
            if (buffer.getAvailableDataSize() >= session._frame_size)
            {
                util::Buffer pack = util::IOBuffer::getSmallBuffer(session._frame_size);

                memcpy(pack.producer(), buffer.consumer(), session._frame_size);

                pack.produce_unsafe(session._frame_size);
                buffer.consume_unsafe(session._frame_size);

                NetPipe pipe(_id,
                        session._conn_id,
                        session._mask,
                        session._channel_id);
                _processor->process(pipe, pack);

                session._state = NetProcessor::Session::FRAME_HEAD;
                return 1;
            }

            break;
        }
        default:
            break;
    }

    return 0;
}

void NetProcessor::on_timer(int event, void* data)
{
    NetProcessor* processor = (NetProcessor*)data;
    if (event & evnet::EV_TIMER)
        processor->_timer_queue.check();
}

void NetProcessor::TimeWheel::check()
{
    _current_idx = (_current_idx+1) % _size;
    _timedout_idx = (_timedout_idx+1) % _size;

    fprintf(stderr, "%d:%d:%d\n", _current_idx, _timedout_idx, _size);
    TimeWheelList& list = _wheel[_timedout_idx];
    while (list.empty() == false)
    {
        NetConnection* conn = list.front();
        fprintf(stderr, "timeout: %d:%d\n", conn->myID(), conn->myMask());
        list.pop_front();
        _net_processor->close_session(conn);
        _net_processor->delConnection(conn);
    }
}

void NetProcessor::init_empty_conn_list()
{
    int num = MAX_CONNECTION_EACH_MANAGER / 100;
    if (num == 0)
        num = MAX_CONNECTION_EACH_MANAGER - 1;

    if (num > 0)
    {
        _conn_empty_list = new NetConnection*[num];
        for (int i = 0; i < num; ++i)
            _conn_empty_list[i] = new NetConnection();
    }

    _conn_empty_list_num = num;
    _conn_empty_list_size = num;
}

void NetProcessor::init_conn_ids()
{
    int id = MAX_CONNECTION_EACH_MANAGER;
    for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
        _conn_ids[i] = --id;
    _conn_id_num = MAX_CONNECTION_EACH_MANAGER;

    memset(_connections, 0, sizeof(_connections));
}

}
