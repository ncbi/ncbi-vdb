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
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>

#include "reader-cmn.h"
#include "debug.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>

struct TableReader
{
    const VCursor* curs;
    const TableReaderColumn* cols;
    int64_t curr;
};


rc_t CC TableReader_Make( const TableReader** cself, const VTable* table,
                          TableReaderColumn* cols, size_t cache )
{
    rc_t rc = 0;
    const VCursor* curs;

    if ( table == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcInvalid );
    }
    else
    {
        rc = VTableCreateCachedCursorRead( table, &curs, cache );
        if ( rc == 0 )
        {
            rc = TableReader_MakeCursor( cself, curs, cols );
            VCursorRelease( curs );
        }
    }
    ALIGN_DBGERR( rc );
    return rc;
}


rc_t CC TableReader_MakeCursor( const TableReader** cself, const VCursor* cursor,
                                TableReaderColumn* cols )
{
    rc_t rc = 0;
    TableReader* obj = NULL;

    if ( cself == NULL || cursor == NULL || cols == NULL || cols->name == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcInvalid );
    }
    else if ( ( obj = calloc( 1, sizeof( *obj ) ) ) == NULL )
    {
        rc = RC( rcAlign, rcType, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        rc = VCursorAddRef( cursor );
        if ( rc == 0 )
        {
            obj->curs = cursor;
/*            obj->cursor_open = false; */
            obj->cols = cols;
            while ( rc == 0 && cols->name != NULL)
            {
                if ( !( cols->flags & ercol_Skip ) )
                {
                    rc = VCursorAddColumn( obj->curs, &cols->idx, "%s", cols->name );
                    if ( rc != 0 )
                    {
                        if ( ( rc != 0 && ( cols->flags & ercol_Optional ) ) || GetRCState( rc ) == rcExists )
                        {
                            rc = 0;
                        }
                        else
                        {
                            ALIGN_DBGERRP( "cannot add column '%s' to cursor", rc, cols->name );
                        }
                    }
                }
                cols++;
            }
            if ( rc == 0 )
            {
                rc = VCursorOpen( obj->curs );
            }
        }
    }


    if( rc == 0 )
    {
        *cself = obj;
/*        ALIGN_DBG("ok%c", '!'); */
    }
    else
    {
        TableReader_Whack( obj );
        ALIGN_DBGERRP( "TableReader_MakeCursor() failed", rc, 0 );
    }
    return rc;
}


void CC TableReader_Whack( const TableReader* cself )
{
    if ( cself != NULL )
    {
        VCursorRelease( cself->curs );
        free( ( TableReader* ) cself );
    }
}

/*
rc_t CC TableReader_OpenCursor( const TableReader* cself )
{
    TableReader * tr = ( TableReader * )cself;
    VCursor * curs = ( VCursor * )tr->curs;
    rc_t rc = VCursorOpen( curs );
    tr->cursor_open = ( rc == 0 );
    return rc;
}
*/

rc_t CC TableReader_ReadRow( const TableReader* cself, int64_t rowid )
{
    rc_t rc = 0;
	TableReaderColumn * c = NULL;
	
    if ( cself == NULL )
    {
        rc = RC(rcAlign, rcType, rcOpening, rcSelf, rcNull);
    }
    else if ( cself->curr != rowid )
    {
/*
        if ( !cself->cursor_open )
            rc = TableReader_OpenCursor( cself );
*/
        if ( rc == 0 )
        {
            rc = VCursorCloseRow( cself->curs );
            if ( rc == 0 )
            {
                rc = VCursorSetRowId( cself->curs, rowid );
                if ( rc == 0 )
                {
                    rc = VCursorOpenRow( cself->curs );
                    if ( rc == 0 )
                    {
                        uint32_t boff = 0;
                        c = ( TableReaderColumn* )( cself->cols );
                        while ( c->name != NULL && rc == 0 )
                        {
                            if ( c->idx != 0 )
                            {
                                /* TBD - FIX ME
                                   this can be dangerous, since VCursorCellData
                                   can cause cache flushes, invalidating previous
                                   reads. THESE MUST BE FETCHED UPON DEMAND, NOT
                                   PREFETCHED!!
                                */
                                rc = VCursorCellData( cself->curs, c->idx, NULL, (const void**)&c->base.var, &boff, &c->len );
                                if ( rc != 0 )
                                {
                                    if ( c->flags & ercol_Optional )
                                        rc = 0;
                                    c->base.var = NULL;
                                    c->len = 0;
                                }
                                else if ( boff != 0 )
                                {
                                    rc = RC( rcAlign, rcType, rcReading, rcData, rcUnsupported );
                                }
                            }
                            c++;
                        }
                    }
                }
            }
        }
    }

    if ( rc != 0 )
    {
        ALIGN_DBGERRP( "column %s row %ld", rc, c ? c->name : "<none>", rowid );
    }
    else
    {
        ( ( TableReader* )cself )->curr = rowid;
    }
    return rc;
}


rc_t CC TableReader_IdRange( const TableReader* cself, int64_t* first, uint64_t* count )
{
    rc_t rc = 0;
    if ( cself == NULL )
    {
        rc = RC( rcAlign, rcType, rcRetrieving, rcSelf, rcNull );
    }
    else
    {
/*
        if ( !cself->cursor_open )
            rc = TableReader_OpenCursor( cself );
        if ( rc == 0 )
*/
            rc = VCursorIdRange( cself->curs, 0, first, count );
    }
    return rc;
}


rc_t CC TableReader_OpenIndex( const TableReader* cself, const char* name, const KIndex** idx )
{
    rc_t rc = 0;

    if ( cself == NULL || name == NULL || idx == NULL )
    {
        rc = RC( rcAlign, rcType, rcAccessing, rcParam, rcNull );
    }
    else
    {
/*
        if ( !cself->cursor_open )
            rc = TableReader_OpenCursor( cself );
        if ( rc == 0 )
        {
*/
            const VTable* tbl;
            rc = VCursorOpenParentRead( cself->curs, &tbl );
            if ( rc == 0 )
            {
                rc = VTableOpenIndexRead( tbl, idx, "%s", name );
                VTableRelease( tbl );
            }
/*        } */
    }
    return rc;
}


rc_t CC TableReader_PageIdRange( const TableReader *cself, int64_t rowid,
                                 int64_t *pfirst, int64_t *plast )
{
    rc_t rc = 0;
    
    if ( cself == NULL )
    {
        rc = RC( rcAlign, rcType, rcOpening, rcSelf, rcNull );
    }
    else
    {
/*
        if ( !cself->cursor_open )
            rc = TableReader_OpenCursor( cself );
        if ( rc == 0 )
        {
*/
            if ( pfirst != NULL || plast != NULL )
            {
                int64_t first = INT64_MAX;
                int64_t last = INT64_MIN;
                unsigned i;
                
                for ( i = 0; cself->cols[ i ].name != NULL; ++i )
                {
                    if ( cself->cols[i].idx != 0 )
                    {
                        int64_t tfirst;
                        int64_t tlast;
                        
                        rc = VCursorPageIdRange( cself->curs, cself->cols[ i ].idx, rowid, &tfirst, &tlast );
                        if ( rc == 0 )
                        {
                            if ( first > tfirst ) { first = tfirst; }
                            if ( last < tlast ) { last = tlast; }
                        }
                    }
                }
                if ( pfirst != NULL ) { *pfirst = first; }
                if ( plast  != NULL ) { *plast  = last; }
            }
/*        } */
    }
    return rc;
}
