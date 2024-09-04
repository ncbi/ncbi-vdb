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
#include "rindex.h"
#include "dbmgr.h"
#include "database-cmn.h"
#include "rtable.h"
#include "kdb-cmn.h"
#include "rcolumn.h"
#undef KONST

#include "cc.h"

#include <kdb/index.h>
#include <kdb/meta.h>
#include <kdb/kdb-priv.h>
#include <kdb/namelist.h>

#include <kfs/file.h>
#include <kfs/md5.h>

#include <klib/refcount.h>
#include <klib/log.h> /* PLOGMSG */
#include <klib/rc.h>
#include <klib/namelist.h>

#include <os-native.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#undef index

static
rc_t KTableCheckMD5 ( const KRTable *self, CCReportFunc report, void *ctx )
{
    CCReportInfoBlock info;
    memset ( & info, 0, sizeof info );

    info.objType = kptTable;
    rc_t rc = KTableGetName( & self->dad, &info.objName);
    if ( rc == 0 )
    {
        rc = DirectoryCheckMD5 ( self->dir, "md5", & info, report, ctx );
    }
    return rc;
}

#if 0
static bool KTableCheckColumnsFilter(const KDirectory *dir, const char *name, void *data)
{
    return (KDirectoryPathType(dir, "%s", name) & ~kptAlias) == kptDir;
}
#endif

typedef struct KTableCheckColumn_pb_s {
    KTable const *self;
    CCReportFunc report;
    void *rpt_ctx;
    unsigned n;
    int level;
    uint32_t depth;
} KTableCheckColumn_pb_t;

static rc_t CC KTableCheckColumn(const KDirectory *dir, uint32_t type, const char *name, void *data)
{
    KTableCheckColumn_pb_t *pb = (KTableCheckColumn_pb_t *)data;
    CCReportInfoBlock info;

    memset(&info, 0, sizeof(info));

    if ((type & ~kptAlias) != kptDir) {
        char mesg[4096];

        snprintf(mesg, sizeof(mesg), "unexpected object '%s'", name);
        rc_t rc = KTableGetName(pb->self, &info.objName);
        if ( rc == 0 )
        {
            info.objId = 0;
            info.objType = kptTable;
            info.type = ccrpt_Done;
            info.info.done.mesg = mesg;
            info.info.done.rc = 0;
            rc = pb->report(&info, pb->rpt_ctx);
        }
        return rc;
    }
    else {
        bool hasZombies;
        uint32_t ktype = KDBPathType(dir, &hasZombies, name);
        rc_t rc;

        info.objType = kptColumn;
        info.objId = pb->n++;
        info.objName = name;
        info.type = ccrpt_Visit;
        info.info.visit.depth = pb->depth + 1;
        rc = pb->report(&info, pb->rpt_ctx);
        if (rc) return rc;

        info.type = ccrpt_Done;

        if (hasZombies) {
            info.info.done.rc = 0;
            info.info.done.mesg = "Column may be truncated";
            rc = pb->report(&info, pb->rpt_ctx);
            if (rc) return rc;
        }
        info.info.done.rc = SILENT_RC(rcDB, rcTable, rcValidating, rcType, rcIncorrect);
        if ((ktype & ~kptAlias) == kptColumn) {
            const KColumn *col;

            info.info.done.rc = KTableOpenColumnRead(pb->self, &col, "%s", name);
            if (info.info.done.rc == 0) {
                info.info.done.rc = KColumnConsistencyCheck(col, pb->level, &info, pb->report, pb->rpt_ctx);
                KColumnRelease(col);
                return info.info.done.rc;
            }
        }
#if 0
        if (platform != SRA_PLATFORM_UNDEFINED && platform != SRA_PLATFORM_454
            && name != NULL && name[0] != '\0' && strcmp(name, "SIGNAL") == 0)
        {
            (void)PLOGMSG(klogWarn, (klogWarn, "COLUMN '$(name)' IS INCOMPLETE",
                "name=%s", name));
            return 0;
        }
#endif
        info.info.done.mesg = "Failed to open column";
        return pb->report(&info, pb->rpt_ctx);
    }
}

static
rc_t KTableCheckColumns ( const KRTable *self, uint32_t depth, int level,
    CCReportFunc report, void *ctx )
{
    KTableCheckColumn_pb_t pb;

    pb.self = & self -> dad;
    pb.report = report;
    pb.rpt_ctx = ctx;
    pb.n = 0;
    pb.level = level;
    pb.depth = depth;
    return KDirectoryVisit_v1(self->dir, false, KTableCheckColumn, &pb, "col");
}

static
rc_t KTableCheckIndexMD5(const KDirectory *dir,
                         CCReportInfoBlock *nfo,
                         CCReportFunc report, void *ctx)
{
    char md5[4096];

    snprintf(md5, sizeof(md5), "%s.md5", nfo->objName);
    if (KDirectoryPathType(dir, "%s", md5) != kptNotFound)
        return DirectoryCheckMD5(dir, md5, nfo, report, ctx);
    return 0;
}

static const KDirectory *KTableFindIndexDir(const KRTable *self)
{
    const KDirectory *idxDir;

    rc_t rc = KDirectoryOpenDirRead ( self -> dir, & idxDir, false, "idx" );
    if ( rc == 0 )
        return idxDir;

    /* some old tables have a single index at table level */
    KDirectoryAddRef ( self -> dir );
    return self -> dir;
}

