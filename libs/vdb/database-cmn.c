/* TODO: move them to interfaces/klib/strings.h */
#define MAGIC_CACHE_VDBCACHE  "VDB_CACHE_VDBCACHE"
#define MAGIC_LOCAL_VDBCACHE  "VDB_LOCAL_VDBCACHE"
#define MAGIC_REMOTE_VDBCACHE "VDB_REMOTE_VDBCACHE"

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

#define KONST const
#include "database-priv.h"
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "linker-priv.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <kdb/kdb-priv.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <klib/debug.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/namelist.h>
#include <klib/log.h>
#include <sysalloc.h>

#include "../vfs/path-priv.h"     /* VPath */
#include "../vfs/resolver-priv.h" /* rcResolver */

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */


/* Whack
 */
rc_t CC VDatabaseWhack ( VDatabase *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "VDatabase" );

    /* release user data */
    if ( self -> user != NULL && self -> user_whack != NULL )
    {
        ( * self -> user_whack ) ( self -> user );
        self -> user = NULL;
        self -> user_whack = NULL;
    }

    /* release dad */
    if ( self -> dad != NULL )
    {
        rc = VDatabaseSever ( self -> dad );
        if ( rc == 0 )
            self -> dad = NULL;
    }
    /* release cache_db */
    if ( self -> cache_db != NULL)
    {
	rc = VDatabaseRelease ( self -> cache_db );
	if ( rc == 0 )
		self -> cache_db = NULL;
    }

    /* remove from mgr */
    if ( rc == 0 )
        rc = VDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        /* complete */
        KMetadataRelease ( self -> meta );
        KDatabaseRelease ( self -> kdb );
        VSchemaRelease ( self -> schema );

        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "VDatabase", "whack", "vdb" );
    return rc;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC VDatabaseAddRef ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VDatabase" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcDatabase, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VDatabaseRelease ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VDatabase" ) )
        {
        case krefWhack:
            return VDatabaseWhack ( ( VDatabase* ) self );
        case krefNegative:
            return RC ( rcVDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 *  internal reference management
 */
VDatabase *VDatabaseAttach ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VDatabase" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VDatabase* ) self;
}

rc_t VDatabaseSever ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VDatabase" ) )
        {
        case krefWhack:
            return VDatabaseWhack ( ( VDatabase* ) self );
        case krefNegative:
            return RC ( rcVDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make - PRIVATE
 *  creates an initialized database object
 *  expects a mgr and optionally a parent db,
 *  enforces schema validity according to some rules,
 */
rc_t VDatabaseMake ( VDatabase **dbp,
    const VDBManager *mgr, const VDatabase *dad, const VSchema *schema )
{
    rc_t rc;

    /* create a structure */
    VDatabase *db = calloc ( 1, sizeof * db );
    if ( db == NULL )
        rc = RC ( rcVDB, rcDatabase, rcConstructing, rcMemory, rcExhausted );
    else
    {
        /* create a modifiable schema */
        rc = VSchemaMake ( ( VSchema** ) & db -> schema, schema );
        if ( rc == 0 )
        {
            db -> mgr = VDBManagerAttach ( mgr );
            db -> dad = VDatabaseAttach ( dad );
            KRefcountInit ( & db -> refcount, 1, "VDatabase", "make", "vdb" );

            * dbp = db;
            return 0;
        }

        VDatabaseWhack ( db );
    }

    * dbp = NULL;

    return rc;
}


/* OpenRead
 *  finish opening a db for read
 */
static
rc_t CC VDatabaseOpenRead ( VDatabase *self )
{
    /* open metadata */
    rc_t rc = KDatabaseOpenMetadataRead ( self -> kdb, & self -> meta );
    if ( rc == 0 )
    {
        /* fetch stored schema */
        rc = VDatabaseLoadSchema ( self );
        if ( rc == 0 && self -> sdb == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSchema, rcNotFound );
    }

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB), ("VDatabaseOpenRead = %d\n", rc));

    return rc;
}


/* OpenDBRead
 * VOpenDBRead
 *  open a database for read
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN, NULL OKAY ] - schema object containg database
 *  declaration to be used in creating db.
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t VDBManagerVPathOpenLocalDBRead ( const VDBManager *self,
    const VDatabase **dbp, const VSchema *schema, const VPath * path )
{
    rc_t rc;
    VDatabase * db;

    if ( schema == NULL )
        schema = self -> schema;

    rc = VDatabaseMake ( & db, self, NULL, schema );
    if ( rc == 0 )
    {
        db -> read_only = true;

        rc = KDBManagerVPathOpenLocalDBRead ( self -> kmgr, & db -> kdb, path );
        if ( rc == 0 )
        {
            rc = VDatabaseOpenRead ( db );
            if ( rc == 0 )
            {
                * dbp = db;
                return 0;
            }
        }
        
        VDatabaseWhack ( db );
    }

    return rc;
}

static
rc_t VDBManagerVPathOpenRemoteDBRead ( const VDBManager *self,
    const VDatabase **dbp, const VSchema *schema, const VPath * remote, const VPath * cache )
{
    rc_t rc;
    VDatabase * db;

    if ( schema == NULL )
        schema = self -> schema;

    rc = VDatabaseMake ( & db, self, NULL, schema );
    if ( rc == 0 )
    {
        db -> read_only = true;

        rc = KDBManagerVPathOpenRemoteDBRead ( self -> kmgr, & db -> kdb, remote, cache );
        if ( rc == 0 )
        {
            rc = VDatabaseOpenRead ( db );
            if ( rc == 0 )
            {
                * dbp = db;
                return 0;
            }
        }
        
        VDatabaseWhack ( db );
    }

    return rc;
}

#include "../kdb/dbmgr-priv.h"
static void ad(const KDBManager *self, const VPath *aPath, const VPath **path)
{
    String spath;
    const char *slash;
    assert(self);
    if (VPathGetPath(aPath, &spath) != 0)
        return;
    if ((KDirectoryPathType(self->wd, spath.addr) & ~kptAlias) != kptDir)
        return;
    slash = strrchr(spath.addr, '/');
    if (slash)
        ++slash;
    else
        slash = spath.addr;
    if ((KDirectoryPathType(self->wd, "%s/%s.sra", spath.addr, slash)
        & ~kptAlias) != kptFile)
    {
        return;
    }
    VFSManagerMakePath(self->vfsmgr, (VPath **)path,
        "%s/%s.sra", spath.addr, slash);
}

typedef enum {
    eCheckExistFalse,
    eCheckExistTrue,
} ECheckExist;

typedef enum {
    eCheckFilePathFalse,
    eCheckFilePathTrue,
} ECheckFilePath; 

typedef enum {
    eCheckUrlFalse,
    eCheckUrlTrue,
} ECheckUrl;

/* Returned rc != 0: env-var was set but somehow invalid
                     or it is eet to empty string == query returns not found
 *  rc == 0:
 *      * path != NULL: env-var was set and valid: use it!
 *      * path == NULL:         was not set, keep going the old way
 */
static rc_t VFSManagerMagicResolve(const VFSManager *self,
    VPath ** path, const char * name,
    ECheckExist checkExist,
    ECheckFilePath checkPath,
    ECheckUrl checkUrl,
    bool * envVarWasSet)
{
    rc_t rc = 0;

    const char * magic = getenv(name);

    assert(path);
    *path = NULL;

    if (magic == NULL) {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), ("'%s' magic not set\n", name));
        return 0;
    }

    assert(envVarWasSet);
    *envVarWasSet = true;

    /* variable set to empty: VResolverQuery returns not found */
    if (magic[0] == '\0') {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), ("'%s' magic empty\n", name));
        return RC(rcVDB, rcResolver, rcResolving, rcName, rcNotFound);
    }

    rc = VFSManagerMakePath(self, path, "%s", magic);

    if (rc == 0) {
        assert(path);

        if (checkUrl == eCheckUrlTrue) {
            if (!(*path)->from_uri)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is not URL\n", name, magic));
                rc = RC(rcVDB, rcResolver, rcResolving, rcName, rcInvalid);
            }
            else if ((*path)->scheme_type != vpuri_http
                && (*path)->scheme_type != vpuri_https)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is not HTTP[S] URL\n", name, magic));
                rc = RC(rcVDB, rcResolver, rcResolving, rcName, rcInvalid);
            }
        }

        if (rc != 0) {
            VPathRelease(*path);
            *path = NULL;
            return rc;
        }
        else
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                "'%s' magic '%s' found\n", name, magic));
    }

    else
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
            "'%s' magic '%s' cannot be converted to VPath: %R\n",
            name, magic, rc));

    return rc;
}

