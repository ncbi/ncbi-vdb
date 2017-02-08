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
#include <vdb/extern.h>
#include <insdc/insdc.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/database.h>
#include <vdb/vdb-priv.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/pack.h>
#include <klib/sort.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct node {
    BSTNode link;
    char const *name;
    uint8_t *bitmap;
    int64_t first_row;
    uint64_t row_count;
    size_t namelen;
};
typedef struct node Node;

static
int64_t CC NodeNameCmp( void const *a, BSTNode const *n )
{
    String b;
    
    StringInit( &b, ( ( Node const * )n )->name, ( ( Node const * )n )->namelen,
                    ( uint32_t )( ( Node const * )n )->namelen );
    return StringOrderNoNullCheck( a, &b );
}

static
int64_t CC NodeCmp( BSTNode const *A, BSTNode const *B )
{
    String a;
    String b;
    
    StringInit( &a, ( ( Node const * )A )->name, ( ( Node const * )A )->namelen, 
                    ( uint32_t )( ( Node const * )A )->namelen );
    StringInit( &b, ( ( Node const * )B )->name, ( ( Node const * )B )->namelen,
                    ( uint32_t )( ( Node const * )B )->namelen );
    return StringOrderNoNullCheck( &a, &b );
}

static
void CC NodeWhack( BSTNode *n, void *data )
{
    free( n );
}


static
rc_t NodeProcessIDs( Node *self, unsigned row, VCursor const *curs, uint32_t const cid[],
                     bool buf[], size_t sz, size_t *bmax, INSDC_coord_len const max_seq_len,
                     bool isPrimary, int64_t const ID[], unsigned N )
{
    rc_t rc = 0;
    unsigned i;
    bitsz_t dummy;
    size_t max = *bmax;
    
    for ( i = 0; i != N; ++i )
    {
        uint64_t const *globalRefStart;
        bool const *preserveQual;
        uint32_t elem_bits;
        uint32_t boff;
        uint32_t n;
        unsigned offset;

        rc = VCursorCellDataDirect( curs, ID[ i ], cid[ 0 ], &elem_bits,
                                    ( void const ** )&globalRefStart, &boff, &n );
        if ( rc != 0 )
            return rc;

        rc = VCursorCellDataDirect( curs, ID[ i ], cid[ 1 ], &elem_bits,
                                    ( void const ** )&preserveQual, &boff, &n );
        if ( rc != 0 )
            return rc;
        
        offset = ( globalRefStart[ 0 ] ) % max_seq_len;
        if ( offset + n > max )
        {
            max = offset + n;
        }

        if ( max >= sz ) 
        {
            return RC( rcXF, rcNode, rcConstructing, rcBuffer, rcInsufficient );
        }

        {
            unsigned j;
            
            for ( j = 0; j != n; ++j )
            {
                buf[ j + offset ] |= preserveQual[ j ];
            }
        }
    }

    Pack( 8, 1, buf, max_seq_len, NULL, self->bitmap, ( size_t )row * max_seq_len,
         ( size_t )self->row_count * max_seq_len, &dummy );

    if ( max > max_seq_len )
    {
        max -= max_seq_len;
        memmove( buf, buf + max_seq_len, max );
        memset( buf + max, 0, max_seq_len );
    }
    else
    {
        memset( buf, 0, max );
        max = 0;
    }
    *bmax = max;

    return rc;
}


static
rc_t OpenAlignCursor( VCursor const **curs, uint32_t cid[], VDatabase const *db, bool isPrimary )
{
    char const * tname = isPrimary ? "PRIMARY_ALIGNMENT" : "SECONDARY_ALIGNMENT";
    VTable const * atbl;
    rc_t rc = VDatabaseOpenTableRead( db, &atbl, tname );
    
    if ( rc == 0 )
    {
        VCursor const *acurs;
        rc = VTableCreateCursorRead( atbl, &acurs );
        VTableRelease( atbl );
        
        while ( rc == 0 )
        {
            rc = VCursorAddColumn( acurs, &cid[ 0 ], "(U64)GLOBAL_REF_START" );
            assert( rc == 0 );
            if ( rc != 0 )
                break;
            
            rc = VCursorAddColumn( acurs, &cid[ 1 ], "(bool)REF_PRESERVE_QUAL" );
            assert( rc == 0 );
            if ( rc != 0 )
                break;
            
            rc = VCursorOpen( acurs );
            if ( rc != 0 )
                break;

            *curs = acurs;
            return 0;
        }
        VCursorRelease( acurs );
    }
    return rc;
}


