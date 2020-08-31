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
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vfs/path.h>

#include "wgs.h"

static void WGS_stamp(WGS *self) {
    static uint64_t generation = 0;
    self->lastAccessStamp = generation++;
}

static rc_t openCursor(WGS* self, VDatabase const *db)
{
    VTable const *tbl = NULL;
    rc_t rc = VDatabaseOpenTableRead(db, &tbl, "SEQUENCE");
    VDatabaseRelease(db);
    if (rc) return rc;

    rc = VTableCreateCachedCursorRead(tbl, &self->curs, 0);
    VTableRelease(tbl);
    if (rc) return rc;

    rc = VCursorAddColumn(self->curs, &self->colID, "(INSDC:4na:bin)READ");
    if (rc == 0) {
        rc = VCursorOpen(self->curs);
        if (rc == 0) {
            WGS_stamp(self);
            return 0;
        }
    }
    WGS_close(self);
    return rc;
}

rc_t WGS_reopen(WGS *self, VDBManager const *mgr, unsigned seq_id_len, char const *seq_id)
{
    VDatabase const *db = NULL;
    rc_t rc = 0;

    if (self->url)
        rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, self->url);
    else
        rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%.*s", (int)seq_id_len, seq_id);

    if (rc) return rc;

    return openCursor(self, db);
}

void WGS_close(WGS *self)
{
    VCursorRelease(self->curs);
    self->curs = NULL;
}

void WGS_whack(WGS *self)
{
    VCursorRelease(self->curs);
    VPathRelease(self->url);
}

rc_t WGS_init(WGS *self, VPath const *url, VDatabase const *db)
{
    rc_t rc = 0;

    memset(self, 0, sizeof(*self));
    VDatabaseAddRef(db);
    rc = openCursor(self, db);
    if (rc == 0) {
        self->url = url;
        VPathAddRef(url);
    }
    return rc;
}

unsigned WGS_getBases(WGS *self, uint8_t *dst, unsigned start, unsigned len, int64_t row)
{
    void const *value = NULL;
    uint32_t length = 0;
    rc_t const rc = VCursorCellDataDirect(self->curs, row, self->colID, NULL, &value, NULL, &length);

    if (rc == 0 && start < length) {
        unsigned const remain = length - start;
        unsigned const n = remain < len ? remain : len;
        memmove(dst, ((uint8_t const *)value) + start, n);
        WGS_stamp(self);
        return n;
    }
    return 0;
}

unsigned WGS_splitName(int64_t *prow, unsigned const namelen, char const *name)
{
    unsigned digits = 0;
    unsigned i;

    for (i = 0; i < namelen; ++i) {
        int const ch = name[i];
        if (ch >= '0' && ch <= '9')
            ++digits;
        else
            digits = 0;
    }
    if (digits > 2) {
        unsigned const row_len = digits - 2;
        unsigned const acc_len = namelen - row_len;
        int64_t row = 0;

        for (i = acc_len; i < namelen; ++i) {
            int const ch = name[i];
            row = row * 10 + (ch - '0');
        }
        *prow = row;
        return acc_len;
    }
    return 0;
}
char const *WGS_Scheme(void) {
    return "NCBI:WGS:db:contig";
}
