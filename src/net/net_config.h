#ifndef _NET_CONFIG_H__
#define _NET_CONFIG_H__

namespace knet { namespace net {

class NetConfig
{
    public:
        NetConfig();
        ~NetConfig() { }

    public:
        int g_io_read_buffer_init;
        int g_io_write_buffer_init;
        int g_conn_idle_timeout;               // 空闲链接的timeout,秒级
};

}}

#endif

