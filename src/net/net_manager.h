#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include "net_connection.h"
#include "net_processor.h"
#include "id_creator.h"
#include "list.h"
#include "ev.h"
#include "defines.h"

#include <string.h>

namespace global
{

extern knet::NetManager* g_net_managers[NET_MANAGER_NUM];

}

namespace knet
{

class NetRequestProcessor;
class Acceptor;
class TcpSocket;
class NetManager
{
    public:
        NetManager(NetRequestProcessor* processor, int idle_timeout = 0):
            _reactor(evnet::ev_init(evnet::EV_REACTOR_EPOLL)),
            _acceptor(0),
            _id(_id_creator.nextID()),
            _idle_timeout(idle_timeout),
            _net_processor(this, processor),
            _mask_generator(0),
            _conn_empty_list(0)
        {
            assert(_id < NET_MANAGER_NUM);
            assert(MAX_CONNECTION_EACH_MANAGER > 0);

            global::g_net_managers[_id] = this;

            init_empty_conn_list();
            init_conn_ids();
        }

        ~NetManager();

        void startAcceptor(int port);
        void run();

        inline int getIdleTimeout() const
        {
            return _idle_timeout;
        }

        // 新连接到来
        //NetConnection will be owned by NetManager
        inline void newConnection(TcpSocket* sock)
        {
            NetConnection* conn = 0;
            if (likely(_conn_empty_list_num > 0))
                conn = _conn_empty_list[--_conn_empty_list_num];
            else
                conn = new NetConnection();

            assert(conn->myID() == -1);

            conn->setSocket(sock);
            conn->setCallbackObj(&_net_processor);

            addConnection(conn);
        }

        inline int addConnection(NetConnection* conn)
        {
            const int id = getConnID();
            if (likely(id >= 0))
            {
                assert(_connections[id] == 0);

                conn->join(this, _reactor, id, _mask_generator.nextID());
                _connections[id] = conn;
                return 0;
            }
            else
            {
                delConnection(conn);
                return -1;
            }
        }

        inline void delConnection(NetConnection* conn)
        {
            // 在close之前获取id
            int id = conn->myID();
            if (likely(id >= 0))
            {
                conn->close();
                assert(conn == _connections[id]);

                _connections[id] = 0;
                // 回收id
                _conn_ids[_conn_id_num++] = id;

                if (_conn_empty_list_num < _conn_empty_list_size)
                    _conn_empty_list[_conn_empty_list_num++] = conn;
                else
                    delete conn;
            }
            else
            {
                conn->close();
                delete conn;
                fprintf(stderr, "ok???????\n");
            }
        }

        inline int myID() const { return _id; }
        inline evnet::EvLoop* getReactor() { return _reactor; }

        inline int send(int conn_id, int mask, int channel_id, util::Buffer& buffer)
        {
            // 包含负值的检查
            assert((unsigned int)conn_id < MAX_CONNECTION_EACH_MANAGER);

            NetConnection* conn = _connections[conn_id];
            if (unlikely(conn == 0 || conn->myMask() != mask)) return -1;

            char* buf = buffer.getBuffer();
            assert(buffer.consumer() - buf == 8);

            *((unsigned int*)buf) = htonl(buffer.getAvailableDataSize());
            *((unsigned int*)(buf+4)) = htonl(channel_id);
            buffer.consume_unsafe(-8);

            return conn->send(buffer);
        }

        void info()
        {
            for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
                if (_connections[i])
                    fprintf(stderr, "+++++++++++++%d\n", _connections[i]->sendCount());
        }

    private:
        void init_empty_conn_list();
        void init_conn_ids();

        inline int getConnID()
        {
            if (likely(_conn_id_num > 0))
                return _conn_ids[--_conn_id_num];
            return -1;
        }

    private:
        evnet::EvLoop* _reactor;
        Acceptor* _acceptor;
        int _id;
        // 空闲链接的timeout，不是收发数据的timeout
        // 秒级
        int _idle_timeout;
        NetProcessor _net_processor;
        util::IDCreatorUnsafe _mask_generator;

        NetConnection** _conn_empty_list;
        int _conn_empty_list_num;
        int _conn_empty_list_size;

        // connection的id,约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
        int _conn_ids[MAX_CONNECTION_EACH_MANAGER];
        int _conn_id_num;

        // connection的id到NetConnection的映射表
        NetConnection* _connections[MAX_CONNECTION_EACH_MANAGER];

    private:
        static util::IDCreator _id_creator;
};

}

#endif

