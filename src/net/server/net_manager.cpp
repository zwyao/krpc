#include "net_manager.h"
#include "net_processor.h"
#include "acceptor.h"

namespace knet { namespace server {

NetManager::~NetManager()
{
    if (_reactor)
        evnet::ev_destroy(_reactor);

    if (_net_processor)
        delete _net_processor;

    if (_acceptor)
        delete _acceptor;
}

void NetManager::startNetProcessor()
{
    _net_processor = new NetProcessor(this, _request_processor);
    if (_net_processor == 0)
    {
        fprintf(stderr, "NetProcessor start error\n");
        exit(-1);
    }
    _net_processor->init();
}

void NetManager::startAcceptor(int port)
{
    if (_net_processor == 0)
        startNetProcessor();

    _acceptor = new Acceptor(_net_processor, port);
    if (_acceptor == 0)
    {
        fprintf(stderr, "Acceptor start error\n");
        exit(-1);
    }

    if (!_acceptor->isReady())
    {
        fprintf(stderr, "Acceptor start error\n");
        exit(-1);
    }
}

void NetManager::run()
{
    ev_loop(_reactor);
}

}}


