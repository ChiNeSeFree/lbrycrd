
#include "memoryfile.h"

#include <cstdio>
#include <cstdlib>

CMemoryFile::CMemoryFile(const std::string& name, size_type size) : file_name(name), file_size(size)
{
    assert(size);
    assert(!name.empty());
    std::remove(name.c_str());
    using namespace boost::interprocess;
    mapped_file = new memory_mapped_file(create_only, name.c_str(), size);
}

CMemoryFile::~CMemoryFile()
{
    delete mapped_file;
    std::remove(file_name.c_str());
}

void* CMemoryFile::allocate(size_type size)
{
    return mapped_file->allocate(size, std::nothrow);
}

void CMemoryFile::deallocate(void* ptr)
{
    if (ptr) {
        mapped_file->deallocate(ptr);
    }
}

bool CMemoryFile::owns(const void* ptr) const
{
    return mapped_file->belongs_to_segment(ptr);
}

bool CMemoryFile::empty() const
{
    return mapped_file->all_memory_deallocated();
}

CMemoryFile::size_type CMemoryFile::size() const
{
    return file_size;
}
