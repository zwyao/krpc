#include "frame.h"

#include <arpa/inet.h>

namespace krpc
{

int DefaultBinaryMsg::unserialize(knet::util::Buffer* buffer)
{
    return 0;
}

int DefaultBinaryMsg::serialize(knet::util::Buffer* buffer)
{
    return 0;
}

}

