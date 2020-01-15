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

#define TRACK_REFERENCES 0
/* should match dbmgr.c and/or wdbmgr.c */

#define KONST const
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "linker-priv.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>

#include <vfs/manager.h> /* VFSManager */
#include <vfs/manager-priv.h> /* VFSManagerSetResolver */

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/kdb-priv.h> /* KDBManagerGetVFSManager */
#include <kdb/table.h>
#include <kdb/meta.h>

#include <kfg/config.h>
#include <kfs/directory.h>
#include <kfs/dyload.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if 1
#define DEBUG_PRINT( fmt, ... ) ( void ) 0
#else
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "%s - " fmt "\n", __func__, __VA_ARGS__)
#endif

/*--------------------------------------------------------------------------
 * VDBManager
 *  opaque handle to library
 */

/* Whack
 */
static
rc_t VDBManagerWhack ( VDBManager *self )
{
    rc_t rc;

    KRefcountWhack ( & self -> refcount, "VDBManager" );

    rc = KDBManagerRelease ( self -> kmgr );
    if ( rc == 0 )
    {
        /* release user data */
        if ( self -> user != NULL && self -> user_whack != NULL )
        {
            ( * self -> user_whack ) ( self -> user );
            self -> user = NULL;
            self -> user_whack = NULL;
        }

        VSchemaRelease ( self -> schema );
        VLinkerRelease ( self -> linker );
        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "VDBManager", "whack", "vmgr" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC VDBManagerAddRef ( const VDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VDBManager" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VDBManagerRelease ( const VDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VDBManager" ) )
        {
        case krefWhack:
            return VDBManagerWhack ( ( VDBManager* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 *  internal reference management
 */
VDBManager *VDBManagerAttach ( const VDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VDBManager" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VDBManager* ) self;
}

rc_t VDBManagerSever ( const VDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VDBManager" ) )
        {
        case krefWhack:
            return VDBManagerWhack ( ( VDBManager* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VDBManagerVWritable ( const VDBManager *self, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
    return KDBManagerVWritable ( self -> kmgr, path, args );
}

LIB_EXPORT rc_t CC VDBManagerWritable ( const VDBManager *self, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = VDBManagerVWritable ( self, path, args );

    va_end ( args );

    return rc;
}


/* AddSchemaIncludePath
 *  add an include path to schema for locating input files
 */
LIB_EXPORT rc_t CC VDBManagerVAddSchemaIncludePath ( const VDBManager *self, const char *path, va_list args )
{
    if ( self != NULL )
        return VSchemaVAddIncludePath ( self -> schema, path, args );

    return RC ( rcVDB, rcMgr, rcUpdating, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC VDBManagerAddSchemaIncludePath ( const VDBManager *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVAddSchemaIncludePath ( self, path, args );
    va_end ( args );

    return rc;
}


/* AddLoadLibraryPath
 *  add a path[s] to loader for locating dynamic libraries
 */
LIB_EXPORT rc_t CC VDBManagerVAddLoadLibraryPath ( const VDBManager *self, const char *path, va_list args )
{
    if ( self != NULL )
        return VLinkerVAddLoadLibraryPath ( self -> linker, path, args );

    return RC ( rcVDB, rcMgr, rcUpdating, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC VDBManagerAddLoadLibraryPath ( const VDBManager *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVAddLoadLibraryPath ( self, path, args );
    va_end ( args );

    return rc;
}


/* ConfigFromKfg
 */
static
rc_t VDBManagerGetKfgPath ( const KConfig *kfg, const char *path, char *value, size_t value_size, size_t *num_read )
{
    /* open node */
    const KConfigNode *node;
    rc_t rc = KConfigOpenNodeRead ( kfg, & node, "%s", path );
    if ( rc == 0 )
    {
        size_t remaining;
        rc = KConfigNodeRead ( node, 0, value, value_size - 1, num_read, & remaining );
        if ( rc == 0 )
        {
            if ( remaining != 0 )
                rc = RC ( rcVDB, rcMgr, rcConstructing, rcPath, rcExcessive );
            else if ( string_chr ( value, * num_read, '%' ) != NULL )
                rc = RC ( rcVDB, rcMgr, rcConstructing, rcPath, rcInvalid );
            else
                value[*num_read] = '\0';
        }

        KConfigNodeRelease ( node );
    }
    return rc;
}

static
rc_t VDBManagerConfigFromKfg ( VDBManager *self, bool update )
{
    size_t num_read;
    char full [ 4096 ];

    /* open configuration manager */
    KConfig *kfg;
    rc_t rc = KConfigMake ( & kfg, NULL );
    if ( rc != 0 )
        rc = 0;
    else
    {
        /* look for load library paths */
        rc = VDBManagerGetKfgPath ( kfg, update ?
            "vdb/wmodule/paths": "vdb/module/paths",
            full, sizeof full, & num_read );
        if ( rc != 0 )
            rc = 0;
        else
        {
            /* split by ':' */
            const char * path = full;
            const char *colon = string_chr ( full, num_read, ':' );
            while ( colon != NULL )
            {
                /* add path between "path" and "colon" */
                rc = VDBManagerAddLoadLibraryPath ( self, "%.*s", ( int ) ( colon - path ), path );
                if ( rc != 0 )
                    break;
                num_read -= ( colon - path ) - 1;
                path = colon + 1;
                colon = string_chr ( path, num_read, ':' );
            }

            /* add in last portion */
            if ( rc == 0 && num_read != 0 )
                rc = VDBManagerAddLoadLibraryPath ( self, "%.*s", ( int ) ( num_read ), path );
        }

        /* look for schema paths */
        if ( rc == 0 )
        {
            rc = VDBManagerGetKfgPath ( kfg, "vdb/schema/paths", full, sizeof full, & num_read );
            if ( rc == 0 )
            {
                PLOGMSG ( klogDebug, ( klogDebug, "VDBManagerConfigFromKfg: vdb/schema/paths = '$(path)'", "path=%s", full ) );
            }
        }
        if ( rc != 0 )
            rc = 0;
        else
        {
            /* split by ':' */
            const char * path = full;
            const char *colon = string_chr ( full, num_read, ':' );
            while ( colon != NULL )
            {
                /* add path between "path" and "colon" */
                rc = VDBManagerAddSchemaIncludePath ( self, "%.*s", ( int ) ( colon - path ), path );
                if ( rc != 0 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "schema path '$(path)' was rejected",
                                          "path=%.*s", ( int ) ( colon - path ), path ) );
                    rc = 0;
                }
                num_read -= colon - path + 1;
                path = colon + 1;
                colon = string_chr ( path, num_read, ':' );
            }

            /* add in last portion */
            if ( rc == 0 && num_read != 0 )
            {
                rc = VDBManagerAddSchemaIncludePath ( self, "%.*s", ( int ) ( num_read ), path );
                if ( rc != 0 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "schema path '$(path)' was rejected",
                                          "path=%.*s", ( int ) num_read, path ) );
                    rc = 0;
                }
            }
        }

        KConfigRelease ( kfg );
    }

    return rc;
}

/* ConfigFromLibPath
 *  trace this library back to file system
 *  attempt to locate relative paths
 *  add them to library and schema path lists
 */
static
rc_t VDBManagerConfigFromLibPath ( VDBManager *self, bool update )
{
    KDyld *dlmgr;
    rc_t rc = KDyldMake ( & dlmgr );
    if ( rc == 0 )
    {
        const KDirectory *home;
        rc = KDyldHomeDirectory ( dlmgr, & home, ( fptr_t ) VDBManagerConfigPaths );
        if ( rc == 0 )
        {
            char full [ 4096 ];

            /* paths to modules */
            const char *mod_paths [] =
            {
                "ncbi/mod",
                "vdb/mod",
                "../mod"
            };
            const char *wmod_paths [] =
            {
                "ncbi/wmod",
                "vdb/wmod",
                "../wmod"
            };

            /* paths to schema */
            const char *schema_paths [] =
            {
                "ncbi/schema",
                "vdb/schema",
                "../schema"
            };

            /* test for a few possible module paths */
            uint32_t i;
            const char **paths = update ? wmod_paths : mod_paths;
            for ( i = 0; rc == 0 && i < sizeof mod_paths / sizeof mod_paths [ 0 ]; ++ i )
            {
                uint32_t type = KDirectoryPathType ( home, "%s%u", paths [ i ], sizeof ( void* ) * 8 );
                if ( ( type & ~ kptAlias ) == kptDir )
                {
                    /* add full path */
                    rc = KDirectoryResolvePath ( home, true, full, sizeof full, "%s%u", paths [ i ], sizeof ( void* ) * 8 );
                    if ( rc == 0 )
                        rc = VDBManagerAddLoadLibraryPath ( self, "%s", full );
                }
            }

            for ( i = 0; rc == 0 && i < sizeof mod_paths / sizeof mod_paths [ 0 ]; ++ i )
            {
                uint32_t type = KDirectoryPathType ( home, "%s", paths [ i ] );
                if ( ( type & ~ kptAlias ) == kptDir )
                {
                    /* add full path */
                    rc = KDirectoryResolvePath ( home, true, full, sizeof full, "%s", paths [ i ] );
                    if ( rc == 0 )
                        rc = VDBManagerAddLoadLibraryPath ( self, "%s", full );
                }
            }

            for ( i = 0; rc == 0 && i < sizeof schema_paths / sizeof schema_paths [ 0 ]; ++ i )
            {
                uint32_t type = KDirectoryPathType ( home, "%s", schema_paths [ i ] );
                if ( ( type & ~ kptAlias ) == kptDir )
                {
                    /* add full path */
                    rc = KDirectoryResolvePath ( home, true, full, sizeof full, "%s", schema_paths [ i ] );
                    if ( rc == 0 )
                        rc = VDBManagerAddSchemaIncludePath ( self, "%s", full );
                }
            }

            KDirectoryRelease ( home );
        }

        KDyldRelease ( dlmgr );
    }
    return rc;
}

/* ConfigPaths
 *  looks for configuration information to set
 *  include paths for schema parser and
 *  load paths for linker
 */
rc_t VDBManagerConfigPaths ( VDBManager *self, bool update )
{
    rc_t rc = VDBManagerConfigFromKfg ( self, update );
    if ( rc == 0 )
        rc = VDBManagerConfigFromLibPath ( self, update );
    return rc;
}


/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 */
LIB_EXPORT rc_t CC VDBManagerGetUserData ( const VDBManager *self, void **data )
{
    rc_t rc;

    if ( data == NULL )
        rc = RC ( rcVDB, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            * data = self -> user;
            return 0;
        }

        * data = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDBManagerSetUserData ( const VDBManager *cself,
    void *data, void ( CC * destroy ) ( void *data ) )
{
    VDBManager *self = ( VDBManager* ) cself;
    if ( cself == NULL )
        return RC ( rcVDB, rcMgr, rcUpdating, rcSelf, rcNull );

    self -> user = data;
    self -> user_whack = destroy;

    return 0;
}

/* OpenKDBManager
 *  returns a new reference to KDBManager used by VDBManager
 */
LIB_EXPORT rc_t CC VDBManagerOpenKDBManagerRead ( const VDBManager *self, const KDBManager **kmgr )
{
    rc_t rc;

    if ( kmgr == NULL )
        rc = RC ( rcVDB, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDBManagerAddRef ( self -> kmgr );
            if ( rc == 0 )
            {
                * kmgr = self -> kmgr;
                return 0;
            }
        }

        * kmgr = NULL;
    }

    return rc;
}


static
ver_t VDBManagerGetLoaderVersFromMeta ( const KMetadata * meta )
{
    const KMDataNode *node = NULL;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "SOFTWARE/loader" );
    KMDataNodeRelease ( node );
    if ( rc == 0 )
    {
        size_t num_read;
        char vers_string [ 64 ];
        rc = KMDataNodeReadAttr ( node, "vers", vers_string, sizeof vers_string, & num_read );
        if ( rc == 0 )
        {
            char *end, *start = vers_string;
            unsigned long maj = strtoul ( start, & end, 10 );

            KMDataNodeRelease ( node );

            if ( end > start && end [ 0 ] == '.' )
            {
                unsigned long min = strtoul ( start = end + 1, & end, 10 );
                if ( end > start && end [ 0 ] == '.' )
                {
                    unsigned long rel = strtoul ( start = end + 1, & end, 10 );
                    if ( end > start )
                        return ( maj << 24 ) | ( min << 16 ) | rel;
                }
            }
        }
    }
    return 0;
}


/* VDBManagerGetVersFromMeta
 *  helper-function for VDBManagerGetObjVersion
 */
static
ver_t VDBManagerGetVersFromMeta ( const KMetadata * meta, bool is_db )
{
    rc_t rc = 0;
    uint32_t meta_vers;
    const KMDataNode *node;

    /* look for loader version as first clue */
    ver_t version = VDBManagerGetLoaderVersFromMeta ( meta );
    if ( VersionGetMajor ( version ) >= 2 ) {
        return version;
    }

    /* was the run vdb-copied? */
    rc = KMetadataOpenNodeRead ( meta, & node, "SOFTWARE/update" );
    if ( rc == 0 ) {
        KMDataNodeRelease ( node );
        return 0x2000000;
    }

    /* If SOFTWARE/loader/@vers exists but SOFTWARE/update does not
     * then return the former */
    if ( version != 0 ) {
        return version;
    }

    /* THE PRINCIPAL DESCRIPTION OF SOFTWARE VERSION
       WAS NOT FOUND. USE HEURISTICS TO DETERMINE VERSION
       FROM THIS POINT FORWARD. */

    /* look at the version of the metadata file itself
       metdata version 1 means pre-vdb, i.e. v0 */
    rc = KMetadataVersion ( meta, & meta_vers );
    if ( rc == 0 && meta_vers < 2 )
        return 0;

    /* assume all databases are v2 */
    if ( is_db )
        return 0x2000000;


    /* AT THIS POINT, WE KNOW WE MUST HAVE A TABLE,
       BUT WE DON'T HAVE PERFECT EVIDENCE OF VERSION,
       SINCE THE SOFTWARE NODE IS NOT MANDATORY. */

    /* if "schema" exists, it's v2 */
    rc = KMetadataOpenNodeRead ( meta, & node, "schema" );
    if ( rc == 0 )
    {
        KMDataNodeRelease ( node );
        return 0x2000000;
    }

    /* if "col" exists, it's v1 */
    rc = KMetadataOpenNodeRead ( meta, &node, "col" );
    if ( rc == 0 )
    {
        KMDataNodeRelease ( node );
        return 0x1000000;
    }

    /* appears to be v0 */
    return 0;
}


/* VDBManagerGetObjVersion
 *  returns the vdb-version of the given object
 */
LIB_EXPORT rc_t CC VDBManagerGetObjVersion ( const VDBManager *self, ver_t * version, const char *path )
{
    rc_t rc;

    if ( version == NULL )
        rc = RC ( rcVDB, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcEmpty );
        else
        {
            const KTable *tbl;
            const KDatabase *db;
            const KMetadata *meta;

            int path_type = KDBManagerPathType ( self -> kmgr, "%s", path ) & ~ kptAlias;
            switch ( path_type )
            {
            case kptDatabase:
                rc = KDBManagerOpenDBRead ( self -> kmgr, & db, "%s", path );
                if ( rc == 0 )
                {
                    rc = KDatabaseOpenMetadataRead ( db, & meta );
                    KDatabaseRelease ( db );
                }
                break;
            case kptTable:
                rc = KDBManagerOpenTableRead ( self -> kmgr, & tbl, "%s", path );
                if ( rc == 0 )
                {
                    rc = KTableOpenMetadataRead ( tbl, & meta );
                    KTableRelease ( tbl );
                }
                break;

            case kptPrereleaseTbl:
                * version = 0;
                return 0;

            default:
                rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcIncorrect );
            }

            if ( rc == 0 )
            {
                * version = VDBManagerGetVersFromMeta ( meta, path_type == kptDatabase );
                KMetadataRelease ( meta );
                return 0;
            }
        }

        * version = 0;
    }

    return rc;
}

/* VDBManagerGetObjModDate
 *  returns the load/modification timestamp of the given object
 */
LIB_EXPORT rc_t CC VDBManagerGetObjModDate ( const VDBManager *self, KTime_t *timestamp, const char *path )
{
    rc_t rc;

    if ( timestamp == NULL )
        rc = RC ( rcVDB, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcEmpty );
        else
        {
            const KTable *tbl;
            const KDatabase *db;
            const KMetadata *meta;

            int path_type = KDBManagerPathType ( self -> kmgr, "%s", path ) & ~ kptAlias;
            switch ( path_type )
            {
            case kptDatabase:
                rc = KDBManagerOpenDBRead ( self -> kmgr, & db, "%s", path );
                if ( rc == 0 )
                {
                    rc = KDatabaseOpenMetadataRead ( db, & meta );
                    KDatabaseRelease ( db );
                }
                break;
            case kptTable:
                rc = KDBManagerOpenTableRead ( self -> kmgr, & tbl, "%s", path );
                if ( rc == 0 )
                {
                    rc = KTableOpenMetadataRead ( tbl, & meta );
                    KTableRelease ( tbl );
                }
                break;

            case kptPrereleaseTbl:
                rc = 0;
                * timestamp = 0;
                meta = NULL;
                break;

            default:
                rc = RC ( rcVDB, rcMgr, rcAccessing, rcPath, rcIncorrect );
            }

            if ( rc == 0 )
            {
                if ( meta != NULL )
                {
                    const KMDataNode *node;
                    rc = KMetadataOpenNodeRead ( meta, & node, "LOAD/timestamp" );
                    KMetadataRelease ( meta );
                    if ( rc == 0 )
                    {
                        rc = KMDataNodeReadAsI64 ( node, timestamp );
                        KMDataNodeRelease ( node );
                        if ( rc == 0 )
                            return 0;
                    }
                }

                /* TBD - at this point our recourse is to check for
                   the object's timestamp in the filesystem */
            }
        }

        * timestamp = 0;
    }

    return rc;
}

/* ListExternalSchemaModules
 */
LIB_EXPORT rc_t CC VDBManagerListExternalSchemaModules ( const VDBManager *self, KNamelist **listp )
{
    rc_t rc;

    if ( listp == NULL )
        rc = RC ( rcVDB, rcMgr, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcListing, rcSelf, rcNull );
        else
        {
            rc = VLinkerListExternalSchemaModules ( self -> linker, listp );
            if ( rc == 0 )
                return 0;
        }

        * listp = NULL;
    }

    return rc;
}


/* PathType
 *  check the path type of an object/directory path.
 *
 *  this is an extension of the KDirectoryPathType ( see <kdb/manager.h> )
 *  and will return the KDirectory values if a path type is not specifically
 *  a VDB or KDB object.
 */
LIB_EXPORT int CC VDBManagerPathType ( const VDBManager * self,
    const char *path, ... )
{
    int type;

    va_list args;
    va_start ( args, path );

    type = VDBManagerVPathType ( self, path, args );

    va_end ( args );

    return type;
}

LIB_EXPORT int CC VDBManagerVPathType ( const VDBManager * self,
    const char *path, va_list args )
{
    if ( self != NULL )
        return KDBManagerVPathType ( self -> kmgr, path, args );

    return kptBadPath;
}

static int CC VDBManagerVPathTypeUnreliable ( const VDBManager * self,
    const char *path, va_list args )
{
    if ( self != NULL )
        return KDBManagerVPathTypeUnreliable ( self -> kmgr, path, args );

    return kptBadPath;
}

LIB_EXPORT int CC VDBManagerPathTypeUnreliable ( const VDBManager * self,
    const char *path, ... )
{
    int type;

    va_list args;
    va_start ( args, path );

    type = VDBManagerVPathTypeUnreliable ( self, path, args );

    va_end ( args );

    return type;
}

/** Reset VResolver to set protected repository context */
LIB_EXPORT rc_t CC VDBManagerSetResolver
    ( const VDBManager * self, struct VResolver * resolver )
{
    const KDBManager * kbd = NULL;
    rc_t rc = VDBManagerGetKDBManagerRead ( self, & kbd );
    if (rc == 0) {
        VFSManager * vfs = NULL;
        rc = KDBManagerGetVFSManager ( kbd, & vfs );
        if (rc == 0) {
            rc = VFSManagerSetResolver ( vfs, resolver );

            VFSManagerRelease ( vfs );
            vfs = NULL;
        }

        KDBManagerRelease ( kbd );
        kbd = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VDBManagerGetCacheRoot ( const struct VDBManager * self,
    struct VPath const ** path )
{
    rc_t rc;
    if ( path == NULL )
        rc = RC ( rcVDB, rcMgr, rcListing, rcParam, rcNull );
    else
    {
        * path = NULL;
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcListing, rcSelf, rcNull );
        else
        {
            const KDBManager * kbd = NULL;
            rc = VDBManagerGetKDBManagerRead ( self, & kbd );
            if ( rc == 0 )
            {
                VFSManager * vfs = NULL;
                rc = KDBManagerGetVFSManager ( kbd, & vfs );
                if ( rc == 0 )
                {
                    rc = VFSManagerGetCacheRoot ( vfs, path );
                    VFSManagerRelease ( vfs );
                }
                KDBManagerRelease ( kbd );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VDBManagerSetCacheRoot ( const struct VDBManager * self,
    struct VPath const * path )
{
    rc_t rc;
    if ( path == NULL )
        rc = RC ( rcVDB, rcMgr, rcSelecting, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcVDB, rcMgr, rcSelecting, rcSelf, rcNull );
    else
    {
        const KDBManager * kbd = NULL;
        rc = VDBManagerGetKDBManagerRead ( self, & kbd );
        if ( rc == 0 )
        {
            VFSManager * vfs = NULL;
            rc = KDBManagerGetVFSManager ( kbd, & vfs );
            if ( rc == 0 )
            {
                rc = VFSManagerSetCacheRoot ( vfs, path );
                VFSManagerRelease ( vfs );
            }
            KDBManagerRelease ( kbd );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VDBManagerDeleteCacheOlderThan ( const VDBManager * self,
    uint32_t days )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcVDB, rcMgr, rcSelecting, rcSelf, rcNull );
    else
    {
        const KDBManager * kbd = NULL;
        rc = VDBManagerGetKDBManagerRead ( self, & kbd );
        if ( rc == 0 )
        {
            VFSManager * vfs = NULL;
            rc = KDBManagerGetVFSManager ( kbd, & vfs );
            if ( rc == 0 )
            {
                rc = VFSManagerDeleteCacheOlderThan ( vfs, days );
                VFSManagerRelease ( vfs );
            }
            KDBManagerRelease ( kbd );
        }
    }
    return rc;
}
