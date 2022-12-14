/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* Unit tests for klib/btree interface
*/

#include <ktst/unit_test.hpp>

#include <klib/btree.h>
#include <kfs/pagefile.h>
#include <kfs/directory.h>
#include <kfs/file.h>

#include <fstream>
#include <sstream>
#include <set>
#include <memory>

#undef LIB_EXPORT
#define LIB_EXPORT
#include "../libs/klib/btree.c"

using namespace std;

TEST_SUITE(BtreeTestSuite);

struct Pager {
    KPageFile *pager;
    rc_t rc;
};

static void const *PagerAlloc(Pager *self, uint32_t *newid)
{
    KPage *page = NULL;
    self->rc = KPageFileAlloc(self->pager, &page, newid);
    return (void const *)page;
}

static void const *PagerUse(Pager *self, uint32_t pageid)
{
    KPage *page = NULL;
    self->rc = KPageFileGet(self->pager, &page, pageid);
    return (void const *)page;
}

static void const *PagerAccess(Pager *self, void const *page)
{
    void const *mem = NULL;
    self->rc = KPageAccessRead((KPage const *)page, &mem, NULL);
    return mem;
}

static void *PagerUpdate(Pager *self, void const *page)
{
    void *mem = NULL;
    self->rc = KPageAccessUpdate((KPage *)page, &mem, NULL);
    return mem;
}

static void PagerUnuse(Pager *self, void const *page)
{
    KPageRelease((KPage const *)page);
}

static Pager_vt const KPageFile_vt = {
    PagerAlloc,
    PagerUse,
    PagerAccess,
    PagerUpdate,
    PagerUnuse
};

const static string DataDir = "./data/";

class BtreeFixture
{
public:
    BtreeFixture()
    : backing(nullptr)
    {
        THROW_ON_RC( KDirectoryNativeDir_v1 ( &dir ) );
    }
    void Setup( const char* test_name)
    {
        backingFileName = DataDir + test_name + ".backing";
        remove( backingFileName.c_str() );
        THROW_ON_RC( KDirectoryCreateFile_v1 ( dir, &backing, true, 0664, kcmCreate, "%s", backingFileName.c_str() ) );
        THROW_ON_RC( KPageFileMakeUpdate (
                        &pager.pager,
                        backing,
                        100*1024*1024, // 0 = no caching
                        false // no write_through
                        )
        );
    }
    ~BtreeFixture()
    {
        if ( m_node != nullptr )
        {
            KPageFile_vt.unuse( &pager, m_node );
        }

        KPageFileRelease ( pager.pager );
        KFileRelease ( backing );
        KDirectoryRelease ( dir );

        remove( backingFileName.c_str() );
    }

    bool TreeEntry( const string & key, uint32_t id, bool validate = true )
    {   // true if not found and inserted; false if found
        if ( pb.get() == nullptr )
        {
            pb.reset( MakeEntry( string( key.c_str(), key.size() ) ) );
        }
        bool was_inserted;
        rc_t rc = BTreeEntry ( & pb -> root, &pager, &KPageFile_vt, &id, &was_inserted, key.c_str(), key.size() );
        if ( rc )
        {
            ostringstream s;
            s << "BTreeEntry() returned rc=" << rc;
            throw logic_error( s.str() );
        }

        if ( validate )
        {
            Validate();
        }

        return was_inserted;
    }

    static void visit ( const void *key, size_t key_size, uint32_t id, void *data )
    {
        auto iter = (set<string>::const_iterator*)data;

        if ( memcmp( (*iter)->c_str(), key, key_size ) != 0 )
        {
            ostringstream s;
            s << string( "wrong order at id= " )<< id;
            throw logic_error( s.str() );
        }

        ++(*iter);
    }

    void Populate( bool verbose = false, bool validate = true )
    {
        uint32_t id = 0;
        for( auto i = in.begin(); i != in.end(); ++i )
        {
            ++id;
            if (verbose)
                cout << endl << "***Inserting id=" << id << ", size=" << i->size() << ", " << (int)(unsigned char)(*i)[0] << endl;
            THROW_ON_FALSE( TreeEntry ( *i, id, validate ) );
            if (verbose)
                Print();
        }
    }

