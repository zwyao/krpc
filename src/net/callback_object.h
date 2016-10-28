#ifndef __CALLBACK_OBJECT_H__
#define __CALLBACK_OBJECT_H__

#include <new>

namespace knet
{

/* 用来让CBMethodCall继承 */
class CBMethodBase
{
    public:
        CBMethodBase() {}
        virtual ~CBMethodBase() {}

        virtual int execute(int code, void* data) = 0;
};

template <typename T>
class CBMethodCall : public CBMethodBase
{
    public:
        typedef int (T::*MethodPtr)(int code, void* data);

        CBMethodCall(T* obj, MethodPtr method):
            _obj(obj),
            _method(method)
        {
        }

        virtual ~CBMethodCall() {}

        virtual int execute(int code, void* data)
        {
            return (_obj->*_method)(code, data);
        }

    private:
        T* _obj;
        MethodPtr _method;
};

/* 确保虚函数表地址位于对象的起始位置 */
class _force_vfp_to_top
{
    public:
        _force_vfp_to_top() {}
        virtual ~_force_vfp_to_top() {}
};

class CallbackObj : public _force_vfp_to_top
{
    public:
        CallbackObj():_method(0) {}

        virtual ~CallbackObj()
        {
            if (_method)
                _method->~CBMethodBase();
        }

        int handleEvent(int code, void* data)
        {
            return _method->execute(code, data);
        }

        template <typename T>
        void setHandle(T* obj, typename CBMethodCall<T>::MethodPtr method)
        {
            if (_method) _method->~CBMethodBase();
            _method = ::new (&_storage) CBMethodCall<T>(obj, method);
        }

    private:
        struct
        {
            char _mem[sizeof(CBMethodCall<CBMethodBase>)];
        } _storage;

        CBMethodBase* _method;
};

template <typename T>
inline void SET_HANDLE(T* obj, typename CBMethodCall<T>::MethodPtr method)
{
    obj->setHandle(obj, method);
}

}

#endif

