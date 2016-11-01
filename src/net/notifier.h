#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include "callback_object.h"
#include "io_buffer.h"

namespace knet
{

class NetProcessor;
class Notifier : public CallbackObj
{
    public:
        explicit Notifier(NetProcessor* processor);
        virtual ~Notifier();

        int notified(int code, void* data);

    private:
        void start();

    private:
        NetProcessor* const _net_processor;
        int _fd;
        util::IOBuffer _out_buffer;
};

}

#endif

