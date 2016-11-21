#ifndef __BUFFER_LIST_H__
#define __BUFFER_LIST_H__

#include "buffer.h"
#include "list.h"

namespace knet { namespace util {

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
    knet::util::ListOp::ListNode list_node;

    BufferEntry():
        target(-1, -1)
    {
    }

    BufferEntry(Buffer& buf, int id, int m):
        buffer(buf),
        target(id, m)
    {
    }
};

typedef knet::util::List<BufferEntry, &BufferEntry::list_node> TList;

class BufferEntryCache
{
    public:
        BufferEntryCache(int size = 65536):
            _size(size)
        {
        }

        ~BufferEntryCache() { }

        BufferEntry* get()
        {
            if (likely(_list.empty() == false))
            {
                BufferEntry* entry = _list.front();
                _list.pop_front();
                return entry;
            }
            else
            {
                return new BufferEntry();
            }
        }

        void put(BufferEntry* entry)
        {
            entry->buffer.release();
            _list.push_back(entry);
        }

    private:
        void init();

    private:
        int _size;
        TList _list;
};

}

}}

#endif

