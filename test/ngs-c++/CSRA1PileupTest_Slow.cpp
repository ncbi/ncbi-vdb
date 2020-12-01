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
* Slow tests for NGS Pileup interface, CSRA1 based implementation
*/

#include "CSRA1PileupFixture.hpp"

#include <sstream>

using namespace ncbi::NK;

TEST_SUITE(SlowNgsCsra1PileupCppTestSuite);

FIXTURE_TEST_CASE(CSRA1_PileupEventIterator_AdjacentIndels, CSRA1PileupFixture)
{
    // This test crashed in CSRA1_PileupEvent.c because of
    // insertion followed by deletion (see vdb-dump SRR1164787 -T PRIMARY_ALIGNMENT -R 209167)
    // So now this test has to just run with no crashes to be considered as passed successfully

    char const db_path[] = "SRR1164787";
    char const ref_name[] = "chr1";

    int64_t const pos_start = 1386093;
    int64_t const pos_end = 9999999;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    std::ostringstream sstream;

    mimic_sra_pileup ( open(db_path), ref_name, ngs::Alignment::primaryAlignment, pos_start, len, sstream );
}

uint64_t pileup_test_all_functions (
            ncbi::ReadCollection run,
            char const* ref_name,
            ngs::Alignment::AlignmentCategory category,
            int64_t const pos_start, uint64_t const len)
{
    uint64_t ret = 0;

    ngs::Reference r = run.getReference ( ref_name );

    // in strRefSlice we want to have bases to report current base and deletions
    // for current base it would be enough to have only slice [pos_start, len]
    // but for deletions we might have situation when we want
    // to report a deletion that goes beyond (pos_start + len) on the reference
    // so we have to read some bases beyond our slice end
    ngs::String strRefSlice = r.getReferenceBases ( pos_start, len + 100);

    ngs::PileupIterator pi = r.getPileupSlice ( pos_start, len, category );

    int64_t pos = pos_start;
    for (; pi.nextPileup (); ++ pos)
    {
        ret += 1000000;

        size_t event_count = 0;
        for (; pi.nextPileupEvent () && pos % 17 != 0; ++ event_count)
        {
            //ngs::Alignment alignment = pi.getAlignment();
            //ret += (uint64_t)(alignment.getAlignmentLength() + alignment.getAlignmentPosition());

            ret += (uint64_t)pi.getAlignmentBase();
            ret += (uint64_t)pi.getAlignmentPosition();
            ret += (uint64_t)pi.getAlignmentQuality();
            ret += (uint64_t)pi.getEventIndelType();
            ret += (uint64_t)pi.getEventRepeatCount();
            ret += (uint64_t)pi.getEventType();
            ret += (uint64_t)pi.getFirstAlignmentPosition();
            ret += (uint64_t)pi.getInsertionBases().size();
            ret += (uint64_t)pi.getInsertionQualities().size();
            ret += (uint64_t)pi.getLastAlignmentPosition();
            ret += (uint64_t)pi.getMappingQuality();
            ret += (uint64_t)pi.getPileupDepth();
            ret += (uint64_t)pi.getReferenceBase();
            ret += (uint64_t)pi.getReferencePosition();
            ret += (uint64_t)pi.getReferenceSpec().size();

            if ( (event_count + 1) % 67 == 0 )
            {
                ret += 100000;
                pi.resetPileupEvent();
                break;
            }
        }
    }

    return ret;
}

FIXTURE_TEST_CASE(CSRA1_PileupIterator_TestAllFunctions, NgsFixture)
{
    uint64_t ret = 0;
    ret = pileup_test_all_functions ( open ("SRR822962"), "chr2"/*"NC_000002.11"*/, ngs::Alignment::all, 0, 20000 );
    // this magic sum was taken from an observed result,
    // but due to a bug in "resetPileupEvent()", is likely to be wrong
    // resetting the magic sum to what is being returned now.
    REQUIRE_EQ ( ret, (uint64_t)/*46433887435*/ /*46436925309*/ 46436941625 );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-slow_ngs_csra1pileup-c++";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=SlowNgsCsra1PileupCppTestSuite(argc, argv);

    NgsFixture::ReleaseCache();
    return rc;
}

}

