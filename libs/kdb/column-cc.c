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
 *============================================================================
 *
 */

#include <kdb/extern.h>

#define KONST const
#include "column-priv.h"
#undef KONST

#include "cc-priv.h"

#include <klib/rc.h>

#include <string.h>

struct col_check_ctx {
    CCReportFunc report;
    void *ctx;
    int found;
    rc_t rc;
    bool failed;
};

static
rc_t CC col_check_report(const CCReportInfoBlock *nfo, void *Ctx)
{
    struct col_check_ctx *ctx = Ctx;
    
    if (nfo->type == ccrpt_MD5) {
        const char *fname = nfo->info.MD5.file;
        
        if (strcmp(fname, "data") == 0)
            ctx->found |= 1;
        else if (strcmp(fname, "idx0") == 0)
            ctx->found |= 2;
        else if (strcmp(fname, "idx1") == 0)
            ctx->found |= 4;
        else if (strcmp(fname, "idx2") == 0)
            ctx->found |= 8;
        
        if (nfo->info.MD5.rc != 0 && !ctx->failed) {
            ctx->failed = true;
            ctx->rc = nfo->info.MD5.rc;
        }
    }
    else if (nfo->type == ccrpt_Done && nfo->info.done.rc != 0 && !ctx->failed) {
        ctx->failed = true;
        ctx->rc = nfo->info.done.rc;
    }

    return 0;
}

static
rc_t CC CheckExists(const KDirectory *dir, uint32_t type, const char *name, void *Ctx)
{
    struct col_check_ctx *ctx = Ctx;
    
    if ((type & ~kptAlias) == kptFile) {
        if (strcmp(name, "data") == 0)
            ctx->found |= 1;
        else if (strcmp(name, "idx0") == 0)
            ctx->found |= 2;
        else if (strcmp(name, "idx1") == 0)
            ctx->found |= 4;
        else if (strcmp(name, "idx2") == 0)
            ctx->found |= 8;
    }
    return 0;
}

static
rc_t KColumnCheckMD5(const KColumn *self,
                     CCReportInfoBlock *nfo,
                     CCReportFunc report, void *ctx)
{
    struct col_check_ctx local_ctx;
    rc_t rc;
    
    local_ctx.report = report;
    local_ctx.ctx = ctx;
    local_ctx.found = 0;
    local_ctx.failed = false;
    local_ctx.rc = 0;
    
    rc = DirectoryCheckMD5(self->dir, "md5", nfo, col_check_report, &local_ctx);
    if (rc == 0 && !local_ctx.failed && local_ctx.found != 0x0F) {
        local_ctx.found = 0;
        KDirectoryVisit(self->dir, false, CheckExists, &local_ctx, NULL);
        nfo->type = ccrpt_Done;
        if (local_ctx.found == 0x0F) {
            nfo->info.done.rc = RC(rcDB, rcColumn, rcValidating, rcChecksum, rcNotFound);
            nfo->info.done.mesg = "Some files are missing checksums";
        }
        else {
            nfo->info.done.rc = RC(rcDB, rcColumn, rcValidating, rcFile, rcNotFound);
            nfo->info.done.mesg = "Some files are missing";
        }
        rc = report(nfo, ctx);
    }
    else if (rc == 0) {
        nfo->type = ccrpt_Done;
        nfo->info.done.rc = 0;
        nfo->info.done.mesg = "md5 ok";
        rc = report(nfo, ctx);
    }
    else {
        nfo->type = ccrpt_Done;
        rc = report(nfo, ctx);
    }

    return rc;
}

static
rc_t KColumnCheckBlobs(const KColumn *self,
                       CCReportInfoBlock *nfo,
                       CCReportFunc report, void *ctx)
{
    int64_t start;
    uint64_t row;
    uint64_t rows;
    rc_t rc;
    
    rc = KColumnIdRange(self, &start, &rows);
    if (rc) {
        nfo->info.done.rc = rc;
        nfo->info.done.mesg = "could not be read";
        nfo->type = ccrpt_Done;
        return report(nfo, ctx);
    }
    for (row = 0; row < rows && rc == 0; ) {
        const KColumnBlob *blob;
        int64_t first;
        uint32_t count;
        
        rc = KColumnOpenBlobRead(self, &blob, row + start);
        if (rc) {
            if (GetRCObject(rc) == (enum RCObject)rcBlob && GetRCState(rc) == rcNotFound) {
                rc = 0;
                ++row; /* try with the next row; linear scan seems wrong */
                continue;
            }
            nfo->info.done.rc = rc;
            nfo->info.done.mesg = "could not be read";
            nfo->type = ccrpt_Done;
            return report(nfo, ctx);
        }
        rc = KColumnBlobIdRange(blob, &first, &count);
        if (rc) {
            KColumnBlobRelease(blob);
            nfo->info.done.rc = rc;
            nfo->info.done.mesg = "could not be read";
            nfo->type = ccrpt_Done;
            return report(nfo, ctx);
        }
        rc = KColumnBlobValidate(blob);
        KColumnBlobRelease(blob);
        if (rc) {
            nfo->info.done.rc = rc;
            nfo->info.done.mesg = "contains bad data";
            nfo->type = ccrpt_Done;
            return report(nfo, ctx);
        }
        nfo->type = ccrpt_Blob;
        nfo->info.blob.start = first;
        nfo->info.blob.count = count;
        rc = report(nfo, ctx);

        row += count;
    }
    nfo->info.done.rc = 0;
    nfo->info.done.mesg = "checksums ok";
    nfo->type = ccrpt_Done;
    rc = report(nfo, ctx);
    return rc;
}

LIB_EXPORT
rc_t CC KColumnConsistencyCheck(const KColumn *self,
    uint32_t level, CCReportInfoBlock *nfo, CCReportFunc report, void *ctx )
{
    rc_t rc = 0;

    bool indexOnly = level & CC_INDEX_ONLY;
    if (indexOnly) {
        level &= ~CC_INDEX_ONLY;
    }

    if (KDirectoryPathType(self->dir, "md5") != kptNotFound)
        rc = level == 0 ? KColumnCheckMD5(self, nfo, report, ctx) : 0;
    else {
        nfo->type = ccrpt_Done;
        nfo->info.done.mesg = "missing md5 file";
        nfo->info.done.rc = 0; /* RC(rcDB, rcColumn, rcValidating, rcFile, rcNotFound); */
        
        rc = report(nfo, ctx);
        
        if (level == 0)
            level = 1;
    }
    
    if (rc == 0 && level > 0)
        rc = KColumnCheckBlobs(self, nfo, report, ctx);
    return rc;
}
