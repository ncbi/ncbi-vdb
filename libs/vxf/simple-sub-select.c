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
#include <vdb/xform.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>


#include <klib/rc.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <insdc/sra.h>
#include <insdc/insdc.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct SubSelect SubSelect;
struct SubSelect
{
    const VCursor *curs;
    uint32_t idx;
    const VCursor *native_curs; /** Native cursors are either master cursor or sub_cursors - no need to AddRef **/
    bool  first_time;
    char  *col_name;
    uint32_t col_name_len;
};

static
rc_t CC simple_sub_select ( void *data, const VXformInfo *info,
    int64_t local_row_id, VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    const SubSelect *self = ( const void* ) data;

    rc_t rc;
    const void *base;
    uint32_t elem_bits, boff, row_len;
    int32_t  idx=0; /*** 0 - means no index ***/

    const int64_t *remote_row_id = argv [ 0 ] . u . data . base;
    remote_row_id += argv [ 0 ] . u . data . first_elem;

    assert ( argv [ 0 ] . u . data . elem_bits == 64 );
    if(argv [ 0 ] . u . data . elem_count == 0){/** alow empty returns ***/
        rc = KDataBufferResize ( rslt -> data, 0 );
        rslt -> elem_count = 0;
        return 0;
    }
    assert ( argv [ 0 ] . u . data . elem_count == 1 );
    if(argc > 1 ){
    	const int32_t *remote_idx = argv[1].u.data.base;
        idx=remote_idx[argv[1].u.data.first_elem];
    }
    if(self->native_curs && self->first_time){ /*** can we use native cursor for the data? ***/
        SubSelect *mself = (SubSelect *)self;
        uint32_t idx;
        rc = VCursorAddColumn ( mself->native_curs, & idx, "%.*s", mself->col_name_len, mself->col_name );
        if(rc == 0 || GetRCState(rc) == rcExists){
            VCursorRelease( mself->curs);
            mself->curs = mself->native_curs;
            mself->idx  = idx;
        }
        mself->first_time = false;
        rc = 0; /** reset rc? **/
    }
	
    /* sub-select */
    rc = VCursorCellDataDirect ( self -> curs, * remote_row_id, self -> idx,
                                 & elem_bits, & base, & boff, & row_len );
    if ( rc == 0 )
    {
        uint8_t *cbase=(uint8_t*)base + (boff>>3);
        rslt -> data -> elem_bits = elem_bits;
        if(idx < 0 || idx > row_len){ /** out of bounds **/
            rc = KDataBufferResize ( rslt -> data, 0 );
            rslt -> elem_count = 0;
            return rc;
        } else if (idx > 0){ /*** do subset ***/
            row_len=1;
            cbase += (elem_bits>>3)*(idx-1);
            boff  += elem_bits * (idx-1);
        }
	
        rc = KDataBufferResize ( rslt -> data, row_len );
        if ( rc == 0 )
        {
            bitsz_t bits = ( bitsz_t ) elem_bits * row_len;

            if ( ( elem_bits & 7 ) != 0 )
                bitcpy ( rslt -> data -> base, 0, base, boff, bits );
            else{
                assert((boff&7)==0);
                memmove ( rslt -> data -> base, cbase, bits>>3 );
            }	

            rslt -> elem_count = row_len;
        }
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = KDataBufferResize ( rslt -> data, 0 );
        rslt -> elem_count = 0;
    }

    return rc;
}


/* open_sub_cursor
 */
