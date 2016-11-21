#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "buffer.h"

namespace knet { namespace global {

extern int g_read_io_buffer_init;

void small_buffer_pool_init(int block_count, int block_size);
void large_buffer_pool_init(int block_count, int block_size);

knet::util::Buffer getSmallBuffer(int size);
knet::util::Buffer getLargeBuffer(int size);

}}

#endif