static
rc_t NodeLoadIDs( Node *self, VTable const *tbl, VDatabase const *db, bool buf[],
                  size_t sz, INSDC_coord_len const max_seq_len, bool isPrimary )
{
    char const *cname = isPrimary ? "PRIMARY_ALIGNMENT_IDS" : "SECONDARY_ALIGNMENT_IDS";
    VCursor const *algn;
    uint32_t algn_cid[ 2 ];
    rc_t rc = OpenAlignCursor( &algn, algn_cid, db, isPrimary );

    if ( rc == 0 )
    {
        VCursor const *curs;
        rc = VTableCreateCursorRead( tbl, &curs );
        
        if ( rc == 0 )
        {
            uint32_t cid;
            
            rc = VCursorAddColumn( curs, &cid, cname );
            if ( rc == 0 )
            {
                rc = VCursorOpen( curs );
                if ( rc == 0 )
                {
                    unsigned i;
                    size_t bmax = 0;
                    
                    memset( buf, 0, sz );
                    for ( i = 0; i != self->row_count; ++i )
                    {
                        void const *base;
                        uint32_t elem_bits;
                        uint32_t boff;
                        uint32_t n;
                        
                        rc = VCursorCellDataDirect( curs, self->first_row + i, cid,
                                                    &elem_bits, &base, &boff, &n );
                        if ( rc != 0 )
                            break;
                        if ( n > 0 )
                        {
                            rc = NodeProcessIDs( self, i, algn, algn_cid,
                                                 buf, sz, &bmax, max_seq_len,
                                                 isPrimary, base, n );
                            if ( rc != 0 )
                                break;
                        }
                    }
                }
            }
            VCursorRelease( curs );
        }
        VCursorRelease( algn );
    }
    else if ( !isPrimary )
        rc = 0;
    
    return rc;
}


static
rc_t NodeLoad( Node *self, VTable const *tbl, VDatabase const *db, bool buf[],
               size_t sz, INSDC_coord_len const max_seq_len )
{
    rc_t rc = NodeLoadIDs( self, tbl, db, buf, sz, max_seq_len, true );
    if ( rc == 0 )
    {
        rc = NodeLoadIDs( self, tbl, db, buf, sz, max_seq_len, false );
    }
    return rc;
}


static
rc_t NodeMake( Node **rslt, VTable const *tbl, String const *refName, INSDC_coord_len const max_seq_len )
{
    Node *self;
    int64_t data[ 2 ];
    VDatabase const *db;
    VCursor const *curs;
    rc_t rc = VTableCreateCursorRead( tbl, &curs );
    
    if ( rc == 0 )
    {
        uint32_t cid;
        
        rc = VCursorAddColumn( curs, &cid, "NAME_RANGE" );
        if ( rc == 0 )
        {
            rc = VCursorParamsSet( ( struct VCursorParams const * )curs, "QUERY_SEQ_NAME", "%.*s",
                                    refName->len, refName->addr );
            if ( rc == 0 )
            {
                rc = VCursorOpen( curs );
                if ( rc == 0 )
                {
                    rc = VCursorOpenRow( curs );
                    if ( rc == 0 )
                    {
                        uint32_t dummy;
                        rc = VCursorRead( curs, cid, 64, data, 2, &dummy );
                    }
                }
            }
        }
        VCursorRelease( curs );
    }
    if ( rc != 0 )
        return rc;
    
    self = malloc( ( size_t )( sizeof( *self ) + refName->len + ( ( ( data[ 1 ] - data[ 0 ] + 1 ) * max_seq_len + 7 ) >> 3 )  ) );
    if ( self == NULL )
        return RC( rcXF, rcNode, rcConstructing, rcMemory, rcExhausted );
    
    memmove( &self[ 1 ], refName->addr, self->namelen = refName->len );
    self->name = ( char const * )&self[ 1 ];
    self->bitmap = ( uint8_t * )&self->name[ refName->len ];
    self->row_count = data[ 1 ] - ( self->first_row = data[ 0 ] ) + 1;
    
    rc = VTableOpenParentRead( tbl, &db );
    if ( rc == 0 )
    {
        void *scratch = malloc( 32 * max_seq_len );
        if ( scratch != NULL )
        {
            rc = NodeLoad( self, tbl, db, scratch, 32 * max_seq_len, max_seq_len );
            if ( rc == 0 )
            {
                VDatabaseRelease( db );
                free( scratch );
                *rslt = self;
                return 0;
            }
            free( scratch );
        }
        else
        {
            rc = RC( rcXF, rcNode, rcConstructing, rcMemory, rcExhausted );
        }
        VDatabaseRelease( db );
    }
    free( self );
    *rslt = NULL;
    
    return rc;
}


