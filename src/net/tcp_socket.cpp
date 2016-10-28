#include "tcp_socket.h"

#include <errno.h>
#include <arpa/inet.h>

namespace knet
{

int TcpSocket::listen(int port)
{
    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
    {
        perror("Socket: ");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    int flag = 1;
    ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (::bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        ::close(_fd);
        _fd = -1;
        perror("Bind: ");
        return -1;
    }

    flag = ::fcntl(_fd, F_GETFL, 0);
    if (flag < 0 || ::fcntl(_fd, F_SETFL, flag | O_NONBLOCK) < 0)
    {
        ::close(_fd);
        _fd = -1;
        perror("Set Nonblocking: ");
        return -1;
    }

    int rcv_buffer_size = 65535;
    int snd_buffer_size = 2*rcv_buffer_size;
    if (::setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, sizeof(snd_buffer_size)) == -1)
        perror("set socket send buffer size: ");
    if (::setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &rcv_buffer_size, sizeof(rcv_buffer_size)) == -1)
        perror("set socket recv buffer size: ");

    flag = 1;
    ::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));

    struct linger ling = {0, 0};
    ::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

    if (::listen(_fd, 1024) < 0)
    {
        ::close(_fd);
        _fd = -1;
        perror("Listen: ");
        return -1;
    }

    //TODO
    //globals().openNetFds.Update(1);

    return 0;
}

TcpSocket* TcpSocket::accept()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    int fd = ::accept(_fd, (struct sockaddr*)&addr, &addr_len);
    if (fd < 0) return 0;

    int flag = ::fcntl(fd, F_GETFL, 0);
    if (flag < 0 || ::fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0)
    {
        ::close(fd);
        perror("Set Nonblocking: ");
        return 0;
    }

    TcpSocket* sock = new TcpSocket(fd);
    sock->setNoDelay(true);
    sock->setLinger(true, 0);

    /*
    int size = -100;
    socklen_t len = sizeof(size);
    if (::getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, &len) == -1)
        perror("send buffer: ");
    fprintf(stderr, "%d send buffer size: %d\n", fd, size);

    len = sizeof(size);
    if (::getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, &len) == -1)
        perror("recv buffer: ");
    fprintf(stderr, "%d recv buffer size: %d\n", fd, size);
    */

    //TODO
    //globals().openNetFds.Update(1);

    return sock;
}

int TcpSocket::connect(const struct sockaddr_in* addr, bool nonblocking)
{
    this->close();

    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
    {
        perror("Socket: ");
        return -1;
    }

    int buffer_size = 65535;
    ::setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    ::setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));

    if (nonblocking)
    {
        int flag = ::fcntl(_fd, F_GETFL, 0);
        if (flag < 0 || ::fcntl(_fd, F_SETFL, flag | O_NONBLOCK) < 0)
        {
            ::close(_fd);
            perror("Set Nonblocking: ");
            return -1;
        }
    }

    int ret = ::connect(_fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    if (ret < 0 && errno != EINPROGRESS)
    {
        perror("Connect: ");
        ::close(_fd);
        _fd = -1;
        return -1;
    }

    if (ret < 0 && nonblocking)
        return 0;

    //TODO
    //globals().openNetFds.Update(1);

    return 1;
}

int TcpSocket::connect(const ServerLocation& location, bool nonblocking)
{
    struct sockaddr_in addr;
    if (inet_aton(location.hostname.c_str(), &addr.sin_addr) == 0)
        return -1;
    addr.sin_port = htons(location.port);
    addr.sin_family = AF_INET;
    return this->connect(&addr, nonblocking);
}

void TcpSocket::close()
{
    if (_fd < 0) return;
    ::shutdown(_fd, SHUT_RDWR);
    ::close(_fd);
    _fd = -1;
    //TODO
    //globals().openNetFds.Update(-1);
}

}

