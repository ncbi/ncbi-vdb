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

#ifndef _h_align_manager_
#define _h_align_manager_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * AlignMgr
 *  manages the alignment module
 */
typedef struct AlignMgr AlignMgr;

/* MakeRead
 *  make for read-only operations
 * MakeUpdate
 *  make for read/write operations
 */
ALIGN_EXTERN rc_t CC AlignMgrMakeRead ( const AlignMgr **mgr );
ALIGN_EXTERN rc_t CC AlignMgrMakeUpdate ( AlignMgr **mgr );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignMgrAddRef ( const AlignMgr *self );
ALIGN_EXTERN rc_t CC AlignMgrRelease ( const AlignMgr *self );

    
#ifdef __cplusplus
}
#endif

#endif /* _h_align_manager_ */
