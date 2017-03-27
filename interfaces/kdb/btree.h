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

#ifndef _h_kdb_btree_
#define _h_kdb_btree_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_kdb_btree_v1_
#include <kdb/btree-v1.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KBTree
 *  this implementation is an extremely simplified structure
 *  meant to provide the ability to create an index for temporary use
 */
#define KBTreeMakeRead NAME_VERS ( KBTreeMakeRead, KBTREE_VERS )
#define KBTreeMakeUpdate NAME_VERS ( KBTreeMakeUpdate, KBTREE_VERS )
#define KBTreeAddRef NAME_VERS ( KBTreeAddRef, KBTREE_VERS )
#define KBTreeRelease NAME_VERS ( KBTreeRelease, KBTREE_VERS )
#define KBTreeDropBacking NAME_VERS ( KBTreeDropBacking, KBTREE_VERS )
#define KBTreeSize NAME_VERS ( KBTreeSize, KBTREE_VERS )
#define KBTreeFind NAME_VERS ( KBTreeFind, KBTREE_VERS )
#define KBTreeEntry NAME_VERS ( KBTreeEntry, KBTREE_VERS )
#define KBTreeForEach NAME_VERS ( KBTreeForEach, KBTREE_VERS )

#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_btree_ */
