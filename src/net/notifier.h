#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include "callback_object.h"
#include "io_buffer.h"

namespace knet
{

class Notifier : public CallbackObj
{
    public:
        explicit Notifier();
        virtual ~Notifier();

        int notified(int code, void* data);

    private:
        void start();

    private:
        //NetManager* const _net_manager;
        int _fd;
        util::IOBuffer _out_buffer;
};

}

#endif

