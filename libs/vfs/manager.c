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

#include <vfs/extern.h>

#include <sra/srapath.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>
#include <vfs/resolver.h>

#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/wgaencrypt.h>
#include <krypto/ciphermgr.h>

#include <kfg/config.h>
#include <kfg/repository.h>
#include <kfg/properties.h>
#include <kfg/keystore.h>
#include <kfg/keystore-priv.h>
#include <kfg/kfg-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/sra.h>
#include <kfs/tar.h>
#include <kfs/dyload.h>
#include <kfs/kfs-priv.h>
#include <kfs/nullfile.h>
#include <kfs/buffile.h>
#include <kfs/quickmount.h>
#include <kfs/cacheteefile.h>
#include <kfs/cachetee2file.h>
#include <kfs/cachetee3file.h>
#include <kfs/rrcachedfile.h>
#include <kfs/recorder.h>
#include <kfs/lockfile.h>
#include <kfs/logfile.h>
#include <kfs/defs.h>

#include <kns/http.h>
#include <kns/http-priv.h> 
#include <kns/kns-mgr-priv.h>
#include <kns/manager.h>

#include <kxml/xml.h>

#include <klib/debug.h>
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/out.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/strings.h> /* ENV_VDB_REMOTE_NEED_CE */
#include <klib/time.h> 
#include <klib/vector.h>

#include "path-priv.h"
#include "resolver-priv.h"

#include <strtol.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>


#if _DEBUGGING
#define MGR_DEBUG(msg) DBGMSG(DBG_VFS,DBG_FLAG(DBG_VFS_MGR), msg)
#else
#define MGR_DEBUG(msg)
#endif

#define VFS_KRYPTO_PASSWORD_MAX_SIZE 4096

/*--------------------------------------------------------------------------
 * VFSManager
 */

/* currently expected to be a singleton and not use a vtable but
 * be fully fleshed out here */
struct VFSManager
{
    /* the current directory in the eyes of the O/S when created */
    KDirectory * cwd;

    /* configuration manager */
    KConfig * cfg;

    /* krypto's cipher manager */
    KCipherManager * cipher;

    /* SRAPath will be replaced with a VResolver */
    struct VResolver * resolver;

    /* network manager */
    KNSManager * kns;

    /* path to a global password file */
    char *pw_env;
    
    /* encryption key storage */ 
    struct KKeyStore* keystore;

    KRefcount refcount;

    VRemoteProtocols protocols;
};

static const char kfsmanager_classname [] = "VFSManager";

static 
VFSManager * singleton = NULL;


/* Destroy
 *  destroy file
 */
static rc_t VFSManagerDestroy ( VFSManager *self )
{
    if ( self == NULL )
        return RC ( rcVFS, rcFile, rcDestroying, rcSelf, rcNull );
    
    KKeyStoreRelease( self -> keystore );
    free ( self -> pw_env );
    VResolverRelease ( self -> resolver );
    KNSManagerRelease ( self -> kns );
    KCipherManagerRelease ( self -> cipher );
    KConfigRelease ( self -> cfg );
    KDirectoryRelease ( self -> cwd );
    KRefcountWhack (&self->refcount, kfsmanager_classname);

    free (self);
    singleton = NULL;

    return 0;
}

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC VFSManagerAddRef ( const VFSManager *self )
{
    if (self != NULL)
    {
        switch (KRefcountAdd (&self->refcount, kfsmanager_classname))
        {
        case krefOkay:
            break;
        case krefZero:
            return RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcIncorrect);
        case krefLimit:
            return RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcExhausted);
        case krefNegative:
            return RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            return RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
        }
    }
    return 0;
}

/* Release
 *  discard reference to file
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC VFSManagerRelease ( const VFSManager *self )
{
    rc_t rc = 0;
    if (self != NULL)
    {
        switch (KRefcountDrop (&self->refcount, kfsmanager_classname))
        {
        case krefOkay:
        case krefZero:
            break;
        case krefWhack:
            rc = VFSManagerDestroy ((VFSManager*)self);
            break;
        case krefNegative:
            return RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            rc = RC (rcVFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
            break;            
        }
    }
    return rc;
}

enum cache_version
{ cachetee = 0, cachetee_2 = 1, cachetee_3 = 2, rrcache = 3, logging = 4 };

typedef struct caching_params
{
    enum cache_version version;
    size_t cache_page_size;
    uint32_t cache_page_count;
    uint32_t cluster_factor_bits; /* for cachetee_v3 dflt = 5,  1 << 5  = 32 */
    uint32_t page_size_bits;      /* for cachetee_v3 dflt = 15, 1 << 15 = 64k */
    uint32_t cache_amount_mb;     /* for cachetee_v3 dlft = 32 MB */
    
    char temp_cache[ 4096 ];
    
    bool use_file_cache;    /* is caching turned on */
    bool use_cwd;       /* use the current working directory if not cach-location is given */
    bool append;        /* append to existing recording 0...no - 1...yes */
    bool timed;         /* record timing 0...no - 1...yes */
    bool record_inner;  /* record the request made before the cache */    
    bool record_outer;  /* record the request made after the cache */
    bool is_refseq;     /* when used for external reference sequences, decrease cache size */
    bool promote;       /* do we want a promoting cache-tee-file ? */
    bool debug;         /* print cache-debug-messages */
} caching_params;

#define DEFAULT_CACHETEE_VERSION cachetee_3
#define DEFAULT_CACHE_PAGE_SIZE ( 32 * 1024 )
#define DEFAULT_CACHE_PAGE_COUNT ( 10 * 1024 )
#define DEFAULT_CLUSTER_FACTOR_BITS 5
#define DEFAULT_PAGE_SIZE_BITS 15
#define DEFAULT_CACHE_AMOUNT_MB 256

static void get_caching_params( caching_params * params,
                uint32_t dflt_block_size,
                bool is_refseq,
                bool promote )
{
    KConfig * cfg;
    rc_t rc = KConfigMake ( &cfg, NULL );

    /* set some default values... */
    params -> version = DEFAULT_CACHETEE_VERSION;
    params -> cache_page_size = dflt_block_size;
    params -> cache_page_count = DEFAULT_CACHE_PAGE_COUNT;
    params -> cluster_factor_bits = DEFAULT_CLUSTER_FACTOR_BITS;
    params -> page_size_bits = DEFAULT_PAGE_SIZE_BITS;
    params -> cache_amount_mb = DEFAULT_CACHE_AMOUNT_MB;
    params -> temp_cache[ 0 ] = 0;
    params -> use_file_cache = false;
    params -> use_cwd = false;
    params -> append = false;
    params -> timed = false;
    params -> record_inner = false;    
    params -> record_outer = false;
    params -> is_refseq = is_refseq;
    params -> promote = promote;
    params -> debug = false;
    
    if ( rc == 0 )
    {
        size_t written;
        
        /* functions in libs/kfg/properties.c */
        rc = KConfig_Get_CacheTeeVersion( cfg, &( params -> version ), DEFAULT_CACHETEE_VERSION );
        if ( rc != 0 )
            params -> version = DEFAULT_CACHETEE_VERSION;
        
        rc = KConfig_Get_CacheBlockSize( cfg, &( params -> cache_page_size ), dflt_block_size );
        if ( rc != 0 )
            params -> cache_page_size = dflt_block_size;
        
        rc = KConfig_Get_CachePageCount( cfg, &( params -> cache_page_count ), DEFAULT_CACHE_PAGE_COUNT );
        if ( rc != 0 )
            params -> cache_page_count = DEFAULT_CACHE_PAGE_COUNT;
        
        rc = KConfig_Get_CacheClusterFactorBits( cfg, &( params -> cluster_factor_bits ), DEFAULT_CLUSTER_FACTOR_BITS );
        if ( rc != 0 )
            params -> cluster_factor_bits = DEFAULT_CLUSTER_FACTOR_BITS;        

        rc = KConfig_Get_CachePageSizeBits( cfg, &( params -> page_size_bits ), DEFAULT_PAGE_SIZE_BITS );
        if ( rc != 0 )
            params -> page_size_bits = DEFAULT_PAGE_SIZE_BITS;
        
        rc = KConfig_Get_Cache_Amount( cfg, &( params -> cache_amount_mb ) );
        if ( rc == 0 )
        {
            if ( params -> cache_amount_mb == 0 || params -> cache_amount_mb < DEFAULT_CACHE_AMOUNT_MB )
                params -> cache_amount_mb = DEFAULT_CACHE_AMOUNT_MB;
        }
        else
            params -> cache_amount_mb = DEFAULT_CACHE_AMOUNT_MB;
        
        rc = KConfig_Get_Temp_Cache( cfg, params -> temp_cache, sizeof( params -> temp_cache ), &written );
        if ( rc != 0 )
            params -> temp_cache[ 0 ] = 0;

        rc = KConfig_Get_User_Public_Cached( cfg, &( params -> use_file_cache ) );
        if ( rc != 0 )
            params -> use_file_cache = false;
        
        rc = KConfig_Get_CacheLogUseCWD( cfg, &( params -> use_cwd ), false );
        if ( rc != 0 )
            params -> use_cwd = false;
        
        rc = KConfig_Get_CacheLogAppend( cfg, &( params -> append ), false );
        if ( rc != 0 )
            params -> append = false;
        
        rc = KConfig_Get_CacheLogTimed ( cfg, &( params -> timed ), false );
        if ( rc != 0 )
            params -> timed = 0;
        
        rc = KConfig_Get_CacheLogOuter( cfg, &( params -> record_outer ), false );
        if ( rc != 0 )
            params -> record_outer = false;
        
        rc = KConfig_Get_CacheLogInner( cfg, &( params -> record_inner ), false );
        if ( rc != 0 )
            params -> record_inner = false;

        rc = KConfig_Get_CacheDebug( cfg, &( params -> debug ), false );
        if ( rc != 0 )
            params -> debug = false;

        KConfigRelease ( cfg );
    }
}

static const char * extract_acc_from_url( const char * url )
{
    char * res = string_rchr ( url, string_size( url ), '/' );
    if ( res != NULL )
        return ++res;
    return url;
}

static rc_t wrap_in_logfile( KDirectory * dir,
                             const KFile **cfp,
                             const char * loc,
                             const char * fmt,
                             const caching_params * cps )
{
    const KFile * temp_file;
    const char * rec_loc = cps -> use_cwd ? extract_acc_from_url( loc ) : loc;
    rc_t rc = MakeLogFile ( dir,
                            &temp_file,
                            ( KFile * )*cfp,
                            cps -> append > 0,
                            cps -> timed > 0,
                            fmt,
                            rec_loc );
    if ( rc == 0 )
    {
        KFileRelease ( * cfp );
        * cfp = temp_file;
    }
    return rc;
}

static rc_t wrap_in_cachetee( KDirectory * dir,
                              const KFile **cfp,
                              const char * loc,
                              const caching_params * cps )
{
    rc_t rc = 0;
    if ( cps -> record_outer )
        rc = wrap_in_logfile( dir, cfp, loc, "%s.outer.rec", cps );
    if ( rc == 0 )
    {
        const KFile * temp_file;
        if ( cps -> promote )
        {
            rc = KDirectoryMakeCacheTeePromote ( dir,
                                                 &temp_file,
                                                 *cfp,
                                                 cps -> cache_page_size, 
                                                 "%s",
                                                 loc );
        
        }
        else
        {
            rc = KDirectoryMakeCacheTee ( dir,
                                          &temp_file,
                                          *cfp,
                                          cps -> cache_page_size,
                                          "%s",
                                          loc );
        }
        if ( rc == 0 )
        {
            KFileRelease ( * cfp );
            * cfp = temp_file;

            if ( cps -> record_inner )
                rc = wrap_in_logfile( dir, cfp, loc, "%s.inner.rec", cps );
        }
    }
    return rc;
}

static rc_t wrap_in_cachetee2( KDirectory * dir,
                               const KFile **cfp,
                               const char * loc,
                               const caching_params * cps )
{
    rc_t rc = 0;
    if ( cps -> record_outer )
        rc = wrap_in_logfile( dir, cfp, loc, "%s.outer.rec", cps );
    if ( rc == 0 )
    {
        const KFile * temp_file;
        rc_t rc = KDirectoryMakeCacheTee2 ( dir,
                                            &temp_file,
                                            *cfp,
                                            cps -> cache_page_size,
                                            "%s",
                                            loc );
        if ( rc == 0 )
        {
            KFileRelease ( * cfp );
            * cfp = temp_file;
            
            if ( cps -> record_inner )
                rc = wrap_in_logfile( dir, cfp, loc, "%s.inner.rec", cps );
        }
    }
    return rc;
}

static rc_t wrap_in_rr_cache( KDirectory * dir,
                              const KFile **cfp,
                              const char * loc,
                              const caching_params * cps )
{
    rc_t rc = 0;
    if ( cps -> record_outer )
        rc = wrap_in_logfile( dir, cfp, loc, "%s.outer.rec", cps );
    if ( rc == 0 )
    {
        const KFile * temp_file;
        rc_t rc = MakeRRCached ( &temp_file, *cfp, cps -> cache_page_size, cps -> cache_page_count );
        if ( rc == 0 )
        {
            KFileRelease ( * cfp );
            * cfp = temp_file;

            if ( cps -> record_inner )
                rc = wrap_in_logfile( dir, cfp, loc, "%s.inner.rec", cps );
        }
    }
    return rc;
}

#if WINDOWS
    static const char * fallback_cache_location = "c:\\temp";
    const char * get_fallback_cache_location( void )
    {
        return fallback_cache_location;
    }
#else
    static const char * fallback_cache_location = "/var/tmp";
    const char * get_fallback_cache_location( void )
    {
        const char * c = getenv ( "TMPDIR" );
        if ( c != NULL )
            return c;
        return fallback_cache_location;
    }
#endif


static const String * make_id( const VPath * path )
{
    const String * res = NULL;
    
    /* first try to extract a id from the path */
    String path_id = { 0, 0, 0 };
    rc_t rc = VPathGetId ( path, &path_id );
    if ( rc == 0 && path_id . len > 0 )
    {
        rc = StringCopy ( &res, &path_id );
    }
    /* if we have no id now, as a last resort use a timestamp */
    if ( res == NULL )
    {
        size_t num_writ = 0;
        char buffer [ 4096 ];
        static atomic32_t counter;
#ifdef _WIN32
        KTime_t t = KTimeStamp();
        rc = string_printf ( buffer, sizeof buffer, &num_writ
                             , "t_%lu.%u"
                             , t
                             , atomic32_read_and_add ( & counter, 1 )
            );
#else
        uint32_t sys_GetPID ( void );
        uint32_t pid = sys_GetPID ();
        int sys_GetHostName ( char * buffer, size_t buffer_size );
        int status = sys_GetHostName ( buffer, sizeof buffer );
        if ( status != 0 )
        {
            KTime_t t = KTimeStamp();
            rc = string_printf ( buffer, sizeof buffer, &num_writ
                                 , "t%u_%lu.%u"
                                 , pid
                                 , t
                                 , atomic32_read_and_add ( & counter, 1 )
                );
        }
        else
        {
            num_writ = strlen ( buffer );
            rc = string_printf ( & buffer [ num_writ ], sizeof buffer - num_writ, &num_writ
                                 , "-%u.%u"
                                 , pid
                                 , atomic32_read_and_add ( & counter, 1 )
                );
        }
#endif
        if ( rc == 0 )
        {
            String S;
            StringInitCString( &S, buffer );
            rc = StringCopy ( &res, &S );
        }
    }
    return res;
}

