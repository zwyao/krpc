#ifndef __KNET_GLOBAL_H__
#define __KNET_GLOBAL_H__

#include "buffer.h"

namespace knet { namespace global {

void buffer_pool_init(int block_size, int block_count);
knet::util::Buffer getBuffer(int size);
int getBufferSize();

}}

#endif

