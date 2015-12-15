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

/**
* Unit tests for search
*/

#include <ktst/unit_test.hpp>
#include <kapp/main.h> /* KMain */

#include <stdexcept>

#include <stdio.h>

#include "search-vdb.h"
#include <search/ref-variation.h>

using namespace std;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(TestSuiteSearch, argsHandler);

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

void trim_eol(char* psz)
{
    size_t len = strlen(psz);
    if (psz[len - 2] == '\n' || psz[len - 2] == '\r')
        psz[len - 2] = '\0';
    else if (psz[len - 1] == '\n' || psz[len - 1] == '\r')
        psz[len - 1] = '\0';
}

#if 0
#include "PerfCounter.h"
#include <klib/checksum.h>

TEST_CASE(TempCRC)
{
    std::cout << "Testing crc32 speed..." << std::endl;
    size_t const size = (size_t)10 << 20;
    char* buf = new char [size];
    CPerfCounter counter( "CRC32" );

    for ( size_t i = 0; i < size; ++i )
    {
        buf[i] = i;
    }

    size_t const ALIGN_BYTES = _ARCH_BITS / 8;

    printf ("allocated %saligned (address=%p)\n", (int)((size_t)buf % ALIGN_BYTES) ? "un" : "", buf);

    size_t offset = 2;
    uint32_t crc32;

    {
        CPCount count(counter);
        crc32 = ::CRC32 ( 0, buf + offset, size - offset );
    }

    printf ("Caclulated CRC32: 0x%08X (%.2lf MB/s)\n", crc32, (size >> 20)/counter.GetSeconds());

    delete[]buf;
}
#endif

TEST_CASE(SearchCompare)
{
    //std::cout << "This is search algorithm time comparison test" << std::endl << std::endl;
    std::cout << "SearchCompare test is currently OFF" << std::endl;

    /*
    TimeTest("SRR067432", SearchTimeTest::ALGORITHM_AGREP);
    TimeTest("SRR067432", SearchTimeTest::ALGORITHM_LCS_DP);
    TimeTest("SRR067432", SearchTimeTest::ALGORITHM_NUCSTRSTR);
    TimeTest("SRR067432", SearchTimeTest::ALGORITHM_LCS_SAIS);*/

    //std::cout << std::endl << "Time comparison test acomplished" << std::endl;
}

TEST_CASE(TestCaseAgrep)
{
    std::cout << "TestCaseAgrep test is currently OFF" << std::endl;
    //VDBSearchTest::CFindLinker().Run(AGREP_MODE_ASCII|AGREP_ALG_MYERS, "SRR067432");//"SRR068408");
    //VDBSearchTest::CFindLinker().Run(AGREP_MODE_ASCII|AGREP_ALG_MYERS, "SRR067408_extr");

/*
    char const pszRunNamesFile[] = "454Runs.txt";
    FILE* fRuns = fopen(pszRunNamesFile, "r");
    if (!fRuns)
    {
        printf("Failed to open file: %s\n", pszRunNamesFile);
        return;
    }
    bool bSkipping = true;
    for (; !feof(fRuns); )
    {
        char szRuns[512] = "";
        char* psz = fgets(szRuns, sizeof(szRuns), fRuns);
        if (!psz)
            break;
        trim_eol(psz);
        if (bSkipping && !strcmp(psz, "SRR067408"))
        {
            bSkipping = false;
        }
        if (bSkipping)
            continue;
        VDBSearchTest::CFindLinker().Run(AGREP_MODE_ASCII|AGREP_ALG_MYERS, psz);
    }*/
}

// Fgrep

static void RunFgrep ( FgrepFlags p_alg )
{   // VDB-2669: creates uninitialized memory
    Fgrep* fg;
    const char* queries[] = { "RRRRRAH" };
    if ( FgrepMake ( & fg, FGREP_MODE_ASCII | p_alg, queries, 1 ) ) // this used to leave uninitialized memory ...
        throw logic_error ( "RunFgrep: FgrepMake() failed" );
    
    const std::string str ( 1000, 'A' );
    FgrepMatch matchinfo;
    if ( 0 != FgrepFindFirst ( fg, str . data (), str . size (), & matchinfo ) ) // ... the use of which showed up in valgrind here, and sometimes caused a crash
        throw logic_error ( "RunFgrep: FgrepFindFirst() found a false match" );
    
    FgrepFree ( fg );
}

TEST_CASE ( DumbGrep_Crash )
{
    RunFgrep ( FGREP_ALG_DUMB );
}

TEST_CASE ( BoyerMooreGrep_Crash )
{
    RunFgrep ( FGREP_ALG_BOYERMOORE );
}

TEST_CASE ( AhoGrep_Crash )
{
    RunFgrep ( FGREP_ALG_AHOCORASICK );
}