static
rc_t open_sub_cursor ( SubSelect **fself, const VXfactInfo *info, const VFactoryParams *cp, const VCursor *native_curs )
{
    rc_t rc=0;
    const VTable *tbl, *ftbl = NULL;
    char name[256]="";
    const VCursor *curs;

    if ( cp -> argv [ 0 ] . count > 0 )
    {
        sprintf(name,"%.*s",(int)cp->argv[0].count,cp->argv[0].data.ascii);
        rc = VCursorLinkedCursorGet(native_curs,name,&curs);
        if(rc == 0)
        {
            /* curs is an owned reference */
            rc = VCursorOpenParentRead(curs,&ftbl);
            if(rc != 0)
            {
                VCursorRelease ( curs );
                return rc;
            }
            tbl = ftbl;
        }
        else
        {
            const VDatabase *db;
            uint64_t cache_size = 32*1024*1024;
            uint64_t native_cursor_cache_size = VCursorGetCacheCapacity(native_curs);

            rc = VTableOpenParentRead ( info -> tbl, & db );
            if(rc != 0) return rc;
            rc = VDatabaseOpenTableRead ( db, & ftbl, "%s", name);
            if(rc != 0) return rc;
            VDatabaseRelease ( db );
            tbl = ftbl;
            if(native_cursor_cache_size/4 > cache_size){
                /* share cursor size with native cursor **/
                cache_size = native_cursor_cache_size/4;
                native_cursor_cache_size -= cache_size;
                VCursorSetCacheCapacity((VCursor*)native_curs,native_cursor_cache_size);
            }
            rc = VTableCreateCachedCursorRead ( tbl, (const VCursor**)& curs, cache_size ); /*** some random io is expected ***/
            if(rc != 0) return rc;
            rc = VCursorPermitPostOpenAdd( curs );
            if(rc != 0) return rc;
            rc = VCursorOpen( curs );
            if(rc != 0) return rc;
            rc = VCursorLinkedCursorSet(native_curs,name,curs);
            if(rc != 0) return rc;
        }
        native_curs = NULL;
    }
    else /** we don't know if native_curs permits adding columns **/
    {
        tbl = info -> tbl;
        rc = VTableCreateCachedCursorRead ( tbl, & curs, 0 ); /*** some random io is expected ***/
    }
    if ( rc == 0 )
    {
        uint32_t idx;
        rc = VCursorAddColumn ( curs, & idx, "%.*s",
                                ( int ) cp -> argv [ 1 ] . count, cp -> argv [ 1 ] . data . ascii );
        if ( rc == 0 || GetRCState(rc) == rcExists)
        {
            rc = VCursorOpen ( curs );
            if ( rc == 0 )
            {
                VTypedesc src;
                rc = VCursorDatatype ( curs, idx, NULL, & src );
                if ( rc == 0 )
                {
                    /* selected column should have same characteristics */
                    if ( src . domain != info -> fdesc . desc . domain                 ||
                         src . intrinsic_bits != info -> fdesc . desc . intrinsic_bits ||
                         src . intrinsic_dim != info -> fdesc . desc. intrinsic_dim )
                    {
                        rc = RC ( rcXF, rcFunction, rcConstructing, rcType, rcInconsistent );
                    }
                    else
                    {
                        SubSelect *self = malloc ( sizeof * self );
                        if ( self == NULL )
                            rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
                        else
                        {
                            self -> curs = curs;
                            self -> idx = idx;
                            self->native_curs = native_curs;
                            self->first_time  = true;
                            self->col_name_len  = cp->argv[1].count;
                            self->col_name = malloc(self->col_name_len);
                            memmove(self->col_name,cp->argv[1].data.ascii,self->col_name_len);
                            * fself = self;
                            if ( ftbl != NULL )
                                VTableRelease ( ftbl );
                            return 0;
                        }
                    }
                }
            }
        }
        VCursorRelease ( curs );
    }
    if ( ftbl != NULL )
        VTableRelease ( ftbl );

    return rc;
}


/* close_sub_cursor
 */
static
void CC close_sub_cursor ( void *data )
{
    SubSelect *self = data;
    if(self->curs != self->native_curs) VCursorRelease ( self -> curs );
    free ( self->col_name);
    free ( self );
}


/* 
 * function < type T >
 * T simple_sub_select < ascii tbl, ascii col > ( I64 row * I32 elem_idx(1-based) )
 *     = vdb:simple_sub_select;
 */
VTRANSFACT_IMPL ( vdb_simple_sub_select_1, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    SubSelect *fself;

    assert ( cp -> argc == 2 );
    assert ( dp -> argc == 1 || dp -> argc == 2);

    rc = open_sub_cursor ( & fself, info, cp, (const VCursor*)info->parms );
    if ( rc == 0 )
    {
        rslt -> self = fself;
        rslt -> u . rf = simple_sub_select;
        rslt -> variant = vftRow;
        rslt -> whack = close_sub_cursor;
    }

    return rc;
}

VTRANSFACT_IMPL ( vdb_simple_sub_select, 0, 9, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    return VTRANSFACT_NAME ( vdb_simple_sub_select_1 ) ( self, info, rslt, cp, dp );
}
