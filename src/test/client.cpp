#include "tcp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

using namespace knet;

int g_channel_id_send = 0;
int g_channel_id_recv = 0;
int g_channel_id = 0;
int g_count = 50000000;
void echo(int fd, const char* str)
{
    char outbuffer[1024];
    char* p = outbuffer + 8;
    strcpy(p, str);
    int len = strlen(p)+1;
    int str_len = len;

    p -= 8;

    *((unsigned int*)p) = htonl(len);
    *((unsigned int*)(p+4)) = htonl(g_channel_id);

    len += 8;
    int ret = ::write(fd, outbuffer, len);
    assert(ret == len);

    char inbuffer[1024];
    ret = ::read(fd, inbuffer, 1024);
    assert(ret == len);

    p = inbuffer + 8;
    fprintf(stderr, "%s:%d\n", p, ntohl(*((unsigned int*)(inbuffer+4))));

    assert(ntohl(*((unsigned int*)(inbuffer+4))) == g_channel_id);
    assert(strcmp(p, str) == 0);
    p -= 8;
    assert(ntohl(*((unsigned int*)p)) == str_len);

    ++g_channel_id;
}

void* send(void* arg)
{
    TcpSocket* sock = (TcpSocket*)arg;
    int fd = sock->fd();

    char buffer[1024];
    char* p = buffer + 8;
    strcpy(p, "hello world!12345678");
    int len = strlen(p)+1;
    *((unsigned int*)buffer) = htonl(len);
    len += 8;

    int i = 0;
    //while (i < g_count)
    while (true)
    {
        *((unsigned int*)(buffer+4)) = htonl(g_channel_id_send);
        ++g_channel_id_send;

        int left = len;
        p = buffer;
        while (left > 0)
        {
            int ret = ::write(fd, p, left);
            if (ret >= 0)
            {
                left -= ret;
                p += ret;
            }
            else
                break;
        }

        ++i;
    }
}

void* recv(void* arg)
{
    TcpSocket* sock = (TcpSocket*)arg;
    int fd = sock->fd();

    int buffer_size = 1048576;
    char* buffer = (char*)::malloc(buffer_size);
    char* p = buffer;
    int have = 0;

    int i = 0;
    //while (i < g_count)
    while (true)
    {
        int ret = ::read(fd, p, buffer_size-have);
        if (ret > 0)
        {
            have += ret;
            p += ret;

            char* data = buffer;
            while (have >= 29)
            {
                assert(ntohl(*((unsigned int*)data)) == 21);
                assert(ntohl(*((unsigned int*)(data+4))) == g_channel_id_recv);
                assert(strcmp(data+8, "hello world!12345678") == 0);

                data += 29;
                have -= 29;

                ++g_channel_id_recv;
                ++i;
            }

            assert(p - data == have);
            if (data < p)
            {
                memcpy(buffer, data, have);
                p = buffer+have;
            }
            else
            {
                p = buffer;
            }
        }
        else
            assert(0);
    }
}

int main(int argc, char** argv)
{
    ServerLocation target;
    target.hostname = argv[1];
    target.port = atoi(argv[2]);
    g_count = atoi(argv[3]);

    TcpSocket sock;
    int ret = sock.connect(target);
    sock.setNoDelay(0);
    assert(ret == 1);

    struct timeval tv1, tv2;

    /*
    gettimeofday(&tv1, 0);
    echo(sock.fd(), argv[3]);
    echo(sock.fd(), argv[3]);
    echo(sock.fd(), argv[3]);
    gettimeofday(&tv2, 0);
    fprintf(stderr, "%d us\n", (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec));
    */

    gettimeofday(&tv1, 0);
    pthread_t r;
    pthread_t w;
    pthread_create(&r, 0, recv, &sock);
    pthread_create(&w, 0, send, &sock);

    pthread_join(r, 0);
    pthread_join(w, 0);
    gettimeofday(&tv2, 0);
    long cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
    fprintf(stderr, "%ld us: %d\n", cost, cost/g_count);
}

