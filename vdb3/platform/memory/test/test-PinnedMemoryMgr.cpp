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

#include <memory/PinnedMemoryMgr.hpp>

#include <vector>

#include <gtest/gtest.h>

#include <memory/PrimordialMemoryMgr.hpp>
#include <memory/RawMemoryBlock.hpp>
#include <memory/UniqueRawMemoryBlock.hpp>

#include "MemoryManagerItf_Test.hpp"
#include "TrackingMemoryManagerItf_Test.hpp"

using namespace std;
using namespace VDB3;

// PinnedMemoryMgr

// inteface conformance
INSTANTIATE_TYPED_TEST_SUITE_P(PinnedMemoryMgr_ItfConformance, MemoryManagerItf_Test, PinnedMemoryMgr);
INSTANTIATE_TYPED_TEST_SUITE_P(PinnedMemoryMgr_TrackingItfConformance, TrackingMemoryManagerItf_Test, PinnedMemoryMgr);

TEST ( PinnedMemoryMgr, Instantiate )
{
    PinnedMemoryMgr mgr;
}

TEST ( PinnedMemoryMgr, Allocate )
{
    PinnedMemoryMgr mgr;
    MemoryManagerItf::pointer p = mgr.allocate(1);
    ASSERT_NE ( nullptr, p );
    mgr.deallocate(p, 1);
}

/**
* Fake locker class for testing. Keeps track of currently "pinned" blocks
*/
class TestLocker : public PinnedMemoryMgr :: MemoryLockerItf
{
public:
    TestLocker(){}
    virtual ~TestLocker () {}
    /**
     * Add an address range to the list of "pinned" ranges
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     */
    virtual void lock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        blocks [ ptr ] = bytes;
    }
    /**
     * Remove an address range from the list of "pinned" ranges
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     */
    virtual void unlock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        assert ( bytes == blocks [ ptr ] );
        blocks . erase ( ptr );
    }

    typedef std::map < MemoryManagerItf :: pointer, MemoryManagerItf :: size_type > Blocks; ///< block start -> block size
    Blocks blocks; ///< currently pinned blocks
};

TEST ( PinnedMemoryMgr, CustomLocker_Pin )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 1 );
    ASSERT_EQ ( 1, tl . blocks [ p ] );

    mgr . deallocate ( p, 1 );
}

TEST ( PinnedMemoryMgr, CustomLocker_Unpin )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 1 );

    mgr . deallocate ( p, 1 );
    ASSERT_EQ ( tl . blocks . end (), tl . blocks . find ( p ) );
}

TEST ( PinnedMemoryMgr, CustomLocker_Alloc_0_size )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 0 );
    ASSERT_EQ ( nullptr, p );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 10 );

    // should call unlock(p1, 1), lock(p2, 100)
    const MemoryManagerItf :: size_type NewSize = 1000;
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, NewSize );
    ASSERT_NE ( p1, p2 );
    ASSERT_EQ ( NewSize, tl . blocks [ p2 ] );
    ASSERT_EQ ( tl . blocks . end (), tl . blocks . find ( p1 ) );

    mgr . deallocate ( p2, NewSize );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_0_size )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 1 );
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, 0 );
    ASSERT_EQ ( nullptr, p2 );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

static PrimordialMemoryMgr primMgr;

/**
* Under valgrind, the default heap manager always moves the block on reallocate()
* In order to test scenarios when reallocaiton does not move the block, we will use this
* simple manager.
*/
class NoMoveReallocMgr : public TrackingMemoryManager
{ // never moves the block on reallocate
public:
    virtual pointer reallocate ( pointer ptr, size_type new_size )
    {   // do not move
        setBlockSize ( ptr, new_size );
        return ptr;
    }
};

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_same_size )
{
    TestLocker tl;
    auto nmm = make_shared < NoMoveReallocMgr > ();
    PinnedMemoryMgr mgr ( nmm, & tl );
    const MemoryManagerItf :: size_type Size = 100;
    MemoryManagerItf :: pointer p1 = mgr . allocate ( Size );
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, Size );
    ASSERT_EQ ( p1, p2 );
    ASSERT_EQ ( 1, tl . blocks . size () );

    mgr . deallocate ( p2, Size ); // or could be p1
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_shrink )
{
    TestLocker tl;
    auto nmm = make_shared < NoMoveReallocMgr > ();
    PinnedMemoryMgr mgr ( nmm, & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 100 );
    const MemoryManagerItf :: size_type NewSize = 99;
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, NewSize );
    ASSERT_EQ ( p1, p2 );
    ASSERT_EQ ( NewSize, tl . blocks [ p1 ] );

    mgr . deallocate ( p2, NewSize ); // or could be p1
    ASSERT_EQ ( 0, tl . blocks . size () );
}

// functionality is not lost when using the VDB-facing (no tracking) API

TEST ( PinnedMemoryMgr, Alloc_NoTracking )
{
    TestLocker tl;
    auto mgr = make_shared < PinnedMemoryMgr > ( & tl );

    // block sizes are not tracked by the manager but filling on allocation still occurs
    RawMemoryBlock rmb ( mgr, 1 );  // uses mgr -> allocateNoTracking()

    // is not tracked by mgr
    ASSERT_THROW( mgr -> getBlockSize( rmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type
    // is pinned
    ASSERT_EQ ( 1, tl . blocks [ const_cast<void*>( rmb.ptr() ) ] );
}

TEST ( PinnedMemoryMgr, Dealloc_NoTracking )
{
    TestLocker tl;
    auto mgr = make_shared < PinnedMemoryMgr > ( & tl );

    const byte_t * ptr;
    {   // block sizes are not tracked by the manager but trashing on deallocation still occurs
        RawMemoryBlock rmb ( mgr, 2 );
        ptr = rmb . data();
    }   // the destructor uses mgr -> deallocateUntracked()

    // is unpinned
    ASSERT_EQ ( tl . blocks. end (), tl . blocks . find ( MemoryManagerItf :: pointer ( ptr ) ) );
}

TEST ( PinnedMemoryMgr, Realloc_NoTracking )
{
    TestLocker tl;
    auto mgr = make_shared < PinnedMemoryMgr > ( & tl );

    // block sizes are not tracked by the manager but filling/trashing on reallocation still occurs
    UniqueRawMemoryBlock rmb ( mgr, 2 ); // UniqueRawMemoryBlock is resizeable
    const byte_t * p1 = rmb . data();

    const MemoryManagerItf :: size_type NewSize = 100;
    rmb . resize ( NewSize );

    const byte_t * p2 = rmb . data();

    // the block got moved
    ASSERT_NE ( p1, p2 );
    // the new block is pinned
    ASSERT_EQ ( NewSize, tl . blocks [ MemoryManagerItf :: pointer ( p2 ) ] );
    // the old block is unpinned
    ASSERT_EQ ( tl . blocks . end (), tl . blocks . find ( MemoryManagerItf :: pointer ( p1 ) ) );
}
