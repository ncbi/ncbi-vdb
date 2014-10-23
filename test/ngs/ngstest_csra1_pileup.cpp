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
* Unit tests for NGS C interface, CSRA1 archives, Pileup-related functions
*/

#include "ngs_c_fixture.hpp"

#include "NGS_Pileup.h"

#include <kdb/manager.h>

#include <vdb/manager.h>
#include <vdb/vdb-priv.h>

#include <klib/printf.h>

#include <limits.h>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(NgsCsra1PileupTestSuite);

const char* CSRA1_PrimaryOnly   = "SRR1063272";
const char* CSRA1_WithSecondary = "SRR833251";
const char* CSRA1_WithCircularReference = "SRR821492";

class CSRA1_Fixture : public NGS_C_Fixture
{
public:
    CSRA1_Fixture()
    : m_pileup (0)
    {
    }
    virtual void Release()
    {
        if (m_ctx != 0)
        {
            if ( m_pileup != 0 )
            {
                NGS_PileupRelease ( m_pileup, m_ctx );
            }
        }
        NGS_C_Fixture :: Release ();
    }
    
    NGS_Pileup*     m_pileup;
};

// PileupIterator, full reference
//TODO: CSRA1_PileupIteratorFull_NoAccessBeyondEnd
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessBeforeNext, CSRA1_Fixture)
{
    ENTRY_GET_REF ( CSRA1_PrimaryOnly, "supercont2.1" );
    
    m_pileup = NGS_ReferenceGetPileups( m_ref, ctx, true, false); 
    REQUIRE ( ! FAILED () && m_pileup );

    REQUIRE_NULL ( NGS_PileupGetReferenceSpec ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    NGS_PileupGetReferencePosition ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    REQUIRE_NULL ( NGS_PileupGetPileupEvents ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    NGS_PileupGetPileupDepth ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    EXIT;
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(CSRA1_PileupIterator_AccessAfterNext, CSRA1_Fixture)
{
    ENTRY_GET_REF ( CSRA1_PrimaryOnly, "supercont2.1" );
    
    m_pileup = NGS_ReferenceGetPileups( m_ref, ctx, true, false); 
    REQUIRE ( ! FAILED () && m_pileup );
    REQUIRE ( NGS_PileupIteratorNext ( m_pileup, ctx ) );

    REQUIRE_STRING ( "supercont2.1", NGS_PileupGetReferenceSpec ( m_pileup, ctx ) ); 
    
    REQUIRE_EQ ( (int64_t)0, NGS_PileupGetReferencePosition ( m_pileup, ctx ) ); 
    REQUIRE ( ! FAILED () );
    
    NGS_PileupEvent * events = NGS_PileupGetPileupEvents ( m_pileup, ctx ); 
    REQUIRE ( ! FAILED () && events );
    
    REQUIRE_EQ ( (unsigned int)0, NGS_PileupGetPileupDepth ( m_pileup, ctx ) ); 
    
    EXIT;
}
#endif
////TODO: Pileup
// NGS_PileupGetReferenceSpec
// NGS_PileupGetReferencePosition
// NGS_PileupGetPileupEvents
// NGS_PileupGetPileupDepth


////TODO: PileupEventIterator
//TODO: CSRA1_PileupEventIterator_NoAccessBeyondEnd
//TODO: CSRA1_PileupEventIterator_NoAccessBeforeNext
//TODO: CSRA1_PileupEventIterator_AccessAfterNext

////TODO: PileupEvent
//TODO: NGS_PileupEventGetReferenceSpec
//TODO: NGS_PileupEventGetReferencePosition
//TODO: NGS_PileupEventGetMappingQuality
//TODO: NGS_PileupEventGetAlignmentId
//TODO: NGS_PileupEventGetAlignment
//TODO: NGS_PileupEventGetAlignmentPosition
//TODO: NGS_PileupEventGetFirstAlignmentPosition
//TODO: NGS_PileupEventGetLastAlignmentPosition
//TODO: NGS_PileupEventGetEventType
//TODO: NGS_PileupEventGetAlignmentBase
//TODO: NGS_PileupEventGetAlignmentQuality
//TODO: NGS_PileupEventGetInsertionBases
//TODO: NGS_PileupEventGetInsertionQualities
//TODO: NGS_PileupEventGetDeletionCount

////TODO: PileupIterator, reference slice
////TODO: PileupIterator, full circular reference 
////TODO: PileupIterator, circular reference slice

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

const char UsageDefaultName[] = "test-ngs_csra1_pileup";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t m_coll=NgsCsra1PileupTestSuite(argc, argv);
    return m_coll;
}

}

