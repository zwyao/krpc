#include "net_config.h"

namespace knet { namespace net {

NetConfig::NetConfig():
    g_io_write_buffer_init(65536),
    g_conn_idle_timeout(0)
{
}

}}