static rc_t wrap_in_cachetee3( KDirectory * dir,
                               const KFile **cfp,
                               const char * cache_loc,
                               const caching_params * cps,
                               const VPath * path )
{
    rc_t rc = 0;
    const KFile * temp_file;
    uint32_t cluster_factor = ( 1 << ( cps -> cluster_factor_bits - 1 ) );
    size_t page_size = ( 1 << ( cps -> page_size_bits - 1 ));
    size_t cache_amount = ( ( size_t )cps -> cache_amount_mb * 1024 * 1024 );
    size_t ram_page_count = ( cache_amount + page_size - 1 ) / page_size;
    bool ram_only = true;

    if ( cps -> debug )
    {
        const String * uri = NULL;
        rc_t rc1 = VPathMakeUri ( path, &uri );
        
        KOutMsg( "{\n " );
        KOutMsg( "cache.cluster-factor ... %d\n", cluster_factor );
        KOutMsg( "cache.page_size ........ %d bytes\n", page_size );
        KOutMsg( "cache.amount ........... %d MB\n", cps -> cache_amount_mb );
        KOutMsg( "cache.page_count ....... %d\n", ram_page_count );
        KOutMsg( "cache_loc (resolver) ... %s\n", cache_loc == NULL ? "NULL" : cache_loc );
        if ( rc1 == 0 )
        {
            if ( uri != NULL )
                KOutMsg( "uri : %S\n", uri );
            else
                KOutMsg( "uri : NULL\n" );
            
            StringWhack( uri );
        }
    }
    
    if ( cps -> use_file_cache )
    {
        char location[ 4096 ];
        bool remove_on_close = false;
        bool promote = cps -> promote;
        location[ 0 ] = 0;
    
        if ( cps -> debug )
            KOutMsg( "use file-cache\n" );

        /* if we have been given a location, we use it. CacheTeeV3 can deal with invalid/unreachable ones! */
        if ( cache_loc != NULL )
        {
            rc = KDirectoryResolvePath ( dir, true, location, sizeof location,
                                         "%s", cache_loc );
        }
        
        /* if we have no given location or it does not exist or it is not read/writable for us */
        if ( location[ 0 ] == 0 )
        {
            const String * id = make_id( path );
            if ( id != NULL )
            {
                remove_on_close = true;
                promote = false;
                
                if ( cps -> temp_cache[ 0 ] != 0 )
                {
                    /* we have user given temp cache - location ( do not try promotion, remove-on-close ) */
                    rc = KDirectoryResolvePath ( dir, true, location, sizeof location,
                                                 "%s/%s.sra", cps -> temp_cache, id -> addr );
                }
                else
                {
                    /* fallback to hardcoded path location ( do not try promotion, remove-on-close */
                    rc = KDirectoryResolvePath ( dir, true, location, sizeof location,
                                                 "%s/%s.sra",
                                                 get_fallback_cache_location(),
                                                 id -> addr );
                }
                StringWhack ( id );
            }
            else
                rc = SILENT_RC( rcVFS, rcPath, rcReading, rcFormat, rcInvalid );
        }
        
        if ( cps -> debug )
        {
            KOutMsg( "cache.remove-on-close ... %s\n", remove_on_close ? "Yes" : "No" );
            KOutMsg( "cache.try-promote ....... %s\n", promote ? "Yes" : "No" );            
            KOutMsg( "cache location: '%s', rc = %R\n", location, rc );
        }
        
        if ( rc == 0 )
            /* check if location is writable... */
            rc = KDirectoryMakeKCacheTeeFile_v3 ( dir,
                                                  &temp_file,
                                                  *cfp,
                                                  page_size,
                                                  cluster_factor,
                                                  ram_page_count,
                                                  promote,
                                                  remove_on_close,
                                                  "%s", location );
        ram_only = ( rc != 0 );
    }
    
    if ( ram_only )
    {
        if ( cps -> debug )
            KOutMsg( "use no file-cache\n" );

        rc = KDirectoryMakeKCacheTeeFile_v3 ( dir,
                                              &temp_file,
                                              *cfp,
                                              page_size,
                                              cluster_factor,
                                              ram_page_count,
                                              false,
                                              false,
                                              "" );
    }

    if ( cps -> debug )
        KOutMsg( "}\n" );
    
    if ( rc == 0 )
    {
        KFileRelease ( * cfp );
        * cfp = temp_file;
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * VFSManagerMakeHTTPFile
 
 enum cache_version
 { cachetee = 0, cachetee_2 = 1, cachetee_3 = 2, rrcache = 3, logging = 4 };


 */
static
rc_t VFSManagerMakeHTTPFile( const VFSManager * self,
                             const KFile **cfp,
                             const VPath * path,
                             const char * cache_location,
                             uint32_t blocksize,
                             bool high_reliability,
                             bool is_refseq,
                             bool promote )
{
    const String * uri = NULL;
    rc_t rc = VPathMakeString ( path, &uri );

    if (rc == 0) {
        String objectType;
        String refseq;
        CONST_STRING(&refseq, "refseq");
        rc = VPathGetObjectType(path, &objectType);
        if (rc == 0) {
            if (!is_refseq)
                is_refseq = StringEqual(&objectType, &refseq);
            if (!is_refseq) {
                assert(uri);
                is_refseq = strstr(uri->addr, refseq.addr) != NULL;
            }
        }
    }

    if ( rc == 0 ) {
        bool hasMagic = getenv(ENV_MAGIC_LOCAL);
        bool ceRequired = false;
        bool payRequired = false;
        {
            const char * name = path->sraClass == eSCvdbcache ?
                ENV_MAGIC_CACHE_NEED_CE : ENV_MAGIC_REMOTE_NEED_CE;
            const char * magic = getenv(name);
            if (is_refseq) {
                if (magic != NULL)
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                        "'%s' magic ignored for refseq\n", name));
            }
            else
                if (magic != NULL) {
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                        "'%s' magic found\n", name));
                    ceRequired = true;
                }
                else {
                    ceRequired = path->ceRequired;
                    if (hasMagic)
                        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                            "'%s' magic not set\n", name));
                }
        }
        {
            const char * name = path->sraClass == eSCvdbcache ?
                ENV_MAGIC_CACHE_NEED_PMT : ENV_MAGIC_REMOTE_NEED_PMT;
            const char * magic = getenv(name);
            if (is_refseq) {
                if (magic != NULL)
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                        "'%s' pmtReq magic ignored for refseq\n", name));
            }
            if (magic != NULL) {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic found\n", name));
                payRequired = true;
            }
            else {
                payRequired = path->payRequired;
                if (hasMagic)
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                        "'%s' magic not set\n", name));
            }
        }
        rc = KNSManagerMakeReliableHttpFile ( self -> kns,
                                              cfp,
                                              NULL,
                                              0x01010000,
                                              high_reliability, 
                                              ceRequired,
                                              payRequired,
                                              uri -> addr );

        /* in case we are not able to open the remote-file : return with error-code */
        if ( rc == 0 )
        {
            /* let's try to get some details about how to do caching from the configuration */    
            caching_params cps;
            get_caching_params( &cps, blocksize, is_refseq, promote );
            if ( cps . version == cachetee_3 )
            {
                rc = wrap_in_cachetee3( self -> cwd, cfp, cache_location, &cps, path );
            }
            else
            {
                if ( cache_location == NULL )
                {
                    const String * id = make_id( path );
                    if ( id != NULL )
                    {
                        /* the user has turned off caching... ( we should not make a cache-tee )*/
                        switch( cps . version )
                        {
                            case cachetee   : ;  /* fall-through into rr-cache !!! */
                            case cachetee_2 : ;  /* fall-through into rr-cache !!! */
                            case rrcache    : rc = wrap_in_rr_cache( self -> cwd, cfp, id -> addr, &cps ); break;
                            case logging    : rc = wrap_in_logfile( self -> cwd, cfp, id -> addr, "%s.rec", &cps ); break;
                            case cachetee_3 : break; /* in common path above */
                        }
                        StringWhack ( id );
                    }
                }
                else
                {
                    /* the user has turned on caching... */
                    switch( cps . version )
                    {
                        case cachetee   : rc = wrap_in_cachetee( self -> cwd, cfp, cache_location, &cps ); break;
                        case cachetee_2 : rc = wrap_in_cachetee2( self -> cwd, cfp, cache_location, &cps ); break;
                        case rrcache    : rc = wrap_in_rr_cache( self -> cwd, cfp, cache_location, &cps ); break;
                        case logging    : rc = wrap_in_logfile( self -> cwd, cfp, cache_location, "%s.rec", &cps ); break;
                        case cachetee_3 : break; /* in common path above */
                    }
                }
            }
        }
        free( ( void * )uri );
    }
    return rc;
}

static rc_t CC VFSManagerGetConfigPWFile (const VFSManager * self, char * b, size_t bz, size_t * pz)
{
    const char * env;
    const KConfigNode * node;
    size_t oopsy;
    size_t z = 0;
    rc_t rc;

    if (pz)
        *pz = 0;

    env = getenv (ENV_KRYPTO_PWFILE);
    if (!env)
        env = self->pw_env;
    if (env)
    {
        z = string_copy (b, bz, env, string_size (env));
    
        /* force a NUL that string_copy might have omitted 
         * even if this truncates the path */
        b[bz-1] = '\0';

        if (pz)
            *pz = z;
       
        return 0;
    }
    
    { /* If we are in a protected area, there may be an area-specific key file */
        const KRepositoryMgr *repoMgr;
        rc = KConfigMakeRepositoryMgrRead ( self->cfg, &repoMgr );
        if (rc == 0)
        {
            const KRepository* prot;
            rc = KRepositoryMgrCurrentProtectedRepository ( repoMgr, &prot );
            if (rc == 0)
            {
                rc = KRepositoryEncryptionKeyFile (prot, b, bz, pz);
                if (rc != 0 || b[0] == '\0')
                    rc = KRepositoryEncryptionKey (prot, b, bz, pz);

                KRepositoryRelease(prot);
            }
            KRepositoryMgrRelease(repoMgr);
        }
        if (GetRCState(rc) == rcNotFound)
            rc = RC (rcVFS, rcMgr, rcOpening, rcEncryptionKey, rcNotFound);
    }

    if (rc != 0)
    {   /* fall back on an old-style global password file*/
        rc = KConfigOpenNodeRead (self->cfg, &node, "%s", KFG_KRYPTO_PWFILE);
        if (rc)
        {
            /* if not found, change object from path to encryption key */
            if (GetRCState(rc) == rcNotFound)
                rc = RC (rcVFS, rcMgr, rcOpening, rcEncryptionKey, rcNotFound);
        }
        else
        {
            rc = KConfigNodeRead (node, 0, b, bz-1, &z, &oopsy);
            if (rc == 0)
            {
                if (oopsy != 0)
                    rc = RC (rcKrypto, rcMgr, rcReading, rcBuffer, rcInsufficient);
                else
                {
                    b[z] = '\0';
                    *pz = z;
                }
            }
            KConfigNodeRelease (node);
        }
    }
    
    return rc;
}

static
rc_t GetEncryptionKey(const VFSManager * self, const VPath * vpath, char* obuff, size_t buf_size, size_t *pwd_size)
{
    /* -----
     * #if 0
     * first check the insecure password on the command line hack 
     * #endif 
     *
     * then check the option for pwfile in the VPath
     * then check the option for pwfd
     * then check the keystore. if necessary, keystore will 
     *          check the environment      
     *          check the configuration
     */

#if 0
    /* obviously not used yet */
    if (VPathOption (vpath, vpopt_temporary_pw_hack, obuff, buf_size, &z) == 0)
    {
        if (z < 1)
            rc = RC (rcVFS, rcPath, rcConstructing, rcParam, rcInvalid);
        else
        {
            size_t x = 0;
            size_t y = 0;
            int ch, h, l;

            while (x < z)
            {
                h = tolower(obuff[x++]);
                l = tolower(obuff[x++]);

                if (!isxdigit(h) || !isxdigit(l))
                    rc = RC (rcVFS, rcPath, rcConstructing, rcParam, rcInvalid);

                if (isdigit(h))
                    ch = (h - '0') << 4;
                else
                    ch = (h + 10 - 'a') << 4;
                if (isdigit(l))
                    ch |= (l - '0');
                else
                    ch |= (l + 10 - 'a');

                /* added for compatibility with other passwords */
                if ((ch == '\r') || (ch == '\n'))
                    break;
                obuff[y++] = (char)ch;
            }
            obuff[y] = '\0';
            assert (z == x);
            assert (z/2 == y);
            z = y;
            * pwd_size = z;
        }
    }
#endif    

    rc_t rc = 0;
    rc_t rc2;
    size_t z;

    if (VPathOption (vpath, vpopt_pwpath, obuff, buf_size - 1, &z)
        == 0)
    {
        const KFile * pwfile;
        obuff [z] = '\0';
        rc = KDirectoryOpenFileRead(self->cwd, &pwfile, "%s", obuff);
        if (rc == 0)
        {
            rc = KKeyStoreSetTemporaryKeyFromFile(self->keystore, pwfile);
            rc2 = KFileRelease(pwfile);
            if (rc == 0)
                rc = rc2;
        }
    }
    else if (VPathOption (vpath, vpopt_pwfd, obuff, buf_size - 1, &z)
        == 0)
    {
        /* -----
         * pwfd is not fully a VPath at this point: we 
         * should obsolete it
         */
        const KFile * pwfile;
        obuff [z] = '\0';
        rc = KFileMakeFDFileRead (&pwfile, atoi (obuff));
        if (rc == 0)
        {
            rc = KKeyStoreSetTemporaryKeyFromFile(self->keystore, pwfile);
            rc2 = KFileRelease(pwfile);
            if (rc == 0)
                rc = rc2;
        }
    }

    if (rc == 0)
    {
        KEncryptionKey* enc_key = NULL;

        /* here, we are only interested in global keys - at least for now */

        /* Get Key for current protected repository ( or global ) */
        rc = KKeyStoreGetKey(self->keystore, NULL, &enc_key);
        if (rc != 0 && self->resolver != NULL) {
            bool has_project_id = false;
            uint32_t projectId = 0;
            rc_t r2 = VResolverGetProjectId(self->resolver, &projectId);
            has_project_id = projectId != 0;
  
  /* Get Key for protected repository that was used to create self's resolver */
            if (r2 == 0 && has_project_id) {
                rc = KKeyStoreGetKeyByProjectId(
                    self->keystore, NULL, &enc_key,projectId);
            }
        }

        if (rc == 0)
        {
/* VDB-3590: Encryption key is a sequence of bytes.
             It is not a string and can represent an invalid UNICODE sequence */
            memmove(obuff, enc_key->value.addr, enc_key->value.size);	    
            *pwd_size = enc_key->value.size;

            if (*pwd_size != enc_key->value.size)
                rc = RC(rcVFS, rcPath, rcReading, rcBuffer, rcInsufficient);
            rc2 = KEncryptionKeyRelease(enc_key);
            if (rc == 0)
                rc = rc2;
        }
    }
    
    if ( GetRCState ( rc ) == rcNoPerm && GetRCObject ( rc ) == ( enum RCObject ) rcEncryptionKey )
    {
        LOGMSG ( klogErr, "You do not have read permissions to decrypt data from this project." );
        LOGMSG ( klogErr, "Please contact your PI to request an NGC token with decrypt permissions." );
        LOGMSG ( klogErr, "Import the new NGC file before decrypting again." );
        LOGMSG ( klogErr, "If you continue to have problems, contact sra@ncbi.nlm.nih.gov for assistance." );
    }
    
    rc2 = KKeyStoreSetTemporaryKeyFromFile(self->keystore, NULL); /* forget the temp key if set */
    if (rc == 0)
        rc = rc2;
    return rc;
}

