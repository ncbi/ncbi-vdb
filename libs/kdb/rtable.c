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

#include "rtable.h"

#include <kdb/kdb-priv.h>

#include "rdbmgr.h"
#include "database-cmn.h"
#include "kdb-cmn.h"
#include "rkdb.h"
#include "rcolumn.h"
#include "rindex.h"
#include "rmeta.h"

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/rc.h>

#include <kfs/arc.h>

#include <stdio.h>

/*--------------------------------------------------------------------------
 * KTable
 *  a collection of columns indexed by row id, metadata, indices
 */

static rc_t KRTableWhack ( KTable *self );
static bool CC KRTableLocked ( const KTable *self );
static bool CC KRTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args );
static bool CC KRTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KRTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args );
static rc_t CC KRTableOpenManagerRead ( const KTable *self, const KDBManager **mgr );
static rc_t CC KRTableOpenParentRead ( const KTable *self, const KDatabase **db );
static bool CC KRTableHasRemoteData ( const KTable *self );
static rc_t CC KRTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir );
static rc_t CC KRTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args );
static rc_t CC KRTableOpenMetadataRead ( const KTable *self, const KMetadata **metap );
static rc_t CC KRTableVOpenIndexRead ( const KTable *self, const KIndex **idxp, const char *name, va_list args );
static rc_t CC KRTableGetPath ( const KTable *self, const char **path );
static rc_t CC KRTableGetName (KTable const *self, char const **rslt);
static rc_t CC KRTableListCol ( const KTable *self, KNamelist **names );
static rc_t CC KRTableListIdx ( const KTable *self, KNamelist **names );
static rc_t CC KRTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal );

static KTable_vt KRTable_vt =
{
    KRTableWhack,
    KTableBaseAddRef,
    KTableBaseRelease,
    KRTableLocked,
    KRTableVExists,
    KRTableIsAlias,
    KRTableVWritable,
    KRTableOpenManagerRead,
    KRTableOpenParentRead,
    KRTableHasRemoteData,
    KRTableOpenDirectoryRead,
    KRTableVOpenColumnRead,
    KRTableOpenMetadataRead,
    KRTableVOpenIndexRead,
    KRTableGetPath,
    KRTableGetName,
    KRTableListCol,
    KRTableListIdx,
    KRTableMetaCompare
};

#define CAST() assert( bself->vt == &KRTable_vt ); KRTable * self = (KRTable *)bself

/* GetPath
 *  return the absolute path to table
 */
static
rc_t CC
KRTableGetPath ( const KTable *bself, const char **path )
{
    CAST();

    if ( path == NULL )
        return RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );

    *path = self->path;

    return 0;
}

/* Whack
 */
static
rc_t
KRTableWhack ( KTable *bself )
{
    CAST();

    rc_t rc = 0;

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc == 0 )
            self -> db = NULL;
    }

    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        return KTableBaseWhack( bself );
    }

    KRefcountInit ( & bself -> refcount, 1, "KTable", "whack", "ktbl" );

    return rc;
}

static
rc_t CC
KRTableGetName(KTable const *bself, char const **rslt)
{
    CAST();

    char *sep;

    *rslt = self->path;
    sep = strrchr(self->path, '/');
    if (sep != NULL)
        *rslt = sep + 1;
    return 0;
}

/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
rc_t KRTableMake ( const KRTable **tblp, const KDirectory *dir, const char *path, const KDBManager * mgr, bool prerelease  )
{
    KRTable *tbl;

    assert ( tblp != NULL );
    assert ( path != NULL );

    size_t path_size = strlen(path);
    tbl = malloc ( sizeof * tbl + path_size);
    if ( tbl == NULL )
    {
        * tblp = NULL;
        return RC ( rcDB, rcTable, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( tbl, 0, sizeof * tbl );
    tbl -> dad . vt = & KRTable_vt;
    KRefcountInit ( & tbl -> dad . refcount, 1, "KTable", "make", path );
    tbl -> dir = dir;
    string_copy ( tbl -> path, path_size + 1, path, path_size );

    /* YES,
      DBG_VFS should be used here to be printed along with other VFS messages */
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE), ("Making KTable '%s'\n", path));

    tbl -> mgr = KDBManagerAttach ( mgr );
    tbl -> prerelease = prerelease;

    * tblp = tbl;
    return 0;
}


