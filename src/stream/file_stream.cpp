#include "file_stream.h"

namespace krpc
{

void FileStream::open()
{
    FILE* fp = fopen(_file_name.c_str(), _mode.c_str());
    if (fp == 0)
        throw StreamException(StreamException::NOT_OPEN,
                "cannot open file.");

    if (_file) fclose(_file);

    _file = fp;
}

void FileStream::close()
{
    if (_file)
    {
        fclose(_file);
        _file = 0;
    }
}

void FileStream::flush()
{
    if (_file)
        fflush(_file);
}

}

