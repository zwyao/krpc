#ifndef __KRPC_CHANNEL_H__
#define __KRPC_CHANNEL_H__

#include "struct.h"
#include "frame.h"

#include <string>

namespace krpc
{

class Channel
{
    public:
        /* client side call */
        Channel(EndPoint* endpoint);
        Channel(EndPoint* endpoint, const std::string& stream_id);

        /* server side call */
        Channel(EndPoint* endpoint,
                const std::string& stream_id,
                Connection* connection);

        virtual ~Channel();

        virtual int send(FrameMsg* frame);
        virtual int receive(FrameMsg* frame);

        const std::string& streamId() { return _stream_id; }

    private:
        Channel(const Channel&);
        const Channel& operator=(const Channel&);

    protected:
        EndPoint _endpoint;
        std::string _stream_id;
        Connection* _connection;
};

};

#endif

