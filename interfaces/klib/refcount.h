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

#ifndef _h_klib_refcount_
#define _h_klib_refcount_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

#ifndef FORCE_TRACK_REFERENCES
#define FORCE_TRACK_REFERENCES 0
#endif

/* normally turned off */
#if FORCE_TRACK_REFERENCES
#undef TRACK_REFERENCES
#define TRACK_REFERENCES 1
#elif ! defined TRACK_REFERENCES
#define TRACK_REFERENCES 0
#endif

/* include logging interface */
#if TRACK_REFERENCES

#include <klib/debug.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRefcount inline implementation
 */

#if TRACK_REFERENCES

#undef REFNEW
#undef CNTMSG
#undef REFMSG
#undef REFNEW_COMMA
#undef REFMSG_COMMA

#ifndef REFMOD
#define REFMOD DBG_REF
#endif

#ifndef REFCOND
#define REFCOND DBG_REF_ANY
#endif

#define REFNEW( clsname, op, name, instance, refcount )                 \
    DBGMSG ( REFMOD, REFCOND, ( "created %s, operation %s, name '%s', " \
                                "instance 0x%zX: initial refcount %d\n",  \
                                clsname, op, name, instance, refcount ))
#define CNTMSG( clsname, op, instance, refcount )                       \
    DBGMSG ( REFMOD, REFCOND, ( "about to %s instance 0x%zX: prior refcount = %d for %s\n", \
                                op, instance, refcount, clsname ))
#define REFMSG( clsname, op, instance ) \
    CNTMSG ( clsname, op, instance, atomic32_read ( instance ) )
#define REFNEW_COMMA( clsname, op, name, instance, refcount ) \
    REFNEW ( clsname, op, name, instance, refcount ),
#define REFMSG_COMMA( clsname, op, instance ) \
    REFMSG ( clsname, op, instance ),

#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_refcount_ */
