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

#ifndef _h_kfg_properties_
#define _h_kfg_properties_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_kfg_config_
#include <kfg/config.h>
#endif

#ifndef _h_kfg_ngc_
#include <kfg/ngc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* get/set HTTP proxy path */
KFG_EXTERN rc_t CC KConfig_Get_Http_Proxy_Path
    ( const KConfig *self, char *buffer, size_t buffer_size, size_t *written );
KFG_EXTERN rc_t CC KConfig_Set_Http_Proxy_Path
    ( KConfig *self, const char *value );

/* get/set enabled-state for HTTP proxy */
KFG_EXTERN rc_t CC KConfig_Get_Http_Proxy_Enabled
    ( const KConfig *self, bool *enabled, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_Http_Proxy_Enabled
    ( KConfig *self, bool enabled );

/* get/set priority of environmnet vs. configuration for HTTP proxy */
KFG_EXTERN rc_t CC KConfig_Has_Http_Proxy_Env_Higher_Priority
    ( const KConfig *self, bool *enabled );
KFG_EXTERN rc_t CC KConfig_Set_Http_Proxy_Env_Higher_Priority
    ( KConfig *self, bool enabled );


/* get the %HOME% path from config
 */
KFG_EXTERN rc_t CC KConfig_Get_Home( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );

/* get/set the default path for proposing a path for ngc-imports
 */
KFG_EXTERN rc_t CC KConfig_Get_Default_User_Path( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KConfig_Set_Default_User_Path( const KConfig *self, const char * value );

/* get/set the enabled-state for the 3 repository-categories
 */
KFG_EXTERN rc_t CC KConfig_Get_Remote_Access_Enabled
    ( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Get_Remote_Main_Cgi_Access_Enabled
    ( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Get_Remote_Aux_Ncbi_Access_Enabled
    ( const KConfig *self, bool * enabled );

KFG_EXTERN rc_t CC KConfig_Set_Remote_Access_Enabled( KConfig *self, bool enabled );

KFG_EXTERN rc_t CC KConfig_Get_Site_Access_Enabled( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Set_Site_Access_Enabled( KConfig *self, bool enabled );

KFG_EXTERN rc_t CC KConfig_Get_User_Access_Enabled( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Set_User_Access_Enabled( KConfig *self, bool enabled );

/* get/set the use of all valid certificates
 */
KFG_EXTERN rc_t CC KConfig_Get_Allow_All_Certs( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Set_Allow_All_Certs( KConfig *self, bool enabled );

/* get/set the the cache-enabled-state for the public/protected repositories
 */
KFG_EXTERN rc_t CC KConfig_Get_User_Public_Enabled( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Set_User_Public_Enabled( KConfig *self, bool enabled );

KFG_EXTERN rc_t CC KConfig_Get_User_Public_Cached( const KConfig *self, bool * enabled );
KFG_EXTERN rc_t CC KConfig_Set_User_Public_Cached( KConfig *self, bool enabled );

KFG_EXTERN rc_t CC KConfig_Get_User_Protected_Cached( const KConfig *self, bool * enabled, const char * name );
KFG_EXTERN rc_t CC KConfig_Set_User_Protected_Cached( KConfig *self, bool enabled, const char * name );


/* get/set the the cache-location for the public/protected repositories
 */
KFG_EXTERN rc_t CC KConfig_Get_User_Public_Cache_Location( const KConfig *self,
    char * value, size_t value_size, size_t * written );
KFG_EXTERN rc_t CC KConfig_Set_User_Public_Cache_Location( KConfig *self, const char * value );


/* Query protected repositories
 */
KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryCount( const KConfig *self,
    uint32_t * count );
KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryName( const KConfig *self,
    uint32_t id, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryPathById( const KConfig *self,
    uint32_t id, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KConfigSetProtectedRepositoryPathById( KConfig *self, uint32_t id, const char * value );

KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryIdByName( const KConfig *self,
    const char * name, uint32_t * id );
KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryPathByName( const KConfig *self,
    const char * name, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryDescriptionByName(
    const KConfig *self,
    const char * name, char * buffer, size_t buffer_size, size_t * written );

KFG_EXTERN rc_t CC KConfigDoesProtectedRepositoryExist( const KConfig *self, const char * name, bool * res );


KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryEnabledById( const KConfig *self, uint32_t id, bool * enabled );
KFG_EXTERN rc_t CC KConfigSetProtectedRepositoryEnabledById( KConfig *self, uint32_t id, bool enabled );

KFG_EXTERN rc_t CC KConfigGetProtectedRepositoryCachedById( const KConfig *self, uint32_t id, bool * enabled );
KFG_EXTERN rc_t CC KConfigSetProtectedRepositoryCachedById( KConfig *self, uint32_t id, bool enabled );

/* reads /tools/prefetch/download_to_cache
   return true if not found */
KFG_EXTERN rc_t CC KConfig_Get_Prefetch_Download_To_Cache ( const KConfig *self, bool * download_to_cache );
KFG_EXTERN rc_t CC KConfig_Set_Prefetch_Download_To_Cache ( KConfig *self, bool download_to_cache );


/* reads /libs/cloud/accept_aws_charges
   returns false if not found  */
KFG_EXTERN rc_t CC KConfig_Get_User_Accept_Aws_Charges ( const KConfig *self, bool * accepts_charges );
KFG_EXTERN rc_t CC KConfig_Set_User_Accept_Aws_Charges ( KConfig *self, bool accepts_charges );

/* reads /libs/cloud/accept_gcp_charges
   returns false if not found  */
KFG_EXTERN rc_t CC KConfig_Get_User_Accept_Gcp_Charges ( const KConfig *self, bool * accepts_charges );
KFG_EXTERN rc_t CC KConfig_Set_User_Accept_Gcp_Charges ( KConfig *self, bool accepts_charges );

/* reads /libs/cloud/report_instance_identity
   returns false if not found  */
KFG_EXTERN rc_t CC KConfig_Get_Report_Cloud_Instance_Identity ( const KConfig *self, bool * value );
KFG_EXTERN rc_t CC KConfig_Set_Report_Cloud_Instance_Identity ( KConfig *self, bool value );

/* reads /libs/temp_cache */
KFG_EXTERN rc_t CC KConfig_Get_Temp_Cache ( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KConfig_Set_Temp_Cache ( KConfig *self, const char * value );

/* reads /gcp/credential_file */
KFG_EXTERN rc_t KConfig_Get_Gcp_Credential_File ( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t KConfig_Set_Gcp_Credential_File ( KConfig *self, const char * value );

/* reads /aws/credential_file */
KFG_EXTERN rc_t KConfig_Get_Aws_Credential_File ( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t KConfig_Set_Aws_Credential_File ( KConfig *self, const char * value );

/* reads /aws/profile, returns "default" if missing or empty */
KFG_EXTERN rc_t KConfig_Get_Aws_Profile ( const KConfig *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t KConfig_Set_Aws_Profile ( KConfig *self, const char * value );

/* reads /libs/cache_amount, returns 0 if missing or empty */
KFG_EXTERN rc_t CC KConfig_Get_Cache_Amount ( const KConfig *self, uint32_t * value );
KFG_EXTERN rc_t CC KConfig_Set_Cache_Amount( KConfig *self, uint32_t value );

/* getters/setters for cachetee-integration into vfs-manager */
KFG_EXTERN rc_t CC KConfig_Get_CacheTeeVersion( const KConfig *self, uint32_t * value, uint32_t dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheTeeVersion( KConfig *self, uint32_t value );
KFG_EXTERN rc_t CC KConfig_Get_CacheBlockSize( const KConfig *self, size_t * value, size_t dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheBlockSize( KConfig *self, size_t value );
KFG_EXTERN rc_t CC KConfig_Get_CachePageCount( const KConfig *self, uint32_t * value, uint32_t dflt );
KFG_EXTERN rc_t CC KConfig_Set_CachePageCount( KConfig *self, uint32_t value );
KFG_EXTERN rc_t CC KConfig_Get_CacheClusterFactorBits( const KConfig *self, uint32_t * value, uint32_t dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheClusterFactorBits( KConfig *self, uint32_t value );
KFG_EXTERN rc_t CC KConfig_Get_CachePageSizeBits( const KConfig *self, uint32_t * value, uint32_t dflt );
KFG_EXTERN rc_t CC KConfig_Set_CachePageSizeBits( KConfig *self, uint32_t value );
KFG_EXTERN rc_t CC KConfig_Get_CacheLogUseCWD( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheLogUseCWD( KConfig *self, bool value );
KFG_EXTERN rc_t CC KConfig_Get_CacheLogAppend( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheLogAppend( KConfig *self, bool value );
KFG_EXTERN rc_t CC KConfig_Get_CacheLogTimed( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheLogTimed( KConfig *self, bool value );
KFG_EXTERN rc_t CC KConfig_Get_CacheLogOuter( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheLogOuter( KConfig *self, bool value );
KFG_EXTERN rc_t CC KConfig_Get_CacheLogInner( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheLogInner( KConfig *self, bool value );

KFG_EXTERN rc_t CC KConfig_Get_CacheDebug( const KConfig *self, bool * value, bool dflt );
KFG_EXTERN rc_t CC KConfig_Set_CacheDebug( KConfig *self, bool value );

KFG_EXTERN rc_t CC KConfig_Get_GUID( const KConfig *self, char * value, size_t value_size, size_t * written );
KFG_EXTERN rc_t CC KConfig_Set_GUID( KConfig *self, const char * value );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_properties_ */
