#include "frame.h"

#include <arpa/inet.h>

namespace krpc
{

int DefaultBinaryMsg::unserialize(util::Buffer* buffer)
{
    return 0;
}

int DefaultBinaryMsg::serialize(util::Buffer* buffer)
{
    return 0;
}

}

