#ifndef __KRPC_PROTOCOL_EXCEPTION_H__
#define __KRPC_PROTOCOL_EXCEPTION_H__

#include "kexception.h"

namespace krpc
{

class ProtocolException : public TException
{
    public:
        enum ExceptionType
        {
            UNKNOWN             = 0,
            INVALID_DATA        = 1,
            NEGATIVE_SIZE       = 2,
            SIZE_LIMIT          = 3,
            BAD_VERSION         = 4,
            NOT_IMPLEMENTED     = 5,
        };

        ProtocolException():
            _type(UNKNOWN)
        {
        }

        ProtocolException(ExceptionType type):
            _type(type)
        {
        }

        ProtocolException(ExceptionType type, const std::string& msg):
            TException(msg),
            _type(type)
        {
        }

        virtual ~ProtocolException() throw() {}

        ExceptionType getType() const throw() { return _type; }

        virtual const char* what() const throw()
        {
            if (_message.empty())
            {
                switch (_type)
                {
                    case UNKNOWN         : return "ProtocolException: Unknown protocol exception";
                    case INVALID_DATA    : return "ProtocolException: Invalid data";
                    case NEGATIVE_SIZE   : return "ProtocolException: Negative size";
                    case SIZE_LIMIT      : return "ProtocolException: Exceeded size limit";
                    case BAD_VERSION     : return "ProtocolException: Invalid version";
                    case NOT_IMPLEMENTED : return "ProtocolException: Not implemented";
                    default              : return "ProtocolException: (Invalid exception type)";
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

}

#endif

