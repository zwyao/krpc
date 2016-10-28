#ifndef __MESSAGE_H__
#define __MESSAGE_H__

class Connection;
struct Package
{
    Package* next;

    Connection* owner;
    char* data;
    int size;
    int skip;
};

struct SendList
{
    Package* head;
    Package* tail;
    int total;
};

#endif
