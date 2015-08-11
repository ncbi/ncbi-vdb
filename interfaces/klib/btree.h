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

#ifndef _h_klib_btree_
#define _h_klib_btree_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * defines
 */

#define PGBITS 15
#define PGSIZE ( 1U << PGBITS )
    

/* A pager must allocate pages of exactly 32k
 * page id's can not be larger than 31 bits
 * and page id's must be greater than zero
 * thus the maximum amount of memory that a pager
 * can provide is 32k * (2**31-1) = ~64G
 * keys are stored compressed, so 64G can store a lot of keys.
 *
 * If you don't provide a pager, but you do provide a backing file
 * KPageFile will be used to provide the paging.
 * If you don't provide a pager or a backing file, pages will be
 * provided by malloc, up to the given memory limit.
 */
typedef struct Pager Pager;
    
typedef struct Pager_vt Pager_vt;
struct Pager_vt {
    /* Pager and Pager_vt is expected to work like this:
     *  uint32_t id = 0;
     *  void const *page = vt->alloc(pager, &id);
     *  if (page != NULL) {
     *      void *content = vt->update(pager, page);
     *      assert(contents != NULL);
     *
     *  ... // do something with page contents
     *
     *      vt->unuse(pager, page); contents = NULL; page = NULL;
     *
     *  ...
     *
     *      page = use(pager, id);
     *      assert(page != NULL);
     *      void const *data = vt->access(pager, page);
     *      assert(data != NULL);
     *
     *      ...
     *
     *      vt->unuse(pager, page); data = NULL, page = NULL;
     *  }
     *  else {
     *      // can't allocate anymore pages
     *  }
     */

    /* it's reasonable for this function to fail, e.g. a pager may
     * limit the number of pages that it's willing to hand out
     */
    void const *(*alloc)(Pager *self, uint32_t *newid);
    
    /* these functions can't fail in any meaningful sense
     * these functions failing is like an assert or a sudden
     * hardware failure, in other words, not recoverable.
     */
    void const *(*use   )(Pager *self, uint32_t pageid);
    void const *(*access)(Pager *self, void const *page);
    void       *(*update)(Pager *self, void const *page);
    void        (*unuse )(Pager *self, void const *page);
};

/* Find
 *  searches for a match
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
KLIB_EXTERN rc_t CC BTreeFind ( uint32_t root, Pager *pager, Pager_vt const *vt, uint32_t *id,
    const void *key, size_t key_size );


/* Entry
 *  searches for a match or creates a new entry
 *
 *  "was_inserted" [ OUT ] - if true, the returned value was the result of an
 *   insertion and can be guaranteed to be all 0 bits. otherwise, the returned
 *   value will be whatever was there previously.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
KLIB_EXTERN rc_t CC BTreeEntry ( uint32_t *root, Pager *pager, Pager_vt const *vt, uint32_t *id,
    bool *was_inserted, const void *key, size_t key_size );

/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */

KLIB_EXTERN rc_t CC BTreeForEach ( uint32_t root, Pager *pager, Pager_vt const *vt, bool reverse,
                                 void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data );

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_btree_ */
