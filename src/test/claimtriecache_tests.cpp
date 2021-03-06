#include "claimtrie.h"
#include "main.h"
#include "nameclaim.h"
#include "uint256.h"

#include "test/test_bitcoin.h"
#include <boost/test/unit_test.hpp>
using namespace std;


class CClaimTrieCacheTest : public CClaimTrieCache {
public:
    CClaimTrieCacheTest(CClaimTrie* base):
        CClaimTrieCache(base, false){}

    bool recursiveComputeMerkleHash(CClaimTrieNode* tnCurrent,
                                    std::string sPos) const
    {
        return CClaimTrieCache::recursiveComputeMerkleHash(tnCurrent, sPos);
    }

    bool recursivePruneName(CClaimTrieNode* tnCurrent, unsigned int nPos, std::string sName, bool* pfNullified) const
    {
        return CClaimTrieCache::recursivePruneName(tnCurrent,nPos,sName, pfNullified);
    }

    bool insertSupportIntoMap(const std::string& name, CSupportValue support, bool fCheckTakeover) const
    {
        return CClaimTrieCache::insertSupportIntoMap(name, support, fCheckTakeover);
    }

    int cacheSize()
    {
        return cache.size();
    }

    nodeCacheType::iterator getCache(std::string key)
    {
        return cache.find(key);
    }

};

CMutableTransaction BuildTransaction(const uint256& prevhash)
{
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.nLockTime = 0;
    tx.vin.resize(1);
    tx.vout.resize(1);
    tx.vin[0].prevout.hash = prevhash;
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript();
    tx.vin[0].nSequence = std::numeric_limits<unsigned int>::max();
    tx.vout[0].scriptPubKey = CScript();
    tx.vout[0].nValue = 0;

    return tx;
}

BOOST_FIXTURE_TEST_SUITE(claimtriecache_tests, RegTestingSetup)


BOOST_AUTO_TEST_CASE(merkle_hash_single_test)
{
    // check empty trie
    uint256 one(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));
    CClaimTrieCacheTest cc(pclaimTrie);
    BOOST_CHECK(one == cc.getMerkleHash());

    // check trie with only root node
    CClaimTrieNode base_node;
    cc.recursiveComputeMerkleHash(&base_node, "");
    BOOST_CHECK(one == cc.getMerkleHash());
}

