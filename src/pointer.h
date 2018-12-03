#ifndef POINTER_H
#define POINTER_H

#include <boost/shared_ptr.hpp>

template <typename T>
class CPointer
{
public:
    CPointer()
    {
    }

    explicit CPointer(T* ptr) : ptr(ptr)
    {
    }

    CPointer(const CPointer& other) : ptr(other.ptr)
    {
    }

    void reset()
    {
        ptr.reset();
    }

    operator T*() const
    {
        return ptr.get();
    }

    operator bool() const
    {
        return ptr.get();
    }

    T* operator->() const
    {
        return ptr.get();
    }

    T& operator*() const
    {
        return *ptr.get();
    }

    CPointer& operator=(const CPointer& other)
    {
        if (this != &other)
            ptr = other.ptr;
        return *this;
    }

    CPointer& operator=(T* optr)
    {
        if (ptr.get() != optr)
            ptr.reset(optr);
        return *this;
    }

    bool operator==(const CPointer& other) const
    {
        return ptr == other.ptr;
    }

    bool operator!=(const CPointer& other) const
    {
        return !(*this == other);
    }

private:
    boost::shared_ptr<T> ptr;
};

#endif // POINTER_H
