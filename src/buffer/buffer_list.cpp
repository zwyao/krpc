#include "buffer_list.h"

namespace knet { namespace util {

namespace BufferList
{

void BufferEntryCache::init()
{
    for (int i = 0; i < _size; ++i)
    {
        BufferEntry* entry = new BufferEntry();
        _list.push_back(entry);
    }
}

}

}}

