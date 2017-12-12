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

#ifndef _h_vdb_view_
#define _h_vdb_view_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct VDBManager;
struct VTable;
struct VSchema;

/*--------------------------------------------------------------------------
 * VView
 *  a collection of read-only columns
 */
typedef struct VView VView;

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VViewAddRef ( const VView *self );
VDB_EXTERN rc_t CC VViewRelease ( const VView *self );

/* OpenView
 *  open a view for read using manager.
 *
 *  "view" [ OUT ] - return parameter for newly opened view
 *
 *  "schema" [ IN ] - schema object with the view schema
 *
 *  View's parameters will have to be bound using VViewBindParameterXXX() before View can be used
 */
VDB_EXTERN rc_t CC VDBManagerOpenView (
    struct VDBManager const *   self,
    const VView **              view,
    const struct VSchema *      schema,
    const char *                name );

/* BindParameterTable
 *  Bind a view's parameter to a table.
 *
 *  "param_name" [ IN ] - parameter's name
 *  "table" [ IN ] - the table to bind the parameter to. Has to have the same type as the corresponding
 *  parameter of the view
 */
VDB_EXTERN rc_t CC VViewBindParameterTable (
    const VView * self,
    const char * param_name,
    const struct VTable * table );

/* BindParameterView
 *  Bind a view's parameter to a view.
 *
 *  "param_name" [ IN ] - parameter's name
 *  "view" [ IN ] - the view to bind the parameter to. Has to have the same type as the corresponding
 *  parameter of "self"
 */
VDB_EXTERN rc_t CC VViewBindParameterView (
    const VView * self,
    const char * param_name,
    const struct VView * view );

#ifdef __cplusplus
}
#endif

#endif /*  _h_vdb_view_ */
