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
#include "blast-mgr.h" /* BTableType */
#include <kdb/manager.h> /* KDBManagerRelease */
#include <kdb/meta.h> /* KMetadataRelease */
#include <kfg/config.h> /* KConfig */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/log.h> /* KLogLevel */
#include <klib/out.h>  /* KOutHandlerSetStdOut */
#include <klib/refcount.h> /* KRefcount */
#include <klib/status.h> /* STSMSG */
#include <sra/sraschema.h> /* VDBManagerMakeSRASchema */
#include <vdb/database.h> /* VDBManagerOpenDBRead */
#include <vdb/manager.h> /* VDBManager */
#include <vdb/schema.h> /* VSchema */
#include <vdb/table.h> /* VDBManagerOpenTableRead */
#include <vdb/vdb-priv.h> /* VDBManagerOpenKDBManagerRead */

#include <vfs/manager.h> /* VFSManager */
#include <vfs/path.h> /* VPath */
#include <vfs/resolver.h> /* VResolver */

#include <sysalloc.h>
#include <stdio.h> /* fprintf */
#include <string.h> /* memset */

#define TOOLKIT "sratoolkit2_10_5"

/******************************************************************************/

static const char VDB_BLAST_MGR[] = "VdbBlastMgr";

struct VdbBlastMgr {
    KRefcount refcount;
    VSchema *schema;
    const VDBManager *mgr;
    const KDBManager *kmgr;
    VFSManager * vfs;
    KDirectory *dir;
    VResolver *resolver;
};

rc_t _VdbBlastMgrNativeToPosix(const VdbBlastMgr *self,
    const char *native, char *posix, size_t size)
{
    rc_t rc = 0;
    VPath *temp_v_path = NULL;
    assert(self);
    rc = VFSManagerMakeSysPath(self->vfs, &temp_v_path, native);
    if (rc == 0) {
        size_t written = 0;
        rc = VPathReadPath(temp_v_path, posix, size, &written);
    }
    RELEASE(VPath, temp_v_path);
    return rc;
}

static
void _VdbBlastMgrWhack(VdbBlastMgr *self)
{
    assert(self);

    VSchemaRelease(self->schema);
    VDBManagerRelease(self->mgr);
    KDBManagerRelease(self->kmgr);
    VFSManagerRelease ( self -> vfs );
    KDirectoryRelease(self->dir);
    VResolverRelease(self->resolver);

    memset(self, 0, sizeof *self);

    free(self);

    STSMSG(1, ("Deleted VdbBlastMgr"));
}

LIB_EXPORT
VdbBlastMgr* CC VdbBlastInit(uint32_t *status)
{
    VdbBlastMgr *item = NULL;
    rc_t rc = 0;

    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastErr;

    if (getenv(TOOLKIT) != NULL) {
#if _DEBUGGING
fprintf(stderr, "DEBUGNG with " TOOLKIT " release\n");
#else
fprintf(stderr, "RELEASE with " TOOLKIT " release\n");
#endif
    }
    item = calloc(1, sizeof *item);
    if (item == NULL) {
        *status = eVdbBlastMemErr;
        return item;
    }

    if ( rc == 0 )
    {
        rc = VFSManagerMake ( & item -> vfs );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "Error in VFSManagerMake" );
    }

    if (rc == 0) {
        rc = VFSManagerGetCWD( item -> vfs, &item->dir);
        if (rc != 0)
        {   LOGERR(klogInt, rc, "Error in VFSManagerGetCWD"); }
    }

    if (rc == 0) {
        rc = VDBManagerMakeRead(&item->mgr, NULL);
        if (rc != 0)
        {   LOGERR(klogInt, rc, "Error in VDBManagerMakeRead"); }
    }

    if (rc == 0) {
        rc = VDBManagerOpenKDBManagerRead(item->mgr, &item->kmgr);
        if (rc != 0)
        {   LOGERR(klogInt, rc, "Error in VDBManagerOpenKDBManagerRead"); }
    }

    if (rc == 0) {
        rc = VDBManagerMakeSRASchema(item->mgr, &item->schema);
        if (rc != 0)
        {   LOGERR(klogInt, rc, "Error in VDBManagerMakeSRASchema"); }
    }

    if (rc == 0) {
        VFSManager* mgr = NULL;
        KConfig* cfg = NULL;
        if (rc == 0) {
            rc = VFSManagerMake(&mgr);
        }
        if (rc == 0) {
            rc = KConfigMake(&cfg, NULL);
        }
        if (rc == 0) {
            rc = VFSManagerMakeResolver(mgr, &item->resolver, cfg);
        }
        RELEASE(KConfig, cfg);
        RELEASE(VFSManager, mgr);
    }

    if (rc != 0) {
        _VdbBlastMgrWhack(item);
        item = NULL;
        STSMSG(1, ("Error: failed to create VdbBlastMgr"));
    }
    else {
        KRefcountInit(&item->refcount, 1, VDB_BLAST_MGR, __func__, "mgr");
        *status = eVdbBlastNoErr;
        STSMSG(1, ("Created VdbBlastMgr"));
    }

    return item;
}

