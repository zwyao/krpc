#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include "callback_object.h"
#include "io_buffer.h"

#include <stdint.h>
#include <unistd.h>

namespace knet { namespace server {

class NetProcessor;
class Notifier : public CallbackObj
{
    public:
        explicit Notifier(NetProcessor* processor);
        virtual ~Notifier();

        int notified(int code, void* data);

        inline void notify()
        {
            uint64_t one = 1;
            ::write(_fd, &one, sizeof(one));
        }

    private:
        void start();

    private:
        NetProcessor* const _net_processor;
        int _fd;
        util::IOBuffer _out_buffer;
};

}}

#endif

