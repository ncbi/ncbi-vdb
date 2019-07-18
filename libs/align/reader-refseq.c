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
#include <klib/text.h>
#include <insdc/insdc.h>
#include <vdb/manager.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <align/reader-refseq.h>
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

static const TableReaderColumn TableReaderRefSeq_cols[] =
{
    /* order important, see code below! */
    {0, "(INSDC:dna:text)READ", {NULL}, 0, 0},
    {0, "(INSDC:4na:bin)READ", {NULL}, 0, ercol_Skip},
    {0, "QUALITY", {NULL}, 0, ercol_Skip | ercol_Optional},
    {0, "SEQ_LEN", {NULL}, 0, 0},
    {0, NULL, {NULL}, 0, 0}
};

struct TableReaderRefSeq {
    const TableReader* base;
    TableReaderColumn cols[sizeof(TableReaderRefSeq_cols)/sizeof(TableReaderRefSeq_cols[0])];
    char seq_id[256];
    uint32_t max_seq_len;
    INSDC_coord_len total_seq_len;
    bool circular;
    bool has_md5;
    uint8_t md5[16];
    const TableReaderColumn* read;
    const TableReaderColumn* seq_len;
};

LIB_EXPORT rc_t CC TableReaderRefSeq_MakeTable(const TableReaderRefSeq** cself, const VDBManager* vmgr,
                                               const VTable* table, uint32_t options, size_t cache)
{
    rc_t rc = 0;
    TableReaderRefSeq* self = NULL;

    if( cself == NULL || table == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (self = calloc(1, sizeof(*self))) == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcMemory, rcExhausted);
    } else {
        const TableReader* tmp;
        TableReaderColumn static_cols[] = {
            /* order important, see code below! */
            {0, "MAX_SEQ_LEN", {NULL}, 0, 0},
            {0, "SEQ_ID", {NULL}, 0, 0},
            {0, "TOTAL_SEQ_LEN", {NULL}, 0, 0},
            {0, "CIRCULAR", {NULL}, 0, 0},
            {0, "MD5", {NULL}, 0, ercol_Optional},
            {0, NULL, {NULL}, 0, 0}
        };
        if( (rc = TableReader_Make(&tmp, table, static_cols, 0)) == 0 ) {
            if( (rc = TableReader_ReadRow(tmp, 1)) == 0 ) {
                self->max_seq_len = static_cols[0].base.u32[0];
                self->total_seq_len = static_cols[2].base.u64[0];
                if( self->total_seq_len != static_cols[2].base.u64[0] ) {
                    rc = RC(rcAlign, rcType, rcConstructing, rcData, rcOutofrange);
                }
                if( static_cols[1].base.var != NULL ) {
                    if ( static_cols[ 1 ].len > sizeof( self->seq_id ) - 1 )
                    {
                        rc = RC( rcAlign, rcType, rcConstructing, rcBuffer, rcInsufficient );
                    }
                    else
                    {
                        string_copy( self->seq_id, ( sizeof self->seq_id ) -  1, static_cols[1].base.str, static_cols[1].len );
                        self->seq_id[ static_cols[ 1 ].len ] = '\0';
                    }
                }
                self->circular = static_cols[3].base.buul[0];
                if( static_cols[4].base.var != NULL ) {
                    memmove(self->md5, static_cols[4].base.var, sizeof(self->md5));
                    self->has_md5 = true;
                }
            }
            TableReader_Whack(tmp);
        }

        memmove(self->cols, TableReaderRefSeq_cols, sizeof(TableReaderRefSeq_cols));
        if( options & errefseq_4NA) {
            self->cols[0].flags |= ercol_Skip;
            self->cols[1].flags &= ~ercol_Skip;
            self->read = &self->cols[1];
        } else {
            self->read = &self->cols[0];
        }
        self->seq_len = &self->cols[3];
        rc = TableReader_Make(&self->base, table, self->cols, cache);
    }
    if( rc == 0 ) {
        *cself = self;
        /* ALIGN_DBG("table 0x%p opened 0x%p", table, self); */
    } else {
        TableReaderRefSeq_Whack(self);
        ALIGN_DBGERRP("table for 0x%p", rc, table);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableReaderRefSeq_MakePath(const TableReaderRefSeq** cself, const VDBManager* vmgr,
                                              const char* path, uint32_t options, size_t cache)
{
    rc_t rc = 0;
    const VTable* tbl = NULL;

    if( vmgr == NULL || path == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (rc = VDBManagerOpenTableRead(vmgr, &tbl, NULL, "%s", path)) == 0 ) {
        rc = TableReaderRefSeq_MakeTable(cself, vmgr, tbl, options, cache);
        VTableRelease(tbl);
    }
    if( rc == 0 ) {
        /* ALIGN_DBG("table %s opened 0x%p", path, *cself); */
    } else {
        ALIGN_DBGERRP("table for %s", rc, path);
    }
    return rc;
}

LIB_EXPORT void CC TableReaderRefSeq_Whack(const TableReaderRefSeq* cself)
{
    if( cself != NULL ) {
        /* ALIGN_DBG("table 0x%p closed", cself); */
        TableReader_Whack(cself->base);
        free((TableReaderRefSeq*)cself);
    }
}

LIB_EXPORT rc_t CC TableReaderRefSeq_SeqId(const TableReaderRefSeq* cself, const char** id, uint32_t* id_sz)
{
    rc_t rc = 0;
    if( cself == NULL || id == NULL || id_sz == NULL ) {
        rc = RC(rcAlign, rcType, rcReading, rcParam, rcNull);
    } else {
        *id = cself->seq_id;
        *id_sz = string_size(cself->seq_id);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC TableReaderRefSeq_SeqLength(const TableReaderRefSeq* cself, INSDC_coord_len* len)
{
    rc_t rc = 0;
    if( cself == NULL || len == NULL ) {
        rc = RC(rcAlign, rcType, rcReading, rcParam, rcNull);
    } else {
        *len = cself->total_seq_len;
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC TableReaderRefSeq_Circular(const TableReaderRefSeq* cself, bool* circular)
{
    rc_t rc = 0;
    if( cself == NULL || circular == NULL ) {
        rc = RC(rcAlign, rcType, rcReading, rcParam, rcNull);
    } else {
        *circular = cself->circular;
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC TableReaderRefSeq_MD5(const TableReaderRefSeq* cself, const uint8_t** md5)
{
    rc_t rc = 0;
    if( cself == NULL || md5 == NULL ) {
        rc = RC(rcAlign, rcType, rcReading, rcParam, rcNull);
    } else {
        *md5 = cself->has_md5 ? cself->md5 : NULL;
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC TableReaderRefSeq_Read(const TableReaderRefSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                          uint8_t* buffer, INSDC_coord_len* written)
{
    rc_t rc = 0;

    if ( cself == NULL || buffer == NULL || written == NULL )
	{
        rc = RC( rcAlign, rcType, rcReading, rcParam, rcNull );
    }
	else if( len == 0 )
	{
        *written = 0;
    }
	else
	{
		rc = ReferenceSeq_ReOffset( cself->circular, cself->total_seq_len, &offset );
		if ( rc == 0 )
		{
			INSDC_coord_len q = 0;
			*written = 0;
			do
			{
				int64_t rowid = offset / cself->max_seq_len + 1;
				INSDC_coord_zero s = offset % cself->max_seq_len;
				
				rc = TableReader_ReadRow( cself->base, rowid );
				if ( rc == 0 && ( cself->read->len == 0 || cself->read->base.str == NULL ) )
				{
					/* TableReader_ReadRow() can return rc == 0 for an optional column!
					   in these cases len/base.str are zero/NULL */
					rc = RC( rcAlign, rcType, rcReading, rcItem, rcNull );			
				}
				
				if ( rc == 0 )
				{
					q = cself->seq_len->base.coord_len[0] - s;
					if ( q > len )
					{
						q = len;
					}
					memmove( &buffer[*written], cself->read->base.str + s, q );
					*written += q;
					offset += q;
					len -= q;
				}
				
				/* SEQ_LEN < MAX_SEQ_LEN is last row unless it is CIRCULAR */
				if ( rc == 0 && ( cself->seq_len->base.coord_len[ 0 ] < cself->max_seq_len ) )
				{
					if ( !cself->circular )
					{
						break;
					}
					offset = 0;
				}
			} while( rc == 0 && q > 0 && len > 0 );
		}
	}
    ALIGN_DBGERR( rc );
    return rc;
}
