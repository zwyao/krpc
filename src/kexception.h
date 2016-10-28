#ifndef __KRPC_KEXCEPTION_H__
#define __KRPC_KEXCEPTION_H__

#include <exception>
#include <string>

namespace krpc
{

class TException : public std::exception
{
    public:
        TException() {}

        explicit TException(const std::string& message):
            _message(message)
        {
        }

        virtual ~TException() throw() {}

        virtual const char* what() const throw()
        {
            if (_message.empty())
            {
                return "Default TException.";
            }
            else
            {
                return _message.c_str();
            }   
        }

    protected:
        std::string _message;

};

}

#endif

