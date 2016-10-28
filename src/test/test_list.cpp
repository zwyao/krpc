#include "list.h"

#include <assert.h>
#include <stdio.h>

using namespace krpc;

class ListItem
{
    public:
        ListItem(int v):
            _value(v)
        {
        }

        ~ListItem() {}

        int value() const
        {
            return _value;
        }

    private:
        int _value;

    public:
        ListOp::ListNode _node;
};

void test_front_2_tail()
{
    List<ListItem, &ListItem::_node> tlist;
    assert(tlist.empty() == true);

    for (int i = 0; i < 10; ++i)
    {
        ListItem* item = new ListItem(i);
        tlist.push_back(item);
    }

    assert(tlist.empty() == false);
    assert(tlist.size() == 10);

    for (int i = 0; i < 10; ++i)
    {
        ListItem* item = tlist.front();
        tlist.pop_front();
        fprintf(stderr, "%d ", item->value());
        delete item;
    }
    fprintf(stderr, "\n");

    assert(tlist.empty() == true);
    assert(tlist.size() == 0);
}

void test_tail_2_front()
{
    List<ListItem, &ListItem::_node> tlist;
    assert(tlist.empty() == true);

    for (int i = 0; i < 10; ++i)
    {
        ListItem* item = new ListItem(i);
        tlist.push_front(item);
    }

    assert(tlist.empty() == false);
    assert(tlist.size() == 10);

    for (int i = 0; i < 10; ++i)
    {
        ListItem* item = tlist.back();
        tlist.pop_back();
        fprintf(stderr, "%d ", item->value());
        delete item;
    }
    fprintf(stderr, "\n");

    assert(tlist.empty() == true);
    assert(tlist.size() == 0);
}

int main(int argc, char** argv)
{
    test_front_2_tail();
    test_tail_2_front();
}

