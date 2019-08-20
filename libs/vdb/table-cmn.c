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

#define KONST const
#include "table-priv.h"
#include "cursor-table.h"
#include "database-priv.h"
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "schema-dump.h"
#include "linker-priv.h"

#include <vdb/vdb-priv.h>
#include <vdb/cursor.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/table.h>
#include <kdb/index.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>
#include <klib/symbol.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>

#include "../kdb/table-priv.h" /* KTableGetName */

#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VTable
 *  a collection of columns indexed by row id, metadata, indices
 */

/* Whack
 */
rc_t VTableWhack ( VTable *self )
{
    KRefcountWhack ( & self -> refcount, "VTable" );

    BSTreeWhack ( & self -> read_col_cache, VColumnRefWhack, NULL );
    BSTreeWhack ( & self -> write_col_cache, VColumnRefWhack, NULL );
    VTableRelease(self -> cache_tbl);

    KMDataNodeRelease ( self -> col_node );
    KMetadataRelease ( self -> meta );
    KTableRelease ( self -> ktbl );
    VSchemaRelease ( self -> schema );
    VLinkerRelease ( self -> linker );
    VDatabaseSever ( self -> db );
    VDBManagerSever ( self -> mgr );

    free ( self );
    return 0;
}


/* AddRef
 */
