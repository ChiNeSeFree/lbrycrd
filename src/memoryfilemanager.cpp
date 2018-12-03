
#include "memoryfilemanager.h"
#include "tinyformat.h"
#include "util.h"

#include <algorithm>
#include <boost/chrono/chrono.hpp>
#include <boost/filesystem.hpp>

CMemoryFileManager::CMemoryFileManager(size_type size) : file_size(size)
{
    assert(size);
    alloc_ptr = dealloc_ptr = createFile(size);
}

CMemoryFileManager::~CMemoryFileManager()
{
    files.clear();
}

void* CMemoryFileManager::allocate(size_type size)
{
    void* ptr;
    while (!(ptr = alloc_ptr->allocate(size))) {
        if (alloc_ptr != files.back()) {
            alloc_ptr = files.back();
            assert(alloc_ptr->size() >= size);
        } else {
            alloc_ptr = createFile(std::max(file_size, size));
        }
    }
    return ptr;
}

void CMemoryFileManager::deallocate(void* ptr)
{
    if (!ptr) return;
    assert(owns(ptr));
    if (!dealloc_ptr->owns(ptr)) {
        std::list<CPointer<CMemoryFile> >::iterator it;
        for (it = files.begin(); it != files.end(); ++it) {
            if ((*it)->owns(ptr)) break;
        }
        assert(it != files.end());
        dealloc_ptr = *it;
    }
    dealloc_ptr->deallocate(ptr);
    if (dealloc_ptr->empty() && files.size() > 1) {
        std::list<CPointer<CMemoryFile> >::iterator it;
        for (it = files.begin(); it != files.end(); ++it) {
            if (dealloc_ptr == *it) {
                files.erase(it);
                break;
            }
        }
        if (alloc_ptr == dealloc_ptr) {
            alloc_ptr = dealloc_ptr = files.back();
        } else {
            dealloc_ptr = files.back();
        }
    }
}

bool CMemoryFileManager::owns(const void* ptr) const
{
    std::list<CPointer<CMemoryFile> >::const_iterator it;
    for (it = files.begin(); it != files.end(); ++it) {
        if ((*it)->owns(ptr)) return true;
    }
    return false;
}

bool CMemoryFileManager::grow(size_type size)
{
    if (alloc_ptr != files.back()) {
        CMemoryFile* file = files.back();
        if (file->size() >= size) return false;
        /// not enough size remove it
        files.pop_back();
    }
    return createFile(size);
}

bool CMemoryFileManager::shrink()
{
    if (files.size() == 1) return false;
    const size_t last_size = files.size();
    std::list<CPointer<CMemoryFile> >::iterator it;
    for (it = files.begin(); it != files.end();) {
        if ((*it)->empty()) {
            it = files.erase(it);
        } else {
            ++it;
        }
    }
    return last_size != files.size();
}

size_t CMemoryFileManager::size() const
{
    return files.size();
}

using namespace boost::filesystem;

static bool DirectoryExists(const path& path)
{
    try {
        TryCreateDirectory(path);
        return true;
    } catch (const filesystem_error&) {
    }
    return false;
}

CMemoryFile* CMemoryFileManager::createFile(size_type size)
{
    std::string file_name;
    static const path shem_path = GetDataDir() / "shem";
    const path init_path = DirectoryExists(shem_path) ? shem_path : current_path();

    do {
        using namespace boost::chrono;
        const system_clock::time_point tp = system_clock::now();
        const milliseconds msec = duration_cast<milliseconds>(tp.time_since_epoch());
        file_name = (init_path / strprintf("%x", msec.count())).string();
    } while (exists(file_name));

    CMemoryFile* file = new CMemoryFile(file_name, size);
    files.insert(files.end(), CPointer<CMemoryFile>(file));
    return file;
}
