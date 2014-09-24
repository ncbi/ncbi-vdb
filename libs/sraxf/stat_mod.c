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
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stat_mod.h"

#define N_QUAL_VALUES 41
#define N_DIMER_VALUES 17
#define N_GC_VALUES 16
#define N_HP_VALUES 25
#define COUNTER_BLOCK_SIZE 100

#define CASE_MATCH      0
#define CASE_IGNORE     1
#define CASE_MISMATCH   2
#define CASE_INSERT     4
#define CASE_DELETE     8

typedef struct counter
{
    uint32_t count;
} counter;


typedef struct counter_vector
{
    counter *v;
    uint32_t n_counters;
} counter_vector;


typedef struct spotgrp
{
    BSTNode node;
    const String *name;
    counter_vector cnv[ N_DIMER_VALUES ][ N_GC_VALUES ][ N_HP_VALUES ][ N_QUAL_VALUES ];
} spotgrp;

/******************************************************************************
    for the spot-group ( tree-node ), contains a tree of counter's
******************************************************************************/
static void CC whack_spotgroup( BSTNode *n, void *data )
{
    spotgrp * sg = ( spotgrp * )n;
    uint32_t idx, count;
    count = ( ( sizeof sg->cnv ) / sizeof( sg->cnv[0] ) );
    for ( idx = 0; idx < count; ++idx )
    {
        counter_vector * cv = (counter_vector *)&( sg->cnv[ idx ] );
        if ( cv->v != NULL )
        {
            free( cv->v );
        }
    }
    if ( sg->name != NULL )
        StringWhack ( sg->name );
    free( n );
}


static spotgrp * make_spotgrp( const char *src, const size_t len )
{
    spotgrp * sg = calloc( 1, sizeof sg[ 0 ] );
    if ( sg != NULL )
    {
        String s;
        StringInit( &s, src, len, len );
        if ( StringCopy ( &sg->name, &s ) != 0 )
        {
            free( sg );
            sg = NULL;
        }
    }
    return sg;
}


static int CC spotgroup_find( const void *item, const BSTNode *n )
{
    spotgrp * sg = ( spotgrp* ) n;
    return StringCompare ( ( String* ) item, sg->name );
}


static spotgrp * find_spotgroup( statistic *self, const char *src, const size_t len )
{
    String s;
    BSTNode *node;

    StringInit( &s, src, len, len );
    if ( self->last_used_spotgroup != NULL )
    {
        spotgrp * sg = ( spotgrp* )self->last_used_spotgroup;
        if ( StringCompare ( &s, sg->name ) == 0 )
            return sg;
    }

    node = BSTreeFind ( &self->spotgroups, &s, spotgroup_find );
    if ( node == NULL )
        return NULL;
    else
    {
        self->last_used_spotgroup = node;
        return ( spotgrp *) node;
    }
}


static rc_t spotgroup_enter_values( spotgrp * spotgroup,
                                    uint8_t const quality,
                                    uint8_t const dimer_code,
                                    uint8_t const gc_content,
                                    uint8_t const hp_run,
                                    uint32_t const cycle,
                                    uint8_t const rd_case)
{
    uint8_t q = quality;
    uint8_t d = dimer_code;
    uint8_t g = gc_content;
    uint8_t h = hp_run;
    counter_vector * cv;

    if ( q >= N_QUAL_VALUES ) q = ( N_QUAL_VALUES - 1 );
    if ( d >= N_DIMER_VALUES ) d = ( N_DIMER_VALUES - 1 );
    if ( g >= N_GC_VALUES ) g = ( N_GC_VALUES - 1 );
    if ( h >= N_HP_VALUES ) h = ( N_HP_VALUES - 1 );
    cv = &( spotgroup->cnv[ d ][ g ][ h ][ q ] );

    if ( cv->v ==  NULL )
    {
        /* the counter-block was not used before at all */
        cv->n_counters = ( ( cycle / COUNTER_BLOCK_SIZE ) + 1 ) * COUNTER_BLOCK_SIZE;
        cv->v = calloc( cv->n_counters, sizeof cv->v[0] );
        if ( cv->v == NULL )
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted );
    }
    else
    {
        if ( cycle >= cv->n_counters )
        {
            /* the counter-block has to be extended */
            void * tmp;
            uint32_t org_len = cv->n_counters;
            counter *to_zero_out;
            
            cv->n_counters = ( ( cycle / COUNTER_BLOCK_SIZE ) + 1 ) * COUNTER_BLOCK_SIZE;
            /* prevent from leaking memory by capturing the new pointer in temp. var. */
            tmp = realloc( cv->v, cv->n_counters * ( sizeof cv->v[0] ) );
            if ( tmp == NULL )
	            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted );

			/* the added part has to be set to zero */
			to_zero_out = tmp;
			to_zero_out += org_len;
			memset( to_zero_out, 0, ( cv->n_counters - org_len ) * ( sizeof *to_zero_out ) );
			cv->v = tmp;
        }
    }
    assert( cycle < cv->n_counters );

    {
        counter * cnt = &( cv->v[ cycle ] );
        
        ++cnt->count;
    }
    return 0;
}


static int CC spotgroup_sort( const BSTNode *item, const BSTNode *n )
{
    spotgrp * sg1 = ( spotgrp* ) item;
    spotgrp * sg2 = ( spotgrp* ) n;
    return StringCompare ( sg1->name, sg2->name );
}


