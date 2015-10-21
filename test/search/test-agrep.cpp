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

#include <../libs/search/search-priv.h>
#include <search/nucstrstr.h>

#include <stdexcept>

#include <stdio.h>

#include "search-vdb.h"

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
    VRefVariation* self;
    INSDC_dna_text ref[] = "ACGTACGTACGTACGTACGTACGTACGTACGT";
    REQUIRE_RC_FAIL ( VRefVariationIUPACMake ( &self, ref, string_size ( ref ), 0, "", 0, 0 ) );
}

static 
void 
PrintMatrix ( const int p_matrix[], size_t p_rows, size_t p_cols )
{
    for ( size_t i = 0; i < p_rows; ++i )
    {
        for ( size_t j = 0; j < p_cols; ++j )
        {
            cout << p_matrix [ i * p_cols + j ] << " ";
        }
        cout << endl;
    }
}

TEST_CASE ( SmithWaterman_calculate_matrix_for_exact_match )
{
    const INSDC_dna_text Ref[] = "ACGTACGTACGTACGTACGTACGTACGTACGT";
    const INSDC_dna_text Query[] = "ACGTACGTACG";
    const size_t Rows = sizeof ( Ref ) / sizeof ( INSDC_dna_text );
    const size_t Cols = sizeof ( Query ) / sizeof ( INSDC_dna_text );
    int matrix [ Rows * Cols ];
    int maxScore = -1;
    REQUIRE_RC ( calculate_similarity_matrix ( Ref, Rows - 1, Query, Cols - 1, matrix, true, & maxScore ) ); 
    //PrintMatrix ( matrix, Rows, Cols );
    REQUIRE_EQ ( int ( Cols - 1 ) * 2, maxScore ); // exact match
}

TEST_CASE ( SmithWaterman_calculate_matrix_for_total_mismatch )
{
    const INSDC_dna_text Ref[] =    "AAAAAAAAAAAAAAAAAAAAAA";
    const INSDC_dna_text Query[] =  "GGGGG";
    const size_t Rows = sizeof ( Ref ) / sizeof ( INSDC_dna_text );
    const size_t Cols = sizeof ( Query ) / sizeof ( INSDC_dna_text );
    int matrix [ Rows * Cols ];
    int maxScore = -1;
    REQUIRE_RC ( calculate_similarity_matrix ( Ref, Rows - 1, Query, Cols - 1, matrix, true, & maxScore ) ); 
    //PrintMatrix ( matrix, Rows, Cols );
    REQUIRE_EQ ( 0, maxScore ); // total mismatch
}

// Nucstrstr
static 
void 
ConvertAsciiTo2NAPacked ( const string& p_read, unsigned char* pBuf2NA, size_t nBuf2NASize )
{
    static unsigned char map [ 1 << ( sizeof ( char ) * 8 ) ];
    map['A'] = map['a'] = 0;
    map['C'] = map['c'] = 1;
    map['G'] = map['g'] = 2;
    map['T'] = map['t'] = 3;
    
    static size_t shiftLeft [ 4 ] = { 6, 4, 2, 0 };

    fill ( pBuf2NA, pBuf2NA + nBuf2NASize, 0 );
    
    for ( size_t iChar = 0; iChar < p_read . size (); ++iChar )
    {
        size_t iByte = iChar / 4;
        if ( iByte > nBuf2NASize )
        {
            assert ( false );
            break;
        }

        pBuf2NA[iByte] |= map [ size_t ( p_read [ iChar ] ) ] << shiftLeft [ iChar % 4 ];
    }
}

static
int
RunNucStrtr ( const string& p_ref, const string& p_query, bool p_positional )
{
    unsigned char buf2na [ 1024 ];
    ConvertAsciiTo2NAPacked ( p_ref, buf2na, sizeof ( buf2na ) );
    
    NucStrstr *nss;
    if ( NucStrstrMake ( & nss, p_positional ? 1 : 0, p_query . c_str (), p_query . size () ) != 0 )
        throw logic_error ( "RunNucStrtr: NucStrstrMake() failed" );
    unsigned int selflen = 0u;
    // NB: for now, all searches start are from the beginning ( nucstrstr.h says "may be >= 4"; not sure what that means )
    const unsigned int pos = 0;
    int ret = NucStrstrSearch ( nss, ( const void * ) buf2na, pos, p_ref . size () - pos, & selflen );
    NucStrstrWhack ( nss );
    return ret;
}

TEST_CASE ( Nucstrstr_NonPositional_NotFound )
{
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "ACTA", false ) );
}

TEST_CASE ( Nucstrstr_NonPositional_Found_AtStart )
{
    REQUIRE_NE ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "ACGTACGTACG", false ) );
}

TEST_CASE ( Nucstrstr_NonPositional_Found_InMiddle )
{
    REQUIRE_NE ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "GTACGTACG", false ) );
}

TEST_CASE ( Nucstrstr_Positional_NoExpr_NotFound )
{
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "ACCGT", true ) );
}

TEST_CASE ( Nucstrstr_Positional_NoExpr_Found_AtStart )
{
    REQUIRE_EQ ( 1, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "ACGTACGTACG", true ) );
}

TEST_CASE ( Nucstrstr_Positional_NoExpr_Found_InMiddle )
{
    REQUIRE_EQ ( 4, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "TACGTACG", true ) );
}

TEST_CASE ( Nucstrstr_Positional_Not_False )
{
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "!TACGTACG", true ) );
}
TEST_CASE ( Nucstrstr_Positional_Not_True )
{   // "!" returns only 0 or 1, not the position
    REQUIRE_EQ ( 1, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "!AG", true ) );
}

TEST_CASE ( Nucstrstr_Positional_Caret_Found )
{   
    REQUIRE_EQ ( 1, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "^AC", true ) );
}
TEST_CASE ( Nucstrstr_Positional_Caret_NotFound )
{   
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGT", "^C", true ) );
}

TEST_CASE ( Nucstrstr_Positional_Dollar_Found )
{   
    REQUIRE_EQ ( 33, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TGCA$", true ) );
}
TEST_CASE ( Nucstrstr_Positional_Dollar_NotFound )
{   
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCAA", "TGCA$", true ) );
}

TEST_CASE ( Nucstrstr_Positional_OR_Found_1 )
{   
    REQUIRE_EQ ( 4, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TGAA|TACG", true ) );
}
TEST_CASE ( Nucstrstr_Positional_OR_Found_2 )
{   
    REQUIRE_EQ ( 4, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TGAA||TACG", true ) );
}
TEST_CASE ( Nucstrstr_Positional_OR_Found_FirstMatchPositionReported )
{   
    REQUIRE_EQ ( 2, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TGAA|CGT|TACG", true ) );
}
TEST_CASE ( Nucstrstr_Positional_OR_NotFound )
{   
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TGAA|TACA", true ) );
}

TEST_CASE ( Nucstrstr_Positional_AND_Found_LastMatchPositionReported )
{   
    REQUIRE_EQ ( 4, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "CGTA&TACG", true ) );
}
TEST_CASE ( Nucstrstr_Positional_AND_NotFound )
{   
    REQUIRE_EQ ( 0, RunNucStrtr ( "ACGTACGTACGTACGTACGTACGTACGTACGTTGCA", "TACG&TACA", true ) );
}

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
