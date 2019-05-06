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

#include <ktst/unit_test.hpp> /* TEST_SUITE */
#include <kapp/main.h> /* KAppVersion */

#include <kfs/directory.h>
#include <vdb/manager.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <insdc/insdc.h>

/*****************************************************************************
    This test 
*****************************************************************************/

extern "C" {
    #define UNIT_TEST_FUNCTION 1
    #include "../../libs/sraxf/syn_quality.c"
}

ver_t CC KAppVersion( void ) { return 0; }
rc_t CC Usage( const Args * args ) { return 0; }
const char UsageDefaultName[] = "";
rc_t UsageSummary( const char * progname ) { return 0; }

TEST_SUITE( SynQualityTestSuite );

TEST_CASE( NoInput ) {
    REQUIRE(UnitTest_0Read_NoFilter() == 0);
}
TEST_CASE( OneRead_NoFilter ) {
    REQUIRE(UnitTest_1Read_NoFilter() == 0);
}
TEST_CASE( TwoRead_NoFilter ) {
    REQUIRE(UnitTest_2Read_NoFilter() == 0);
}
TEST_CASE( OneRead_Pass ) {
    REQUIRE(UnitTest_1Read_Pass() == 0);
}
TEST_CASE( OneRead_Fail ) {
    REQUIRE(UnitTest_1Read_Fail() == 0);
}
TEST_CASE( TwoRead_Pass ) {
    REQUIRE(UnitTest_2Read_Pass() == 0);
}
TEST_CASE( TwoRead_Fail ) {
    REQUIRE(UnitTest_2Read_Fail() == 0);
}

rc_t CC KMain ( int argc, char *argv [] )
{
    return SynQualityTestSuite( argc, argv );
}