/******************************************************************************
    for the statistic ( tree-node ), contains a tree of spot-groups's
******************************************************************************/


rc_t make_statistic( statistic *self,
                     uint32_t gc_window,
                     bool has_alignments )
{
    memset( self, 0, sizeof *self );
    
    BSTreeInit( &self->spotgroups );
    self->last_used_spotgroup = NULL;
    self->gc_window = gc_window > N_GC_VALUES ? N_GC_VALUES : gc_window;
    
    return 0;
}


void whack_statistic( statistic *self )
{
    BSTreeWhack ( &self->spotgroups, whack_spotgroup, NULL );
}

static rc_t validate_row_data(statistic const *self, 
                              row_input const *row_data)
{
    rc_t rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
    unsigned const n_bases = row_data->read_len;
    
    if (row_data->read == NULL || row_data->quality == NULL) {
        return rc;
    }
    if (n_bases != row_data->quality_len) {
        return rc;
    }
    return 0;
}

rc_t extract_statistic_from_row(statistic *self, 
                                row_input const *data)
{
    rc_t rc = 0;
    spotgrp *sg;
    char const *spotgrp_base;
    uint32_t spotgrp_len;
    unsigned i;
    uint8_t lb = 4;
    unsigned hpr = 0;
    unsigned gcc = 0;
    
    if (data == NULL) {
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcNull);
    }
    if (self == NULL) {
        return RC(rcXF, rcFunction, rcExecuting, rcSelf, rcNull);
    }
    rc = validate_row_data(self, data);
    if (rc)
        return rc;
    
    spotgrp_base = data->spotgroup;
    spotgrp_len = data->spotgroup_len;
    
    if (spotgrp_base == NULL || spotgrp_len == 0) {
        spotgrp_base = "";
        spotgrp_len = 0;
    }

    sg = find_spotgroup( self, spotgrp_base, spotgrp_len );
    if ( sg == NULL )
    {
        sg = make_spotgrp( spotgrp_base, spotgrp_len );
        if ( sg == NULL )
        {
            return RC( rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted );
        }
        else
        {
            rc = BSTreeInsert ( &self->spotgroups, (BSTNode *)sg, spotgroup_sort );
            if (rc)
                return rc;
        }
    }
    for (i = 0; i < data->read_len && rc == 0; ++i) {
        unsigned const base = data->read[i];
        unsigned dimer;

        if (base > 3) {
            dimer = 16;
            hpr = 0;
        }
        else {
            dimer = (lb > 3) ? 16 : ((lb << 2) | base);
            if (lb == base)
                ++hpr;
            else
                hpr = 0;
        }
        if (i > 0)
            rc = spotgroup_enter_values(sg, data->quality[i], dimer, gcc, hpr, data->base_pos_offset + i, CASE_MATCH);

        if (base == 1 || base == 2)
            ++gcc;
        if (i >= self->gc_window) {
            unsigned const out = data->read[i - self->gc_window];
            
            if (out == 1 || out == 2)
                --gcc;
        }
        lb = base;
	}
    return rc;
}


typedef struct iter_ctx
{
    bool ( CC * f ) ( stat_row const * row, void *data );
    void * data;
    const char * name;
    bool run;
    stat_row row;
    uint64_t n;
} iter_ctx;


static bool CC spotgroup_iter( BSTNode *n, void *data )
{
    spotgrp *sg = ( spotgrp * ) n;
    iter_ctx *ctx = ( iter_ctx * )data;
    static char const *dimer_chars[] = {
        "AA", "AC", "AG", "AT",
        "CA", "CC", "CG", "CT",
        "GA", "GC", "GG", "GT",
        "TA", "TC", "TG", "TT",
        "NN"
    };

    ctx->row.spotgroup = (char *)sg->name->addr;
    for ( ctx->row.quality = 0; ctx->row.quality < N_QUAL_VALUES && ctx->run; ++ctx->row.quality )
    {
        uint8_t dimer_nr;
        for ( dimer_nr = 0; dimer_nr < N_DIMER_VALUES && ctx->run; ++dimer_nr )
        {
            ctx->row.dimer = dimer_chars[dimer_nr];
            for( ctx->row.gc_content = 0; ctx->row.gc_content < N_GC_VALUES; ++ctx->row.gc_content )
            {
                for ( ctx->row.hp_run = 0; ctx->row.hp_run < N_HP_VALUES && ctx->run; ++ctx->row.hp_run )
                {
                    uint32_t pos;
                    counter_vector * cv = &sg->cnv[ dimer_nr ][ ctx->row.gc_content ][ ctx->row.hp_run ][ ctx->row.quality ];
                    for ( pos = 0; pos < cv->n_counters; ++pos )
                    {
                        counter * c = &cv->v[ pos ];
                        if ( c->count > 0 )
                        {
                            ctx->row.base_pos = pos;
                            ctx->row.count = c->count;

                            ctx->run = ctx->f( &ctx->row, ctx->data );
                            ctx->n++;
                         }
                    }
                }
            }
        }
    }
    return( !ctx->run );
}


uint64_t foreach_statistic( statistic * self,
    bool ( CC * f ) ( stat_row const * row, void * f_data ), void *f_data )
{
    iter_ctx ctx;
    ctx.f = f;
    ctx.data = f_data;
    ctx.run = true;
    ctx.n = 0;
    BSTreeDoUntil ( &self->spotgroups, false, spotgroup_iter, &ctx );
    return ctx.n;
}
