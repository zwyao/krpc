#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "buffer.h"

namespace knet { namespace global {

extern int g_read_io_buffer_init;

void buffer_pool_init(int block_count, int block_size);

knet::util::Buffer getBuffer(int size);

}}

#endif