/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KRTableLocked ( const KTable *bself )
{
    CAST();

    rc_t rc = KDBRWritable ( self -> dir, "." );
    return GetRCState ( rc ) == rcLocked;
}


/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
bool CC
KRTableVExists ( const KTable *bself, uint32_t type, const char *name, va_list args )
{
    CAST();

    if ( name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptIndex:
            ns = "idx";
            break;

        case kptColumn:
            ns = "col";
            break;

        default:
            return false;
        }

        rc = KDBVMakeSubPath ( self -> dir, path, sizeof path, ns, 3, name, args );
        if ( rc == 0 )
        {
            switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
            {
            case kptFile:
            case kptDir:
            case kptFile | kptAlias:
            case kptDir | kptAlias:
                return true;
            }
        }
    }
    return false;
}

/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name,
 *
 *  "name" [ IN ] - NUL terminated object name
 */
static
bool CC
KRTableIsAlias ( const KTable *bself, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    CAST();

    if ( name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptIndex:
            ns = "idx";
            break;

        case kptColumn:
            ns = "col";
            break;

        default:
            return false;
        }

        rc = KDBMakeSubPath ( self -> dir, path, sizeof path, ns, 3, name );
        if ( rc == 0 )
        {
            switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
            {
            case kptFile | kptAlias:
            case kptDir | kptAlias:
                if ( resolved != NULL && rsize != 0 )
                {
                    const KDirectory *sub;
                    rc = KDirectoryOpenDirRead ( self -> dir, & sub, false, "%s", ns );
                    * resolved = 0;
                    if ( rc != 0 )
                        PLOGERR ( klogWarn, ( klogWarn, rc, "failed to open subdirectory '$(sub)'", "sub=%s", ns ) );
                    else
                    {
                        rc = KDirectoryResolveAlias ( sub, false, resolved, rsize, "%s", name );
                        KDirectoryRelease ( sub );
                        if ( rc != 0 )
                            PLOGERR ( klogWarn, ( klogWarn, rc, "failed to resolve alias '$(alias)'", "alias=%s", name ) );
                    }
                }
                return true;
            }
        }
    }

    if ( resolved != NULL && rsize != 0 )
        * resolved = 0;

    return false;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
rc_t CC
KRTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return (rc_t) - 1;
}

/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC
KRTableOpenManagerRead ( const KTable *bself, const KDBManager **mgr )
{
    CAST();

    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KDBManagerAddRef ( self -> mgr );
        if ( rc == 0 )
        {
            * mgr = self -> mgr;
            return 0;
        }
    }

    return rc;
}


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
static
rc_t CC
KRTableOpenParentRead ( const KTable *bself, const KDatabase **db )
{
    CAST();

    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KDatabaseAddRef ( self -> db );
        if ( rc == 0 )
        {
            * db = self -> db;
            return 0;
        }
    }

    return rc;
}


/* GetDirectory
 *  access the directory in use
 */