static rc_t VFSManagerCacheMagicResolve(
    const VFSManager * self, VPath ** path, bool * envVarWasSet)
{
    return VFSManagerMagicResolve(self, path,
        MAGIC_CACHE_VDBCACHE,
        eCheckExistFalse, eCheckFilePathTrue, eCheckUrlFalse, envVarWasSet);
}

static rc_t VFSManagerLocalMagicResolve(
    const VFSManager * self, VPath ** path, bool * envVarWasSet)
{
    return VFSManagerMagicResolve(self, path,
        MAGIC_LOCAL_VDBCACHE,
        eCheckExistTrue, eCheckFilePathTrue, eCheckUrlFalse, envVarWasSet);
}

static rc_t VFSManagerRemoteMagicResolve(
    const VFSManager * self, VPath ** path, bool * envVarWasSet)
{
    return VFSManagerMagicResolve(self, path,
        MAGIC_REMOTE_VDBCACHE,
        eCheckExistFalse, eCheckFilePathFalse, eCheckUrlTrue, envVarWasSet);
}

static rc_t DBManagerOpenVdbcache(const VDBManager *self,
    const VFSManager * vfs, const VDatabase * db, const VSchema *schema,
    const VPath * orig, bool is_accession, const VResolver * resolver,
    const VPath ** pcache,
    const VPath * plocal, const VPath * premote)
    /* just one of plocal | premote is set */
{
    rc_t rc2 = 0;

    /* CSRA databases may have an associated "vdbcache" */
    const VDatabase * vdbcache = NULL;

    VPath * clocal = NULL;  /* local  VPath to vdbcache DB */
    VPath * ccache = NULL;  /* cache  VPath to vdbcache DB - for remote */
    VPath * cremote = NULL; /* remote VPath to vdbcache DB */

    bool magicWasSet = false;

    /* if principal was local */
    if (plocal != NULL)
    {
        rc2 = VFSManagerLocalMagicResolve(vfs, &clocal, &magicWasSet);
        if (rc2 == 0 && clocal == NULL)
            /* make local path to vdbcache out of DB's path
               just when magic variable is not set */
            rc2 = VFSManagerMakePathWithExtension(
                vfs, &clocal, plocal, ".vdbcache");
        if (rc2 == 0)
        {
            rc2 = VDBManagerVPathOpenLocalDBRead( /* try to open local */
                self, &vdbcache, schema, clocal); /* vdbcache DB */
            if (rc2 != 0)
            {   /* local vdbcache db does not exist */
                rc2 = 0;
                if (!is_accession)
                {
                    VPath * acc;
                    rc2 = VFSManagerExtractAccessionOrOID(vfs, &acc, orig);
                    if (rc2 == 0)
                    {
                        VPathRelease(orig);
                        orig = acc;
                    }
                }

                /* was not found locally - try to get one remotely */
                if (rc2 == 0
                    && !magicWasSet) /* check it remotely just when not */
                {                    /* called by driver tool: it sets EnvVar */
                    /* We need suppress error message in the
                     * case if here any error happened
                     */
                    KLogLevel lvl = KLogLevelGet();
                    KLogLevelSet(klogFatal);
                    assert(premote == NULL);
                    assert(pcache && *pcache == NULL);
     /* YES,
      DBG_VFS should be used here to be printed along with other VFS messages */
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE),
                (">>>>>>>>>>>>>> calling VResolverQuery to locate vdbcache..."
                            "\n"));
                    rc2 = VResolverQuery(
                        resolver, 0, orig, NULL, &premote, pcache);
                    assert((rc2 == 0) ||
                        (rc2 != 0 && premote == NULL));

                    /* Here we are restoring log level
                     */
                    KLogLevelSet(lvl);
                }
            }
        }
    }

    /* if principal was remote, or attempting remote vdbcache */
    if (premote != NULL)
    {
        rc2 = VFSManagerRemoteMagicResolve(vfs, &cremote, &magicWasSet);
        if (rc2 == 0 && cremote == NULL) {
            /* check if names service returned vdbcache */
            bool vdbcacheChecked = false;

            /* check remote path to vdbcache 
               just when magic variable is not set */
            rc2 = VPathGetVdbcache(premote,
                (const VPath **)& cremote, &vdbcacheChecked);

            /* try to manually build remote vdbcache path
               just when names service was not able to return vdbcache:
               should never happen these days */
            if (rc2 != 0 || vdbcacheChecked == false)
                rc2 = VFSManagerMakePathWithExtension(
                    vfs, (VPath**)& cremote, premote, ".vdbcache");
        }

        if (rc2 == 0) {
            rc2 = VFSManagerCacheMagicResolve(vfs, &ccache, &magicWasSet);
            if (rc2 == 0 && ccache == NULL) {
                /* manually build cache path to vdbcache
                   just when magic variable is not set */
                if (*pcache != NULL)
                    rc2 = VFSManagerMakePathWithExtension(
                        vfs, &ccache, *pcache, ".vdbcache");
            }
        }

        if (rc2 == 0)
            rc2 = VDBManagerVPathOpenRemoteDBRead(
                self, &vdbcache, schema, cremote, ccache);
    }

    VPathRelease(clocal);
    VPathRelease(cremote);
    VPathRelease(ccache);

    /* if "vdbcache" is anything but NULL, we got the cache */
    ((VDatabase*)db)->cache_db = vdbcache;

    return rc2;
}

