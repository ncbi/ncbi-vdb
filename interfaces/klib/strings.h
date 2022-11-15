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
* ==============================================================================
*
*/

#ifndef _h_klib_strings_
#define _h_klib_strings_

#ifdef __cplusplus
extern "C" {
#endif

#define ENV_MAGIC_CACHE  "VDB_CACHE_URL"
#define ENV_MAGIC_CACHE_VDBCACHE  "VDB_CACHE_VDBCACHE"
#define ENV_MAGIC_LOCAL  "VDB_LOCAL_URL"
#define ENV_MAGIC_LOCAL_VDBCACHE  "VDB_LOCAL_VDBCACHE"
#define ENV_MAGIC_REMOTE "VDB_REMOTE_URL"
#define ENV_MAGIC_REMOTE_VDBCACHE "VDB_REMOTE_VDBCACHE"

#define ENV_MAGIC_REMOTE_NEED_CE  "VDB_REMOTE_NEED_CE"
#define ENV_MAGIC_REMOTE_NEED_PMT "VDB_REMOTE_NEED_PMT"

#define ENV_MAGIC_CACHE_NEED_CE  "VDB_CACHE_NEED_CE"
#define ENV_MAGIC_CACHE_NEED_PMT "VDB_CACHE_NEED_PMT"

#define ENV_MAGIC_CE_TOKEN "VDB_CE_TOKEN"
#define ENV_MAGIC_OPT_BITMAP "VDB_OPT_BITMAP"
#define ENV_MAGIC_PLATFORM_NAME "NCBI_USAGE_PLATFORM_NAME"
#define ENV_MAGIC_PLATFORM_VERSION "NCBI_USAGE_PLATFORM_VERSION"

#define ENV_VAR_LOG_HTTP_RETRY "NCBI_VDB_LOG_HTTP_RETRY"
#define ENV_VAR_SESSION_ID "VDB_SESSION_ID"

#define ENV_VAR_QUALITY_PREFERENCE "NCBI_VDB_QUALITY"
#define ENV_VAR_PARAMETER_BITS "VDB_OPT_BITMAP"


#define SDL_CGI "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_strings_ */
