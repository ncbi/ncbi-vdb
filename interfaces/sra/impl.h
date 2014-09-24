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

#ifndef _h_sra_impl_h_
#define _h_sra_impl_h_

#ifndef _h_sra_path_extern_
#include <sra/path-extern.h>
#endif

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
struct KNamelist;
struct KDirectory;
typedef union SRAPath_vt SRAPath_vt;
typedef struct SRAPath_vt_v1 SRAPath_vt_v1;
typedef struct SRAPath_vt_v2 SRAPath_vt_v2;


/*--------------------------------------------------------------------------
 * SRAPath
 */
struct SRAPath
{
    const SRAPath_vt *vt;
};

#ifndef SRAPATH_IMPL
#define SRAPATH_IMPL struct SRAPath
#endif

struct SRAPath_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    rc_t ( CC *addref ) ( const SRAPATH_IMPL *self );
    rc_t ( CC *release ) ( const SRAPATH_IMPL *self );
    rc_t ( CC *version ) ( const SRAPATH_IMPL *self, uint32_t *version );
    rc_t ( CC *clear ) ( SRAPATH_IMPL *self );
    rc_t ( CC *addRepPath ) ( SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *addVolPath ) ( SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *fullPath ) ( const SRAPATH_IMPL *self, const char *rep, const char *vol,
			 const char *acc, char *path, size_t path_max );
    bool ( CC *testPath ) ( const SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *findPath ) ( const SRAPATH_IMPL *self, const char *acc, char *path, size_t path_max, size_t *rep_len );
    rc_t ( CC *listPath ) ( const SRAPATH_IMPL *self, struct KNamelist **runs, bool deep );
};

struct SRAPath_vt_v2
{
    /* version == 2.x */
    uint32_t maj;
    uint32_t min;

    rc_t ( CC *addref ) ( const SRAPATH_IMPL *self );
    rc_t ( CC *release ) ( const SRAPATH_IMPL *self );
    rc_t ( CC *version ) ( const SRAPATH_IMPL *self, uint32_t *version );
    rc_t ( CC *clear ) ( SRAPATH_IMPL *self );
    rc_t ( CC *addRepPath ) ( SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *addVolPath ) ( SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *fullPath ) ( const SRAPATH_IMPL *self, const char *rep, const char *vol,
			 const char *acc, char *path, size_t path_max );
    bool ( CC *testPath ) ( const SRAPATH_IMPL *self, const char *path );
    rc_t ( CC *findPath ) ( const SRAPATH_IMPL *self, const char *acc, char *path, size_t path_max, size_t *rep_len );
};

union SRAPath_vt
{
    SRAPath_vt_v1 v1;
    SRAPath_vt_v2 v2;
};


/* SRAPathMakeImpl
 *  make the SRAPath object within a shared library called "libsra-path"
 */
MOD_EXPORT rc_t CC SRAPathMakeImpl ( struct SRAPath **pm, struct KDirectory const *dir );

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_impl_h_ */
