/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was readten as part of
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
#include <align/extern.h>

#include <klib/rc.h>
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/vector.h>
#include <klib/out.h>
#include <klib/text.h>
#include <insdc/insdc.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/cursor.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <align/iterator.h>
#include <align/reference.h>
#include <align/refseq-mgr.h>
#include <os-native.h>
#include <sysalloc.h>

#include "reader-cmn.h"
#include "reference-cmn.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

/* This is a temporary define, used to test code to deal with
   circular references and new means of tracking window end */

enum EReferenceList_ColNames
{
    ereflst_cn_READ_dna,
    ereflst_cn_READ_4na,
    ereflst_cn_SEQ_LEN,
    ereflst_cn_PRIMARY_ALIGNMENT_IDS,
    ereflst_cn_SECONDARY_ALIGNMENT_IDS,
    ereflst_cn_EVIDENCE_INTERVAL_IDS,
    ereflst_cn_OVERLAP_REF_POS,
    ereflst_cn_OVERLAP_REF_LEN
};

const TableReaderColumn ReferenceList_cols[] =
{
    { 0, "(INSDC:dna:text)READ",    { NULL }, 0, 0 },
    { 0, "(INSDC:4na:bin)READ",     { NULL }, 0, ercol_Skip },
    { 0, "SEQ_LEN",                 { NULL }, 0, 0 },
    { 0, "PRIMARY_ALIGNMENT_IDS",   { NULL }, 0, ercol_Skip },
    { 0, "SECONDARY_ALIGNMENT_IDS", { NULL }, 0, ercol_Skip | ercol_Optional },
    { 0, "EVIDENCE_INTERVAL_IDS",   { NULL }, 0, ercol_Skip | ercol_Optional },
    { 0, "OVERLAP_REF_POS",         { NULL }, 0, ercol_Optional },
    { 0, "OVERLAP_REF_LEN",         { NULL }, 0, ercol_Optional },
    { 0, NULL,                      { NULL }, 0, 0 }
};

enum EPlacementIterator_ColNames
{
    eplacementiter_cn_REF_POS,
    eplacementiter_cn_REF_LEN,
    eplacementiter_cn_MAPQ,
    eplacementiter_cn_READ_GROUP
};

const TableReaderColumn PlacementIterator_cols[] =
{
    { 0, "REF_POS",     { NULL }, 0, 0 },
    { 0, "REF_LEN",     { NULL }, 0, 0 },
    { 0, "MAPQ",        { NULL }, 0, 0 },
    { 0, "SPOT_GROUP",  { NULL }, 0, 0 },
    { 0, NULL,          { NULL }, 0, 0 }
};

struct ReferenceList
{
    KRefcount refcount;
    const RefSeqMgr* refseqmgr;
    const VCursor* cursor;
    BSTree name_tree;
    BSTree seqid_tree;
    uint32_t options;
    size_t cache;
    uint32_t max_seq_len;
    uint32_t nodes_qty;
    uint32_t nodes_max_qty;
    const TableReader* reader;
    TableReaderColumn reader_cols[ sizeof( ReferenceList_cols ) / sizeof( ReferenceList_cols[ 0 ] ) ];
    const TableReader* iter;
    TableReaderColumn iter_cols[ sizeof( PlacementIterator_cols ) / sizeof( PlacementIterator_cols[ 0 ] ) ];
    /* last are children using realloc!! */
    ReferenceObj* nodes[ 2 ];
};

struct ReferenceObj
{
    /* we use this in 2 lists so we need to adjust results of Find, etc calls for name tree!! */
    BSTNode by_seqid; /* primary key */
    BSTNode by_name; /* struct addr will be by_name[-1]; */
    ReferenceList* mgr;
    uint32_t id;
    uint32_t bin;
    char* name;
    char* seqid;
    bool circular;
    bool read_present;
    int64_t start_rowid;
    int64_t end_rowid;
    INSDC_coord_len seq_len;
};


static int64_t CC ReferenceObj_CmpSeqId( const void *item, const BSTNode *n )
{
    return strcasecmp( ( const char* )item, ( ( const ReferenceObj* )n )->seqid );
}


static int64_t CC ReferenceObj_SortSeqId( const BSTNode *item, const BSTNode *n )
{
    return ReferenceObj_CmpSeqId( ( ( const ReferenceObj* )item )->seqid, n );
}


static int64_t CC ReferenceObj_CmpName( const void *item, const BSTNode *n )
{
    return strcasecmp( ( const char* )item, ( ( const ReferenceObj* )&n[ -1 ] )->name );
}


static int64_t CC ReferenceObj_SortName( const BSTNode *item, const BSTNode *n )
{
    return ReferenceObj_CmpName( ( ( const ReferenceObj* )&item[ -1 ] )->name, n );
}

