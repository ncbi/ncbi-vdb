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

#include <memory/QuotaMemoryMgr.hpp>

#include <gtest/gtest.h>

#include <memory/RawMemoryBlock.hpp>
#include <memory/UniqueRawMemoryBlock.hpp>

#include "MemoryManagerItf_Test.hpp"
#include "TrackingMemoryManagerItf_Test.hpp"

using namespace std;
using namespace VDB3;

// QuotaMemoryMgr

// interface conformance

const QuotaMemoryMgr :: size_type Quota = 1000;

/**
 * MemoryManagerItf_Test requires the implementation class to have a default ctor.
 */
class TestQuotaMemoryMgr : public QuotaMemoryMgr
{
public:
    /**
    * Constructor.
    * @param p_allow_to_extend if true, will allow quota extension
    */
    TestQuotaMemoryMgr ( bool p_allow_to_extend = false )
    :   QuotaMemoryMgr ( Quota ),
        allow_to_extend ( p_allow_to_extend )
    {
    }

    virtual bool update_quota( size_type min_extension )
    {
        if ( allow_to_extend )
        {
            set_quota ( quota () + min_extension );
            return true;
        }
        return false;
    }

    bool allow_to_extend; ///< if true, allow quota extension
};

INSTANTIATE_TYPED_TEST_SUITE_P(QuotaMemoryMgr_ItfConformance, MemoryManagerItf_Test, TestQuotaMemoryMgr);
INSTANTIATE_TYPED_TEST_SUITE_P(QuotaMemoryMgr_TrackingItfConformance, TrackingMemoryManagerItf_Test, TestQuotaMemoryMgr);

TEST ( QuotaMemoryMgr, Instantiate_Default_quota )
{
    TestQuotaMemoryMgr mgr;
    ASSERT_EQ ( Quota, mgr . quota () );
    ASSERT_EQ ( Quota, mgr . total_free () );
    ASSERT_EQ ( Quota, mgr . max_free () );
    ASSERT_EQ ( 0, mgr . total_used () );
}

TEST ( QuotaMemoryMgr, Instantiate_alloc )
{
    TestQuotaMemoryMgr mgr;
    const QuotaMemoryMgr :: size_type BlockSize = 10;
    TestQuotaMemoryMgr :: pointer p = mgr . allocate ( BlockSize );
    ASSERT_EQ ( Quota, mgr . quota () );
    ASSERT_EQ ( Quota - BlockSize, mgr . total_free () );
    ASSERT_EQ ( Quota - BlockSize, mgr . max_free () );
    ASSERT_EQ ( BlockSize, mgr . total_used () );

    mgr.deallocate( p, BlockSize );
}

TEST ( QuotaMemoryMgr, Instantiate_dealloc )
{
    TestQuotaMemoryMgr mgr;
    const QuotaMemoryMgr :: size_type BlockSize = 10;
    TestQuotaMemoryMgr :: pointer p = mgr . allocate ( BlockSize );
    mgr.deallocate( p, BlockSize );
    ASSERT_EQ ( Quota, mgr . quota () );
    ASSERT_EQ ( Quota, mgr . total_free () );
    ASSERT_EQ ( Quota, mgr . max_free () );
    ASSERT_EQ ( 0, mgr . total_used () );
}

TEST ( QuotaMemoryMgr, Instantiate_realloc_grow )
{
    TestQuotaMemoryMgr mgr;
    const QuotaMemoryMgr :: size_type BlockSize = 10;
    const QuotaMemoryMgr :: size_type NewSize = 40;
    TestQuotaMemoryMgr :: pointer p = mgr . allocate ( BlockSize );
    p = mgr . reallocate ( p, NewSize );
    ASSERT_EQ ( Quota, mgr . quota () );
    ASSERT_EQ ( Quota - NewSize, mgr . total_free () );
    ASSERT_EQ ( Quota - NewSize, mgr . max_free () );
    ASSERT_EQ ( NewSize, mgr . total_used () );

    mgr.deallocate( p, NewSize );
}

