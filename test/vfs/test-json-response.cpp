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

#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> // TEST_SUITE

#include "../../libs/vfs/json-response.h" /* Response4Make4 */

using std::string;

/*static rc_t validate ( const char * input ) {
    KJsonValue * root = NULL;
    rc_t rc = KJsonValueMake ( & root, input, NULL, 0 );
    KJsonValueWhack ( root );
    return rc;
}*/

TEST_CASE ( testIncomplete ) {
    Response4 * response = NULL;

    REQUIRE_RC_FAIL ( Response4Make4 ( & response, 0 ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":{}}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":[]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":[{}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response,
        "{\"sequence\":[{\"acc\":\"SRR000000\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{},\"acc\":\"SRR000000\"}]}" ) );
}

TEST_CASE(testinsufficient) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4Make4 ( & response,
       "{\"sequence\":[{\"status\":{\"code\":404},\"acc\":\"SRR000000\"}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );

    REQUIRE_RC_FAIL ( Response4Make4 ( & response,
       "{\"sequence\":[{\"status\":{\"code\":200},\"acc\":\"SRR000001\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"\"}]}"
    ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\"}]}"
    ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                                "\"link\":\"\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                                "\"link\":\"https://h\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                                "\"format\":\"\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                                "\"format\":\"sra\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                                "\"format\":\"sra\",\"link\":\"\"}]}" ) );
    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
      "\"format\":\"sra\",\"link\":\"http://\"}]}" ) );
}

TEST_CASE(testMin) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"run\","
      "\"format\":\"sra\",\"link\":\"http://h/\"}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );

    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\",\"itemClass\":\"bar\","
      "\"format\":\"sra\",\"link\":\"http://h/\"}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );

    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
     "{\"status\":{\"code\":200},\"acc\":\"SRR000001\","
      "\"link\":\"http://h/\"}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );
}

TEST_CASE(testFlat) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
   "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
   "\"acc\":\"SRR000001\",\"itemClass\":\"run\","
   "\"format\":\"sra\",\"link\":\"http://h/\",\"size\":3,"
   "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );

    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
   "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
   "\"acc\":\"SRR000001\",\"itemClass\":\"bar\","
   "\"format\":\"sra\",\"link\":\"http://h/\",\"size\":3,"
   "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );


    REQUIRE_RC      ( Response4Make4 ( & response, "{\"sequence\":["
   "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
   "\"acc\":\"SRR000001\",\"itemClass\":\"run\","
   "\"format\":\"sra\",\"link\":\"fasp://h/\",\"size\":3,"
   "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000}]}" ) );
    REQUIRE_RC      ( Response4Release ( response ) );
}

