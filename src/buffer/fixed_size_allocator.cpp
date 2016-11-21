#include "fixed_size_allocator.h"
#include "defines.h"

#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

namespace knet { namespace util {

class FixedSizeAllocator::ChunkAllocator
{
    public:
        ChunkAllocator():
            _memory(0),
            _memory_size(0),
            _memory_start(0),
            _free_map(0),
            _free_index(0),
            _avail_count(0),
            _total_count(0),
            _block_size(0),
            _next(0),
            _prev(0)
        {
        }

        ~ChunkAllocator()
        {
            assert(_avail_count == _total_count);
            release();
            fprintf(stderr, "release block count: %d, total count: %d\n",
                    _avail_count,
                    _total_count);
        }

        bool init(int block_size, int block_count);
        void release();
        inline char* allocate();
        inline void  deallocate(char* ptr);

        int getFreeBlock() const { return _avail_count; }
        int getTotalBlock() const { return _total_count; }

    public:
        void* _memory;
        size_t _memory_size;

        char* _memory_start;
        int* _free_map;
        int _free_index;
        int _avail_count;
        int _total_count;
        int _block_size;

        ChunkAllocator* _next;
        ChunkAllocator* _prev;
};

bool FixedSizeAllocator::ChunkAllocator::init(int block_size, int block_count)
{
    if (block_count <= 0) return false;

    int const page_size = (int)sysconf(_SC_PAGESIZE);
    int const page_mask = page_size - 1;
    //页的倍数
    _memory_size = (block_size * block_count + page_mask) & ~page_mask;
    _memory = mmap(0, _memory_size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANON,
            -1, 0);
    if (_memory == MAP_FAILED)
    {
        _memory = 0;
        return false;
    }

    /*
    if (mlock(_memory, _memory_size) != 0)
    {
        release();
        fprintf(stderr, "mlock error, please check ulimit\n");
        return false;
    }
    */

    _free_map = new int[block_count + 1];
    _memory_start = (char*)_memory;

    _avail_count = block_count;
    _total_count = block_count;
    _block_size = block_size;
    for (int i = 0; i < _total_count; ++i)
        _free_map[i] = i + 1;
    _free_map[_total_count] = 0;
    _free_index = 1;

    fprintf(stderr, "page_size: %d, _block_size: %d, _block_count: %d, _memory_size: %ld, _memory: %p\n",
            page_size,
            _block_size,
            block_count,
            _memory_size,
            _memory);

    return true;
}

void FixedSizeAllocator::ChunkAllocator::release()
{
    if (_free_map)
    {
        delete [] _free_map;
        _free_map = 0;
    }

    if (_memory && munmap(_memory, _memory_size) != 0)
        fprintf(stderr, "munmap error\n");
    _memory = 0;
}

char* FixedSizeAllocator::ChunkAllocator::allocate()
{
    if (_avail_count <= 0)
    {
        assert(_free_index == 0 && _avail_count == 0);
        return 0; 
    }

    const int index = _free_index;
    assert(index <= _total_count);

    --_avail_count;
    _free_index = _free_map[index];
    return (_memory_start + ((index-1)*_block_size));
}

void FixedSizeAllocator::ChunkAllocator::deallocate(char* ptr)
{
    assert(ptr >= _memory_start);

    const int offset = ptr - _memory_start;
    const int index = offset/_block_size + 1;

    assert(offset % _block_size == 0);
    assert(index <= _total_count);

    ++_avail_count;
    _free_map[index] = _free_index;
    _free_index = index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FixedSizeAllocator::FixedSizeAllocator():
    _chunks(0),
    _last_chunk(0),
    _alloc_chunk(0),
    _dealloc_chunk(0),
    _block_size(0),
    _block_count(0),
    _chunk_size(0),
    _chunk_count(0),
    _deallocator(this)
{
}

FixedSizeAllocator::FixedSizeAllocator(int block_size):
    _chunks(0),
    _last_chunk(0),
    _alloc_chunk(0),
    _dealloc_chunk(0),
    _block_size(block_size),
    _block_count(256),
    _chunk_size(_block_size*_block_count),
    _chunk_count(0),
    _deallocator(this)
{
    assert(_block_size > 0);
    assert(_chunk_size/_block_count == _block_size);
}

FixedSizeAllocator::FixedSizeAllocator(int block_size, int block_count):
    _chunks(0),
    _last_chunk(0),
    _alloc_chunk(0),
    _dealloc_chunk(0),
    _block_size(block_size),
    _block_count(block_count),
    _chunk_size(_block_size*_block_count),
    _chunk_count(0),
    _deallocator(this)
{
    assert(_block_size > 0);
    assert(_chunk_size/_block_count == _block_size);
}

FixedSizeAllocator::~FixedSizeAllocator()
{
    ChunkAllocator* cur_chunk = 0;
    ChunkAllocator* index_chunk = _chunks;
    while (index_chunk != 0)
    {
        //不用检查内存是否已经全部回收,由使用者保证
        //assert(index_chunk->_avail_count == _block_count);

        cur_chunk = index_chunk;
        index_chunk = index_chunk->_next;

        cur_chunk->release();
        delete cur_chunk;
    }
}

void FixedSizeAllocator::init(int block_size, int block_count)
{
    _block_size = block_size;
    _block_count = block_count;
    _chunk_size = _block_count * _block_size;

    assert(_block_size > 0);
    assert(_chunk_size/_block_count == _block_size);
}

char* FixedSizeAllocator::allocate()
{
    util::Guard<util::MutexLocker> mu(_mutex_locker);

    char* mem = tryAllocate();
    if (likely(mem != 0)) return mem;

    if (_chunk_count == 10)
    {
        ChunkAllocator* chunk = _chunks;
        int avail = 0;
        while (chunk)
        {
            avail += chunk->_avail_count;
            chunk = chunk->_next;
        }

        //fprintf(stderr, "out of memory in pool: %d\n", avail);
        return 0;
    }

    ChunkAllocator* new_chunk = new ChunkAllocator;
    if (new_chunk->init(_block_size, _block_count) == false)
        return 0;

    if (_last_chunk != 0)
    {
        _last_chunk->_next = new_chunk;
        new_chunk->_prev = _last_chunk;
        _last_chunk = new_chunk;
    }
    else
    {
        _chunks = _last_chunk = new_chunk;
    }

    ++_chunk_count;
    _alloc_chunk = new_chunk;
    _dealloc_chunk = _chunks;

    return _alloc_chunk->allocate();
}

char* FixedSizeAllocator::tryAllocate()
{
    if (likely(_alloc_chunk && _alloc_chunk->_avail_count > 0))
        return _alloc_chunk->allocate();

    ChunkAllocator* find_avail = _chunks;
    while (find_avail)
    {
        if (find_avail->_avail_count > 0)
        {
            _alloc_chunk = find_avail;
            return _alloc_chunk->allocate();
        }

        find_avail = find_avail->_next;
    }

    return 0;
}

void FixedSizeAllocator::deallocate(char* ptr)
{
    util::Guard<util::MutexLocker> mu(_mutex_locker);

    assert(_dealloc_chunk != 0);

    _dealloc_chunk = findVicinity(ptr);
    _dealloc_chunk->deallocate(ptr);

    if (_dealloc_chunk->_avail_count == _block_count)
    {
        _alloc_chunk = _dealloc_chunk;
        return;
    }
    /*
    if (_dealloc_chunk->_avail_count == _block_count)
    {
        if (_last_chunk == _dealloc_chunk)
        {
            if (_chunk_count > 1 && _dealloc_chunk->_prev->_avail_count == _block_count)
            {
                ChunkAllocator* new_last_chunk = _dealloc_chunk->_prev;
                new_last_chunk->_next = 0;

                _dealloc_chunk->release();
                delete _dealloc_chunk;

                --_chunk_count;
                _last_chunk = new_last_chunk;
                _alloc_chunk = new_last_chunk;
                _dealloc_chunk = _chunks;
            }
        }
        else if (_last_chunk->_avail_count == _block_count)
        {
            ChunkAllocator* new_last_chunk = _last_chunk->_prev;
            new_last_chunk->_next = 0;

            _last_chunk->release();
            delete _last_chunk;

            --_chunk_count;
            _last_chunk = new_last_chunk;
            _alloc_chunk = _dealloc_chunk;
        }
        else
        {
            if (_dealloc_chunk->_prev == 0)
            {
                _chunks = _dealloc_chunk->_next;
                _chunks->_prev = 0;
            }
            else
            {
                _dealloc_chunk->_prev->_next = _dealloc_chunk->_next;
                _dealloc_chunk->_next->_prev = _dealloc_chunk->_prev;
            }
            _dealloc_chunk->_next = 0;
            _dealloc_chunk->_prev = _last_chunk;
            _last_chunk->_next = _dealloc_chunk;

            _last_chunk = _dealloc_chunk;
            _alloc_chunk = _dealloc_chunk;
        }
    }
    */
}

FixedSizeAllocator::ChunkAllocator* FixedSizeAllocator::findVicinity(char* ptr)
{
    char* memory = ptr;
    ChunkAllocator* low_chunk = _dealloc_chunk;
    ChunkAllocator* high_chunk = _dealloc_chunk->_next;

    while (true)
    {
        if (low_chunk != 0)
        {
            if ((unsigned int)(memory-low_chunk->_memory_start) < (unsigned int)_chunk_size)
                return low_chunk;

            low_chunk = low_chunk->_prev;
        }

        if (high_chunk != 0)
        {
            if ((unsigned int)(memory-high_chunk->_memory_start) < (unsigned int)_chunk_size)
                return high_chunk;

            high_chunk = high_chunk->_next;
        }

        assert(low_chunk != 0 || high_chunk != 0);
    }

    assert(0 && "ooh!");

    return 0;
}

}}

