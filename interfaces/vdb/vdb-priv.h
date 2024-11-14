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

#ifndef _h_vdb_vdb_priv_
#define _h_vdb_vdb_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDatabase;
struct KDataBuffer;
struct KDBManager;
struct KDirectory;
struct KMDataNode;
struct KMetadata;
struct KNamelist;
struct KTable;
struct String;
struct VCursor;
struct VCursorParams;
struct VDatabase;
struct VDBDependencies;
struct VDBManager;
struct VFSManager;
struct VPath;
struct VResolver;
struct VSchema;
struct VTable;
struct VTypedef;
struct VXformInfo;
struct PageMapIterator;

struct KDataBuffer;
struct KNamelist;
struct BSTree;
struct VColumn;
struct VCtxId;

/*--------------------------------------------------------------------------
 * VDBManager
 */


/* MakeRsrc
 *  common make, regardless of library
 */
VDB_EXTERN rc_t CC VDBManagerMakeRsrc ( struct VDBManager **mgr, struct VFSManager * vfs );


/* OpenKDBManager
 *  returns a new reference to KDBManager used by VDBManager
 */
VDB_EXTERN rc_t CC VDBManagerOpenKDBManagerRead ( struct VDBManager const *self,
    struct KDBManager const **kmgr );
VDB_EXTERN rc_t CC VDBManagerOpenKDBManagerUpdate ( struct VDBManager *self,
    struct KDBManager **kmgr );

#define VDBManagerGetKDBManagerRead VDBManagerOpenKDBManagerRead
#define VDBManagerGetKDBManagerUpdate VDBManagerOpenKDBManagerUpdate


/* ListExternalSchemaModules
 */
VDB_EXTERN rc_t CC VDBManagerListExternalSchemaModules ( struct VDBManager const *self,
    struct KNamelist **list );

/* DisablePagemapThread
 *  this can cause difficulties for some clients
 */
VDB_EXTERN rc_t CC VDBManagerDisablePagemapThread ( struct VDBManager const *self );

/* DisableFlushThread
 *  Disable the background cursor flush thread, may be useful when debugging
 */
VDB_EXTERN rc_t CC VDBManagerDisableFlushThread ( struct VDBManager *self );

/* Make with custom KDBManager */
VDB_EXTERN rc_t CC VDBManagerMakeReadWithKDBManager (
    struct VDBManager const **mgr, const struct KDBManager *kdbmgr );

/* Make with custom VFSManager */
VDB_EXTERN rc_t CC VDBManagerMakeReadWithVFSManager (
    struct VDBManager const **mgr,
    struct KDirectory const *wd, struct VFSManager *vmgr );
VDB_EXTERN rc_t CC VDBManagerMakeUpdateWithVFSManager (
    struct VDBManager **mgr, struct KDirectory *wd, struct VFSManager *vmgr );
VDB_EXTERN rc_t CC VDBManagerMakeWithVFSManager ( struct VDBManager const **mgr,
    struct KDirectory const *wd, struct VFSManager *vmgr );

/** Reset VResolver to set protected repository context */
VDB_EXTERN rc_t CC VDBManagerSetResolver
    ( struct VDBManager const * self, struct VResolver * resolver );

/** Unreliable object: do not report occured erros */
VDB_EXTERN int CC VDBManagerPathTypeUnreliable ( const struct VDBManager * self,
     const char *object, ... );

/** Add  caller-defined schema function factories */
typedef struct VLinkerIntFactory VLinkerIntFactory;
struct VTransDesc; // see vdb/xform.h
struct VLinkerIntFactory
{
    rc_t ( CC * f ) ( struct VTransDesc *desc );
    const char *name;
};

VDB_EXTERN rc_t CC VDBManagerAddFactories ( const struct VDBManager * self,
    const VLinkerIntFactory *fact, uint32_t count );

/*--------------------------------------------------------------------------
 * VSchema
 */

/* DescribeTypedef - PRIVATE
 *  a type is defined as either:
 */
VDB_EXTERN rc_t CC VSchemaDescribeTypedef ( struct VSchema const *self,
    struct VTypedef *def, uint32_t type_id );

/* LastIntrinsicTypeId - PRIVATE
 *  used to allow cast to intrinsics
 */
VDB_EXTERN uint32_t CC VSchemaLastIntrinsicTypeId ( struct VSchema const *self );

/* ListLegacyTables - PRIVATE
 *  scan all tables defined in schema
 *  return the fully-qualified and versioned name of each legacy SRA table
 */
VDB_EXTERN rc_t CC VSchemaListLegacyTables ( struct VSchema const *self, struct KNamelist **list );

/* DumpToKMDataNode
 *  given a VSchema, an updatable KMDataNode, an object spec string and object type,
 *  find the typed object within VSchema by name, dump its schema text into the node,
 *  and set the attribute name to the full object name and version.
 */
