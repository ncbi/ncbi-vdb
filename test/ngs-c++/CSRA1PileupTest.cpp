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
* Unit tests for NGS Pileup interface, CSRA1 based implementation
*/

#include <ngs/ncbi/NGS.hpp>

#include <ktst/unit_test.hpp>

#include <sysalloc.h>
#include <assert.h>
#include <memory.h> // memset
#include <stdio.h>

#include <sstream>

using namespace ncbi::NK;

TEST_SUITE(NgsCsra1PileupCppTestSuite);

TEST_CASE(CSRA1_PileupIterator_GetDepth)
{
    char const db_path[] = "SRR341578";

    std::vector <uint32_t> vecDepthSlice, vecDepthEntire, vecRef;

    int64_t const pos_start = 20017;
    uint64_t const len = 5;

    vecRef.push_back(1); // 20017
    vecRef.push_back(0); // 20018
    vecRef.push_back(1); // 20019
    vecRef.push_back(1); // 20020
    vecRef.push_back(3); // 20021

    {
        ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
        ngs::ReferenceIterator ri = run.getReferences ();

        ri.nextReference ();
        ri.nextReference ();

        ngs::PileupIterator pi = ri.getPileups ( ngs::Alignment::primaryAlignment );

        uint64_t ref_pos = 0;
        for (; pi.nextPileup (); ++ ref_pos)
        {
            if ( ref_pos >= (uint64_t)pos_start && ref_pos < (uint64_t)pos_start + len )
                vecDepthEntire.push_back ( pi.getPileupDepth () );
        }
    }
    {
        ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
        ngs::ReferenceIterator ri = run.getReferences ();

        ri.nextReference ();
        ri.nextReference ();

        ngs::PileupIterator pi = ri.getPileupSlice ( pos_start, len, ngs::Alignment::primaryAlignment );

        uint64_t ref_pos = (uint64_t)pos_start;
        for (; pi.nextPileup (); ++ ref_pos)
        {
            if ( ref_pos >= (uint64_t)pos_start && ref_pos < (uint64_t)pos_start + len )
                vecDepthSlice.push_back ( pi.getPileupDepth () );
        }
    }

    REQUIRE_EQ ( vecRef.size(), vecDepthEntire.size() );
    REQUIRE_EQ ( vecRef.size(), vecDepthSlice.size() );

    for ( size_t i = 0; i < (size_t)len; ++i )
    {
        REQUIRE_EQ ( vecRef [i], vecDepthEntire [i] );
        REQUIRE_EQ ( vecRef [i], vecDepthSlice [i] );
    }
}


TEST_CASE(CSRA1_PileupEventIterator_GetType)
{
    char const db_path[] = "SRR341578";

    int64_t const pos_start = 20022;
    uint64_t const len = 1;

    ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
    ngs::ReferenceIterator ri = run.getReferences ();

    ri.nextReference ();
    ri.nextReference ();

    ngs::PileupEvent::PileupEventType arrRefEvents [] =
    {
        (ngs::PileupEvent::PileupEventType)(ngs::PileupEvent::mismatch | ngs::PileupEvent::alignment_minus_strand),
        ngs::PileupEvent::mismatch,
        ngs::PileupEvent::mismatch,
        (ngs::PileupEvent::PileupEventType)(ngs::PileupEvent::mismatch | ngs::PileupEvent::alignment_start),
        (ngs::PileupEvent::PileupEventType)(ngs::PileupEvent::mismatch | ngs::PileupEvent::alignment_minus_strand  | ngs::PileupEvent::alignment_start),
        (ngs::PileupEvent::PileupEventType)(ngs::PileupEvent::mismatch | ngs::PileupEvent::alignment_start)
    };

    ngs::PileupIterator pi = ri.getPileupSlice ( pos_start, len, ngs::Alignment::primaryAlignment );

    for (; pi.nextPileup (); )
    {
        REQUIRE_EQ ( pi.getPileupDepth(), (uint32_t)6 );
        for (size_t i = 0; pi.nextPileupEvent (); ++i)
        {
            REQUIRE_EQ ( pi.getEventType (), arrRefEvents [i] );
        }
    }
}

struct PileupEventStruct
{
    ngs::PileupEvent::PileupEventType event_type;
    ngs::PileupEvent::PileupEventType next_event_type;
    uint32_t repeat_count, next_repeat_count;
    int mapping_quality;
    char alignment_base;
    bool deletion_after_this_pos;
    ngs::String insertion_bases;
};

struct PileupLine
{
    typedef std::vector <PileupEventStruct> TEvents;

    uint32_t depth;
    TEvents vecEvents;
};

