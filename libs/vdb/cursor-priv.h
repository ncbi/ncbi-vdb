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

#ifndef _h_cursor_priv_
#define _h_cursor_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
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

#include "blob-priv.h"

#define MTCURSOR_DBG( msg ) DBGMSG ( DBG_VDB, DBG_FLAG ( DBG_VDB_MTCURSOR ), msg )



#define VCURSOR_WRITE_MODES_SUPPORTED 0

/* temporary - until the full kproc for Windows is operational */
#if defined(WINDOWS) || defined(_WIN32) || defined(NCBI_WITHOUT_MT)
#define VCURSOR_FLUSH_THREAD 0
#else
#define VCURSOR_FLUSH_THREAD 1
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KLock;
struct KCondition;
struct KThread;
struct KNamelist;
struct KDlset;
struct VTable;
struct VCtxId;
struct VSchema;
struct SColumn;
struct VColumn;
struct VPhysical;
struct VView;
struct VCursor;
struct SNameOverload;

/*--------------------------------------------------------------------------
 * VCursorCache
 *  a dual-level vector
 */
typedef struct VCursorCache VCursorCache;
struct VCursorCache
{
    Vector cache;
};


/* Init
 */
#define VCursorCacheInit( self, start, block ) \
    VectorInit ( & ( self ) -> cache, start, block )

/* Whack
 */
void VCursorCacheWhack ( VCursorCache *self,
    void ( CC * whack ) ( void *item, void *data ), void *data );

/* Get
 *  retrieve object by cid
 */
void *VCursorCacheGet ( const VCursorCache *self, struct VCtxId const *cid );

/* Set
 *  set object by cid
 */
rc_t VCursorCacheSet ( VCursorCache *self,
    struct VCtxId const *cid, const void *item );

/* Swap
 *  swap object by cid
 */
rc_t VCursorCacheSwap ( VCursorCache *self,
    struct VCtxId const *cid, const void *item, void **prior );


/*--------------------------------------------------------------------------
 * VCursor
 */
enum
{
    vcConstruct,
    vcFailed,
    vcReady,
    vcRowOpen,
    vcRowCommitted,
    vcPageCommit
};

enum
{
    vfReady,
    vfBusy,
    vfFgErr,
    vfBgErr,
    vfExit
};

/* MakeColumn
*/
rc_t CC VCursorMakeColumn ( struct VCursor *self,
    struct VColumn **col, struct SColumn const *scol, Vector *cx_bind );

/* Whack - Private
*/
rc_t VCursorWhackInt ( const struct VCursor * p_self );

/* Columns
*/
VCursorCache * VCursorColumns ( struct VCursor * self );

/* PhysicalColumns
*/
VCursorCache * VCursorPhysicalColumns ( struct VCursor * self );

/* GetRow
*/
Vector * VCursorGetRow ( struct VCursor * self );

/* GetTable
* If cursor is on a table, returns the table itself.
* If cursor is on a view, returns the view's primary table:
*   If the view's first parameter is a table, return the table bound to the parameter.
*   If the view's first parameter is itself a view, return the primary table of the view bound to the parameter.
*/
const struct VTable * VCursorGetTable ( const struct VCursor * self );

/* IsReadOnly
*/
bool VCursorIsReadOnly ( const struct VCursor * self );

VBlobMRUCache * VCursorGetBlobMruCache ( struct VCursor * self );
uint32_t VCursorIncrementPhysicalProductionCount ( struct VCursor * curs );

const struct KSymbol * VCursorFindOverride ( const struct VCursor *self, const struct VCtxId *cid );

rc_t VCursorLaunchPagemapThread ( struct VCursor *self );
const PageMapProcessRequest* VCursorPageMapProcessRequest ( const struct VCursor *self );

bool VCursorCacheActive ( const struct VCursor * self, int64_t * cache_empty_end );

rc_t VCursorInstallTrigger ( struct VCursor * self, struct VProduction * prod );

rc_t VCursorRowFindNextRowId ( const Vector * self, uint32_t idx, int64_t start_id, int64_t * next );


/* GetColspec
 *  a "colspec" is either a simple column name or a typed name expression
 *  evaluates colspec and find an SColumn
 *
 *  "idx" [ OUT ] - return parameter for column index
*/
rc_t VCursorGetColidx ( const struct VCursor *       p_self,
                        const struct SColumn *       p_scol,
                        const struct SNameOverload * p_name,
                        uint32_t                     p_type,
                        uint32_t *                   p_idx );

/* SetRowIdRead - PRIVATE
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
rc_t VCursorSetRowIdRead ( struct VCursor *self, int64_t row_id );

/* OpenRowRead
 * CloseRowRead
 */
rc_t VCursorOpenRowRead ( struct VCursor *self );
rc_t VCursorCloseRowRead ( struct VCursor *self );

#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_priv_ */