// Smith-Waterman
TEST_CASE ( SmithWaterman_crash)
{
    RefVariation* self;
    INSDC_dna_text ref[] = "ACGTACGTACGTACGTACGTACGTACGTACGT";
    REQUIRE_RC_FAIL ( RefVariationIUPACMake ( &self, ref, string_size ( ref ), 0, 0, "", 0, ::refvarAlgSW ) );
}

#ifdef _WIN32
#define PRSIZET "I"
#else
#define PRSIZET "z"
#endif

void print_refvar_obj (::RefVariation const* obj)
{
    size_t allele_len = 0, allele_start = 0, allele_len_on_ref = 0;
    char const* allele = NULL;
    ::RefVariationGetAllele( obj, & allele, & allele_len, & allele_start );
    ::RefVariationGetAlleleLenOnRef ( obj, & allele_len_on_ref );

    printf ("<no ref name>:%"PRSIZET"u:%"PRSIZET"u:%.*s\n",
        allele_start, allele_len_on_ref, (int)allele_len, allele);
}

#undef PRSIZET


void vrefvar_bounds (::RefVarAlg alg, char const* ref,
    size_t ref_len, size_t pos, size_t len_on_ref,
    char const* query, size_t query_len)
{

    ::RefVariation* obj;

    rc_t rc = ::RefVariationIUPACMake ( & obj, ref, ref_len, pos, len_on_ref, query, query_len, alg );

    print_refvar_obj ( obj );

    if ( rc == 0 )
        ::RefVariationRelease( obj );
}

void vrefvar_bounds_n(::RefVarAlg alg)
{
    //                  01234567890123456789
    char const ref[] = "NNNNNNNNNNTAACCCTAAC";
    //                       CCCCTTAGG-

    size_t pos = 5, len_on_ref = 10;
    char const query[] = "CCCCTTAGG";

    vrefvar_bounds ( alg, ref, strlen(ref), pos, len_on_ref, query, strlen(query) );
}

void vrefvar_bounds_0(::RefVarAlg alg)
{
    //                  01234567890123456789
    char const ref[] = "TAACCCTAAC";
    //                  TTAGG-

    size_t pos = 0, len_on_ref = 5;
    char const query[] = "TAGG";

    vrefvar_bounds ( alg, ref, strlen(ref), pos, len_on_ref, query, strlen(query) );
}

void vrefvar_bounds_N0(::RefVarAlg alg)
{
    //                  01234567890123456789
    char const ref[] = "NNNNNTAACCCTAAC";
    //                  CCCCTTTAGG-

    size_t pos = 0, len_on_ref = 10;
    char const query[] = "CCCCTTAGG";

    vrefvar_bounds ( alg, ref, strlen(ref), pos, len_on_ref, query, strlen(query) );
}

TEST_CASE ( SmithWaterman_bounds_N )
{
    printf ("TODO: this test is derived from the real example which hangs up now (2015-12-14):\n");
    printf ("echo \"67068302 NC_000001.10:9995:10:CCCCTTAGG\" | var-expand --algorithm=sw\n");
    vrefvar_bounds_n ( ::refvarAlgSW );
    vrefvar_bounds_n ( ::refvarAlgRA );

    vrefvar_bounds_0 ( ::refvarAlgSW );
    vrefvar_bounds_0 ( ::refvarAlgRA );

    vrefvar_bounds_N0 ( ::refvarAlgSW );
    vrefvar_bounds_N0 ( ::refvarAlgRA );
}

#if SHOW_UNIMPLEMENTED
TEST_CASE ( SmithWaterman_basic )
{
    VRefVariation* self;
    INSDC_dna_text ref[] = "ACGTACGTACGTACGTACGTACGTACGTACGT";
    INSDC_dna_text var[] = "";
    
    REQUIRE_RC ( VRefVariationIUPACMake ( &self, ref, string_size ( ref ), 0, var, string_size ( var ), 0 ) );
    
    REQUIRE_EQ ( string ( "ACGTACGT" ), string ( VRefVariationIUPACGetVariation ( self ) ) );
    /*
    REQUIRE_EQ ( string ( ref ), string ( VRefVariationIUPACGetRefChunk ( self ) ) );
    REQUIRE_EQ ( string_size ( ref ), VRefVariationIUPACGetRefChunkSize ( self ) );
    REQUIRE_EQ ( (size_t)8, VRefVariationIUPACGetVarStart ( self ) );
    REQUIRE_EQ ( (size_t)8, VRefVariationIUPACGetVarSize ( self ) );
    REQUIRE_EQ ( (size_t)4, VRefVariationIUPACGetVarLenOnRef ( self ) );
    */
    REQUIRE_RC ( VRefVariationIUPACRelease ( self ) );
}
#endif

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-search";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc = TestSuiteSearch(argc, argv);
    return rc;
}

} // end of extern "C"
