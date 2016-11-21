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
#include <errno.h>

}

#include <string>

namespace knet { namespace net {

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
            if (::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
                fprintf(stderr, "Set TCP_NODELAY error\n");
        }

        void setLinger(bool on, int linger)
        {
            struct linger l = {(on ? 1 : 0), linger};
            ::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
        }

        int getSocketError()
        {
            int optval;
            socklen_t optlen = sizeof(optval);
            if (::getsockopt(_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                return errno;
            else
                return optval;
        }

        struct sockaddr_in6 getLocalAddr()
        {
            struct sockaddr_in6 localaddr;
            bzero(&localaddr, sizeof(localaddr));
            socklen_t addrlen = sizeof(localaddr);
            ::getsockname(_fd, (struct sockaddr*)(&localaddr), &addrlen);
            return localaddr;
        }

        struct sockaddr_in6 getPeerAddr()
        {
            struct sockaddr_in6 peeraddr;
            bzero(&peeraddr, sizeof(peeraddr));
            socklen_t addrlen = sizeof(peeraddr);
            ::getpeername(_fd, (struct sockaddr*)(&peeraddr), &addrlen);
            return peeraddr;
        }

        bool isSelfConnect()
        {
            struct sockaddr_in6 localaddr = getLocalAddr();
            struct sockaddr_in6 peeraddr = getPeerAddr();
            if (localaddr.sin6_family == AF_INET)
            {
                const struct sockaddr_in* laddr4 = (struct sockaddr_in*)(&localaddr);
                const struct sockaddr_in* raddr4 = (struct sockaddr_in*)(&peeraddr);
                return laddr4->sin_port == raddr4->sin_port
                    && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
            }
            else if (localaddr.sin6_family == AF_INET6)
            {
                return localaddr.sin6_port == peeraddr.sin6_port
                    && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
            }
            else
            {
                return false;
            }
        }

    private:
        int _fd;
};

}}

#endif

