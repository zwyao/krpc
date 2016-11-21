#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <assert.h>

namespace knet { namespace util {

class ListOp
{
    public:
        struct ListNode
        {
            ListNode* next;
            ListNode* prev;

            ListNode():next(this),prev(this) {}
        };

        static void insert(ListNode* node, ListNode* after)
        {
            if (node == after) return;

            node->next = after->next;
            node->prev = after;
            node->next->prev = node;
            node->prev->next = node;
        }

        static void remove(ListNode* node)
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next = node->prev = node;
        }
};

template <typename T, ListOp::ListNode T::*list_node>
class List
{
    public:
        List():
            _root(new ListOp::ListNode()),
            _size(0)
        {
        }

        ~List()
        {
            if (_root) delete _root;
            _root = 0;
        }

        void swap(List<T, list_node>& other)
        {
            ListOp::ListNode* tmp_root = _root;
            _root = other._root;
            other._root = tmp_root;

            int tmp_size = _size;
            _size = other._size;
            other._size = tmp_size;
        }

        void push_back(T* item)
        {
            ListOp::insert(&(item->*list_node), _root->prev);
            ++_size;
        }

        void push_front(T* item)
        {
            ListOp::insert(&(item->*list_node), _root);
            ++_size;
        }

        void pop_back()
        {
            ListOp::remove(_root->prev);
            --_size;
        }

        void pop_front()
        {
            ListOp::remove(_root->next);
            --_size;
        }

        void remove(T* item)
        {
            ListOp::remove(&(item->*list_node));
            --_size;
        }

        T* back()
        {
            return (T*)((char*)(_root->prev)-(char*)_node_offset);
        }

        T* front()
        {
            return (T*)((char*)(_root->next)-(char*)_node_offset);
        }

        T* next(T* item)
        {
            return (T*)((char*)((item->*list_node).next)-(char*)_node_offset);
        }

        T* prev(T* item)
        {
            return (T*)((char*)((item->*list_node).prev)-(char*)_node_offset);
        }

        bool empty() const
        {
            return (_root->next == _root);
        }

        int size() const { return _size; }

        ListOp::ListNode& root() { return *_root; }

        void clear()
        {
            //由使用方负责执行正真的clear
            assert(_size == 0);
            while (_size > 0)
            {
                abort();

                /*
                T* v = (T*)front();
                pop_front();
                --_size;
                delete v;
                */
            }
        }

    private:
        static ListOp::ListNode* const _node_offset;

    protected:
        ListOp::ListNode* _root;
        int _size;
};

template <typename T, ListOp::ListNode T::*list_node>
ListOp::ListNode* const List<T, list_node>::_node_offset = &(((T*)(0))->*list_node);

}}

#endif

