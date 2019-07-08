#ifndef _h_vfs_resolver_priv_
#define _h_vfs_resolver_priv_

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


#include <vfs/resolver.h> /* VResolver */


#ifdef __cplusplus
extern "C" {
#endif


/* QueryWithDir:
 * get local/cache location when URL is requested or target dir specified:
 * when resolveAccToCache is false: use FILE app rather than REFSEQ
 * inOutDir - returned cache location is in the output (or current) directory
 *
 * outDir==NULL resolveAccToCache==true : resolve all to cache
 * outDir==NULL resolveAccToCache==false: resolve accessions to cache,
 *                                                files - to cwd
 * outDir!=NULL: resolve all to outDir
 */
VFS_EXTERN rc_t CC VResolverQueryWithDir ( const VResolver * self,
    VRemoteProtocols protocols, const struct VPath * query,
    const struct VPath ** local, const struct VPath ** remote,
    const struct VPath ** cache, bool resolveAccToCache,
    const char * outDir, bool * inOutDir, bool queryIsUrl,
    const struct VPath * oldRemote, const VPath * oldMapping );


VFS_EXTERN rc_t CC VResolverGetProject ( const VResolver * self,
                                         uint32_t * project );


#ifdef __cplusplus
}
#endif


#endif /* _h_vfs_resolver_priv_ */