TEST_CASE(testAlternative) {
    Response4 * response = NULL;

    REQUIRE_RC_FAIL ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"run\",\"format\":\"sra\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
        "\"alternatives\":[{\"link\":\"\"      }]}]}" ) );


    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"run\",\"format\":\"sra\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
        "\"alternatives\":[{\"link\":\"http://h/\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"bar\",\"format\":\"sra\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
        "\"alternatives\":[{\"link\":\"http://h/\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"alternatives\":[{\"link\":\"http://h/\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testAlternatives) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"run\",\"format\":\"sra\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
   "\"alternatives\":[{\"link\":\"http://h/\"},{\"link\":\"fasp://h\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"bar\",\"format\":\"sra\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
   "\"alternatives\":[{\"link\":\"http://h/\"},{\"link\":\"fasp://h\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
   "\"alternatives\":[{\"link\":\"http://h/\"},{\"link\":\"fasp://h\"}]}]}" ) );
    REQUIRE_RC ( Response4Release ( response ) );


    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"run\",\"size\":3,"
        "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000,"
        "\"alternatives\":[{\"format\":\"sra\",\"link\":\"http://h/\"}]}]"
      ",\"timestamp\":1500000000}" ) );
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testGroup) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"run\","
        "\"group\":[{\"format\":\"sra\",\"link\":\"http://h/\",\"size\":3,"
        "\"md5\":\"00000000000000000000000000000000\",\"modDate\":1000000000},"
        "      {\"format\":\"vdbcache\",\"link\":\"http://h/v\",\"size\":2,"
        "\"md5\":\"11111111111111111111111111111111\",\"modDate\":1000000001}"
      "]}], \"timestamp\":1500000001}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"itemClass\":\"bar\","
        "\"group\":[{\"format\":\"sra\",\"link\":\"http://h/\",\"size\":3,"
        "\"md5\":\"00000000000000000000000000000000\",\"modDate\":1000000000},"
        "      {\"format\":\"vdbcache\",\"link\":\"http://h/v\",\"size\":2,"
        "\"md5\":\"11111111111111111111111111111111\",\"modDate\":1000000001}"
      "]}], \"timestamp\":1500000001}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000001\","
        "\"group\":[{\"format\":\"run\",\"link\":\"http://h/\",\"size\":3,"
        "\"md5\":\"00000000000000000000000000000000\",\"modDate\":1000000000},"
        "      {\"format\":\"vdbcache\",\"link\":\"http://h/v\",\"size\":2,"
        "\"md5\":\"11111111111111111111111111111111\",\"modDate\":1000000001}"
      "]}], \"timestamp\":1500000002}" ) );
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testSequence) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
         "\"acc\":\"SRR000001\",\"itemClass\":\"run\","
         "\"format\":\"sra\",\"link\":\"fasp://h/\",\"size\":3,"
         "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000},"
        "{\"status\":{\"code\":404,\"msg\":\"not found\"},\"acc\":\"SRR0000\"}]"
      ",\"timestamp\":1500000003}" ) );
    REQUIRE_RC ( Response4Release ( response ) );


    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
         "\"acc\":\"SRR000001\",\"itemClass\":\"run\","
         "\"format\":\"sra\",\"link\":\"fasp://h/\",\"size\":3,"
         "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000},"
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000002\","
          "\"itemClass\":\"run\",\"format\":\"sra\",\"link\":\"http://h/\"}]"
      ",\"timestamp\":1500000004}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
         "\"acc\":\"SRR000001\",\"itemClass\":\"foo\","
         "\"format\":\"sra\",\"link\":\"fasp://h/\",\"size\":3,"
         "\"md5\":\"9bde35fefa9d955f457e22d9be52bcd9\",\"modDate\":1000000000},"
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000002\","
          "\"itemClass\":\"bar\",\"format\":\"sra\",\"link\":\"http://h/\"}]"
      ",\"timestamp\":1500000004}" ) );
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response, "{\"sequence\":["
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
         "\"acc\":\"SRR000001\","
         "\"link\":\"fasp://h/\",\"size\":3},"
        "{\"status\":{\"code\":200,\"msg\":\"ok\"},\"acc\":\"SRR000002\","
          "\"link\":\"http://h/\"}]"
      ",\"timestamp\":1500000005}" ) );
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testSRP) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
            "\"acc\":\"SRP000002\",\"sequence\":[{"
                "\"acc\":\"SRR000001\",\"itemClass\":\"run\","
                "\"format\":\"sra\",\"link\":\"http://h/1\"},"
           "{    \"acc\":\"SRR000002\",\"itemClass\":\"run\","
                "\"format\":\"sra\",\"link\":\"http://h/2\"}]}]"
         ",\"timestamp\":1500000006}"));
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testGaP) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
                "\"acc\":\"SRR001000\",\"itemClass\":\"run\","
                "\"format\":\"sra\",\"link\":\"http://h/1\","
                "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"}]"
         ",\"timestamp\":1500000007}"));
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
                "\"id\":1,\"itemClass\":\"file\","
                "\"format\":\"file\",\"name\":\"f.txt\",\"link\":\"http://h/\","
                "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"}]"
         ",\"timestamp\":1500000008}"));
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
                "\"id\":1,\"itemClass\":\"file\","
                "\"format\":\"file\",\"name\":\"f.txt\",\"link\":\"http://h/\","
                "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"},"
      "{\"status\":{\"code\":403,\"msg\":\"access denied\"},\"id\":2}"
         "],\"timestamp\":1500000009}"));
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
               "\"id\":1,\"itemClass\":\"file\","
               "\"format\":\"file\",\"name\":\"1.txt\",\"link\":\"http://h/1\","
               "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"},"
                       "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
               "\"id\":2,\"itemClass\":\"file\","
               "\"format\":\"file\",\"name\":\"2.txt\",\"link\":\"http://h/2\","
               "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"}"
         "],\"timestamp\":1500000010}"));
    REQUIRE_RC ( Response4Release ( response ) );

    REQUIRE_RC ( Response4Make4 ( & response,
        "{\"sequence\":[{\"status\":{\"code\":200,\"msg\":\"ok\"},"
               "\"id\":1,\"itemClass\":\"file\","
               "\"format\":\"file\",\"name\":\"1.txt\",\"link\":\"http://h/1\","
               "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"},"
                       "{\"status\":{\"code\":200,\"msg\":\"ok\"},"
               "\"id\":2,\"itemClass\":\"file\","
               "\"format\":\"file\",\"name\":\"2.txt\",\"link\":\"http://h/2\","
               "\"tic\":\"0AB1C23D-E4F5-6A7B-C890-1D2345678E90\"}"
         "],\"timestamp\":1500000011}"));
    REQUIRE_RC ( Response4Release ( response ) );
}

TEST_CASE(testLive) {
    Response4 * response = NULL;

    REQUIRE_RC ( Response4Make4 ( & response,
"{"
	"\"version\": 4.0,"
	"\"sequence\": ["
	"	{"
	"		\"status\": {"
	"			\"code\": 200,"
	"			\"msg\": \"ok\""
	"		},"
	"		\"acc\": \"SRR053325\","
	"		\"format\": \"sra\","
	"		\"group\": ["
	"			{"
	"				\"itemClass\": \"run\","
	"				\"service\": \"ftp-ncbi\","
	"				\"md5\": \"f0a3970669cd5719c146fd79bce3b50b\","
	"				\"modDate\": 1327031309,"
	"				\"size\": 31681,"
	"				\"alternatives\": ["
	"					{"
	"						\"link\":"
   "\"https://sra-download.ncbi.nlm.nih.gov/traces/sra59/SRR/000052/SRR053325\""
	"					}"
	"				]"
	"			}"
	"		]"
	"	}"
	"],"
	"\"timestamp\": 1540239237"
"}"    ));
    REQUIRE_RC ( Response4Release ( response ) );

if(1);
  //REQUIRE_RC(validate(
}


TEST_SUITE ( TestResolver4 )

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
if ( 
0 ) assert ( ! KDbgSetString ( "VFS-JSON" ) );
    return TestResolver4 ( argc, argv );
    }
}
