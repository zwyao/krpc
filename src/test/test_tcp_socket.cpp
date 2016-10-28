#include "../net/tcp_socket.h"

#include <stdio.h>
#include <assert.h>

using namespace krpc;

int main(int argc, char** argv)
{
    TcpSocket sock;
    int ret = sock.listen(9090);
    assert(ret == 0);

    TcpSocket* new_sock = sock.accept();
    if (new_sock == 0)
        perror("accept: ");

    getchar();
}

