#include "conn_id_generator.h"

#include <string.h>

namespace knet { namespace net {

ConnIdGenerator::ConnIdGenerator()
{
    init_conn_ids();
}

void ConnIdGenerator::init_conn_ids()
{
    int id = MAX_CONNECTION_EACH_MANAGER;
    for (int i = 0; i < MAX_CONNECTION_EACH_MANAGER; ++i)
        _conn_ids[i] = --id;
    _conn_id_num = MAX_CONNECTION_EACH_MANAGER;
}

ConnIdMap::ConnIdMap()
{
    init_conn();
}

void ConnIdMap::init_conn()
{
    memset(_connections, 0, sizeof(_connections));
}

}}

