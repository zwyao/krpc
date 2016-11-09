#include "net_manager.h"
#include "net_processor.h"
#include "acceptor.h"
#include "accept_adaptor.h"

namespace knet { namespace server {

NetManager::~NetManager()
{
    if (_net_processor)
        delete _net_processor;

    if (_acceptor)
        delete _acceptor;
}

void NetManager::startNetProcessor()
{
    _net_processor = new NetProcessor(_request_processor, _idle_timeout);
    if (_net_processor == 0)
    {
        fprintf(stderr, "NetProcessor start error\n");
        exit(-1);
    }
}

void NetManager::startAcceptor(int port)
{
    if (_net_processor == 0)
        startNetProcessor();

    _acceptor = new Acceptor(new AcceptAdaptor(_net_processor), port);
    if (_acceptor == 0)
    {
        fprintf(stderr, "Acceptor start error\n");
        exit(-1);
    }
    if (_acceptor->listen() != 0)
    {
        fprintf(stderr, "Acceptor start error\n");
        exit(-1);
    }
}

void NetManager::run()
{
    _net_processor->run();
}

}}


