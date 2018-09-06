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


#ifdef __cplusplus
extern "C" {
#endif


struct KSrvRespObj;
struct KNSManager;
struct KService;
struct KSrvResponse;
struct Response4;
struct VPathSet;


/* make name service call : request: 1 object, response: 1 object */
VFS_EXTERN
rc_t CC KService1NameWithVersion ( const struct KNSManager * mgr,
    const char * cgi_url, const char * acc, size_t acc_sz,
    const char * ticket, VRemoteProtocols protocols,
    const struct VPath ** remote, const struct VPath ** mapping,
    bool refseq_ctx, const char * names_version, bool aProtected );


/******************************** KSrvResponse ********************************/
rc_t KSrvResponseMake ( struct KSrvResponse ** self );
rc_t KSrvResponseAddRef ( const struct KSrvResponse * self );
rc_t KSrvResponseAppend ( struct KSrvResponse * self,
                          const struct VPathSet * set );
rc_t KSrvResponseAddLocalAndCache ( struct KSrvResponse * self, uint32_t idx,
                                    const struct VPathSet * localAndCache );
rc_t KSrvResponseGet ( const struct KSrvResponse * self, uint32_t idx,
                       const struct VPathSet ** set );
rc_t KSrvResponseSetR4 ( struct KSrvResponse * self,
                         const struct Response4 * r );

/**************************** KService ****************************************/
/* resolve oid->file mapping inside of VFS:
  resolve (resolve oid<->name mapping in resolver):
   0: default VResolver's behavior
   1: resolve
   2: don't resolve */
rc_t KServiceResolveName ( struct KService * service, int resolve );

/**************************** KServiceNamesExecute ****************************/
/* Execute Names Service Call using current default protocol version;
   get KSrvResponse (remote-only resolution) */
rc_t KServiceNamesExecute ( struct KService * self, VRemoteProtocols protocols, 
                            const struct KSrvResponse ** response );
/***************** Interface services.c -> remote-services.c  *****************/
rc_t KServiceGetConfig ( struct KService * self, const struct KConfig ** kfg);
rc_t KServiceGetResolver ( struct KService * self, const String * ticket,
                           VResolver ** resolver );
int KServiceGetResolveName ( const struct KService * self );
/******************************** TESTS ********************************/
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
#define TESTING_SERVICES_VS_OLD_RESOLVING 1
*/

#ifdef __cplusplus
}
#endif


#endif /* _h_libs_vfs_services_priv_ */
