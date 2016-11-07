#include "net_manager.h"
#include "when_receive_packet.h"
#include "global.h"

using namespace knet;
using namespace knet::server;
using namespace util;

class Demo : public WhenReceivePacket
{
    public:
        Demo () { }
        virtual ~Demo () { }

        virtual int process(NetPipe& pipe, Buffer& pack)
        {
            //fprintf(stderr, "%s:%d\n", pack.consumer(), pack.getAvailableDataSize());

            Buffer buffer = global::getSmallBuffer(pack.getAvailableDataSize());
            memcpy(buffer.producer(), pack.consumer(), pack.getAvailableDataSize());
            buffer.produce_unsafe(pack.getAvailableDataSize());

            pipe.sendForceAsyn(buffer);
            //pipe.send(buffer);
        }
};

int main(int argc, char** argv)
{
    global::small_buffer_pool_init(1024, 1024);
    global::large_buffer_pool_init(1024, 8192);

    Demo demo;
    NetManager net_manager(&demo, 1);
    net_manager.startAcceptor(9000);
    net_manager.run();
}

