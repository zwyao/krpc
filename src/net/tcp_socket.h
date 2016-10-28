#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

extern "C"
{
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
}

#include <string>

namespace knet
{

struct ServerLocation
{
    std::string hostname;
    int port;
};

class TcpSocket
{
    public:
        TcpSocket(): _fd(-1) { } 
        TcpSocket(int fd): _fd(fd) { } 
        ~TcpSocket() { this->close(); }

        int listen(int port);
        TcpSocket* accept();
        int connect(const struct sockaddr_in* addr, bool nonblocking = false);
        int connect(const ServerLocation& location, bool nonblocking = false);
        void close();

        bool isGood() const { return _fd >= 0; }
        int fd() const { return _fd; }

        void setNoDelay(bool on)
        {
            int flag = (on ? 1 : 0);
            ::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
        }

        void setLinger(bool on, int linger)
        {
            struct linger l = {(on ? 1 : 0), linger};
            ::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
        }

    private:
        int _fd;
};

};

#endif

