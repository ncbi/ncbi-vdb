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

#ifndef _h_prod_priv_
#define _h_prod_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_xform_priv_
#include "xform-priv.h"
#endif

#ifndef _h_schema_priv_
#include "schema-priv.h"
#endif

#ifndef _h_blob_priv_
#include "blob-priv.h"
#endif

#ifndef _h_klib_debug_
#include <klib/debug.h> /* DBG_VDB */
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define PROD_CACHE  	  2                   	/** default size of Production Cache ***/

/* IT IS A BAD IDEA TO USE EXTERN COLUMNS IN EXPRESSIONS
   but it used to be okay in version 0 of the schema, so
   it still occurs sometimes. We have allowed opening
   columns from within schema but were afraid to alter the
   row vector because this disturbs the serial allocation
   of ids that could fool programmers trying to be clever.
   Unfortunately, this resulted in a stray reference after
   we shifted reference ownership to the row vector.

   Setting this symbol to 1 will cause schema-opened columns
   to be inserted into the row vector, disturbing the index
   series passed out to clients, but not losing the reference. */
#define OPEN_COLUMN_ALTERS_ROW 1

#if _DEBUGGING
#define PROD_NAME 1
#endif

#define VDB_DEBUG(msg) DBGMSG(DBG_VDB,DBG_FLAG(DBG_VDB_RESOLVE), msg )

/*--------------------------------------------------------------------------
 * forwards
 */
struct VBlob;
struct KColumn;
struct VColumn;
struct VPhysical;
struct VProdResolve;
struct VBlobMRUCacheCursorContext;


/*--------------------------------------------------------------------------
 * VProduction
 */

/* major production types */
enum
{
    /* simple assignment from another production */
    prodSimple,

    /* function result and input parameters */
    prodFunc,

    /* a type of function with its own sub-productions */
    prodScript,

    /* an adapter production that references a VPhysical */
    prodPhysical,

    /* an adapter production referencing a VColumn */
    prodColumn,

    /* pivot into another table's row-id space */
    prodPivot
};

/* read/write chain */
enum
{
    chainInvalid,
    chainEncoding,
    chainDecoding,
    chainUncommitted
};

#if ! VPRODUCTION_DECLARED_IN_XFORM_H
typedef struct VProduction VProduction;
#endif
struct VProduction
{
#if PROD_NAME
    const char *name;
#endif
#if PROD_CACHE
    /* cached output */
    struct VBlob *cache [ PROD_CACHE ];
    uint32_t cache_cnt;
    uint32_t cache_wash_access_cnt;
#endif
    /* format declaration and element type description */
    VFormatdecl fd;
    VTypedesc desc;

    /* schema contextual id */
    VCtxId cid;

    /* owned vector id */
    uint32_t oid;

    /* production major variant ( enum see above ) */
    uint8_t var;

    /* production sub-variant ( specific to each major variant */
    uint8_t sub;

    /* indication of read/write chain */
    uint8_t chain;

    /* true if used as control parameter */
    bool control;
    /* is this production directly connected to a Column in a Cursor */
    VBlobMRUCacheCursorContext cctx;
};


/* Make
 *  allocation and parent initialization
 *  called from the "Make" functions below
 *
 *  "prod" [ OUT ] - returned production
 *
 *  "size" [ IN ] - sizeof sub-type
 *
 *  "owned" [ IN ] - vector to contain productions
 *
 *  "var" [ IN ] - variant code, e.g. prodSimple, prodFunc
 *
 *  "sub" [ IN ] - sub-variant code, e.g. prodSimplePage2Blob
 *
 *  "name" [ IN, NULL OKAY ] - optional object name, derived
 *  from schema if possible
 *
 *  "fd" [ IN ] and "desc" [ IN ] - production type description
 *
 *  "cid" [ IN ] - contextual ( two-level ) id
 *
 *  "chain" [ IN ] - which chain(s) are supported
 *    chainEncoding    - when going from input to physical
 *    chainDecoding    - when pulling from physical to output
 *    chainUncommitted - when resolving trigger and validation expressions
 */
