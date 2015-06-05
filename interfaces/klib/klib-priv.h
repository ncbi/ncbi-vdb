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

#ifndef _h_klib_priv_
#define _h_klib_priv_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_writer_
#include <klib/writer.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 */


/* reporting structure
 */
typedef struct ReportFuncs ReportFuncs;
struct ReportFuncs
{
    void ( CC * report ) ( uint32_t indent, const char* name, uint32_t count, ... );
    void ( CC * reportData ) ( uint32_t indent, const char* name, const char* data, uint32_t count, ... );
    rc_t ( CC * reportData1 ) ( const char* data );
    void ( CC * reportOpen ) ( uint32_t indent, const char* name, uint32_t count, ... );
    void ( CC * reportOpen1 ) ( uint32_t indent, const char* name );
    void ( CC * reportClose ) ( uint32_t indent, const char* name );
    void ( CC * reportClose1 ) ( const char* name );
    void ( CC * reportError ) ( uint32_t indent, rc_t rc, const char* function );
    void ( CC * reportErrorStr ) ( uint32_t indent, rc_t rc, const char* function,
        const char* name, const char* val );
    void ( CC * reportErrorStrImpl ) ( uint32_t indent, rc_t rc, const char* function,
        const char* name, const char* val, bool eol );
    void ( CC * reportErrorStrInt ) ( uint32_t indent, rc_t rc, const char* function,
        const char* names, const char* vals, const char* namei, uint32_t vali );
    void ( CC * reportError3Str ) ( uint32_t indent, rc_t rc, const char* function,
        const char* name, const char* v1, const char* v2, const char* v3, bool eol );
};


/* InitKFS
 */
KLIB_EXTERN void CC ReportInitKFS (
    rc_t ( CC * report_cwd ) ( const ReportFuncs *f, uint32_t indent ),
    rc_t ( CC * report_redirect ) ( KWrtHandler* handler,
        const char* filename, bool* to_file, bool finalize ) );

/* InitConfig
 *  returns appname if available
 */
KLIB_EXTERN const char* CC ReportInitConfig ( rc_t ( CC * report )
    ( const ReportFuncs *f, uint32_t indent,
      uint32_t configNodesSkipCount, va_list args ) );


/* InitVDB
 */
KLIB_EXTERN rc_t CC ReportInitVDB(
    rc_t (CC *report_obj)(const ReportFuncs *f,
        uint32_t indent, const char *path, bool *wasDbOrTableSet),
    rc_t (CC *report_software)(const ReportFuncs *f,
        uint32_t indent, const char *argv_0, const char *date, ver_t tool_ver),
    rc_t (CC *whack)(void));


/* RecordZombieFile
 */
KLIB_EXTERN void CC ReportRecordZombieFile ( void );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_priv_ */
