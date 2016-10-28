#include "../callback_object.h"

#include <stdio.h>

using namespace krpc;

class CallbackA : public CallbackObj
{
    public:
        CallbackA()
        {
            SET_HANDLE(this, &CallbackA::callback_a);
        }

        ~CallbackA() {}

        int callback_a(int code, void* data)
        {
            fprintf(stderr, "callback_a\n");
            return 0;
        }

};

class CallbackB : public CallbackObj
{
    public:
        CallbackB()
        {
            setHandle(this, &CallbackB::callback_b);
        }

        ~CallbackB() {}

        int callback_b(int code, void* data)
        {
            fprintf(stderr, "callback_b\n");
            return 0;
        }

};

int main(int argc, char** argv)
{
    CallbackA c1;
    CallbackB c2;

    CallbackObj* cb = &c1;
    cb->handleEvent(1, 0);

    cb = &c2;
    cb->handleEvent(1, 0);

    return 0;
}

