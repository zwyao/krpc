#ifndef __CONN_ID_GENERATOR_H__
#define __CONN_ID_GENERATOR_H__

#include "locker.h"
#include "defines.h"
#include "buffer_list.h"

#include <assert.h>

namespace knet
{

class ConnIdGenerator
{
    private:
        typedef util::SpinLocker  ConnIDLocker;

    public:
        ConnIdGenerator();
        ~ConnIdGenerator() { }

        inline int get()
        {
            util::Guard<ConnIDLocker> m(_id_locker);
            if (likely(_conn_id_num > 0))
                return _conn_ids[--_conn_id_num];
            return -1;
        }

        inline void put(int id)
        {
            util::Guard<ConnIDLocker> m(_id_locker);
            _conn_ids[_conn_id_num++] = id;
        }

    private:
        void init_conn_ids();

    private:
        // connection的id,约定值,范围[0, MAX_CONNECTION_EACH_MANAGER-1]
        int _conn_ids[MAX_CONNECTION_EACH_MANAGER];
        int _conn_id_num;
        ConnIDLocker _id_locker;
};

class NetConnection;
class ConnIdMap
{
    public:
        ConnIdMap();
        ~ConnIdMap() { }

        NetConnection* get(int id)
        {
            // 包含负值的检查
            assert((unsigned int)id < MAX_CONNECTION_EACH_MANAGER);
            return _connections[id];
        }

        void set(int id, NetConnection* conn)
        {
            assert(_connections[id] == 0);
            _connections[id] = conn;
        }

        void erase(int id, NetConnection* conn)
        {
            assert(_connections[id] == conn);
            _connections[id] = 0;
        }

    private:
        void init_conn();

    private:
        NetConnection* _connections[MAX_CONNECTION_EACH_MANAGER];

};

class ConnSendList
{
    private:
        struct SendList
        {
            util::BufferList::TList _connections[MAX_CONNECTION_EACH_MANAGER];
            int id[256];
            int id_size;
        };

        typedef util::SpinLocker PendingDataLocker;

    public:
        ConnSendList():_list(&_one) { }
        ~ConnSendList() { }

        void put(util::BufferList::BufferEntry* entry)
        {
            util::Guard<PendingDataLocker> m(_pending_data_locker);
        }

    private:
        PendingDataLocker _pending_data_locker;
        SendList _one;
        SendList _two;
        SendList* _list;
};

}

#endif

