#include "global.h"
#include "fixed_size_allocator.h"

namespace
{

knet::util::FixedSizeAllocator g_buffer_allocator;
bool g_buffer_init = false;
int  g_buffer_size = 4096;

inline knet::util::Buffer getBufferHelper(int size)
{
    if (likely(g_buffer_init && size <= g_buffer_size))
    {
        char* mem = g_buffer_allocator.allocate();
        if (likely(mem != 0))
            return knet::util::Buffer(mem, g_buffer_size, g_buffer_allocator.getDeallocator());
        else
            return size > 0 ? knet::util::Buffer(size) : knet::util::Buffer(g_buffer_size);
    }
    else
    {
        return size > 0 ? knet::util::Buffer(size) : knet::util::Buffer(g_buffer_size);
    }
}

}

namespace knet { namespace global {

void buffer_pool_init(int block_size, int block_count)
{
    if (block_count <= 0 || block_size <= 0)
    {
        fprintf(stderr, "Buffer pool init error\n");
        abort();
    }

    if (g_buffer_init == false)
    {
        g_buffer_allocator.init(block_size, block_count);
        g_buffer_init = true;
        g_buffer_size = block_size;
    }
}

knet::util::Buffer getBuffer(int size)
{
    util::Buffer buffer = getBufferHelper(size+8);
    buffer.produce_unsafe(8);
    buffer.consume_unsafe(8);
    return buffer;
}

int getBufferSize()
{
    return g_buffer_size;
}

}}

