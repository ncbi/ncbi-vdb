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

#include <memory/RawMemoryBlock.hpp>

#include <memory/PrimordialMemoryMgr.hpp>
#include <memory/QuotaMemoryMgr.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace VDB3;

/**
 *  Test fixture class encapsulating PrimordialMemoryMgr.
 */
class RawMemoryBlock_Fixture : public ::testing::Test
{
protected:

    const bytes_t Size = 13; ///< common block size
    const byte_t Filler1 = byte_t ( 0x5a ); ///< a filler byte
    const byte_t Filler2 = byte_t ( 0x25 ); ///< another filler byte

    MemoryMgr pm = make_shared < PrimordialMemoryMgr > ();   ///< the underlying memory manager
};

// RawMemoryBlock

TEST_F ( RawMemoryBlock_Fixture, Raw_Allocate_Deallocate_Size )
{
    RawMemoryBlock rmb ( pm, Size );
    ASSERT_NE ( nullptr, rmb . data() );
    ASSERT_EQ ( Size, rmb . size() );
    // dtor will deallocate
}

TEST_F ( RawMemoryBlock_Fixture, Raw_Copy_Share_Fill )
{
    RawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1;
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 );
    ASSERT_EQ ( Filler1, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler1, rmb2.data() [ 1 ] );
}

TEST_F ( RawMemoryBlock_Fixture, Raw_Clone )
{
    RawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1 . clone();
    rmb2.fill ( Filler2 );
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 ); // does not affect rmb2:
    ASSERT_EQ ( Filler2, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler2, rmb2.data() [ 1 ] );
}

TEST_F ( RawMemoryBlock_Fixture, Raw_RefCount )
{
    RawMemoryBlock rmb1 ( pm, Size );
    ASSERT_EQ ( 1, rmb1 . refcount () );

    {
        auto rmb2 = rmb1;
        ASSERT_EQ ( 2, rmb1 . refcount () );
        ASSERT_EQ ( 2, rmb2 . refcount () );
    }
    ASSERT_EQ ( 1, rmb1 . refcount () );
}

TEST_F ( RawMemoryBlock_Fixture, STL )
{   // use as an element in an STL container
    RawMemoryBlock rmb1 ( pm, Size );
    RawMemoryBlock rmb2 ( pm, Size + 1 );
    {
        std::vector<RawMemoryBlock> v;
        v.push_back(rmb1);
        v.push_back(rmb2);
        ASSERT_EQ ( 2, rmb1 . refcount () );
        ASSERT_EQ ( 2, rmb2 . refcount () );
        ASSERT_EQ ( Size, v[0] . size () );
        ASSERT_EQ ( Size + 1, v[1] . size () );
    }
    ASSERT_EQ ( 1, rmb1 . refcount () );
    ASSERT_EQ ( 1, rmb2 . refcount () );
}

TEST_F ( RawMemoryBlock_Fixture, NoTracking )
{   // when using a size-tracking memory manager, tracking on this block is not done,
    // since it knows its size itself
    auto tmm = make_shared < TrackingMemoryManager > ( pm );
    MemoryMgr qmm = make_shared < QuotaMemoryMgr > ( tmm, 100000 );
    {
        RawMemoryBlock rmb ( qmm, Size );
        ASSERT_THROW( tmm -> getBlockSize( rmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type
        //TODO: verify quota updated
    }
}