rc_t VProductionMake ( VProduction **prod, Vector *owned, size_t size,
    int var, int sub, const char *name, const VFormatdecl *fd,
    const VTypedesc *desc, const VCtxId *cid, uint8_t chain );


/* Whack
 *  virtual whacker
 *  calls variant specific "Destroy" function
 *  frees object
 *
 *  "owned" [ IN, NULL OKAY ] - a Vector owning production
 *  if not NULL, production will be removed from vector
 */
void CC VProductionWhack ( void *item, void *owned );


#if ! VPRODUCTION_DECLARED_IN_XFORM_H
/* IdRange
 *  obtains intersection of all physical sources
 *
 *  "first" [ IN/OUT ] and "last" [ IN/OUT ] - range to intersect
 */
rc_t VProductionColumnIdRange ( const VProduction *self,
    int64_t *first, int64_t *last );
rc_t VProductionPageIdRange ( VProduction *self,
    int64_t id, int64_t *first, int64_t *last );

/* RowLength
 *  get row length for a particular row
 */
uint32_t VProductionRowLength ( const VProduction *self, int64_t row_id );

/* FixedRowLength
 *  get fixed row length for entire column
 *  returns 0 if not fixed
 */
uint32_t VProductionFixedRowLength ( const VProduction *self, int64_t row_id, bool ignore_self );

#endif /* ! VPRODUCTION_DECLARED_IN_XFORM_H */

/* ReadBlob
 *  fetch a blob containing at least the requested row
 *  returns a new reference
 */
rc_t VProductionReadBlob ( const VProduction *self, struct VBlob **vblob, int64_t * id , uint32_t cnt, struct VBlobMRUCacheCursorContext* cctx);

/* IsStatic
 *  trace all the way to a physical production
 */
rc_t VProductionIsStatic ( const VProduction *self, bool *is_static );

/* GetKColumn
 *  drills down to physical production to get a KColumn,
 *  and if that fails, indicate whether the column is static
 */
rc_t VProductionGetKColumn ( const VProduction * self, struct KColumn ** kcol, bool * is_static );


/*--------------------------------------------------------------------------
 * VSimpleProd
 *  single input param
 */
enum
{
    /* an implicit cast ( assignment ) or an explicit cast from schema */
    prodSimpleCast,

    /* perform blob class manipulation at page boundary */
    prodSimplePage2Blob,

    /* serialization interface at kcolumn boundary */
    prodSimpleSerial2Blob,
    prodSimpleBlob2Serial
};

typedef struct VSimpleProd VSimpleProd;
struct VSimpleProd
{
    VProduction dad;
    VProduction *in;
    struct VCursor const *curs;
};

rc_t VSimpleProdMake ( VProduction **prod, Vector *owned,
    struct VCursor const *curs,int sub, const char *name, const VFormatdecl *fd,
    const VTypedesc *desc, const VCtxId *cid,
    VProduction *in, uint8_t chain );

#define VSimpleProdDestroy( self ) \
    ( ( void ) 0 )

/* Read
 *  return a blob containing row id
 */
rc_t VSimpleProdRead ( VSimpleProd *self, struct VBlob **vblob, int64_t * id, uint32_t cnt, struct VBlobMRUCacheCursorContext *cctx );


/*--------------------------------------------------------------------------
 * VFunctionProd
 *  function input params are VProduction*
 *  extern C function pointer and self object
 */
enum
{
    prodFuncExternal = vftLastFuncProto - 1,

    /* internal types */
    prodFuncBuiltInCompare,
    prodFuncByteswap
};

typedef struct VFunctionProd VFunctionProd;
struct VFunctionProd
{
    VProduction dad;

    /* back-pointer to owning cursor
       not a reference, not to be released */
    struct VCursor const *curs;

    /* object and optional destructor */
    void *fself;
    void ( CC * whack ) ( void *self );

    /* runtime function */
    union
    {
        /* public types */
        VRowFunc rf;
        VNonDetRowFunc ndf;
        VArrayFunc af;
        VFixedRowFunc pf;
        VBlobFunc bf;

        /* merge type */
        VBlobFuncN bfN;

        /* compare type */
        VBlobCompareFunc cf;

