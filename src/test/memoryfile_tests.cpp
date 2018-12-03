
#include "allocator.h"
#include "claimtrie.h"
#include "memoryfilemanager.h"
#include "main.h"
#include "pointer.h"
#include "test/test_bitcoin.h"

#include <iostream>
#include <map>
#include <memory>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

/// keep in sync with allocator

class CClaimTrieNodeTest;

typedef std::map <
    unsigned char,
    CPointer <CClaimTrieNodeTest>,
    std::less <unsigned char>,
    CContainerAllocator <
        std::pair <
            const unsigned char,
            CPointer <CClaimTrieNodeTest>
        >, CClaimTrieNodeTest
    >
> nodeMapTypeTest;

class CClaimTrieNodeTest
{
public:
    nodeMapTypeTest children;

    static void* operator new(size_t size)
    {
        ++allocations;
        return instance().allocate(size);
    }

    static void operator delete(void* ptr)
    {
        --allocations;
        instance().deallocate(ptr);
    }

    static CMemoryFileManager& instance()
    {
        static CMemoryFileManager manager(10000);
        return manager;
    }

    static size_t allocations;
};

size_t CClaimTrieNodeTest::allocations = 0;

BOOST_FIXTURE_TEST_SUITE(memoryfile_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(allocation_tests)
{
    CMemoryFileManager& manager = CClaimTrieNodeTest::instance();

    /// allocate some objects
    CPointer<CClaimTrieNodeTest> node(new CClaimTrieNodeTest);
    node->children['a'] = new CClaimTrieNodeTest;
    node->children['b'] = new CClaimTrieNodeTest;

    /// operator new should be called 3 times
    BOOST_CHECK(CClaimTrieNodeTest::allocations == 3);

    /// Check that pointers are part of managed file
    BOOST_CHECK(manager.owns(node));
    BOOST_CHECK(node->children.size() == 2);
    BOOST_CHECK(node->children.find('a') != node->children.end());
    BOOST_CHECK(node->children.find('b') != node->children.end());
    BOOST_CHECK(manager.owns(node->children['a']));
    BOOST_CHECK(manager.owns(node->children['b']));

    BOOST_CHECK(manager.size() == 1);

    /// growing should not invalidate allocations
    BOOST_CHECK(manager.grow(10000));

    /// manager should increase managed files
    BOOST_CHECK(manager.size() == 2);

    /// test that allocation still counts
    node->children['c'] = new CClaimTrieNodeTest;

    BOOST_CHECK(CClaimTrieNodeTest::allocations == 4);

    /// Check that pointers are still part of managed file
    BOOST_CHECK(manager.owns(node));
    BOOST_CHECK(node->children.size() == 3);
    BOOST_CHECK(node->children.find('a') != node->children.end());
    BOOST_CHECK(node->children.find('b') != node->children.end());
    BOOST_CHECK(node->children.find('c') != node->children.end());
    BOOST_CHECK(manager.owns(node->children['a']));
    BOOST_CHECK(manager.owns(node->children['b']));
    BOOST_CHECK(manager.owns(node->children['c']));

    /// deleting one child
    node->children['b'].reset();

    /// allocation should decrease
    BOOST_CHECK(CClaimTrieNodeTest::allocations == 3);

    /// let shrink
    BOOST_CHECK(manager.shrink());

    /// manager should not use second mapped file at all
    BOOST_CHECK(manager.size() == 1);

    /// test that allocations still counts
    node->children['a'] = new CClaimTrieNodeTest;

    BOOST_CHECK(CClaimTrieNodeTest::allocations == 3);

    /// create second file again
    BOOST_CHECK(manager.grow(10000));

    /// verify that manager increase managed files
    BOOST_CHECK(manager.size() == 2);

    /// remove all children
    node->children.clear();

    BOOST_CHECK(CClaimTrieNodeTest::allocations == 1);

    /// remove entire node should trigger manager to delete one of managed files
    node.reset();

    /// chaecks that we don't have any other allocations
    BOOST_CHECK(CClaimTrieNodeTest::allocations == 0);

    /// verify that manager has only one managed file
    BOOST_CHECK(manager.size() == 1);
}

BOOST_AUTO_TEST_CASE(memory_mapping_tests)
{
    const std::string ext(".testFile");
    std::list<CPointer<CMemoryFile> > files;

    /// try to allocate a huge amount of managed files
    const int max_files = 128;

    for (int i = 0; i < max_files; ++i) {
        std::string file_name = boost::lexical_cast<std::string>(i) + ext;
        /// creates a file with size of 1MB
        CMemoryFile* file = new CMemoryFile(file_name, 1024);
        files.insert(files.end(), CPointer<CMemoryFile>(file));
        /// verfy that managed file is created on filesystem
        BOOST_CHECK(boost::filesystem::exists(file_name));
    }

    /// all files are created
    BOOST_CHECK(files.size() == max_files);

    /// unmap and deletes all mapped files
    files.clear();

    BOOST_CHECK(files.size() == 0);

    /// mapped files should be gone
    for (int i = 0; i < max_files; ++i) {
        std::string file_name = boost::lexical_cast<std::string>(i) + ext;
        BOOST_CHECK(!boost::filesystem::exists(file_name));
    }
}

BOOST_AUTO_TEST_SUITE_END()