/*
 * This is still hack - must match VFSManagerResolvePathRelativeDir()
 */
LIB_EXPORT rc_t CC VFSManagerWGAValidateHack (const VFSManager * self, 
                                              const KFile * file,
                                              const char * path) /* we'll move this to a vpath */
{
    VPath * vpath;
    rc_t rc = 0;

    rc = VPathMake (&vpath, path);
    if (rc == 0)
    {
        size_t z;
        char obuff [VFS_KRYPTO_PASSWORD_MAX_SIZE + 2]; /* 1 for over-read and 1 for NUL */
        rc = GetEncryptionKey(self, vpath, obuff, sizeof(obuff), &z);

        if (rc == 0)
        {
            rc = WGAEncValidate (file, obuff, z);
        }
    }
    return rc;
}



/* ResolvePath
 *
 * take a VPath and resolve to a final form apropriate for KDB
 *
 * that is take a relative path and resolve it against the CWD
 * or take an accession and resolve into the local or remote 
 * VResolver file based on config. It is just a single resolution percall
 */
static rc_t VFSManagerResolvePathResolver (const VFSManager * self,
                                           uint32_t flags,
                                           const VPath * in_path,
                                           VPath ** out_path)
{
    rc_t rc = 0;

    *out_path = NULL;

    /*
     * this RC perculates up for ncbi-acc: schemes but not for
     * no scheme uris
     */
    if ((flags & vfsmgr_rflag_no_acc) == vfsmgr_rflag_no_acc)
    {
        /* hack */
        if ( VPathGetUri_t ( in_path ) == vpuri_none )
            rc = SILENT_RC (rcVFS, rcMgr, rcResolving, rcSRA, rcNotAvailable);
        else
            rc = RC (rcVFS, rcMgr, rcResolving, rcSRA, rcNotAvailable);
    }
    else
    {
        bool not_done = true;

        /*
         * cast because we seem to have the restriction on the output from
         * VResolver that seems too restrictive
         */
        if ((flags & vfsmgr_rflag_no_acc_local) == 0)
        {
            rc = VResolverQuery(self->resolver, 0, in_path,
                (const VPath **)out_path, NULL, NULL);
            if (rc == 0)
                not_done = false;
        }
            
        if (not_done && ((flags & vfsmgr_rflag_no_acc_remote) == 0))
        {
            rc = VResolverRemote (self->resolver, self -> protocols,
                in_path, (const VPath **)out_path);
        }
    }
    return rc;
}


static rc_t VFSManagerResolvePathInt (const VFSManager * self,
                                      uint32_t flags,
                                      const KDirectory * base_dir,
                                      const VPath * in_path,
                                      VPath ** out_path)
{
    rc_t rc;
    char * pc;
    VPUri_t uri_type;

    assert (self);
    assert (in_path);
    assert (out_path);

    uri_type = VPathGetUri_t ( in_path );
    switch ( uri_type )
    {
    default:
        rc = RC (rcVFS, rcMgr, rcResolving, rcPath, rcInvalid);
        break;

    case vpuri_not_supported:
    case vpuri_ncbi_legrefseq:
        rc = RC (rcVFS, rcMgr, rcResolving, rcPath, rcUnsupported);
        break;

    case vpuri_ncbi_acc:
        rc = VFSManagerResolvePathResolver (self, flags, in_path, out_path);
        break;

    case vpuri_none:
        /* for KDB purposes, no scheme might be an accession */
        if (flags & vfsmgr_rflag_kdb_acc)
        {
             /* no '/' is permitted in an accession */
            pc = string_chr (in_path->path.addr, in_path->path.size, '/');
            if (pc == NULL)
            {
                rc = VFSManagerResolvePathResolver (self, flags, in_path, out_path);
                if (rc == 0)
                    break;
            }
        }
        /* Fall through */
    case vpuri_ncbi_vfs:
    case vpuri_file:
        /* check for relative versus full path : assumes no 'auth' not starting with '/' */
        if (in_path->path.addr[0] == '/')
        {
            rc = VPathAddRef (in_path);
            if (rc == 0)
                *out_path = (VPath *)in_path; /* oh these const ptr are annoying */
        }
        else
        {
            /* not 'properly' handling query, fragment etc. for relative path
             * assumes path within VPath is ASCIZ
             */
            size_t s;
            VPath * v;
            char u [32 * 1024];

            switch ( uri_type )
            {
            default:
                rc = RC (rcVFS, rcMgr, rcResolving, rcFunction, rcInvalid);
                break;

            case vpuri_ncbi_vfs:
                string_printf ( u, sizeof u, & s, "%S:", & in_path -> scheme );
                rc = KDirectoryResolvePath ( base_dir, true, & u [ s ], sizeof u - s,
                    "%.*s", ( int ) in_path -> path . size, in_path -> path . addr );
                if ( rc == 0 )
                {
                    s = string_size ( u );
                    rc = string_printf ( & u [ s ], sizeof u - s, NULL,
                        "%S%S", & in_path -> query, & in_path -> fragment );
                }
                if (rc == 0)
                    rc = VPathMake (&v, u);
                break;

            case vpuri_none:
            case vpuri_file:
                rc = KDirectoryResolvePath ( base_dir, true, u, sizeof u,
                    "%.*s", ( int ) in_path -> path . size, in_path -> path . addr );
                rc = VPathMake (&v, u);
                break;
            }
            if (rc == 0)
                *out_path = v;
        }
        break;

        /* these are considered fully resolved already */
    case vpuri_http:
    case vpuri_https:
    case vpuri_ftp:
        rc = VPathAddRef (in_path);
        if (rc == 0)
            *out_path = (VPath*)in_path;
        break;

    }
    return rc;
}


LIB_EXPORT rc_t CC VFSManagerResolvePath (const VFSManager * self,
                                          uint32_t flags,
                                          const VPath * in_path,
                                          VPath ** out_path)
{
    if (out_path == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    *out_path = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcSelf, rcNull);

    if (in_path == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    return VFSManagerResolvePathInt (self, flags, self->cwd, in_path, out_path);
}

LIB_EXPORT rc_t CC VFSManagerResolvePathRelative (const VFSManager * self,
                                                  uint32_t flags,
                                                  const struct  VPath * base_path,
                                                  const struct  VPath * in_path,
                                                  struct VPath ** out_path)
{
    const KDirectory * dir;
    rc_t rc;

    if (out_path == NULL)
        rc = RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    *out_path = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcSelf, rcNull);

    if (in_path == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    rc = VFSManagerOpenDirectoryRead (self, &dir, base_path);
    if (rc == 0)
        rc = VFSManagerResolvePathInt (self, flags, dir, in_path, out_path);

    return rc;
}

/*
 * This is still hack - must match VFSManagerGetEncryptionKey()
 */

LIB_EXPORT rc_t CC VFSManagerResolvePathRelativeDir (const VFSManager * self,
                                                     uint32_t flags,
                                                     const KDirectory * base_dir,
                                                     const VPath * in_path,
                                                     VPath ** out_path)
{
    if (out_path == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    *out_path = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcSelf, rcNull);

    if (in_path == NULL)
        return RC (rcVFS, rcMgr, rcResolving, rcParam, rcNull);

    return VFSManagerResolvePathInt (self, flags, base_dir, in_path, out_path);
}


/* OpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
static
rc_t VFSManagerOpenFileReadDecryption (const VFSManager *self,
                                       const KDirectory * dir,
                                       const KFile ** f,
                                       const KFile * file,
                                       const VPath * path,
                                       bool force_decrypt,
                                       bool * was_encrypted)
{
    rc_t rc = 0;
    size_t z;
    char obuff [VFS_KRYPTO_PASSWORD_MAX_SIZE + 2]; /* 1 for over-read and 1 for NUL */
    bool has_enc_opt;

    if (was_encrypted)
        *was_encrypted = false;

    /* -----
     * at this point we have no fatal errors and we have the
     * file opened but we have not seen if we have to decrypt
     * or use other query options
     */
    has_enc_opt = (VPathOption (path, vpopt_encrypted, obuff,
                                sizeof obuff, &z) == 0);

    if ((has_enc_opt == false) &&
        (force_decrypt == false))
    {
        /* if we are not told to decrypt, don't and we are done */
        KFileAddRef (file);
        *f = file;
    }

    else /* we are told to decrypt if possible */
    {
        /* -----
         * pre-read 4kb from the 'encrypted file'
         */
        rc = KFileRandomAccess (file);
        if (rc == 0)
            ;
        /* most common and easiest option is it has random
         * access - a no-op here
         */
        else if (GetRCState(rc) == rcUnsupported)
        {
            const KFile * buffile;

            rc = KBufFileMakeRead (&buffile, file, 32 * 2 * 1024);
            if (rc)
                ;
            else
            {
                /* there is an extra reference to file now, but
                 * it gets removed after this function returns
                 */
                file = buffile;
            }
        }
        
        if (rc == 0)
        {
            size_t tz;
            char tbuff [4096];

            /* we now have a file from which we can pre-read the
             * possible encrypted format header */
            rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
            if (rc == 0)
            {
                /* 
                 * we've successfully read 4KB from the file,
                 * now decide if is actually an encrypted file
                 * format we support
                 */
                const KFile * encfile;

                /* is this the header of an ecnrypted file? */
                if (KFileIsEnc (tbuff, tz) == 0)
                {
                    if (was_encrypted)
                        *was_encrypted = true;
                    rc = GetEncryptionKey(self, path, obuff, sizeof(obuff), &z);
                    if (rc == 0)
                    {
                        KKey key;

                        /* create the AES Key */
                        rc = KKeyInitRead (&key, kkeyAES128, obuff, z);
                        if (rc)
                            ;
                        else
                        {
                            rc = KEncFileMakeRead (&encfile, file, &key);
                            if (rc)
                                ;
                            else
                            {
                                const KFile * buffile;

                                /*
                                 * TODO: make the bsize a config item not a hard constant
                                 */
                                rc = KBufFileMakeRead (&buffile, encfile,
                                                       256 * 1024 * 1024);
                                if (rc == 0)
                                {
                                    *f = buffile;
                                    /* *f keeps a reference to encfile, can release it here */
                                    KFileRelease (encfile);
                                    return 0;
                                }
                                KFileRelease (encfile);
                            }
                        }
                    }
                }
                else if (KFileIsWGAEnc (tbuff, tz) == 0)
                {
                    if (was_encrypted)
                        *was_encrypted = true;
                    rc = GetEncryptionKey(self, path, obuff, sizeof(obuff), &z);
                    if (rc == 0)
                    {
                        rc = KFileMakeWGAEncRead (&encfile, file, obuff, z);
                        if (rc)
                            ;
                        else
                        {
                            /* we'll release anextra reference to file
                             * after this function returns
                             */
                            *f = encfile;
                            return 0;
                        }
                    }
                }
                else
                {
                    /* -----
                     * not encrypted in a manner we can decrypt so 
                     * give back the raw file (possibly buffered
                     *
                     * since file is released in the caller
                     * we need another reference
                     */
                    KFileAddRef (file);
                    *f = file;
                    return 0;
                }
            }
        }
    }
    return rc;
}


/*
 * try to open the file as a regular file
 */
static
rc_t VFSManagerOpenFileReadRegularFile (char * pbuff, size_t z,
                                        KFile const ** file,
                                        const KDirectory * dir)
{
    rc_t rc;
    char rbuff [8192];

    assert ((pbuff) && (pbuff[0]));
    assert (*file == NULL);

    rc = KDirectoryResolvePath (dir, true, rbuff, sizeof rbuff,
                                "%s", pbuff);
    if (rc)
        ; /* log? */
    else
    {
        /* validate that the file system agrees the path refers
         * to a regular file (even if through a link */
        uint32_t type;

        type = KDirectoryPathType (dir, "%s", rbuff);
        switch (type & ~kptAlias)
        {
        case kptNotFound:
            rc = RC (rcVFS, rcMgr, rcOpening, rcFile,
                     rcNotFound);
            break;

        case kptBadPath:
            rc = RC (rcVFS, rcMgr, rcOpening, rcFile,
                     rcInvalid);
            break;

        case kptDir:
        case kptCharDev:
        case kptBlockDev:
        case kptFIFO:
        case kptZombieFile:
            rc = RC (rcVFS, rcMgr, rcOpening, rcFile,
                     rcIncorrect);
            break;

        default:
            rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcUnknown);
            break;

        case kptFile:
            /*
             * this is the good/successful path: open the file 
             * as a const KFile
             */
            rc = KDirectoryOpenFileRead (dir, file, "%s", rbuff);
            break;
        }
    }

    return rc;
}

/*
 * if successful set *file to a usable KFile * and return 0
 * if unsuccessful but without error, set *file to NULL and return 0
 * if an error encountered set *file to NULL and return non-zero.
 */