    void Validate()
    {
        btree_validate ( pb -> root, &pager, &KPageFile_vt );
    }

    void Print()
    {
        btree_printf ( pb -> root, &pager, &KPageFile_vt );
    }

    EntryData* MakeEntry( const string & key )
    {
        EntryData * ret = new EntryData;
        ret -> pager = & pager;
        ret -> vt = & KPageFile_vt;
        ret -> root = pb.get() == nullptr ? 0 : pb -> root;
        ret -> id = & m_id;
        ret -> key = key.c_str();
        ret -> key_size = key.size();
        ret -> was_inserted = false;
        return ret;
    }

    LeafNode* MakeLeaf( uint32_t * rootId = nullptr )
    {
        uint32_t new_id = 0;
        const void * page = pb->vt->alloc(pb->pager, &new_id);
        THROW_ON_FALSE( page != nullptr );
        if ( rootId != nullptr )
        {
            * rootId = new_id << 1;
        }
        LeafNode *ret = (LeafNode *)pb->vt->access(pb->pager, page);
        THROW_ON_FALSE( ret != nullptr );
        pb->vt->unuse( pb->pager, page );
        return ret;
    }

    bool CheckCount( const LeafNode & node, unsigned int count, unsigned int key_length = 1 )
    { // for testing, assume all keys have teh same length
        if ( node.count != count )
        {
            cerr << "CheckCount/count: expected " << count << ", actual " << node.count << endl;
            return false;
        }
        if ( count * ( key_length + sizeof(uint32_t) ) != node.key_bytes )
        {
            cerr << "CheckCount/key_length: expected " << key_length << ", actual " << count * ( key_length + sizeof(uint32_t) ) << endl;
            return false;
        }
        return true;
    }

    bool CheckPrefix( const LeafNode & node, const string & prefix = string() )
    {
        if ( node.key_prefix_len != prefix.size() )
        {
            return false;
        }
        if ( node.key_prefix_len == 0 )
        {
            return true;
        }
        return string( &((const char *)&node)[node.key_prefix], node.key_prefix_len ) == prefix;
    }

    bool CheckWindow( const LeafNode & node, unsigned int index, uint16_t lower, uint16_t upper )
    {
        if ( node.win[index].lower == lower && node.win[index].upper == upper )
        {
            return true;
        }
        cerr << "CheckWindow[" << index << "]: expected " << lower << "-" << upper <<", actual " << node.win[index].lower << "-" << node.win[index].upper << endl;
        return false;
    }

    bool CheckKey( const LeafNode & node, unsigned int index, const string & key )
    {
        string actual ( &((const char *)&node)[node.ord[index].key], node.ord[index].ksize );
        if ( actual != key )
        {
            cerr << "CheckKey[" << index << "]: expected " << key << ", actual " << actual << " " << (int)((const char *)&node)[node.ord[index].key] << " " << node.ord[index].ksize << endl;
            return false;
        }
        return true;
    }

protected:
    string backingFileName;

    KDirectory_v1 *dir;
    KFile *backing;
    Pager pager;

    vector<string> in;
    uint32_t m_id;
    unique_ptr<EntryData> pb;
    LeafNode* m_node = nullptr; // memory owned by pager
};

