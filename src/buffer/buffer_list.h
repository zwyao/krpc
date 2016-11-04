#ifndef __BUFFER_LIST_H__
#define __BUFFER_LIST_H__

#include "list.h"

namespace util
{

namespace BufferList
{

struct Target
{
    int conn_id;
    int mask;

    Target(int id, int m):
        conn_id(id),
        mask(m)
    {
    }
};

struct BufferEntry
{
    Buffer buffer;
    Target target;
    util::ListOp::ListNode list_node;

    BufferEntry(Buffer& buf, int id, int m):
        buffer(buf),
        target(id, m)
    {
    }
};

typedef util::List<BufferEntry, &BufferEntry::list_node> TList;

}

}

#endif

