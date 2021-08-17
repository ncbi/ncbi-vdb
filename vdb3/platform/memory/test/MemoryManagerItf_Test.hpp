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

#include <map>

#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>


// MemoryManagerItf interface conformance suite

template <typename T>
class MemoryManagerItf_Test : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(MemoryManagerItf_Test);

TYPED_TEST_P ( MemoryManagerItf_Test, Allocate_0_bytes )
{
    TypeParam pm;
    auto ptr = pm . allocate ( 0 );
    ASSERT_EQ ( nullptr, ptr );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Reallocate_NULL )
{
    TypeParam pm;
    auto ptr = pm . reallocate ( nullptr, 100 );
    ASSERT_NE ( nullptr, ptr );
    pm . deallocate ( ptr, 100 );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Reallocate_0_size )
{
    TypeParam pm;
    auto ptr = pm . allocate ( 10 );
    ptr = pm . reallocate ( ptr, 0 );
    ASSERT_EQ ( nullptr, ptr );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Reallocate_grow )
{
    TypeParam pm;
    auto ptr = pm . allocate ( 10 );
    ptr = pm . reallocate ( ptr, 100 );
    ASSERT_NE ( nullptr, ptr );
    pm . deallocate ( ptr, 100 );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Reallocate_shrink )
{
    TypeParam pm;
    auto ptr = pm . allocate ( 10 );
    ptr = pm . reallocate ( ptr, 1 );
    ASSERT_NE ( nullptr, ptr );
    pm . deallocate ( ptr, 1 );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Deallocate_NULL )
{
    TypeParam pm;
    pm . deallocate ( nullptr, 10 );
}

TYPED_TEST_P ( MemoryManagerItf_Test, Allocate_Deallocate )
{
    TypeParam pm;
    auto ptr = pm . allocate ( 1 );
    ASSERT_NE ( nullptr, ptr );
    pm . deallocate ( ptr, 1 );
}

// use as an allocator for STL containers

TYPED_TEST_P ( MemoryManagerItf_Test, STL_vector )
{   // use as an allocator for an STL vector
    TypeParam pm;
    typename TypeParam::template allocator < int > alloc ( pm );
    std::vector < int, decltype ( alloc ) > v ( alloc );
    v . push_back ( 1 );
    v . push_back ( 2 );
    v . push_back ( 3 );
    ASSERT_EQ( 3, v.size() );
    ASSERT_EQ( 1, v [ 0 ] );
    ASSERT_EQ( 2, v [ 1 ] );
    ASSERT_EQ( 3, v [ 2 ] );
}

TYPED_TEST_P ( MemoryManagerItf_Test, STL_map )
{   // use as an allocator for an STL map
    TypeParam pm;
    typename TypeParam::template allocator < std :: pair < const int, std :: string > > alloc ( pm );
    std::map < int, std :: string, std :: less < int >, decltype ( alloc ) > v ( alloc );
    v [ 1 ] = "uno";
    v [ 2 ] = "dos";
    v [ 3 ] = "tres";
    ASSERT_EQ( 3, v.size() );
    ASSERT_EQ( "uno", v [ 1 ] );
    ASSERT_EQ( "dos", v [ 2 ] );
    ASSERT_EQ( "tres", v [ 3 ] );
}

TYPED_TEST_P ( MemoryManagerItf_Test, STL_multimap )
{   // use as an allocator for an STL multimap
    TypeParam pm;
    typename TypeParam::template allocator < std :: pair < const int, std :: string > > alloc ( pm );
    typedef std::multimap < int, std :: string, std :: less < int >, decltype ( alloc ) > MultiMap;
    MultiMap v ( alloc );
    v . insert ( typename MultiMap :: value_type ( 1, "uno" ) );
    v . insert ( typename MultiMap :: value_type ( 1, "eins" ) );
    v . insert ( typename MultiMap :: value_type ( 2, "dos" ) );
    v . insert ( typename MultiMap :: value_type ( 2, "zwei" ) );
    v . insert ( typename MultiMap :: value_type ( 3, "drei" ) );
    ASSERT_EQ( 5, v.size() );
    typename MultiMap :: const_iterator it = v . begin ();
    ASSERT_EQ( 1, it -> first );
    ASSERT_EQ( "uno", it -> second );
    ++it;
    ASSERT_EQ( 1, it -> first );
    ASSERT_EQ( "eins", it -> second );
    ++it;
    ASSERT_EQ( 2, it -> first );
    ASSERT_EQ( "dos", it -> second );
    ++it;
    ASSERT_EQ( 2, it -> first );
    ASSERT_EQ( "zwei", it -> second );
    ++it;
    ASSERT_EQ( 3, it -> first );
    ASSERT_EQ( "drei", it -> second );
    ++it;
    ASSERT_EQ( v . end(), it );
}

//TODO: array, deque, forward_list, list, queue, priority_queue, set, multiset, stack,
// unordered_map, unordered_multimap, unordered_set, unordered_multiset, vector<bool>

REGISTER_TYPED_TEST_SUITE_P(MemoryManagerItf_Test
    , Allocate_0_bytes
    , Reallocate_NULL
    , Reallocate_0_size
    , Reallocate_grow
    , Reallocate_shrink
    , Deallocate_NULL
    , Allocate_Deallocate
    , STL_vector
    , STL_map
    , STL_multimap
    );

