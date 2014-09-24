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

#ifndef _h_klib_report_
#define _h_klib_report_

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
 * An unrecoverable error happened.
 * We can help to solve it
 * by reporting information about known application execution environment.
 */


/* Init
 *  initialize with common information
 *
 *  "argc" [ IN ] and "argv" [ IN ] - intact command line
 *
 *  "tool_version" [ IN ] - version of tool
 */
KLIB_EXTERN void CC ReportInit ( int argc, char *argv [],
    ver_t tool_version );


/* BuildDate
 *  set the build date of the tool
 *
 *  "date" [ IN ] - pre-processor __DATE__
 */
KLIB_EXTERN void CC ReportBuildDate ( const char *date );


/* Silence
 *  tell report to be silent at exit
 *  useful especially in response to ^C
 */
KLIB_EXTERN void CC ReportSilence ( void );


/* Finalize
 *  perform cleanup
 *  optionally report error condition
 *
 *  "rc" [ IN ] - report environment information if not zero
 */
KLIB_EXTERN rc_t CC ReportFinalize ( rc_t rc );

/* ForceFinalize
 *  Finalize forcing report generation to stdout
 */
KLIB_EXTERN rc_t CC ReportForceFinalize ( void );


/* ResetObject
 *  Resets the name of the current object beeing processes(path or accession),
 *  usually from command line.
 *  Should be called each time
 *  when starting next command line argument processing.
 */
KLIB_EXTERN rc_t CC ReportResetObject ( const char *path );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_report_ */
