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


#include <vfs/resolver.h> /* VRemoteProtocols */


#ifdef __cplusplus
extern "C" {
#endif


struct KNSManager;


/* make name service call : request: 1 object, response: 1 object */
VFS_EXTERN
rc_t CC KService1NameWithVersion ( const struct KNSManager * mgr,
    const char * cgi_url, const char * acc, size_t acc_sz,
    const char * ticket, VRemoteProtocols protocols,
    const struct VPath ** remote, const struct VPath ** mapping,
    bool refseq_ctx, const char * names_version );


#ifdef __cplusplus
}
#endif


#endif /* _h_libs_vfs_services_priv_ */