/* generate_preserve_qual
 *  compute the positions where corresponding qualities should be preserved
 *  when converting to an analysis database.
 * Preserve where:
 *  there is a mismatch
 *  two positions to each side of an insert
 *
 * extern function bool NCBI:align:generate_preserve_qual #1
 *  ( utf8 ref_name, INSDC:coord:len seq_len, U32 max_seq_len );
 */
static
rc_t CC generate_preserve_qual_impl( void *Self, VXformInfo const *info,
                                     int64_t row_id, VRowResult *rslt,
                                     uint32_t argc, VRowData const argv[] )
{
    BSTree *tree = Self;
    Node *map;
    rc_t rc;
    String refName;
    uint32_t const max_seq_len = ( ( uint32_t const * )argv[ 2 ].u.data.base )[ argv[ 2 ].u.data.first_elem ];
    INSDC_coord_len const seq_len = ( ( INSDC_coord_len const * )argv[ 1 ].u.data.base )[ argv[ 1 ].u.data.first_elem ];

    assert( argv[ 1 ].u.data.elem_bits == sizeof( seq_len ) * 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( max_seq_len ) * 8 );
    
    rslt->data->elem_bits = rslt->elem_bits;
    rslt->elem_count = seq_len;
    rc = KDataBufferResize( rslt->data, rslt->elem_count );
    if ( rc != 0 )
        return rc;
    memset( rslt->data->base, 0, ( size_t )( ( seq_len * rslt->elem_bits + 7 ) >> 3 ) );
    return 0;

    /* ??? this code cannot be reached !!! */
    StringInit( &refName,
                &( ( char const * )argv[ 0 ].u.data.base )[ argv[ 0 ].u.data.first_elem ],
                ( size_t )argv[ 0 ].u.data.elem_count,
                ( uint32_t )argv[ 0 ].u.data.elem_count );
    map = ( Node * )BSTreeFind( tree, &refName, NodeNameCmp );
    if ( !map )
    {
        rc = NodeMake( &map, info->tbl, &refName, max_seq_len );
        if ( rc != 0 )
            return rc;
        rc = BSTreeInsert( tree, &map->link, NodeCmp );
        if ( rc != 0 )
            return rc;
    }

    {{
        size_t const start = ( size_t )( ( row_id - map->first_row ) * max_seq_len );
        size_t dummy;
        
        Unpack( 1, 8, map->bitmap, start, seq_len, NULL, rslt->data->base, seq_len, &dummy );
    }}
    
    return 0;
}

static
void CC Whack_generate_preserve_qual( void *vp )
{
    BSTreeWhack( vp, NodeWhack, NULL );
    free( vp );
}


VTRANSFACT_IMPL ( NCBI_align_generate_preserve_qual, 1, 0, 0 )
    ( const void *Self, VXfactInfo const *info,
      VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = malloc( sizeof( BSTree ) );
    if ( rslt->self )
    {
        BSTreeInit( ( BSTree * )rslt->self );
        rslt->u.rf = generate_preserve_qual_impl;
        rslt->variant = vftIdDepRow;
        rslt -> whack = Whack_generate_preserve_qual;
        return 0;
    }
    return RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
}
