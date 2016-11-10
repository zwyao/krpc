#ifndef __ACCEPT_ADAPTOR_H__
#define __ACCEPT_ADAPTOR_H__

#include "callback_object.h"

namespace knet
{

class NetProcessor;
class AcceptAdaptor: public CallbackObj
{
    public:
        explicit AcceptAdaptor(NetProcessor* processor):
            _net_processor(processor)
        {
            SET_HANDLE(this, &AcceptAdaptor::start);
        }

        virtual ~AcceptAdaptor() { }

        int start(int code, void* data);
        int when(int code, void* data);

    private:
        NetProcessor* _net_processor;

};

}

#endif

