#include "net_processor.h"
#include "net_manager.h"
#include "net_event.h"
#include "net_config.h"
#include "raw_data_handler.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>

static int g_count = 0;

namespace knet { namespace net {

namespace detail 
{

knet::util::IDCreator g_processor_id_creator;
knet::net::NetProcessor* g_net_processors[NET_MANAGER_NUM] = {0};

}

NetProcessor::NetProcessor(NetConfig* config, RawDataHandler* handler):
    _reactor(evnet::ev_init(evnet::EV_REACTOR_EPOLL)),
    _idle_timer(0),
    _data_handler(handler),
    _config(config),
    _conn_id_gen(),
    _notifier(this),
    _idle_queue(),
    _mask_generator(),
    _write_buffer_allocator(_config->g_io_write_buffer_init),
    _thread_id(knet::util::CurrentThread::getTid()),
    _id(detail::g_processor_id_creator.nextID()),
    _frame_limit(64*1024*1024),
    _conn_empty_list(0)
{
    if (_reactor == 0)
    {
        fprintf(stderr, "Net driver start error\n");
        abort();
    }

    assert(_data_handler != 0);
    assert(MAX_CONNECTION_EACH_MANAGER > 0);

    if (_id >= NET_MANAGER_NUM)
    {
        fprintf(stderr, "Too many net processor\n");
        abort();
    }
    detail::g_net_processors[_id] = this;

    init_conn_list();
    init_session();

    setup_timer(_config->g_conn_idle_timeout);
    evnet::ev_set_loopend(_reactor, do_pending, this);

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

int NetProcessor::process_read(NetProcessor::Session& session, knet::util::Buffer& buffer)
{
    int pack_num = 0;
    int data_size = 0;
    while ((data_size = check_data(session, buffer)) > 0)
        ++pack_num;

    if (session._state == NetProcessor::Session::INVALID)
        return -1;

    //if (pack_num > 0 && buffer.getAvailableSpaceSize() < data_total_size)
    if (pack_num > 0)
        buffer.compact();

    return 0;
}

int NetProcessor::check_data(NetProcessor::Session& session, knet::util::Buffer& buffer)
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
            session._req_id = ntohl(*((unsigned int*)(frame+4)));
            // 跳过8字节
            buffer.consume_unsafe(8);

            // 包含负值的检查
            if (unlikely((unsigned int)session._frame_size > (unsigned int)_frame_limit))
            {
                fprintf(stderr, "frame size: %d too large(limit %d), request id: %u<%d>\n",
                        session._frame_size,
                        _frame_limit,
                        session._req_id,
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
                knet::util::Buffer pack = knet::global::getBuffer(session._frame_size);

                memcpy(pack.producer(), buffer.consumer(), session._frame_size);

                pack.produce_unsafe(session._frame_size);
                buffer.consume_unsafe(session._frame_size);

                NetPipe pipe(_id,
                        session._conn_id,
                        session._mask,
                        session._req_id);
                _data_handler->handle(pipe, pack);

                session._state = NetProcessor::Session::FRAME_HEAD;
                return session._frame_size+8;
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
    knet::util::BufferList::TList pending_data;
    {
        knet::util::Guard<PendingDataLocker> m(_pending_data_locker);
        pending_data.swap(_pending_data_list);
    }

    int count = 0;
    while (!pending_data.empty())
    {
        knet::util::BufferList::BufferEntry* entry = pending_data.front();
        pending_data.pop_front();
        ++count;

        int conn_id = entry->target.conn_id;
        int64_t mask = entry->target.mask;

        // 包含负值的检查
        assert((unsigned int)conn_id < MAX_CONNECTION_EACH_MANAGER);

        NetConnection* conn = _conn_id_map.get(conn_id);
        if (likely(conn != 0 && conn->myMask() == mask))
        {
            conn->send(entry, _write_buffer_allocator);
        }
        else
        {
            delete entry;
        }
    }
    g_count = count;
}

void NetProcessor::process_pending_connection()
{
    PendingConnectingList pending_conn;
    {
        knet::util::Guard<PendingConnectingLocker> m(_pending_conn_locker);
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
    fprintf(stderr, "++++++++++++++++++ last queue length %d\n", g_count);
    //_net_processor->_write_buffer_allocator.printInfo();
    TimeWheelList& list = _wheel[_timedout_idx];
    while (list.empty() == false)
    {
        NetConnection* conn = list.front();
        fprintf(stderr, "timeout: %d:%ld\n", conn->myID(), conn->myMask());
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

