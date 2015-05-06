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
#include "index-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "table-priv.h"
#include "kdb-priv.h"
#include "column-priv.h"
#undef KONST

#include "cc-priv.h"

#include <kdb/index.h>

#include <kfs/file.h>
#include <kfs/md5.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/namelist.h>
#include <kdb/namelist.h>
#include <os-native.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#undef index

static void KDatabaseGetName(KDatabase const *self, char const **rslt)
{
    char *sep;
    
    *rslt = self->path;
    sep = strrchr(self->path, '/');
    if (sep != NULL)
        *rslt = sep + 1;
}

static
rc_t KDatabaseCheckMD5 (const KDatabase *self, CCReportFunc report, void *ctx)
{
    CCReportInfoBlock info;
    memset (& info, 0, sizeof info);

    info.objType = kptDatabase;
    KDatabaseGetName(self, &info.objName);
    
    return DirectoryCheckMD5 (self -> dir, "md5", & info, report, ctx);
}

static
rc_t KDatabaseCheckTables (const KDatabase *self, uint32_t depth, int level, CCReportFunc report, void *ctx)
{
    uint32_t n;
    
    KNamelist *list;
    rc_t rc = KDatabaseListTbl (self, & list);
    if (rc != 0)
        return rc;
    
    rc = KNamelistCount (list, & n);
    if (rc == 0)
    {
        CCReportInfoBlock nfo;
        memset (& nfo, 0, sizeof nfo);
        nfo.objType = kptTable;
        
        for (nfo.objId = 0; rc == 0 && nfo.objId != (int) n; ++ nfo.objId)
        {
            rc = KNamelistGet (list, nfo.objId, &nfo.objName);
            if (rc == 0)
            {
                const KTable *tbl;
                
                nfo.type = ccrpt_Visit;
                nfo.info.visit.depth = depth + 1;
                rc = report(&nfo, ctx); if (rc) break;
                
                rc = KDatabaseOpenTableRead (self, & tbl, "%s", nfo.objName);
                if (rc == 0)
                {
                    rc = KTableConsistencyCheck (tbl, depth + 1, level, report,
                        ctx, SRA_PLATFORM_UNDEFINED);
                    KTableRelease (tbl);
                }
            }
        }
    }
    
    KNamelistRelease (list);
    return rc;
}

static
rc_t KDatabaseCheckIndexMD5 (const KDirectory *dir,
    CCReportInfoBlock *nfo, CCReportFunc report, void *ctx)
{
    char md5 [ 4100 ];

    snprintf (md5, sizeof md5, "%s.md5", nfo -> objName);
    return DirectoryCheckMD5 (dir, md5, nfo, report, ctx);
}

static
const KDirectory *KDatabaseFindIndexDir (const KDatabase *self)
{
    const KDirectory *idxDir;
    
    rc_t rc = KDirectoryOpenDirRead (self -> dir, & idxDir, false, "idx");
    if (rc == 0)
        return idxDir;

    return NULL;
}

static
rc_t KDatabaseCheckIndices (const KDatabase *self, uint32_t depth, uint32_t level, CCReportFunc report, void *ctx)
{
    uint32_t n;

    KNamelist *list;
    rc_t rc = KDatabaseListIdx (self, & list);
    if (rc != 0)
    {
        if (GetRCState (rc) == rcNotFound)
            return 0;
        return rc;
    }

/* TEST    assert(0); */
    rc = KNamelistCount (list, & n);
    if (rc == 0)
    {
        const KDirectory *idxDir = KDatabaseFindIndexDir (self);
        if (idxDir == NULL)
            rc = RC (rcDB, rcDatabase, rcValidating, rcDirectory, rcNull);
        else
        {
            CCReportInfoBlock nfo;
            memset (& nfo, 0, sizeof nfo);
            nfo.objType = kptIndex;

            for (nfo.objId = 0; rc == 0 && nfo.objId != (int) n; ++ nfo.objId)
            {
                rc = KNamelistGet(list, nfo.objId, &nfo.objName);
                if (rc != 0)
                    break;
            
                nfo.type = ccrpt_Visit;
                nfo.info.visit.depth = depth + 1;
                rc = report(&nfo, ctx); if (rc) break;
                
                rc = KDatabaseCheckIndexMD5 (idxDir, &nfo, report, ctx);
                if (rc == 0 && level > 0)
                {
                    const KIndex *idx;
                    rc = KDatabaseOpenIndexRead (self, &idx, "%s", nfo.objName);
                    if (rc != 0)
                    {
                        nfo.type = ccrpt_Done;
                        nfo.info.done.rc = rc;
                        nfo.info.done.mesg = "could not be opened";
                    }
                    else
                    {
                        nfo.type = ccrpt_Index;
                        rc = KIndexConsistencyCheck(idx, level < 3 ? 1 : 3,
                                                    &nfo.info.index.start_id,
                                                    &nfo.info.index.id_range,
                                                    &nfo.info.index.num_keys,
                                                    &nfo.info.index.num_rows,
                                                    &nfo.info.index.num_holes);
                        KIndexRelease(idx);
                        if (rc != 0)
                        {
                            nfo.type = ccrpt_Done;
                            nfo.info.done.rc = rc;
                            nfo.info.done.mesg = "could not be validated";
                        }
                    }

                    rc = report (& nfo, ctx);
                }
            }

            KDirectoryRelease (idxDir);
        }
    }

    KNamelistRelease (list);
    return rc;
}

