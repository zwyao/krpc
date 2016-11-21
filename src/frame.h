#ifndef __KRPC_FRAME_MSG_H__
#define __KRPC_FRAME_MSG_H__

#include "buffer.h"

namespace krpc
{

class FrameMsg
{
    public:
        FrameMsg():_size(0) {}
        virtual ~FrameMsg() {}

        virtual int unserialize(const char* bytes, int len) = 0;
        virtual int serialize(char* bytes) = 0;

        int size() const { return _size; }

    protected:
        int _size;
};

class DefaultBinaryMsg : public FrameMsg
{
    public:
        DefaultBinaryMsg():
            _stream_id(0),
            _bytes(0),
            _bytes_len(0)
        {}

        virtual ~DefaultBinaryMsg();

        virtual int unserialize(knet::util::Buffer* buffer);
        virtual int serialize(knet::util::Buffer* buffer);

    private:
        int _stream_id;
        char* _bytes;
        int _bytes_len;
};

};

#endif

