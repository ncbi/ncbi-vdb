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

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
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
    vpuri_ftp,
    vpuri_ncbi_legrefseq,
    vpuri_ncbi_obj,     /* for dbGaP objects */
    vpuri_fasp,         /* for Aspera downloads */
    vpuri_count
} VPUri_t;

#define VPathGetScheme_t LegacyVPathGetScheme_t
VFS_EXTERN rc_t CC VPathGetScheme_t ( const VPath * self, VPUri_t * uri_type );

#define VPathGetUri_t LegacyVPathGetUri_t
VPUri_t VPathGetUri_t (const VPath * self);


#ifdef __cplusplus
}
#endif

#endif /* _h_path_priv_ */