void print_line (
    PileupLine const& line,
    char const* name,
    int64_t pos_start,
    int64_t pos,
    ngs::String const& strRefSlice,
    std::ostream& os)
{
    os
        << name
        << "\t" << (pos + 1)    // + 1 to be like sra-pileup - 1-based position
        << "\t" << strRefSlice [pos - pos_start]
        << "\t" << line.depth
        << "\t";

    for (PileupLine::TEvents::const_iterator cit = line.vecEvents.begin(); cit != line.vecEvents.end(); ++ cit)
    {
        PileupEventStruct const& pileup_event = *cit;

        ngs::PileupEvent::PileupEventType eventType = pileup_event.event_type;

        if ( ( eventType & ngs::PileupEvent::alignment_start ) != 0 )
        {
            int32_t c = pileup_event.mapping_quality + 33;
            if ( c > '~' ) { c = '~'; }
            if ( c < 33 ) { c = 33; }

            os << "^" << (char)(c);
        }

        bool reverse = ( eventType & ngs::PileupEvent::alignment_minus_strand ) != 0;

        switch ( eventType & 7 )
        {
        case ngs::PileupEvent::match:
            os << (reverse ? "," : ".");
            break;
        case ngs::PileupEvent::mismatch:
            os
                << (reverse ?
                (char)tolower( pileup_event.alignment_base )
                : (char)toupper( pileup_event.alignment_base ));
            break;
        case ngs::PileupEvent::deletion:
            os << (reverse ? "<" : ">");
            break;
        }

        if ( pileup_event.insertion_bases.size() != 0 )
        {
            bool next_reverse = ( pileup_event.next_event_type & ngs::PileupEvent::alignment_minus_strand ) != 0;
            os
                << "+"
                << pileup_event.insertion_bases.size();

            for ( uint32_t i = 0; i < pileup_event.insertion_bases.size(); ++i )
            {
                os
                    << (next_reverse ?
                    (char)tolower(pileup_event.insertion_bases[i])
                    : (char)toupper(pileup_event.insertion_bases[i]));
            }
        }


        if ( pileup_event.deletion_after_this_pos )
        {
            uint32_t count = pileup_event.next_repeat_count;
            os << "-" << count;

            for ( uint32_t i = 0; i < count; ++i )
            {
                os
                    << (reverse ?
                    (char)tolower(strRefSlice [pos - pos_start + i + 1]) // + 1 means "deletion is at the NEXT position"
                    : (char)toupper(strRefSlice [pos - pos_start + i + 1])); // + 1 means "deletion is at the NEXT position"
            }

        }

        if ( ( eventType & ngs::PileupEvent::alignment_stop ) != 0 )
            os << "$";
    }
    os << std::endl;
}

void clear_line ( PileupLine& line )
{
    line.depth = 0;
    line.vecEvents.clear ();
}

void mark_line_as_starting_deletion ( PileupLine& line, uint32_t repeat_count, size_t alignment_index )
{
    PileupEventStruct& pileup_event = line.vecEvents [ alignment_index ];
    if ( ( pileup_event.event_type & 7 ) != ngs::PileupEvent::deletion)
    {
        pileup_event.next_repeat_count = repeat_count;
        pileup_event.deletion_after_this_pos = true;
    }
}

void mark_line_as_starting_insertion ( PileupLine& line, ngs::String const& insertion_bases, size_t alignment_index, ngs::PileupEvent::PileupEventType next_event_type )
{
    PileupEventStruct& pileup_event = line.vecEvents [ alignment_index ];
    pileup_event.insertion_bases = insertion_bases;
    pileup_event.next_event_type = next_event_type;
}

