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

#ifndef _h_vdb_report_
#define _h_vdb_report_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VDBManager;
struct VDatabase;
struct VTable;


/*--------------------------------------------------------------------------
 * An unrecoverable error happened.
 * We can help to solve it
 * by reporting information about known application execution environment.
 */


/* SetVDBManager
 *  remember the manager in use
 */
VDB_EXTERN rc_t CC ReportSetVDBManager ( struct VDBManager const *mgr );


/* SetDatabase
 *  To be called after application successfully opened Database
 *  specified in command line.
 *  If the application supports multiple objects passed in command line
 *  to be processed in a loop: should be called for each argument database.
 *
 *  "path" [ IN ] - path to the database that is used to access it
 */
VDB_EXTERN rc_t CC ReportResetDatabase ( const char *path,
    struct VDatabase const *db );


/* SetTable
 *  To be called after application successfully opened Table
 *  specified in command line.
 *  If the application supports multiple objects passed in command line
 *  to be processed in a loop: should be called for each argument table.
 *
 *  "path" [ IN ] - path to the table that is used to access it
 */
VDB_EXTERN rc_t CC ReportResetTable ( const char *path,
    struct VTable const *tbl );


#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_report_ */
