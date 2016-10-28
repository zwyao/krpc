#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "defines.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

namespace util
{

typedef void (*buffer_deallocator) (char* ptr);

inline void donothing_deallocator(char* ptr) { (void)ptr; }
inline void free_deallocator(char* ptr) { ::free(ptr); }
inline void delete_deallocator(char* ptr) { delete [] ptr; }

class Buffer
{
    public:
        Buffer():
            _base(0),
            _end(0),
            _producer(0),
            _consumer(0),
            _deallocator(donothing_deallocator)
        {
        }

        explicit Buffer(int capacity):
            _base((char*)malloc(capacity)),
            _end(_base+capacity),
            _producer(_base),
            _consumer(_base),
            _deallocator(free_deallocator)
        {
            assert(_base != 0 && capacity > 0);
            //fprintf(stderr, "malloc constructor\n");
        }

        Buffer(char* buffer, int capacity, buffer_deallocator deallocator):
            _base(buffer),
            _end(_base+capacity),
            _producer(_base),
            _consumer(_base),
            _deallocator(deallocator)
        {
            assert(_base != 0 && capacity > 0 && deallocator != 0);
            //fprintf(stderr, "specifial constructor\n");
        }

        Buffer(Buffer& other):
            _base(other._base),
            _end(other._end),
            _producer(other._producer),
            _consumer(other._consumer),
            _deallocator(other._deallocator)
        {
            other._base = other._end = other._producer = other._consumer = 0;
            other._deallocator = donothing_deallocator;
            //fprintf(stderr, "copy constructor\n");
        }

        Buffer(const Buffer& other):
            _base(other._base),
            _end(other._end),
            _producer(other._producer),
            _consumer(other._consumer),
            _deallocator(other._deallocator)
        {
            Buffer& that = const_cast<Buffer&>(other);
            that._base = that._end = that._producer = that._consumer = 0;
            that._deallocator = donothing_deallocator;
            //fprintf(stderr, "const copy constructor\n");
        }

        Buffer& operator=(Buffer& other)
        {
            //fprintf(stderr, "=\n");
            _deallocator(_base);

            _base        = other._base;
            _end         = other._end;
            _producer    = other._producer;
            _consumer    = other._consumer;
            _deallocator = other._deallocator;

            other._base = other._end = other._producer = other._consumer = 0;
            other._deallocator = donothing_deallocator;
            return *this;
        }

        Buffer& operator=(const Buffer& other)
        {
            //fprintf(stderr, "const =\n");
            _deallocator(_base);

            _base        = other._base;
            _end         = other._end;
            _producer    = other._producer;
            _consumer    = other._consumer;
            _deallocator = other._deallocator;

            Buffer& that = const_cast<Buffer&>(other);
            that._base = that._end = that._producer = that._consumer = 0;
            that._deallocator = donothing_deallocator;
            return *this;
        }

        ~Buffer()
        {
            if (_base)
                _deallocator(_base);
        }

        void init(char* buffer, int capacity, buffer_deallocator deallocator)
        {
            assert(_base == 0);

            _base = buffer;
            _end = _base + capacity;
            _producer = _base;
            _consumer = _base;
            _deallocator = deallocator;

            assert(_base != 0 && capacity > 0 && _deallocator);
        }

        char* getBuffer() { return _base; }
        char* producer() { return _producer; }
        char* consumer() { return _consumer; }

        int consume_unsafe(int bytes) { _consumer += bytes; return bytes; }
        int produce_unsafe(int bytes) { _producer += bytes; return bytes; }

        int consume(int bytes)
        {
            int data_available = getAvailableDataSize();
            int nbytes = mmax(0, mmin(data_available, bytes));
            _consumer += nbytes;
            return nbytes;
        }

        int produce(int bytes)
        {
            int space_available = getAvailableSpaceSize();
            int nbytes = mmax(0, mmin(space_available, bytes));
            _producer += nbytes;
            return nbytes;
        }

        int getAvailableSpaceSize() const
        {
            return _end - _producer;
        }

        int getAvailableDataSize() const
        {
            return _producer - _consumer;
        }

        int capacity() const
        {
            return _end - _base;
        }

        void moveTo(Buffer& other) const
        {
            //TODO
            int avl_data_size = getAvailableDataSize();
            memcpy(other._producer, _consumer, avl_data_size);
            other._producer += avl_data_size;
        }

        void copyTo(Buffer& other) const
        {
            int avl_data_size = getAvailableDataSize();
            memcpy(other._producer, _consumer, avl_data_size);
            other._producer += avl_data_size;
        }

        void clear()
        {
            _producer = _consumer = _base;
        }

        void release()
        {
            _deallocator(_base);

            _base = 0;
            _end = 0;
            _producer = 0;
            _consumer = 0;
            _deallocator = donothing_deallocator;
        }

        void compact()
        {
            if (_base != _consumer)
            {
                int sz = getAvailableDataSize();
                memmove(_base, _consumer, sz);
                _consumer = _base;
                _producer = _base + sz;
            }
        }

    protected:
        char* _base;
        char* _end;
        char* _producer;
        char* _consumer;
        buffer_deallocator _deallocator;
};

}

#endif

