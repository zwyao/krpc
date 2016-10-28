#include "io_buffer.h"
#include "global.h"

#include <unistd.h>

using namespace util;

int main(int argc, char** argv)
{
    global::small_buffer_pool_init(1024, 1024);
    global::large_buffer_pool_init(1024, 8192);

    util::Buffer buffer = global::getSmallBuffer(1000);
    memcpy(buffer.producer(), "hello", 5);
    buffer.produce_unsafe(5);
    fprintf(stderr, "%d\n", buffer.capacity());
    fprintf(stderr, "%d\n", buffer.getAvailableSpaceSize());
    /*
    IOBuffer io_buffer;
    io_buffer.read(0, 2);
    fprintf(stderr, "%d\n", io_buffer.byte_size());
    io_buffer.read(0, 10000);
    fprintf(stderr, "%d\n", io_buffer.byte_size());

    Buffer& buffer = io_buffer.buffer();
    write(1, buffer.consumer(), io_buffer.byte_size());
    */
}

