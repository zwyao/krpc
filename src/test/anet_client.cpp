#include "tcp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

using namespace knet;

int g_channel_id_send = 1;
int g_channel_id_recv = 1;
int g_channel_id = 1;
int g_count = 50000000;
void echo(int fd)
{
    char inbuffer[1024];
    char outbuffer[1024];
    char* p = outbuffer + 16;
    strcpy(p, "hello world!");
    int len = strlen(p)+1;
    *((unsigned int*)(outbuffer)) = htonl(0x416e4574);
    *((unsigned int*)(outbuffer+4)) = htonl(g_channel_id);
    *((unsigned int*)(outbuffer+8)) = htonl(1);
    *((unsigned int*)(outbuffer+12)) = htonl(len);

    ++g_channel_id;

    len += 16;
    int ret = ::write(fd, outbuffer, len);
    assert(ret == len);

    ret = ::read(fd, inbuffer, 1024);
    assert(ret == len);

    p = inbuffer + 16;
    fprintf(stderr, "%s\n", p);

    assert(strcmp(p, "hello world!") == 0);
    p -= 4;
    assert(ntohl(*((unsigned int*)p)) == 13);
}

void* send(void* arg)
{
    TcpSocket* sock = (TcpSocket*)arg;
    int fd = sock->fd();

    char buffer[1024];
    char* p = buffer + 16;
    strcpy(p, "hello world!");
    int len = strlen(p)+1;

    *((unsigned int*)(buffer)) = htonl(0x416e4574);
    *((unsigned int*)(buffer+8)) = htonl(1);
    *((unsigned int*)(buffer+12)) = htonl(len);
    len += 16;

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
                assert(ntohl(*((unsigned int*)(data+4))) == g_channel_id_recv);
                assert(ntohl(*((unsigned int*)(data+12))) == 13);
                assert(strcmp(data+16, "hello world!") == 0);

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
    echo(sock.fd());
    echo(sock.fd());
    echo(sock.fd());
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

