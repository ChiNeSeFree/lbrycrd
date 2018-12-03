#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "memoryfilemanager.h"

#include <limits>

#ifndef MEM_FILE_SIZE
#define MEM_FILE_SIZE   size_t(1024) * 1024 * 1024
#endif

template <typename T>
class CTypeAllocator
{
public:
    /// Custom new / delete operators to make use of mapped file
    static void* operator new(size_t size)
    {
        return instance().allocate(size);
    }

    static void operator delete(void* ptr)
    {
        instance().deallocate(ptr);
    }

    static CMemoryFileManager& instance()
    {
        static CMemoryFileManager manager(MEM_FILE_SIZE);
        return manager;
    }
};

template <typename T, typename A = T>
class CContainerAllocator
{
public:
    typedef T value_type;

    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;

    typedef T& reference;
    typedef const T& const_reference;

    template <typename U>
    struct rebind {
        typedef CContainerAllocator<U, A> other;
    };

    CContainerAllocator()
    {
    }

    pointer address(reference r) const
    {
        return &r;
    }

    const_pointer address(const_reference r) const
    {
        return &r;
    }

    pointer allocate(size_type n, const_pointer = 0)
    {
        void* ptr = CTypeAllocator<A>::instance().allocate(n * sizeof(T));
        return reinterpret_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type)
    {
        CTypeAllocator<A>::instance().deallocate(p);
    }

    void construct(pointer p, const_reference value)
    {
        new(p) T(value);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
};

template <typename T, typename A> inline
bool operator==(const CContainerAllocator<T, A>&, const CContainerAllocator<T, A>&)
{
    return true;
}

template <typename T, typename A> inline
bool operator!=(const CContainerAllocator<T, A>&, const CContainerAllocator<T, A>&)
{
    return false;
}

#endif // ALLOCATOR_H