void mimic_sra_pileup (
            char const* db_path,
            char const* ref_name,
            ngs::Alignment::AlignmentCategory category,
            int64_t const pos_start, uint64_t const len,
            std::ostream& os)
{
    ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
    ngs::Reference r = run.getReference ( ref_name );
    ngs::String const& canonical_name = r.getCanonicalName ();

    // in strRefSlice we want to have bases to report current base and deletions
    // for current base it would be enough to have only slice [pos_start, len]
    // but for deletions we might have situation when we want
    // to report a deletion that goes beyond (pos_start + len) on the reference
    // so we have to read some bases beyond our slice end
    ngs::String strRefSlice = r.getReferenceBases ( pos_start, len + 10000 );

    ngs::PileupIterator pi = r.getPileupSlice ( pos_start, len, category );

    PileupLine line_prev, line_curr;

    // maps current line alignment vector index to
    // previous line alignment vector index
    // mapAlignmentIdx[i] contains adjustment for index, not the absolute value
    std::vector <int64_t> mapAlignmentIdxPrev, mapAlignmentIdxCurr;

    int64_t pos = pos_start;
    for (; pi.nextPileup (); ++ pos)
    {
        line_curr.depth = pi.getPileupDepth ();
        line_curr.vecEvents.reserve (line_curr.depth);
        mapAlignmentIdxCurr.reserve (line_curr.depth);

        int64_t current_stop_count = 0; // number of encountered stops
        bool increased_stop_count = false; // we have increased count (skipped position) on the last step

        for (; pi.nextPileupEvent (); )
        {
            PileupEventStruct pileup_event;

            pileup_event.deletion_after_this_pos = false;
            pileup_event.event_type = pi.getEventType ();

            if ( ( pileup_event.event_type & ngs::PileupEvent::alignment_start ) != 0 )
                pileup_event.mapping_quality = pi.getMappingQuality();

            if ((pileup_event.event_type & 7) == ngs::PileupEvent::mismatch)
                pileup_event.alignment_base = pi.getAlignmentBase();

            if (increased_stop_count)
            {
                if (pileup_event.event_type & ngs::PileupEvent::alignment_stop)
                {
                    ++current_stop_count;
                    mapAlignmentIdxCurr [mapAlignmentIdxCurr.size() - 1] = current_stop_count;
                }
                else
                    increased_stop_count = false;
            }
            else
            {
                if (pileup_event.event_type & ngs::PileupEvent::alignment_stop)
                {
                    ++current_stop_count;
                    increased_stop_count = true;
                }
                mapAlignmentIdxCurr.push_back ( current_stop_count );
            }

            if ( pos != pos_start )
            {
                // here in mapAlignmentIdxPrev we have already initialized
                // indicies for line_prev
                // so we can find corresponding alignment by doing:
                // int64_t idx = line_curr.vecEvents.size()
                // line_prev.vecEvents [ idx + mapAlignmentIdxPrev [idx] ]

                size_t idx_curr_align = line_curr.vecEvents.size();

                if (mapAlignmentIdxPrev.size() > idx_curr_align)
                {
                    if ((pileup_event.event_type & 7) == ngs::PileupEvent::deletion)
                        mark_line_as_starting_deletion ( line_prev, pi.getEventRepeatCount(), mapAlignmentIdxPrev [idx_curr_align] + idx_curr_align );
                    if ( pileup_event.event_type & ngs::PileupEvent::insertion )
                        mark_line_as_starting_insertion ( line_prev, pi.getInsertionBases().toString(), mapAlignmentIdxPrev [idx_curr_align] + idx_curr_align, pileup_event.event_type );
                }
            }

            line_curr.vecEvents.push_back ( pileup_event );
        }

        if ( pos != pos_start ) // there is no line_prev for the first line - nothing to print
        {
            // print previous line
            print_line ( line_prev, canonical_name.c_str(), pos_start, pos - 1, strRefSlice, os );
        }

        line_prev = line_curr;
        mapAlignmentIdxPrev = mapAlignmentIdxCurr;

        clear_line ( line_curr );
        mapAlignmentIdxCurr.clear();
    }
    // TODO: if the last line should contain insertion or deletion start ([-+]<number><seq>)
    // we have to look ahead 1 more position to be able to discover this and
    // modify line_prev, but if the last line is the very last one for the whole
    // reference - we shouldn't do that. This all isn't implemented yet in this function
    print_line ( line_prev, canonical_name.c_str(), pos_start, pos - 1, strRefSlice, os );
}

TEST_CASE(CSRA1_PileupEventIterator_AdjacentIndels)
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

    mimic_sra_pileup ( db_path, ref_name, ngs::Alignment::primaryAlignment, pos_start, len, sstream );
}

