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
*  warranties of performance, merchantability  or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#pragma once

#include <memory/TrackingMemoryManagerItf.hpp>

#include <gtest/gtest.h>

// MemoryManagerItf interface conformance suite

template <typename T>
class TrackingMemoryManagerItf_Test : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(TrackingMemoryManagerItf_Test);

const VDB3::TrackingMemoryManager :: size_type SmallSize = 2;
const VDB3::TrackingMemoryManager :: size_type BigSize = 222;

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize_NULL)
{
    TypeParam mgr;
    ASSERT_THROW ( mgr . getBlockSize ( nullptr ), std :: logic_error ); //TODO: use a VDB3 exception
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize)
{
    TypeParam mgr;
    typename TypeParam :: pointer p = mgr . allocate ( SmallSize );
    ASSERT_EQ ( SmallSize, mgr . getBlockSize ( p ) );

    mgr.deallocate ( p, SmallSize );
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize_postDeallocate)
{
    TypeParam mgr;
    typename TypeParam::pointer p = mgr . allocate ( SmallSize );
    mgr . deallocate ( p, SmallSize );
    ASSERT_THROW ( mgr . getBlockSize ( p ), std :: logic_error ); //TODO: use a VDB3 exception
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize_postReallocate_To_0)
{
    TypeParam mgr;
    typename TypeParam::pointer p1 = mgr . allocate ( SmallSize );
    typename TypeParam::pointer p2 = mgr . reallocate ( p1, 0 );
    ASSERT_EQ ( nullptr, p2 );
    // p1 is gone
    ASSERT_THROW ( mgr . getBlockSize ( p1 ), std :: logic_error ); //TODO: use a VDB3 exception
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize_postReallocate_ToGrow)
{
    TypeParam mgr;
    typename TypeParam::pointer p1 = mgr . allocate ( SmallSize );
    typename TypeParam::pointer p2 = mgr . reallocate ( p1, BigSize );
    ASSERT_THROW ( mgr . getBlockSize ( p1 ), std :: logic_error ); //TODO: use a VDB3 exception
    ASSERT_EQ ( BigSize, mgr . getBlockSize ( p2 ) );

    mgr . deallocate ( p2, BigSize );
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, getBlockSize_postReallocate_To_Shrink)
{
    TypeParam mgr;
    typename TypeParam::pointer p1 = mgr . allocate ( BigSize );
    typename TypeParam::pointer p2 = mgr . reallocate ( p1, SmallSize );
    ASSERT_EQ ( SmallSize, mgr . getBlockSize ( p2 ) );

    mgr . deallocate ( p2, SmallSize );
}

// Non-tracking operations

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, allocateUntracked_NoTracking)
{
    TypeParam mgr;
    void * p = mgr . allocateUntracked ( SmallSize );
    ASSERT_THROW ( mgr . getBlockSize ( p ), std :: logic_error ); //TODO: use a VDB3 exception

    mgr.deallocateUntracked ( p, SmallSize );
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, allocateUntracked_reallocate)
{   // reallocate() works only on tracked blocks, use reallocateUntracked
    TypeParam mgr;
    void * p = mgr . allocateUntracked ( SmallSize );
    ASSERT_THROW ( mgr . reallocate ( p, SmallSize + 1 ), std :: logic_error ); //TODO: use a VDB3 exception

    mgr.deallocateUntracked ( p, SmallSize );
}

TYPED_TEST_P ( TrackingMemoryManagerItf_Test, allocateUntracked_deallocate)
{   // deallocate() works only on tracked blocks, use deallocateUntracked
    TypeParam mgr;
    void * p = mgr . allocateUntracked ( SmallSize );

    // deallocate() is noexcept, so its failure goes unnoticed
    mgr . deallocate ( p, SmallSize );

    // still need to call deallocateUntracked() to avoid memory leak
    mgr.deallocateUntracked ( p, SmallSize );
}

REGISTER_TYPED_TEST_SUITE_P(TrackingMemoryManagerItf_Test
    , getBlockSize_NULL
    , getBlockSize
    , getBlockSize_postDeallocate
    , getBlockSize_postReallocate_To_0
    , getBlockSize_postReallocate_ToGrow
    , getBlockSize_postReallocate_To_Shrink
    , allocateUntracked_NoTracking
    , allocateUntracked_reallocate
    , allocateUntracked_deallocate
);