TEST ( QuotaMemoryMgr, Instantiate_realloc_shrink )
{
    TestQuotaMemoryMgr mgr;
    const QuotaMemoryMgr :: size_type BlockSize = 100;
    const QuotaMemoryMgr :: size_type NewSize = 40;
    TestQuotaMemoryMgr :: pointer p = mgr . allocate ( BlockSize );
    p = mgr . reallocate ( p, NewSize );
    ASSERT_EQ ( Quota, mgr . quota () );
    ASSERT_EQ ( Quota - NewSize, mgr . total_free () );
    ASSERT_EQ ( Quota - NewSize, mgr . max_free () );
    ASSERT_EQ ( NewSize, mgr . total_used () );

    mgr.deallocate( p, NewSize );
}

TEST ( QuotaMemoryMgr, Instantiate_alloc_quota_exceeded )
{
    TestQuotaMemoryMgr mgr;
    ASSERT_THROW ( mgr . allocate ( Quota + 1 ), std :: bad_alloc );
}

TEST ( QuotaMemoryMgr, Instantiate_alloc_quota_exceeded_extended )
{
    TestQuotaMemoryMgr mgr (true ); // allow to extend quota
    TestQuotaMemoryMgr :: pointer p;
    const QuotaMemoryMgr :: size_type BlockSize = Quota + 1;
    ASSERT_NO_THROW ( p = mgr . allocate ( BlockSize ) );
    ASSERT_EQ ( BlockSize, mgr . quota () );

    mgr.deallocate( p, BlockSize );
}

TEST ( QuotaMemoryMgr, Instantiate_realloc_quota_exceeded )
{
    TestQuotaMemoryMgr mgr;
    const QuotaMemoryMgr :: size_type BlockSize = Quota;
    const QuotaMemoryMgr :: size_type NewSize = Quota + 1;
    TestQuotaMemoryMgr :: pointer p;
    ASSERT_NO_THROW ( p = mgr . allocate ( BlockSize ) );
    ASSERT_THROW ( mgr . reallocate ( p, NewSize ), std :: bad_alloc );

    mgr.deallocate( p, BlockSize );
}

TEST ( QuotaMemoryMgr, Instantiate_realloc_quota_exceeded_extended )
{
    TestQuotaMemoryMgr mgr (true ); // allow to extend quota
    const QuotaMemoryMgr :: size_type BlockSize = Quota;
    const QuotaMemoryMgr :: size_type NewSize = Quota + 1;
    TestQuotaMemoryMgr :: pointer p;
    ASSERT_NO_THROW ( p = mgr . allocate ( BlockSize ) );
    ASSERT_NO_THROW ( p = mgr . reallocate ( p, NewSize ) );

    mgr.deallocate( p, NewSize );
}

// functionality is not lost when using the VDB-facing (no tracking) API

TEST ( QuotaMemoryMgr, Alloc_NoTracking )
{
    auto mgr = make_shared < TestQuotaMemoryMgr > ();

    // block sizes are not tracked by the manager but quota is updated on allocation
    RawMemoryBlock rmb ( mgr, 1 );  // uses mgr -> allocateNoTracking()

    // is not tracked by mgr
    ASSERT_THROW( mgr -> getBlockSize( rmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type

    // quota updated
    ASSERT_EQ ( rmb . size(), mgr -> total_used () );
}

TEST ( QuotaMemoryMgr, Dealloc_NoTracking )
{
    auto mgr = make_shared < TestQuotaMemoryMgr > ();

    {
        RawMemoryBlock rmb ( mgr, 1 );  // uses mgr -> allocateNoTracking()
    }

    // quota restored
    ASSERT_EQ ( 0, mgr -> total_used () );
}

TEST ( QuotaMemoryMgr, Realloc_NoTracking )
{
    auto mgr = make_shared < TestQuotaMemoryMgr > ();

    // block sizes are not tracked by the manager but quota is updated on reallocation
    UniqueRawMemoryBlock rmb ( mgr, 2 ); // UniqueRawMemoryBlock is resizeable

    // is not tracked by mgr
    ASSERT_THROW( mgr -> getBlockSize( rmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type
    ASSERT_EQ ( rmb . size(), mgr -> total_used () );
    rmb . resize ( 100 );

    // quota updated
    ASSERT_EQ ( rmb . size(), mgr -> total_used () );
}
