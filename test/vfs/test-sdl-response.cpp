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

//#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> // TEST_SUITE

#include "../../libs/vfs/json-response.h" /* Response4MakeSdl */

TEST_SUITE(TestSdlResolver)

/*static rc_t validate ( const char * input ) {
    KJsonValue * root = NULL;
    rc_t rc = KJsonValueMake ( & root, input, NULL, 0 );
    KJsonValueWhack ( root );
    return rc;
}*/

TEST_CASE(testExample) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4MakeSdl ( & response,
"{"
    "\"version\": \"2\","
    "\"result\": ["
        "{"
            "\"bundle\": \"SRR850901\","
            "\"status\": 200,"
            "\"msg\": \"ok\","
            "\"files\": ["
                "{"
                    "\"object\": \"srapub|SRR850901\","
                    "\"type\": \"sra\","
                    "\"name\": \"SRR850901\","
                    "\"size\": 323741972,"
                    "\"md5\": \"5e213b2319bd1af17c47120ee8b16dbc\","
                    "\"modificationDate\": \"2016-11-19T07:35:20Z\","
                    "\"link\": \"https://sra-download.ncbi.nlm.nih.gov/traces/sra3/SRR/000830/SRR850901\","
                    "\"service\": \"sra-ncbi\","
                    "\"region\": \"public\""
                "},"
                "{"
                    "\"object\": \"srapub|SRR850901\","
                    "\"type\": \"vdbcache\","
                    "\"name\": \"SRR850901.vdbcache\","
                    "\"size\": 17615526,"
                    "\"md5\": \"2eb204cedf5eefe45819c228817ee466\","
                    "\"modificationDate\": \"2016-02-08T16:31:35Z\","
                    "\"link\": \"https://sra-download.ncbi.nlm.nih.gov/traces/sra11/SRR/000830/SRR850901.vdbcache\","
                    "\"service\": \"sra-ncbi\","
                    "\"region\": \"public\""
                "}"
            "]"
        "}"
    "]"
"}"
) );
    REQUIRE_RC      ( Response4Release ( response ) );
}

TEST_SUITE ( TestResolverSdl )

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
//if ( 0 ) assert ( ! KDbgSetString ( "VFS-JSON" ) );
    return TestSdlResolver( argc, argv );
    }
}
