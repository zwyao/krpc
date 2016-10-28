#include "tcp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

using namespace knet;

int g_channel_id = 0;
int g_count = 5000000;
void echo(int fd)
{
    char inbuffer[1024];
    char outbuffer[1024];
    char* p = outbuffer + 8;
    strcpy(p, "hello world!");
    int len = strlen(p)+1;

    p -= 8;

    *((unsigned int*)p) = htonl(len);
    *((unsigned int*)(p+4)) = htonl(g_channel_id);

    ++g_channel_id;

    len += 8;
    int ret = ::write(fd, outbuffer, len);
    assert(ret == len);

    ret = ::read(fd, inbuffer, 1024);
    assert(ret == len);

    p = inbuffer + 8;
    fprintf(stderr, "%s\n", p);

    assert(strcmp(p, "hello world!") == 0);
    p -= 8;
    assert(ntohl(*((unsigned int*)p)) == 13);
}

void* send(void* arg)
{
    TcpSocket* sock = (TcpSocket*)arg;
    int fd = sock->fd();

    char buffer[1024];
    char* p = buffer + 8;
    strcpy(p, "hello world!");
    int len = strlen(p)+1;
    *((unsigned int*)buffer) = htonl(len);
    len += 8;

    int i = 0;
    while (i < g_count)
    {
        *((unsigned int*)(buffer+4)) = htonl(g_channel_id);
        ++g_channel_id;

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

    char buffer[1024];
    char* p = buffer;
    int have = 0;

    int i = 0;
    while (i < g_count)
    {
        int ret = ::read(fd, p, 1024-have);
        if (ret > 0)
        {
            have += ret;
            p += ret;

            char* data = buffer;
            while (have >= 21)
            {
                assert(ntohl(*((unsigned int*)data)) == 13);
                assert(strcmp(data+8, "hello world!") == 0);

                data += 21;
                have -= 21;

                ++i;
            }

            if (data > buffer && have > 0)
            {
                assert(p - data == have);
                memcpy(buffer, data, have);
                p = buffer+have;
            }
            else if (data > buffer)
            {
                assert(p == data);
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

    TcpSocket sock;
    int ret = sock.connect(target);
    assert(ret == 1);

    struct timeval tv1, tv2;

    gettimeofday(&tv1, 0);
    echo(sock.fd());
    gettimeofday(&tv2, 0);
    fprintf(stderr, "%d us\n", (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec));

    /*
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
    */
}