LIB_EXPORT rc_t CC VTableAddRef ( const VTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VTable" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcTable, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VTableRelease ( const VTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VTable" ) )
        {
        case krefWhack:
            return VTableWhack ( ( VTable* ) self );
        case krefNegative:
            return RC ( rcVDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 *  internal
 */
VTable *VTableAttach ( const VTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VTable" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VTable* ) self;
}

rc_t VTableSever ( const VTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VTable" ) )
        {
        case krefWhack:
            return VTableWhack ( ( VTable* ) self );
        case krefNegative:
            return RC ( rcVDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make - PRIVATE
 */
rc_t VTableMake ( VTable **tblp,
    const VDBManager *mgr, const VDatabase *db, const VSchema *schema )
{
    rc_t rc;

    /* create a structure */
    VTable *tbl = calloc ( sizeof * tbl, 1 );
    if ( tbl == NULL )
        rc = RC ( rcVDB, rcTable, rcConstructing, rcMemory, rcExhausted );
    else
    {
        /* since the object has been zeroed, Whack will work properly */
        rc = VSchemaMake ( ( VSchema** ) & tbl -> schema, schema );
        if ( rc == 0 )
        {
            tbl -> mgr = VDBManagerAttach ( mgr );
            tbl -> db = VDatabaseAttach ( db );

            BSTreeInit ( & tbl -> read_col_cache );
            BSTreeInit ( & tbl -> write_col_cache );

            KRefcountInit ( & tbl -> refcount, 1, "VTable", "make", "vtbl" );

            tbl -> cmode = ( uint8_t ) kcmOpen;
            tbl -> checksum = ( uint8_t ) kcsNone;

            rc = VLinkerMake ( & tbl -> linker, mgr -> linker, mgr -> linker -> dl );
            if ( rc == 0 )
            {
                * tblp = tbl;
                return 0;
            }
        }

        VTableWhack ( tbl );
    }

    * tblp = NULL;

    return rc;
}


/* OpenRead
 *  finish initialization on open for read
 */
static
rc_t VTableOpenRead ( VTable *self )
{
    /* open metadata */
    rc_t rc = KTableOpenMetadataRead ( self -> ktbl, & self -> meta );
    if ( rc == 0 )
    {
        /* open "col" node, if present */
        rc = KMetadataOpenNodeRead ( self -> meta, & self -> col_node, "col" );
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
        {
            /* fetch stored schema */
            rc = VTableLoadSchema ( self );
            if ( rc == 0 && self -> stbl == NULL ) {
                const char * path = NULL;
                KTableGetName ( self -> ktbl, & path );
                rc = RC ( rcVDB, rcTable, rcOpening, rcSchema, rcNotFound );
                PLOGERR ( klogErr, ( klogErr, rc,
		    "Format of your Run File is obsolete.\n"
		    "Please download the latest version of Run '$(path)'",
		    "path=%s", path ) );
            }
        }
    }

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB), ("VTableOpenRead = %d\n", rc));

    return rc;
}


/* OpenTableRead
 * VOpenTableRead
 *  open a table for read
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
LIB_EXPORT rc_t CC VDBManagerOpenTableReadVPath ( const VDBManager *self,
    const VTable **tblp, const VSchema *schema,
    const struct VPath *path )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            VTable *tbl;

            /* if no schema is given, always pass intrinsic */
            if ( schema == NULL )
                schema = self -> schema;

            rc = VTableMake ( & tbl, self, NULL, schema );
            if ( rc == 0 )
            {
                tbl -> read_only = true;
                rc = KDBManagerOpenTableReadVPath( self -> kmgr, & tbl -> ktbl, path );
                if ( rc == 0 )
                {
                    tbl -> blob_validation = KTableHasRemoteData ( tbl -> ktbl );
                    rc = VTableOpenRead ( tbl );
                    if ( rc == 0 )
                    {
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        * tblp = tbl;
                        return 0;
                    }
                }
                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VDBManagerVOpenTableRead ( const VDBManager *self,
    const VTable **tblp, const VSchema *schema,
    const char *path, va_list args )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            VTable *tbl;

            /* if no schema is given, always pass intrinsic */
            if ( schema == NULL )
                schema = self -> schema;

            rc = VTableMake ( & tbl, self, NULL, schema );
            if ( rc == 0 )
            {
                tbl -> read_only = true;
                rc = KDBManagerVOpenTableRead ( self -> kmgr, & tbl -> ktbl, path, args );
                if ( rc == 0 )
                {
                    tbl -> blob_validation = KTableHasRemoteData ( tbl -> ktbl );
                    rc = VTableOpenRead ( tbl );
                    if ( rc == 0 )
                    {
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        * tblp = tbl;
                        return 0;
                    }
                }
                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDBManagerOpenTableRead ( const VDBManager *self, const VTable **tbl,
    const VSchema *schema, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVOpenTableRead ( self, tbl, schema, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVOpenTableRead ( const VDatabase *self,
    const VTable **tblp, const char *path, va_list args )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSelf, rcNull );
        else
        {
            VTable *tbl;
            rc = VTableMake ( & tbl, self -> mgr, self, self -> schema );
            if ( rc == 0 )
            {
                tbl -> read_only = true;

                rc = KDatabaseVOpenTableRead ( self -> kdb, & tbl -> ktbl, path, args );
                if ( rc == 0 )
                {
                    tbl -> blob_validation = KTableHasRemoteData ( tbl -> ktbl );
                    rc = VTableOpenRead ( tbl );
                    if ( rc == 0 )
                    {
                        * tblp = tbl;
                        return 0;
                    }
                }

                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseOpenTableRead ( const VDatabase *self,
    const VTable **tbl, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDatabaseVOpenTableRead ( self, tbl, path, args );
    va_end ( args );

    if ( rc == 0 && self->cache_db != NULL )
    {
        rc_t rc2;
        const VTable *ctbl;

        va_start ( args, path );
        rc2 = VDatabaseVOpenTableRead ( self->cache_db, &ctbl, path, args );
        va_end ( args );

        DBGMSG( DBG_VDB, DBG_FLAG( DBG_VDB_VDB ), ( "VDatabaseOpenTableRead(vdbcache) = %d\n", rc2 ) );
        if ( rc2 == 0 )
        {
            ( ( VTable* ) ( *tbl ) ) -> cache_tbl = ctbl;
        }
    }
    return rc;
}


/* Locked
 *  returns true if locked
 */
LIB_EXPORT bool CC VTableLocked ( const VTable *self )
{
    if ( self != NULL )
        return KTableLocked ( self -> ktbl );
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
LIB_EXPORT rc_t CC VTableVWritable ( const VTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
    else
        rc = KTableVWritable ( self -> ktbl, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VTableWritable ( const VTable *self, uint32_t type, const char * name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VTableVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* OpenMetadataRead
 *  opens metadata file
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
LIB_EXPORT rc_t CC VTableOpenMetadataRead ( const VTable *self, struct KMetadata const **meta )
{
    rc_t rc;

    if ( meta == NULL )
        rc = RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
        else
        {
            * meta = self -> meta;
            return KMetadataAddRef ( self -> meta );
        }

        * meta = NULL;
    }

    return rc;
}


/* OpenIndexRead
 * VOpenIndexRead
 *  open an index for read
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
LIB_EXPORT rc_t CC VTableVOpenIndexRead ( const VTable *self,
    struct KIndex const **idx, const char *name, va_list args )
{
    rc_t rc;
    KIdxType type;
    uint32_t version;
    const KMDataNode *node;

    if ( self == NULL )
    {
        if ( idx == NULL )
            return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
        * idx = NULL;
        return RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }

    /* open the requested index */
    rc = KTableVOpenIndexRead ( self -> ktbl, idx, name, args );
    if ( rc != 0 )
        return rc;

    /* get version and type */
    rc = KIndexVersion ( * idx, & version );
    if ( rc == 0 )
        rc = KIndexType ( * idx, & type );
    if ( rc != 0 )
        return 0;

    /* look for versions 2 or 3, type kitText or kitText with projection */
    switch ( version )
    {
    case 2:
    case 3:
        switch ( type )
        {
        case kitText:
        case kitText | kitProj:
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }

    /* look for a node in metadata for repairing maximum id */
    rc = KMetadataOpenNodeRead ( self -> meta, & node, "/.seq/spot" );
    if ( rc == 0 )
    {
        int64_t maxid;
        rc = KMDataNodeReadAsI64 ( node, & maxid );
        if ( rc == 0 )
            KIndexSetMaxRowId ( * idx, maxid );
        KMDataNodeRelease ( node );
    }

    return 0;
}

LIB_EXPORT rc_t CC VTableOpenIndexRead ( const VTable *self,
    struct KIndex const **idx, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VTableVOpenIndexRead ( self, idx, name, args );
    va_end ( args );

    return rc;
}


/* ListColumns
 *  list readable column names
 *
 *  "names" [ OUT ] - return parameter for namelist
 *
 *  availability: v2.1
 */
static
rc_t list_readable_columns ( const VTable *cself )
{
    VTable *self = ( VTable* ) cself;

    VTableCursor *curs;
    rc_t rc = VTableCreateCursorReadInternal ( self, (const VTableCursor **) & curs );
    if (  rc == 0 )
    {
        /* let this private VCursor-function list the columns */
        rc = VCursorListReadableColumns ( curs, & self -> read_col_cache );
        VCursorRelease ( ( VCursor * ) curs );
        if ( rc == 0 )
            self -> read_col_cache_valid = true;
    }

    return rc;
}

rc_t make_column_namelist ( const BSTree *columns, KNamelist **names )
{
    VNamelist *list;
    rc_t rc = VNamelistMake ( & list, 8 );
    if ( rc == 0 )
    {
        const String *last = NULL;
        const VColumnRef *cref = ( const VColumnRef* )
            BSTreeFirst ( columns );
        while ( cref != NULL )
        {
            if ( last == NULL || ! StringEqual ( last, & cref -> name ) )
            {
                rc = VNamelistAppend ( list, cref -> name . addr );
                if ( rc != 0 )
                    break;

                last = & cref -> name;
            }

            cref = ( const VColumnRef* )
                BSTNodeNext ( & cref -> n );
        }

        if ( rc == 0 )
            rc = VNamelistToNamelist ( list, names );

        VNamelistRelease ( list );
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableListReadableColumns ( const VTable *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else
        {
            if ( ! self -> read_col_cache_valid )
                rc = list_readable_columns ( self );

            if ( self -> read_col_cache_valid )
                return make_column_namelist ( & self -> read_col_cache, names );
        }

        * names = NULL;
    }
    return rc;
}

/* v2.0 interface
 */
LIB_EXPORT rc_t CC VTableListCol ( const VTable *cself, KNamelist **names )
{
    return VTableListReadableColumns ( cself, names );
}



/* ListPhysColumns
 *  avail: 2.4
 */
LIB_EXPORT rc_t CC VTableListPhysColumns ( const VTable *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        * names = NULL;

        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else
        {
            KNamelist *kcol_names;
            rc = KTableListCol ( self -> ktbl, & kcol_names );
            if ( rc == 0 )
            {
                uint32_t kcol_count;
                rc = KNamelistCount ( kcol_names, & kcol_count );
                if ( rc == 0 )
                {
                    uint32_t scol_count = 0;
                    KNamelist *scol_names = NULL;
                    const KMDataNode *col_node = self -> col_node;

#if LAZY_OPEN_COL_NODE
                    if ( col_node == NULL )
                    {
                        rc = KMetadataOpenNodeRead ( self -> meta, & ( ( VTable* ) self ) -> col_node, "col" );
                        if ( rc == 0 || GetRCState ( rc ) != rcNotFound )
                            col_node = self -> col_node;
                    }
#endif
                    if ( col_node != NULL )
                    {
                        rc = KMDataNodeListChildren ( col_node, & scol_names );
                        if ( rc == 0 )
                            rc = KNamelistCount ( scol_names, & scol_count );
                    }

                    if ( rc == 0 )
                    {
                        VNamelist *vnames;
                        rc = VNamelistMake ( & vnames, kcol_count + scol_count );
                        if ( rc == 0 )
                        {
                            uint32_t i;
                            const char *name;

                            for ( i = 0; i < kcol_count && rc == 0; ++ i )
                            {
                                rc = KNamelistGet ( kcol_names, i, & name );
                                if ( rc == 0 )
                                    rc = VNamelistAppend ( vnames, name );
                            }

                            for ( i = 0; i < scol_count && rc == 0; ++ i )
                            {
                                rc = KNamelistGet ( scol_names, i, & name );
                                if ( rc == 0 )
                                    rc = VNamelistAppend ( vnames, name );
                            }

                            if ( rc == 0 )
                            {
                                rc = VNamelistToNamelist ( vnames, names );
                                if ( rc == 0 )
                                    VNamelistReorder ( vnames, false );
                            }
                        }

                        VNamelistRelease ( vnames );
                    }

                    KNamelistRelease ( scol_names );
                }

                KNamelistRelease ( kcol_names );
            }
        }
    }

    return rc;
}


/* ListDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
 *  into "typedecls" of the default datatype for the named column
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 *
 *  availability: v2.1
 */

rc_t make_column_typelist ( const BSTree *columns,
    const char *col, uint32_t *dflt_idx, KNamelist **typedecls )
{
    VNamelist *list;
    rc_t rc = VNamelistMake ( & list, 8 );
    if ( rc == 0 )
    {
        uint32_t idx;
        const VColumnRef *first;

        String col_name;
        StringInitCString ( & col_name, col );

        first = ( const VColumnRef* )
            BSTreeFind ( columns, & col_name, VColumnRefCmpString );
        if ( first != NULL )
        {
            const VColumnRef *cref = ( const VColumnRef* ) BSTNodePrev ( & first -> n );
            while ( cref != NULL && StringEqual ( & first -> name, & cref -> name ) )
            {
                first = cref;
                cref = ( const VColumnRef* ) BSTNodePrev ( & cref -> n );
            }

            for ( cref = first, idx = 0; ; ++ idx )
            {
                rc = VNamelistAppend ( list, cref -> typedecl );
                if ( rc != 0 )
                    break;

                if ( cref -> dflt )
                    * dflt_idx = idx;

                cref = ( const VColumnRef* ) BSTNodeNext ( & cref -> n );
                if ( cref == NULL || ! StringEqual ( & first -> name, & cref -> name ) )
                    break;
            }
        }

        if ( rc == 0 )
            rc = VNamelistToNamelist ( list, typedecls );

        VNamelistRelease ( list );
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableListReadableDatatypes ( const VTable *self,
    const char *col, uint32_t *dflt_idx, KNamelist **typedecls )
{
    rc_t rc;

    uint32_t dummy;
    if ( dflt_idx == NULL )
        dflt_idx = & dummy;

    * dflt_idx = 0;

    if ( typedecls == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else if ( col == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcName, rcNull );
        else if ( col [ 0 ] == 0 )
            rc = RC ( rcVDB, rcTable, rcListing, rcName, rcEmpty );
        else
        {
            if ( ! self -> read_col_cache_valid )
                rc = list_readable_columns ( self );

            if ( self -> read_col_cache_valid )
                return make_column_typelist ( & self -> read_col_cache, col, dflt_idx, typedecls );
        }

        * typedecls = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableColumnDatatypes( const VTable *cself, const char *col,
    uint32_t *dflt_idx, KNamelist **typedecls )
{
    return VTableListReadableDatatypes ( cself, col, dflt_idx, typedecls );
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VTableOpenManagerRead ( const VTable *self, const VDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT rc_t CC VTableOpenParentRead ( const VTable *self, const VDatabase **db )
{
    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VDatabaseAddRef ( self -> db );
            if ( rc == 0 )
            {
                * db = self -> db;
                return 0;
            }
        }

        * db = NULL;
    }

    return rc;
}


/* OpenSchema
 *  duplicate reference to table schema
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VTableOpenSchema ( const VTable *self, const VSchema **schema )
{
    rc_t rc;

    if ( schema == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
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


/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 */
LIB_EXPORT rc_t CC VTableGetUserData ( const VTable *self, void **data )
{
    rc_t rc;

    if ( data == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            * data = self -> user;
            return 0;
        }

        * data = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableSetUserData ( const VTable *cself,
    void *data, void ( CC * destroy ) ( void *data ) )
{
    VTable *self = ( VTable* ) cself;
    if ( cself == NULL )
        return RC ( rcVDB, rcTable, rcUpdating, rcSelf, rcNull );

    self -> user = data;
    self -> user_whack = destroy;

    return 0;
}


/* GetKTable
 *  returns a new reference to underlying KTable
 */
LIB_EXPORT rc_t CC VTableGetKTableRead ( const VTable *self, const KTable **ktbl )
{
    rc_t rc;

    if ( ktbl == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            * ktbl = self -> ktbl;
            return KTableAddRef ( * ktbl );
        }

        * ktbl = NULL;
    }

    return rc;
}



/* Typespec
 *  retrieve table typespec
 *
 *  AVAILABILITY: version 2.1
 *
 *  "ts_buff" [ IN ] and "ts_size" [ IN ] - buffer for return of NUL-terminated
 *  table typespec, e.g. 'NCBI:tbl:abc#1.1'
 */
#define USE_SDUMPER 0

#if USE_SDUMPER
typedef struct flush_tblspec_pb flush_tblspec_pb;
struct flush_tblspec_pb
{
    char *buff;
    size_t size;
    size_t bytes;
};

static
rc_t CC flush_tbl_typespec ( void *data, const void *buffer, size_t size )
{
    flush_tblspec_pb *pb = data;
    if ( pb -> bytes + size >= pb -> size )
        return RC ( rcVDB, rcTable, rcAccessing, rcBuffer, rcInsufficient );
    memmove ( & pb -> buff [ pb -> bytes ], buffer, size );
    pb -> bytes += size;
    pb -> buff [ pb -> bytes ] = 0;
    return 0;
}
#endif

LIB_EXPORT rc_t CC VTableTypespec ( const VTable *self, char *ts_buff, size_t ts_size )
{
    rc_t rc = 0;

    if ( ts_size == 0 )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcBuffer, rcInsufficient );
    else if ( ts_buff == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcBuffer, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            rc_t rc2;
#if USE_SDUMPER
            SDumper dumper;
            flush_tblspec_pb pb;

            /* initialize pb */
            pb . buff = ts_buff;
            pb . size = ts_size;
            pb . bytes = 0;

            /* initialize dumper for extracting name */
            SDumperInit ( & dumper, self -> schema, sdmCompact, flush_tbl_typespec, & pb );

            /* dump only the table name */
            assert ( self -> stbl != NULL );
            rc = SDumperPrint ( & dumper, "%N%V", self -> stbl -> name, self -> stbl -> version );

            /* done */
            rc2 = SDumperWhack ( & dumper );
            if ( rc == 0 )
            {
                if ( rc2 == 0 )
                    return 0;

                rc = rc2;
            }
#else
            rc2 = string_printf ( ts_buff, ts_size, NULL, "%N#%V", self -> stbl -> name, self -> stbl -> version );
            if ( rc2 == 0 )
                return 0;
#endif
        }

        ts_buff [ 0 ] = 0;
    }

    return rc;
}


/* HasStaticColumn - PRIVATE
 *  given a physical column name, report whether it exists
 *  and is ( currently ) static
 *  avail: 2.5
 */
LIB_EXPORT bool CC VTableHasStaticColumn ( const VTable *self, const char *name )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        /* a full check of this might include testing that the column
           can be opened for read or write, but the API is purposely
           vague on what this means.

           currently the meaning of this ( PRIVATE ) API is whether
           there is a metadata column entry with this name - period. */
        const KMDataNode *node;
        rc_t rc = KMetadataOpenNodeRead ( self -> meta, & node, "/col/%s", name );
        if ( rc == 0 )
        {
            KMDataNodeRelease ( node );
            return true;
        }
    }

    return false;
}
LIB_EXPORT bool CC VTableVHasStaticColumn ( const VTable *self, const char *name, va_list args )
{
    char full[4096];
    if ( name == NULL )
        full [ 0 ] = 0;
    else
    {
        int len;

        /* generate full name */
        if ( args == NULL )
            len = snprintf ( full, sizeof full, "%s", name );
        else
            len = vsnprintf ( full, sizeof full, name, args );
        if ( len < 0 || len >= sizeof full )
        {
            rc_t rc = RC ( rcVDB, rcColumn, rcOpening, rcName, rcExcessive );
            LOGERR ( klogInt, rc, "failed to format column name" );
            return false;
        }
    }
    return VTableHasStaticColumn( self, full );
}


/*--------------------------------------------------------------------------
 * VColumnRef
 *  cache entry
 */

/* Whack
 */
void CC VColumnRefWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}


/* Make
 *  make a column ref from an SColumn*
 */
rc_t VColumnRefMake ( VColumnRef **rp, const VSchema *schema, const SColumn *scol )
{
    char text [ 256 ];
    rc_t rc = VTypedeclToText ( & scol -> td, schema, text, sizeof text );
    if ( rc == 0 )
    {
        size_t tdsize = strlen ( text );
        const String *name = & scol -> name -> name;
        VColumnRef *cref = malloc ( sizeof * cref + name -> size + tdsize );
        if ( cref == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcMemory, rcExhausted );
        else
        {
            strcpy ( cref -> typedecl, text );
            string_copy ( & cref -> typedecl [ tdsize + 1 ], name -> size + 1, name -> addr, name -> size );
            StringInit ( & cref -> name, & cref -> typedecl [ tdsize + 1 ], name -> size, name -> len );
            cref -> td = scol -> td;
            cref -> cid = scol -> cid;
            cref -> dflt = scol -> dflt;

            * rp = cref;
        }
    }

    return rc;
}


/* CmpString
 *  compares by name only
 *  "item" is a const String*
 */
int64_t CC VColumnRefCmpString ( const void *item, const BSTNode *n )
{
    const VColumnRef *cref = ( const VColumnRef* ) n;
    return StringCompare ( item, & cref -> name );
}

/* Sort
 *  compares by ( name, type ) pair
 */
int64_t CC VColumnRefSort ( const BSTNode *item, const BSTNode *n )
{
    const VColumnRef *a = ( const VColumnRef* ) item;
    const VColumnRef *b = ( const VColumnRef* ) n;

    /* sorted first by name - case sensitive ASCII alphabetically */
    int64_t diff = StringCompare ( & a -> name, & b -> name );
    if ( diff != 0 )
        return diff;

    /* sort by cid */
    return VCtxIdCmp ( & a -> cid, & b -> cid );
}

#if 0 /* more for later */
static
rc_t create_cursor_all_readable_columns(const VTable *self,
                                        unsigned *ncol, uint32_t **idx,
                                        const VCursor **curs)
{
    KNamelist *list;
    rc_t rc = VTableListReadableColumns(self, &list);

    if (rc == 0) {
        rc = VTableCreateCursorReadInternal(self, curs);
        if (rc == 0) {
            uint32_t n;

            rc = KNamelistCount(list, ncol);
            if (rc == 0) {
                n = *ncol;
                *idx = malloc(n * sizeof(**idx));
                if (idx) {
                    unsigned i;

                    for (i = 0; i != (unsigned)n; ++i) {
                        const char *name;

                        rc = KNamelistGet(list, i, &name);
                        if (rc)
                            break;
                        rc = VCursorAddColumn(*curs, &(*idx)[i], "%s", name);
                        if (rc)
                            break;
                    }
                    if (rc)
                        free(*idx);
                }
                else
                    rc = RC(rcVDB, rcTable, rcValidating, rcMemory, rcExhausted);
            }
            if (rc)
                VCursorRelease(*curs);
        }
        KNamelistRelease(list);
    }
    if (rc) {
        *idx = NULL;
        *curs = NULL;
        *ncol = 0;
    }
    return rc;
}

static
rc_t fetch_all_rows(const VCursor *curs, unsigned ncol, const uint32_t cid[/* ncol */])
{
    int64_t start;
    int64_t stop;
    int64_t row;
    unsigned i;
    rc_t rc;

    for (i = 0; i != ncol; ++i) {
        int64_t cstart;
        uint64_t ccount;

        rc = VCursorIdRange(curs, cid[i], &cstart, &ccount);
        if (rc)
            return rc;
        if (i == 0) {
            start = cstart;
            stop = cstart + ccount;
        }
        else {
            if (start > cstart)
                start = cstart;
            if (stop < cstart + ccount)
                stop = cstart + ccount;
        }
    }
    for (row = start; row != stop; ++row) {
        for (i = 0; i != ncol; ++i) {
            uint32_t elem_bits;
            const void *base;
            uint32_t offset;
            uint32_t length;

            rc = VCursorCellDataDirect(curs, row, cid[i], &elem_bits,
                                       &base, &offset, &length);
            if (rc)
                return rc;
        }
    }
    return 0;
}

VDB_EXTERN
rc_t CC VTableConsistencyCheck(const VTable *self, int level)
{
    uint32_t *cid;
    unsigned ncol;
    const VCursor *curs;
    rc_t rc = create_cursor_all_readable_columns(self, &ncol, &cid, &curs);

    if (rc)
        return rc;
    rc = fetch_all_rows(curs, ncol, cid);
    free(cid);
    VCursorRelease(curs);
    return rc;
}
#endif


static bool VTableStaticEmpty( const struct VTable *self )
{
    bool res = true;
    KNamelist * col_names;
    rc_t rc = KMDataNodeListChildren ( self->col_node, &col_names );
    if ( rc == 0 )
    {
        uint32_t count;
        rc = KNamelistCount ( col_names, &count );
        if ( rc == 0 && count > 0 )
        {
            uint32_t idx;
            for ( idx = 0; rc == 0 && res && idx < count; ++idx )
            {
                const char * col_name;
                rc = KNamelistGet ( col_names, idx, &col_name );
                if ( rc == 0 )
                {
                    const KMDataNode * this_col;
                    rc = KMDataNodeOpenNodeRead ( self->col_node, &this_col, "%s/row_count", col_name );
                    if ( rc == 0 )
                    {
                        uint64_t this_row_count;
                        rc = KMDataNodeReadAsU64( this_col, &this_row_count );
                        if ( rc == 0 )
                        {
                            if ( this_row_count > 0 )
                                res = false; /* this will terminate the for-loop and leads to return( false ) */
                        }
						else
						{
							rc = 0;
						}
                        KMDataNodeRelease ( this_col );
                    }
                }
            }
        }
        KNamelistRelease( col_names );
    }
    return res;
}


static bool VTablePhysicalEmpty( const struct VTable *self )
{
    bool res = true;
    KNamelist * col_names;
    rc_t rc = KTableListCol ( self -> ktbl, &col_names );
    if ( rc == 0 )
    {
        uint32_t count;
        rc = KNamelistCount ( col_names, &count );
        if ( rc == 0 && count > 0 )
        {
            uint32_t idx;
            for ( idx = 0; rc == 0 && res && idx < count; ++idx )
            {
                const char * col_name;
                rc = KNamelistGet ( col_names, idx, &col_name );
                if ( rc == 0 )
                {
                    const KColumn * col;
                    rc = KTableOpenColumnRead ( self -> ktbl, &col, "%s", col_name );
                    if ( rc == 0 )
                    {
                        int64_t id_first;
                        uint64_t id_count;
                        rc = KColumnIdRange ( col, &id_first, &id_count );
                        if ( rc == 0 )
                        {
                            if ( id_count > 0 )
                                res = false; /* this will terminate the for-loop and leads to return( false ) */
                        }
                        KColumnRelease ( col );
                    }
                }
            }
        }
        KNamelistRelease( col_names );
    }
    return res;
}


/* IsEmpty
 * first tries to find out if there are static columns, that are not empty
 * send it checks if the KTable is empty
 */
LIB_EXPORT rc_t CC VTableIsEmpty ( const struct VTable *self, bool * empty )
{
    rc_t rc;

    if ( empty == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else
        {
			bool static_empty = VTableStaticEmpty( self );
			bool phys_empty = VTablePhysicalEmpty( self );
            *empty = ( static_empty && phys_empty );
            return 0;
        }
        * empty = false;
    }
    return rc;
}
