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

#ifndef _h_kproc_timeout_
#define _h_kproc_timeout_

#ifndef _h_kproc_extern_
#include <kproc/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_os_native_
#include <os-native.h> /* struct timeout_t */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * system specific timeout object
 */
typedef struct timeout_t timeout_t;


/* Init
 *  initialize a timeout in milliseconds
 */
KPROC_EXTERN rc_t CC TimeoutInit ( timeout_t * tm, uint32_t msec );


/* Prepare
 *  ensures that a timeout is prepared with an absolute value
*/
KPROC_EXTERN rc_t CC TimeoutPrepare ( timeout_t * tm );


/* Remaining
 *  ask how many milliseconds remain before timeout expires
 */
KPROC_EXTERN uint32_t TimeoutRemaining ( timeout_t * tm );


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_timeout_ */
