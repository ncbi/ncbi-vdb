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

#include <memory/TypedMemoryBlock.hpp>

#include <memory/PrimordialMemoryMgr.hpp>
#include <memory/QuotaMemoryMgr.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace VDB3;

/**
 *  Test fixture class encapsulating PrimordialMemoryMgr.
 */
class TypedMemoryBlock_Fixture : public ::testing::Test
{
protected:
    MemoryMgr pm = make_shared < PrimordialMemoryMgr > ();   ///< the underlying memory manager
};

// TypedMemoryBlock

/**
 *  A user class for testing TypedMemoryBlock<>
 */
class C
{
public:
    /**
     * Constructor.
     * @param m1 some type
     * @param m2 another type
     */
    C(int m1, float m2) : mem1(m1), mem2(m2) {}

    /**
     * Copy constructor.
     * @param that source
     */
    C(const C& that) : mem1(that.mem1), mem2(that.mem2){}

    /**
     * Destructor.
     */
    ~C() { ++ dtor_called; }

    int mem1;   ///< data member
    float mem2; ///< another data member

    static uint32_t dtor_called;    ///< incremented with every call to the destructor
};

uint32_t C :: dtor_called = 0;

TEST_F ( TypedMemoryBlock_Fixture, Typed_Allocate_Deallocate_Size_Data )
{
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb ( pm, 1, 2.3f );
        ASSERT_EQ ( ( bytes_t ) sizeof ( C ), tmb . size() );
        ASSERT_EQ ( 1, tmb . data() . mem1 );
        ASSERT_EQ ( 2.3f, tmb . data() . mem2 );
    }
    ASSERT_EQ ( 1, C :: dtor_called );
}

TEST_F ( TypedMemoryBlock_Fixture, Typed_Copy )
{
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
        TypedMemoryBlock<C> tmb2 ( tmb1 ); // a new reference to C
        ASSERT_EQ ( tmb1 . data() . mem1, tmb2 . data() . mem1 );
        ASSERT_EQ ( tmb1 . data() . mem2, tmb2 . data() . mem2 );
        // same using conversion operator
        ASSERT_EQ ( ( ( C & ) tmb1 ) . mem1, ( ( C & ) tmb2 ) . mem1 );
        ASSERT_EQ ( ( ( C & ) tmb1 ) . mem2, ( ( C & ) tmb2 ) . mem2 );
    }
    ASSERT_EQ ( 1, C :: dtor_called ); // only 1 C existed
}

TEST_F ( TypedMemoryBlock_Fixture, Typed_Share )
{
    TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
    TypedMemoryBlock<C> tmb2 ( tmb1 );
    ( ( C & ) tmb1 ) . mem1 = 2;
    ASSERT_EQ ( tmb1 . data() . mem1, tmb2 . data() . mem1 );
}

TEST_F ( TypedMemoryBlock_Fixture, Typed_Clone )
{
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
        auto tmb2 = tmb1 . clone(); // a new C object
        ( ( C & ) tmb1 ) . mem1 = 2;
        ASSERT_EQ ( 1, tmb2 . data() . mem1 ); // did not change
    }
    ASSERT_EQ ( 2, C :: dtor_called ); // 2 Cs existed
}

TEST_F ( TypedMemoryBlock_Fixture, NoTracking )
{   // when using a size-tracking memory manager, tracking on this block is not done,
    // since it knows its size itself
    auto tmm = make_shared < TrackingMemoryManager > ( pm );
    MemoryMgr qmm = make_shared < QuotaMemoryMgr > ( tmm, 100000 );
    {
        TypedMemoryBlock<C> tmb ( qmm, 1, 2.3f );
        ASSERT_THROW( tmm -> getBlockSize( tmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type
    }
}