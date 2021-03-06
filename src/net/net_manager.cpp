#include "net_manager.h"
#include "net_processor.h"
#include "net_connector.h"
#include "acceptor.h"
#include "accept_adaptor.h"

namespace knet { namespace net {

NetManager::NetManager(NetConfig* config, RawDataHandler* handler):
    _acceptor(0),
    _net_processor(0),
    _connector(new NetConnector()),
    _data_handler(handler),
    _config(config)
{
    assert(_connector != 0);
    assert(_data_handler != 0);
    assert(_config != 0);
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
    _net_processor = new NetProcessor(_config, _data_handler);
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

NetPipe NetManager::getPipe(const ServerLocation& location)
{
    NetPipe pipe;
    TcpSocket* const sock = _connector->connect(location);
    if (sock)
    {
        NetConnection* conn = _net_processor->newConnection(sock);
        if (conn != 0)
        {
            return NetPipe(_net_processor->myID(),
                    conn->myID(),
                    conn->myMask(),
                    1);
        }
    }

    return pipe;
}

}}