static
rc_t VFSManagerOpenFileReadSpecial (char * pbuff, size_t z, KFile const ** file)
{
    rc_t rc = 0;
    static const char dev [] = "/dev/";
    static const char dev_stdin [] = "/dev/stdin";
    static const char dev_null [] = "/dev/null";

    assert (pbuff);
    assert (z);
    assert (file);

    *file = NULL;

    /*
     * Handle a few special case path names that are pre-opened
     * 'file descriptors'
     *
     * This probably needs to be system specific eventually
     *
     * First check for the path being in the 'dev' directory in
     * posix/unix terms
     */
    if (string_cmp (dev, sizeof dev - 1, pbuff, z, sizeof dev - 1) != 0)
        rc = 0; /* we're done */

    else
    {
        if (strcmp (dev_stdin, pbuff) == 0)
            rc = KFileMakeStdIn (file);

        else if (strcmp (dev_null, pbuff) == 0)
            rc = KFileMakeNullRead (file);

        else if (strncmp ("/dev/fd/", pbuff, sizeof "/dev/fd/" - 1) == 0)
        {
            char * pc;
            size_t ix;

            pc = pbuff + sizeof "/dev/fd/" - 1;

            for (ix = 0; isdigit (pc[ix]); ++ix)
                assert (ix <= z);

            if ((ix > 0)&&(pc[ix] == '\0'))
            {
                int fd;

                fd = atoi (pc);
                rc = KFileMakeFDFileRead (file, fd);
            }
        }
    }

    return rc;
}

static
rc_t VFSManagerOpenFileReadInt (const VFSManager *self,
                                const KDirectory * dir,
                                KFile const **f,
                                const VPath * path,
                                bool force_decrypt,
                                bool * was_encrypted)
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    const KFile * file = NULL;
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    rc = VPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc)
        ; /* log? */
    else
    {
        /* -----
         * try to open path as a special file if requested
         *
         * *file will be set or a usable file or to NULL and rc will reflect
         * any error
         */
        rc = VFSManagerOpenFileReadSpecial (pbuff, num_read, &file);

        if (rc == 0)
        {
            /* -----
             * If we didn't open the file using the special
             * logic above for special paths open the file and have no error,
             * continue
             */
            if (file == NULL)
                rc = VFSManagerOpenFileReadRegularFile (pbuff, num_read,
                                                        &file, dir);
            /*
             * we either have an rc to return with or we have an open KFile:
             * check for possible encryption that we are told to decrypt
             */
            if (rc == 0)
            {
                rc = VFSManagerOpenFileReadDecryption (self, dir, f, file, path,
                                                       force_decrypt, was_encrypted);
            }
            /* release file if we are here and it is open */
            KFileRelease (file);
        }
    }
    return rc;
}


static
rc_t VFSManagerOpenFileReadDirectoryRelativeInt (const VFSManager *self,
                                                 const KDirectory * dir,
                                                 KFile const **f,
                                                 const VPath * path,
                                                 bool force_decrypt,
                                                 bool * was_encrypted)
{
    rc_t rc;

    if (f == NULL)
        rc = RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

    else
    {
        *f = NULL;

        if ((f == NULL) || (path == NULL))
            rc = RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

        else if (self == NULL)
            rc = RC (rcVFS, rcMgr, rcOpening, rcSelf, rcNull);

        else
        {

            rc = VFSManagerOpenFileReadInt (self, dir, f, path, force_decrypt, was_encrypted);
        }
    }
    return rc;
}


/* we will create a KFile from a http or ftp url... */
static rc_t VFSManagerOpenCurlFile ( const VFSManager *self,
                                     KFile const **f,
                                     const VPath * path,
                                     uint32_t blocksize,
                                     bool promote )
{
    rc_t rc;
    bool high_reliability, is_refseq;

    if ( f == NULL )
        return RC( rcVFS, rcMgr, rcOpening, rcParam, rcNull );
    *f = NULL;
    if ( self == NULL )
        return RC( rcVFS, rcMgr, rcOpening, rcSelf, rcNull );
    if ( path == NULL )
        return RC( rcVFS, rcMgr, rcOpening, rcParam, rcNull );

    high_reliability = VPathIsHighlyReliable ( path );
    is_refseq = VPathHasRefseqContext ( path );
    if ( self->resolver != NULL )
    {
        const VPath * local_cache;

        /* find cache - vresolver call */
        rc = VResolverCache ( self->resolver, path, &local_cache, 0 );
        if ( rc == 0 )
        {
            /* we did find a place for local cache --> use it! */
            rc = VFSManagerMakeHTTPFile( self,
                                         f,
                                         path,
                                         local_cache -> path.addr,
                                         blocksize,
                                         high_reliability,
                                         is_refseq,
                                         promote );
            {
                rc_t rc2 = VPathRelease ( local_cache );
                if ( rc == 0 )
                {
                    rc = rc2;
                }
            }
        }
        else
        {
            /* we did NOT find a place for local cache --> we are not caching! */
            rc = VFSManagerMakeHTTPFile( self,
                                         f,
                                         path,
                                         NULL,
                                         blocksize,
                                         high_reliability,
                                         is_refseq,
                                         promote );
        }
    }
    else
    {
        /* no resolver has been found ---> we cannot do caching! */
        rc = VFSManagerMakeHTTPFile( self,
                                     f,
                                     path,
                                     NULL,
                                     blocksize,
                                     high_reliability,
                                     is_refseq,
                                     promote );
    }
    return rc;
}

LIB_EXPORT
rc_t CC VFSManagerOpenFileReadDirectoryRelative (const VFSManager *self,
                                                 const KDirectory * dir,
                                                 KFile const **f,
                                                 const VPath * path)
{
    return VFSManagerOpenFileReadDirectoryRelativeInt (self, dir, f, path, false, NULL);
}

LIB_EXPORT
rc_t CC VFSManagerOpenFileReadDirectoryRelativeDecrypt (const VFSManager *self,
                                                        const KDirectory * dir,
                                                        KFile const **f,
                                                        const VPath * path) /*,
                                                        bool force_decrypt) */
{
    return VFSManagerOpenFileReadDirectoryRelativeInt (self, dir, f, path, true, NULL);
}


static rc_t ResolveVPathByVResolver( struct VResolver * resolver, const VPath ** path )
{
    rc_t rc;

    if ( resolver == NULL )
        rc = RC ( rcVFS, rcFile, rcOpening, rcSRA, rcUnsupported );
    else
    {
        const VPath * tpath;
        rc = VResolverLocal ( resolver, *path, &tpath );
        if ( rc == 0 )
        {
            VPathRelease ( *path );
            *path = tpath;
        }
    }
    return rc;
}

static rc_t ResolveVPathBySRAPath( const VPath ** path )
{
    * path = NULL;
    return RC ( rcVFS, rcFile, rcOpening, rcSRA, rcUnsupported );
}

