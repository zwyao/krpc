#include "net_manager.h"
#include "acceptor.h"

namespace global
{

knet::NetManager* g_net_managers[NET_MANAGER_NUM] = {0};

}

namespace knet
{

util::IDCreator NetManager::_id_creator(0);

NetManager::~NetManager()
{
    if (_reactor)
        evnet::ev_destroy(_reactor);

    if (_acceptor)
        delete _acceptor;

    for (int i = 0; i < _conn_empty_list_num; ++i)
        delete _conn_empty_list[i];
    delete _conn_empty_list;
}

void NetManager::startAcceptor(int port)
{
    _acceptor = new Acceptor(this, port);
    if (!_acceptor->isReady())
    {
        fprintf(stderr, "Acceptor start error\n");
        exit(-1);
    }
}

void NetManager::run()
{
    assert(_reactor != 0);

    ev_loop(_reactor);
}

void NetManager::init_empty_conn_list()
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

void NetManager::init_conn_ids()
{
    int id = MAX_CONNECTION_EACH_MANAGER;
    for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
        _conn_ids[i] = --id;
    _conn_id_num = MAX_CONNECTION_EACH_MANAGER;

    memset(_connections, 0, sizeof(_connections));
}

}


