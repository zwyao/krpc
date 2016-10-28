#ifndef __KRPC_SERVICE_H__
#define __KRPC_SERVICE_H__

namespace krpc
{

class MethodDescriptor;
class RpcController;
class Message;

class Service
{
    public:
        Service() {}
        virtual ~Service() {}

        virtual void callMethod(const MethodDescriptor* method,
                RpcController* controller,
                const Message* request,
                Message* response) = 0;
};

}

#endif

