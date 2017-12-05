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

#ifndef _h_kfs_rrcachedfile_
#define _h_kfs_rrcachedfile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct Recorder;

enum cache_event { CE_REQUEST, CE_FOUND, CE_ENTER, CE_DISCARD, CE_FAILED };

typedef void ( *on_cache_event )( void * data, enum cache_event event,
                                  uint64_t pos, size_t len, uint32_t block_nr );

KFS_EXTERN rc_t CC MakeRRCached ( struct KFile const **rr_cached,
                                  struct KFile const *to_wrap,
                                  uint32_t page_size,
                                  uint32_t page_count );

KFS_EXTERN rc_t CC SetRRCachedEventHandler( struct KFile const * self,
                        void * data, on_cache_event handler );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_rrcachedfile_ */