BOOST_AUTO_TEST_CASE(merkle_hash_multiple_test)
{
    CClaimValue unused;
    uint256 hash0(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));
    uint160 hash160;
    CMutableTransaction tx1 = BuildTransaction(hash0);
    COutPoint tx1OutPoint(tx1.GetHash(), 0);
    CMutableTransaction tx2 = BuildTransaction(tx1.GetHash());
    COutPoint tx2OutPoint(tx2.GetHash(), 0);
    CMutableTransaction tx3 = BuildTransaction(tx2.GetHash());
    COutPoint tx3OutPoint(tx3.GetHash(), 0);
    CMutableTransaction tx4 = BuildTransaction(tx3.GetHash());
    COutPoint tx4OutPoint(tx4.GetHash(), 0);
    CMutableTransaction tx5 = BuildTransaction(tx4.GetHash());
    COutPoint tx5OutPoint(tx5.GetHash(), 0);
    CMutableTransaction tx6 = BuildTransaction(tx5.GetHash());
    COutPoint tx6OutPoint(tx6.GetHash(), 0);

    uint256 hash1;
    hash1.SetHex("71c7b8d35b9a3d7ad9a1272b68972979bbd18589f1efe6f27b0bf260a6ba78fa");

    uint256 hash2;
    hash2.SetHex("c4fc0e2ad56562a636a0a237a96a5f250ef53495c2cb5edd531f087a8de83722");

    uint256 hash3;
    hash3.SetHex("baf52472bd7da19fe1e35116cfb3bd180d8770ffbe3ae9243df1fb58a14b0975");

    uint256 hash4;
    hash4.SetHex("c73232a755bf015f22eaa611b283ff38100f2a23fb6222e86eca363452ba0c51");

    BOOST_CHECK(pclaimTrie->empty());

    CClaimTrieCache ntState(pclaimTrie, false);
    ntState.insertClaimIntoTrie(std::string("test"), CClaimValue(tx1OutPoint, hash160, 50, 100, 200));
    ntState.insertClaimIntoTrie(std::string("test2"), CClaimValue(tx2OutPoint, hash160, 50, 100, 200));

    BOOST_CHECK(pclaimTrie->empty());
    BOOST_CHECK(!ntState.empty());
    BOOST_CHECK(ntState.getMerkleHash() == hash1);

    ntState.insertClaimIntoTrie(std::string("test"), CClaimValue(tx3OutPoint, hash160, 50, 101, 201));
    BOOST_CHECK(ntState.getMerkleHash() == hash1);
    ntState.insertClaimIntoTrie(std::string("tes"), CClaimValue(tx4OutPoint, hash160, 50, 100, 200));
    BOOST_CHECK(ntState.getMerkleHash() == hash2);
    ntState.insertClaimIntoTrie(std::string("testtesttesttest"), CClaimValue(tx5OutPoint, hash160, 50, 100, 200));
    ntState.removeClaimFromTrie(std::string("testtesttesttest"), tx5OutPoint, unused);
    BOOST_CHECK(ntState.getMerkleHash() == hash2);
    ntState.flush();

    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash2);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState1(pclaimTrie, false);
    ntState1.removeClaimFromTrie(std::string("test"), tx1OutPoint, unused);
    ntState1.removeClaimFromTrie(std::string("test2"), tx2OutPoint, unused);
    ntState1.removeClaimFromTrie(std::string("test"), tx3OutPoint, unused);
    ntState1.removeClaimFromTrie(std::string("tes"), tx4OutPoint, unused);

    BOOST_CHECK(ntState1.getMerkleHash() == hash0);

    CClaimTrieCache ntState2(pclaimTrie, false);
    ntState2.insertClaimIntoTrie(std::string("abab"), CClaimValue(tx6OutPoint, hash160, 50, 100, 200));
    ntState2.removeClaimFromTrie(std::string("test"), tx1OutPoint, unused);

    BOOST_CHECK(ntState2.getMerkleHash() == hash3);

    ntState2.flush();

    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash3);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState3(pclaimTrie, false);
    ntState3.insertClaimIntoTrie(std::string("test"), CClaimValue(tx1OutPoint, hash160, 50, 100, 200));
    BOOST_CHECK(ntState3.getMerkleHash() == hash4);
    ntState3.flush();
    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash4);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState4(pclaimTrie, false);
    ntState4.removeClaimFromTrie(std::string("abab"), tx6OutPoint, unused);
    BOOST_CHECK(ntState4.getMerkleHash() == hash2);
    ntState4.flush();
    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash2);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState5(pclaimTrie, false);
    ntState5.removeClaimFromTrie(std::string("test"), tx3OutPoint, unused);

    BOOST_CHECK(ntState5.getMerkleHash() == hash2);
    ntState5.flush();
    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash2);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState6(pclaimTrie, false);
    ntState6.insertClaimIntoTrie(std::string("test"), CClaimValue(tx3OutPoint, hash160, 50, 101, 201));

    BOOST_CHECK(ntState6.getMerkleHash() == hash2);
    ntState6.flush();
    BOOST_CHECK(!pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash2);
    BOOST_CHECK(pclaimTrie->checkConsistency());

    CClaimTrieCache ntState7(pclaimTrie, false);
    ntState7.removeClaimFromTrie(std::string("test"), tx3OutPoint, unused);
    ntState7.removeClaimFromTrie(std::string("test"), tx1OutPoint, unused);
    ntState7.removeClaimFromTrie(std::string("tes"), tx4OutPoint, unused);
    ntState7.removeClaimFromTrie(std::string("test2"), tx2OutPoint, unused);

    BOOST_CHECK(ntState7.getMerkleHash() == hash0);
    ntState7.flush();
    BOOST_CHECK(pclaimTrie->empty());
    BOOST_CHECK(pclaimTrie->getMerkleHash() == hash0);
    BOOST_CHECK(pclaimTrie->checkConsistency());
}

