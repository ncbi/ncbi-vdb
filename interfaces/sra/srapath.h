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

#ifndef _h_sra_srapath_
#define _h_sra_srapath_

#ifndef _h_sra_extern_
#include <sra/extern.h>
#endif

#ifndef _h_klib_callconv_
#include <klib/callconv.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TOOLS_USE_SRAPATH 0

/*--------------------------------------------------------------------------
 * forwards
 */
struct KNamelist;
struct KDirectory;


/*--------------------------------------------------------------------------
 * SRAPath
 *  manages accession -> path conversion
 */
typedef struct SRAPath SRAPath;


/* Make
 *  create path manager
 *
 *  the path manager should already be configured with
 *  standard search paths, but can be augmented by using
 *  the Add*Path messages.
 *
 *  "dir" [ IN, NULL OKAY ] - optional root directory to use
 *  attaches a new reference
 */
SRA_EXTERN rc_t CC SRAPathMake ( SRAPath **pm, struct KDirectory const *dir );


/* AddRef
 * Release
 */
SRA_EXTERN rc_t CC SRAPathAddRef ( const SRAPath *self );
SRA_EXTERN rc_t CC SRAPathRelease ( const SRAPath *self );

/* Version
 *  returns the library version
 */
SRA_EXTERN rc_t CC SRAPathVersion ( const SRAPath *self, uint32_t *version );


/* Clear
 *  forget existing server and volume paths for the default repository
 */
SRA_EXTERN rc_t CC SRAPathClear ( SRAPath *self );


/* AddRepPath
 *  add a replication path to the default repository
 *
 *  "rep" [ IN ] - NUL-terminated server search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "/panfs/traces01:/panfs/traces31"
 *
 *  NB - servers are searched in the order provided,
 *  first to last, until one of them satisfies a request,
 *  at which time the successful server is placed at the
 *  head of the search path.
 */
SRA_EXTERN rc_t CC SRAPathAddRepPath ( SRAPath *self, const char *rep );


/* AddVolPath
 *  add a volume path to the default repository
 *
 *  "vol" [ IN ] - NUL-terminated volume search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "sra2:sra1:sra0"
 *
 *  NB - volumes are searched in the order provided,
 *  first to last. they are never re-ordered.
 */
SRA_EXTERN rc_t CC SRAPathAddVolPath ( SRAPath *self, const char *vol );


/* Full
 *  creates full path from server, volume & accession
 *
 *  "rep" [ IN ] - NUL terminated full path of replication
 *  server, e.g. "/panfs/traces01"
 *
 *  "vol" [ IN ] - NUL terminated relative path of volume,
 *  e.g. "sra2"
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *  e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 */
SRA_EXTERN rc_t CC SRAPathFull ( const SRAPath *self, const char *rep,
    const char *vol, const char *accession, char *path, size_t path_max );


/* Test
 *  returns true if path appears to be accession
 *  the test is a heuristic, and may return false positives
 *
 *  "path" [ IN ] - NUL terminated path to run
 */
SRA_EXTERN bool CC SRAPathTest ( const SRAPath *self, const char *path );


/* Find
 *  finds location of run within rep-server/volume matrix
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *   e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 *
 *  returns 0 if path exists, rc state rcNotFound if
 *  path cannot be found, and rcInsufficient if buffer is
 *  too small.
 */
SRA_EXTERN rc_t CC SRAPathFind ( const SRAPath *self,
    const char *accession, char *path, size_t path_max );


/* List
 *  list all runs known to SRAPath
 *  this can be VERY SLOW due to the large number of runs
 *
 *  "runs" [ OUT ] - return parameter for ordered list of accessions
 *
 *  "deep" [ IN ] - when false, list only first responding server
 */
SRA_EXTERN rc_t CC SRAPathList ( const SRAPath *self, struct KNamelist **runs, bool deep );


#ifdef __cplusplus
}
#endif

#endif /* _h_sra_srapath_ */