static
rc_t KDatabaseCheckDatabases (const KDatabase *self, uint32_t depth, int level, CCReportFunc report, void *ctx)
{
    uint32_t n;

    KNamelist *list;
    rc_t rc = KDatabaseListDB (self, & list);
    if (rc != 0)
    {
        if (GetRCState (rc) == rcNotFound)
            return 0;
        return rc;
    }

    rc = KNamelistCount (list, & n);
    if (rc == 0)
    {
        CCReportInfoBlock nfo;
        memset (& nfo, 0, sizeof nfo);
        nfo.objType = kptDatabase;
        
        for (nfo.objId = 0; rc == 0 && nfo.objId != (int) n; ++ nfo.objId)
        {
            rc = KNamelistGet (list, nfo.objId, &nfo.objName);
            if (rc == 0)
            {
                const KDatabase *db;
                
                nfo.type = ccrpt_Visit;
                nfo.info.visit.depth = depth + 1;
                rc = report(&nfo, ctx); if (rc) break;
                
                rc = KDatabaseOpenDBRead (self, & db, "%s", nfo.objName);
                if (rc == 0)
                {
                    rc = KDatabaseConsistencyCheck (db, depth + 1, level, report, ctx);
                    KDatabaseRelease (db);
                }
            }
        }
    }
    
    KNamelistRelease (list);
    return rc;
}

LIB_EXPORT
rc_t CC KDatabaseConsistencyCheck (const KDatabase *self,
    uint32_t depth, uint32_t level, CCReportFunc report, void *ctx)
{
    rc_t rc = 0;
    uint32_t type;

    uint32_t aLevel = level;
    bool indexOnly = level & CC_INDEX_ONLY;
    if (indexOnly) {
        level &= ~CC_INDEX_ONLY;
    }

    if (self == NULL)
        return RC (rcDB, rcDatabase, rcValidating, rcSelf, rcNull);
    
    if (depth == 0) {
        CCReportInfoBlock info;
        
        KDatabaseGetName(self, &info.objName);
        info.objId = 0;
        info.objType = kptDatabase;
        info.type = ccrpt_Visit;
        info.info.visit.depth = 0;
        
        rc = report(&info, ctx);
        if (rc) return rc;
    }
    
    type = KDirectoryPathType(self->dir, "md5");
    if (type == kptZombieFile) {
        CCReportInfoBlock info;
        
        KDatabaseGetName(self, &info.objName);
        info.objId = 0;
        info.objType = kptTable;
        info.type = ccrpt_Done;
        info.info.done.mesg = "Database may be truncated";
        info.info.done.rc = 0;
        
        rc = report(&info, ctx);
    }
    else if (type != kptNotFound)
        rc = KDatabaseCheckMD5 (self, report, ctx);
    else {
        CCReportInfoBlock info;
        
        KDatabaseGetName(self, &info.objName);
        info.objId = 0;
        info.objType = kptDatabase;
        info.type = ccrpt_Done;
        info.info.done.mesg = "no md5 file";
        info.info.done.rc = 0;
        
        rc = report(&info, ctx);
    }
    

    if (rc == 0)
        rc = KDatabaseCheckTables (self, depth, aLevel, report, ctx);

    if (rc == 0)
        rc = KDatabaseCheckIndices (self, depth, level, report, ctx);

    if (rc == 0)
        rc = KDatabaseCheckDatabases (self, depth, level, report, ctx);

    return rc;
}