LIB_EXPORT rc_t CC VDBManagerOpenDBReadVPath ( const VDBManager *self,
    const VDatabase ** dbp, const VSchema *schema,
    const VPath * path )
{
    rc_t rc = 0;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            /* need VFS manager to make a path */
            VFSManager * vfs;
            rc = KDBManagerGetVFSManager ( self -> kmgr, & vfs );
            if ( rc == 0 )
            {
                /* get a reference to its pre-built resolver */
                VResolver * resolver;
                rc = VFSManagerGetResolver ( vfs, & resolver );
                if ( rc == 0 )
                {
                    /* turn spec in "path_fmt" + "args" into VPath "orig" */
                    const VPath * aOrig = path;
                  /*rc = VFSManagerVMakePath ( vfs, &aOrig, path_fmt, args );*/
                    if ( rc == 0 )
                    {
                        /* the original VPath may get resolved into other paths */
                        const VPath * plocal = NULL, * premote = NULL, * pcache = NULL;

                        const VPath * orig = aOrig;
                        bool is_accession;

                        ad(self->kmgr, aOrig, &orig);

                        /* check whether we were given a path or accession */
                        is_accession = VPathIsAccessionOrOID ( orig );

                        /* if the original is not an accession */
                        if ( ! is_accession )
                        {
                            /* just create a new reference to original */
                            rc = VPathAddRef ( orig );
                            if ( rc == 0 )
                                plocal = orig;
                        }
                        else
                        {
                            /* otherwise, ask resolver to find a local path,
                               or get a remote path and optional place to cache data */
                            rc = VResolverQuery ( resolver, 0, orig, & plocal, & premote, & pcache );
                            if ( rc != 0 && GetRCState ( rc ) == rcNotFound )
                            {
                                rc = VPathAddRef ( orig );
                                if ( rc == 0 )
                                {
                                    plocal = orig;
                                }

                            }
                        }
                        if ( rc == 0 )
                        {
                            /* now open the principal database */
                            if ( plocal != NULL )
                                rc = VDBManagerVPathOpenLocalDBRead ( self, dbp, schema, plocal );
                            else if ( premote != NULL )
                                rc = VDBManagerVPathOpenRemoteDBRead ( self, dbp, schema, premote, pcache );
                            else
                            {
                                /* resolver was unable to resolve this, so perhaps it was
                                   not an accession or OID, but a simple file name */
                                rc = VPathAddRef ( orig );
                                if ( rc == 0 )
                                {
                                    plocal = orig;
#define VDB_3531_FIX 1
#if VDB_3531_FIX
                                    if ( pcache != NULL )
                                    {
                                        VPathRelease(pcache);
                                        pcache = NULL;
                                    }
#endif
                                    rc = VDBManagerVPathOpenLocalDBRead ( self, dbp, schema, plocal );
                                }
                            }
                            if ( rc == 0 )
                            {
                                const VDatabase * db = * dbp;
                                if ( VDatabaseIsCSRA ( db ) )
                                {
                                    DBManagerOpenVdbcache(self, vfs, db,
                                        schema, orig, is_accession, resolver, 
                                        &pcache, plocal, premote);
#if 0
                                    /* CSRA databases may have an associated "vdbcache" */
                                    const VDatabase * vdbcache = NULL;
                                    VPath * clocal = NULL, * ccache = NULL;
                                    const VPath * cremote = NULL;
                                    /* if principal was local */
                                    if ( plocal != NULL )
                                    {
                                        rc2 = VFSManagerMakePathWithExtension ( vfs, & clocal, plocal, ".vdbcache" );
                                        if ( rc2 == 0 )
                                        {
                                            rc2 = VDBManagerVPathOpenLocalDBRead ( self, & vdbcache, schema, clocal );
                                            if ( rc2 != 0 )
                                            {
                                                rc2 = 0;
                                                if ( ! is_accession )
                                                {
                                                    VPath * acc;
                                                    rc2 = VFSManagerExtractAccessionOrOID ( vfs, & acc, orig );
                                                    if ( rc2 == 0 )
                                                    {
                                                        VPathRelease ( orig );
                                                        orig = acc;
                                                    }
                                                }

                                                /* was not found locally - try to get one remotely */
                                                if ( rc2 == 0 )
                                                {
                                                        /* We need suppress error message in the 
                                                         * case if here any error happened
                                                         */
                                                    KLogLevel lvl = KLogLevelGet ();
                                                    KLogLevelSet ( klogFatal );
                                                    assert ( premote == NULL );
                                                    assert ( pcache == NULL );
                                                    rc2 = VResolverQuery ( resolver, 0, orig, NULL, & premote, & pcache );
                                                    assert ( ( rc2 == 0 ) ||
                                                        ( rc2 != 0 && premote == NULL ) );

                                                        /* Here we are restoring log level
                                                         */
                                                    KLogLevelSet ( lvl );
                                                }
                                            }
                                        }
                                    }

                                    /* if principal was remote, or attempting remote vdbcache */
                                    if ( premote != NULL )
                                    {
                                        /* check
                                           if names service returned vdbache */
                                        bool vdbcacheChecked = false;
                                        rc2 = VPathGetVdbcache ( premote,
                                            & cremote, & vdbcacheChecked );

                                        /* try to manually build remote vdbcache path
                                           just when names service was not able to return vdbcache: shold never happen these days */
                                        if ( rc2 != 0 || vdbcacheChecked == false )
                                            rc2 = VFSManagerMakePathWithExtension ( vfs, (VPath**) & cremote, premote, ".vdbcache" );

                                        if ( rc2 == 0 && pcache != NULL )
                                            rc2 = VFSManagerMakePathWithExtension ( vfs, & ccache, pcache, ".vdbcache" );
                                        if ( rc2 == 0 )
                                            rc2 = VDBManagerVPathOpenRemoteDBRead ( self, & vdbcache, schema, cremote, ccache );
                                    }

                                    VPathRelease ( clocal );
                                    VPathRelease ( cremote );
                                    VPathRelease ( ccache );

                                    /* if "vdbcache" is anything but NULL, we got the cache */
                                    ( ( VDatabase* ) db ) -> cache_db = vdbcache;
#endif
                                }
                            }

                            VPathRelease ( plocal );
                            VPathRelease ( premote );
                            VPathRelease ( pcache );
                        }

                        /*if (aOrig != orig)
                            VPathRelease(aOrig);

                        VPathRelease ( orig );*/
                    }

                    VResolverRelease ( resolver );
                }

                VFSManagerRelease ( vfs );
            }
        }

        if ( rc != 0 )
            * dbp = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VDBManagerVOpenDBRead ( const VDBManager *self,
    const VDatabase ** dbp, const VSchema *schema,
    const char * path_fmt, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            /* need VFS manager to make a path */
            VFSManager * vfs;
            rc = KDBManagerGetVFSManager ( self -> kmgr, & vfs );
            if ( rc == 0 )
            {
                /* get a reference to its pre-built resolver *
                VResolver * resolver;
                rc = VFSManagerGetResolver ( vfs, & resolver );*/
                if ( rc == 0 )
                {
                    /* turn spec in "path_fmt" + "args" into VPath "orig" */
                    VPath * aOrig = NULL;
                    rc = VFSManagerVMakePath ( vfs, &aOrig, path_fmt, args );
                    if ( rc == 0 )
                        rc = VDBManagerOpenDBReadVPath ( self, dbp, schema, aOrig );
                    VPathRelease ( aOrig );

#if 0
                    {
                        /* the original VPath may get resolved into other paths */
                        const VPath * plocal = NULL, * premote = NULL, * pcache = NULL;

                        const VPath * orig = aOrig;
                        ad(self->kmgr, aOrig, &orig);

                        /* check whether we were given a path or accession */
                        bool is_accession = VPathIsAccessionOrOID ( orig );

                        /* if the original is not an accession */
                        if ( ! is_accession )
                        {
                            /* just create a new reference to original */
                            rc = VPathAddRef ( orig );
                            if ( rc == 0 )
                                plocal = orig;
                        }
                        else
                        {
                            /* otherwise, ask resolver to find a local path,
                               or get a remote path and optional place to cache data */
                            rc = VResolverQuery ( resolver, 0, orig, & plocal, & premote, & pcache );
                            if ( rc != 0 && GetRCState ( rc ) == rcNotFound )
                            {
                                rc = VPathAddRef ( orig );
                                if ( rc == 0 )
                                {
                                    plocal = orig;
                                }

                            }
                        }
                        if ( rc == 0 )
                        {
                            /* now open the principal database */
                            if ( plocal != NULL )
                                rc = VDBManagerVPathOpenLocalDBRead ( self, dbp, schema, plocal );
                            else if ( premote != NULL )
                                rc = VDBManagerVPathOpenRemoteDBRead ( self, dbp, schema, premote, pcache );
                            else
                            {
                                /* resolver was unable to resolve this, so perhaps it was
                                   not an accession or OID, but a simple file name */
                                rc = VPathAddRef ( orig );
                                if ( rc == 0 )
                                {
                                    plocal = orig;
#define VDB_3531_FIX 1
#if VDB_3531_FIX
                                    if ( pcache != NULL )
                                    {
                                        VPathRelease(pcache);
                                        pcache = NULL;
                                    }
#endif
                                    rc = VDBManagerVPathOpenLocalDBRead ( self, dbp, schema, plocal );
                                }
                            }
                            if ( rc == 0 )
                            {
                                const VDatabase * db = * dbp;
                                if ( VDatabaseIsCSRA ( db ) )
                                {
                                    DBManagerOpenVdbcache(self, vfs, db,
                                        schema, orig, is_accession, resolver, 
                                        &pcache, plocal, premote);
#if 0
                                    /* CSRA databases may have an associated "vdbcache" */
                                    const VDatabase * vdbcache = NULL;
                                    VPath * clocal = NULL, * ccache = NULL;
                                    const VPath * cremote = NULL;
                                    /* if principal was local */
                                    if ( plocal != NULL )
                                    {
                                        rc2 = VFSManagerMakePathWithExtension ( vfs, & clocal, plocal, ".vdbcache" );
                                        if ( rc2 == 0 )
                                        {
                                            rc2 = VDBManagerVPathOpenLocalDBRead ( self, & vdbcache, schema, clocal );
                                            if ( rc2 != 0 )
                                            {
                                                rc2 = 0;
                                                if ( ! is_accession )
                                                {
                                                    VPath * acc;
                                                    rc2 = VFSManagerExtractAccessionOrOID ( vfs, & acc, orig );
                                                    if ( rc2 == 0 )
                                                    {
                                                        VPathRelease ( orig );
                                                        orig = acc;
                                                    }
                                                }

                                                /* was not found locally - try to get one remotely */
                                                if ( rc2 == 0 )
                                                {
                                                        /* We need suppress error message in the 
                                                         * case if here any error happened
                                                         */
                                                    KLogLevel lvl = KLogLevelGet ();
                                                    KLogLevelSet ( klogFatal );
                                                    assert ( premote == NULL );
                                                    assert ( pcache == NULL );
                                                    rc2 = VResolverQuery ( resolver, 0, orig, NULL, & premote, & pcache );
                                                    assert ( ( rc2 == 0 ) ||
                                                        ( rc2 != 0 && premote == NULL ) );

                                                        /* Here we are restoring log level
                                                         */
                                                    KLogLevelSet ( lvl );
                                                }
                                            }
                                        }
                                    }

                                    /* if principal was remote, or attempting remote vdbcache */
                                    if ( premote != NULL )
                                    {
                                        /* check
                                           if names service returned vdbache */
                                        bool vdbcacheChecked = false;
                                        rc2 = VPathGetVdbcache ( premote,
                                            & cremote, & vdbcacheChecked );

                                        /* try to manually build remote vdbcache path
                                           just when names service was not able to return vdbcache: shold never happen these days */
                                        if ( rc2 != 0 || vdbcacheChecked == false )
                                            rc2 = VFSManagerMakePathWithExtension ( vfs, (VPath**) & cremote, premote, ".vdbcache" );

                                        if ( rc2 == 0 && pcache != NULL )
                                            rc2 = VFSManagerMakePathWithExtension ( vfs, & ccache, pcache, ".vdbcache" );
                                        if ( rc2 == 0 )
                                            rc2 = VDBManagerVPathOpenRemoteDBRead ( self, & vdbcache, schema, cremote, ccache );
                                    }

                                    VPathRelease ( clocal );
                                    VPathRelease ( cremote );
                                    VPathRelease ( ccache );

                                    /* if "vdbcache" is anything but NULL, we got the cache */
                                    ( ( VDatabase* ) db ) -> cache_db = vdbcache;
#endif
                                }
                            }

                            VPathRelease ( plocal );
                            VPathRelease ( premote );
                            VPathRelease ( pcache );
                        }

                        if (aOrig != orig)
                            VPathRelease(aOrig);

                        VPathRelease ( orig );
                    }

                    VResolverRelease ( resolver );
#endif
                }

                VFSManagerRelease ( vfs );
            }
        }

        if ( rc != 0 )
            * dbp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDBManagerOpenDBRead ( const VDBManager *self,
    const VDatabase **db, const VSchema *schema,
    const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVOpenDBRead ( self, db, schema, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVOpenDBRead ( const VDatabase *self,
    const VDatabase **dbp, const char *name, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSelf, rcNull );
        else
        {
            VDatabase *db;
            rc = VDatabaseMake ( & db, self -> mgr, self, self -> schema );
            if ( rc == 0 )
            {
                db -> read_only = true;

                rc = KDatabaseVOpenDBRead ( self -> kdb, & db -> kdb, name, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenRead ( db );
                    if ( rc == 0 )
                    {
                        * dbp = db;
                        return 0;
                    }
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseOpenDBRead ( const VDatabase *self,
    const VDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVOpenDBRead ( self, db, name, args );
    va_end ( args );

    return rc;
}


/* Locked
 *  returns true if locked
 */
LIB_EXPORT bool CC VDatabaseLocked ( const VDatabase *self )
{
    /* TBD - no concept of virtual database locking
       other than physical database locking */
    if ( self != NULL )
        return KDatabaseLocked ( self -> kdb );
    return false;
}


/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name, 
 *
 *  "name" [ IN ] - NUL terminated object name
 */
LIB_EXPORT bool CC VDatabaseIsAlias ( const VDatabase *self, uint32_t type,
    char *resolved, size_t rsize, const char *name )
{
    if ( self != NULL )
        return KDatabaseIsAlias ( self -> kdb, type, resolved, rsize, name );

    if ( resolved != NULL && rsize != 0 )
        * resolved = 0;

    return false;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VDatabaseVWritable ( const VDatabase *self, uint32_t type, 
        const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
    else
        rc = KDatabaseVWritable ( self -> kdb, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseWritable ( const VDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VDatabaseVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}

/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
LIB_EXPORT rc_t CC VDatabaseOpenMetadataRead ( const VDatabase *self, const KMetadata **meta )
{
    rc_t rc;
    if ( meta == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        * meta = NULL;

        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            /* we operate under the notion of
               single-threaded operation, so hand out
               read or update capable object */
            rc = KMetadataAddRef ( self -> meta );
            if ( rc == 0 )
                * meta = self -> meta;
        }
    }

    return rc;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VDatabaseOpenManagerRead ( const VDatabase *self, const VDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VDBManagerAddRef ( self -> mgr );
            if ( rc == 0 )
            {
                * mgr = self -> mgr;
                return 0;
            }
        }

        * mgr = NULL;
    }

    return rc;
}


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VDatabaseOpenParentRead ( const VDatabase *self, const VDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VDatabaseAddRef ( self -> dad );
            if ( rc == 0 )
            {
                * par = self -> dad;
                return 0;
            }
        }

        * par = NULL;
    }

    return rc;
}


/* OpenSchema
 *  duplicate reference to table schema
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VDatabaseOpenSchema ( const VDatabase *self, const VSchema **schema )
{
    rc_t rc;

    if ( schema == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VSchemaAddRef ( self -> schema );
            if ( rc == 0 )
            {
                * schema = self -> schema;
                return 0;
            }
        }

        * schema = NULL;
    }

    return rc;
}


/* lists the tables of the database
 * 
 * "names" [ OUT ] - return parameter for tables 
 */
LIB_EXPORT rc_t CC VDatabaseListTbl ( const VDatabase *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcListing, rcSelf, rcNull );
        else
        {
            if ( self->kdb == NULL )
                rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
            else
            {
                return KDatabaseListTbl ( self->kdb, names );
            }
        }
        *names = NULL;
    }
    return rc;
}

/* lists the sub-databases of the database
 * 
 * "names" [ OUT ] - return parameter for databases 
 */
LIB_EXPORT rc_t CC VDatabaseListDB ( const VDatabase *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcListing, rcSelf, rcNull );
        else
        {
            if ( self->kdb == NULL )
                rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
            else
            {
                return KDatabaseListDB ( self->kdb, names );
            }
        }
        *names = NULL;
    }
    return rc;
}

