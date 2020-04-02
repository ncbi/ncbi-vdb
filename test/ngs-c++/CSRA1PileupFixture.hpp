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

#pragma once

/**
* Test fixture for NGS Pileup interface, CSRA1 based implementation
*/

#include "ngsfixture.hpp"

class CSRA1PileupFixture : public NgsFixture
{
public:
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
        std::ostream& os);

    void clear_line ( PileupLine& line );

    void mark_line_as_starting_deletion ( PileupLine& line, uint32_t repeat_count, size_t alignment_index );

    void mark_line_as_starting_insertion ( PileupLine& line, ngs::String const& insertion_bases, size_t alignment_index, ngs::PileupEvent::PileupEventType next_event_type );

    void mimic_sra_pileup (
                ngs::ReadCollection run,
                char const* ref_name,
                ngs::Alignment::AlignmentCategory category,
                int64_t const pos_start, uint64_t const len,
                std::ostream& os);
};



