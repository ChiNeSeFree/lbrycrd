#ifndef MEMORY_FILE_H
#define MEMORY_FILE_H

#include <boost/interprocess/indexes/null_index.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/mutex_family.hpp>
#include <string>

/**
 * Class represents a fixed size file mapped in virtual memory
 */
class CMemoryFile
{
public:
    /// single proccess accessed flat memory file
    typedef boost::interprocess::basic_managed_mapped_file <
        char,
        boost::interprocess::rbtree_best_fit <
            boost::interprocess::null_mutex_family,
            boost::interprocess::offset_ptr<void>
        >,
        boost::interprocess::null_index
    > memory_mapped_file;
    typedef memory_mapped_file::size_type size_type;

    /**
     * Construct memory mapped file
     * @param[in] name      name of file
     * @param[in] size      size in bytes
     */
    CMemoryFile(const std::string& name, size_type size);

    /**
     * Destructs and deletes memory mapped file
     */
    ~CMemoryFile();

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
     * Checks whether pointer is owned by mapped file
     * @param[in] ptr       pointer to check
     */
    bool owns(const void* ptr) const;

    /**
     * Checks whether file does have allocations
     */
    bool empty() const;

    /**
     * Size of mapped file
     */
    size_type size() const;

private:
    /**
     * Forbid copying
     */
    CMemoryFile(const CMemoryFile&);
    CMemoryFile& operator=(const CMemoryFile&);

    memory_mapped_file* mapped_file;
    const std::string file_name;
    const size_type file_size;
};

#endif // MEMORY_FILE_H
