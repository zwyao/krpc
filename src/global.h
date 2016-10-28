#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "buffer.h"

namespace global
{

void small_buffer_pool_init(int block_count, int block_size);
void large_buffer_pool_init(int block_count, int block_size);

util::Buffer getSmallBuffer(int size);
util::Buffer getLargeBuffer(int size);

}

#endif

