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
#include <os-native.h>
#include <sysalloc.h>

#include "reader-cmn.h"
#include "reference-cmn.h"
#include "reader-wgs.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

static const TableReaderColumn TableReaderWGS_cols[] =
{
    /* order important, see code below! */
    {0, "(INSDC:dna:text)READ", {NULL}, 0, 0},
    {0, "(INSDC:4na:bin)READ", {NULL}, 0, ercol_Skip},
    {0, NULL, {NULL}, 0, 0}
};

struct TableReaderWGS {
    TableReader const *base;
    TableReaderColumn cols[sizeof(TableReaderWGS_cols)/sizeof(TableReaderWGS_cols[0])];
    TableReaderColumn const *read;
};

rc_t TableReaderWGS_MakeTable(TableReaderWGS const **const pself,
                              VDBManager const *vmgr,
                              VTable const *const table,
                              uint32_t const options,
                              size_t const cache)
{
    assert(pself != NULL);
    assert(table != NULL);
    {
        TableReaderWGS *const self = calloc(1, sizeof(*self));
    
        memmove(self->cols, TableReaderWGS_cols, sizeof(TableReaderWGS_cols));
        self->read = &self->cols[0];
    
        if (options != 0) {
            self->cols[0].flags |=  ercol_Skip;
            self->cols[1].flags &= ~ercol_Skip;
            self->read = &self->cols[1];
        }
        {
            rc_t const rc = TableReader_Make(&self->base, table, self->cols, cache);
            if (rc == 0) {
                *pself = self;
                return 0;
            }
            free(self);

            return rc;
        }
    }
}

void TableReaderWGS_Whack(TableReaderWGS const *const self)
{
    if (self) {
        TableReader_Whack(self->base);
        free((TableReaderWGS *)self);
    }
}

rc_t TableReaderWGS_SeqLength(TableReaderWGS const *const self, int64_t row, INSDC_coord_len *const result)
{
    assert(self != NULL);
    assert(result != NULL);
    {
        rc_t const rc = TableReader_ReadRow(self->base, row);
        if (rc == 0)
            *result = self->read->len;

        ALIGN_DBGERR(rc);
        return rc;
    }
}

rc_t TableReaderWGS_Circular(TableReaderWGS const *const self, int64_t row, bool *const result)
{
    assert(self != NULL);
    assert(result != NULL);
    
    *result = false;

    return 0;
}

rc_t TableReaderWGS_MD5(TableReaderWGS const *const self, int64_t row, uint8_t const **const result)
{
    assert(self != NULL);
    assert(result != NULL);
    
    *result = NULL;
    
    return 0;
}

rc_t TableReaderWGS_Read(TableReaderWGS const *const self, int64_t const row,
                         INSDC_coord_zero const offset,
                         INSDC_coord_len const len,
                         uint8_t *const buffer,
                         INSDC_coord_len *const written)
{
    assert(self != NULL);
    assert(buffer != NULL);
    assert(written != NULL);
    
    *written = 0;
    if (len == 0)
        return 0;
    {
        rc_t const rc = TableReader_ReadRow(self->base, row);
        if (rc == 0) {
            INSDC_coord_len const max = self->read->len;

            if (offset >= max)
                return 0;
            {
                uint8_t const *const src = self->read->base.u8 + offset;
                INSDC_coord_len const end = offset + len;
                INSDC_coord_len const N = end < max ? (end - offset) : (max - offset);
        
                *written = N;
                memmove(buffer, src, N);
            }
        }
        ALIGN_DBGERR(rc);
        return rc;
    }
}