LIB_EXPORT
VdbBlastMgr* CC VdbBlastMgrAddRef(VdbBlastMgr *self)
{
    if (self == NULL) {
        STSMSG(2, ("VdbBlastMgrAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_MGR) == krefOkay) {
        STSMSG(2, ("VdbBlastMgrAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlastMgrAddRef"));
    return NULL;
}

LIB_EXPORT
void CC VdbBlastMgrRelease(VdbBlastMgr *self)
{
    if (self == NULL)
    {   return; }

    STSMSG(2, ("VdbBlastMgrRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_MGR) != krefWhack)
    {   return; }

    _VdbBlastMgrWhack(self);
}


LIB_EXPORT bool CC VdbBlastMgrIsCSraRun(
    const VdbBlastMgr *self, const char *rundesc)
{
    bool csra = false;
    rc_t rc = 0;
    const VDBManager *aMgr = NULL;
    const VDBManager *mgr = NULL;
    const VDatabase *db = NULL;
    if (self != NULL) {
        mgr = self->mgr;
    }
    if (mgr == NULL) {
        rc = VDBManagerMakeRead(&mgr, NULL);
        if (rc != 0) {
            return false;
        }
        if (self != NULL && self->mgr == NULL) {
            ((VdbBlastMgr*)self)->mgr = mgr;
        }
        else {
            aMgr = mgr;
        }
    }
    rc = VDBManagerOpenDBRead(mgr, &db,
        self == NULL ? NULL : self->schema, "%s", rundesc);
    if (rc == 0) {
        csra = VDatabaseIsCSRA(db);
    }
    RELEASE(VDatabase, db);
    RELEASE(VDBManager, aMgr);
    return csra;
}


static bool _VdbBlastMgrSchemaEquals(const VdbBlastMgr *self,
    const VDatabase *db, const char *rundesc,
    const char* name, uint32_t max_chars)
{
    bool equals = false;

    const KMetadata *meta = NULL;
    const KMDataNode *node = NULL;

    char buffer[512] = "";
    size_t size = 0;

    rc_t rc = 0;

    assert(self);

    if (db != NULL) {
        rc = VDatabaseOpenMetadataRead(db, &meta);
    }
    else {
        const VTable *tbl = NULL;
        rc = VDBManagerOpenTableRead(self->mgr, &tbl, self->schema, rundesc);

        if (rc == 0) {
            rc = VTableOpenMetadataRead(tbl, &meta);
        }

        RELEASE(VTable, tbl);
    }

    if (rc == 0) {
        rc = KMetadataOpenNodeRead(meta, &node, "schema");
    }

    if (rc == 0) {
        rc = KMDataNodeReadAttr(node, "name", buffer, sizeof buffer, &size);
    }

    if (rc == 0) {
        STSMSG(1, ("%s.schema@name='%.*s'", rundesc, (uint32_t)size, buffer));

        if (size >= sizeof name && 
            string_cmp(buffer, max_chars, name, max_chars, max_chars)
                == 0)
        {
            equals = true;
        }
    }

    RELEASE(KMDataNode, node);
    RELEASE(KMetadata, meta);

    return equals;
}

BTableType _VdbBlastMgrBTableType(
    const VdbBlastMgr *self, const char *rundesc)
{
    BTableType type = btpSRA;

    rc_t rc = 0;

    const VDatabase *db = NULL;

    assert(self);

    rc = VDBManagerOpenDBRead(self->mgr, &db, NULL, rundesc);

    if (rc == 0) {
        const char name[] = "NCBI:WGS:db:contig";

        if (_VdbBlastMgrSchemaEquals(self, db, rundesc, name, sizeof name - 1))
        {
            STSMSG(1, ("%s is WGS", rundesc));
            type = btpWGS;
        }

        RELEASE(VDatabase, db);
    }
    else {
        const char name[] = "NCBI:refseq:tbl:reference";

        if (_VdbBlastMgrSchemaEquals(self, db, rundesc, name, sizeof name - 1))
        {
            STSMSG(1, ("%s is REFSEQ", rundesc));
            type = btpREFSEQ;
        }
    }

    if (type == btpSRA) {
        STSMSG(1, ("%s is SRA", rundesc));
    }

    return type;
}

static
VdbBlastStatus _VdbBlastMgrOpenSequenceTable(const VdbBlastMgr *self,
    const char *path,
    const VTable **tbl,
    const VDatabase **db)
{
    KPathType type = kptNotFound;
    VSchema *schema = NULL;

    assert(self && tbl && db);
    *tbl = NULL;

    /* Always use VDBManagerMakeSRASchema to VDBManagerOpenTableRead
       Otherwise CMP_BASE_COUNT column sometimes cannot be found */
    schema = self->schema;

    type = KDBManagerPathType(self->kmgr, "%s", path);
    if (type == kptNotFound) {
        STSMSG(1, ("Error: cannot find '%s'", path));
        return eVdbBlastErr;
    }

    if ((type & ~kptAlias) == kptDatabase) {
        const char *table = "SEQUENCE";
        rc_t rc = VDBManagerOpenDBRead(self->mgr, db, NULL, "%s", path);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VDBManagerOpenDBRead($(name))", "name=%s", path));
            STSMSG(1, ("Error: failed to open DB '%s'", path));
            return eVdbBlastErr;
        }
        assert(*db);
        rc = VDatabaseOpenTableRead(*db, tbl, table);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VDatabaseOpenTableRead($(name), $(tbl))",
                "name=%s,tbl=%s", path, table));
            STSMSG(1, ("Error: failed to open DB table '%s/%s'", path, table));
        }
        else {
            STSMSG(1, ("Opened DB table '%s/%s'", path, table));
        }

        return rc != 0 ? eVdbBlastErr : eVdbBlastNoErr;
    }

    while (true) {
        rc_t rc = VDBManagerOpenTableRead(self->mgr, tbl, schema, "%s", path);
        if (rc == 0) {
            if (schema == NULL) {
                STSMSG(1, ("Opened table '%s'(schema=NULL)", path));
            }
            else {
                STSMSG(1, ("Opened table '%s'(SRASchema)", path));
            }
            return eVdbBlastNoErr;
        }

        assert(self->schema);

        if (schema == NULL)
        {   schema = self->schema; }
        else {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VDBManagerOpenTableRead($(name))", "name=%s", path));
            STSMSG(1, ("Error: failed to open table '%s'", path));
            return eVdbBlastRunErr;
        }
    }
}

VdbBlastStatus _VdbBlastMgrFindNOpenSeqTable(const VdbBlastMgr *self,
    const char *rundesc,
    const VTable **tbl,
    BTableType *type,
    char **fullpath,
    const VDatabase **db)
{
    VdbBlastStatus status = eVdbBlastNoErr;
    char *path = (char*) rundesc;

    assert(self && type);

    status = _VdbBlastMgrOpenSequenceTable(self, path, tbl, db);
    if (status == eVdbBlastNoErr) {
        STSMSG(1, ("Added run %s(%s)", rundesc, path));
    }
    else {
        STSMSG(1, ("Error: failed to add run %s(%s)", rundesc, path));
    }

    *type = _VdbBlastMgrBTableType(self, path);

    if (path != rundesc) {
        free(path);
        path = NULL;
    }

    return status;
}

/******************************************************************************/

/* KConfigPrint
 * print current configuration to the output handler
 */
LIB_EXPORT uint32_t VdbBlastMgrKConfigPrint(const VdbBlastMgr *self) {
    KConfig *kfg = NULL;

    rc_t rc = KConfigMake(&kfg, NULL);

    if (rc == 0) {
        rc = KConfigPrint(kfg, 0);
    }

    {
        rc_t rc2 = KConfigRelease(kfg);
        if (rc == 0 && rc2 != 0) {
            rc = rc2;
        }
    }

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when printing Configiration");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKDbgSetString(const VdbBlastMgr *self,
    const char *string)
{
    rc_t rc = KDbgSetString(string);

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when setting KDbgString");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogHandlerSet(const VdbBlastMgr *self,
    KWrtWriter writer, void *data)
{
    rc_t rc = KLogHandlerSet(writer, data);

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when setting KLogHandler");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogHandlerSetStdErr(
    const VdbBlastMgr *self)
{
    rc_t rc = KLogHandlerSetStdErr();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KLogHandler to StdErr");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogHandlerSetStdOut(
    const VdbBlastMgr *self)
{
    rc_t rc = KLogHandlerSetStdOut();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KLogHandler to StdOut");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogLibHandlerSet(const VdbBlastMgr *self,
    KWrtWriter writer, void *data)
{
    rc_t rc = KLogLibHandlerSet(writer, data);

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when setting KLogLibHandler");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKLogLibHandlerSetStdErr(const VdbBlastMgr *self)
{
    rc_t rc = KLogLibHandlerSetStdErr();

    if (rc != 0) {
        LOGERR(klogInt, rc,
            "An error occured when setting KLogLibHandler to StdErr");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKLogLibHandlerSetStdOut(const VdbBlastMgr *self)
{
    rc_t rc = KLogLibHandlerSetStdOut();

    if (rc != 0) {
        LOGERR(klogInt, rc,
            "An error occured when setting KLogLibHandler to StdOut");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKOutHandlerSet(const VdbBlastMgr *self,
    VdbBlastKWrtWriter writer, void *data)
{
    rc_t rc = KOutHandlerSet(writer, data);

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when setting KOutHandler");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKOutHandlerSetStdErr(const VdbBlastMgr *self)
{
    rc_t rc = KOutHandlerSetStdErr();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KOutHandler to StdErr");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKOutHandlerSetStdOut(const VdbBlastMgr *self)
{
    rc_t rc = KOutHandlerSetStdOut();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KOutHandler to StdOut");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKStsHandlerSetStdErr(const VdbBlastMgr *self)
{
    rc_t rc = KStsHandlerSetStdErr();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KStsHandler to StdErr");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT
uint32_t CC VdbBlastMgrKStsHandlerSetStdOut(const VdbBlastMgr *self)
{
    rc_t rc = KStsHandlerSetStdOut();

    if (rc != 0) {
        LOGERR(
            klogInt, rc, "An error occured when setting KStsHandler to StdOut");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogLevelSet(const VdbBlastMgr *self,
    KLogLevel lvl)
{
    rc_t rc = KLogLevelSet(lvl);

    if (rc != 0) {
        LOGERR(klogInt, rc, "An error occured when setting KLogLevel");
        return eVdbBlastErr;
    }

    return eVdbBlastNoErr;
}

LIB_EXPORT uint32_t CC VdbBlastMgrKLogLevelSetInfo(
    const VdbBlastMgr *self)
{   return VdbBlastMgrKLogLevelSet(self, klogInfo); }

LIB_EXPORT uint32_t CC VdbBlastMgrKLogLevelSetWarn(
    const VdbBlastMgr *self)
{   return VdbBlastMgrKLogLevelSet(self, klogWarn); }

LIB_EXPORT VdbBlastStatus CC VdbBlastMgrKStsLevelSet(const VdbBlastMgr *self,
    uint32_t level)
{
    KStsLevelSet(level);
    return eVdbBlastNoErr; 
}
