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

#ifndef _h_path_priv_
#define _h_path_priv_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_klib_data_buffer_
#include <klib/data-buffer.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifndef _h_vfs_path_
#include <vfs/path.h>
#endif

#ifndef _h_vfs_resolver_
#include <vfs/resolver.h> /* VRemoteProtocols */
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KSrvError;

/* VPath Type: -----------------------------------------------------------------
 * how many extended properties ( from name resolver response ) are initialized
 */
typedef enum {
    eVPnoExt,  /* does not have extended part */
    eVPWithId, /* has object-id */
    eVPext,    /* has all extanded properties */
} EVPathType;
typedef enum {
    eSCundefined,
    eSCrun,
    eSCpileup,
    eSCrealign,
    eSCvdbcache,
} ESraClass;
/*
* VPath
*/
struct VPath
{
    KDataBuffer data;

    String scheme;
    String auth;
    String host;
    String portname;
    String path;
    String query;
    String fragment;

    KRefcount refcount;

    uint32_t obj_id;
    uint32_t acc_code;

    uint32_t ipv4;
    uint16_t ipv6 [ 8 ];
    uint16_t portnum;

    uint8_t scheme_type;
    uint8_t host_type;
    uint8_t path_type;

    bool from_uri;
    bool missing_port;
    bool highly_reliable;

    ESraClass sraClass; /* type of SRA file: undefined, run, pileup etc */

    /* how many extended properties ( from name resolver response )
       are initialized */
    EVPathType ext;

    String     id;           /* object-id */

    String     tick;         /* dbGaP download ticket */
    uint64_t   osize;        /* object's un-encrypted size in bytes */
    KTime_t    modification; /* object's modification date. 0 if unknown */
    KTime_t    expiration;   /* expiration date of this VPath object.
                                0 if infinite */

    int64_t    projectId;    /* dbGaP project Id:
                                <0 (not set): public URL
                                >=0 (vaoid values): protected URL
                                    N.B. 0 is a valid dbGaP project Id */

    uint8_t    md5 [ 16 ];  /* md5 checksum object's un-encrypted if known */
    bool       has_md5;

    String     service;      /* s3, gs, sra-ncbi, ftp-ncbi, sra-sos, etc. */
    String     objectType;
    String     type;

    String     name;
    String     nameExtension; /* file extension in name. don't free */

    const VPath * vdbcache;
    bool          vdbcacheChecked; /* no need to check vdbcache URL when
                                  vdbcacheChecked = true and vdbcache == NULL */

    bool       ceRequired;
    bool       payRequired;

    const String * accOfParentDb; /* for refseqs */

    bool magic; /* was created from magic env.var. */

    /* version of names service returned this VPath: 3.0 or SDL ... */
    uint32_t version; 
};

enum VPathVariant
{
    vpInvalid,
    vpOID,
    vpAccession,
    vpNameOrOID,
    vpNameOrAccession,
    vpName,
    vpRelPath,
    vpUNCPath,
    vpFullPath,
    vpAuth,
    vpHostName,
    vpEndpoint
};

enum VHostVariant
{
    vhDNSName,
    vhIPv4,
    vhIPv6
};

/* legacy support */
#define VPathMake LegacyVPathMake
VFS_EXTERN rc_t VPathMake ( VPath ** new_path, const char * posix_path );
#define VPathMakeFmt LegacyVPathMakeFmt
rc_t VPathMakeFmt ( VPath ** new_path, const char * fmt, ... );
#define VPathMakeVFmt LegacyVPathMakeVFmt
rc_t VPathMakeVFmt ( VPath ** new_path, const char * fmt, va_list args );
VFS_EXTERN rc_t LegacyVPathMakeSysPath ( VPath ** new_path, const char * sys_path );

