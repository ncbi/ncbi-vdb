// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
/*
* Exersize validRunFileNameExt(accession, file) with different arguments.
* It will return true if "file" is a valid file name for "accession".
*=============================================================================*/

#include <cmath>

#define _LIBRARY       1

#include "../../libs/vdb/database-cmn.c"
#include <kapp/args.h> /* Args */
#include <ktst/unit_test.hpp> /* ver_t */

#define ALL

class Fixture{
protected:
    String acc;
    String xNoqual;
    String file;
    Fixture(){
        CONST_STRING(&acc, "SRR123456");
        CONST_STRING(&xNoqual, ".sralite");
        VDBManagerSetQualityString(NULL, "RZ");
    }
};

TEST_SUITE(DatabaseCmnTestSuite);

#ifdef ALL
FIXTURE_TEST_CASE(FULL, Fixture){
    CONST_STRING(&file, "SRR123456.sra");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(NOT_FULL, Fixture) {
    CONST_STRING(&file, "SRR000001.sra");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(ZERO, Fixture) {
    CONST_STRING(&file, "SRR123456.sralite");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(NOT_ZERO, Fixture) {
    CONST_STRING(&file, "SRR000001.sralite");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(BAD_ZERO, Fixture) {
    CONST_STRING(&file, "SRR123456.sral");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(BAD, Fixture) {
    CONST_STRING(&file, "SRR123456.srn");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_FULL, Fixture) {
    CONST_STRING(&file, "SRR123456_dbGaP-0.sra");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_NOT_FULL, Fixture) {
    CONST_STRING(&file, "SRR000001_dbGaP-10.sra");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_ZERO, Fixture) {
    CONST_STRING(&file, "SRR123456_dbGaP-210.sralite");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_NOT_ZERO, Fixture) {
    CONST_STRING(&file, "SRR000001_dbGaP-3210.sralite");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_BAD_ZERO, Fixture) {
    CONST_STRING(&file, "SRR000001_dbGaP-3210.srali");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(GAP_BAD, Fixture) {
    CONST_STRING(&file, "SRR000001_dbGaP-3210.sralitn");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(BAD_GAP_FULL, Fixture) {
    CONST_STRING(&file, "SRR123456_dbgap-0.sra");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(BAD_NO_NUM_GAP_FULL, Fixture) {
    CONST_STRING(&file, "SRR123456_dbGaP-.sra");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
FIXTURE_TEST_CASE(BAD_NUM_GAP_FULL, Fixture) {
    CONST_STRING(&file, "SRR123456_dbGaP-X.sra");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
    VDBManagerSetQualityString(NULL, "ZR");
    REQUIRE(!validRunFileNameExt(&acc, &file, &xNoqual));
}
#endif

int main(int argc, char * argv[])
{
    return DatabaseCmnTestSuite(argc, argv);
}
