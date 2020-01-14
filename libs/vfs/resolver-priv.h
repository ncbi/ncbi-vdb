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

#ifndef _h_resolver_priv_
#define _h_resolver_priv_

#include <klib/text.h> /* String */

#ifndef _h_vfs_resolver_
#include <vfs/resolver.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_PROTOCOLS eProtocolHttpHttps

/*--------------------------------------------------------------------------
 * KConfig Repository Structure
 *  expressed in pseudo BNF:
 *
 *  base-config
 *      = "repository" <category>*
 *      ;
 *
 *  category
 *      = "remote" <remote-sub-category>*
 *      | "site" <site-sub-category>*
 *      | "user" <user-sub-category>*
 *      ;
 *
 *  remote-sub-category
 *      = "main" <public-remote-named-repository>*
 *      | "aux" <public-remote-named-repository>*
 *      | "protected" <protected-remote-named-repository>*
 *      ;
 *
 *  site-sub-category
 *      = "main" <site-named-repository>*
 *      | "aux" <site-named-repository>*
 *      ;
 *
 *  user-sub-category
 *      = "public" <public-user-named-repository>*
 *      | "protected" ID <protected-user-named-repository>*
 *      ;
 *
 *  public-remote-named-repository
 *      = ID <public-remote-repository>
 *      ;
 *
 *  protected-remote-named-repository
 *      = ID <protected-remote-repository>
 *      ;
 *
 *  site-named-repository
 *      = ID <site-repository>
 *      ;
 *
 *  public-user-named-repository
 *      = ID <public-user-repository>
 *      ;
 *
 *  protected-user-named-repository
 *      = ID <protected-user-repository>
 *      ;
 *
 *  public-remote-repository
 *      = <public-remote-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  protected-remote-repository
 *      = <protected-remote-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  site-repository
 *      = <local-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  public-user-repository
 *      = <local-root> [ <disabled> ] [ <cache-enabled> ] <apps>*
 *      ;
 *
 *  protected-user-repository
 *      = <local-root> <passwd> <download-ticket> [ <disabled> ] [ <cache-enabled> ] <apps>*
 *      ;
 *
 *  apps
 *      = "refseq" <app>
 *      | "sra" <app>
 *      | "wgs" <app>
 *      ;
 *
 *  app
 *      = [ <disabled> ] "volumes" <volume>*
 *      ;
 *
 *  volume
 *      = <expansion-alg> "=" <path-list>
 *      ;
 *
 *  expansion-alg
 *      = "flat" | "sra1000" | "sra1024" | "fuse1000" | "refseq" | ...
 *      ;
 *
 *  disabled
 *      = "disabled" "=" ( "true" | * )  # any value other than "true" is false
 *      ;
 *
 *  cache-enabled
 *      = "cache-enabled" "=" ( "true" | * )  # any value other than "true" is false
 *      ;
 */

#define rcResolver   rcTree

struct KDataBuffer;
struct KNgcObj;
struct KNSManager;
struct String;
struct VResolverAccToken;
struct VResolverAlg;


rc_t VPathCheckFromNamesCGI(const struct VPath *path,
    const struct String *ticket, int64_t projectId,
    const struct VPath **mapping);


/*--------------------------------------------------------------------------
 * VResolverAlg
 *  represents a set of zero or more volumes
 *  each of which is addressed using a particular expansion algorithm
 */
typedef enum
{
    appUnknown,
    appAny,
    appFILE,
    appREFSEQ,
    appSRA,
    appWGS,
    appNANNOT,
    appNAKMER,
    appSRAPileup,
    appSRARealign,
    appCount
} VResolverAppID;

typedef enum
{
    algCGI,
    algFlat,
    algFlatAD,
    algWithExtFlat,
    algAD,    /* Accession as Directory */
    algSRAAD,    /* Accession as Directory for SRA */
    algSRAFlat,
    algSRA1024,
    algSRA1000,
    algFUSE1000,
    algREFSEQ,
    algREFSEQAD, /* Accession as Directory for Refseq*/
    algWGS2,                /* ordered to be of higher precedence than algWGS */
    algWGS,
    algWGSFlat,
    algFuseWGS,
    algSRA_NCBI,
    algSRA_EBI,

    algNANNOTFlat,
    algNANNOT,
    algFuseNANNOT,
    algNAKMERFlat,
    algNAKMER,
    algFuseNAKMER,

    algPileup_NCBI,
    algPileup_EBI,
    algPileup_DDBJ,

    /* leave as last value */
    algUnknown
} VResolverAlgID;

#define versSDL2 1
typedef uint8_t VERSNS;

rc_t VResolverAlgMake(struct VResolverAlg **alg, const struct String *root,
     VResolverAppID app_id, VResolverAlgID alg_id, bool protctd, bool disabled);

void CC VResolverAlgWhack ( void *item, void *ignore );

rc_t VResolverAlgParseResolverCGIResponse ( const struct KDataBuffer *result,
    const struct VPath ** path, const struct VPath ** mapping,
    const struct String *acc, const struct String *ticket );

rc_t VResolverAlgParseResolverCGIResponse_3_0(const char *start,
    size_t size, const struct VPath **path, const struct VPath **mapping,
    const struct String *acc, const struct String *ticket);

/* RemoteProtectedResolve
 *  use NCBI CGI to resolve accession into URL
 */
rc_t VResolverAlgRemoteProtectedResolve( const struct VResolverAlg *self,
    const struct KNSManager *kns, VRemoteProtocols protocols,
    const struct String *acc, const struct VPath **path,
    const struct VPath **mapping, bool legacy_wgs_refseq, const char * version);

/** get projectId ( valid for protected user repository ) */
rc_t VResolverGetProjectId ( const VResolver * self, uint32_t * projectId );

/* RemoteResolve
 *  resolve an accession into a remote VPath or not found
 *  may optionally open a KFile to the object in the process
 *  of finding it
 *
 *  2. determine the type of accession we have, i.e. its "app"
 *  3. search all local algorithms of app type for accession
 *  4. return not found or new VPath
 */
rc_t VResolverRemoteResolve ( const VResolver *self,
    VRemoteProtocols protocols, const struct String * accession,
    const struct VPath ** path, const struct VPath **mapping,
    const struct KFile ** opt_file_rtn, bool refseq_ctx, bool is_oid, const char * version );

/* version of name service protocol */
rc_t VResolverSetVersion ( VResolver *self, const char * version );

/* resolve oid->file mapping inside of VFS
  resolve (resolve oid<->name mapping in resolver):
   0: default VResolver's behavior
   1: resolve
   2: don't resolve */
rc_t VResolverResolveName ( VResolver *self, int resolve );

bool VResolverResolveToAd(const VResolver *self);

/*rc_t VFSManagerMakeDbgapResolver(const struct VFSManager * self,
    VResolver ** new_resolver, const struct KConfig * cfg,
    const struct KNgcObj * ngc);*/

/* default behavior to resolve oid->file mapping inside of VFS */
#define DEFAULT_RESOVE_OID_NAME true

void KConfigReadRemoteProtocols ( struct KConfig const * self, VRemoteProtocols * remote_protos );

VResolverAppID get_accession_app(const String * accession, bool refseq_ctx,
    struct VResolverAccToken *tok, bool *legacy_wgs_refseq,
    bool resolveAllAccToCache, bool * forDirAdjusted,
    const String * parentAcc, int64_t projectId);

#ifdef __cplusplus
}
#endif

#endif /* _h_resolver_priv_ */