TEST_CASE(CSRA1_PileupEventIterator_DeletionAndEnding)
{

    //sra-pileup SRR497541 -r "Contig307.Contig78.Contig363_2872688_2872915.Contig307.Contig78.Contig363_1_2872687":106436-106438 -s -n
    // There should be no "g-3taa$" for the position 106438, only "g$" must be there

    char const db_path[] = "SRR497541";
    char const ref_name[] = "Contig307.Contig78.Contig363_2872688_2872915.Contig307.Contig78.Contig363_1_2872687";

    int64_t const pos_start = 106436-1;
    int64_t const pos_end = 106438 - 1;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << ref_name << "\t106436\tT\t106\tC$C$,$.,$,$,$,$.-5TATAA.-5TATAA,-5tataa.-5TATAA.-5TATAA.$.-5TATAA.-5TATAA,-5tataa.-5TATAA,-5tataa,-5tataa.-5TATAA,-5tataa,-5tataa.-5TATAA,-5tataa,-5tataa,-5tataa,-5tataa.-5TATAA.-5TATAA,-5tataa.-5TATAA,-5tataa,$.-5TATAA,-5tataa,,-5tataa,$.-5TATAA,-5tataa,-5tataa.-5TATAA,-5tataa.$.$.-5TATAA.-5TATAA.-5TATAA.-5TATAA,-5tataa.-5TATAA.-5TATAA.-5TATAA.-5TATAA.-5TATAA.-5TATAA.-5TATAA,-5tataa.-5TATAA,-5tataa,-5tataa,-5tataa.-5TATAA.$,-5tataa.-5TATAA.-5TATAA.-5TATAA.-5TATAA,-5tataa,-5tataa,-5tataa.-5TATAA,-5tataa,-5tataa.-5TATAA,-5tataa,-5tataa.-5TATAA.-5TATAA.$.-5TATAA,-5tataa.-5TATAA.-5TATAA.-5TATAA.-5TATAA,-5tataa,-5tataa.-5TATAA,-5tataa.-5TATAA.-5TATAA.-5TATAA,-5tataa,-5tataa,-5tataa,-5tataa,-5tataa,-5tataa.-5TATAA.-5TATAA,-5tataa,-5tataa^]g" << std::endl;
    sstream_ref << ref_name << "\t106437\tT\t92\tC$>><>>>><><<><<><<<<>><><><c<><<><>>>><>>>>>>><><<<><>>>><<<><<><<>>><>>>><<><>>><<<<<<>><<," << std::endl;
    sstream_ref << ref_name << "\t106438\tA\t91\t>><>>>><><<><<><<<<>><><><g$<><<><>>>><>>>>>>><><<<><>>>><<<><<><<>>><>>>><<><>>><<<<<<>><<," << std::endl;

    mimic_sra_pileup ( db_path, ref_name, ngs::Alignment::primaryAlignment, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

TEST_CASE(CSRA1_PileupEventIterator_LongDeletions)
{

    //SRR1113221 "chr4" 10204 10208
    // 1. At the position 10205 we want to have the full lenght (6) deletion
    // 2. Previously we had a misplaced insertion at the position 10204 (+3att)

    char const db_path[] = "SRR1113221";

    int64_t const pos_start = 10204-1;
    int64_t const pos_end = 10208 - 1;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    // The output must be the same as for "sra-pileup SRR1113221 -r "chr4":10204-10208 -s -n"
    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "NC_000004.11\t10204\tT\t37\ta,$c+2tc,,+3att.$.$.+2TT.,,.......,..<......,...g...^!." << std::endl;
    sstream_ref << "NC_000004.11\t10205\tC\t37\t,$,t+3ttt,..,t.......,A.<....-6TCGGCT.-6TCGGCT.,...,....^$,^!.^$." << std::endl;
    sstream_ref << "NC_000004.11\t10206\tT\t40\t,,,$..a$g$....$.$N.,..,...-6CGGCTG>>.,...-6CGGCTGa....,..^$.^$.^%.^$." << std::endl;
    sstream_ref << "NC_000004.11\t10207\tC\t40\t,$,$.$.$.....,..,..>>>.,..>t-6ggctgc...+2TC.,......^$.^$.^$.^$.^A." << std::endl;
    sstream_ref << "NC_000004.11\t10208\tG\t39\t.....,..,..>>>.,..><....,...........^$,^M.^$." << std::endl;

    mimic_sra_pileup ( db_path, "chr4", ngs::Alignment::primaryAlignment, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}


TEST_CASE(CSRA1_PileupEventIterator_Deletion)
{
    // deletions
    char const db_path[] = "SRR341578";

    int64_t const pos_start = 2427;
    int64_t const pos_end = 2428;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    // The output must be the same as for "sra-pileup SRR341578 -r NC_011752.1:2428-2429 -s -n"
    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "NC_011752.1\t2428\tG\t34\t..,.,.-1A.-1A.-1A.-1A.-1A.-1A,-1a,-1a.-1A,-1a,-1a.-1A.-1A.-1A,-1a,-1a,-1a,-1a.-1A,-1a,-1a.-1A,-1a.-1A.-1A,-1a.^F,^F," << std::endl;
    sstream_ref << "NC_011752.1\t2429\tA\t34\t.$.$,$.,>>>>>><<><<>>><<<<><<><>><Ggg" << std::endl;

    mimic_sra_pileup ( db_path, "gi|218511148|ref|NC_011752.1|", ngs::Alignment::all, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

TEST_CASE(CSRA1_PileupEventIterator_Insertion)
{
    // simple matches, mismatch, insertion, mapping quality
    char const db_path[] = "SRR341578";

    int64_t const pos_start = 2017;
    int64_t const pos_end = 2018;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    // The output must be the same as for "sra-pileup SRR341578 -r NC_011752.1:2018-2019 -s -n"
    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "NC_011752.1\t2018\tT\t17\t.....,,A,,..+2CA..^F.^F.^:N" << std::endl;
    sstream_ref << "NC_011752.1\t2019\tC\t19\t.....,,.,,.......^F.^F," << std::endl;

    mimic_sra_pileup ( db_path, "gi|218511148|ref|NC_011752.1|", ngs::Alignment::all, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

TEST_CASE(CSRA1_PileupEventIterator_TrickyInsertion)
{
    // the insertion occurs in 1 or more previous chunks but not the current

    char const db_path[] = "SRR341578";

    int64_t const pos_start = 380000;
    int64_t const pos_end = 380001;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    // The output must be the same as for "sra-pileup SRR341578 -r NC_011748.1:380001-380002 -s -n"
    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "NC_011748.1\t380001\tT\t61\t....,,...,......,,...,,.....,,..,.,,,,...,,,,,,,+2tc.,.....G....," << std::endl;
    sstream_ref << "NC_011748.1\t380002\tT\t61\t.$.$.$.$,$,$...,......,,...,,.....,,A.,.,,,,...,,,,,,,.,.....G....," << std::endl;

    mimic_sra_pileup ( db_path, "gi|218693476|ref|NC_011748.1|", ngs::Alignment::primaryAlignment, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}


TEST_CASE(CSRA1_PileupIterator_StartingZeros)
{
    // this is transition from depth == 0 to depth == 1
    // initial code had different output for primaryAlignments vs all

    int64_t const pos_start = 19374;
    int64_t const pos_end = 19375;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1); //3906625;

    // when requesting category == all, the output must be the same as with
    // primaryAlignments
    // reference output: sra-pileup SRR833251 -r "gi|169794206|ref|NC_010410.1|":19375-19376 -s -n

    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "gi|169794206|ref|NC_010410.1|\t19375\tC\t0\t" << std::endl;
    sstream_ref << "gi|169794206|ref|NC_010410.1|\t19376\tA\t1\t^!." << std::endl;

    mimic_sra_pileup ( "SRR833251", "gi|169794206|ref|NC_010410.1|", ngs::Alignment::all, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

TEST_CASE(CSRA1_PileupIterator_MapQuality)
{
    // different mapping quality
    // there was a bug caused by usage of char c instead int32_t c
    // in printing code inside mimic_sra_pileup

    int64_t const pos_start = 183830-1;
    int64_t const pos_end = pos_start;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "gi|169794206|ref|NC_010410.1|\t183830\tA\t1\t^~," << std::endl;

    mimic_sra_pileup ( "SRR833251", "gi|169794206|ref|NC_010410.1|", ngs::Alignment::all, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

TEST_CASE(CSRA1_PileupIterator_Depth)
{
    // if ngs::Alignment::all is used here
    // there will be discrepancy with sra-pileup

    int64_t const pos_start = 519533-1;
    int64_t const pos_end = pos_start;
    uint64_t const len = (uint64_t)(pos_end - pos_start + 1);

    std::ostringstream sstream;
    std::ostringstream sstream_ref;

    sstream_ref << "gi|169794206|ref|NC_010410.1|\t519533\tC\t1\t," << std::endl;

    mimic_sra_pileup ( "SRR833251", "gi|169794206|ref|NC_010410.1|", ngs::Alignment::primaryAlignment, pos_start, len, sstream );

    REQUIRE_EQ ( sstream.str (), sstream_ref.str () );
}

uint64_t pileup_test_all_functions (
            char const* db_path,
            char const* ref_name,
            ngs::Alignment::AlignmentCategory category,
            int64_t const pos_start, uint64_t const len)
{
    uint64_t ret = 0;

    ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
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

TEST_CASE(CSRA1_PileupIterator_TestAllFunctions)
{
    uint64_t ret = 0;
    ret = pileup_test_all_functions ( "SRR822962", "chr2"/*"NC_000002.11"*/, ngs::Alignment::all, 0, 20000 );
    REQUIRE_EQ ( ret, (uint64_t)46433887435 );
}

/////////////////////////////////////////
// Experimenting with Smith-Waterman

#ifndef min
#define min(x,y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x,y) ((y) >= (x) ? (y) : (x))
#endif

#define max4(x1, x2, x3, x4) (max( max((x1),(x2)), max((x3),(x4)) ))

unsigned char const map_char_to_4na [256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1,14, 2,13, 0, 0, 4,11, 0, 0,12, 0, 3,15, 0,
    0, 0, 5, 6, 8, 0, 7, 9, 0,10, 0, 0, 0, 0, 0, 0,
    0, 1,14, 2,13, 0, 0, 4,11, 0, 0,12, 0, 3,15, 0,
    0, 0, 5, 6, 8, 0, 7, 9, 0,10, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int compare_4na ( char ch2na, char ch4na )
{
    unsigned char bits4na = map_char_to_4na [(int)ch4na];
    unsigned char bits2na = map_char_to_4na [(int)ch2na];

    //return (bits2na & bits4na) != 0 ? 2 : -1;

    unsigned char popcnt4na;
    // TODO: optimize, maybe using _popcnt
    switch ( bits4na )
    {
    case 1:
    case 2:
    case 4:
    case 8:
        popcnt4na = 1;
        break;
    case 7:
    case 11:
    case 13:
    case 14:
        popcnt4na = 3;
        break;
    case 15:
        popcnt4na = 4;
        break;
    case 0:
        popcnt4na = 0;
        break;
    //case 3:
    //case 5:
    //case 6:
    //case 9:
    //case 10:
    //case 12:
    //    popcnt4na = 2;
    //    break;
    default:
        popcnt4na = 2;
        break;
    }

    return (bits2na & bits4na) != 0 ? 12 / popcnt4na : -6;
}

#define COMPARE_4NA 0

int similarity_func (char ch2na, char ch4na)
{
#if COMPARE_4NA == 1
    return compare_4na ( ch2na, ch4na );
#else
    return ch2na == ch4na ? 2 : -1;
#endif
}

int gap_score_func ( size_t idx )
{
#if COMPARE_4NA == 1
    return -6*(int)idx;
#else
    return -(int)idx;
#endif
}


#define CACHE_MAX_COLS 1
#define CACHE_MAX_ROWS 1


template <bool t_reverse> class CStringIterator
{
    char const* m_arr;
    size_t m_size;
public:
    CStringIterator (char const* arr, size_t size)
        : m_arr(arr), m_size(size)
    {
    }

    char const& operator[] (size_t i) const;
    size_t get_straight_index (size_t logical_index);
};

template <> char const& CStringIterator<false>::operator[] (size_t i) const
{
    return m_arr [i];
}
template <> char const& CStringIterator<true>::operator[] (size_t i) const
{
    return m_arr [m_size - i - 1];
}
template <> size_t CStringIterator<false>::get_straight_index (size_t logical_index)
{
    return logical_index;
}
template <> size_t CStringIterator<true>::get_straight_index (size_t logical_index)
{
    return m_size - logical_index - 1;
}


template <bool reverse> void calculate_similarity_matrix (
    char const* text, size_t size_text,
    char const* query, size_t size_query,
    int* matrix)
{
    size_t ROWS = size_text + 1;
    size_t COLUMNS = size_query + 1;

    // init 1st row and column with zeros
    memset ( matrix, 0, COLUMNS * sizeof(matrix[0]) );
    for ( size_t i = 1; i < ROWS; ++i )
        matrix [i * COLUMNS] = 0;

    // arrays to store maximums for all previous rows and columns
#ifdef CACHE_MAX_COLS
    typedef std::pair<int, size_t> TMaxPos;
    std::vector<TMaxPos> vec_max_cols(COLUMNS, TMaxPos(0, 0));
    std::vector<TMaxPos> vec_max_rows(ROWS, TMaxPos(0, 0));
#endif

    CStringIterator<reverse> text_iterator(text, size_text);
    CStringIterator<reverse> query_iterator(query, size_query);

    for ( size_t i = 1; i < ROWS; ++i )
    {
        for ( size_t j = 1; j < COLUMNS; ++j )
        {
            int sim = similarity_func ( text_iterator[i-1], query_iterator[j-1] );

#ifdef CACHE_MAX_COLS
            int cur_score_del = vec_max_cols[j].first + gap_score_func(j - vec_max_cols[j].second);
#else
            int cur_score_del = -1;
            for ( size_t k = 1; k < i; ++k )
            {
                int cur = matrix [ (i - k)*COLUMNS + j ] + gap_score_func(k);
                if ( cur > cur_score_del )
                    cur_score_del = cur;
            }
#endif

#ifdef CACHE_MAX_ROWS
            int cur_score_ins = vec_max_rows[i].first + gap_score_func(i - vec_max_rows[i].second);;
#else
            int cur_score_ins = -1;
            for ( size_t l = 1; l < j; ++l )
            {
                int cur = matrix [ i*COLUMNS + (j - l) ] + gap_score_func(l);
                if ( cur > cur_score_ins )
                    cur_score_ins = cur;
            }
#endif

            matrix[i*COLUMNS + j] = max4 (0,
                                          matrix[(i-1)*COLUMNS + j - 1] + sim,
                                          cur_score_del,
                                          cur_score_ins);

#ifdef CACHE_MAX_COLS
            if ( matrix[i*COLUMNS + j] > vec_max_cols[j].first )
                vec_max_cols[j] = TMaxPos(matrix[i*COLUMNS + j], j);

            vec_max_cols[j].first += gap_score_func(1);
#endif
#ifdef CACHE_MAX_ROWS
            if ( matrix[i*COLUMNS + j] > vec_max_rows[i].first )
                vec_max_rows[i] = TMaxPos(matrix[i*COLUMNS + j], i);

            vec_max_rows[i].first += gap_score_func(1);
#endif

        }
    }

}

void sw_find_indel_box ( int* matrix, size_t ROWS, size_t COLUMNS,
                         int* ret_row_start, int* ret_row_end,
                         int* ret_col_start, int* ret_col_end )
{
    // find maximum score in the matrix
    size_t max_row = 0, max_col = 0;
    size_t max_i = 0;

    size_t i = ROWS*COLUMNS - 1;
    //do
    //{
    //    if ( matrix[i] > matrix[max_i] )
    //        max_i = i;
    //    --i;
    //}
    //while (i > 0);

    // TODO: prove the lemma: for all i: matrix[i] <= matrix[ROWS*COLUMNS - 1]
    // (i.e. matrix[ROWS*COLUMNS - 1] is always the maximum element in the valid SW-matrix)

    max_i = ROWS*COLUMNS - 1;

    max_row = max_i / COLUMNS;
    max_col = max_i % COLUMNS;


    // traceback to (0,0)-th element of the matrix
    *ret_row_start = *ret_row_end = *ret_col_start = *ret_col_end = -1;

    i = max_row;
    size_t j = max_col;
    bool prev_indel = false;
    while (true)
    {
        if (i > 0 && j > 0)
        {
            if ( matrix [(i - 1)*COLUMNS + (j - 1)] >= matrix [i*COLUMNS + (j - 1)] &&
                 matrix [(i - 1)*COLUMNS + (j - 1)] >= matrix [(i - 1)*COLUMNS + j])
            {
                --i;
                --j;

                if (prev_indel)
                {
                    *ret_row_start = i;
                    *ret_col_start = j;
                }
                prev_indel = false;
            }
            else if ( matrix [(i - 1)*COLUMNS + (j - 1)] < matrix [i*COLUMNS + (j - 1)] )
            {
                if ( *ret_row_end == -1 )
                {
                    *ret_row_end = i;
                    *ret_col_end = j;
                }
                --j;
                prev_indel = true;
            }
            else
            {
                if ( *ret_row_end == -1 )
                {
                    *ret_row_end = i;
                    *ret_col_end = j;
                }
                --i;
                prev_indel = true;
            }
        }
        else if ( i > 0 )
        {
            if ( *ret_row_end == -1 )
            {
                *ret_row_end = i;
                *ret_col_end = 0;
            }
            *ret_row_start = 0;
            *ret_col_start = 0;
            break;
        }
        else if ( j > 0 )
        {
            if ( *ret_row_end == -1 )
            {
                *ret_row_end = 0;
                *ret_col_end = j;
            }
            *ret_row_start = 0;
            *ret_col_start = 0;
            break;
        }
        else
        {
            break;
        }

    }
}

// get_ref_slice returns reference slice of sufficient length for Smith-Waterman algorithm
ngs::String get_ref_slice ( ngs::Reference const& ref,
                            int64_t ref_pos_ins,
                            int64_t ins_bases_length)
{
    int64_t safe_half_length = ins_bases_length / 2 + 1;

    int64_t ref_start = (ref_pos_ins - safe_half_length) >= 0 ?
                        (ref_pos_ins - safe_half_length) : 0;

    return ref.getReferenceBases ( ref_start, safe_half_length * 2 );
}

// make_query returns the query for Smith-Waterman algorithm as follows:
// <1st half of reference slice> + <insertion bases> + <2nd half of the reference slice>
// reference slice must be of sufficient length - get_ref_slice() retruns it
ngs::String make_query ( ngs::String const& ref_slice,
                         char const* ins_bases, size_t ins_bases_length)
{
    assert ( ref_slice.size() >= 2 * ( ins_bases_length / 2 + 1 ) );

    ngs::String ret;
    ret.reserve ( ref_slice.size() + ins_bases_length );

    ret.append ( ref_slice.begin(), ref_slice.begin() + ref_slice.size()/2 );
    ret.append ( ins_bases, ins_bases + ins_bases_length );
    ret.append ( ref_slice.begin() + ref_slice.size()/2, ref_slice.end() );

    return ret;
}

template <bool reverse> void print_matrix ( int const* matrix,
                                            char const* ref_slice, size_t ref_slice_size,
                                            char const* query, size_t query_size)
{
    size_t COLUMNS = ref_slice_size + 1;
    size_t ROWS = query_size + 1;

    int print_width = 2;

    CStringIterator<reverse> ref_slice_iterator(ref_slice, ref_slice_size);
    CStringIterator<reverse> query_iterator(query, query_size);

    printf ("  %*c ", print_width, '-');
    for (size_t j = 0; j < COLUMNS; ++j)
        printf ("%*c ", print_width, ref_slice_iterator[j]);
    printf ("\n");

    for (size_t i = 0; i < ROWS; ++i)
    {
        if ( i == 0 )
            printf ("%c ", '-');
        else
            printf ("%c ", query_iterator[i-1]);
        
        for (size_t j = 0; j < COLUMNS; ++j)
        {
            printf ("%*d ", print_width, matrix[i*COLUMNS + j]);
        }
        printf ("\n");
    }
}

void print_indel (char const* name, char const* text, size_t text_size, int indel_start, int indel_end)
{
    printf ( "%s: %.*s[%.*s]%.*s\n",
                name,
                (indel_start + 1), text,
                indel_end - (indel_start + 1), text + (indel_start + 1),
                (int)(text_size - indel_end), text + indel_end
           );
}

void analyse_run ( ngs::String const& ref_slice, char const* ins_bases, size_t ins_bases_length )
{
    ngs::String query = make_query ( ref_slice, ins_bases, ins_bases_length );

    std::cout
        << "ref_slice: "
        << ref_slice << std::endl
        << "query    : " << query << std::endl;

    // building sw-matrix for chosen reference slice and sequence

    size_t COLUMNS = ref_slice.size() + 1;
    size_t ROWS = query.size() + 1;
    std::vector<int> matrix( ROWS * COLUMNS );

    calculate_similarity_matrix<false> ( query.c_str(), query.size(), ref_slice.c_str(), ref_slice.size(), &matrix[0] );
    //print_matrix<reverse> (&matrix[0], ref_slice.c_str(), ref_slice.size(), query.c_str(), query.size());
    int row_start, col_start, row_end, col_end;
    sw_find_indel_box ( & matrix[0], ROWS, COLUMNS, &row_start, &row_end, &col_start, &col_end );


    calculate_similarity_matrix<true> ( query.c_str(), query.size(), ref_slice.c_str(), ref_slice.size(), &matrix[0] );
    int row_start_rev, col_start_rev, row_end_rev, col_end_rev;
    sw_find_indel_box ( & matrix[0], ROWS, COLUMNS, &row_start_rev, &row_end_rev, &col_start_rev, &col_end_rev );

    CStringIterator<false> ref_slice_iterator(ref_slice.c_str(), ref_slice.size());
    CStringIterator<false> query_iterator(query.c_str(), query.size());

    row_start = min ( (int)query.size() - row_end_rev - 1, row_start );
    row_end   = max ( (int)query.size() - row_start_rev - 1, row_end );
    col_start = min ( (int)ref_slice.size() - col_end_rev - 1, col_start );
    col_end   = max ( (int)ref_slice.size() - col_start_rev - 1, col_end );

    printf ("indel box found: (%d, %d) - (%d, %d)\n", row_start, col_start, row_end, col_end );

    print_indel ( "reference", ref_slice.c_str(), ref_slice.size(), col_start, col_end );
    print_indel ( "query    ", query.c_str(), query.size(), row_start, row_end );
}

void find_ref_in_runs (char const* ref_name, int64_t pos)
{
    //"SRR341578" "gi|218511148|ref|NC_011752.1|" 2018
    ngs::ReadCollection run = ncbi::NGS::openReadCollection ("SRR341578");
    ngs::Reference ref = run.getReference ( /*"gi|218511148|ref|*/ref_name );

    ngs::String ref_slice = get_ref_slice ( ref, pos, 2 );

    std::cout << "Reference around pos=" << pos << ": " << ref_slice << std::endl;

    char const* db_names[] =
    {
        "SRR341575",
        "SRR341576",
        "SRR341577",
        "SRR341579",
        "SRR341580",
        "SRR341581",
        "SRR341582",
        "SRR341578"
    };

    for (size_t i = 0; i < sizeof (db_names)/sizeof (db_names[0]); ++i )
    {
        try
        {
            ngs::ReadCollection r = ncbi::NGS::openReadCollection ( db_names[i] );
            ngs::Reference rr = r.getReference ( ref_name );
            ngs::String ref_slice_cur = get_ref_slice ( rr, pos, 2 );

            if ( ref_slice_cur ==  ref_slice )
            {
                std::cout << db_names[i] << " has the same reference" << std::endl;
                analyse_run( ref_slice_cur, "CA", 2 );
            }
            else
            {
                std::cout << db_names[i] << " has DIFFERENT reference: " << ref_slice_cur << std::endl;
            }
        }
        catch (ngs::ErrorMsg const& e)
        {
            std::cout << db_names[i] << " failed: " << e.what() << std::endl;
        }
    }
}

TEST_CASE(CSRA1_Experimenting)
{
    // given: ref_name, position, insertion bases at this position
    // to find: the real insertion position and bases

    find_ref_in_runs ( "NC_011752.1", 2018 );

    char const ref_name[] = "chr2";
    int64_t ref_pos_ins = 100689;
    char const insertion_bases[] = "AGTCAA";
    char const db_path[] = "SRR822962";

    ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
    ngs::Reference ref = run.getReference ( ref_name );
    
    ngs::String ref_slice = get_ref_slice ( ref, ref_pos_ins, sizeof (insertion_bases) - 1 );
    ngs::String query = make_query ( ref_slice, insertion_bases, sizeof (insertion_bases) - 1 );

    ref_slice = "TTGGACGGTT";
    query = "TTGGACACGGTT";

    std::cout
        << "ref_slice: "
        << ref_slice << std::endl
        << "query    : " << query << std::endl;

    // building sw-matrix for chosen reference slice and sequence

    size_t COLUMNS = ref_slice.size() + 1;
    size_t ROWS = query.size() + 1;
    std::vector<int> matrix( ROWS * COLUMNS );

    {
        calculate_similarity_matrix<false> ( query.c_str(), query.size(), ref_slice.c_str(), ref_slice.size(), &matrix[0] );

        //print_matrix<false> ( & matrix[0], ref_slice.c_str(), ref_slice.size(), query.c_str(), query.size() );

        int row_start, col_start, row_end, col_end;
        sw_find_indel_box ( & matrix[0], ROWS, COLUMNS, &row_start, &row_end, &col_start, &col_end );

        printf ("indel box found: (%d, %d) - (%d, %d)\n", row_start, col_start, row_end, col_end );

        print_indel ( "reference", ref_slice.c_str(), ref_slice.size(), col_start, col_end );
        print_indel ( "query    ", query.c_str(), query.size(), row_start, row_end );
    }
    {
        std::cout << "and reverse..." << std::endl;
        calculate_similarity_matrix<true> ( query.c_str(), query.size(), ref_slice.c_str(), ref_slice.size(), &matrix[0] );

        int row_start, col_start, row_end, col_end;
        sw_find_indel_box ( & matrix[0], ROWS, COLUMNS, &row_start, &row_end, &col_start, &col_end );

        CStringIterator<true> query_iterator ( query.c_str(), query.size() );
        CStringIterator<true> ref_slice_iterator ( ref_slice.c_str(), ref_slice.size() );

        row_start = query_iterator.get_straight_index ( row_start );
        row_end = query_iterator.get_straight_index ( row_end );
        col_start = ref_slice_iterator.get_straight_index ( col_start );
        col_end = ref_slice_iterator.get_straight_index ( col_end );

        std::swap ( row_start, row_end );
        std::swap ( col_start, col_end );


        printf ("indel box found: (%d, %d) - (%d, %d)\n", row_start, col_start, row_end, col_end );

        print_indel ( "reference", ref_slice.c_str(), ref_slice.size(), col_start, col_end );
        print_indel ( "query    ", query.c_str(), query.size(), row_start, row_end );
    }

    REQUIRE_EQ ( 0, 0 );
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

const char UsageDefaultName[] = "test-ngs_csra1pileup-c++";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=NgsCsra1PileupCppTestSuite(argc, argv);
    return rc;
}

}

