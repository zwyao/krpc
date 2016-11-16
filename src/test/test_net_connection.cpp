#include "net_manager.h"
#include "when_receive_packet.h"
#include "global.h"

using namespace knet;
using namespace util;

class Demo : public WhenReceivePacket
{
    public:
        Demo (int sync):_sync(sync) { }
        virtual ~Demo () { }

        virtual int process(NetPipe& pipe, Buffer& pack)
        {
            //fprintf(stderr, "%s:%d\n", pack.consumer(), pack.getAvailableDataSize());

            Buffer buffer = global::getSmallBuffer(pack.getAvailableDataSize());
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
    global::small_buffer_pool_init(128, 102400);
    //global::large_buffer_pool_init(8192, 8192);

    Demo demo(atoi(argv[1]));
    NetManager net_manager(&demo);
    //net_manager.setIdleTimeout(1);
    //net_manager.setWriteBufferBaseSize(131072);
    net_manager.startAcceptor(9000);
    net_manager.run();
}