VDB_EXTERN rc_t CC VSchemaDumpToKMDataNode ( struct VSchema const * self,
    struct KMDataNode * node, const char * spec );


/*--------------------------------------------------------------------------
 * VDatabase
 */

/* OpenDBReadVPathLight
 *  used just to analyze VDatabase:
 *  - don't try to locate and open vdbcache
 *  - don't fail if database's quality does not match requested one
 */
VDB_EXTERN rc_t CC VDBManagerOpenDBReadVPathLight(
    struct VDBManager const *self, const struct VDatabase **db,
    struct VSchema const *schema, const struct VPath *path);

/* OpenKDatabase
 *  returns a new reference to underlying KDatabase
 */
VDB_EXTERN rc_t CC VDatabaseOpenKDatabaseRead ( struct VDatabase const *self, struct KDatabase const **kdb );
VDB_EXTERN rc_t CC VDatabaseOpenKDatabaseUpdate ( struct VDatabase *self, struct KDatabase **kdb );


/* IsCSRA
 *  ask an open database if it conforms to cSRA schema
 */
VDB_EXTERN bool CC VDatabaseIsCSRA ( struct VDatabase const *self );

/* Get accession and path of databasepath (is avalibable)
 *  acc and path need to be released
 */
VDB_EXTERN rc_t CC VDatabaseGetAccession(const struct VDatabase * self,
    const struct String ** acc, const struct String ** path);

/* VDatabaseMemberType
 * Characterize a database member
 */
enum
{
    dbmUnknown,
    dbmDatabase,
    dbmTable,
    dbmViewAlias
};
VDB_EXTERN rc_t VDatabaseMemberType(const struct VDatabase * self, const char * name, uint32_t * type);

/*--------------------------------------------------------------------------
 * VTable
 */

/* OpenKTable
 *  returns a new reference to underlying KTable
 */
VDB_EXTERN rc_t CC VTableOpenKTableRead ( struct VTable const *self, struct KTable const **ktbl );
VDB_EXTERN rc_t CC VTableOpenKTableUpdate ( struct VTable *self, struct KTable **ktbl );

#define VTableGetKTableRead VTableOpenKTableRead
#define VTableGetKTableUpdate VTableOpenKTableUpdate


/* DropColumn
 */
VDB_EXTERN rc_t CC VTableDropColumn ( struct VTable *self, const char *name, ... );
VDB_EXTERN rc_t CC VTableVDropColumn ( struct VTable *self, const char *name, va_list args );

/* RenameColumn
 */
VDB_EXTERN rc_t CC VTableRenameColumn ( struct VTable *self, bool force,
    const char *from, const char *to );

/* CopyColumn
 */
VDB_EXTERN rc_t CC VTableCopyColumn (  struct VTable *self
                                     , bool force
                                     , struct VTable const *source
                                     , const char *name );

/* ListPhysColumns
 * ListSeededWritableColumns
 *  avail: 2.4
 */
VDB_EXTERN rc_t CC VTableListPhysColumns ( struct VTable const *self, struct KNamelist **names );
VDB_EXTERN rc_t CC VTableListSeededWritableColumns ( struct VTable *self,
    struct KNamelist **names, struct KNamelist const *seed );


/* HasStaticColumn
 *  given a physical column name, report whether it exists
 *  and is ( currently ) static
 *  avail: 2.5
 */
VDB_EXTERN bool CC VTableHasStaticColumn ( struct VTable const *self, const char *name );
VDB_EXTERN bool CC VTableVHasStaticColumn ( struct VTable const *self, const char *name, va_list args );


/* VUntypedTableTest
 *  support for tables created before embedded schema
 *
 *  "ktbl" [ IN ] - physical table to recognize
 *
 *  "meta" [ IN ] - open metadata for table
 *
 *  returns true if table is recognized
 */
typedef bool ( CC * VUntypedTableTest )
   ( struct KTable const *ktbl, struct KMetadata const *meta );


/* StoreSchema
 *  stores current schema definition in metadata
 *
 *  useful if table was opened using a later version of schema than
 *  was used for its creation.
 */
VDB_EXTERN rc_t VTableStoreSchema ( struct VTable *self );


/*--------------------------------------------------------------------------
 * VCursor
 */


/* CreateCursorView
 *  creates a read-only cursor onto table
 *  uses table schema as base
 *  uses view described in viewspec as view
 *
 *  AVAILABILITY: version 2.3
 *
 *  for pre-vdb-2.3 tables, defaults to CreateCursorRead, ignoring
 *  schema specification
 *
 *  "curs" [ OUT ] - return parameter for newly created cursor
 *
 *  "viewspec" [ IN ] - NUL terminated string describing the view schema
 */