/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 */
LIB_EXPORT rc_t CC VDatabaseGetUserData ( const VDatabase *self, void **data )
{
    rc_t rc;

    if ( data == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            * data = self -> user;
            return 0;
        }

        * data = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseSetUserData ( const VDatabase *cself,
    void *data, void ( CC * destroy ) ( void *data ) )
{
    VDatabase *self = ( VDatabase* ) cself;
    if ( cself == NULL )
        return RC ( rcVDB, rcDatabase, rcUpdating, rcSelf, rcNull );

    self -> user = data;
    self -> user_whack = destroy;

    return 0;
}


/* OpenKDatabase
 *  returns a new reference to underlying KDatabase
 */
LIB_EXPORT rc_t CC VDatabaseOpenKDatabaseRead ( const VDatabase *self, const KDatabase **kdb )
{
    rc_t rc;

    if ( kdb == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDatabaseAddRef ( self -> kdb );
            if ( rc == 0 )
            {
                * kdb = self -> kdb;
                return 0;
            }
        }

        * kdb = NULL;
    }

    return rc;
}



/* Typespec
 *  retrieve db typespec
 *
 *  AVAILABILITY: version 2.2
 *
 *  "ts_buff" [ IN ] and "ts_size" [ IN ] - buffer for return of NUL-terminated
 *  db typespec, e.g. 'NCBI:db:abc#1.1'
 */