LIB_EXPORT rc_t CC VFSManagerOpenFileReadWithBlocksize ( const VFSManager *self,
                                            KFile const **f,
                                            const VPath * path_,
                                            uint32_t blocksize,
                                            bool promote )
{
    rc_t rc;

    if ( f == NULL )
        rc = RC (rcVFS, rcMgr, rcOpen, rcParam, rcNull);
    else
    {
        *f = NULL;

        if  (self == NULL )
            rc = RC ( rcVFS, rcMgr, rcOpen, rcSelf, rcNull );
        else if ( f == NULL )
            rc = RC ( rcVFS, rcMgr, rcOpen, rcParam, rcNull );
        else
        {
            rc = VPathAddRef ( path_ );
            if ( rc == 0 )
            {
                const VPath * path = path_;
                VPUri_t uri_type = VPathGetUri_t ( path );

                switch ( uri_type )
                {
                default:
                case vpuri_invalid:
                    rc = RC (rcVFS, rcFile, rcOpening, rcPath, rcInvalid);
                    break;

                case vpuri_not_supported:
                    rc = RC (rcVFS, rcFile, rcOpening, rcPath, rcUnsupported);
                    break;

                case vpuri_ncbi_acc:
                    if ( self->resolver != NULL )
                        rc = ResolveVPathByVResolver( self->resolver, &path );
                    else
                        rc = ResolveVPathBySRAPath( &path );

                    if ( rc != 0 )
                        break;

                /* !!! fall through !!! */

                case vpuri_none:
                case vpuri_ncbi_vfs:
                case vpuri_file:
                    rc = VFSManagerOpenFileReadDirectoryRelativeInt ( self, self->cwd, f, path, false, NULL );
                    break;

                case vpuri_ncbi_legrefseq:
                    rc = RC ( rcVFS, rcFile, rcOpening, rcPath, rcIncorrect );
                    break;

                case vpuri_http:
                case vpuri_https:
                case vpuri_ftp:
                    rc = VFSManagerOpenCurlFile ( self, f, path, blocksize, promote );
                    break;
                }
                VPathRelease (path);
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerOpenFileRead ( const VFSManager *self,
                                            KFile const **f,
                                            const VPath * path )
{
    return VFSManagerOpenFileReadWithBlocksize ( self, f, path, DEFAULT_CACHE_PAGE_SIZE, false );
}

LIB_EXPORT rc_t CC VFSManagerOpenFileReadPromote ( const VFSManager *self,
                                            KFile const **f,
                                            const VPath * path )
{
    return VFSManagerOpenFileReadWithBlocksize ( self, f, path, DEFAULT_CACHE_PAGE_SIZE, true );
}

LIB_EXPORT rc_t CC VFSManagerOpenFileReadDecrypt (const VFSManager *self,
                                                  KFile const **f,
                                                  const VPath * path)
{
    return VFSManagerOpenFileReadDirectoryRelativeInt ( self, self->cwd, f, path, true, NULL );
}

LIB_EXPORT
rc_t CC VFSManagerOpenDirectoryUpdateDirectoryRelative (const VFSManager *self,
                                                        const KDirectory * dir,
                                                        KDirectory **d,
                                                        const VPath * path)
{
    rc_t rc;
    VPUri_t uri_type;

    if ((d == NULL) || (path == NULL))
        return RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

    *d = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcOpening, rcSelf, rcNull);

    uri_type = VPathGetUri_t ( path );
    switch ( uri_type )
    {
    case vpuri_http :
    case vpuri_https:
    case vpuri_ftp :
        return RC( rcVFS, rcMgr, rcOpening, rcParam, rcWrongType );

    default :
        {
            uint32_t type;

            /* WHY NOT JUST TRY TO OPEN THE DIRECTORY,
               AND LET KFS TELL US WHAT'S WRONG? */

            type = KDirectoryPathType (dir, "%.*s", ( int ) path -> path . size, path -> path . addr );
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                rc = RC (rcVFS, rcMgr, rcOpening, rcDirectory, rcNotFound);
                break;

            case kptFile:
                rc = RC (rcVFS, rcMgr, rcOpening, rcDirectory, rcReadonly);
                break;

            case kptBadPath:
                rc = RC (rcVFS, rcMgr, rcOpening, rcDirectory, rcInvalid);
                break;

            case kptDir:
                rc = KDirectoryOpenDirUpdate ((KDirectory*)dir, d, false, "%.*s", ( int ) path -> path . size, path -> path . addr);
                return rc;

            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = RC (rcVFS, rcMgr, rcOpening, rcDirectory, rcIncorrect);
                break;

            default:
                rc = RC (rcVFS, rcMgr, rcOpening, rcDirectory, rcUnknown);
                break;
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VFSManagerOpenDirectoryUpdate (const VFSManager *self,
                                                  KDirectory **d,
                                                  const VPath * path)
{
    return VFSManagerOpenDirectoryUpdateDirectoryRelative (self, self->cwd, d, path);
}


static
rc_t TransformFileToDirectory(const KDirectory * dir,
                              const KFile * file,
                              KDirectory const **d,
                              const char *path_str,
                              bool was_encrypted)
{
    rc_t rc;

    rc = KFileRandomAccess( file );
    if (rc)
        PLOGERR(klogErr,(klogErr, rc, "Can not use files without random access"
                         " as database archives '$(P)'", "P=%s", path_str));
    else
    {
        size_t tz;
        char tbuff [4096];

        rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
        if ( rc )
            LOGERR (klogErr, rc, "Error reading the head of an archive to use as a database object");
        else
        {
            /* we only use KAR/SRA or tar files as archives so try to identify
             * as our KAR/SRA file.
             IT IS NOT TRUE ANYMORE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
            if ( KFileIsSRA( tbuff, tz ) == 0 )
                /* if it was open it as a directory */
                rc = KDirectoryOpenSraArchiveReadUnbounded_silent_preopened( dir, d, false, file, "%s", path_str );

            else
            {
                rc = KDirectoryOpenTarArchiveRead_silent_preopened( dir, d, false, file, "%s", path_str );

                /*
                 * if RC here we did not have an SRA and did not have
                 * a tar file we could use; assume the problem was:
                 * - decryption if the file was encrypted
                 * - or it is not an archive
                 */
                if (rc != 0) {
                    if ( was_encrypted ) {
                     /* the following RC update is not correct anymore but:
                        TODO: check tools/libraries
                            that expect this returned code and fix them
                        rc = RC(rcVFS, rcEncryptionKey, rcOpening, rcEncryption,
                             rcIncorrect );
                        PLOGERR (klogErr, (klogErr, rc,
                            "could not use '$(P)' as an "
                            "archive it was encrypted so the password"
                            " was possibly wrong or it is not SRA or"
                            " TAR file", "P=%s", path_str)); */
                        MGR_DEBUG(("TransformFileToDirectory: could not use "
                            "'%s' as an archive - it was encrypted and "
                            "the password was possibly wrong "
                            "or it is not SRA or TAR file\n", path_str));
                    }
                    else {
                        PLOGERR (klogInfo, (klogInfo, rc,
                            "could not use '$(P)' as an "
                            "archive not identified as SRA or"
                            " TAR file", "P=%s", path_str));
                    }
                }
            }
        }
    }
    return rc;
}

static
rc_t VFSManagerOpenDirectoryReadHttp (const VFSManager *self,
                                      const KDirectory * dir,
                                      KDirectory const **d,
                                      const VPath * path,
                                      bool force_decrypt,
                                      bool reliable,
                                      bool promote )
{
    const KFile * file = NULL;
    rc_t rc = VFSManagerOpenCurlFile ( self, &file, path, DEFAULT_CACHE_PAGE_SIZE, promote );
    if ( rc != 0 )
    {
        bool toLog = false;
        const char extension[] = ".vdbcache";
        const String * s = & path -> path;
        assert ( s );
        /* do not log error for .vdbcache http files : find a better solution */
        if ( s -> addr == NULL
            || s -> size < sizeof extension || s -> len < sizeof extension
            || string_cmp ( s -> addr + s -> size - sizeof extension + 1,
                sizeof extension - 1,
                extension, sizeof extension - 1, sizeof extension - 1 ) != 0 )
        {
            toLog = reliable;
        }
        if ( toLog ) {
          const String * p = NULL;
          rc_t rc2 = VPathMakeString ( path, & p );
          if ( rc2 == 0 ) {
                PLOGERR ( klogErr, ( klogErr, rc, "error with https open '$(path)'",
                                       "path=%S", p ) );
                free (  ( void * ) p );
          } else {
            PLOGERR ( klogErr, ( klogErr, rc, "error with https open '$(scheme):$(path)'",
                             "scheme=%S,path=%S", & path -> scheme, s ) );
          }
        }
    }
    else
    {
        const char mountpointpath[] = "/";
        const KDirectory * mountpoint;

        rc = KQuickMountDirMake (self->cwd, &mountpoint, file,
                                 mountpointpath, sizeof mountpointpath - 1, 
                                 path->path.addr, path->path.size);
        if (rc)
        {
            PLOGERR (klogInt, (klogErr, rc, "error creating mount "
                               "'$(M)' for '$(F)", "M=%s,F=%S",
                               mountpointpath, &path->path));
        }
        else
        {
            const KFile * f;
            bool was_encrypted = false;

            rc = VFSManagerOpenFileReadDecryption (self, mountpoint, &f,
                                                   file, path,
                                                   force_decrypt,
                                                   &was_encrypted);
            if (rc == 0)
            {
                    
                rc = TransformFileToDirectory (mountpoint, f, d, 
                                               path->path.addr,
                                               was_encrypted);
                /* hacking in the fragment bit */
                if ((rc == 0) && (path->fragment . size > 1 ) )
                {
                    const KDirectory * tempd = * d;
                    const char * fragment = path -> fragment . addr + 1;
                    int frag_size = ( int ) path -> fragment . size - 1;

                    assert ( fragment [ -1 ] == '#' );
                        
                    rc = KDirectoryOpenDirRead (tempd, d, false, "%.*s", frag_size, fragment );
                    
                    KDirectoryRelease (tempd);
                }
                KFileRelease (f);
            }
            KDirectoryRelease (mountpoint);
        }
        KFileRelease (file);
    }
    return rc;
}

/* similar to VFSManagerOpenDirectoryReadHttp but already resolved */
static
rc_t VFSManagerOpenDirectoryReadHttpResolved (const VFSManager *self,
                                              KDirectory const **d,
                                              const VPath * path,
                                              const VPath * cache,
                                              bool force_decrypt,
                                              bool promote)
{
    const String * uri = NULL;
    rc_t rc = VPathMakeString ( path, &uri );
    if ( rc == 0 )
    {
        /* check how the path has been marked */
        bool high_reliability = VPathIsHighlyReliable ( path );
        bool is_refseq = VPathHasRefseqContext ( path );

        const KFile * file = NULL;
        rc = VFSManagerMakeHTTPFile( self,
                                     &file,
                                     path,
                                     cache == NULL ? NULL : cache -> path . addr,
                                     DEFAULT_CACHE_PAGE_SIZE,
                                     high_reliability,
                                     is_refseq,
                                     promote );
        if ( rc != 0 )
        {
            if ( high_reliability )
            {
                PLOGERR ( klogErr, ( klogErr, rc, "error with https open '$(U)'",
                                     "U=%S", uri ) );
            }
        }
        else
        {
            const char mountpointpath[] = "/";
            const KDirectory * mountpoint;

            rc = KQuickMountDirMake (self->cwd, &mountpoint, file,
                                     mountpointpath, sizeof mountpointpath - 1, 
                                     path->path.addr, path->path.size);
            if (rc)
            {
                PLOGERR (klogInt, (klogErr, rc, "error creating mount "
                                   "'$(M)' for '$(F)", "M=%s,F=%S",
                                   mountpointpath, &path->path));
            }
            else
            {
                const KFile * f;
                bool was_encrypted = false;

                rc = VFSManagerOpenFileReadDecryption (self, mountpoint, &f,
                                                       file, path,
                                                       force_decrypt,
                                                       &was_encrypted);
                if ( rc == 0 )
                {
                        
                    rc = TransformFileToDirectory ( mountpoint, f, d, 
                                                    path -> path . addr,
                                                    was_encrypted);
                    /* hacking in the fragment bit */
                    if ( ( rc == 0 ) && ( path -> fragment . size > 1 ) )
                    {
                        const KDirectory * tempd = * d;
                        const char * fragment = path -> fragment . addr + 1;
                        int frag_size = ( int ) path -> fragment . size - 1;

                        assert ( fragment [ -1 ] == '#' );
                            
                        rc = KDirectoryOpenDirRead (tempd, d, false, "%.*s", frag_size, fragment );
                        
                        KDirectoryRelease (tempd);
                    }
                    KFileRelease (f);
                }
                KDirectoryRelease (mountpoint);
            }
            KFileRelease (file);
        }
        free( ( void * )uri );
    }
    return rc;
}


static
rc_t VFSManagerOpenDirectoryReadKfs (const VFSManager *self,
                                     const KDirectory * dir,
                                     KDirectory const **d,
                                     const VPath * path,
                                     bool force_decrypt)
{
    const KFile * file = NULL;
    char rbuff[ 4096 ]; /* resolved path buffer */
    rc_t rc;

    assert (self);
    assert (dir);
    assert (d);
    assert (path);
    assert ((force_decrypt == false) || (force_decrypt == true));
    assert (*d == NULL);

    file = NULL;

    rc = KDirectoryResolvePath( dir, true, rbuff, sizeof rbuff, "%.*s", ( int ) path -> path . size, path -> path . addr );
    if ( rc == 0 )
    {
        uint32_t type;
        bool was_encrypted;

        type = KDirectoryPathType( dir, "%s", rbuff );
        switch (type & ~kptAlias)
        {
        case kptNotFound:
            rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcNotFound );
            break;

        case kptFile:
            rc = VFSManagerOpenFileReadDirectoryRelativeInt (self, dir, 
                                                             &file, path, 
                                                             force_decrypt,
                                                             &was_encrypted);
            if (rc == 0)
                rc = TransformFileToDirectory (dir, file, d, rbuff,
                                               was_encrypted);
            break;

        case kptBadPath:
            rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcInvalid );
            break;

        case kptDir:
            rc = KDirectoryOpenDirRead( dir, d, false, "%s", rbuff );
            return rc;

        case kptCharDev:
        case kptBlockDev:
        case kptFIFO:
        case kptZombieFile:
            rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcIncorrect );
            break;

        default:
            rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcUnknown );
            break;
        }

        /* hacking in the fragment bit */
        /* the C grammar specifies order of precedence... */
        if ((rc == 0) && (path->fragment.size > 1 ))
        {
            const KDirectory * tempd = * d;
            const char * fragment = path -> fragment . addr + 1;
            int frag_size = ( int ) path -> fragment . size - 1;

            assert ( fragment [ -1 ] == '#' );
            
            rc = KDirectoryOpenDirRead (tempd, d, false, "%.*s", frag_size, fragment );
            
            KDirectoryRelease (tempd);
        }
    }

    KFileRelease(file);

    return rc;
}


static
rc_t VFSManagerOpenDirectoryReadLegrefseq (const VFSManager *self,
                                           const KDirectory * dir,
                                           KDirectory const **d,
                                           const VPath * path,
                                           bool force_decrypt)
{
    const KFile * file;
    const KDirectory * dd;
    size_t num_read;
    char pbuff [4096]; /* path buffer */
    rc_t rc;

    assert (self);
    assert (dir);
    assert (d);
    assert (path);
    assert ((force_decrypt == false) || (force_decrypt == true));
    assert (*d == NULL);

    file = NULL;
    dd = NULL;

    /* hier part only */
    rc = VPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if ( rc == 0 )
    {
        char rbuff[ 4096 ]; /* resolved path buffer */
        rc = KDirectoryResolvePath( dir, true, rbuff, sizeof rbuff, "%s", pbuff );
        if ( rc == 0 )
        {
            uint32_t type;
            bool was_encrypted;

            type = KDirectoryPathType( dir, "%s", rbuff );
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcNotFound );
                break;

            case kptFile:
                rc = VFSManagerOpenFileReadDirectoryRelativeInt (self, dir, 
                                                                 &file, path, 
                                                                 force_decrypt,
                                                                 &was_encrypted);
                if (rc == 0)
                    rc = TransformFileToDirectory (dir, file, &dd, rbuff,
                                                   was_encrypted);
                break;

            case kptBadPath:
                rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcInvalid );
                break;

            case kptDir:
                rc = KDirectoryOpenDirRead( dir, &dd, false, "%s", rbuff );
                break;

            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcIncorrect );
                break;

            default:
                rc = RC( rcVFS, rcMgr, rcOpening, rcDirectory, rcUnknown );
                break;
            }

            if (rc == 0)
            {
                if ( path -> fragment . size < 2 )
                    rc = RC( rcVFS, rcMgr, rcOpening, rcPath, rcIncorrect );
                else
                {
                    const char *fragment = path -> fragment . addr + 1;
                    int frag_size = ( int ) path -> fragment . size - 1;
                    assert ( fragment [ -1 ] == '#' );

                    rc = KDirectoryOpenDirRead (dd, d, false, "%.*s", frag_size, fragment );

                    KDirectoryRelease (dd);
                }
            }
        }
    }
    return rc;
}


static
rc_t VFSManagerOpenDirectoryReadDirectoryRelativeInt (const VFSManager *self,
                                                      const KDirectory * dir,
                                                      KDirectory const **d,
                                                      const VPath * path_,
                                                      bool force_decrypt,
                                                      bool reliable,
                                                      bool promote)
{
    rc_t rc;
    do 
    {
        if (d == NULL)
        {
            rc =  RC (rcVFS, rcDirectory, rcOpening, rcParam, rcNull);
            break;
        }

        *d = NULL;

        if (self == NULL)
        {
            rc = RC (rcVFS, rcDirectory, rcOpening, rcSelf, rcNull);
            break;
        }

        if ((dir == NULL) || (path_ == NULL))
        {
            rc = RC (rcVFS, rcDirectory, rcOpening, rcParam, rcNull);
            break;
        }

#if 0
        if ((force_decrypt != false) && (force_decrypt != true))
        {
            rc = RC (rcVFS, rcDirectory, rcOpening, rcParam, rcInvalid);
            break;
        }
#endif

        rc = VPathAddRef (path_);
        if ( rc )
            break;
        else
        {
            const VPath *path = path_;
            VPUri_t uri_type = VPathGetUri_t ( path );

            switch ( uri_type )
            {
            default:
            case vpuri_invalid:
                rc = RC (rcVFS, rcDirectory, rcOpening, rcPath, rcInvalid);
                break;


            case vpuri_not_supported:
                rc = RC (rcVFS, rcDirectory, rcOpening, rcPath, rcUnsupported);
                break;

            case vpuri_ncbi_acc:
                if ( self->resolver != NULL )
                    rc = ResolveVPathByVResolver( self->resolver, &path );
                else
                    rc = ResolveVPathBySRAPath( &path );
                if ( rc != 0 )
                    break;

            /* !!! fall through !!! */

            case vpuri_none:
            case vpuri_ncbi_vfs:
            case vpuri_file:
                rc = VFSManagerOpenDirectoryReadKfs ( self, dir, d, path, force_decrypt );
                break;

            case vpuri_ncbi_legrefseq:
                rc = VFSManagerOpenDirectoryReadLegrefseq ( self, dir, d, path, force_decrypt );
                break;

            case vpuri_http:
            case vpuri_https:
            case vpuri_ftp:
                rc = VFSManagerOpenDirectoryReadHttp ( self, dir, d, path,
                                                      force_decrypt, reliable, promote );
                break;
            }
            VPathRelease ( path ); /* same as path_ if not uri */
        }
    } while (0);
    return rc;
}


LIB_EXPORT 
rc_t CC VFSManagerOpenDirectoryReadDirectoryRelative (const VFSManager *self,
                                                      const KDirectory * dir,
                                                      KDirectory const **d,
                                                      const VPath * path)
{
    /* HACK - this function should not be exported.
       in order to not change the signature, we are synthesizing
       a "promote" parameter as "true" to mimic old behavior */
    return VFSManagerOpenDirectoryReadDirectoryRelativeInt (self, dir, d, path,
        false, true, true);
}


LIB_EXPORT 
rc_t CC VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt (const VFSManager *self,
                                                             const KDirectory * dir,
                                                             KDirectory const **d,
                                                             const VPath * path)
{
    /* HACK - this function should not be exported.
       in order to not change the signature, we are synthesizing
       a "promote" parameter as "true" to mimic old behavior */
    return VFSManagerOpenDirectoryReadDirectoryRelativeInt (self, dir, d, path,
        true, true, true);
}


LIB_EXPORT rc_t CC VFSManagerOpenDirectoryReadDecrypt (const VFSManager *self,
                                                       KDirectory const **d,
                                                       const VPath * path)
{
    /* HACK - this function should not be exported.
       in order to not change the signature, we are synthesizing
       a "promote" parameter as "true" to mimic old behavior */
    return VFSManagerOpenDirectoryReadDirectoryRelativeInt (self, self->cwd, d,
        path, true, true, true);
}

LIB_EXPORT rc_t CC VFSManagerOpenDirectoryReadDecryptUnreliable (
                                                       const VFSManager *self,
                                                       KDirectory const **d,
                                                       const VPath * path)
{
    return VFSManagerOpenDirectoryReadDirectoryRelativeInt (self, self->cwd, d,
        path, true, false, true);
}


LIB_EXPORT rc_t CC VFSManagerOpenDirectoryRead (const VFSManager *self,
                                                KDirectory const **d,
                                                const VPath * path)
{
    if ( self == NULL )
        return RC (rcVFS, rcDirectory, rcOpening, rcSelf, rcNull);
    /* HACK - this function should not be exported.
       in order to not change the signature, we are synthesizing
       a "promote" parameter as "true" to mimic old behavior */
    return VFSManagerOpenDirectoryReadDirectoryRelativeInt (self, self->cwd, d,
        path, false, true, true);
}

LIB_EXPORT 
rc_t CC VFSManagerOpenDirectoryReadDecryptRemote (const VFSManager *self,
                                                  KDirectory const **d,
                                                  const VPath * path,
                                                  const VPath * cache)
{
    rc_t rc;
    if ( self == NULL )
        return RC (rcVFS, rcDirectory, rcOpening, rcSelf, rcNull);
    if ( path == NULL )
        return RC (rcVFS, rcDirectory, rcOpening, rcParam, rcNull);
    /* cache == NULL is ok */
    if ( d == NULL )
        return RC (rcVFS, rcDirectory, rcOpening, rcParam, rcNull);
    *d = NULL;

    switch ( VPathGetUri_t ( path ) )
    {
    case vpuri_http:
    case vpuri_https:
    case vpuri_ftp:
        /* HACK - this function should not be exported.
           in order to not change the signature, we are synthesizing
           a "promote" parameter as "true" to mimic old behavior */
        rc = VFSManagerOpenDirectoryReadHttpResolved ( self, d, path, cache, true, true );
        break;
        
    default:
        rc = RC (rcVFS, rcDirectory, rcOpening, rcPath, rcInvalid);
        break;
    }
    
    return rc;
}


/* OpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC VFSManagerOpenFileWrite (const VFSManager *self,
                                            KFile **f, bool update,
                                            const VPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    KFile * file = NULL;
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if ((f == NULL) || (path == NULL))
        return RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

    *f = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = VPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        /* handle a few special case path names
         * This probably needs to be system specifica eventually
         */
        if (strncmp ("/dev/", pbuff, sizeof "/dev/" - 1) == 0)
        {

            if (strcmp ("/dev/stdout", pbuff) == 0)
                rc = KFileMakeStdOut (&file);
            else if (strcmp ("/dev/stderr", pbuff) == 0)
                rc = KFileMakeStdErr (&file);
            else if (strcmp ("/dev/null", pbuff) == 0)
                rc = KFileMakeNullUpdate (&file);
            else if (strncmp ("/dev/fd/", pbuff, sizeof "/dev/fd/" - 1) == 0)
            {
                char * pc;
                size_t ix;

                pc = pbuff + sizeof "/dev/fd/" - 1;

                for (ix = 0; isdigit (pc[ix]); ++ix)
                    ;

                if ((ix > 0)&&(pc[ix] == '\0'))
                {
                    int fd = atoi (pc);

                    rc = KFileMakeFDFileWrite (&file, update, fd);
                }
            }
        }
        if ((rc == 0)&&(file == NULL))
        {
            char rbuff [4096];

            rc = KDirectoryResolvePath (self->cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
            if (rc == 0)
            {
                uint32_t type;

                type = KDirectoryPathType (self->cwd, "%s", rbuff);
                switch (type & ~kptAlias)
                {
                case kptNotFound:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcNotFound);
                    break;

                case kptFile:
                    rc = KDirectoryOpenFileWrite (self->cwd, &file, update, "%s", rbuff);
                    break;

                case kptBadPath:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcInvalid);
                    break;
                case kptDir:
                case kptCharDev:
                case kptBlockDev:
                case kptFIFO:
                case kptZombieFile:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcIncorrect);
                    break;

                default:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcUnknown);
                    break;
                }
            }
        }
    }
    if (rc == 0)
    {
        size_t z;
        char obuff [VFS_KRYPTO_PASSWORD_MAX_SIZE+2];

        if (VPathOption (path, vpopt_encrypted, obuff, sizeof obuff, &z) == 0)
        {
            rc = GetEncryptionKey(self, path, obuff, sizeof(obuff), &z);
            if (rc == 0)
            {
                KKey key;
                KFile * encfile;
            
                rc = KKeyInitUpdate (&key, kkeyAES128, obuff, z);
                if (rc == 0)
                {
                    rc = KEncFileMakeWrite (&encfile, file, &key);
                    if (rc == 0)
                    {
                        KFileRelease (file); /* owned by encfile now */
                        *f = encfile;
                        return 0;
                    }
                }
            }
            if (rc)
                KFileRelease (file);
        }
        else
        {
            *f = file;
            return 0;
        }
    }
    return rc;
}


/* CreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] VPath representing the path, URL or URN of the desired file
 */
LIB_EXPORT rc_t CC VFSManagerCreateFile ( const VFSManager *self, KFile **f,
                                          bool update, uint32_t access, KCreateMode mode, const VPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    KFile * file = NULL;
    size_t num_read;
    rc_t rc;
    bool file_created = false;
    char pbuff [4096];
    char rbuff [4096];

    if ((f == NULL) || (path == NULL))
        return RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

    *f = NULL;

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = VPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {

        /* handle a few special case path names
         * This probably needs to be system specifica eventually
         */
        if (strncmp ("/dev/", pbuff, sizeof "/dev/" - 1) == 0)
        {

            if (strcmp ("/dev/stdout", pbuff) == 0)
                rc = KFileMakeStdOut (&file);
            else if (strcmp ("/dev/stderr", pbuff) == 0)
                rc = KFileMakeStdErr (&file);
            else if (strcmp ("/dev/null", pbuff) == 0)
                rc = KFileMakeNullUpdate (&file);
            else if (strncmp ("/dev/fd/", pbuff, sizeof "/dev/fd/" - 1) == 0)
            {
                char * pc;
                size_t ix;

                pc = pbuff + sizeof "/dev/fd/" - 1;

                for (ix = 0; isdigit (pc[ix]); ++ix)
                    ;

                if ((ix > 0)&&(pc[ix] == '\0'))
                {
                    int fd = atoi (pc);

                    rc = KFileMakeFDFileWrite (&file, update, fd);
                }
            }
        }
        if ((rc == 0)&&(file == NULL))
        {
            rc = KDirectoryResolvePath (self->cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
            if (rc == 0)
            {
                uint32_t type;

                type = KDirectoryPathType (self->cwd, "%s", rbuff);
                switch (type & ~kptAlias)
                {
                case kptNotFound:
                case kptFile:
                    rc = KDirectoryCreateFile (self->cwd, &file, update, access, mode,
                                               "%s", rbuff);
                    if (rc == 0)
                        file_created = true;
                    break;

                case kptBadPath:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcInvalid);
                    break;
                case kptDir:
                case kptCharDev:
                case kptBlockDev:
                case kptFIFO:
                case kptZombieFile:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcIncorrect);
                    break;

                default:
                    rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcUnknown);
                    break;
                }
            }
        }
    }
    if (rc == 0)
    {
        size_t z;
        char obuff [VFS_KRYPTO_PASSWORD_MAX_SIZE+2];

        if (VPathOption (path, vpopt_encrypted, obuff, sizeof obuff, &z) == 0)
        {
            rc = GetEncryptionKey(self, path, obuff, sizeof(obuff), &z);
            if (rc == 0)
            {
                KKey key;
                KFile * encfile;
                rc = KKeyInitUpdate (&key, kkeyAES128, obuff, z);

                obuff[z] = '\0';

                rc = KEncFileMakeWrite (&encfile, file, &key);
                if (rc == 0)
                {
                    KFileRelease (file); /* now owned by encfile */
                    *f = encfile;
                    return 0;   
                }
            }
            if (rc)
                KFileRelease (file);
        }
        else
        {
            *f = file;
            return 0;
        }
    }
    if (rc && file_created)
        KDirectoryRemove (self->cwd, true, "%s", rbuff);
    return rc;
}


/* Remove
 *  remove an accessible object from its directory
 *
 *  "force" [ IN ] - if true and target is a directory,
 *  remove recursively
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT rc_t CC VFSManagerRemove ( const VFSManager *self, bool force,
                                      const VPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if (path == NULL)
        return RC (rcVFS, rcMgr, rcOpening, rcParam, rcNull);

    if (self == NULL)
        return RC (rcVFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = VPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        char rbuff [4096];
    
        rc = KDirectoryResolvePath (self->cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
        if (rc == 0)
        {
            uint32_t type;

            type = KDirectoryPathType (self->cwd, "%s", rbuff);
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                break;

            case kptFile:
            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = KDirectoryRemove (self->cwd, force, "%s", rbuff);
                break;

            case kptBadPath:
                rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcInvalid);
                break;
/*                 rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcIncorrect); */
/*                 break; */

            default:
                rc = RC (rcVFS, rcMgr, rcOpening, rcFile, rcUnknown);
                break;
            }
        }
    }
    return rc;
}

/* RemoteProtocols
 */
LIB_EXPORT VRemoteProtocols CC  VRemoteProtocolsParse ( const String * protos )
{
    VRemoteProtocols parsed_protos = 0;

    bool have_proto [ eProtocolMask + 1 ];

    size_t i, end;
    const char * start;
    String http, https, fasp;

    CONST_STRING ( & http,  "http"  );
    CONST_STRING ( & https, "https" );
    CONST_STRING ( & fasp,  "fasp"  );

    end = protos -> size;
    start = protos -> addr;

    memset ( have_proto, 0, sizeof have_proto );

    for ( i = end; i > 0; )
    {
        -- i;
        if ( i == 0 || start [ i ] == ',' )
        {
            VRemoteProtocols parsed_proto = 0;

            /* beginning of protocol string is either 0 or 1 past the comma */
            size_t begin = ( i == 0 ) ? 0 : i + 1;

            /* capture single protocol string */
            String proto;
            StringInit ( & proto, & start [ begin ], end - begin, string_len ( & start [ begin ], end - begin ) );

            /* trim white space */
            StringTrim ( & proto, & proto );

            /* compare against known protocols */
            if ( StringCaseEqual ( & http, & proto ) )
                parsed_proto = eProtocolHttp;
            else if ( StringCaseEqual ( & https, & proto ) )
                parsed_proto = eProtocolHttps;
            else if ( StringCaseEqual ( & fasp, & proto ) )
                parsed_proto = eProtocolFasp;

            if ( parsed_proto != eProtocolNone && ! have_proto [ parsed_proto ] )
            {
                parsed_protos <<= 3;
                parsed_protos |= parsed_proto;
                have_proto [ parsed_proto ] = true;
            }

            end = i;
        }
    }

    return parsed_protos;
}

void KConfigReadRemoteProtocols ( const KConfig * self, VRemoteProtocols * remote_protos )
{
    String * protos;
    rc_t rc = KConfigReadString ( self, "/name-resolver/remote-protocols", & protos );
    if ( rc == 0 )
    {
        VRemoteProtocols parsed_protos = VRemoteProtocolsParse ( protos );
        if ( parsed_protos != 0 )
            * remote_protos = parsed_protos;

        StringWhack ( protos );
    }
}

/* Make
 */
LIB_EXPORT rc_t CC VFSManagerMake ( VFSManager ** pmanager )
{
    return VFSManagerMakeFromKfg(pmanager, NULL);
}

/* Make
 */
static rc_t CC VFSManagerMakeFromKfgImpl ( struct VFSManager ** pmanager,
    struct KConfig * cfg, bool local )
{
    rc_t rc;

    if (pmanager == NULL)
        return RC (rcVFS, rcMgr, rcConstructing, rcParam, rcNull);

    *pmanager = NULL;
    if (!local)
        *pmanager = singleton;
    if ( *pmanager != NULL )
    {
        rc = VFSManagerAddRef ( singleton );
        if ( rc != 0 )
            *pmanager = NULL;
    }
    else
    {
        VFSManager * obj;

        obj = calloc (1, sizeof (*obj));
        if (obj == NULL)
            rc = RC (rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted);
        else
        {
            KRefcountInit (& obj -> refcount, 1,
                kfsmanager_classname, "init", "singleton" );

            /* hard-coded default */
            obj -> protocols = DEFAULT_PROTOCOLS;

            rc = KDirectoryNativeDir ( & obj -> cwd );
            if ( rc == 0 )
            {
                if (cfg == NULL)
                    rc = KConfigMake ( & obj -> cfg, NULL );
                else
                {
                    rc = KConfigAddRef ( cfg );
                    if (rc == 0)
                        obj -> cfg = cfg;
                }
                if ( rc == 0 )
                {
                    /* look for remote protocols in configuration */
                    KConfigReadRemoteProtocols ( obj -> cfg, & obj -> protocols );

                    rc = KCipherManagerMake ( & obj -> cipher );
                    if ( rc == 0 )
                    {
                        rc = KKeyStoreMake ( & obj -> keystore, obj -> cfg );
                        if ( rc == 0 )
                        {
                            if (local)
                                rc = KNSManagerMakeLocal ( & obj -> kns, cfg );
                            else
                                rc = KNSManagerMakeWithConfig
                                                         ( & obj -> kns, cfg );
                            if ( rc != 0 )
                            {
                                LOGERR ( klogWarn, rc, "could not build network manager" );
                                rc = 0;
                            }

                            rc = VFSManagerMakeResolver ( obj, & obj -> resolver, obj -> cfg );
                            if ( rc != 0 )
                            {
                                LOGERR ( klogWarn, rc, "could not build vfs-resolver" );
                                rc = 0;
                            }

                            *pmanager = obj;
                            if (!local)
                                singleton = obj;
                            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_MGR),  ("%s(%p)\n", __func__, cfg));
                            return 0;
                        }
                    }
                }
            }
        }

        VFSManagerDestroy (obj);
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerMakeFromKfg ( struct VFSManager ** pmanager,
    struct KConfig * cfg)
{
    return VFSManagerMakeFromKfgImpl(pmanager, cfg, false);
}

LIB_EXPORT rc_t CC VFSManagerMakeLocal ( struct VFSManager ** pmanager,
    struct KConfig * cfg)
{
    return VFSManagerMakeFromKfgImpl(pmanager, cfg, true);
}

