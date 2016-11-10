#include "net_processor.h"
#include "net_manager.h"
#include "when_receive_packet.h"
#include "net_event.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>

namespace knet { namespace server {

namespace detail 
{

util::IDCreator g_processor_id_creator(0);
knet::server::NetProcessor* g_net_processors[NET_MANAGER_NUM] = {0};

}

NetProcessor::NetProcessor(WhenReceivePacket* processor, int idle_timeout):
    _reactor(evnet::ev_init(evnet::EV_REACTOR_EPOLL)),
    _idle_timer(0),
    _processor(processor),
    _idle_queue(),
    _mask_generator(-1),
    _conn_id_gen(),
    _thread_id(util::CurrentThread::getTid()),
    _id(detail::g_processor_id_creator.nextID()),
    _frame_limit(global::g_read_io_buffer_limit - 8),
    _conn_empty_list(0)
{
    if (_reactor == 0)
    {
        fprintf(stderr, "Net driver start error\n");
        abort();
    }

    assert(_processor != 0);
    assert(MAX_CONNECTION_EACH_MANAGER > 0);

    if (_id >= NET_MANAGER_NUM)
    {
        fprintf(stderr, "Too many net processor\n");
        abort();
    }
    detail::g_net_processors[_id] = this;

    init_conn_list();
    init_session();

    setup_timer(idle_timeout);
    evnet::ev_set_wakeup(_reactor, do_pending, this);

    SET_HANDLE(this, &NetProcessor::process);

    fprintf(stderr, "net processor %d(%d) start\n", _id, _thread_id);
}

NetProcessor::~NetProcessor()
{
    if (_reactor)
        evnet::ev_destroy(_reactor);

    if (_idle_timer)
        evnet::EvTimer::destroy(_idle_timer);

    for (int i = 0; i < _conn_empty_list_num; ++i)
        delete _conn_empty_list[i];
    delete _conn_empty_list;
}

void NetProcessor::run()
{
    ev_loop(_reactor);
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
            if (_idle_timer) _idle_queue.add(input->the_conn);
            break;

        case EVENT_NET_READ:
            assert(session._mask == input->the_conn->myMask());

            if (_idle_timer) _idle_queue.update(input->the_conn);
            if (process_read(session, *(input->the_buffer)) != 0)
            {
                session.close();
                if (_idle_timer) _idle_queue.remove(input->the_conn);
                delConnection(input->the_conn);
                return -1;
            }
            break;

        case EVENT_NET_EOF:
            assert(session._mask == input->the_conn->myMask());

            session.close();
            if (_idle_timer) _idle_queue.remove(input->the_conn);
            delConnection(input->the_conn);
            break;

        case EVENT_NET_ERROR:
            assert(session._mask == input->the_conn->myMask());

            session.close();
            if (_idle_timer) _idle_queue.remove(input->the_conn);
            delConnection(input->the_conn);
            break;

        case EVENT_CONNECTING:
            assert(session._state == NetProcessor::Session::IDLE);
            assert(session._mask == -1);

            session._state = NetProcessor::Session::CONNECTING;
            session._mask = input->the_conn->myMask();
            break;

        case EVENT_CONNECTED:
            assert(session._state == NetProcessor::Session::CONNECTING);
            assert(session._mask == input->the_conn->myMask());

            session._state = NetProcessor::Session::FRAME_HEAD;
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
                fprintf(stderr, "frame size: %d too large(limit %d), channel id: %u<%d>\n",
                        session._frame_size,
                        _frame_limit,
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

void NetProcessor::send_pending_data()
{
    util::BufferList::TList pending_data;
    {
        util::Guard<PendingLocker> m(_pending_locker);
        pending_data.swap(_pending_data_list);
    }

    while (!pending_data.empty())
    {
        util::BufferList::BufferEntry* entry = pending_data.front();
        pending_data.pop_front();

        int conn_id = entry->target.conn_id;
        int mask = entry->target.mask;

        // 包含负值的检查
        assert((unsigned int)conn_id < MAX_CONNECTION_EACH_MANAGER);

        NetConnection* conn = _connections[conn_id];
        if (likely(conn != 0 && conn->myMask() == mask))
        {
            conn->send(entry);
        }
        else
        {
            fprintf(stderr, "Net processor mask error?????????????????:%d:%d(%d:%d)\n",
                    conn->myID(),
                    conn->myMask(),
                    conn_id,
                    mask);
            delete entry;
            continue;
        }
    }
}

void NetProcessor::process_pending_connection()
{
    PendingConntionList pending_conn;
    {
        util::Guard<PendingLocker> m(_pending_locker);
        pending_conn.swap(_pending_conn_list);
    }

    while (!pending_conn.empty())
    {
        NetConnection* conn = pending_conn.front();
        pending_conn.pop_front();
        attachConnection(conn, evnet::EV_IO_WRITE);
    }
}

void NetProcessor::do_pending(void* data)
{
    NetProcessor* processor = (NetProcessor*)data;
    processor->send_pending_data();
    processor->process_pending_connection();
}

void NetProcessor::on_idle_timer(int event, void* data)
{
    NetProcessor* processor = (NetProcessor*)data;
    if (event & evnet::EV_TIMER)
        processor->_idle_queue.check();
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

void NetProcessor::init_conn_list()
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

    memset(_connections, 0, sizeof(_connections));
}

void NetProcessor::init_session()
{
    for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
        _session_set[i]._conn_id = i;
}

void NetProcessor::setup_timer(int timeout)
{
    if (timeout > 0)
    {
        _idle_timer = evnet::EvTimer::create();
        _idle_queue.init(this, timeout*2);
        if (_idle_timer == 0)
        {
            fprintf(stderr, "Create timer error\n");
            abort();
        }

        // TODO 可以根据timeout大小确定检测周期，而不是固定的0.5秒
        _idle_timer->set(0.5, 0.5, NetProcessor::on_idle_timer, this);
        _idle_timer->addTimer(_reactor);
    }
}

}}