LIB_EXPORT rc_t CC VDatabaseTypespec ( const VDatabase *self, char *ts_buff, size_t ts_size )
{
    rc_t rc;

    if ( ts_size == 0 )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcBuffer, rcInsufficient );
    else if ( ts_buff == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcBuffer, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc_t rc2;
            rc2 = string_printf ( ts_buff, ts_size, NULL, "%N#%V", self -> sdb -> name, self -> sdb -> version );
            if ( rc2 == 0 )
                return 0;
            rc = rc2;
        }

        ts_buff [ 0 ] = 0;
    }

    return rc;
}


/* IsCSRA
 *  ask an open database if it conforms to cSRA schema
 */
LIB_EXPORT bool CC VDatabaseIsCSRA ( const VDatabase *self )
{
    if ( self != NULL )
    {
        if ( KDatabaseExists ( self -> kdb, kptTable, "PRIMARY_ALIGNMENT" ) )
        {
            return KDatabaseExists ( self -> kdb, kptTable, "SEQUENCE" );
        }
    }

    return false;
}

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

LIB_EXPORT
rc_t CC VDatabaseGetAccession(const VDatabase * self, const String ** aAcc)
{
    rc_t rc = 0;

    const VDBManager *mgr = NULL;
    const KDBManager* kmgr = NULL;
    VFSManager* vfsmgr = NULL;
    const KDatabase * kdb = NULL;

    const char * path = NULL;

    if (rc == 0)
        rc = VDatabaseOpenManagerRead(self, &mgr);
    if (rc == 0)
        rc = VDBManagerGetKDBManagerRead(mgr, &kmgr);
    if (rc == 0)
        rc = KDBManagerGetVFSManager(kmgr, &vfsmgr);
    if (rc == 0)
        rc = VDatabaseOpenKDatabaseRead(self, &kdb);
    if (rc == 0)
        rc = KDatabaseGetPath(kdb, &path);

    if (rc == 0) {
        /* path = "/S/S.sra"; */
        uint32_t pathLen = string_measure(path, NULL);
        const char * last = string_rchr(path, pathLen, '/'); /* find the last '/' */
        if (last != NULL) {
            uint32_t fileLen = pathLen - (last - path) - 1;
            uint32_t l = pathLen - fileLen - 1;
            const char * start = NULL;
            uint32_t accLen = 0;
            String acc;

            start = string_rchr(path, l, '/'); /* find the second last '/' */
            if (start == NULL)
                start = path;
            else
                ++start;

            accLen = last - start;
            StringInit(&acc, start, accLen, accLen);

            if (fileLen == accLen + 4) {
                const char * file = last + 1;
                if (string_cmp(file, fileLen,
                    acc.addr, acc.size, acc.size) == 0)
                {
                    const char ext[] = ".sra";
                    if (string_cmp(file + accLen, fileLen - accLen,
                        ext, sizeof ext - 1, sizeof ext - 1) == 0)
                    {
                        rc = StringCopy(aAcc, &acc);
                    }
                }
            }
        }
    }

    RELEASE(VDBManager, mgr);
    RELEASE(KDBManager, kmgr);
    RELEASE(VFSManager, vfsmgr);
    RELEASE(KDatabase, kdb);

    return rc;
}
