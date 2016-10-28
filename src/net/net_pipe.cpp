#include "net_pipe.h"
#include "net_manager.h"

namespace knet
{

int NetPipe::sendUnsafe(util::Buffer& pack)
{
    NetManager* manager = global::g_net_managers[_manager_id];
    assert(manager != 0 && manager->myID() == _manager_id);
    int ret = manager->send(_conn_id, _mask, _channel_id, pack);
    return ret >= 0 ? 0 : -1;
}

int NetPipe::send(util::Buffer& pack)
{
    return 0;
}

}

