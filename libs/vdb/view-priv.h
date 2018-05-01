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

#ifndef _h_view_priv_
#define _h_view_priv_

#include <vdb/view.h>

#include <klib/refcount.h>
#include <klib/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SView;
struct VCursor_vt;
struct VViewCursor;
struct VTable;
struct String;
struct VLinker;

struct VView
{
    /* open references */
    KRefcount refcount;

    /* schema and view description */
    const struct SView *    sview;
    const struct VSchema *  schema;

    /* parameter bindings */
    Vector bindings;

    /* intrinsic functions */
    struct VLinker * linker;
};

rc_t VViewCursorMake ( const VView * p_view, struct VViewCursor ** p_curs );

/* PrimaryTable
    return VTable representing the view's rowId space */
const struct VTable * VViewPrimaryTable( const VView * p_self );

/* GetBoundObject
*  returns VTable or VView bound to the specified parameter of the given SView, which corresponds to p_self or one of its parents.
*  NULL if not found.
*/
const void * VViewGetBoundObject( const VView * p_self, const struct SView * p_sview, uint32_t p_paramId );

#ifdef __cplusplus
}
#endif

#endif /* _h_view_priv_*/