static
rc_t KTableCheckIndices(const KRTable *self, uint32_t depth, int level, CCReportFunc report, void *ctx)
{
    uint32_t n;
    const KMetadata *meta;
    int64_t max_row_id = 0;

    KNamelist *list;
    rc_t rc = KTableListIdx( & self->dad, &list);
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) == rcNotFound )
            return 0;
        return rc;
    }

    rc = KTableOpenMetadataRead ( & self->dad, & meta );
    if ( rc == 0 )
    {
        const KMDataNode *seq;
        rc = KMetadataOpenNodeRead ( meta, & seq, "/.seq/spot" );
        if ( rc == 0 )
        {
            rc = KMDataNodeReadAsI64 ( seq, & max_row_id );
            if ( rc != 0 )
                max_row_id = 0;
            KMDataNodeRelease ( seq );
        }

        KMetadataRelease ( meta );
    }

    rc = KNamelistCount(list, &n);
    if ( rc == 0 )
    {
        const KDirectory *idxDir = KTableFindIndexDir (self);
        if ( idxDir == NULL )
            rc = RC ( rcDB, rcTable, rcValidating, rcDirectory, rcNull );
        else
        {
            CCReportInfoBlock nfo;
            memset(&nfo, 0, sizeof(nfo));
            nfo.objType = kptIndex;

            for ( nfo . objId = 0; rc == 0 && nfo . objId != (int) n; ++ nfo . objId)
            {
                rc = KNamelistGet(list, nfo.objId, &nfo.objName);
                if ( rc != 0 )
                    break;

                nfo.type = ccrpt_Visit;
                nfo.info.visit.depth = depth + 1;
                rc = report(&nfo, ctx); if (rc) break;

                rc = KTableCheckIndexMD5(idxDir, &nfo, report, ctx);
                if (rc == 0 && level > 0)
                {
                    const KIndex *idx;
                    rc = KTableOpenIndexRead(& self->dad, &idx, "%s", nfo.objName);

                    if ( rc != 0 )
                    {
                        nfo.type = ccrpt_Done;
                        nfo.info.done.rc = rc;
                        nfo.info.done.mesg = "could not be opened";
                    }
                    else
                    {
                        KIndexSetMaxRowId ( idx, max_row_id );

                        nfo.type = ccrpt_Index;
                        rc = KIndexConsistencyCheck(idx, level < 3 ? 1 : 3,
                                                    &nfo.info.index.start_id,
                                                    &nfo.info.index.id_range,
                                                    &nfo.info.index.num_keys,
                                                    &nfo.info.index.num_rows,
                                                    &nfo.info.index.num_holes);
                        KIndexRelease ( idx );
                        if ( rc != 0 )
                        {
                            nfo.type = ccrpt_Done;
                            nfo.info.done.rc = rc;
                            nfo.info.done.mesg = "could not be validated";
                        }
                    }

                    rc = report(&nfo, ctx);
                }
            }

            KDirectoryRelease(idxDir);
        }
    }

    KNamelistRelease(list);

    return rc;
}

LIB_EXPORT
rc_t CC KTableConsistencyCheck(const KTable *bself, uint32_t depth, uint32_t level,
    CCReportFunc report, void *ctx)
{
    const KRTable *self = (const KRTable *)bself;
    rc_t rc = 0;
    uint32_t type;

    bool indexOnly = (bool)( level & CC_INDEX_ONLY );
    if (indexOnly) {
        level &= ~CC_INDEX_ONLY;
    }

    if (self == NULL)
        return RC(rcDB, rcTable, rcValidating, rcSelf, rcNull);

    if (depth == 0) {
        CCReportInfoBlock info;

        rc = KTableGetName(bself, &info.objName);
        if ( rc == 0 )
        {
            info.objId = 0;
            info.objType = kptTable;
            info.type = ccrpt_Visit;
            info.info.visit.depth = 0;

            rc = report(&info, ctx);
        }
        if (rc) return rc;
    }

    type = KDirectoryPathType(self->dir, "md5");
    if (type == kptZombieFile) {
        CCReportInfoBlock info;

        rc = KTableGetName(bself, &info.objName);
        if ( rc == 0 )
        {
            info.objId = 0;
            info.objType = kptTable;
            info.type = ccrpt_Done;
            info.info.done.mesg = "Table may be truncated";
            info.info.done.rc = 0;

            rc = report(&info, ctx);
        }
    }
    else if (type != kptNotFound) {
        if (!indexOnly) {
            rc = KTableCheckMD5(self, report, ctx);
        }
    }
    else {
        CCReportInfoBlock info;

        rc = KTableGetName(bself, &info.objName);
        if ( rc == 0 )
        {
            info.objId = 0;
            info.objType = kptTable;
            info.type = ccrpt_Done;
            info.info.done.mesg = "missing md5 file";
            info.info.done.rc = 0;

            rc = report(&info, ctx);
        }
    }

    if ( rc == 0 && ! indexOnly )
        rc = KTableCheckColumns(self, depth, level, report, ctx);

    if ( rc == 0 )
        rc = KTableCheckIndices(self, depth, level, report, ctx);

    return rc;
}
