#ifndef __KRPC_BUFFER_STREAM_H__
#define __KRPC_BUFFER_STREAM_H__

#include "stream.h"
#include "defines.h"

#include <string.h>
#include <stdio.h>

namespace krpc
{

class BufferBase : public VirtualStream<BufferBase>
{
    public:
        virtual ~BufferBase() {}

        unsigned int read(char* buf, unsigned int len)
        {
            char* p = _rbase + len;
            if (likely(p <= _rbound))
            {
                ::memcpy(buf, _rbase, len);
                _rbase = p;
                return len;
            }

            return readSlow(buf, len);
        }

        void write(const char* buf, unsigned int len)
        {
            char* p = _wbase + len;
            if (likely(p <= _wbound))
            {
                ::memcpy(_wbase, buf, len);
                _wbase = p;
                return;
            }

            writeSlow(buf, len);
        }

        unsigned int readAll(char* buf, unsigned int len)
        {
            char* p = _rbase + len;
            if (likely(p <= _rbound))
            {
                ::memcpy(buf, _rbase, len);
                _rbase = p;
                return len;
            }

            return krpc::readAll(this, buf, len);
        }

    protected:
        virtual unsigned int readSlow(char* buf, unsigned int len) = 0;
        virtual void writeSlow(const char* buf, unsigned int len) = 0;

        void setReadBuffer(char* buf, int len)
        {
            _rbase = buf;
            _rbound = buf + len;
        }

        void setWriteBuffer(char* buf, int len)
        {
            _wbase = buf;
            _wbound = buf + len;
        }

    protected:
        BufferBase():
            _rbase(0),
            _rbound(0),
            _wbase(0),
            _wbound(0)
        {
        }

        char* _rbase;
        char* _rbound;

        char* _wbase;
        char* _wbound;
};

class BufferStream : public VirtualStream<BufferStream, BufferBase>
{
    public:
        static const int BUFFER_DEAULT_SIZE = 512;

        explicit BufferStream(Stream* stream):
            _stream(stream),
            _rbufSize(BUFFER_DEAULT_SIZE),
            _wbufSize(BUFFER_DEAULT_SIZE),
            _rbufBase(new char[_rbufSize]),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        BufferStream(Stream* stream, int buf_sz):
            _stream(stream),
            _rbufSize(buf_sz),
            _wbufSize(buf_sz),
            _rbufBase(new char[_rbufSize]),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        BufferStream(Stream* stream, int rbuf_sz, int wbuf_sz):
            _stream(stream),
            _rbufSize(rbuf_sz),
            _wbufSize(wbuf_sz),
            _rbufBase(new char[_rbufSize]),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        virtual ~BufferStream() {}

        virtual void open()
        {
            _stream->open();
        }

        virtual void close()
        {
            flush();
            _stream->close();
        }

        virtual void flush();

    protected:
        virtual unsigned int readSlow(char* buf, unsigned int len);
        virtual void writeSlow(const char* buf, unsigned int len);

    private:
        void initBufferPointer()
        {
            setReadBuffer(_rbufBase, 0);
            setWriteBuffer(_wbufBase, _wbufSize);
        }

    protected:
        Stream* _stream;
        int _rbufSize;
        int _wbufSize;
        char* _rbufBase;
        char* _wbufBase;
};

class BufferStreamFactory
{
    public:
        BufferStreamFactory() {}

        virtual ~BufferStreamFactory() {}
};

class FramedStream : public VirtualStream<FramedStream, BufferBase>
{
    public:
        static const int BUFFER_DEAULT_SIZE = 512;

        explicit FramedStream(Stream* stream):
            _stream(stream),
            _rbufSize(0),
            _wbufSize(BUFFER_DEAULT_SIZE),
            _rbufBase(0),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        /* 保证写buffer至少有sizeof(int)+1字节 */
        FramedStream(Stream* stream, int buf_sz):
            _stream(stream),
            _rbufSize(0),
            _wbufSize(buf_sz>(int)sizeof(int)?buf_sz:sizeof(int)+1),
            _rbufBase(0),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        /* 保证buffer至少有sizeof(int)+1字节 */
        FramedStream(Stream* stream, int rbuf_sz, int wbuf_sz):
            _stream(stream),
            _rbufSize(rbuf_sz),
            _wbufSize(wbuf_sz>(int)sizeof(int)?wbuf_sz:sizeof(int)+1),
            _rbufBase(new char[_rbufSize]),
            _wbufBase(new char[_wbufSize])
        {
            initBufferPointer();
        }

        virtual ~FramedStream() {}

        virtual void open()
        {
            _stream->open();
        }

        virtual void close()
        {
            flush();
            _stream->close();
        }

        virtual void flush();
        virtual unsigned int readEnd();
        virtual unsigned int writeEnd();

    protected:
        virtual unsigned int readSlow(char* buf, unsigned int len);
        virtual void writeSlow(const char* buf, unsigned int len);

    private:
        unsigned int readFrame();

        void initBufferPointer()
        {
            setReadBuffer(_rbufBase, 0);
            setWriteBuffer(_wbufBase, _wbufSize);
            _wbase += sizeof(int);
        }

    private:
        Stream* _stream;
        int _rbufSize;
        int _wbufSize;
        char* _rbufBase;
        char* _wbufBase;
};

}

#endif