FIXTURE_TEST_CASE( InsertIntoEmpty, BtreeFixture )
{
    Setup( GetName() );

    pb.reset( MakeEntry( "A" ) );
    m_node = MakeLeaf( &pb -> root );

    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 1 ) );
    REQUIRE( CheckKey( * m_node, 0, "A" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'A'+1, 1, 1 ));
}
FIXTURE_TEST_CASE( InsertWithoutSplit_Last, BtreeFixture )
{
    Setup( GetName() );

    pb.reset( MakeEntry( "A" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );

    pb.reset( MakeEntry( "B" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 2 ) );

    REQUIRE( CheckKey( * m_node, 0, "A" ) );
    REQUIRE( CheckKey( * m_node, 1, "B" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'B', 1, 2 ));
    REQUIRE( CheckWindow( * m_node, 'B'+1, 2, 2 ));
}

FIXTURE_TEST_CASE( InsertWithoutSplit_First, BtreeFixture )
{
    Setup( GetName() );

    pb.reset( MakeEntry( "B" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );

    pb.reset( MakeEntry( "A" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 2 ) );

    REQUIRE( CheckKey( * m_node, 0, "A" ) );
    REQUIRE( CheckKey( * m_node, 1, "B" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'B', 1, 2 ));
    REQUIRE( CheckWindow( * m_node, 'B'+1, 2, 2 ));
}

FIXTURE_TEST_CASE( InsertWithoutSplit_Middle, BtreeFixture )
{
    Setup( GetName() );

    pb.reset( MakeEntry( "A" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );

    pb.reset( MakeEntry( "C" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );

    pb.reset( MakeEntry( "B" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 3 ) );

    REQUIRE( CheckKey( * m_node, 0, "A" ) );
    REQUIRE( CheckKey( * m_node, 1, "B" ) );
    REQUIRE( CheckKey( * m_node, 2, "C" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'B', 1, 2 ));
    REQUIRE( CheckWindow( * m_node, 'C', 2, 3 ));
    REQUIRE( CheckWindow( * m_node, 'C'+1, 3, 3 ));
}

// API
FIXTURE_TEST_CASE( Basic_Insert, BtreeFixture )
{
    Setup( GetName() );
    in.push_back( "A" );
    Populate();
}
FIXTURE_TEST_CASE( Simple_Prefix, BtreeFixture )
{
    Setup( GetName() );
    in.push_back( "A" );
    in.push_back( "AAA" );
    in.push_back( "AA" );
    Populate();
}

// key size limit
FIXTURE_TEST_CASE( KeyTooLong, BtreeFixture )
{   // leaf(A) + B -> branch(B, left=(A), right=()) VDB-5026
    Setup( GetName() );
    string key (11000, 'A');
    pb.reset( MakeEntry( string( key.c_str(), key.size() ) ) );
    bool was_inserted;
    REQUIRE_RC_FAIL( BTreeEntry ( & pb -> root, &pager, &KPageFile_vt, &m_id, &was_inserted, key.c_str(), key.size() ) );
}

FIXTURE_TEST_CASE( Split_leaf, BtreeFixture )
{   // leaf(A) + C -> leaf( A C )
    // + D -> leaf( A C  D )
    // + B -> branch(C, left=(A B), right=(D) )
    Setup( GetName() );
    in.push_back( string(8000, 'A' ) );
    in.push_back( string(9000, 'C' ) );
    in.push_back( string(10000, 'D' ) );
    in.push_back( string(10000,  'B' ) );
    Populate();
    //Print();
}

FIXTURE_TEST_CASE( Split_LeftOfMedian, BtreeFixture )
{   // leaf(B,C) + A -> split(B, left=(A), right=(C))
    Setup( GetName() );

    pb.reset( MakeEntry( "B" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );

    pb.reset( MakeEntry( "C" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );

    Split split;
    SplitInit ( & split );
    LeafNode *newNode = MakeLeaf( & split.right);

    pb.reset( MakeEntry( "A" ) );

    REQUIRE_RC( split_leaf ( pb.get(), m_node, newNode, 0, & split) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 1 ) );
    REQUIRE( CheckKey( * m_node, 0, "A" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'A'+1, 1, 1 ));

    REQUIRE( CheckPrefix( * newNode ) );
    REQUIRE( CheckCount( * newNode, 1 ) );
    REQUIRE( CheckKey( * newNode, 0, "C" ) );
    REQUIRE( CheckWindow( * newNode, 'C'-1, 0, 0 ));
    REQUIRE( CheckWindow( * newNode, 'C', 0, 1 ));
    REQUIRE( CheckWindow( * newNode, 'C'+1, 1, 1 ));

    REQUIRE_EQ( string("B"), string((const char*)split.key, split.ksize));
    REQUIRE_EQ( 0, (int)split.left );
    REQUIRE_EQ( 4, (int)split.right );
}

FIXTURE_TEST_CASE( Split_RightOfMedian, BtreeFixture )
{   // leaf(A,B,C,E) + D -> split(C, left=(A,B), right=(D,E))
    Setup( GetName() );

    pb.reset( MakeEntry( "A" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );
    pb.reset( MakeEntry( "B" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );
    pb.reset( MakeEntry( "C" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 2 ) );
    pb.reset( MakeEntry( "E" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 3 ) );

    Split split;
    SplitInit ( & split );
    LeafNode *newNode = MakeLeaf( & split.right);

    pb.reset( MakeEntry( "D" ) );

    REQUIRE_RC( split_leaf ( pb.get(), m_node, newNode, 3, & split) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 2 ) );
    REQUIRE( CheckKey( * m_node, 0, "A" ) );
    REQUIRE( CheckKey( * m_node, 1, "B" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'B', 1, 2 ));
    REQUIRE( CheckWindow( * m_node, 'B'+1, 2, 2 ));

    REQUIRE( CheckPrefix( * newNode ) );
    REQUIRE( CheckCount( * newNode, 2 ) );

    REQUIRE( CheckKey( * newNode, 0, "D" ) );
    REQUIRE( CheckKey( * newNode, 1, "E" ) );

    REQUIRE( CheckWindow( * newNode, 'D'-1, 0, 0 ));
    REQUIRE( CheckWindow( * newNode, 'D', 0, 1 ));
    REQUIRE( CheckWindow( * newNode, 'E', 1, 2 ));
    REQUIRE( CheckWindow( * newNode, 'E'+1, 2, 2 ));

    REQUIRE_EQ( string("C"), string((const char*)split.key, split.ksize));
    REQUIRE_EQ( 0, (int)split.left );
    REQUIRE_EQ( 4, (int)split.right );
}

FIXTURE_TEST_CASE( Split_Median, BtreeFixture )
{   // leaf(A,B,D,E) + C -> split(D, left=(A,B,C), right=(E))
    Setup( GetName() );

    pb.reset( MakeEntry( "A" ) );
    m_node = MakeLeaf( &pb -> root );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 0 ) );
    pb.reset( MakeEntry( "B" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 1 ) );
    pb.reset( MakeEntry( "D" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 2 ) );
    pb.reset( MakeEntry( "E" ) );
    REQUIRE_RC( leaf_insert( pb.get(), m_node, 3 ) );

    Split split;
    SplitInit ( & split );
    LeafNode *newNode = MakeLeaf( & split.right);

    pb.reset( MakeEntry( "C" ) );

    REQUIRE_RC( split_leaf ( pb.get(), m_node, newNode, 2, & split) );
    Validate();

    REQUIRE( CheckPrefix( * m_node ) );

    REQUIRE( CheckCount( * m_node, 3 ) );
    REQUIRE( CheckKey( * m_node, 0, "A" ) );
    REQUIRE( CheckKey( * m_node, 1, "B" ) );
    REQUIRE( CheckKey( * m_node, 2, "C" ) );

    REQUIRE( CheckWindow( * m_node, 'A'-1, 0, 0 ));
    REQUIRE( CheckWindow( * m_node, 'A', 0, 1 ));
    REQUIRE( CheckWindow( * m_node, 'B', 1, 2 ));
    REQUIRE( CheckWindow( * m_node, 'C', 2, 3 ));
    REQUIRE( CheckWindow( * m_node, 'C'+1, 3, 3 ));

    REQUIRE( CheckPrefix( * newNode ) );
    REQUIRE( CheckCount( * newNode, 1 ) );

    REQUIRE( CheckKey( * newNode, 0, "E" ) );

    REQUIRE( CheckWindow( * newNode, 'E'-1, 0, 0 ));
    REQUIRE( CheckWindow( * newNode, 'E', 0, 1 ));
    REQUIRE( CheckWindow( * newNode, 'E'+1, 1, 1 ));

    REQUIRE_EQ( string("D"), string((const char*)split.key, split.ksize));
    REQUIRE_EQ( 0, (int)split.left );
    REQUIRE_EQ( 4, (int)split.right );
}

//TODO: prefixes

FIXTURE_TEST_CASE( Insert, BtreeFixture )
{
    Setup( GetName() );

    uint32_t id = 0;
    REQUIRE( TreeEntry ( string("1"), id ) );
    Validate();

    REQUIRE_EQ( 0u, id ); // not changed
}

FIXTURE_TEST_CASE( Find, BtreeFixture )
{
    Setup( GetName() );

    uint32_t id = 1;
    const char *key = "1";
    size_t key_size = strlen(key);
    REQUIRE( TreeEntry ( string(key), id ) );
    Validate();

    REQUIRE_RC( BTreeFind ( pb -> root, &pager, &KPageFile_vt, &id, key, key_size ) );
    REQUIRE_EQ( 1u, id ); // found
}

//TODO: ForEach, both directions

FIXTURE_TEST_CASE( BTree_insert, BtreeFixture )
{   // VDB-4959
    std::ifstream file("./btree_keys.bin", std::ios::binary);
    file.unsetf(std::ios::skipws);

    Setup( GetName() );

    uint32_t id = 0;
    while ( file )
    {
        size_t size;
        file.read((char*)&size, sizeof(size));
        if ( ! file ) break;
        char * buf = new char[size];
        file.read(buf, (streamsize)size);
        if ( ! file ) break;

        ++id;
//cout << "inserting " << size << endl;
        REQUIRE( TreeEntry ( string(  buf, size ), id, false ) );

//cout << "root=" << root << ",size=" << size << ", id=" << id << endl;

        delete [] buf;
    }

    Validate();
}


// FIXTURE_TEST_CASE( ConstraintViolation, BtreeFixture )
// {
//     Setup( GetName() );

//     uint32_t id = 0;
//     vector<string> in;
//     in.push_back( string(10886, 103 ) ); //'g'
//     in.push_back( string(16915, 105 ) ); //'i'
//     in.push_back( string(18335, 81) ); //'Q'
//     in.push_back( string(492, 74) ); //'J'
//     in.push_back( string(1421, 41) );
//     in.push_back( string(10027, 186) );
//     in.push_back( string(59, 242) );
//     in.push_back( string(13926, 227) );

//     in.push_back( string(3426, 124) );

//     in.push_back( string(15736, 84) ); // 'T' -- this does not fit into the same 32K page as it neighbor 'Q', triggering a constraint violation rc, see btree.c:leaf_insert()

//     in.push_back( string(15368, 27) );
//     in.push_back( string(16429, 231) );
//     in.push_back( string(1530, 118) );
//     in.push_back( string(5123, 46) );
//     in.push_back( string(3135, 51) );
//     in.push_back( string(19802, 201) );

//     for( auto i = in.begin(); i != in.end(); ++i )
//     {
//         ++id;
//         cout << endl << "***Inserting id=" << id << ", size=" << i->size() << ", " << (int)(unsigned char)(*i)[0] << endl;
//         REQUIRE( TreeEntry ( *i, id ) );
//     }
//     Validate();
// }

FIXTURE_TEST_CASE( BTree_randomInserts, BtreeFixture )
{
    Setup( GetName() );

    uint32_t id = 0;
    for( auto i = 0u; i != 1000000; ++i )
    {   // key sizes 1..1024 are taken from sra-sort ( they are not enforced but longer keys
        // are likelier to trigger a constraint violation error in btree.c:leaf_insert() )
        string s = string( 1 + (size_t)rand() % 1024, rand() % 256 );
        id = i +1;
        //cout << endl << "***Inserting id=" << id << ", size=" << s.size() << ", " << (int)(unsigned char)s[0] << endl;
        TreeEntry ( s, id, false );
    }
    Validate();
}

FIXTURE_TEST_CASE( LotsaFixedSizeInserts, BtreeFixture )
{
    Setup( GetName() );

    for( auto i = 0u; i < 375000; ++i )
    {
        int key = rand();
        // does not have to return "true" as duplicates are allowed
        TreeEntry ( string( (char *)&key, sizeof(key) ), i, false );
    }
    Validate();
}

//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-btree";

rc_t CC KMain(int argc, char* argv[])
{
    KConfigDisableUserSettings();
    rc_t rc = BtreeTestSuite(argc, argv);
    return rc;
}
}
