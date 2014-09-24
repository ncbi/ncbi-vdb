#ifndef _h_libs_blast_blast_mgr_
#define _h_libs_blast_blast_mgr_

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

#ifndef _h_klib_printf_
#include <klib/printf.h> /* otherwise magic error */
#endif

#ifndef _h_ncbi_vdb_blast_
#include <ncbi/vdb-blast.h> /* VdbBlastStatus */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if _DEBUGGING
#define S STSMSG(9,(""));
#else
#define S
#endif

#define RELEASE(type, thing) do { rc_t rc2 = type##Release(thing); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } thing = NULL; } while (false)

struct VDatabase;
struct VTable;

typedef uint32_t BTableType;
enum {
      btpUndefined
    , btpSRA
    , btpWGS
    , btpREFSEQ
};

BTableType _VdbBlastMgrBTableType(const struct VdbBlastMgr *self,
    const char *rundesc);

VdbBlastStatus _VdbBlastMgrFindNOpenSeqTable(const VdbBlastMgr *self,
    const char *rundesc,
    const struct VTable **tbl,
    BTableType *type,
    char **fullpath,
    const struct VDatabase **db);

rc_t _VdbBlastMgrNativeToPosix(const VdbBlastMgr *self, const char *native, char *posix, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _h_libs_blast_blast_mgr_ */
