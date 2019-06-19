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

#ifndef _h_cloud_impl_
#define _h_cloud_impl_

#ifndef _h_cloud_extern_
#include <cloud/extern.h>
#endif

#ifndef _h_cloud_cloud_
#include <cloud/cloud.h>
#endif

#ifndef _h_kfc_refcount_impl_
#include <kfc/refcount-impl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
typedef union Cloud_vt Cloud_vt;



/*--------------------------------------------------------------------------
 * Cloud
 *  generic cloud interface
 */
struct Cloud
{
    const Cloud_vt * vt;
    KRefcount refcount;
};

#ifndef CLOUD_IMPL
#define CLOUD_IMPL struct Cloud
#endif

typedef struct Cloud_vt_v1 Cloud_vt_v1;
struct Cloud_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( CLOUD_IMPL * self );
    rc_t ( CC * make_cet ) ( const CLOUD_IMPL * self, struct String const ** cet );
    rc_t ( CC * add_cet_to_req ) ( const CLOUD_IMPL * self, struct KClientHttpRequest * req );
    rc_t ( CC * add_user_pays_to_req ) ( const CLOUD_IMPL * self, struct KClientHttpRequest * req );
    /* end minor version == 0 */
};

union Cloud_vt
{
    Cloud_vt_v1 v1;
};


/* Init
 *  initialize a newly allocated cloud object
 */
CLOUD_EXTERN rc_t CC CloudInit ( Cloud * self, const Cloud_vt * vt, const char * classname );


#ifdef __cplusplus
}
#endif

#endif /* _h_cloud_impl_ */
