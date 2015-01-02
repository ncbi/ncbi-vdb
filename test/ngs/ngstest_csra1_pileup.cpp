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
#include "NGS_PileupEvent.h"

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

#define ENTRY_GET_PILEUP(acc,ref) \
    ENTRY_GET_REF(acc,ref) \
    m_pileup = NGS_ReferenceGetPileups( m_ref, ctx, true, false); \
    REQUIRE ( ! FAILED () && m_pileup );

#define ENTRY_GET_PILEUP_NEXT(acc,ref) \
    ENTRY_GET_PILEUP(acc,ref) \
    REQUIRE ( NGS_PileupIteratorNext ( m_pileup, ctx ) );
    
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
    
    void Advance ( uint32_t count )
    {
        while ( count > 0 )
        {
            if  ( ! NGS_PileupIteratorNext ( m_pileup, m_ctx ) )
                throw std :: logic_error ( "CSRA1_Fixture::Advance : NGS_PileupIteratorNext() failed" );
            --count;
        }
    }
    
    NGS_Pileup*     m_pileup;
};

// PileupIterator, full reference

// no access before a call to NGS_PileupIteratorNext():

FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessBeforeNext_PileupGetReferenceSpec, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );

    REQUIRE_NULL ( NGS_PileupGetReferenceSpec ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessBeforeNext_PileupGetReferencePosition, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );

    NGS_PileupGetReferencePosition ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessBeforeNext_PileupGetPileupEvents, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );

    REQUIRE_NULL ( NGS_PileupGetPileupEvents ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessBeforeNext_PileupGetPileupDepth, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );

    NGS_PileupGetPileupDepth ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    EXIT;
}

// access after a call to NGS_PileupIteratorNext():

FIXTURE_TEST_CASE(CSRA1_PileupIterator_AccessAfterNext_PileupGetReferenceSpec, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );
    REQUIRE_STRING ( "supercont2.1", NGS_PileupGetReferenceSpec ( m_pileup, ctx ) ); 
    EXIT;
}

FIXTURE_TEST_CASE(CSRA1_PileupIterator_AccessAfterNext_PileupGetReferencePosition, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );

    REQUIRE_EQ ( (int64_t)0, NGS_PileupGetReferencePosition ( m_pileup, ctx ) ); // 0 is probably wrong - find out the correct answer
    REQUIRE ( ! FAILED () );
    
    EXIT;
}

FIXTURE_TEST_CASE(CSRA1_PileupIterator_AccessAfterNext_PileupGetPileupEvents, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );

    NGS_PileupEvent * events = NGS_PileupGetPileupEvents ( m_pileup, ctx ); 
    REQUIRE ( ! FAILED () && events );
    
    NGS_PileupEventRelease ( events, ctx );
    EXIT;
}

FIXTURE_TEST_CASE(CSRA1_PileupIterator_AccessAfterNext_PileupGetPileupDepth, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );
    REQUIRE_EQ ( (unsigned int)0, NGS_PileupGetPileupDepth ( m_pileup, ctx ) ); // TODO: verify that 0 is correct
    EXIT;
}

// no access after the end of iteration
#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessAfterEnd_PileupGetReferenceSpec, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );
    while ( NGS_PileupIteratorNext ( m_pileup, ctx ) ) {}

    REQUIRE_NULL ( NGS_PileupGetReferenceSpec ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessAfterEnd_PileupGetReferencePosition, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );
    while ( NGS_PileupIteratorNext ( m_pileup, ctx ) ) {}
    
    NGS_PileupGetReferencePosition ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessAfterEnd_PileupGetPileupEvents, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );
    while ( NGS_PileupIteratorNext ( m_pileup, ctx ) ) {}

    REQUIRE_NULL ( NGS_PileupGetPileupEvents ( m_pileup, ctx ) ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
FIXTURE_TEST_CASE(CSRA1_PileupIterator_NoAccessAfterEnd_PileupGetPileupDepth, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP ( CSRA1_PrimaryOnly, "supercont2.1" );
    while ( NGS_PileupIteratorNext ( m_pileup, ctx ) ) {}

    NGS_PileupGetPileupDepth ( m_pileup, ctx ); 
    REQUIRE_FAILED ();
    
    EXIT;
}
#endif

// regular operation

FIXTURE_TEST_CASE(CSRA1_PileupIterator_PileupGetReferencePosition, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );
    
    REQUIRE_EQ ( (int64_t)0, NGS_PileupGetReferencePosition ( m_pileup, ctx ) ); 
    REQUIRE ( NGS_PileupIteratorNext ( m_pileup, ctx ) );
    REQUIRE_EQ ( (int64_t)1, NGS_PileupGetReferencePosition ( m_pileup, ctx ) ); 
    REQUIRE ( NGS_PileupIteratorNext ( m_pileup, ctx ) );
    REQUIRE_EQ ( (int64_t)2, NGS_PileupGetReferencePosition ( m_pileup, ctx ) ); 
    
    EXIT;
}

FIXTURE_TEST_CASE(CSRA1_PileupIterator_PileupGetPileupDepth, CSRA1_Fixture)
{
    ENTRY_GET_PILEUP_NEXT ( CSRA1_PrimaryOnly, "supercont2.1" );
    
    Advance(85);
    REQUIRE_EQ ( (unsigned int)1, NGS_PileupGetPileupDepth ( m_pileup, ctx ) ); // TODO: verify that 1 is correct
    
    EXIT;
}

//// PileupEvent

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

