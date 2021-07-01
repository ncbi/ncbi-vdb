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

#include "../goodbye.cpp"

#include <iostream>

#include <gtest/gtest.h>

using namespace std;

// Fixture-less tests

TEST(Goodbye, Fatal)
{
    ASSERT_EQ( "Goodbye", NCBI::VDB3::GoodbyeMsg() );
}

TEST(Goodbye, nonFatal)
{
    // uncomment the next line if you want to see non-fatal asseretions in action
    // EXPECT_TRUE(false); // reoports non-fatal failure and continues the function
    ASSERT_EQ( "Goodbye", NCBI::VDB3::GoodbyeMsg() );
}

// Tests with fixtures

///
/// Googletest fixture classes are derived from \::testing\::Test
///
class Fixture : public ::testing::Test
{
 protected:
    //! constructor
    Fixture()
    : m_value ( 0 )
    {
    }

    //! SetUp
    void SetUp() override
    {
        m_value = 1;
    }
    //! TearDown
    void TearDown() override
    {
        m_value = 2;
    }

    int m_value; //!< fixture member
};

TEST_F(Fixture, Simple)
{
    EXPECT_EQ( 1, m_value );
}

// main

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
