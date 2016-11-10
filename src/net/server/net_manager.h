#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 请正确的初始化 大内存池和小内存池
 *
 * 每个链接有两个IO buffer，一个用来存储到来的数据，一个用来存储发送出去的数据
 * 分别是readbuffer和writebuffer
 * readbuffer采用固定大小，g_read_io_buffer_limit字节，当收到完整的packet时，
 * 会固定调用util::IOBuffer::getSmallBuffer(size)，获取内存，如果size大于 <小内存池> 的大小，
 * 会使用malloc函数申请
 * 对于有些请求数据很大的应用，这个util::IOBuffer::getSmallBuffer显得有些词不达意
 * 所以IO buffer内存池有大小之分也是略显无奈，因为请求数据和输出数据本身谁大谁小，
 * 网络库本身无法判断，这个只能留给 应用去区分
 *
 * 说了这么多，只是想提醒，网络库本身使用getSmallBuffer来获取内存，存储到来的请求数据
 * 如果getSmallBuffer里的内存 < g_read_io_buffer_limit-8，则会调用malloc分配
 *
 *
 * writebuffer维护一个发送buffer列表
 *
 */
namespace knet { namespace server {

class WhenReceivePacket;
class NetProcessor;
class Acceptor;
class NetConnector;
class NetManager
{
    public:
        NetManager(WhenReceivePacket* processor, int idle_timeout = 0);
        ~NetManager();

        /*
         * 启动监听
         */
        void startAcceptor(int port);

        /*
         *
         */
        void startNetProcessor();

        void run();

        inline int getIdleTimeout() const { return _idle_timeout; }

    private:
        Acceptor* _acceptor;
        NetProcessor* _net_processor;
        NetConnector* _connector;
        WhenReceivePacket* const _request_processor;
        // 空闲链接的timeout，不是收发数据的timeout
        // 秒级
        int _idle_timeout;
};

}}

#endif

