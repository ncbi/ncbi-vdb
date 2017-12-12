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
* NB. For now, we only support 1-table views, so for a view this will be
* the table its parameter is bound to.
*/
const struct VTable * VCursorGetTable ( const struct VCursor * self );

/** pagemap supporting thread **/
rc_t VCursorLaunchPagemapThread(struct VCursor *self);
rc_t VCursorTerminatePagemapThread(struct VCursor *self);
const PageMapProcessRequest* VCursorPageMapProcessRequest(const struct VCursor *self);

bool VCursorCacheActive ( const struct VCursor * self, int64_t * cache_empty_end );

Vector * VCursorTriggers ( struct VCursor * self );

bool VCursorIsReadOnly ( const struct VCursor * self );

VBlobMRUCache * VCursorGetBlobMruCache ( struct VCursor * self );

uint32_t VCursorIncrementPhysicalProductionCount ( struct VCursor * curs );


#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_priv_ */
