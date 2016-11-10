#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "callback_object.h"

namespace knet
{

class Acceptor
{
    public:
        Acceptor():
            _cb(0),
            _port(-1)
        {
        }

        /*
         * cb will owned by me
         */
        Acceptor(CallbackObj* cb, int port):
            _cb(cb),
            _port(port)
        {
        }

        virtual ~Acceptor()
        {
            if (_cb) delete _cb;
        }

        int listen();

        void setPort(int port) { _port = port; }

        /*
         * cb will owned by me
         */
        void setAcceptCallback(CallbackObj* cb) { _cb = cb; }

    private:
        CallbackObj* _cb;
        int _port;
};

}

#endif