static
rc_t CC
KRTableOpenDirectoryRead ( const KTable *bself, const KDirectory **dir )
{
    CAST();

    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        * dir = self -> dir;
        return KDirectoryAddRef ( * dir );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KNamelist
 */

/* List
 *  create table listings
 */
struct FilterData
{
    const KDBManager * mgr;
    int type;
};

static
bool CC KDatabaseListFilter ( const KDirectory *dir, const char *name, void *data_ )
{
    struct FilterData * data = data_;
    return ( KDBManagerOpenPathTypeRead ( data->mgr, dir, name,
        NULL, data->type, NULL, false, NULL ) == 0 );
}

static
rc_t CC
KRTableListCol ( const KTable *bself, KNamelist **names )
{
    CAST();

    struct FilterData data;

    data.mgr = self->mgr;
    data.type = kptColumn;

    return KDirectoryList ( self -> dir,
        names, KDatabaseListFilter, &data, "col" );
}

static
bool CC
KTableListIdxFilter ( const KDirectory *dir, const char *name, void *data )
{
    const size_t sz = strlen(name);

    if (sz > 4 && strcmp(&name[sz - 4], ".md5") == 0)
        return false;
    return true;
}

static
bool CC
KTableListSkeyFilter ( const KDirectory *dir, const char *name, void *data )
{
    if ( strcmp ( name, "skey" ) == 0 )
        return true;
    return false;
}

static
rc_t CC
KRTableListIdx ( const KTable *bself, KNamelist **names )
{
    CAST();

    if ( ! self -> prerelease )
    {
        return KDirectoryList ( self -> dir,
            names, KTableListIdxFilter, NULL, "idx" );
    }

    return KDirectoryList ( self -> dir,
        names, KTableListSkeyFilter, NULL, "." );
}

/* HasRemoteData
 *  indicates whether some/all table data comes from network resource
 *  such as HttpFile or CacheteeFile
 */
static
bool CC
KRTableHasRemoteData ( const KTable *bself )
{
    CAST();

    bool result = KDirectoryIsKArcDir ( self -> dir ) &&
            KArcDirIsFromRemote ( (const KArcDir * ) self -> dir );
    return result;
}

static
rc_t CC
KRTableVOpenColumnRead ( const KTable *bself, const KColumn **colp, const char *name, va_list args )
{
    CAST();

    rc_t rc;
    char path [ 256 ];

    if ( colp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * colp = NULL;

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "col", 3, name, args );
    if ( rc == 0 )
    {
        rc = KDBRManagerVOpenColumnReadInt_noargs ( self -> mgr,
                                           colp, self -> dir, false, path );
        if ( rc == 0 )
        {
            KRColumn *col = ( KRColumn* ) * colp;
            col -> tbl = KTableAttach ( bself );
        }
    }
    return rc;
}

static
rc_t CC
KRTableOpenMetadataRead ( const KTable *bself, const KMetadata **metap )
{
    CAST();

    rc_t rc;
    KRMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBRManagerOpenMetadataReadInt ( self -> mgr, & meta,
        self -> dir, 0, self -> prerelease );
    if ( rc == 0 )
    {
        meta -> tbl = KTableAttach ( bself );
        * metap = & meta -> dad;
    }

    return rc;
}

/* KTableMetaCompare
 *  test if 2 tables have the same MetaDataNode ( and content ) for a given path
 */
/* >>>>>> !!! any changes here have to be duplicated in wtable.c !!! <<<<<< */
static
rc_t CC
KRTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal )
{
    rc_t rc = 0;
    if ( NULL == other || NULL == path || NULL == equal ) {
        rc = RC ( rcDB, rcTable, rcComparing, rcParam, rcNull );
    } else {
        const KMetadata *self_meta;
        rc = KTableOpenMetadataRead( self, &self_meta );
        if ( 0 == rc ) {
            const KMetadata *other_meta;
            rc = KTableOpenMetadataRead( other, &other_meta );
            if ( 0 == rc ) {
                const KMDataNode * self_node;
                rc = KMetadataOpenNodeRead( self_meta, &self_node, path );
                if ( 0 == rc ) {
                    const KMDataNode * other_node;
                    rc = KMetadataOpenNodeRead( other_meta, &other_node, path );
                    if ( 0 == rc ) {
                        rc = KMDataNodeCompare( self_node, other_node, equal );
                        KMDataNodeRelease( other_node );
                    }
                    KMDataNodeRelease( self_node );
                }
                KMetadataRelease( other_meta );
            }
            KMetadataRelease( self_meta );
        }
    }
    return rc;
}

static
rc_t CC
KRTableVOpenIndexRead ( const KTable *bself, const KIndex **idxp, const char *name, va_list args )
{
    CAST();

    rc_t rc = 0;
    char path [ 256 ];

    if ( idxp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self -> prerelease )
    {
        int len = 0;
        /* VDB-4386: cannot treat va_list as a pointer! */
        /*if ( args == 0 )
            len = snprintf ( path, sizeof path, "%s", name );
        else*/
        if ( name != NULL )
            len = vsnprintf ( path, sizeof path, name, args );
        else
            path[0] = '\0';
        if ( len < 0 || ( size_t ) len >= sizeof path )
            return RC ( rcDB, rcTable, rcOpening, rcPath, rcExcessive );
    }
    else
    {
        rc = KDBVMakeSubPath ( self -> dir,
            path, sizeof path, "idx", 3, name, args );
    }

    if ( rc == 0 )
    {
        KRIndex *idx;
        rc = KDBRManagerOpenIndexReadInt ( self -> mgr, & idx, self -> dir, path );
        if ( rc == 0 )
        {
            idx -> tbl = ( const KRTable * ) KTableAttach ( bself );
            * idxp = & idx -> dad;
        }
    }
    return rc;
}


