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

#ifndef _h_table_priv_
#define _h_table_priv_

#ifndef _h_vdb_table_
#include <vdb/table.h>
#endif

#ifndef _h_schema_priv_
#include "schema-priv.h"
#endif

#ifndef _h_vdb_schema_
#include <vdb/schema.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef KONST
#define KONST
#endif

#ifndef SKONST
#define SKONST const
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LAZY_OPEN_COL_NODE 0


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct STable;
struct SColumn;
struct VLinker;
struct VDatabase;
struct KMetadata;
struct VDBManager;


/*--------------------------------------------------------------------------
 * VTable
 *  represents a table
 *  normally implemented as a directory
 *  but may be a single archive file
 *  in either executable or streamable format
 */
struct VTable
{
    /* creating manager */
    struct VDBManager KONST *mgr;

    /* enclosing database - NULL OKAY */
    struct VDatabase KONST *db;

    /* schema and table description
       a new schema is created to house schema
       obtained from existing table/column metadata */
    VSchema SKONST *schema;
    struct STable const *stbl;

    /* intrinsic functions */
    struct VLinker *linker;

    /* underlying physical layer table */
    struct KTable KONST *ktbl;

    /* open table metadata */
    struct KMetadata KONST *meta;

    /* "col" subnode of metadata */
    struct KMDataNode KONST *col_node;

    /* caches of readable and writable columns */
    BSTree read_col_cache;
    BSTree write_col_cache;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );

    /* page size for creating columns */
    size_t pgsize;

    /* open references */
    KRefcount refcount;

    /* create and checksum modes for columns */
    uint8_t cmode;
    uint8_t checksum;

    /* open mode */
    bool read_only;

    /* cache validity */
    bool read_col_cache_valid;
    bool write_col_cache_valid;
};


/* Whack
 */
rc_t VTableWhack ( VTable *self );


/* Attach
 * Sever
 *  internal reference management
 */
VTable *VTableAttach ( const VTable *self );
rc_t VTableSever ( const VTable *self );


/* Make
 *  creates an initialized table object
 *  expects either a mgr or a parent db,
 *  creates a modifiable schema
 */
rc_t VTableMake ( VTable **tblp,
    struct VDBManager const *mgr,
    struct VDatabase const *db,
    const VSchema *schema );


/* LoadSchema
 *  looks in metadata for stored schema
 *  or searches available definitions for match
 */
rc_t VTableLoadSchema ( VTable *self );

/* StoreSchema
 *  stores schema definition in metadata
 */
VDB_EXTERN rc_t VTableStoreSchema ( VTable *self );

/* support for listing
 */
rc_t make_column_namelist ( const BSTree *columns, struct KNamelist **names );
rc_t make_column_typelist ( const BSTree *columns, const char *col,
    uint32_t *dflt_idx, struct KNamelist **typedecls );


/*--------------------------------------------------------------------------
 * VColumnRef
 *  cache entry
 */
typedef struct VColumnRef VColumnRef;
struct VColumnRef
{
    BSTNode n;
    String name;
    VTypedecl td;
    VCtxId cid;
    bool dflt;
    char typedecl [ 2 ];
};


/* Whack
 *  for dropping a BSTree
 */
void CC VColumnRefWhack ( BSTNode *n, void *ignore );


/* Make
 *  make a column ref from an SColumn*
 */
rc_t VColumnRefMake ( VColumnRef **cref,
    const VSchema *schema, struct SColumn const *scol );


/* CmpString
 *  compares by name only
 *  "item" is a const String*
 */
int CC VColumnRefCmpString ( const void *item, const BSTNode *n );

/* Sort
 *  compares by (name,type) pair
 */
int CC VColumnRefSort ( const BSTNode *item, const BSTNode *n );


#ifdef __cplusplus
}
#endif

#endif /* _h_table_priv_ */
