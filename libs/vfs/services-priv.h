#ifndef _h_libs_vfs_services_priv_
#define _h_libs_vfs_services_priv_


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


#include <kfg/kart.h>      /* EObjectType */
#include <vfs/resolver.h> /* VRemoteProtocols */
#include <vfs/services.h> /* ESrvFileFormat */


#ifdef __cplusplus
extern "C" {
#endif


struct KSrvRespObj;
struct KNSManager;
struct KRepositoryMgr;
struct KService;
struct KSrvRespFile;
struct KSrvResponse;
struct Locations;
struct Response4;
struct VPath;
struct VPathSet;

#define VERSION_3_0 0x03000000
#define VERSION_4_0 0x04000000

ver_t InitVersion(const char * src, const String * ticket);

/* make name service call : request: 1 object, response: 1 object */
VFS_EXTERN
rc_t CC KService1NameWithVersion ( const struct KNSManager * mgr,
    const char * cgi_url, const char * acc, size_t acc_sz,
    const char * ticket, VRemoteProtocols protocols,
    const struct VPath ** remote, const struct VPath ** mapping,
    bool refseq_ctx, const char * names_version, bool aProtected );


/******************************** KSrvResponse ********************************/
rc_t KServiceGetKSrvResponse( struct KService * self, struct KSrvResponse ** r);

rc_t KSrvResponseMake ( struct KSrvResponse ** self );
rc_t KSrvResponseAddRef ( const struct KSrvResponse * self );
rc_t KSrvResponseAppend ( struct KSrvResponse * self,
                          const struct VPathSet * set );
rc_t KSrvResponseAddLocalAndCache ( struct KSrvResponse * self, uint32_t idx,
                                    const struct VPathSet * localAndCache );
rc_t KSrvResponseAddLocalAndCacheToTree(
    struct KSrvResponse * self, const struct KSrvRespFile * file);
rc_t KSrvResponseGet ( const struct KSrvResponse * self, uint32_t idx,
                       const struct VPathSet ** set );

/* DON'T RELEASE RETURNED id-s !!! */
rc_t KSrvResponseGetIds ( const struct KSrvResponse * self, uint32_t idx,
                          const char ** reqId, const char ** respId );

rc_t KSrvResponseGetMapping(const KSrvResponse * self, uint32_t idx,
        const struct VPath ** mapping, const struct VPath ** vdbcacheMapping);

rc_t KSrvResponseGetOSize(const KSrvResponse * self, uint32_t idx,
    uint64_t * osize);

rc_t KSrvResponseGetR4 ( const struct KSrvResponse * self,
                         struct Response4 ** r );
rc_t KSrvResponseSetR4 ( struct KSrvResponse * self, struct Response4 * r );

rc_t KSrvRespFileAddRef(const struct KSrvRespFile * self);
rc_t KSrvRespFileAddLocalAndCache ( struct KSrvRespFile * file,
                                    const struct VPathSet * localAndCache );

/* DON'T FREE RETURNED STRINGS !!! */
rc_t KSrvRespFileGetAccOrName ( const struct KSrvRespFile * self,
                                const char ** out, const char ** tic);
rc_t KSrvRespFileGetId  ( const struct KSrvRespFile * self, uint64_t * id,
                                                            const char ** tic );
rc_t KSrvRespFileGetMapping(const struct KSrvRespFile * self,
    const struct VPath ** mapping);

rc_t LocationsAddCache ( struct Locations * self,
                         const struct VPath * path, rc_t rc );
rc_t LocationsAddLocal ( struct Locations * self,
                         const struct VPath * path, rc_t rc );

/**************************** KService ****************************************/
/* resolve oid->file mapping inside of VFS:
  resolve (resolve oid<->name mapping in resolver):
   0: default VResolver's behavior
   1: resolve
   2: don't resolve */
rc_t KServiceResolveName ( struct KService * service, int resolve );

/* call to set VResolverCacheEnable to vrAlwaysEnable
   to simulate prefetch mode
void KServiceSetCacheEnable(struct KService * self,
    VResolverEnableState enable);
VResolverEnableState KServiceGetCacheEnable(const struct KService * self);*/

/**************************** KServiceNamesExecute ****************************/
/* Execute Names Service Call using current default protocol version;
   get KSrvResponse (remote-only resolution) */
rc_t KServiceNamesExecute ( struct KService * self, VRemoteProtocols protocols, 
                            const struct KSrvResponse ** response );

rc_t KServiceNamesExecuteExtImpl ( struct KService * self,
    VRemoteProtocols protocols, const char * cgi, const char * version,
    const struct KSrvResponse ** response, const char * expected );

/***************** Interface services.c -> remote-services.c  *****************/
rc_t KServiceGetConfig ( struct KService * self, const struct KConfig ** kfg);
rc_t KServiceGetVFSManager ( const KService * self,
    const struct VFSManager ** mgr );
rc_t KServiceGetResolver ( struct KService * self, const String * ticket,
                           VResolver ** resolver );
rc_t KServiceGetResolverForProject(struct KService * self, uint32_t project,
    VResolver ** resolver);
int KServiceGetResolveName ( const struct KService * self );

/* don't release returned mgr */
rc_t KServiceGetRepoMgr(KService * self, const struct KRepositoryMgr ** mgr);

const struct KNgcObj * KServiceGetNgcFile(const KService * self,
    bool * isProtected);
/******************************** TESTS ***************************************/
typedef struct {
    const char * id;
    EObjectType type;
    const char * ticket;
} SServiceRequestTestData;

rc_t KServiceCgiTest1 ( const struct KNSManager * mgr, const char * cgi,
    const char * version, const char * acc, const char * ticket,
    VRemoteProtocols protocols, EObjectType objectType,
    const struct VPath * exp, const struct VPath * ex2 );

rc_t KServiceFuserTest ( const struct KNSManager * mgr,  const char * ticket,
    const char * acc, ... );

rc_t SCgiRequestPerformTestNames1 ( const struct KNSManager * mgr,
    const char * cgi, const char * version, const char * acc,
    const char * ticket, VRemoteProtocols protocols, EObjectType objectType );
rc_t KServiceProcessStreamTestNames1 ( const struct KNSManager * mgr,
    const char * b, const char * version, const char * acc,
    const struct VPath * exp, const char * ticket, const struct VPath * ex2,
    int errors );
rc_t KServiceRequestTestNames1 ( const struct KNSManager * mgr,
    const char * cgi, const char * version, const char * acc, size_t acc_sz,
    const char * ticket, VRemoteProtocols protocols,
    EObjectType objectType );

/* Parse "buffer" as names-3.0 response.
   Do not log "errorsToIgnore" messages during response processing */
rc_t KServiceNames3_0StreamTest ( const char * buffer,
    const struct KSrvResponse ** response, int errorsToIgnore );
rc_t KServiceNames3_0StreamTestMany ( const char * buffer,
    const struct KSrvResponse ** response, int errorsToIgnore,
    int itemsInRequest );
rc_t KServiceNamesRequestTest ( const struct KNSManager * mgr, const char * b,
    const char * cgi, VRemoteProtocols protocols,
    const SServiceRequestTestData * d, ... );

rc_t KServiceSearchTest1
    ( const struct KNSManager * mgr, const char * cgi, const char * acc );
rc_t KServiceSearchTest (
    const struct KNSManager * mgr, const char * cgi, const char * acc, ... );
/******************************************************************************/

/* THE FOLLOWING DEFINE TURNS ON COMPARING OLD/NEW RESOLVING CALLS AND
   ASSERTING WHEN THE RESULTS DO NOT MATCH.
   REMOVE IT WHEN MERGING THE BRANCH 
#define TESTING_SERVICES_VS_OLD_RESOLVING 1 */


#ifdef __cplusplus
}
#endif


#endif /* _h_libs_vfs_services_priv_ */
