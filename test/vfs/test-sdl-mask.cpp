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
* ==============================================================================
*/

#include <klib/data-buffer.h> // KDataBuffer
#include <klib/out.h> // OUTMSG
#include <ktst/unit_test.hpp> // TEST_SUITE
#include "../../libs/vfs/services-priv.h" // MaskSdlResponse

using std::string;

TEST_SUITE(TestSdlMaskSuite)

class Fixture {
protected:
    KDataBuffer buf;

    Fixture() {
        rc_t rc(KDataBufferMakeBytes(&buf, 0));
        if (rc != 0)
            throw rc;
    }

    ~Fixture() { KDataBufferWhack(&buf); }

    bool Compare(string exp, const string& tail = "") {
        exp += tail;
        string a((char*)buf.base);
        if (a != exp)
            OUTMSG(("EXP: '%s'\nACT: '%s'\n", exp.c_str(), a.c_str()));
        return a == exp;
    }
};

/* short request parameter is not masked */
FIXTURE_TEST_CASE(TestReq, Fixture) {
    String s;
    CONST_STRING(&s,
        "accept-alternate-locations=can-pay:s3,location-type:location-type,zqa:zra"
    );

    REQUIRE_RC_FAIL(MaskSdlRequestArg(nullptr, nullptr));
    REQUIRE_RC_FAIL(MaskSdlRequestArg(&s, nullptr));
    REQUIRE_RC_FAIL(MaskSdlRequestArg(nullptr, &buf));

    REQUIRE_RC(MaskSdlRequestArg(&s, &buf));
    REQUIRE(Compare(s.addr, "&"));
}

/* long request parameter is masked */
FIXTURE_TEST_CASE(TestLongReq, Fixture) {
    String s;
    CONST_STRING(&s, "accept-alternate-locations=can-pay:s3,location-type:locat"
        "ion-type,zqa:zra,1234567890QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfg");
    REQUIRE_RC(MaskSdlRequestArg(&s, &buf));
    REQUIRE(Compare("accept-alternate-locations=can-pay:s3,location-type:locat"
        "ion-type,zqa:zra,1234567890QWERTYUIOPASDFG**************************",
        "&"));
}