static rc_t ReferenceObj_Alloc( ReferenceObj** self, const char* seqid, size_t seqid_sz,
                                const char* name, size_t name_sz)
{
    rc_t rc = 0;
    if ( self == NULL || seqid == NULL || seqid_sz == 0 || name == NULL || name_sz == 0 )
    {
        rc = RC( rcAlign, rcIndex, rcConstructing, rcParam, rcNull );
    }
    else
    {
        ReferenceObj* obj = calloc( 1,  sizeof( *obj ) + seqid_sz + 1 + name_sz + 1 );
        if ( obj == NULL )
        {
            rc = RC( rcAlign, rcIndex, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            obj->seqid = ( char* )&obj[ 1 ];
            obj->name = obj->seqid;
            obj->name += seqid_sz + 1;
            memmove( obj->seqid, seqid, seqid_sz );
            obj->seqid[ seqid_sz ] = '\0';
            memmove( obj->name, name, name_sz );
            obj->name[ name_sz ] = '\0';
            *self = obj;
        }
    }
    return rc;
}


/* helper function for ReferenceList_MakeCursor() */
static rc_t ReferenceList_handle_filter( const KIndex* iname, const char * filt_name, uint32_t bin_size,
                                         int64_t *start, int64_t tbl_start, uint64_t *count, int *bin_num,
                                         TableReaderColumn *h, const TableReader* tmp )
{
    rc_t rc = 0;
    if ( strncmp( filt_name, "START_ROW:", 10 ) )
    {
        if ( bin_size == 0 || strncmp( filt_name, "BIN_NUM:", 8 ) )
        {
            rc = KIndexFindText( iname, filt_name, start, count, NULL, NULL );
            if( rc == 0 && bin_size > 0 )
            {
                /** change start to the beginning of the bin **/
                *bin_num = ( *start - tbl_start ) / bin_size;
            }
        }
        else
        {
            *bin_num = atoi( filt_name + 8 );
        }
        if ( bin_size > 0 )
        {
            *start = tbl_start + ( bin_size * (*bin_num) );
            rc = TableReader_ReadRow( tmp, *start );
            if ( rc == 0 )
            {
                int64_t r_start;
                char name[ 4096 ];
                if ( h[ 0 ].len < sizeof( name ) )
                {
                    memmove( name, h[ 0 ].base.str, h[ 0 ].len );
                    name[ h[ 0 ].len ] = '\0';
                    rc = KIndexFindText( iname, name, &r_start, count, NULL, NULL );
                    if ( rc == 0 && *start > r_start )
                    { /*** move start to the beginning of the fully contained sequence **/
                        *start = r_start + *count;
                    }
                }
                else
                {
                    rc = RC( rcAlign, rcType, rcConstructing, rcName, rcTooLong );
                }
            }
        }
    }
    else
    {
        int64_t req_start = atoi( filt_name + 10 );
        if ( req_start >= *start && req_start < *start + *count )
        {
            int64_t delta = req_start - *start;
            *start = req_start;
            *count -= delta;
        }
        else
        {
            rc = RC( rcAlign, rcType, rcConstructing, rcId, rcOutofrange );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_MakeCursor( const ReferenceList** cself, const VCursor* cursor, uint32_t options,
                                             const char* filt_name, const uint32_t numbins )
{
    rc_t rc = 0;
    ReferenceList* self = NULL;

    if ( cself == NULL || cursor == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcNull );
    }
    else if ( ( self = calloc( 1, sizeof( *self ) ) ) == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        const TableReader* tmp = NULL;
        uint32_t bin_size;
        int bin_num = -1;
        TableReaderColumn h[] =
        {
            { 0, "NAME",        {NULL}, 0, 0 }, /*0*/
            { 0, "SEQ_ID",      {NULL}, 0, 0 }, /*1*/
            { 0, "SEQ_LEN",     {NULL}, 0, 0 }, /*2*/
            { 0, "CIRCULAR",    {NULL}, 0, 0 }, /*3*/
            { 0, "MAX_SEQ_LEN", {NULL}, 0, 0 }, /*4*/
            { 0, "SEQ_START",   {NULL}, 0, 0 }, /*5*/
            { 0, "CMP_READ",    {NULL}, 0, 0 }, /*6*/
            { 0, NULL,          {NULL}, 0, 0 }
        };
        KRefcountInit( &self->refcount, 1, "ReferenceList", "Make", "align" );
        BSTreeInit( &self->name_tree );
        BSTreeInit( &self->seqid_tree );
        self->options = options;
        self->nodes_max_qty = sizeof( self->nodes ) / sizeof( self->nodes[ 0 ] );

        rc = VCursorAddRef( self->cursor = cursor );
        if ( rc == 0 )
        {
            rc = TableReader_MakeCursor( &tmp, cursor, h );
            if ( rc == 0 )
            {
                int64_t start, tbl_start, tbl_stop;
                uint64_t count;
                const KIndex* iname = NULL;
                bool only_one = false;

                /* index is optional */
                rc_t rctmp = TableReader_OpenIndex( tmp, "i_name", &iname );
                ALIGN_DBGERRP( "index '%s' was not found", rctmp, "i_name" );

                rc = TableReader_IdRange( tmp, &start, &count );
                assert( rc == 0 );
                tbl_start = start;
                tbl_stop  = start + count -1;
                if ( numbins > 0 )
                {
                    bin_size = ( count + numbins -1 ) / numbins;
                }
                else
                {
                    bin_size = 0;
                }

                if ( iname && filt_name )
                {
                    if ( bin_size == 0 )
                    {
                        only_one = true;
                    }
                    rc = ReferenceList_handle_filter( iname, filt_name, bin_size, &start, tbl_start,
                                                      &count, &bin_num, h, tmp );
                }

                if ( rc == 0 )
                {
                    ReferenceObj* node = NULL;
                    uint32_t last_name_len = 0;
                    bool read_determination_done = false;

                    while ( rc == 0  && start <= tbl_stop )
                    {
                        if ( bin_num < 0 && count == 0 )
                        {
                            /*** normal loop without binning ***/
                            break;
                        }
                        rc = TableReader_ReadRow( tmp, start );
                        if ( rc == 0 )
                        {
                            if ( node == NULL || last_name_len != h[0].len ||
                                 strncmp( h[ 0 ].base.str, node->name, h[ 0 ].len) != 0 )
                            {
                                uint32_t cur_bin = ( bin_size > 0 ) ? ( start-tbl_start ) / bin_size : 0;
                                if ( only_one && self->nodes_qty == 1 )
                                {
                                    break;
                                }
                                if ( bin_num >= 0 && cur_bin != bin_num )
                                {
                                    break;
                                }
                                if ( node == NULL && h[ 4 ].len > 0 )
                                {
                                    self->max_seq_len = h[ 4 ].base.u32[ 0 ];
                                }
                                if ( self->nodes_qty == self->nodes_max_qty )
                                {
                                    ReferenceList* tmp = realloc( self, sizeof( *self ) + sizeof( node ) * self->nodes_max_qty );
                                    if ( tmp == NULL )
                                    {
                                        rc = RC(rcAlign, rcType, rcConstructing, rcMemory, rcExhausted);
                                    }
                                    else
                                    {
                                        self = tmp;
                                        self->nodes_max_qty += sizeof( self->nodes ) / sizeof( self->nodes[ 0 ] );
                                    }
                                }
                                if ( rc == 0 )
                                {
                                    rc = ReferenceObj_Alloc( &node, h[ 1 ].base.str, h[ 1 ].len, h[ 0 ].base.str, h[ 0 ].len );
                                    if ( rc == 0 )
                                    {
                                        node->id = self->nodes_qty;
                                        self->nodes[ self->nodes_qty++ ] = node;
                                        last_name_len = h[ 0 ].len;
                                        node->circular = h[ 3 ].len ? h[ 3 ].base.buul[ 0 ] : false;
                                        node->start_rowid = start;
                                        node->seq_len = 0;
                                        node->bin = cur_bin;
                                        read_determination_done = false;
                                        rc = BSTreeInsertUnique( &self->seqid_tree, &node->by_seqid, NULL, ReferenceObj_SortSeqId );
                                        if ( rc == 0 )
                                        {
                                            rc = BSTreeInsertUnique( &self->name_tree, &node->by_name, NULL, ReferenceObj_SortName );
                                        }
                                    }
                                }
                            }
                            if ( rc == 0 )
                            {
                                INSDC_coord_len cur_seq_len = h[ 2 ].base.coord_len[ 0 ];
                                if ( cur_seq_len == 0 )
                                {
                                    /* assign it to max-seq-len */
                                    cur_seq_len = h[ 4 ].base.coord_len[ 0 ];
                                }

                                if ( h[ 6 ].len > 0 )
                                {/** CMP_READ > 0 -- truly local ***/
                                    node->read_present = true; 
                                    read_determination_done = true;
                                }
                                else if ( h[ 5 ].base.coord1[ 0 ] != 0 )
                                { /*** truly remote ***/
                                    node->read_present = false;
                                    read_determination_done = true;
                                } /*** else still not sure **/

                                if ( read_determination_done && iname != NULL )
                                {
                                    /* scroll to last row for this reference projecting the seq_len */
                                    int64_t r_start;
                                    uint64_t r_count;
                                    if ( KIndexFindText( iname, node->name, &r_start, &r_count, NULL, NULL ) == 0 )
                                    {
                                        assert( node->start_rowid == r_start );
                                        /* not last ref row */
                                        if ( start != r_start + r_count - 1 )
                                        {
                                            /* we need to pickup last row SEQ_LEN for this reference
                                            so we step back 2 rows in table from this ref end row
                                            and also skip rows already scanned for read presence */
                                            r_count -= ( start - r_start ) + 2;
                                            node->seq_len += cur_seq_len * r_count;
                                            start += r_count;
                                            count -= r_count;
                                        }
                                    }
                                }
                                node->seq_len += cur_seq_len;
                                node->end_rowid = start;
                            }
                        }
                        else if ( GetRCState( rc ) == rcNotFound && GetRCObject( rc ) == rcRow )
                        {
                            rc = 0;
                        }
                        start++;
                        count--;
                    }
                    for ( start = 0; rc == 0 && start < self->nodes_qty; start++ )
                    {
                        self->nodes[ start ]->mgr = self;
                    }
                    if ( rc == 0 && self->max_seq_len == 0 )
                    {
                        rc = RC(rcAlign, rcType, rcConstructing, rcData, rcCorrupt);
                    }
                }
                KIndexRelease( iname );
            }
        }
        TableReader_Whack( tmp );
    }

    if ( rc == 0 )
    {
        *cself = self;
        /* ALIGN_DBG("created 0x%p with cursor 0x%p", self, cursor); */
    }
    else
    {
        *cself = NULL;
        ReferenceList_Release( self );
        /* ALIGN_DBGERRP( "failed for cursor 0x%p", rc, cursor ); */
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_MakeTable( const ReferenceList** cself, const VTable* table, uint32_t options,
                                            size_t cache, const char* filt_name, const uint32_t numbins )
{
    rc_t rc = 0;
    const VCursor* curs;

    if ( table == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcNull );
    }
    else
    {
        rc = VTableCreateCachedCursorRead( table, &curs, cache );
        if ( rc == 0 )
        {
            rc = VCursorPermitPostOpenAdd( curs );
            if ( rc == 0 )
            {
                rc = ReferenceList_MakeCursor( cself, curs, options, filt_name, numbins );
                if ( rc == 0 )
                {
                    ( (ReferenceList*)(*cself))->cache = cache;
                }
                VCursorRelease(curs);
            }
        }
    }
    ALIGN_DBGERRP( "failed for table 0x%p", rc, table );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_MakeDatabase( const ReferenceList** cself, const VDatabase* db, uint32_t options,
                                               size_t cache, const char* name, const uint32_t numbins )
{
    rc_t rc = 0;
    const VTable* tbl = NULL;
    const char* nm = "REFERENCE";
    /*const char* nm = (options & ereferencelist_useEvidence) ? "EVIDENCE_INTERVAL" : "REFERENCE";*/

    if ( db == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcNull );
    }
    else
    {
        rc = VDatabaseOpenTableRead( db, &tbl, nm );
        if ( rc == 0 )
        {
            rc = ReferenceList_MakeTable( cself, tbl, options, cache, name, numbins );
            VTableRelease( tbl );
        }
    }
    ALIGN_DBGERRP( "failed for database 0x%p", rc, db );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_MakePath( const ReferenceList** cself, const VDBManager* vmgr, const char* dbpath,
                                           uint32_t options, size_t cache, const char* name, const uint32_t numbins )
{
    rc_t rc = 0;
    const VDatabase* db = NULL;

    if ( vmgr == NULL || dbpath == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcNull );
    }
    else
    {
        rc = VDBManagerOpenDBRead( vmgr, &db, NULL, "%s", dbpath );
        if ( rc == 0 )
        {
            rc = ReferenceList_MakeDatabase( cself, db, options, cache, name, numbins );
            VDatabaseRelease( db );
        }
    }
    ALIGN_DBGERRP( "failed for database %s", rc, dbpath );
    return rc;
}


static rc_t ReferenceList_OpenCursor( ReferenceList* self )
{
    rc_t rc = 0;

    assert( self != NULL );

    memmove( self->reader_cols, ReferenceList_cols, sizeof( ReferenceList_cols ) );

    if ( self->options & ereferencelist_4na )
    {
        self->reader_cols[ ereflst_cn_READ_dna ].flags |= ercol_Skip;
        self->reader_cols[ ereflst_cn_READ_4na ].flags &= ~ercol_Skip;
    }

    if ( self->options & ereferencelist_usePrimaryIds )
    {
        self->reader_cols[ ereflst_cn_PRIMARY_ALIGNMENT_IDS ].flags &= ~ercol_Skip;
    }

    if ( self->options & ereferencelist_useSecondaryIds )
    {
        self->reader_cols[ ereflst_cn_SECONDARY_ALIGNMENT_IDS ].flags &= ~ercol_Skip;
    }

    if ( self->options & ereferencelist_useEvidenceIds )
    {
        self->reader_cols[ ereflst_cn_EVIDENCE_INTERVAL_IDS ].flags &= ~ercol_Skip;
    }

    if ( !( self->options &
          ( ereferencelist_usePrimaryIds | ereferencelist_useSecondaryIds | ereferencelist_useEvidenceIds ) ) )
    {
        self->reader_cols[ ereflst_cn_OVERLAP_REF_POS ].flags |= ercol_Skip;
        self->reader_cols[ ereflst_cn_OVERLAP_REF_LEN ].flags |= ercol_Skip;
    }
    rc = TableReader_MakeCursor( &self->reader, self->cursor, self->reader_cols );
    ALIGN_DBGERR( rc );
    return rc;
}


static rc_t ReferenceList_OpenCursor2( ReferenceList* self, align_id_src ids )
{
    rc_t rc = 0;

    assert( self != NULL );
    if ( ids != primary_align_ids && ids != secondary_align_ids && ids != evidence_align_ids )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcOutofrange );
    }
    else if ( ids == primary_align_ids && !( self->options & ereferencelist_usePrimaryIds ) )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else if ( ids == secondary_align_ids && !( self->options & ereferencelist_useSecondaryIds ) )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else if ( ids == evidence_align_ids && !( self->options & ereferencelist_useEvidenceIds ) )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        const VTable* vtbl = NULL;
        rc = VCursorOpenParentRead( self->cursor, &vtbl );
        if ( rc == 0 )
        {
            if ( rc == 0 )
            {
                const VDatabase* db = NULL;
                rc = VTableOpenParentRead( vtbl, &db );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenTableRead( db, &vtbl, ids == primary_align_ids ? "PRIMARY_ALIGNMENT" :
                           ( ids == secondary_align_ids ? "SECONDARY_ALIGNMENT" : "EVIDENCE_INTERVAL" ) );
                    if ( rc == 0 )
                    {
                        memmove( self->iter_cols, PlacementIterator_cols, sizeof( PlacementIterator_cols ) );
                        rc = TableReader_Make( &self->iter, vtbl, self->iter_cols, self->cache );
                    }
                    VDatabaseRelease( db );
                }
            }
            VTableRelease( vtbl );
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


static rc_t ReferenceList_RefSeqMgr( const ReferenceList* cself, const RefSeqMgr** rmgr )
{
    rc_t rc = 0;

    assert( rmgr != NULL );

    if ( cself->refseqmgr == NULL )
    {
        const VTable* vtbl = NULL;
        rc = VCursorOpenParentRead( cself->cursor, &vtbl );
        if ( rc == 0 )
        {
            const VDBManager* vmgr;
            rc = VTableOpenManagerRead( vtbl, &vmgr );
            if ( rc == 0 )
            {
                rc = RefSeqMgr_Make( &( (ReferenceList*)cself )->refseqmgr, vmgr,
                                     ( cself->options & ereferencelist_4na ) ? errefseq_4NA : 0, cself->cache, 2 );
                VDBManagerRelease( vmgr );
            }
            VTableRelease( vtbl );
        }
    }
    *rmgr = cself->refseqmgr;
    ALIGN_DBGERR(rc);
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_AddRef( const ReferenceList *cself )
{
    rc_t rc = 0;
    if ( cself != NULL )
    {
        if ( KRefcountAdd( &cself->refcount, "ReferenceList" ) != krefOkay )
        {
            rc = RC( rcAlign, rcType, rcAttaching, rcError, rcUnexpected );
        }
    }
    return rc;
}


LIB_EXPORT void CC ReferenceList_Release( const ReferenceList* cself )
{
    if ( cself != NULL )
    {
        if ( KRefcountDrop(&cself->refcount, "ReferenceList") == krefWhack )
        {
            ReferenceList* self = ( ReferenceList* )cself;
            TableReader_Whack( self->reader );
            TableReader_Whack( cself->iter );
            RefSeqMgr_Release( self->refseqmgr );
            while( self->nodes_qty-- > 0 )
            {
                free( self->nodes[ self->nodes_qty ] );
            }
            VCursorRelease( cself->cursor );
            KRefcountWhack( &self->refcount, "ReferenceList" );
            free( self );
        }
    }
}


LIB_EXPORT rc_t CC ReferenceList_Count(const ReferenceList* cself, uint32_t* count)
{
    rc_t rc = 0;
    if ( cself == NULL || count == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcNull );
    }
    else
    {
        *count = cself->nodes_qty;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_Find( const ReferenceList* cself, const ReferenceObj** obj,
                                       const char* key, size_t key_sz )
{
    rc_t rc = 0;
    char buf[4096], *b = buf;

    if ( cself == NULL || obj == NULL || key == NULL )
    {
        rc = RC( rcAlign, rcType, rcSearching, rcParam, rcNull );
    }
    else if ( key_sz >= sizeof( buf ) && ( b = malloc( key_sz + 1 ) ) == NULL )
    {
        rc = RC(rcAlign, rcType, rcSearching, rcMemory, rcExhausted);
    }
    else
    {
        memmove( b, key, key_sz );
        b[ key_sz ] = '\0';
        *obj = ( ReferenceObj* )BSTreeFind( &cself->seqid_tree, b, ReferenceObj_CmpSeqId );
        if ( *obj == NULL )
        {
            const BSTNode* n = BSTreeFind( &cself->name_tree, b, ReferenceObj_CmpName );
            if ( n != NULL )
            {
                *obj = ( ReferenceObj* )&n[ -1 ];
            }
        }
        if ( *obj == NULL )
        {
            rc = RC( rcAlign, rcType, rcSearching, rcItem, rcNotFound );
        }
        else
        {
            rc = ReferenceList_AddRef( cself );
            if ( rc != 0 )
            {
                *obj = NULL;
            }
        }

        if ( b != buf )
        {
            free( b );
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceList_Get( const ReferenceList* cself, const ReferenceObj** obj, uint32_t idx )
{
    rc_t rc = 0;
    if ( cself == NULL || obj == NULL || idx >= cself->nodes_qty )
    {
        rc = RC( rcAlign, rcType, rcRetrieving, rcParam, rcInvalid );
    }
    else
    {
        rc = ReferenceList_AddRef( cself );
        if ( rc == 0 )
        {
            *obj = cself->nodes[ idx ];
        }
        else
        {
            *obj = NULL;
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t ReferenceObj_AddRef( const ReferenceObj *cself )
{
    if ( cself == NULL )
    {
        return RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    } 
    else
    {
        return ReferenceList_AddRef( cself->mgr );
    }
}


LIB_EXPORT void ReferenceObj_Release( const ReferenceObj *cself )
{
    ReferenceList_Release( cself ? cself->mgr : NULL );
}


LIB_EXPORT rc_t CC ReferenceObj_Idx( const ReferenceObj* cself, uint32_t* idx )
{
    rc_t rc = 0;
    if ( cself == NULL || idx == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *idx = cself->id;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_IdRange( const ReferenceObj* cself, int64_t* start, int64_t* stop )
{
    rc_t rc = 0;
    if ( cself == NULL || (start == NULL && stop == NULL) )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        if ( start != NULL )
        {
            *start = cself->start_rowid;
        }
        if ( stop != NULL )
        {
            *stop = cself->end_rowid;
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_Bin( const ReferenceObj* cself, uint32_t* bin )
{
    rc_t rc = 0;
    if ( cself == NULL || bin == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *bin = cself->bin;
    }
    ALIGN_DBGERR( rc );
    return rc;

}


LIB_EXPORT rc_t CC ReferenceObj_SeqId( const ReferenceObj* cself, const char** seqid )
{
    rc_t rc = 0;
    if ( cself == NULL || seqid == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *seqid = cself->seqid;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_Name( const ReferenceObj* cself, const char** name )
{
    rc_t rc = 0;
    if ( cself == NULL || name == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *name = cself->name;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_SeqLength( const ReferenceObj* cself, INSDC_coord_len* len )
{
    rc_t rc = 0;
    if ( cself == NULL || len == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *len = cself->seq_len;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_Circular( const ReferenceObj* cself, bool* circular )
{
    rc_t rc = 0;
    if ( cself == NULL || circular == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *circular = cself->circular;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_External( const ReferenceObj* cself, bool* external, char** path )
{
    rc_t rc = 0;

    if ( cself == NULL || external == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        const RefSeqMgr* rmgr;
        *external = !cself->read_present;
        if ( path != NULL && !cself->read_present )
        {
            rc = ReferenceList_RefSeqMgr( cself->mgr, &rmgr );
            if ( rc == 0 )
            {
                *path = NULL;
                rc = RefSeqMgr_Exists( rmgr, cself->seqid, string_size( cself->seqid ), NULL );
                if ( GetRCObject( rc ) == (enum RCObject)rcTable && GetRCState( rc ) == rcNotFound )
                {
                    rc = 0;
                }
            }
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_Read( const ReferenceObj* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                      uint8_t* buffer, INSDC_coord_len* written )
{
    rc_t rc = 0;

    if( cself == NULL || buffer == NULL || written == NULL )
    {
        rc = RC ( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        rc = ReferenceSeq_ReOffset( cself->circular, cself->seq_len, &offset );
        if ( rc == 0 )
        {
            if ( cself->mgr->reader != NULL || ( rc = ReferenceList_OpenCursor( cself->mgr ) ) == 0 )
            {
                int cid = ( cself->mgr->options & ereferencelist_4na ) ? ereflst_cn_READ_4na : ereflst_cn_READ_dna;
                INSDC_coord_len q = 0;
                *written = 0;
                do
                {
                    int64_t rowid = cself->start_rowid + offset / cself->mgr->max_seq_len;
                    INSDC_coord_zero s = offset % cself->mgr->max_seq_len;
                    rc = TableReader_ReadRow( cself->mgr->reader, rowid );
                    if ( rc == 0 )
                    {
                        q = cself->mgr->reader_cols[ereflst_cn_SEQ_LEN].base.coord_len[0] - s;
                        if ( q > len ) { q = len; }
                        memmove( &buffer[ *written ], &cself->mgr->reader_cols[ cid ].base.str[ s ], q );
                        *written += q;
                        offset += q;
                        len -= q;
                    }
                    /* SEQ_LEN < MAX_SEQ_LEN is last row unless it is CIRCULAR */
                    if ( cself->mgr->reader_cols[ ereflst_cn_SEQ_LEN ].base.coord_len[ 0 ] < cself->mgr->max_seq_len )
                    {
                        if ( !cself->circular ) { break; }
                        offset = 0;
                    }
                } while ( rc == 0 && q > 0 && len > 0 );
            }
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC ReferenceObj_GetIdCount( const ReferenceObj* cself, int64_t row_id, uint32_t *count )
{
    rc_t rc = 0;

    if ( cself == NULL || count == NULL )
    {
        rc = RC ( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else if ( cself->mgr == NULL )
    {
        rc = RC ( rcAlign, rcType, rcAccessing, rcItem, rcInvalid );
    }
    else
    {
        *count = 0;

        if ( cself->mgr->reader == NULL )
            rc = ReferenceList_OpenCursor( cself->mgr );

        if ( rc == 0 )
        {
            rc = TableReader_ReadRow( cself->mgr->reader, row_id );
            if ( rc == 0 )
            {
                TableReaderColumn *col = &( cself->mgr->reader_cols[ ereflst_cn_PRIMARY_ALIGNMENT_IDS ] );
                count[ 0 ] = col->len;
                col = &( cself->mgr->reader_cols[ ereflst_cn_SECONDARY_ALIGNMENT_IDS ] );
                count[ 1 ] = col->len;
                col = &( cself->mgr->reader_cols[ ereflst_cn_EVIDENCE_INTERVAL_IDS ] );
                count[ 2 ] = col->len;
            }
        }
    }
    return rc;
}


typedef struct PlacementRecExtensionInfo PlacementRecExtensionInfo;
struct PlacementRecExtensionInfo
{
    /* data, destructor and size for extension 1 */
    void * data;
    void ( CC * destroy ) ( void *obj, void *data );
    size_t size;
};


LIB_EXPORT void * CC PlacementRecordCast ( const PlacementRecord *self, uint32_t ext )
{
    void * res = NULL;
    if ( self != NULL )
    {
        uint8_t * ptr = ( uint8_t * ) self;
        PlacementRecExtensionInfo * ext_info;

/**********************************************
    +---------------+
    | core          |   PlacementRecord *
    +---------------+
    | read_group    |   char *
    +---------------+
    | ext_info1     |   PlacementRecExtensionInfo *
    | ext_info2     |   PlacementRecExtensionInfo *
    +---------------+
    | ext1          |   ??? *
    +---------------+
    | ext2          |   ??? *
    +---------------+
**********************************************/
        ptr += ( sizeof * self );
        ptr += self->spot_group_len;
        ext_info = ( PlacementRecExtensionInfo * )( ptr );
        ptr += ( 2 * ( sizeof * ext_info ) );
        switch( ext )
        {
            case placementRecordExtension0 : res = ( void * ) ptr;
                                             break;

            case placementRecordExtension1 : res = ( void * ) ( ptr + ext_info->size );
                                             break;
        }
    }
    return res;
}


LIB_EXPORT void * CC PlacementRecord_get_ext_data_ptr ( const PlacementRecord *self, uint32_t ext )
{
    void * res = NULL;
    if ( self != NULL )
    {
        uint8_t * ptr = ( uint8_t * ) self;
        PlacementRecExtensionInfo * ext_info;
        ptr += ( sizeof * self );
        ptr += self->spot_group_len;    /* ptr points now to the 1st ext-info-block */
        ext_info = ( PlacementRecExtensionInfo * )( ptr );
        switch( ext )
        {
        case placementRecordExtension0 : res = ext_info[ 0 ].data; break;
        case placementRecordExtension1 : res = ext_info[ 1 ].data; break;
        }
    }
    return res;
}


LIB_EXPORT void CC PlacementRecordWhack( const PlacementRecord *cself )
{
    if ( cself != NULL ) 
    {
        PlacementRecord * self = ( PlacementRecord * )cself;
        PlacementRecExtensionInfo * ext_info;
        uint8_t * ptr = ( uint8_t * )self;
        ptr += sizeof( *self );
        ptr += self->spot_group_len;
        ext_info = ( PlacementRecExtensionInfo * ) ptr;

        /* destroy from the outer callback-block beginning */
        if ( ext_info[ 1 ].destroy != NULL )
        {
            void *obj = PlacementRecordCast ( self, placementRecordExtension1 );
            ext_info[ 1 ].destroy( obj, ext_info[ 1 ].data );
        }

        if ( ext_info[ 0 ].destroy != NULL )
        {
            void *obj = PlacementRecordCast ( self, placementRecordExtension0 );
            ext_info[ 0 ].destroy( obj, ext_info[ 0 ].data );
        }
        /* now deallocate ( or put back into pool ) */
        free( self );
    }
}

struct PlacementIterator
{
    const ReferenceObj* obj;
    INSDC_coord_zero ref_window_start;
    INSDC_coord_len ref_window_len;

    int64_t last_ref_row_of_window_rel;     /* relative to start of reference, not window */
    int64_t cur_ref_row_rel;                /* current row relative to start of reference */
    int64_t rowcount_of_ref;                /* precomputed: how many rows does this reference has */

    /* own reader in case of ref cursor based construction */
    const TableReader* ref_reader;
    TableReaderColumn* ref_cols;
    TableReaderColumn ref_cols_own[sizeof(ReferenceList_cols)/sizeof(ReferenceList_cols[0])];

    /* own reader in case of align cursor based construction */
    const TableReader* align_reader;
    TableReaderColumn* align_cols;
    TableReaderColumn align_cols_own[sizeof(PlacementIterator_cols)/sizeof(PlacementIterator_cols[0])];

    /* current reference table row */
    int64_t current_reftable_row;

    const TableReaderColumn* ids_col;
    Vector ids;
    /* PlacementRecord c-tor params */
    PlacementRecordExtendFuncs ext_0;
    PlacementRecordExtendFuncs ext_1;

    /* if this field is :
     * NULL ... group by original read-group from the source-file
     * points to empty string ... do not perform grouping at all
     * point to non-empty string ... group by this string
    */
    const char * spot_group;
    size_t spot_group_len;
    int32_t min_mapq;

    const VCursor* align_curs;
    void * placement_ctx;           /* source-specific context */
};


static void enter_spotgroup ( PlacementIterator *iter, const char * spot_group )
{
    if ( spot_group == NULL )
    {
        iter->spot_group = NULL;
        iter->spot_group_len = 0;
    }
    else
    {
        iter->spot_group_len = string_size ( spot_group );
        if ( iter->spot_group_len > 0 )
        {
            iter->spot_group = string_dup_measure ( spot_group, NULL );
        }
        else
        {
            iter->spot_group = calloc( 1, 1 );
        }
    }
}


static int64_t calc_overlaped( PlacementIterator * o, align_id_src ids )
{
    int64_t res = o->ref_window_start;
    bool from_ref_table = false;

/*
    uint32_t ofs = 0;
    switch ( ids )
    {
        case primary_align_ids   : ofs = 0; break;
        case secondary_align_ids : ofs = 1; break;
        case evidence_align_ids  : ofs = 2; break;
    }

    if ( o->ref_cols[ ereflst_cn_OVERLAP_REF_LEN ].idx != 0 && 
         o->ref_cols[ ereflst_cn_OVERLAP_REF_LEN ].len > ofs )
    {
        INSDC_coord_len overlap_ref_len = o->ref_cols[ ereflst_cn_OVERLAP_REF_LEN ].base.coord_len[ ofs ];
        if ( overlap_ref_len < o->obj->mgr->max_seq_len )
        {
            if ( o->ref_cols[ ereflst_cn_OVERLAP_REF_POS ].idx != 0 && 
                 o->ref_cols[ ereflst_cn_OVERLAP_REF_POS ].len > ofs )
            {
                res = o->ref_cols[ereflst_cn_OVERLAP_REF_POS].base.coord0[ ofs ];
                from_ref_table = true;
            }
        }
    }
*/

    if ( !from_ref_table )
    {
        /* default is step back 10 rows/50k bases */
        int64_t ref_pos_lookback = ( 10 * o->obj->mgr->max_seq_len );
        if ( o->obj->circular )
        {
            int64_t const half = ( o->obj->seq_len / 2 );

            if ( ref_pos_lookback > half )
            {
                /* go back no more than one full length */
                ref_pos_lookback = half;
            }
            res = ( o->ref_window_start - ref_pos_lookback ); /* could become negative */
        }
        else
        {
            res = ( o->ref_window_start < ref_pos_lookback ? 0 : ( o->ref_window_start - ref_pos_lookback ) );
        }
    }
    return res;
}


LIB_EXPORT rc_t CC ReferenceObj_MakePlacementIterator ( const ReferenceObj* cself,
    PlacementIterator **iter,
    INSDC_coord_zero ref_window_start, INSDC_coord_len ref_window_len,
    int32_t min_mapq,
    struct VCursor const *ref_cur, struct VCursor const *align_cur, align_id_src ids,
    const PlacementRecordExtendFuncs *ext_0, const PlacementRecordExtendFuncs *ext_1,
    const char * spot_group, void * placement_ctx )
{
    rc_t rc = 0;
    PlacementIterator* o = NULL;

    if ( cself == NULL || iter == NULL || ref_window_len < 1 )
    {
        rc = RC(rcAlign, rcType, rcAccessing, rcParam, rcInvalid);
    }
    else if ( ( rc = ReferenceSeq_ReOffset( cself->circular, cself->seq_len, &ref_window_start ) ) != 0 )
    {
    }
    else if ( ( o = calloc( 1, sizeof( *o ) ) ) == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcMemory, rcExhausted );
    }
    else
    {
        rc = ReferenceList_AddRef( cself->mgr );
        if ( rc == 0 )
        {
            ReferenceList* mgr = cself->mgr;
            o->obj = cself;
            /* o->wrapped_around = false; */
            ReferenceObj_AddRef( o->obj );
            o->min_mapq = min_mapq;
            o->placement_ctx = placement_ctx;

            if ( ext_0 != NULL )
            {
                o->ext_0.data = ext_0->data;
                o->ext_0.destroy = ext_0->destroy;
                o->ext_0.populate = ext_0->populate;
                o->ext_0.alloc_size = ext_0->alloc_size;
                o->ext_0.fixed_size = ext_0->fixed_size;
            }

            if ( ext_1 != NULL )
            {
                o->ext_1.data = ext_1->data;
                o->ext_1.destroy = ext_1->destroy;
                o->ext_1.populate = ext_1->populate;
                o->ext_1.alloc_size = ext_1->alloc_size;
                o->ext_1.fixed_size = ext_1->fixed_size;
            }

            if ( ref_cur == NULL )
            {
                if ( mgr->reader == NULL )
                {
                    rc = ReferenceList_OpenCursor( mgr );
                }
                if ( rc == 0 )
                {
                    o->ref_reader = mgr->reader;
                    o->ref_cols = mgr->reader_cols;
                }
            }
            else
            {
                memmove( o->ref_cols_own, ReferenceList_cols, sizeof( o->ref_cols_own ) );
                o->ref_cols = o->ref_cols_own;
                rc = TableReader_MakeCursor( &o->ref_reader, ref_cur, o->ref_cols_own );
            }

            if ( align_cur == NULL )
            {
                bool b_assign = ( mgr->iter != NULL );
                if ( !b_assign )
                {
                    rc = ReferenceList_OpenCursor2( mgr, ids );
                    b_assign = ( rc == 0 );
                }
                if ( b_assign )
                {
                    o->align_reader = mgr->iter;
                    o->align_cols = mgr->iter_cols;
                }
            }
            else
            {
                memmove( o->align_cols_own, PlacementIterator_cols, sizeof( o->align_cols_own ) );
                o->align_cols = o->align_cols_own;
                o->align_curs = align_cur;
                rc = TableReader_MakeCursor( &o->align_reader, align_cur, o->align_cols );
            }

            if ( rc == 0 )
            {
                int64_t first_ref_row_of_window_rel = ( ref_window_start / mgr->max_seq_len );
                int64_t first_ref_row_of_window_abs = ( cself->start_rowid + first_ref_row_of_window_rel );

                /* in bases */
                o->ref_window_start = ref_window_start;
                o->ref_window_len = ref_window_len;

                /* in reference-rows */
                o->last_ref_row_of_window_rel = ref_window_start;
                o->last_ref_row_of_window_rel += ( ref_window_len - 1 );
                o->last_ref_row_of_window_rel /= mgr->max_seq_len;
                o->rowcount_of_ref = ( cself->end_rowid - cself->start_rowid ) + 1;

                /* get effective starting offset based on overlap
                   from alignments which started before the requested pos */
                rc = TableReader_ReadRow( o->ref_reader, first_ref_row_of_window_abs );
                if ( rc == 0 )
                {
                    int64_t ref_pos_overlapped = calc_overlaped( o, ids );
                    ALIGN_DBG( "ref_pos_overlapped: %,li", ref_pos_overlapped );

                    /* the absolute row where we are reading from */
                    o->cur_ref_row_rel = ( ref_pos_overlapped / mgr->max_seq_len ) - 1;

                    VectorInit( &o->ids, 0, 100 );

                    o->ids_col = &o->ref_cols[ids == primary_align_ids ? ereflst_cn_PRIMARY_ALIGNMENT_IDS :
                            ( ids == secondary_align_ids ? ereflst_cn_SECONDARY_ALIGNMENT_IDS : ereflst_cn_EVIDENCE_INTERVAL_IDS ) ];

                    ALIGN_DBG( "iter.last_ref_row_of_window_rel: %,li", o->last_ref_row_of_window_rel );
                    ALIGN_DBG( "iter.rowcount_of_ref: %,li", o->rowcount_of_ref );
                    ALIGN_DBG( "iter.cur_ref_row_rel: %,li", o->cur_ref_row_rel );
                }
            }

            if ( rc != 0 )
            {
                ReferenceObj_Release( o->obj );
            }
        }
    }

    if ( rc == 0 )
    {
        enter_spotgroup ( o, spot_group );
        *iter = o;
        ALIGN_DBG( "iter for %s:%s opened 0x%p", cself->seqid, cself->name, o );
    }
    else
    {
        *iter = NULL;
        PlacementIteratorRelease( o );
        ALIGN_DBGERRP( "iter for %s:%s", rc, cself->seqid, cself->name );
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementIteratorAddRef ( const PlacementIterator *cself )
{
    return ReferenceList_AddRef(cself ? cself->obj->mgr : NULL);
}


static void CC PlacementIterator_whack_recs( void *item, void *data )
{
    PlacementRecordWhack( ( PlacementRecord * ) item );
}


LIB_EXPORT rc_t CC PlacementIteratorRelease ( const PlacementIterator *cself )
{
    if ( cself != NULL )
    {
        PlacementIterator* self = ( PlacementIterator* )cself;

        VectorWhack( &self->ids, PlacementIterator_whack_recs, NULL );

        if ( self->ref_reader != self->obj->mgr->reader )
        {
            TableReader_Whack( self->ref_reader );
        }
        if ( self->align_reader != self->obj->mgr->iter )
        {
            TableReader_Whack( self->align_reader );
        }
        ReferenceObj_Release( self->obj );
        ReferenceList_Release( self->obj->mgr );
        if ( self->spot_group != NULL )
        {
            free( ( void * )self->spot_group );
        }
        free( self );
    }
    return 0;
}


LIB_EXPORT rc_t CC PlacementIteratorRefWindow( const PlacementIterator *self,
                                               const char **idstr, INSDC_coord_zero* pos, INSDC_coord_len* len )
{
    rc_t rc = 0;

    if ( self == NULL || (idstr == NULL || pos == NULL || len == NULL ) )
    {
        rc = RC(rcAlign, rcType, rcAccessing, rcParam, rcInvalid);
    }
    else
    {
        if ( idstr != NULL ) { *idstr = self->obj->seqid; }
        if ( pos != NULL )   { *pos = self->ref_window_start; }
        if ( len != NULL )   { *len = self -> ref_window_len; }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


LIB_EXPORT rc_t CC PlacementIteratorRefObj( const PlacementIterator * self,
                                            struct ReferenceObj const ** refobj )
{
    rc_t rc = 0;

    if ( self == NULL || refobj == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        *refobj = self->obj;
    }
    ALIGN_DBGERR( rc );
    return rc;
}


#if _DEBUGGING
/*
static void CC PlacementRecordVector_dump( void *item, void *data )
{
    const PlacementRecord* i = ( const PlacementRecord* )item;
    ALIGN_DBG( " {pos:%,u, len:%,u, id:%,li}", i->pos, i->len, i->id );
}
*/
#endif


/* this comparison function performs last-to-first
   ordering in the Vector. for this reason, all
   comparisons and return values are reversed. */
static int64_t CC PlacementRecordVector_cmp( const void** left, const void** right, void* data )
{
    const PlacementRecord* l = *( ( const PlacementRecord** )left );
    const PlacementRecord* r = *( ( const PlacementRecord** )right );

    /* order by pos desc
       return right - left rather than
       normal left - right to reverse result */
    int32_t d = r -> pos - l -> pos;
    if ( d != 0 )
        return d;

    /* ...len asc */
    if ( l -> len != r -> len )
        return (int64_t)l -> len - (int64_t)r -> len;

    /* ...id desc */
    return r -> id < l -> id ? -1 : r -> id > l -> id;
}


static rc_t allocate_populate_rec( const PlacementIterator *cself,
                                   PlacementRecord **rec,
                                   struct VCursor const *curs,
                                   int64_t id,
                                   INSDC_coord_zero apos,
                                   INSDC_coord_len alen )
{
    rc_t rc = 0;

    {
        PlacementRecExtensionInfo * ext_info;
        size_t spot_group_len, size0, size1, total_size;

        if ( cself->spot_group_len > 0 )
        {
            /* use the read-group from the PlacementIterator as an fix override ... */
            spot_group_len = cself->spot_group_len;
        }
        else
        {
            if ( cself->spot_group != NULL )
            {
                /* iterator has an empty (not NULL) spot-group: use the orignal data! */
                spot_group_len = cself->align_cols[eplacementiter_cn_READ_GROUP].len;
            }
            else
            {
                /* do not use spotgroups at all ! */
                spot_group_len = 0;
            }
        }

        /* use callback or fixed size to discover the size of portions 0 and 1 */
        if ( cself->ext_0.alloc_size != NULL )
        {
            rc = cself->ext_0.alloc_size( curs, id, &size0, cself->ext_0.data, cself->placement_ctx );
            if ( rc != 0 )
                return rc;
        }
        else
            size0 = cself->ext_0.fixed_size;

        if ( cself->ext_1.alloc_size != NULL )
        {
            rc = cself->ext_1.alloc_size( curs, id, &size1, cself->ext_1.data, cself->placement_ctx );
            if ( rc != 0 )
                return rc;
        }
        else
            size1 = cself->ext_1.fixed_size;
        
        /* allocate the record ( or take it from a pool ) */
        total_size = ( sizeof **rec ) + spot_group_len + ( 2 * ( sizeof *ext_info ) ) + size0 + size1;
        *rec = calloc( 1, total_size );
        if ( *rec == NULL )
        {
            rc = RC( rcAlign, rcType, rcAccessing, rcMemory, rcExhausted );
        }
        else
        {
            PlacementRecord *pr = * rec;
            uint8_t * ptr = ( uint8_t * )( * rec );
            ptr += sizeof ( **rec );
            pr->spot_group = (char *)ptr;
            ptr += spot_group_len ;
            ext_info = ( PlacementRecExtensionInfo * )ptr;

            /* prepopulate the core-record : */
            pr->id  = id;               /* the row-id */
            pr->ref = cself->obj;       /* the ReferenceObj it refers to */
            pr->pos = apos;             /* the positon on the reference */
            pr->len = alen;             /* the length on the reference */
            pr->mapq = cself->align_cols[eplacementiter_cn_MAPQ].base.i32[ 0 ]; /* mapq */

            /* populate the spot-group : (with the values discovered at the "size-phase" ) */
            pr->spot_group_len = spot_group_len;
            if ( cself->spot_group_len > 0 )
            {
                /* we make a copy of the spot-group-override commin from the iterator */
                string_copy ( pr->spot_group, spot_group_len, cself->spot_group, spot_group_len );
            }
            else
            {
                if ( cself->spot_group != NULL )
                {
                    /* iterator has an empty (not NULL) spot-group: use the orignal data! */
                    string_copy( pr->spot_group, spot_group_len,
                                 cself->align_cols[eplacementiter_cn_READ_GROUP].base.str, spot_group_len );
                }
            }

            ext_info[ 0 ].data = cself->ext_0.data;          /* the opt. context ptr. */
            ext_info[ 0 ].destroy = cself->ext_0.destroy;    /* the opt. destructor */
            ext_info[ 0 ].size = size0;                      /* discovered size from above */

            ext_info[ 1 ].data = cself->ext_1.data;          /* the opt. context ptr. */
            ext_info[ 1 ].destroy = cself->ext_1.destroy;    /* the opt. destructor */
            ext_info[ 1 ].size = size1;                      /* discovered size from above */

            /* pass the record now to the opt. populate-callbacks */
            if ( cself->ext_0.populate != NULL )
            {
                void * obj = PlacementRecordCast ( pr, placementRecordExtension0 );
                rc = cself->ext_0.populate( obj, pr, curs,
                                            cself->ref_window_start,
                                            cself->ref_window_len,
                                            cself->ext_0.data,
                                            cself->placement_ctx );
                if ( rc != 0 && cself->ext_0.destroy != NULL )
                {
                    void *obj = PlacementRecordCast ( pr, placementRecordExtension0 );
                    cself->ext_0.destroy( obj, cself->ext_0.data );
                }
            }

            if ( rc == 0 && cself->ext_1.populate != NULL )
            {
                void * obj = PlacementRecordCast ( pr, placementRecordExtension1 );
                rc = cself->ext_1.populate( obj, pr, curs, 
                                            cself->ref_window_start,
                                            cself->ref_window_len,
                                            cself->ext_1.data,
                                            cself->placement_ctx );
                if ( rc != 0 )
                {
                    if ( cself->ext_1.destroy != NULL )
                    {
                        void *obj = PlacementRecordCast ( pr, placementRecordExtension1 );
                        cself->ext_1.destroy( obj, cself->ext_1.data );
                    }
                    if ( cself->ext_0.destroy != NULL )
                    {
                        void *obj = PlacementRecordCast ( pr, placementRecordExtension0 );
                        cself->ext_0.destroy( obj, cself->ext_0.data );
                    }

                }
            }

            if ( rc != 0 )
            {
                /* free */
                free( *rec );
                *rec = NULL;
            }
        }
    }
    return rc;
}


static rc_t make_alignment( PlacementIterator *self, int64_t align_id,
                            INSDC_coord_zero apos, INSDC_coord_len alen )
{
    PlacementRecord *rec;
    rc_t rc = allocate_populate_rec( self, &rec, self->align_curs, align_id, apos, alen );
    if ( rc == 0 )
    {
        /*ALIGN_DBG("align %p: {%li, %u, %u} - added[%u]", rec, cself->ids_col->base.i64[i],
            apos, alen, VectorLength(&cself->ids));*/
        rc = VectorAppend( &self->ids, NULL, rec );
        if ( rc != 0 )
            PlacementRecordWhack( rec );
    }
    else
    {
        if ( GetRCState( rc ) == rcIgnored )
            rc = 0; /* do not break the loop if a record is filtered out! */
    }
    return rc;
}


/*
  we have read a single row from REFERENCE, including the
  alignment ids. use each alignment id from this row to
  populate internal vector
*/
static rc_t read_alignments( PlacementIterator *self )
{
    rc_t rc = 0;
    uint32_t i;
    /* fill out vector */
    /*ALIGN_DBG("align rows: %u", cself->ids_col->len);*/
    for ( i = 0; rc == 0 && i < self->ids_col->len; i++ )
    {
        int64_t row_id = self->ids_col->base.i64[ i ];
        rc = TableReader_ReadRow( self->align_reader, row_id );
        if ( rc == 0 )
        {
            INSDC_coord_zero apos = self->align_cols[ eplacementiter_cn_REF_POS ].base.coord0[ 0 ];
            INSDC_coord_len alen  = self->align_cols[ eplacementiter_cn_REF_LEN ].base.coord_len[ 0 ];

#if 0
            ALIGN_DBG( "alignment read: {row_id:%,li, apos:%,d, alen:%u}", row_id, apos, alen );
#endif

            /* at this point we have the position of the alignment.
               we want it to intersect with the window */

            /* test mapq */
            if ( self->align_cols[eplacementiter_cn_MAPQ].base.i32[ 0 ] < self->min_mapq )
                continue;

            /* case 1: alignment, which is expressed in
               linear coordinates, is entirely to left of
               window. */
            if ( (INSDC_coord_zero)( apos + alen ) <= self -> ref_window_start )
            {
                /* ignore this alignment */
                continue;
            }

            /* case 2: alignment is to the right of window */
            if ( apos >= ( self -> ref_window_start + self -> ref_window_len ) )
            {
                /* if not circular, it cannot intersect */
                if ( ! self -> obj -> circular )
                {
                    /* this seems like it indicates end of window,
                       however there is no guarantee that the ids
                       being fetched are in reference order. they are
                       supposed to be in row order ( cluster ) to
                       reduce random access to cursor. unless row
                       order is guaranteed to be reference order,
                       we cannot know that the loop can be terminated. */
                    continue;
                }

                /* circular, but still within linear range
                   means it cannot wrap around to intersect */
                if ( (INSDC_coord_zero)( apos + alen ) <= self->obj->seq_len )
                    continue;

            }
            else if ( ( self->obj->circular )&&
                       ( apos + alen > self->obj->seq_len )&&
                       ( self->cur_ref_row_rel < 0 ) ) 
            {
                /* the end of the alignment sticks over the end of the reference! 
                   ---> we have the rare case of an alignment that wraps arround !
                   let as insert the alignment 2 times!
                   ( one with neg. position, one at real position ) */
                rc =  make_alignment( self, row_id, apos - self->obj->seq_len, alen );
            }

            /* having arrived here, we know the alignment intersects our window
               apos MAY be < 0 if the alignment wrapped around */
            if ( rc == 0 && self->cur_ref_row_rel >= 0 )
                rc =  make_alignment( self, row_id, apos, alen );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementIteratorNextAvailPos( const PlacementIterator *cself,
    INSDC_coord_zero *pos, INSDC_coord_len *len )
{
    rc_t rc = 0;
    PlacementIterator* self = (PlacementIterator*)cself;

    if ( self == NULL || ( pos == NULL && len == NULL ) )
    {
        rc = RC( rcAlign, rcType, rcSelecting, rcParam, rcInvalid );
    }
    else
    {
        while ( rc == 0 && VectorLength( &self->ids ) == 0 )
        {
            /* read ids */

            self->cur_ref_row_rel++;   /* increment row offset */

#if 0
            ALIGN_DBG( "ref row: ref-start-row-id:%,li - curr-rel-row:%,li - of:%,li",
                       self->obj->start_rowid, self->cur_ref_row_rel, self->last_ref_row_of_window_rel );
#endif

            if ( self->cur_ref_row_rel > self->last_ref_row_of_window_rel )
                rc = SILENT_RC( rcAlign, rcType, rcSelecting, rcRange, rcDone );
            else
            {
                int64_t row = ( self->obj->start_rowid + self->cur_ref_row_rel );
                if ( self->cur_ref_row_rel < 0 )
                    row += self->rowcount_of_ref;

                rc = TableReader_ReadRow( self->ref_reader, row );
                if ( rc == 0 )
                    rc = read_alignments( self );

                if ( ( rc == 0 || GetRCState( rc ) == rcDone) && VectorLength( &cself->ids ) > 0 )
                {
                    VectorReorder( &self->ids, PlacementRecordVector_cmp, NULL );
#if _DEBUGGING && 0
                    ALIGN_DBG( "REFERENCE row %li %u recs order by pos asc, len desc, id asc",
                                row, VectorLength( &cself->ids ) );
                    VectorForEach( &self->ids, true, PlacementRecordVector_dump, NULL );
#endif
                }
            }
        }

        if ( rc == 0 || GetRCState( rc ) == rcDone )
        {
            uint32_t count = VectorLength( &cself->ids );
            if ( count > 0 )
            {
                PlacementRecord * r = VectorLast( &cself->ids );
                rc = 0;
                if ( pos != NULL ) { *pos = r->pos; }
                if ( len != NULL ) { *len = r->len; }

#if 0
                ALIGN_DBG( "PlacementIteratorNextAvailPos( id=%,li, pos=%,d, len=%,u, n=%,u )", r->id, r->pos, r->len, count );
#endif

                if ( !( cself->obj->circular ) && ( r->pos >= ( cself->ref_window_start + cself->ref_window_len ) ) )
                {
                    /* the alignment !starts! after the end of the of the requested window! */
                    rc = SILENT_RC( rcAlign, rcType, rcSelecting, rcRange, rcDone );
                }
            }
            else
            {
                ALIGN_DBG( "PlacementIteratorNextAvailPos( no placements )", 0 );
            }
        }
    }

    if ( rc != 0 && GetRCState(rc) != rcDone )
        ALIGN_DBGERR( rc );

    return rc;
}


LIB_EXPORT rc_t CC PlacementIteratorNextRecordAt( PlacementIterator *cself,
    INSDC_coord_zero pos, const PlacementRecord **rec )
{
    rc_t rc = 0;
    if ( cself == NULL || rec == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        uint32_t vlen = VectorLength( &cself->ids );
        *rec = NULL;
        if ( vlen > 0 )
        {
            PlacementRecord* r = VectorLast( &cself->ids );
            if ( r->pos == pos )
            {
                VectorRemove( &cself->ids, vlen - 1, (void**)rec );
            }
        }
    }

    if ( rc == 0 && *rec == NULL )
    {
        rc = SILENT_RC( rcAlign, rcType, rcSelecting, rcOffset, rcDone );
    }
    else
    {
        ALIGN_DBGERR( rc );
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementIteratorNextIdAt( PlacementIterator *cself,
                                              INSDC_coord_zero pos, int64_t *row_id, INSDC_coord_len *len )
{
    rc_t rc = 0;
    const PlacementRecord* r = NULL;

    if ( cself == NULL || row_id == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcInvalid );
    }
    else
    {
        rc = PlacementIteratorNextRecordAt( cself, pos, &r );
        if ( rc  == 0 )
        {
            *row_id = r->id;
            if ( len != NULL ) { *len = r->len; }
            PlacementRecordWhack( r );
        }
    }

    if ( GetRCState( rc ) != rcDone )
    {
        ALIGN_DBGERR( rc );
    }
    return rc;
}
