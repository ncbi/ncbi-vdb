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

#ifndef _h_dbmgr_priv_
#define _h_dbmgr_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_vdb_manager_
#include <vdb/manager.h>
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


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDBManager;
struct VSchema;
struct VLinker;


/*--------------------------------------------------------------------------
 * VDBManager
 *  handle to library
 */
struct VDBManager
{
    /* underlying physical layer manager */
    struct KDBManager KONST *kmgr;

    /* intrinsic schema elements */
    struct VSchema *schema;

    /* intrinsic functions */
    struct VLinker *linker;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );

    /* open references */
    KRefcount refcount;
};

/* Attach
 * Sever
 *  internal reference management
 */
VDBManager *VDBManagerAttach ( const VDBManager *self );
rc_t VDBManagerSever ( const VDBManager *self );


/* ConfigPaths
 *  looks for configuration information to set
 *  include paths for schema parser and
 *  load paths for linker
 */
rc_t VDBManagerConfigPaths ( VDBManager *self, bool update );


/*--------------------------------------------------------------------------
 * generic whackers
 */
void CC VectMbrWhack ( void *item, void *ignore );
void CC BSTreeMbrWhack ( BSTNode *n, void *ignore );



#ifdef __cplusplus
}
#endif

#endif /* _h_dbmgr_priv_ */