/* error response, no URLs, not masked */
FIXTURE_TEST_CASE(TestRespError, Fixture) {
    string s(
        "{"
        "\"status\": 500,"
        "\"message\" : \"CDbApi_PoolMgr::GetPool: can not find configuration for connection pool sra\""
        "}"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* 403 response, no URLs, not masked */
FIXTURE_TEST_CASE(TestRespDenied, Fixture) {
    string s(
        "["
        "{"
        "\"accession\": \"SRR1219805\","
        "\"status\" : 403,"
        "\"message\" : \"Access denied - please request permission to access phs000710 / UR in dbGaP\""
        "},"
        "{"
        "\"accession\": \"abc\""
        "\"status\" : 404,"
        "\"message\" : \"Cannot resolve accession\""
        "}"
        "]"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

FIXTURE_TEST_CASE(TestRespNoLink, Fixture) {
    REQUIRE_RC_FAIL(MaskSdlResponse(nullptr, nullptr));
    REQUIRE_RC_FAIL(MaskSdlResponse("", nullptr));
    REQUIRE_RC_FAIL(MaskSdlResponse(nullptr, &buf));

    /* 404 response, no URLs, not masked */
    string s(
        "{\"version\": \"2\", \"result\" : [{\"bundle\": \"S\", \"status\" : 404, \"msg\" : \"Cannot resolve accession\"}]} "
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* no long URLs in response, not masked */
FIXTURE_TEST_CASE(TestRespLink, Fixture) {
    string s(
        "{ \"version\": \"2\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-11/SRR000/000/SRR000001/SRR000001.lite.1\"}"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* short URLs with short query, not masked */
FIXTURE_TEST_CASE(TestRespQueryShort, Fixture) {
    string s(
        "{ \"version\": \"2\", \"locations\": [ { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZC\" } ] }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* long query is masked */
FIXTURE_TEST_CASE(TestRespQuery, Fixture) {
    string s(
        "{ \"version\": \"2\",\"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt="
        "eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.eyJleHAiOjE2MjYzOT"
        "\"}"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare("{ \"version\": \"2\",\"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.e...\"}"));
}

/* 2 short URLs with short queries, not masked */
FIXTURE_TEST_CASE(TestRespQueriesShort, Fixture) {
    string s(
        "{ \"version\": \"2\", \"locations\": [ { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZC\" },"
        " { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=qwertyuiopJSUzI1NiIsImtpZC\" }"
        "] }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* 2 URLs with long queries are masked */
FIXTURE_TEST_CASE(TestRespQueries, Fixture) {
    string s(
        "{ \"version\": \"2\", \"locations\": [ { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZC1234\" },"
        " { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=qwertyuiopJSUzI1NiIsImtpZC5678\" }"
        "] }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare("{ \"version\": \"2\", \"locations\": [ { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=eyJh******************...\" },"
        " { \"link\" : \""
        "https://nih.gov/sdlr/sdlr.fcgi?jwt=qwer******************...\" }"
        "] }"));
}

/* complete response without long URLs is not masked */
FIXTURE_TEST_CASE(TestRespSRR000002, Fixture) {
    string s(
        "{ \"version\": \"2\",\"result\" : [{\"bundle\": \"SRR000002\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"srapub|SRR000002.lite\",\"accession\" : \"SRR000002\",\"type\" : \"sra\",\"name\" : \"SRR000002.lite\",\"size\" : 269527026,\"md5\" : \"fe3b994d90ae76fc094a24b30c6eb582\",\"modificationDate\" : \"2022-06-03T19:05:37Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"ncbi\",\"region\" : \"stor01\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-11/SRR000/000/SRR000002/SRR000002.lite.1\"}] }] }] }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(s));
}

/* complete response, long URL is masked */
FIXTURE_TEST_CASE(TestRespLongLink, Fixture) {
    string s(
        "{"
        " \"result\" : ["
        " {"
        " \"files\" : ["
        " {"
        " \"object\": \"remote|10576670\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"bam\","
        " \"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\","
        " \"size\" : 291876795499,"
        " \"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\","
        " \"modificationDate\" : \"2013-09-05T08:57:54Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"s3\","
        " \"region\" : \"us-east-1\","
        " \"expirationDate\" : \"2021-07-16T01:26:08Z\","
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap_files/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/4941154\""
        " }"
        " ]"
        " ]"
        " }"
        " ]"
        " }"
        " ]"
        " }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare(
        "{"
        " \"result\" : ["
        " {"
        " \"files\" : ["
        " {"
        " \"object\": \"remote|10576670\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"bam\","
        " \"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\","
        " \"size\" : 291876795499,"
        " \"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\","
        " \"modificationDate\" : \"2013-09-05T08:57:54Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"s3\","
        " \"region\" : \"us-east-1\","
        " \"expirationDate\" : \"2021-07-16T01:26:08Z\","
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap_files/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/49...\""
        " }"
        " ]"
        " ]"
        " }"
        " ]"
        " }"
        " ]"
        " }"));
}

/* response with multiple URLs, long URLs are masked */
FIXTURE_TEST_CASE(TestResp, Fixture) {
    string s(
        "{"
        " \"version\": \"2\","
        " \"result\" : ["
        " {"
        " \"bundle\": \"SRR2043623\","
        " \"status\" : 200,"
        " \"msg\" : \"ok\","
        " \"files\" : ["
        " {"
        " \"object\": \"remote|10576670\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"bam\","
        " \"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\","
        " \"size\" : 291876795499,"
        " \"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\","
        " \"modificationDate\" : \"2013-09-05T08:57:54Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"s3\","
        " \"region\" : \"us-east-1\","
        " \"expirationDate\" : \"2021-07-16T01:26:08Z\","
        " \"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.eyJleHAiOjE2MjYzOTUxNzAsImlhdCI6MTYyNjM4MDc3MCwianRpIjoiMzVlZmQ3NTItZjNmYi00NDczLWI4MDktMTMyNDczMTgxZTZjIiwibGluayI6Imh0dHBzOi8vMTAwMGdlbm9tZXMuczMuYW1hem9uYXdzLmNvbS9waGFzZTMvZGF0YS9OQTEyODkyL2hpZ2hfY292ZXJhZ2VfYWxpZ25tZW50L05BMTI4OTIubWFwcGVkLklMTFVNSU5BLmJ3YS5DRVUuaGlnaF9jb3ZlcmFnZV9wY3JfZnJlZS4yMDEzMDkwNi5iYW0_bmNiaV9waGlkPUEwOTE1NjQ3MEYwOTlDRjEwMDAwMDAwMDAwMDIwMDAyLjEmeC1hbXotcmVxdWVzdC1wYXllcj1yZXF1ZXN0ZXIiLCJyZWdpb24iOiJ1cy1lYXN0LTEiLCJzZXJ2aWNlIjoiczMiLCJzaWduaW5nQWNjb3VudCI6InMzLnVzLWVhc3QtMS5taXRyZSIsInRpbWVvdXQiOjE0NDAwfQ.aPj_4ORn19WClEWbexDSdaovGwoDPk7A28kwjk0ecpdnbHdJEXgy_VcQSddiBxrfxO8asXPbIS3oA7CikKnki_WovqNnZfFl-6IunVjXK_g_73rpcYCWm70FU-qoEzVJLDbgqFH-9Sv7kt0AgL_3uvVHTe4vXoj-zkB6Gw_HDWTRsZ62xww_1Jbgo39GthmPhjn2PdDe4Jw3KPtazKfpsFYtflHEcZjEWYcjAaEw6anztGXf8a74jLZnj8fUpXa2xeoHdWkeSiIcHnKVF_TcgTM2eQjJIsAww4gYRRuua-xd3UcXqHTOgYg_R_CdQFInjXd0eOjNmbxY0Us5JAZ4rg\","
        " \"ceRequired\" : true"
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap_files|4941154\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"sra\","
        " \"name\" : \"SRR2043623.pileup\","
        " \"size\" : 993762315,"
        " \"md5\" : \"180391bf474ff1905331fba1b3a3cb4e\","
        " \"modificationDate\" : \"2016-04-24T17:32:41Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        " \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap_files/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/4941154\""
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap|SRR2043623\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"sra\","
        " \"name\" : \"SRR2043623\","
        " \"size\" : 89782960107,"
        " \"md5\" : \"6b8072c5ef526cfbce477de3cd60d329\","
        " \"modificationDate\" : \"2016-04-05T04:52:22Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        " \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/SRR2043623\""
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap|SRR2043623\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"vdbcache\","
        " \"name\" : \"SRR2043623.vdbcache\","
        " \"size\" : 2329186083,"
        " \"md5\" : \"e6436017bb3bba0bf267559e8ea860e6\","
        " \"modificationDate\" : \"2016-06-23T19:41:22Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        "                   \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/SRR2043623.vdbcache\""
        " }"
        " ]"
        " }"
        " ]"
        " }"
        " ]"
        " }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare("{"
        " \"version\": \"2\","
        " \"result\" : ["
        " {"
        " \"bundle\": \"SRR2043623\","
        " \"status\" : 200,"
        " \"msg\" : \"ok\","
        " \"files\" : ["
        " {"
        " \"object\": \"remote|10576670\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"bam\","
        " \"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\","
        " \"size\" : 291876795499,"
        " \"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\","
        " \"modificationDate\" : \"2013-09-05T08:57:54Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"s3\","
        " \"region\" : \"us-east-1\","
        " \"expirationDate\" : \"2021-07-16T01:26:08Z\","
        " \"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr/sdlr.fcgi?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwid...\","
        " \"ceRequired\" : true"
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap_files|4941154\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"sra\","
        " \"name\" : \"SRR2043623.pileup\","
        " \"size\" : 993762315,"
        " \"md5\" : \"180391bf474ff1905331fba1b3a3cb4e\","
        " \"modificationDate\" : \"2016-04-24T17:32:41Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        " \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap_files/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/49...\""
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap|SRR2043623\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"sra\","
        " \"name\" : \"SRR2043623\","
        " \"size\" : 89782960107,"
        " \"md5\" : \"6b8072c5ef526cfbce477de3cd60d329\","
        " \"modificationDate\" : \"2016-04-05T04:52:22Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        " \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/SRR2043623\""
        " }"
        " ]"
        " },"
        " {"
        " \"object\": \"sragap|SRR2043623\","
        " \"accession\" : \"SRR2043623\","
        " \"type\" : \"vdbcache\","
        " \"name\" : \"SRR2043623.vdbcache\","
        " \"size\" : 2329186083,"
        " \"md5\" : \"e6436017bb3bba0bf267559e8ea860e6\","
        " \"modificationDate\" : \"2016-06-23T19:41:22Z\","
        " \"locations\" : ["
        " {"
        " \"service\": \"sra-ncbi\","
        " \"region\" : \"dbgap\","
        "                   \"encryptedForProjectId\" : 0,"
        " \"link\" : \"https://gap-download.be-md.ncbi.nlm.nih.gov/sragap/5FC0C409-4915-485E-9DBD-1EBAAF8FCF31/SRR20436...\""
        " }"
        " ]"
        " }"
        " ]"
        " }"
        " ]"
        " }"));
}

/* response with protected and signed URLs, long URLs are masked */
FIXTURE_TEST_CASE(prj_phs710EA_test, Fixture) {
    string s(
        "{ \"version\": \"2\",\"result\" : [{\"bundle\": \"SRR1219805\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10569885\",\"accession\" : \"SRR1219805\",\"type\" : \"bam\",\"name\" : \"NA19031.mapped.ILLUMINA.bwa.LWK.low_coverage.20130415.bam\",\"size\" : 29851641509,\"md5\" : \"c800d4179eaef5e7c6970fefeed1670c\",\"modificationDate\" : \"2014-04-07T18:17:02Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase3/data/NA19031/alignment/NA19031.mapped.ILLUMINA.bwa.LWK.low_coverage.20130415.bam\"}] },{\"object\": \"sragap|SRR1219805.lite\",\"accession\" : \"SRR1219805\",\"type\" : \"sra\",\"name\" : \"SRR1219805.lite\",\"size\" : 1863964532,\"md5\" : \"137902ac247b222096d403f4e24fb32a\",\"modificationDate\" : \"2022-09-15T18:37:47Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor01\",\"expirationDate\" : \"2026-05-11T05:01:11Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.eyJhY2MiOiJTUlIxMjE5ODA1IiwiZXhwIjoxNzc4NDcyMDczLCJmaWxlU2l6ZSI6IjE4NjM5NjQ1MzIiLCJpYXQiOjE3Nzg0NTc2NzMsImlnbm9yZUNlIjoidHJ1ZSIsImxpbmsiOiJodHRwczovL2dhcC1kb3dubG9hZC5iZS1tZC5uY2JpLm5sbS5uaWguZ292L3NvczEvc3JhLWNhLXpxLTIyL1NSUjAwMS8xMjE5L1NSUjEyMTk4MDUvU1JSMTIxOTgwNS5saXRlLjE_bmNiaV9waGlkPTNDMDJEQjBFQTAxMUI2NTEwMDAwMDAwMDAwMTAwMDEwLjEmcHJvamVjdF9pZD0wIiwicmVnaW9uIjoic3RvcjAxIiwic2VydmljZSI6InNyYS1zb3MiLCJzaWduaW5nQWNjb3VudCI6InNyYWdhcF9yLmJlbWQiLCJ0aW1lb3V0IjoxNDQwMH0.a_b_Hdq4gt6NgUEJjVhzYj5GZap1VUKbiMOsRVI9-jI2CuN_KenW1Sqo5UvdJuvdXJepoZuqRYub11p5Y1s4HCKDMNHUq-ajZhxyshlxEn4MDg7tHMViksW_MluCGarE-7ijruRIzlyLekBC9rsK6by8fY29f3HfzkfHVnOEZ5VfqGi2UOsQaa9oUnsITCCLGXH9WXaWuHhWG81XdkDT3ai-WgHD1sl3FTWxxtVb6Sif3MGoP_WNoeodd7S9YxpLC0xY1wExBJZmpRce2kZYaMzhAAI58OYAHQfvAaiy0pyHMU4F4JoCMPYmZgdX2gpaYoCC65UtFeLHxta7jSMU8g\"}] }] },{\"bundle\": \"SRR2043622\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10576671\",\"accession\" : \"SRR2043622\",\"type\" : \"bam\",\"name\" : \"NA12891.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\",\"size\" : 258100635109,\"md5\" : \"99e01e546635c028f4ebec4a27ba0af2\",\"modificationDate\" : \"2013-09-05T08:33:16Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase3/data/NA12891/high_coverage_alignment/NA12891.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\"}] },{\"object\": \"sragap|SRR2043622.lite\",\"accession\" : \"SRR2043622\",\"type\" : \"sra\",\"name\" : \"SRR2043622.lite\",\"size\" : 15253247146,\"md5\" : \"8132a179db8b39b8d4af233c32ebc0f1\",\"modificationDate\" : \"2022-11-21T16:50:47Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor02\",\"expirationDate\" : \"2026-05-11T05:01:12Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.eyJhY2MiOiJTUlIyMDQzNjIyIiwiZXhwIjoxNzc4NDcyMDc0LCJmaWxlU2l6ZSI6IjE1MjUzMjQ3MTQ2IiwiaWF0IjoxNzc4NDU3Njc0LCJpZ25vcmVDZSI6InRydWUiLCJsaW5rIjoiaHR0cHM6Ly9nYXAtZG93bmxvYWQuYmUtbWQubmNiaS5ubG0ubmloLmdvdi9zb3M4L3NyYS1jYS16cS04MjAvU1JSMDAyLzIwNDMvU1JSMjA0MzYyMi9TUlIyMDQzNjIyLmxpdGUuMT9uY2JpX3BoaWQ9M0MwMkRCMEVBMDExQjY1MTAwMDAwMDAwMDAxMDAwMTAuMSZwcm9qZWN0X2lkPTAiLCJyZWdpb24iOiJzdG9yMDIiLCJzZXJ2aWNlIjoic3JhLXNvcyIsInNpZ25pbmdBY2NvdW50Ijoic3JhZ2FwMl9yLmJlbWQiLCJ0aW1lb3V0IjoxNDQwMH0.XkRAg24OpxIDEIZ_S9e6zsnfs1k-H0gd-VSobHrd_qcCxUd9e7qiH2MJjdAzuBmhltwTaBuL5Mv8p7_o8rkLdktV1qPgwTWNs3lU0Vpm8AofKjJDpHYYoaUDgip-7r4ZDH3KUhadLOlXJ_Lbk8I6u45snSRpq0j5V7IIOAeyAjAqLjxwP7if-m8jLRmx1Ac8DEQ_YBFCzcrWKhuwGB9P3dPUcym1P7pBkXr5Sl_JH_ykdcG-QFwfWPMqrie4SIFv82TtfkQkj9Y2tujB7oKZQXyBWI29VaPZKlzLs0-BBz6Dm_BnwHlVoyLEAR-K8DBNlfprEekn8X4dt1UlSS3lkQ\"}] }] },{\"bundle\": \"SRR2043623\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10576670\",\"accession\" : \"SRR2043623\",\"type\" : \"bam\",\"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\",\"size\" : 291876795499,\"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\",\"modificationDate\" : \"2013-09-05T08:57:54Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase3/data/NA12892/high_coverage_alignment/NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\"}] },{\"object\": \"sragap|SRR2043623.lite\",\"accession\" : \"SRR2043623\",\"type\" : \"sra\",\"name\" : \"SRR2043623.lite\",\"size\" : 20826466885,\"md5\" : \"bfebc684f94fcc889ecf30e55f802c03\",\"modificationDate\" : \"2022-08-07T14:12:16Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor02\",\"expirationDate\" : \"2026-05-11T05:01:12Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSldUIn0.eyJhY2MiOiJTUlIyMDQzNjIzIiwiZXhwIjoxNzc4NDcyMDc0LCJmaWxlU2l6ZSI6IjIwODI2NDY2ODg1IiwiaWF0IjoxNzc4NDU3Njc0LCJpZ25vcmVDZSI6InRydWUiLCJsaW5rIjoiaHR0cHM6Ly9nYXAtZG93bmxvYWQuYmUtbWQubmNiaS5ubG0ubmloLmdvdi9zb3M4L3NyYS1jYS16cS04MjAvU1JSMDAyLzIwNDMvU1JSMjA0MzYyMy9TUlIyMDQzNjIzLmxpdGUuMT9uY2JpX3BoaWQ9M0MwMkRCMEVBMDExQjY1MTAwMDAwMDAwMDAxMDAwMTAuMSZwcm9qZWN0X2lkPTAiLCJyZWdpb24iOiJzdG9yMDIiLCJzZXJ2aWNlIjoic3JhLXNvcyIsInNpZ25pbmdBY2NvdW50Ijoic3JhZ2FwMl9yLmJlbWQiLCJ0aW1lb3V0IjoxNDQwMH0.MEyPCbJdnA2RL7dcft48MMmw8FU57sCzsru7nJ_nomi_8nW1BXmF3qYDwfunliOQaeMpHRUrDZZfZ-FRKuPel-MjPKTt1nL_-iM-zS3SykeH4s672ESxsGlw2c5lIfFTnYn40asRgYhorTju0LdKjvJEVhsOMrTFTQeMVmcZT0FDa8H-GTdZscgITX34aaJXKYcGUhcH3OCHE9sdLoc4jpWhSOUMHbxNlyYT8kgeDLW-snoxCburMDjy6KGFYAoQsWkGiMng3UNGs9QLf7ZPuFtceFhZEmHUieFjMDemAaCVcivxbCsHrVaOQL0Gx9eDKPPvm-X2YMbA6ya_sNS0tg\"}] }] },{\"bundle\": \"SRR867664\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"srapub|SRR867664.sralite\",\"accession\" : \"SRR867664\",\"type\" : \"sra\",\"name\" : \"SRR867664.sralite\",\"size\" : 49574593,\"md5\" : \"dc6c25be17bd99c84c68de6035f40168\",\"modificationDate\" : \"2020-08-14T02:35:10Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"ncbi\",\"region\" : \"stor01\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-14/SRR000/867/SRR867664.sralite.1\"}] },{\"object\": \"srapub|SRR867664.sralite\",\"accession\" : \"SRR867664\",\"type\" : \"vdbcache\",\"name\" : \"SRR867664.sralite.vdbcache\",\"size\" : 5634325,\"md5\" : \"d297d23d066ec595e70675ad72eef824\",\"modificationDate\" : \"2020-08-14T02:35:16Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"ncbi\",\"region\" : \"stor01\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-14/SRR000/867/SRR867664.sralite.vdbcache.1\"}] }] }] }"
    );
    REQUIRE_RC(MaskSdlResponse(s.c_str(), &buf));
    REQUIRE(Compare("{ \"version\": \"2\",\"result\" : [{\"bundle\": \"SRR1219805\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10569885\",\"accession\" : \"SRR1219805\",\"type\" : \"bam\",\"name\" : \"NA19031.mapped.ILLUMINA.bwa.LWK.low_coverage.20130415.bam\",\"size\" : 29851641509,\"md5\" : \"c800d4179eaef5e7c6970fefeed1670c\",\"modificationDate\" : \"2014-04-07T18:17:02Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase...\"}] },{\"object\": \"sragap|SRR1219805.lite\",\"accession\" : \"SRR1219805\",\"type\" : \"sra\",\"name\" : \"SRR1219805.lite\",\"size\" : 1863964532,\"md5\" : \"137902ac247b222096d403f4e24fb32a\",\"modificationDate\" : \"2022-09-15T18:37:47Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor01\",\"expirationDate\" : \"2026-05-11T05:01:11Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSld...\"}] }] },{\"bundle\": \"SRR2043622\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10576671\",\"accession\" : \"SRR2043622\",\"type\" : \"bam\",\"name\" : \"NA12891.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\",\"size\" : 258100635109,\"md5\" : \"99e01e546635c028f4ebec4a27ba0af2\",\"modificationDate\" : \"2013-09-05T08:33:16Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase...\"}] },{\"object\": \"sragap|SRR2043622.lite\",\"accession\" : \"SRR2043622\",\"type\" : \"sra\",\"name\" : \"SRR2043622.lite\",\"size\" : 15253247146,\"md5\" : \"8132a179db8b39b8d4af233c32ebc0f1\",\"modificationDate\" : \"2022-11-21T16:50:47Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor02\",\"expirationDate\" : \"2026-05-11T05:01:12Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSld...\"}] }] },{\"bundle\": \"SRR2043623\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"remote|10576670\",\"accession\" : \"SRR2043623\",\"type\" : \"bam\",\"name\" : \"NA12892.mapped.ILLUMINA.bwa.CEU.high_coverage_pcr_free.20130906.bam\",\"size\" : 291876795499,\"md5\" : \"ed1f04b9b80ceef5c2ce4b52b76eed2a\",\"modificationDate\" : \"2013-09-05T08:57:54Z\",\"locations\" : [{\"service\": \"gs\",\"region\" : \"us\",\"link\" : \"https://storage.googleapis.com/genomics-public-data/ftp-trace.ncbi.nih.gov/1000genomes/ftp/phase...\"}] },{\"object\": \"sragap|SRR2043623.lite\",\"accession\" : \"SRR2043623\",\"type\" : \"sra\",\"name\" : \"SRR2043623.lite\",\"size\" : 20826466885,\"md5\" : \"bfebc684f94fcc889ecf30e55f802c03\",\"modificationDate\" : \"2022-08-07T14:12:16Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"sra-sos\",\"region\" : \"stor02\",\"expirationDate\" : \"2026-05-11T05:01:12Z\",\"link\" : \"https://locate.ncbi.nlm.nih.gov/sdlr?jwt=eyJhbGciOiJSUzI1NiIsImtpZCI6InNkbHJraWQxIiwidHlwIjoiSld...\"}] }] },{\"bundle\": \"SRR867664\",\"status\" : 200,\"msg\" : \"ok\",\"files\" : [{\"object\": \"srapub|SRR867664.sralite\",\"accession\" : \"SRR867664\",\"type\" : \"sra\",\"name\" : \"SRR867664.sralite\",\"size\" : 49574593,\"md5\" : \"dc6c25be17bd99c84c68de6035f40168\",\"modificationDate\" : \"2020-08-14T02:35:10Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"ncbi\",\"region\" : \"stor01\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-14/SRR000/867/SRR867664.sralite.1\"}] },{\"object\": \"srapub|SRR867664.sralite\",\"accession\" : \"SRR867664\",\"type\" : \"vdbcache\",\"name\" : \"SRR867664.sralite.vdbcache\",\"size\" : 5634325,\"md5\" : \"d297d23d066ec595e70675ad72eef824\",\"modificationDate\" : \"2020-08-14T02:35:16Z\",\"noqual\" : true,\"locations\" : [{\"service\": \"ncbi\",\"region\" : \"stor01\",\"link\" : \"https://sra-download.ncbi.nlm.nih.gov/sos5/sra-pub-zq-14/SRR000/867/SRR867664.sralite.vdbcache.1\"}] }] }] }"));
}

int main(int argc, char* argv[])
{   return TestSdlMaskSuite(argc, argv); }