VDB_EXTERN rc_t CC VTableCreateCursorView ( struct VTable const *self,
    struct VCursor const **curs, const char *viewspec );

/* PermitPostOpenAdd
 *  allows columns to be added to open cursor
 *  for write cursor, the effect lasts until the first row commit
 */
VDB_EXTERN rc_t CC VCursorPermitPostOpenAdd ( struct VCursor const *self );
/*  SuspendTriggers
 *  blocks resolution of schema-based triggers
 *
 */
VDB_EXTERN rc_t CC VCursorSuspendTriggers ( struct VCursor const *self );

/*  VCursorGetSchema
 *  returns current schema of the open cursor
 */
VDB_EXTERN struct VSchema const* CC VCursorGetSchema ( struct VCursor const *self);

/* PageIdRange
 *  returns id range for page
 *
 *  "id" [ IN ] - page containing this row id is target
 *
 *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
VDB_EXTERN rc_t CC VCursorPageIdRange ( struct VCursor const *self,
    uint32_t idx, int64_t id, int64_t *first, int64_t *last );


/* IsStaticColumn
 *  answers question: "does this column have the same value for every cell?"
 */
VDB_EXTERN rc_t CC VCursorIsStaticColumn ( struct VCursor const *self,
    uint32_t col_idx, bool *is_static );


VDB_EXTERN rc_t CC VCursorLinkedCursorGet(const struct VCursor *cself,const char *tbl, struct VCursor const **curs);
VDB_EXTERN rc_t CC VCursorLinkedCursorSet(const struct VCursor *cself,const char *tbl, struct VCursor const *curs);

VDB_EXTERN uint64_t CC VCursorSetCacheCapacity(struct VCursor *self,uint64_t capacity);
VDB_EXTERN uint64_t CC VCursorGetCacheCapacity(const struct VCursor *self);

/* ListReadableColumns
 *  performs an insert of '*' to cursor
 *  attempts to resolve all read rules
 *  records all SColumns that successfully resolved
 *  populates BTree with VColumnRef objects
 */
rc_t VCursorListReadableColumns ( struct VCursor *self, struct BSTree *columns );

/* GetColumn
*/
VDB_EXTERN struct VColumn * VCursorGetColumn(struct VCursor * self, const struct VCtxId * ctx);

/*--------------------------------------------------------------------------
 * VCursorParams
 */

/* Get
 *  return KDataBuffer value by reference
 */
VDB_EXTERN rc_t CC VCursorParamsGet ( struct VCursorParams const *self,
    const char *name, struct KDataBuffer **value );

/* Set
 *  set a textual parameter value
 */
VDB_EXTERN rc_t CC VCursorParamsSet( struct VCursorParams const *self,
    const char *name, const char *fmt, ... );
VDB_EXTERN rc_t CC VCursorParamsVSet ( struct VCursorParams const *self,
    const char *name, const char *fmt, va_list args );

/* Unset
 *  set the value of a named parameter to "undefined"
 */
VDB_EXTERN rc_t CC VCursorParamsUnset ( struct VCursorParams const *self, const char *name );


/*--------------------------------------------------------------------------
 * xform
 */

/*
  uses vftReserved code
  returns little-endian data, no row-wise rle
  fixed row-length is calculated as dst->elem_count / num_rows
 */
#define vftLegacyBlob vftReserved

typedef struct VLegacyBlobResult VLegacyBlobResult;
struct VLegacyBlobResult
{
    struct KDataBuffer *dst;
    uint8_t byte_order;
};

typedef rc_t ( CC * VNoHdrBlobFunc ) ( void *self,
    struct VXformInfo const *info, VLegacyBlobResult *rslt,
    struct KDataBuffer const *src );

/* function to call to test table against schema */
typedef bool ( CC * VUntypedFunc )
    ( struct KTable const *tbl, struct KMetadata const *meta );


/* Don't release returned quality string */
VDB_EXTERN rc_t CC VDBManagerGetQualityString(const struct VDBManager * self,
    const char ** quality);

/* ListDependenciesExt
 *  create a dependencies object: list all dependencies
 *
 *  "dep" [ OUT ] - return for VDBDependencies object
 *
 *  "missing" [ IN ] - if true, list only missing dependencies
 *                     otherwise, list all dependencies
 *  "directory" [ IN ] - if not NULL - resolve dependencies inside of directory
 *
 * N.B. If missing == true then
 *     just one refseq dependency will be returned for 'container' Refseq files.
 */
VDB_EXTERN rc_t CC VDatabaseListDependenciesExt ( struct VDatabase const *self,
    const struct VDBDependencies **dep, bool missing, const char* directory );

#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_vdb_priv_ */
