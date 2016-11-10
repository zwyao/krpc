#include "net_manager.h"
#include "net_processor.h"
#include "net_connector.h"
#include "acceptor.h"
#include "accept_adaptor.h"

namespace knet { namespace server {

NetManager::NetManager(WhenReceivePacket* processor, int idle_timeout):
    _acceptor(0),
    _net_processor(0),
    _connector(new NetConnector()),
    _request_processor(processor),
    _idle_timeout(idle_timeout)
{
    assert(_connector);
}

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

/*
NetPipe getPipe(const ServerLocation& location)
{
    NetPipe pipe;
    TcpSocket* const sock = _connector->connect(location);
    if (sock)
    {
        int ret = _net_processor->newConnection(sock);
        if (ret == 0)
        {
            NetPipe pipe(_net_processor->myID(),
                    );
        }
    }
}
*/

}}


