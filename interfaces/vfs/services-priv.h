#ifndef _h_vfs_services_priv_
#define _h_vfs_services_priv_

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


#include <vdb/quality.h> /* VQuality */
#include <vfs/services.h> /* KService */


#ifdef __cplusplus
extern "C" {
#endif


struct KNSManager;

rc_t KServiceMakeWithMgr(KService ** self, const struct VFSManager * vMgr,
    const struct KNSManager * mgr, const struct KConfig * kfg);

rc_t KServiceResolve(KService * self, bool local, bool remote);

rc_t KServiceNamesQueryExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const char * outDir,
    const char * outFile, const KSrvResponse ** response );

rc_t KServiceNamesExecuteExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const KSrvResponse ** result );

rc_t KServiceSearchExecuteExt ( KService * self,
    const char * cgi, const char * version,
    const struct Kart ** result );


rc_t KServiceTestNamesExecuteExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version,
    const KSrvResponse ** result, const char * expected );


rc_t KServiceTestNamesQueryExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const KSrvResponse ** result,
    const char * dir, const char * file, const char * expected );


/* Get/Set quality type in service request */
rc_t KServiceGetQuality(const KService * self, const char ** quality);
rc_t KServiceSetQuality(KService * self, const char * quality);


rc_t KService1Search ( const struct KNSManager * mgr, const char * cgi,
    const char * acc, const struct Kart ** result );


rc_t KSrvRespFileGetHttp ( const KSrvRespFile * self,
                           const struct VPath ** path );

/* DON'T FREE RETURNED STRINGS !!! */
rc_t KSrvRespFileGetName(const struct KSrvRespFile * self, const char ** name);


rc_t SraDescConvert(struct KDirectory * dir, const char * path,
    bool * recognized);

rc_t SraDescLoadQuality(const String * sra, VQuality * quality);
rc_t SraDescSaveQuality(const String * sra, VQuality quality);

#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_services_priv_ */