BOOST_AUTO_TEST_CASE(basic_insertion_info_test)
{
    // test basic claim insertions and that get methods retreives information properly
    BOOST_CHECK(pclaimTrie->empty());
    CClaimTrieCacheTest ctc(pclaimTrie);

    // create and insert claim
    CClaimValue unused;
    uint256 hash0(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));
    CMutableTransaction tx1 = BuildTransaction(hash0);
    uint160 claimId = ClaimIdHash(tx1.GetHash(), 0);
    COutPoint claimOutPoint(tx1.GetHash(), 0);
    CAmount amount(10);
    int height = 0;
    int validHeight = 0;
    CClaimValue claimVal(claimOutPoint, claimId, amount, height, validHeight);
    ctc.insertClaimIntoTrie("test", claimVal);

    // try getClaimsForName, getEffectiveAmountForClaim, getInfoForName
    claimsForNameType res = ctc.getClaimsForName("test");
    BOOST_CHECK(res.claims.size() == 1);
    BOOST_CHECK(res.claims[0] == claimVal);

    BOOST_CHECK_EQUAL(10, ctc.getEffectiveAmountForClaim("test", claimId));

    CClaimValue claim;
    BOOST_CHECK(ctc.getInfoForName("test", claim));
    BOOST_CHECK(claim == claimVal);

    // insert a support
    CAmount supportAmount(10);
    uint256 hash1(uint256S("0000000000000000000000000000000000000000000000000000000000000002"));
    CMutableTransaction tx2 = BuildTransaction(hash1);
    COutPoint supportOutPoint(tx2.GetHash(), 0);

    CSupportValue support(supportOutPoint, claimId, supportAmount, height, validHeight);
    ctc.insertSupportIntoMap("test", support, false);

    // try getEffectiveAmount
    BOOST_CHECK_EQUAL(20, ctc.getEffectiveAmountForClaim("test", claimId));
}

BOOST_AUTO_TEST_CASE(recursive_prune_test)
{
    CClaimTrieCacheTest cc(pclaimTrie);
    BOOST_CHECK_EQUAL(0, cc.cacheSize());

    COutPoint outpoint;
    uint160 claimId;
    CAmount amount(20);
    int height = 0;
    int validAtHeight = 0;
    CClaimValue test_claim(outpoint, claimId, amount, height, validAtHeight);

    CClaimTrieNode base_node;
    // base node has a claim, so it should not be pruned
    base_node.insertClaim(test_claim);

    // node 1 has a claim so it should not be pruned
    CClaimTrieNode node_1;
    const char c = 't';
    base_node.children[c] = &node_1;
    node_1.insertClaim(test_claim);
    // set this just to make sure we get the right CClaimTrieNode back
    node_1.nHeightOfLastTakeover = 10;

    //node 2 does not have a claim so it should be pruned
    // thus we should find pruned node 1 in cache
    CClaimTrieNode node_2;
    const char c_2 = 'e';
    node_1.children[c_2] = &node_2;

    cc.recursivePruneName(&base_node, 0, std::string("te"), NULL);
    BOOST_CHECK_EQUAL(1, cc.cacheSize());
    nodeCacheType::iterator it = cc.getCache(std::string("t"));
    BOOST_CHECK_EQUAL(10, it->second->nHeightOfLastTakeover);
    BOOST_CHECK_EQUAL(1, it->second->claims.size());
    BOOST_CHECK_EQUAL(0, it->second->children.size());
}


BOOST_AUTO_TEST_SUITE_END()