typedef enum eVPUri_t
{
    vpuri_invalid = -1,
    vpuri_none, 
    vpuri_not_supported,
    vpuri_ncbi_file,
    vpuri_ncbi_vfs = vpuri_ncbi_file,
    vpuri_file,
    vpuri_ncbi_acc,
    vpuri_http,
    vpuri_https,
    vpuri_ftp,
    vpuri_ncbi_legrefseq,
    vpuri_ncbi_obj,     /* for dbGaP objects */
    vpuri_fasp,         /* for Aspera downloads */
    vpuri_s3,
    vpuri_azure,
    vpuri_google,
    vpuri_scp,
    vpuri_sftp,
    vpuri_count
} VPUri_t;

#define VPathGetScheme_t LegacyVPathGetScheme_t
VFS_EXTERN rc_t CC VPathGetScheme_t ( const VPath * self, VPUri_t * uri_type );

#define VPathGetUri_t LegacyVPathGetUri_t
VPUri_t VPathGetUri_t (const VPath * self);

/* returns true when VPath is protected */
bool VPathGetProjectId(const VPath * self, uint32_t * projectId);

/* projectId:
 *  <0  : pubkic URL
 *  >=0 : protected URL
 *      N.B. 0 is a valid protected project Id
 *
 * version: version of names service that returned this URL
 */
rc_t VPathMakeFromUrl ( VPath ** new_path, const String * url,
    const String * tick, bool ext, const String * id, uint64_t osize,
    KTime_t date, const uint8_t md5 [ 16 ], KTime_t exp_date,
    const char * service, const String * objectType, const String * type,
    bool ceRequired, bool payRequired, const char * name,
    int64_t projectId, uint32_t version );

rc_t VPathAttachVdbcache(VPath * self, const VPath * vdbcache);

rc_t VPathSetId(VPath * self, const String * id);
rc_t VPathSetMagic(VPath * self, bool magic);

/* Equal
 *  compares two VPath-s
 *
 * "notequal" [ OUT ] - is set
 *  to union of bits corresponding to difference in different VPath properties
 *
 *  returns non-0 rc after a failed call to get property from any of VPath-s
 */
rc_t VPathEqual ( const VPath * l, const VPath * r, int * notequal );
/* Close
 *  compares two VPath-s
 *  difference between expirations should be withing expirationRange */
rc_t VPathClose ( const VPath * l, const VPath * r, int * notequal,
                  KTime_t expirationRange );


/***** VPathSet - set of VPath's - genetated from name resolver response ******/

typedef struct VPathSet VPathSet;

rc_t VPathSetRelease ( const VPathSet * self );
rc_t VPathSetGet ( const VPathSet * self, VRemoteProtocols protocols,
    const struct VPath ** path, const struct VPath ** vdbcache );
rc_t VPathSetGetCache ( const VPathSet * self, const struct VPath ** path );
rc_t VPathSetGetLocal ( const VPathSet * self, const struct VPath ** path );

/* name resolver response row converted into VDB objects */
typedef struct {          /*       vdbcache */
    struct VPath * fasp ; struct VPath * vcFasp;
    struct VPath * file ; struct VPath * vcFile;
    struct VPath * http ; struct VPath * vcHttp;
    struct VPath * https; struct VPath * vcHttps;
    struct VPath * s3   ; struct VPath * vcS3;

    struct VPath * mapping  ; /* run/file mapping */
    struct VPath * vcMapping; /* vdbcache mapping */

    const struct KSrvError * error;
    char * reqId;
    char * respId;
    uint64_t osize; /*size of VPath object */
} EVPath;

rc_t VPathSetMake
    ( VPathSet ** self, const EVPath * src, bool singleUrl );

rc_t VPathSetMakeQuery ( VPathSet ** self, const VPath * local, rc_t localRc,
                         const VPath * cache, rc_t cacheRc );

rc_t KSrvErrorMake4(const struct KSrvError ** self,
                    rc_t rc, uint32_t code, const char * msg);


#ifdef __cplusplus
}
#endif

#endif /* _h_path_priv_ */

#if 0
/******************************** KSrvResponse ********************************/
rc_t KSrvResponseRelease ( const KSrvResponse * self );
uint32_t KSrvResponseLength ( const KSrvResponse * self );
/******************************************************************************/
#endif
