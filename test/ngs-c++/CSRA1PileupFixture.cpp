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

#include "CSRA1PileupFixture.hpp"

// #include <kfg/config.h> /* KConfigDisableUserSettings */

// #include <ngs/ncbi/NGS.hpp>

// #include <ktst/unit_test.hpp>

// #include <sysalloc.h>
// #include <assert.h>
// #include <memory.h> // memset
// #include <stdio.h>

// #include <sstream>

void
CSRA1PileupFixture :: print_line (
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

void
CSRA1PileupFixture :: clear_line ( PileupLine& line )
{
    line.depth = 0;
    line.vecEvents.clear ();
}

void
CSRA1PileupFixture :: mark_line_as_starting_deletion ( PileupLine& line, uint32_t repeat_count, size_t alignment_index )
{
    PileupEventStruct& pileup_event = line.vecEvents [ alignment_index ];
    if ( ( pileup_event.event_type & 7 ) != ngs::PileupEvent::deletion)
    {
        pileup_event.next_repeat_count = repeat_count;
        pileup_event.deletion_after_this_pos = true;
    }
}

void
CSRA1PileupFixture :: mark_line_as_starting_insertion ( PileupLine& line, ngs::String const& insertion_bases, size_t alignment_index, ngs::PileupEvent::PileupEventType next_event_type )
{
    PileupEventStruct& pileup_event = line.vecEvents [ alignment_index ];
    pileup_event.insertion_bases = insertion_bases;
    pileup_event.next_event_type = next_event_type;
}

void
CSRA1PileupFixture :: mimic_sra_pileup (
            ngs::ReadCollection run,
            char const* ref_name,
            ngs::Alignment::AlignmentCategory category,
            int64_t const pos_start, uint64_t const len,
            std::ostream& os)
{
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

