#ifndef __KRPC_STRUCT_H__
#define __KRPC_STRUCT_H__

#include <stdint.h>

namespace krpc
{

typedef char TByte;

union EndPoint
{
    struct
    {
        int ip;
        int port;
    } host;

    int64_t host_mask;
};

#define HOST_MASK(ep)   (ep)->host_mask
#define HOST_IP(ep)     (ep)->host.ip
#define HOST_PORT(ep)   (ep)->host.port

#define SET_HOST_IP(ep, ip)      (ep)->host.ip = (ip)
#define SET_HOST_PORT(ep, port)  (ep)->host.ip = (port)

};

#endif

