#include "ev.h"
#include "ev_io.h"
#include "ev_timer.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "server.h"

using namespace evnet;

int main(int argc, char** argv)
{
    TimeServer s;
    s.run(9000);
}