        /* internal types */
        void ( * bswap ) ( void*, const void*, uint64_t );

    } u;

    /* vector of VProduction input parameters */
    Vector parms;

    /* adaptive prefetch parameters */
   int64_t start_id;
   int64_t stop_id;
};


rc_t VFunctionProdMake ( VFunctionProd **prod, Vector *owned,
    struct VCursor const *curs, int sub, const char *name,
    const VFormatdecl *fd, const VTypedesc *desc, uint8_t chain );

void VFunctionProdDestroy ( VFunctionProd *self );


/* BuiltInComparison
 */
rc_t VFunctionProdMakeBuiltInComp ( VProduction **prod, Vector *owned,
    const char *name, struct VProdResolve const *resolve,
    VProduction *orig, VProduction *test );


/* Read
 */
rc_t VFunctionProdRead ( VFunctionProd *self, struct VBlob **vblob, int64_t id , uint32_t cnt);


/*--------------------------------------------------------------------------
 * VScriptProd
 *  function input params are VProduction*
 *  intermediate productions are also VProduction*
 */
enum
{
    prodScriptFunction
};

typedef struct VScriptProd VScriptProd;
struct VScriptProd
{
    VProduction dad;
    VProduction *rtn;
    Vector owned;
    struct VCursor const *curs;
};


rc_t VScriptProdMake ( VScriptProd **prod, Vector *owned,
    struct VCursor const *curs, int sub, const char *name, const VFormatdecl *fd,
    const VTypedesc *desc, uint8_t chain );

void VScriptProdDestroy ( VScriptProd *self );


/* Read
 */
rc_t VScriptProdRead ( VScriptProd *self,
    struct VBlob **vblob, int64_t id, uint32_t cnt);


/*--------------------------------------------------------------------------
 * VPhysicalProd
 *  adapts a VPhysical to VProduction
 */
enum
{
    prodPhysicalOut,
    prodPhysicalKCol
};

typedef struct VPhysicalProd VPhysicalProd;
struct VPhysicalProd
{
    VProduction dad;
    struct VPhysical *phys;
};

rc_t VPhysicalProdMake ( VProduction **prod, Vector *owned,
    struct VCursor *curs, struct VPhysical *phys, int sub, const char *name,
    const VFormatdecl *fd, const VTypedesc *desc );

void VPhysicalProdDestroy ( VPhysicalProd *self );


/* Read
 */
rc_t VPhysicalProdRead ( VPhysicalProd *self,
    struct VBlob **vblob, int64_t id, uint32_t cnt );

rc_t VPhysicalProdColumnIdRange (const VPhysicalProd *self, int64_t *first, int64_t *last );

uint32_t VPhysicalProdFixedRowLength(const VPhysicalProd *self, int64_t row_id );

/*--------------------------------------------------------------------------
 * VColumnProd
 *  adapts a VColumn to VProduction
 */
enum
{
    prodColumnOut
};

typedef struct VColumnProd VColumnProd;
struct VColumnProd
{
    VProduction dad;
    struct VColumn *col;
};


rc_t VColumnProdMake ( VProduction **prodp, Vector *owned,
    struct VColumn *col, int sub, const char *name );

void VColumnProdDestroy ( VColumnProd *self );


/* Read
 */
rc_t VColumnProdRead ( VColumnProd *self,
    struct VBlob **vblob, int64_t id );

/*--------------------------------------------------------------------------
 * VPivotProd
 *  Pivots to a new row-id space
 *  Read side ony
 */
typedef struct VPivotProd VPivotProd;
struct VPivotProd
{
    VProduction   dad;
    VProduction * member; /* column or production */
    VProduction * row_id; /* in the row-id space of col's table */
};

rc_t VPivotProdMake ( VPivotProd ** prodp,
                      Vector *      owned,
                      VProduction * member,
                      VProduction * row_id,
                      const char *  name,
                      int           chain );

void VPivotProdDestroy ( VPivotProd *self );

/* Read
 */
rc_t VPivotProdRead ( VPivotProd *self, struct VBlob **vblob, int64_t * id, uint32_t cnt );

#ifdef __cplusplus
}
#endif

#endif /* _h_prod_priv_ */
