#ifndef _NET_CONFIG_H__
#define _NET_CONFIG_H__

namespace knet { namespace net {

class NetConfig
{
    public:
        NetConfig() { }
        ~NetConfig() { }

    public:
        int g_read_io_buffer_init;
        int g_write_io_buffer_init;
};

}}

#endif

