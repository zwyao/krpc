#ifndef __KRPC_STREAM_H__
#define __KRPC_STREAM_H__

#include "struct.h"
#include "stream_exception.h"

#include <stdlib.h>
#include <stdio.h>

namespace krpc
{

template <class TStream>
unsigned int readAll(TStream* stream, char* buf, unsigned int len)
{
    unsigned int have = 0;
    while (have < len)
    {
        unsigned int get = stream->read(buf+have, len-have);
        if (get == 0)
        {
            throw StreamException(StreamException::END_OF_FILE,
                    "no more data to read.");
        }
        have += get;
    }

    return have;
}

/* 如果纯粹的用虚函数实现多态，在类层次比较多的时候     */
/* 可能忘记实现某个虚函数，这样运行的时候可能带来不确定 */
/* 的情况                                               */
class Stream
{
    public:
        virtual ~Stream() {}

        /* 必须实现                             */
        /* @param buf 数据读取后存放的目标地址  */
        /* @param len 期望读取的字节长度        */
        /* @return    实际读取的字节数          */
        /* @throws    出错抛异常StreamException */
        unsigned int read(char* buf, unsigned int len)
        {
            return read_virt(buf, len);
        }

        /* 必须实现                             */
        /* @param buf 期望写入的字节            */
        /* @param len 期望写入的字节长度        */
        /* @throws    出错抛异常StreamException */
        void write(const char* buf, unsigned int len)
        {
            write_virt(buf, len);
        }

        unsigned int readAll(char* buf, unsigned int len)
        {
            return readAll_virt(buf, len);
        }

    public:
        /* 打开stream                        */
        /* @throws 出错抛异常StreamException */
        virtual void open()
        {
            throw StreamException(StreamException::NOT_OPEN, "cannot open base stream.");
        }

        /* 关闭stream                        */
        /* @throws 出错抛异常StreamException */
        virtual void close()
        {
            throw StreamException(StreamException::NOT_OPEN, "cannot close base stream.");
        }

        /* 把buffer里面的数据写出到底层      */
        /* 适用带buffer的stream              */
        /* @throws 出错抛异常StreamException */
        virtual void flush() { }

        virtual unsigned int readEnd() { return 0; }
        virtual unsigned int writeEnd() { return 0; }

    protected:
        virtual unsigned int read_virt(char* buf, unsigned int len)
        {
            throw StreamException(StreamException::NOT_OPEN, "base stream cannot read.");
        }

        virtual void write_virt(const char* buf, unsigned int len)
        {
            throw StreamException(StreamException::NOT_OPEN, "base stream cannot write.");
        }

        virtual unsigned int readAll_virt(char* buf, unsigned int len)
        {
            return krpc::readAll(this, buf, len);
        }

    protected:
        Stream() {}
};

class DefaultStream : public Stream
{
    public:
        virtual ~DefaultStream() {}

        /* 默认实现 */
        unsigned int read(char* buf, unsigned int len)
        {
            throw StreamException(StreamException::NOT_OPEN, "substream must override read.");
        }

        /* 默认实现 */
        void write(const char* buf, unsigned int len)
        {
            throw StreamException(StreamException::NOT_OPEN, "substream must override write.");
        }

        unsigned int readAll(char* buf, unsigned int len)
        {
            return Stream::readAll_virt(buf, len);
        }

    protected:
        DefaultStream():
            Stream()
        {}
};

template <typename Self, typename Super = DefaultStream>
class VirtualStream : public Super
{
    public:
        virtual ~VirtualStream() {}

    protected:
        virtual unsigned int read_virt(char* buf, unsigned int len)
        {
            return static_cast<Self*>(this)->read(buf, len);
        }

        virtual void write_virt(const char* buf, unsigned int len)
        {
            static_cast<Self*>(this)->write(buf, len);
        }

        virtual unsigned int readAll_virt(char* buf, unsigned int len)
        {
            return static_cast<Self*>(this)->readAll(buf, len);
        }

    protected:
        VirtualStream():
            Super()
        {}
};

};

#endif

