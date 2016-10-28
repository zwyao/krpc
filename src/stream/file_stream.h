#ifndef __KRPC_FILE_STREAM_H__
#define __KRPC_FILE_STREAM_H__

#include "stream_exception.h"
#include "stream.h"
#include "defines.h"

#include <stdio.h>

namespace krpc
{

class FileStream : public VirtualStream<FileStream>
{
    public:
        FileStream():
            _mode("wr"),
            _file(0)
        {
        }

        FileStream(const std::string& file_name, const std::string& mode):
            _file_name(file_name),
            _mode(mode),
            _file(0)
        {
        }

        virtual ~FileStream()
        {
            this->close();
        }

        void open(const std::string& mode)
        {
            _mode = mode;
            open();
        }

        void open(const std::string& file_name, const std::string& mode)
        {
            _file_name = file_name;
            _mode = mode;
            open();
        }

        unsigned int read(char* buf, unsigned int len)
        {
            if (unlikely(_file == 0)) return 0;

            return fread(buf, 1, len, _file);
        }

        void write(const char* buf, unsigned int len)
        {
            if (unlikely(_file == 0))
                throw StreamException(StreamException::NOT_OPEN,
                        "file not open.");

            fwrite(buf, 1, len, _file);
        }

        unsigned int readAll(char* buf, unsigned int len)
        {
            return this->read(buf, len);
        }

        virtual void open();
        virtual void close();
        virtual void flush();

    protected:
        std::string _file_name;
        std::string _mode;
        FILE* _file;
};

}

#endif

