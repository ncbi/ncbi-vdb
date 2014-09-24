/*===========================================================================
*
*                            Public Domain Notice
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

#ifndef _h_vfs_resolver_
#define _h_vfs_resolver_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
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
struct KFile;
struct VPath;
struct KConfig;
struct KRepository;
struct VFSManager;


/*--------------------------------------------------------------------------
 * VResolver
 */
typedef struct VResolver VResolver;


/* Make
 *  ask the VFS manager to make a resolver
 */
VFS_EXTERN rc_t CC VFSManagerMakeResolver ( struct VFSManager const * self,
    VResolver ** new_resolver, struct KConfig const * cfg );
VFS_EXTERN rc_t CC KRepositoryMakeResolver ( struct KRepository const * self,
    VResolver ** new_resolver, struct KConfig const * cfg );


/* AddRef
 * Release
 */
VFS_EXTERN rc_t CC VResolverAddRef ( const VResolver * self );
VFS_EXTERN rc_t CC VResolverRelease ( const VResolver * self );


/* Accepted protocol list ordered by preference */
typedef uint32_t VRemoteProtocols;
enum
{
      eProtocolHttp
    , eProtocolFasp
    , eProtocolFaspHttp
    , eProtocolHttpFasp
    , eProtocolLastDefined
};


/* Query
 *  resolve object location to either an existing local path,
 *  or a pair of remote URL + local cache location.
 *
 *  "protocols" [ IN ] - the desired protocols for remote resolution
 *
 *  "query" [ IN ] - a path that can represent:
 *     accession : a recognizable accession from NCBI or known organization
 *     obj-id    : a dbGaP object id
 *     name      : a dbGaP filename
 *     path      : a filesystem path
 *     url       : a remote location
 *
 *  "local" [ OUT, NULL OKAY ] - optional return parameter for local path:
 *     accession : resolve to local user or site path
 *     obj-id    : resolve to local user protected path
 *     name      : resolve to local user protected path
 *     url       : set to NULL
 *
 *  "remote" [ OUT, NULL OKAY ] - optional return parameter for remote path:
 *     accession : resolve to URL
 *     obj-id    : resolve to URL
 *     name      : resolve to URL
 *     url       : set to duplicate
 *
 *  "cache" [ OUT, NULL OKAY ] - optional return parameter for cache path:
 *     accession : resolve to user cache path
 *     obj-id    : resolve to user cache path
 *     path      : set to NULL
 *     url       : resolve to user cache path
 *
 *  any of the output parameters may be NULL, but not all, i.e. there
 *  must be at least one non-NULL return parameter.
 *
 *  if you DON'T want local resolution, pass NULL for "local" and
 *  the query will be resolved remotely. if you don't want remote
 *  resolution, pass NULL for "remote".
 *
 *  a query that is resolved locally will always return NULL for
 *  "remote" and "cache", if the parameters are provided.
 */
VFS_EXTERN rc_t CC VResolverQuery ( const VResolver * self,
    VRemoteProtocols protocols, struct VPath const * query,
    struct VPath const ** local, struct VPath const ** remote,
    struct VPath const ** cache );


/* Local - DEPRECATED
 *  Find an existing local file/directory that is named by the accession.
 *  rcState of rcNotFound means it does not exist.
 *
 *  other rc code for failure are possible.
 *
 *  Accession must be an ncbi-acc scheme or a simple name with no 
 *  directory paths.
 */
VFS_EXTERN rc_t CC VResolverLocal ( const VResolver * self,
    struct VPath const * accession, struct VPath const ** path );


/* Remote - DEPRECATED
 *  Find an existing remote file that is named by the accession.
 *
 *  rcState of rcNotFound means it did not exist and can not be 
 *  downloaded. Probably a bad accession name.
 *
 *  Need a specific rc for no network configured.
 *  Need a specific rc for network access permitted.
 *
 *  Other rc code for failure are possible.
 *
 *  Accession must be an ncbi-acc scheme or a simple name with no 
 *  directory paths.
 *
 */
VFS_EXTERN rc_t CC VResolverRemote ( const VResolver * self,
    VRemoteProtocols protocols, struct VPath const * accession,
    struct VPath const ** path /* , struct KFile const ** opt_file_rtn */ );

/* Cache - DEPRECATED
 *  Find a cache directory that might or might not contain a partially
 *  downloaded file.
 *
 *  Accession must be an ncbi-acc scheme, an http url or a simple name with no 
 *  directory paths. All three should return the same directory URL as a VPath. (?)
 *  Or should it be a directory or a file url depending upon finding a partial
 *  download? This would require co-ordination with all download mechanisms that
 *  we permit.
 *
 *  With refseq holding wgs objects we have a case were the downloaded file is not
 *  named the same as the original accession as the file archive you want is a
 *  container for other files.
 *
 *  Find local will give a path that has a special scheme in these cases. 
 *  Find remote will give the url for the container that contains the accession
 *  so using the returned VPath from resolve remote is better than the original
 *  accession in this one case.  I think...
 */
VFS_EXTERN rc_t CC VResolverCache ( const VResolver * self,
    struct VPath const * url, struct VPath const ** path, uint64_t file_size );


/* EnableState
 *  modifies how the various properties are interpreted
 */
typedef uint32_t VResolverEnableState;
enum
{
    vrUseConfig = 0,            /* take enable/disable state from KConfig */
    vrAlwaysEnable = 1,         /* always enable, regardless of KConfig   */
    vrAlwaysDisable = 2         /* always disable, regardless of KConfig  */
};


/* LocalEnable
 *  modify settings for using local repositories,
 *  meaning site, user-public and user-protected.
 *
 *  "enable" [ IN ] - enable or disable local access,
 *  or follow settings in KConfig
 *
 *  returns the previous state of "remote-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
VFS_EXTERN VResolverEnableState CC VResolverLocalEnable ( const VResolver * self,
    VResolverEnableState enable );


/* RemoteEnable
 *  modify settings for using remote repositories
 *
 *  "enable" [ IN ] - enable or disable remote access,
 *  or follow settings in KConfig
 *
 *  returns the previous state of "remote-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
VFS_EXTERN VResolverEnableState CC VResolverRemoteEnable ( const VResolver * self,
    VResolverEnableState enable );


/* CacheEnable
 *  modify settings for caching files in user repositories
 *
 *  "enable" [ IN ] - enable or disable user repository cache,
 *  or follow settings in KConfig
 *
 *  returns the previous state of "cache-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
VFS_EXTERN VResolverEnableState CC VResolverCacheEnable ( const VResolver * self,
    VResolverEnableState enable );


#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_resolver_ */
