#include "../buffer/fixed_size_allocator.h"

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include <vector>

using namespace krpc;

FixedSizeAllocator allocator(8, 1024);

void* test(void* arg)
{
    char* mem[1000];
    int count = 10000;
    while (count-- > 0)
    {
        int i = 0;
        while (i < 1000)
        {
            char* ptr = allocator.allocate();
            assert(ptr != 0);
            mem[i++] = ptr;
        }

        i = 0;
        while (i < 1000)
            allocator.deallocate(mem[i++]);
    }

    return 0;
}

int main(int argc, char** argv)
{
    int n = 8;
    pthread_t pid[n];
    for (int i = 0; i < n; ++i)
    {
        pthread_create(pid+i, 0, test, 0);
    }

    struct timeval t1, t2;
    gettimeofday(&t1, 0);
    for (int i = 0; i < n; ++i)
    {
        pthread_join(pid[i], 0);
    }
    gettimeofday(&t2, 0);

    fprintf(stderr, "%d\n", (t2.tv_sec-t1.tv_sec)*1000 + (t2.tv_usec-t1.tv_usec)/1000);
}

