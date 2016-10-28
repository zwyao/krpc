#ifndef __KRPC_STREAM_EXCEPTION_H__
#define __KRPC_STREAM_EXCEPTION_H__

#include "kexception.h"

namespace krpc
{

class StreamException : public TException
{
    public:
        enum ExceptionType
        {
            UNKNOWN             = 0,
            NOT_OPEN            = 1,
            TIMED_OUT           = 2,
            END_OF_FILE         = 3,
            INTERRUPTED         = 4,
            BAD_ARGS            = 5,
            CORRUPTED_DATA      = 6,
            INTERNAL_ERROR      = 7,
        };

        StreamException():
            _type(UNKNOWN)
        {
        }

        explicit StreamException(ExceptionType type):
            _type(type)
        {
        }

        StreamException(const std::string& msg):
            TException(msg),
            _type(UNKNOWN)
        {
        }

        StreamException(ExceptionType type, const std::string& msg):
            TException(msg),
            _type(type)
        {
        }

        virtual ~StreamException() throw() {}

        ExceptionType getType() const throw() { return _type; }

        virtual const char* what() const throw()
        {
            if (_message.empty())
            {
                switch (_type)
                {
                    case UNKNOWN        : return "StreamException: Unknown stream exception";
                    case NOT_OPEN       : return "StreamException: Stream not open";
                    case TIMED_OUT      : return "StreamException: Timed out";
                    case END_OF_FILE    : return "StreamException: End of file";
                    case INTERRUPTED    : return "StreamException: Interrupted";
                    case BAD_ARGS       : return "StreamException: Invalid arguments";
                    case CORRUPTED_DATA : return "StreamException: Corrupted Data";
                    case INTERNAL_ERROR : return "StreamException: Internal error";
                    default             : return "StreamException: (Invalid exception type)";
                }
            }
            else
            {
                return _message.c_str();
            }
        }

    protected:
        ExceptionType _type;
};

};

#endif

