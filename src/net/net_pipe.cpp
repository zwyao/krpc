#include "net_pipe.h"
#include "net_processor.h"

namespace knet
{

int NetPipe::sendUnsafe(util::Buffer& pack)
{
    NetProcessor* processor = detail::g_net_processors[_processor_id];
    assert(processor != 0 && processor->myID() == _processor_id);
    int ret = processor->send(_conn_id, _mask, _channel_id, pack);
    return ret >= 0 ? 0 : -1;
}

int NetPipe::send(util::Buffer& pack)
{
    return 0;
}

}

