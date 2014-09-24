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

#include <klib/defs.h>
#include <klib/container.h>
#include <klib/data-buffer.h>

#define COUNT_INDELS 0

typedef struct row_input
{
    char const * spotgroup;
    uint32_t spotgroup_len;

    uint8_t const * read;
    uint32_t read_len;

    uint8_t const * quality;
    uint32_t quality_len;

    uint32_t base_pos_offset;
} row_input;


typedef struct stat_row
{
    char const *spotgroup;
    char const *dimer;
    uint32_t base_pos;
    uint32_t count;
    uint8_t quality;
    uint8_t hp_run;
    uint8_t gc_content;
} stat_row;


typedef struct statistic
{
    void * last_used_spotgroup;
    uint32_t gc_window;
    BSTree spotgroups;      /* the tree contains 'spotgrp'-node, it collects the statistic */
} statistic;


/*************** the STATISTIC GATHERER ***************/
rc_t make_statistic( statistic *data,
                     uint32_t gc_window,
                     bool ignore_mismatches );


rc_t extract_statistic_from_row( statistic * data, 
                                 row_input const *row_data);

uint64_t foreach_statistic( statistic * data,
    bool ( CC * f ) ( stat_row const * row, void * f_data ), void *f_data );

void whack_statistic( statistic *data );
