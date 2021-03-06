#ifndef __NET_EVENT_H__
#define __NET_EVENT_H__

namespace knet { namespace net {

enum EventCode {
    EVENT_NONE = 1,
    EVENT_LISTEN,
    EVENT_CONNECTING,
    EVENT_CONNECTED,
    EVENT_NEW_CONNECTION,
    EVENT_NET_READ,
    EVENT_NET_WROTE,
    EVENT_NET_EOF,
    EVENT_NET_ERROR,
    EVENT_SYNC_DONE,
    EVENT_CMD_DONE,
    EVENT_INACTIVITY_TIMEOUT,
    EVENT_TIMEOUT
};

}}

#endif

