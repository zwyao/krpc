#ifndef __NET_CONFIG_H__
#define __NET_CONFIG_H__

namespace knet { namespace net {

class NetConfig
{
    public:
        NetConfig();
        ~NetConfig() { }

    public:
        int g_io_write_buffer_init;            // 输出buffer起始大小,按需倍增
        int g_conn_idle_timeout;               // 空闲链接的timeout,秒级
};

}}

#endif

