#ifndef MEMORY_FILE_MANAGER_H
#define MEMORY_FILE_MANAGER_H

#include "memoryfile.h"
#include "pointer.h"

#include <boost/shared_ptr.hpp>
#include <list>
#include <string>

/**
 * Class represents a bunch of memory mapped files
 */
class CMemoryFileManager
{
public:
    typedef CMemoryFile::size_type size_type;
    /**
     * Construct memory mapped file
     * @param[in] size      size in bytes
     */
    explicit CMemoryFileManager(size_type size);

    /**
     * Destructs memory mapped files
     */
    ~CMemoryFileManager();

    /**
     * Allocate an object in memory file
     * Returns newly constructed object
     * @param[in] size      size of object
     */
    void* allocate(size_type size);

    /**
     * Deallocate an object in memory file
     * @param[in] ptr       object to destroy
     */
    void deallocate(void* ptr);

    /**
     * Checks whether pointer is owned by manager
     * @param[in] ptr       pointer to check
     */
    bool owns(const void* ptr) const;

    /**
     * Create a new memory mapped file
     * It's limited to create only one file as a plus
     * Returns true on success
     * @param[in] size      size of file
     */
    bool grow(size_type size);

    /**
     * Reduce memory mapped files count
     * Returns true if files count is decreased
     */
    bool shrink();

    /**
     * Number of managed managed files
     * Minimum number files is 1
     */
    size_t size() const;

private:
    /**
     * Forbid copying
     */
    CMemoryFileManager(const CMemoryFileManager&);
    CMemoryFileManager& operator=(const CMemoryFileManager&);

    /**
     * Creates a mapped file with name 'posix_time'
     */
    CMemoryFile* createFile(size_type size);

    std::list<CPointer<CMemoryFile> > files;
    CMemoryFile* alloc_ptr;
    CMemoryFile* dealloc_ptr;
    const size_type file_size;
};

#endif // MEMORY_FILE_MANAGER_H