LIB_EXPORT rc_t CC VFSManagerGetCWD (const VFSManager * self, KDirectory ** cwd)
{
    rc_t rc;

    if ( cwd == NULL )
        rc = RC (rcVFS, rcMgr, rcAccessing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC (rcVFS, rcMgr, rcAccessing, rcSelf, rcNull);
        else
        {
            rc = KDirectoryAddRef ( self -> cwd );
            if ( rc == 0 )
            {
                * cwd = self -> cwd;
                return 0;
            }
        }

        * cwd = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VFSManagerGetResolver ( const VFSManager * self, struct VResolver ** resolver )
{
    rc_t rc;

    if ( resolver == NULL )
        rc = RC (rcVFS, rcMgr, rcAccessing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC (rcVFS, rcMgr, rcAccessing, rcSelf, rcNull);
        else if ( self -> resolver == NULL )
            rc = RC ( rcVFS, rcMgr, rcAccessing, rcResolver, rcNull );
        else
        {
            rc = VResolverAddRef ( self -> resolver );
            if ( rc == 0 )
            {
                * resolver = self -> resolver;
                return 0;
            }
        }

        * resolver = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VFSManagerSetResolver ( VFSManager * self, VResolver * resolver )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC (rcVFS, rcMgr, rcUpdating, rcSelf, rcNull);
    else if ( resolver == NULL )
        rc = RC (rcVFS, rcMgr, rcUpdating, rcParam, rcNull);
    else if (self -> resolver != resolver ) {
        rc = VResolverAddRef ( resolver );
        if (rc == 0) {
            rc = VResolverRelease ( self -> resolver );
            if (rc == 0) {
                self -> resolver = resolver;
                return 0;
            }
        }
        VResolverRelease ( resolver );
    }

    return rc;
}


LIB_EXPORT rc_t CC VFSManagerGetKNSMgr ( const VFSManager * self, struct KNSManager ** kns )
{
    rc_t rc;

    if ( kns == NULL )
        rc = RC (rcVFS, rcMgr, rcAccessing, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC (rcVFS, rcMgr, rcAccessing, rcSelf, rcNull);
        else
        {
            rc = KNSManagerAddRef ( self -> kns );
            if ( rc == 0 )
            {
                * kns = self -> kns;
                return 0;
            }
        }

        * kns = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC VFSManagerGetKryptoPassword (const VFSManager * self,
                                                char * password,
                                                size_t max_size,
                                                size_t * size)
{
    rc_t rc;

    if (self == NULL)
        rc = RC (rcVFS, rcMgr, rcAccessing, rcSelf, rcNull);

    else if ((password == NULL) || (max_size == 0) || (size == NULL))
        rc = RC (rcVFS, rcMgr, rcAccessing, rcParam, rcNull);

    else
    {
        size_t z;
        char obuff [4096 + 16];

        rc = VFSManagerGetConfigPWFile(self, obuff, sizeof obuff, &z);
        if (rc == 0)
        {
            VPath * vpath;
            rc_t rc2;
            rc = VPathMake (&vpath, obuff);
            if (rc == 0)
                rc = GetEncryptionKey(self, vpath, password, max_size, size);
            rc2 = VPathRelease (vpath);
            if (rc == 0)
                rc = rc2;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerUpdateKryptoPassword (const VFSManager * self, 
                                                   const char * password,
                                                   size_t size,
                                                   char * pwd_dir,
                                                   size_t pwd_dir_size)
{
    static const char temp_extension [] = ".tmp";
    rc_t rc;

    if (self == NULL)
        rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcSelf, rcNull);

    else if ((password == NULL) || (size == 0))
        rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcParam, rcNull);

    else if (size > VFS_KRYPTO_PASSWORD_MAX_SIZE)
        rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcSize, rcExcessive);

    else if ((string_chr (password, size, '\n') != NULL) ||
             (string_chr (password, size, '\r') != NULL))
        rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcEncryptionKey, rcInvalid);

    else
    {
        size_t old_password_file_size;
        char old_password_file [8193];
        
        rc = VFSManagerGetConfigPWFile (self, old_password_file,
                                        sizeof old_password_file - 1,
                                        &old_password_file_size);
        if (rc) {
            if (rc ==
                SILENT_RC(rcKrypto, rcMgr, rcReading, rcBuffer, rcInsufficient))
            {
                rc =
                    RC(rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcExcessive);
            }
            LOGERR (klogErr, rc, "failed to obtain configured path for password file");
        }

        else if (old_password_file_size >= (sizeof old_password_file - 1))
        {
            rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcExcessive);
            PLOGERR (klogErr,
                     (klogErr, rc, "configured path too long for function "
                      "'$(P)' '${F}'", "P=%s,F=%s",
                      old_password_file, __func__));
        }
        else
        {
            KPathType ftype;
            bool old_exists;

            old_password_file[old_password_file_size] = '\0';
            ftype = KDirectoryPathType (self->cwd, "%s", old_password_file);

            switch (ftype)
            {
            case kptNotFound:
                old_exists = false;
                break;

            case kptBadPath:
                rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcInvalid);
                break;

            case kptFile:
                old_exists = true;
                break;

            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
            case kptDataset:
            case kptDatatype:
                rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcIncorrect);
                break;

            default:
                rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcCorrupt);
                break;
            }

            if (rc)
                PLOGERR (klogErr,
                         (klogErr, rc, "cannot use configured path for "
                          "password file '$(P)'", "P=%s", old_password_file));

            else
            {
                VPath * vold;
                /* size_t new_password_file_size; */
                char new_password_file [sizeof old_password_file + sizeof temp_extension];
                size_t password_dir_size;
                char password_dir [sizeof old_password_file];
/*                 bool save_old_password; */
                char * pc;

                memmove (password_dir, old_password_file, old_password_file_size);
                memmove (new_password_file, old_password_file, old_password_file_size);
                memmove (new_password_file + old_password_file_size, temp_extension, sizeof temp_extension);
                /* new_password_file_size = old_password_file_size + sizeof temp_extension - 1; */

                pc = string_rchr (password_dir, old_password_file_size, '/');
                if (pc == NULL)
                {
                    password_dir[0] = '.';
                    pc = password_dir+1;
                }
                *pc = '\0';
                password_dir_size = pc - password_dir;

                if (pwd_dir && pwd_dir_size) {
                    size_t n = string_copy(pwd_dir, pwd_dir_size,
                                           password_dir, password_dir_size + 1);
                    if (n >= pwd_dir_size) {
                        int i = 0;
                        size_t p = pwd_dir_size - 1;
                        pwd_dir[p] = '\0';
                        for (i = 0; i < 3; ++i) {
                            if (p == 0)
                            {   break; }
                            pwd_dir[--p] = '.';
                        }
                        if (p != 0)
                        {   pwd_dir[--p] = ' '; }
                    }
                }

                rc = VPathMake (&vold, old_password_file);
                if (rc)
                    PLOGERR (klogErr,
                             (klogErr, rc, "could not create vpath for "
                              "password file '$(P)'", "P=%s",
                              old_password_file));

                else
                {
                    VPath * vnew;

                    rc = VPathMake (&vnew, new_password_file);
                    if (rc)
                        PLOGERR (klogErr,
                                 (klogErr, rc, "could not create vpath for "
                                  "password file '$(P)'", "P=%s",
                                  new_password_file));

                    else
                    {
                        const KFile * fold = NULL;
                        KFile * fnew = NULL;

                        if (old_exists)
                        {
                            rc = VFSManagerOpenFileRead ( self, &fold, vold );

                            if (rc)
                                PLOGERR (klogErr,
                                         (klogErr, rc, "unable to open existing "
                                          "password file '$(P)'", "P=%s",
                                          old_password_file));
                        }
                        

                        if (rc == 0)
                        {
                            rc = VFSManagerCreateFile (self, &fnew, false, 0600,
                                                       kcmInit|kcmParents,
                                                       vnew);
                            if (rc)
                                PLOGERR (klogErr,
                                         (klogErr, rc, "unable to open temporary "
                                          "password file '$(P)'", "P=%s",
                                          new_password_file));

                            else
                            {
                                uint64_t writ;
                                size_t this_writ;

                                rc = KFileWriteAll (fnew, 0, password, size, &this_writ);
                                if (rc)
                                    PLOGERR (klogErr,
                                             (klogErr, rc, "unable to write "
                                              "password to temporary password "
                                              "file '$(P)'", "P=%s",
                                              new_password_file));

                                else if (this_writ != size)
                                {
                                    rc = RC (rcVFS, rcEncryptionKey, rcWriting,
                                             rcFile, rcInsufficient);
                                    PLOGERR (klogErr,
                                             (klogErr, rc, "unable to write complete "
                                              "password to temporary password "
                                              "file '$(P)'", "P=%s",
                                              new_password_file));
                                }

                                else
                                {
                                    writ = this_writ;

                                    rc = KFileWriteAll (fnew, this_writ, "\n", 1, &this_writ);
                                    if (rc)
                                        PLOGERR (klogErr,
                                                 (klogErr, rc, "unable to write "
                                                  "password to temporary password "
                                                  "file '$(P)'", "P=%s",
                                                  new_password_file));

                                    else if (this_writ != 1)
                                    {
                                        rc = RC (rcVFS, rcEncryptionKey, rcWriting,
                                                 rcFile, rcInsufficient);
                                        PLOGERR (klogErr,
                                                 (klogErr, rc, "unable to write complete "
                                                  "password to temporary password "
                                                  "file '$(P)'", "P=%s",
                                                  new_password_file));
                                    }

                                    else
                                    {
                                        bool do_rename;

                                        do_rename = true;
                                        ++writ;

                                        if (old_exists)
                                        {
                                            size_t read;
                                            size_t this_read;
                                            char buffer [VFS_KRYPTO_PASSWORD_MAX_SIZE+4];

                                            rc = KFileReadAll (fold, 0, buffer,
                                                               sizeof buffer, &this_read);
                                            if (rc)
                                                ;

                                            else
                                            {
                                                read = this_read;
                                                /* look for duplicated password */
                                                if (read > size)
                                                {
                                                    char cc;

                                                    cc = buffer[size];
                                                    if (((cc == '\n') || (cc == '\r')) &&
                                                        (memcmp (buffer, password, size) == 0))
                                                    {
                                                        do_rename = false;
                                                    }
                                                }
                                                if (read)
                                                    rc = KFileWriteAll (fnew, writ, buffer, read, &this_writ);

                                                if (rc)
                                                    ;
                                                else if (do_rename)
                                                {
                                                    writ += this_writ;

                                                    do
                                                    {
                                                        rc = KFileReadAll (fold, read, buffer,
                                                                           sizeof buffer, &this_read);
                                                        if (rc)
                                                            ;

                                                        else if (this_read == 0)
                                                            break;

                                                        else
                                                        {
                                                            rc = KFileWriteAll (fnew, writ, buffer,
                                                                                this_read, &this_writ);
                                                            if (rc)
                                                                ;

                                                            else if (this_read != this_writ)
                                                            {
                                                                rc = RC (rcVFS, rcEncryptionKey, rcWriting,
                                                                         rcFile, rcInsufficient);
                                                                PLOGERR (klogErr,
                                                                         (klogErr, rc,
                                                                          "unable to write complete "
                                                                          "password to temporary password "
                                                                          "file '$(P)'", "P=%s",
                                                                          new_password_file));
                                                            }

                                                            else
                                                            {
                                                                read += this_read;
                                                                writ += this_writ;
                                                            }
                                                        }
                                                    } while (rc == 0);
                                                }
                                            }
                                            KFileRelease (fold);
                                            fold = NULL;
                                        }

                                        KFileRelease (fnew);
                                        fnew = NULL;

                                        if (rc == 0)
                                        {
                                            if (do_rename)
                                            {
                                                rc = KDirectoryRename (self->cwd, true, 
                                                                       new_password_file,
                                                                       old_password_file);
                                            }
                                            else
                                            {
                                                KDirectoryRemove (self->cwd, true, "%s", new_password_file);
                                            }

#if !WINDOWS
                                            if (rc == 0)
                                            {
                                                uint32_t access;

                                                rc = KDirectoryAccess (self->cwd,
                                                                       &access, "%s", password_dir);
                                                if (rc)
                                                    ;

                                                else
                                                {
                                                    if (access & 0027)
                                                        rc = RC (rcVFS, rcEncryptionKey, rcUpdating, rcDirectory, rcExcessive);
                                                }
                                            }
#endif
                                        }
                                    }
                                }
                                KFileRelease (fnew);
                            }
                            KFileRelease (fold);
                        }
                        VPathRelease (vold);
                    }
                    VPathRelease (vnew);
                }
            }
        }
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * KConfig
 *  placing some KConfig code that relies upon VFS here
 */


/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigReadVPath ( struct KConfig const* self, const char* path, struct VPath** result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        struct KConfigNode const *n;
        rc = KConfigOpenNodeRead ( self, & n, "%s", path );
        if ( rc == 0 )
        {
            rc = KConfigNodeReadVPath ( n, result );
            KConfigNodeRelease ( n );
            return rc;
        }

        * result = NULL;
    }

    return rc;
}

/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
LIB_EXPORT rc_t CC KConfigNodeReadVPath ( struct KConfigNode const *self, struct VPath** result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        char buffer [ 4096 ];
        size_t num_read, to_read;
        rc = KConfigNodeRead ( self, 0, buffer, sizeof buffer, & num_read, & to_read );
        if ( rc == 0 )
        {
            char *p;

            if ( to_read == 0 && num_read < sizeof buffer )
            {
                buffer [ num_read ] = 0;
                return VPathMake ( result, buffer );
            }

            p = malloc ( num_read + to_read + 1 );
            if ( p == NULL )
                rc = RC ( rcKFG, rcNode, rcReading, rcMemory, rcExhausted );
            else
            {
                rc = KConfigNodeRead ( self, 0, p, num_read + to_read + 1, & num_read, & to_read );
                if ( rc == 0 )
                {
                    p [ num_read ] = 0;
                    rc = VPathMake ( result, p );
                }

                free ( p );
                return rc;
            }
        }

        * result = NULL;
    }

    return rc;
}


static rc_t VFSManagerResolveAcc( const VFSManager * self,
                                  const struct VPath * source,
                                  struct VPath ** path_to_build,
                                  const struct KFile ** remote_file,
                                  const struct VPath ** local_cache )
{
    rc_t rc;
    const VPath * local, * remote;
    
    assert (self);
    assert (source);
    assert (path_to_build);
    assert (remote_file);
    assert (local_cache);

#if 1
    rc = VResolverQuery ( self -> resolver, self -> protocols, source, & local, & remote, local_cache );
    if ( rc == 0 )
    {
        assert ( local != NULL || remote != NULL );
        assert ( local == NULL || remote == NULL );
        * path_to_build = ( VPath* ) ( ( local != NULL ) ? local : remote );
    }
#else

    /* first try to find it localy */
    rc = VResolverLocal ( self->resolver, source, (const VPath **)path_to_build );
    if ( GetRCState( rc ) == rcNotFound )
    {
        /* if not found localy, try to find it remotely */
        rc = VResolverRemote ( self->resolver, self -> protocols,
            source, (const VPath **)path_to_build, remote_file );
        if ( rc == 0 && remote_file != NULL && local_cache != NULL )
        {
            /* if found and the caller wants to know the location of a local cache file... */
            uint64_t size_of_remote_file = 0;
            if ( *remote_file != NULL )
                rc = KFileSize ( *remote_file, &size_of_remote_file );
            if ( rc ==  0 )
                rc = VResolverCache ( self->resolver, *path_to_build, local_cache, size_of_remote_file );
        }
    }

#endif
    return rc;
}


static rc_t VFSManagerResolveLocal( const VFSManager * self,
                                    const char * local_path,
                                    struct VPath ** path_to_build )
{
    assert ( self != NULL );
    assert ( local_path != NULL && local_path [ 0 ] != 0 );
    assert ( path_to_build != NULL );

    return VFSManagerMakePath ( self, path_to_build, "ncbi-file:%s", local_path );
}

static rc_t VFSManagerResolvePathOrAcc( const VFSManager * self,
                                        const struct VPath * source,
                                        struct VPath ** path_to_build,
                                        const struct KFile ** remote_file,
                                        const struct VPath ** local_cache,
                                        bool resolve_acc )
{
    char buffer[ 4096 ];
    size_t num_read;
    rc_t rc = VPathReadPath ( source, buffer, sizeof buffer, &num_read );
    if ( rc == 0 && num_read > 0 )
    {
        char * pos_of_slash = string_chr ( buffer, string_size( buffer ), '/' );
        if ( pos_of_slash != NULL )
        {
            /* we can now assume that the source is a filesystem-path :
               we build a new VPath and prepend with 'ncbi-file:' */
            rc = VFSManagerResolveLocal( self, buffer, path_to_build );
        }
        else if ( resolve_acc )
        {
            /* we assume the source is an accession! */
            rc = VFSManagerResolveAcc( self, source, path_to_build, remote_file, local_cache );
            if ( GetRCState( rc ) == rcNotFound )
            {
                /* if we were not able to find the source as accession, we assume it is a local path */
                rc = VFSManagerResolveLocal( self, buffer, path_to_build );
            }
        }
        else
        {
            rc = RC ( rcVFS, rcMgr, rcAccessing, rcParam, rcInvalid );
        }
    }
    return rc;
}


