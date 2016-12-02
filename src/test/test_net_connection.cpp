#include "net_manager.h"
#include "net_config.h"
#include "raw_data_handler.h"
#include "global.h"

using namespace knet::net;
using namespace knet::util;

class Demo : public RawDataHandler
{
    public:
        Demo (int sync):_sync(sync) { }
        virtual ~Demo () { }

        virtual int handle(NetPipe& pipe, Buffer& pack)
        {
            //fprintf(stderr, "%s:%d\n", pack.consumer(), pack.getAvailableDataSize());

            Buffer buffer = knet::global::getBuffer(pack.getAvailableDataSize());
            memcpy(buffer.producer(), pack.consumer(), pack.getAvailableDataSize());
            buffer.produce_unsafe(pack.getAvailableDataSize());

            if (_sync)
                pipe.send(buffer);
            else
                pipe.sendForceAsyn(buffer);
        }

    private:
        int _sync;
};

int main(int argc, char** argv)
{
    knet::global::buffer_pool_init(128, 102400);

    Demo demo(atoi(argv[1]));
    NetConfig config;
    config.g_conn_idle_timeout = 1;

    NetManager net_manager(&config, &demo);
    net_manager.startAcceptor(9000);
    net_manager.run();
}