static rc_t VFSManagerResolveRemote( const VFSManager * self,
                                     struct VPath ** source,
                                     struct VPath ** path_to_build,
                                     const struct KFile ** remote_file,
                                     const struct VPath ** local_cache )
{
    rc_t rc = 0;
    *path_to_build = *source;
    if ( local_cache != NULL && remote_file != NULL && self->resolver != NULL )
    {

/*        VFS_EXTERN rc_t CC VPathMakeString ( const VPath * self, const String ** uri ); */
        char full_url[ 4096 ];
        size_t num_read;
        rc = VPathReadPath ( *source, full_url, sizeof full_url, &num_read );
        if ( rc == 0 && num_read > 0 )
        {
            rc = KNSManagerMakeHttpFile ( self -> kns, remote_file, NULL, 0x01010000, full_url );
            if ( rc == 0 )
            {
                uint64_t size_of_remote_file = 0;
                rc = KFileSize ( *remote_file, &size_of_remote_file );
                if ( rc == 0 )
                    rc = VResolverCache ( self->resolver, *source, local_cache, size_of_remote_file );
            }
        }
    }
    *source = NULL;
    return rc;
}

/* DEPRECATED */
LIB_EXPORT rc_t CC VFSManagerResolveSpec ( const VFSManager * self,
                                           const char * spec,
                                           struct VPath ** path_to_build,
                                           const struct KFile ** remote_file,
                                           const struct VPath ** local_cache,
                                           bool resolve_acc )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcAccessing, rcSelf, rcNull );
    else if ( spec == NULL || path_to_build == NULL )
        rc = RC ( rcVFS, rcMgr, rcAccessing, rcParam, rcNull );
    else if ( spec[ 0 ] == 0 )
        rc = RC ( rcVFS, rcMgr, rcAccessing, rcParam, rcEmpty );
    else
    {
        VPath * temp;
        *path_to_build = NULL;
        if ( local_cache != NULL )
            *local_cache = NULL;
        if ( remote_file != NULL ) 
            *remote_file = NULL;
        rc = VPathMake ( &temp, spec );
        if ( rc == 0 )
        {
            VPUri_t uri_type;
            rc = VPathGetScheme_t( temp, &uri_type );
            if ( rc == 0 )
            {
                switch ( uri_type )
                {
                default                  : /* !! fall through !! */
                case vpuri_invalid       : rc = RC ( rcVFS, rcMgr, rcAccessing, rcParam, rcInvalid );
                                           break;

                case vpuri_none          : /* !! fall through !! */
                case vpuri_not_supported : rc = VFSManagerResolvePathOrAcc( self, temp, path_to_build, remote_file, local_cache, resolve_acc );
                                           break;

                case vpuri_ncbi_vfs      : /* !! fall through !! */
                case vpuri_file          : *path_to_build = temp;
                                           temp = NULL;
                                           break;

                case vpuri_ncbi_acc      : if ( resolve_acc )
                                                rc = VFSManagerResolveAcc( self, temp, path_to_build, remote_file, local_cache );
                                           else
                                                rc = RC ( rcVFS, rcMgr, rcAccessing, rcParam, rcInvalid );
                                           break;

                case vpuri_http          : /* !! fall through !! */
                case vpuri_https:
                case vpuri_ftp           : rc = VFSManagerResolveRemote( self, &temp, path_to_build, remote_file, local_cache );
                                           break;

                case vpuri_ncbi_legrefseq: /* ??? */
                                           break;
                }
            }
            if ( temp != NULL )
                VPathRelease ( temp );
        }
    }
    return rc;
}

LIB_EXPORT const struct KConfig* CC VFSManagerGetConfig(const struct VFSManager * self)
{
    if ( self == NULL )
        return NULL;
    return self->cfg;
}

/*
 * Object Id / Object name bindings for accessions and dbGaP files
 */

#define MAX_OBJID_SIZE 20
#define MAX_NAME_SIZE 4096

LIB_EXPORT void VFSManagerSetBindingsFile(struct VFSManager * self, const char* path)
{
    if (self != NULL)
        KKeyStoreSetBindingsFile( self->keystore, path);
}

LIB_EXPORT const char* VFSManagerGetBindingsFile(struct VFSManager * self)
{
    if (self == NULL)
        return NULL;
    return KKeyStoreGetBindingsFile(self->keystore);
}

LIB_EXPORT rc_t CC VFSManagerRegisterObject(struct VFSManager* self, uint32_t oid, const struct VPath* obj)
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcRegistering, rcSelf, rcNull );
    else if ( obj == NULL )
        rc = RC ( rcVFS, rcMgr, rcRegistering, rcParam, rcNull );

/* VDB-3503: VFSManagerRegisterObject is used just to register oid<->filename
             mapping when working with kart files.
             The following tests were added to make sure 'obj' is correct to
             generate cache location of protected files */
    else if ( obj -> path_type == vpNameOrOID )
        rc = RC ( rcVFS, rcMgr, rcRegistering, rcPath, rcWrongType );
    else if ( obj -> scheme_type != vpuri_ncbi_acc && 
              obj -> scheme_type != vpuri_ncbi_file )
        rc = RC ( rcVFS, rcMgr, rcRegistering, rcPath, rcWrongType );
    else if ( obj -> query . size == 0 )
        rc = RC ( rcVFS, rcMgr, rcRegistering, rcQuery, rcEmpty );

    else
    {
        const String* newName;
        rc = VPathMakeString (obj, &newName);
        if (rc == 0)
        {
            rc = KKeyStoreRegisterObject(self->keystore, oid, newName);
            StringWhack(newName);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerGetObject(const struct VFSManager* self, uint32_t oid, struct VPath** obj)
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcRetrieving, rcSelf, rcNull );
    else if ( obj == NULL )
        rc = RC ( rcVFS, rcMgr, rcRetrieving, rcParam, rcNull );
    else
    {
        const String* objName;
        rc = KKeyStoreGetObjectName(self->keystore, oid, &objName);
        if (rc == 0)
        {
            rc = VFSManagerMakePath (self, obj, "%S", objName);
            StringWhack(objName);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerGetObjectId(const struct VFSManager* self, const struct VPath* obj, uint32_t* oid)
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcRetrieving, rcSelf, rcNull );
    else if ( obj == NULL || oid == NULL)
        rc = RC ( rcVFS, rcMgr, rcRetrieving, rcParam, rcNull );
    else
    {
        const String* pathString;
        rc = VPathMakeString(obj, &pathString);
        if (rc == 0)
        {
            rc = VKKeyStoreGetObjectId(self->keystore, pathString, oid);
            StringWhack(pathString);
        }
    }
    return rc;
}


static const char * default_path_key = "/repository/user/default-path";

LIB_EXPORT rc_t CC VFSManagerGetCacheRoot ( const VFSManager * self,
    struct VPath const ** path )
{
    rc_t rc;
    if ( path == NULL )
        rc = RC ( rcVFS, rcMgr, rcListing, rcParam, rcNull );
    else
    {
        * path = NULL;
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcListing, rcSelf, rcNull );
        else if ( self -> cfg == NULL )
            rc = RC ( rcVFS, rcMgr, rcListing, rcItem, rcNull );
        else
        {
            struct String * spath;
            rc = KConfigReadString ( self -> cfg, default_path_key, &spath );
            if ( rc == 0 )
            {
                struct VPath * vp;
                rc = VFSManagerMakePath ( self, &vp, "%S", spath );
                if ( rc == 0 )
                    *path = vp;
                StringWhack( spath );
            }
        }
    }
    return rc;
}


/*
    repo-path for instance '/repository/user/main/public'
    read $(repo-path)/root, put it into frozen-list ( if is not already there )
    write $(repository/user/default-path)/public as value into it ( just in case )
*/
static const char * indirect_root = "$(repository/user/default-path)/%s";

LIB_EXPORT rc_t CC VFSManagerSetCacheRoot ( const VFSManager * self,
    struct VPath const * path )
{
    rc_t rc;
    if ( path == NULL )
        rc = RC ( rcVFS, rcMgr, rcSelecting, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcSelecting, rcSelf, rcNull );
    else if ( self -> cfg == NULL )
        rc = RC ( rcVFS, rcMgr, rcSelecting, rcItem, rcNull );
    else
    {
        /* loop through the user-repositories to set the root property to the indirect path */
        KRepositoryMgr * repo_mgr;
        rc = KConfigMakeRepositoryMgrUpdate ( self -> cfg, &repo_mgr );
        if ( rc == 0 )
        {
            KRepositoryVector user_repos;
            rc = KRepositoryMgrUserRepositories ( repo_mgr, &user_repos );
            if ( rc == 0 )
            {
                uint32_t start = VectorStart( &user_repos );
                uint32_t count = VectorLength( &user_repos );
                uint32_t idx;
                for ( idx = 0; rc == 0 && idx < count; ++idx )
                {
                    KRepository * repo = VectorGet ( &user_repos, idx + start );
                    if ( repo != NULL )
                    {
                        /* ask the repository to add it's current root to the root-history */
                        rc = KRepositoryAppendToRootHistory( repo, NULL );
                        if ( rc == 0 )
                        {
                            char repo_name[ 512 ];
                            size_t repo_name_len;
                            rc = KRepositoryName( repo, repo_name, sizeof repo_name, &repo_name_len );
                            if ( rc == 0 )
                            {
                                char new_root[ 4096 ];
                                size_t num_writ;
                                repo_name[ repo_name_len ] = 0;
                                rc = string_printf( new_root, sizeof new_root, &num_writ, indirect_root, repo_name );
                                if ( rc == 0 )
                                    rc = KRepositorySetRoot( repo, new_root, string_size( new_root ) );
                            }
                        }
                    }
                }
                KRepositoryVectorWhack ( &user_repos );
            }
            KRepositoryMgrRelease ( repo_mgr );
        }

        /* write the new indirect path */
        if ( rc == 0 )
        {
            String const * spath = NULL;
            rc = VPathMakeString ( path, &spath );
            if ( rc == 0 )
            {
                /* in case the path ends in a '/' ( the path-separator ) we have to remove it... */
                if ( spath->addr[ spath->len - 1 ] == '/' )
                {
                    String * p = ( String * )spath;
                    p->len -= 1;
                    p->size -= 1;
                    ( ( char * )p->addr )[ p->len ] = 0;
                }
                rc = KConfigWriteSString( self -> cfg, default_path_key, spath );
                StringWhack( spath );
                /*
                    we do not commit, because ticket VDB-3060: 
                    GBench wants to change the cache-root, but to automatically revert to previous value
                    when GBench exits, this is achieved by not commiting here.
                if ( rc == 0 )
                    rc = KConfigCommit ( self -> cfg );
                */
            }
        }
    }
    return rc;
}


static rc_t inspect_file( KDirectory * dir, KTime_t date, const char * path )
{
    KTime_t file_date;
    rc_t rc = KDirectoryDate ( dir, &file_date, "%s", path );
    if ( rc == 0 )
    {
        if ( file_date < date )
            KDirectoryRemove ( dir, false, "%s", path );
    }
    return rc;
} 


static rc_t inspect_dir( KDirectory * dir, KTime_t date, const char * path )
{
    KNamelist * itemlist;
    rc_t rc = KDirectoryList( dir, &itemlist, NULL, NULL, "%s", path );
    if ( rc == 0 )
    {
        uint32_t count, idx;
        rc = KNamelistCount ( itemlist, &count );
        for ( idx = 0; rc == 0 && idx < count; idx++ )
        {
            const char * item;
            rc = KNamelistGet ( itemlist, idx, &item );
            {
                char item_path[ 4096 ];
                size_t num_writ;
                rc = string_printf ( item_path, sizeof item_path, &num_writ, "%s/%s", path, item );
                if ( rc == 0 )
                {
                    uint32_t pathtype = KDirectoryPathType( dir, "%s", item_path );
                    switch( pathtype )
                    {
                        case kptFile : rc = inspect_file( dir, date, item_path ); break;
                        case kptDir  : rc = inspect_dir( dir, date, item_path ); break; /* recursion! */
                        default : break;
                    }
                }
            }
        }
        KNamelistRelease( itemlist );
    }
    else
    {
		if ( ( GetRCModule( rc ) == rcFS ) && 
			 ( GetRCTarget( rc ) == rcDirectory ) &&
			 ( GetRCContext( rc ) == rcListing ) &&
			 ( GetRCObject( rc ) == ( enum RCObject )rcPath ) &&
			 ( GetRCState( rc ) == rcNotFound ) )
		{
			rc = 0;
		}
		else
		{
			PLOGERR( klogErr, ( klogErr, rc, "KDirectoryList( '$(P)' )", "P=%s", path ) );
		}
    }
    return rc;
}


LIB_EXPORT rc_t CC VFSManagerDeleteCacheOlderThan ( const VFSManager * self,
    uint32_t days )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcVFS, rcMgr, rcSelecting, rcSelf, rcNull );
    else if ( self -> cfg == NULL )
        rc = RC ( rcVFS, rcMgr, rcSelecting, rcItem, rcNull );
    else
    {
        /* loop through the user-repositories to get the root property */
        const KRepositoryMgr * repo_mgr;
        rc = KConfigMakeRepositoryMgrRead ( self -> cfg, &repo_mgr );
        if ( rc == 0 )
        {
            KRepositoryVector user_repos;
            rc = KRepositoryMgrUserRepositories ( repo_mgr, &user_repos );
            if ( rc == 0 )
            {
                uint32_t start = VectorStart( &user_repos );
                uint32_t count = VectorLength( &user_repos );
                uint32_t idx;
                for ( idx = 0; rc == 0 && idx < count; ++idx )
                {
                    KRepository * repo = VectorGet ( &user_repos, idx + start );
                    if ( repo != NULL )
                    {
                        char path[ 4096 ];
                        size_t root_size;
                        rc = KRepositoryRoot ( repo, path, sizeof path, &root_size );
                        if ( rc == 0 )
                        {
                            KTime_t date = KTimeStamp() - ( days * 60 * 60 * 24 );
                            rc = inspect_dir( self->cwd, date, path );
                        }
                    }
                }
                KRepositoryVectorWhack ( &user_repos );
            }
            KRepositoryMgrRelease ( repo_mgr );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerSetAdCaching(VFSManager * self, bool enabled) {
    if (self != NULL)
        return KNSManagerSetAdCaching(self->kns, enabled);
    return 0;
}
