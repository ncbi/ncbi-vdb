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
 * =============================================================================
 *
 */


/******************************************************************************/


#include <vfs/extern.h>

#include <klib/container.h> /* BSTree */
#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* KLogLevel */
#include <klib/out.h> /* KOutMsg */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* String */
#include <klib/time.h> /* KTime */
#include <klib/vector.h> /* Vector */

#include <kfg/config.h> /* KConfigRelease */
#include <kfg/kart-priv.h> /* KartItemMake2 */
#include <kfg/repository.h> /* KRepositoryMgrRelease */

#include <kns/http.h> /* KHttpRequest */
#include <kns/http-priv.h> /* KClientHttpRequestFormatMsg */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeReliableClientRequest */
#include <kns/manager.h> /* KNSManager */
#include <kns/stream.h> /* KStreamMakeFromBuffer */

#include <kproc/timeout.h> /* TimeoutInit */

#include <vfs/manager.h> /* VFSManager */
#include <vfs/services.h> /* KServiceMake */

#include "path-priv.h" /* VPathMakeFmt */
#include "resolver-priv.h" /* VPathCheckFromNamesCGI */
#include "services-priv.h"

#include <ctype.h> /* isdigit */
#include <string.h> /* memset */


/******************************************************************************/


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)


/******************************************************************************/
/* service type - names of search */
typedef enum {
    eSTnames,
    eSTsearch,
} EServiceType;


/* request/response/processing helper objects */
typedef struct {
    struct       KConfig        * kfg;

    const struct KNSManager     * kMgr;
                               /* KNSManagerMakeReliableClientRequest */

    const struct KRepositoryMgr * repoMgr;
                               /* KRepositoryMgrGetProtectedRepository */

    uint32_t timeoutMs;
} SHelper;


/* raw string text */
typedef struct { char * s; } SRaw; 


/******************************************************************************/
/* SERVICE VERSIONS */
#define VERSION_1_0 0x01000000
#define VERSION_1_1 0x01010000
#define VERSION_1_2 0x01020000
#define VERSION_3_0 0x03000000
/*#define VERSION_3_1 0x03010000
#define VERSION_3_2 0x03020000*/


/* version in server request / response */
typedef struct {
    SRaw raw;
    ver_t version;
    uint8_t major;
    uint8_t minor;
} SVersion; 

/* features that are different for different protocol versions ****************/
static bool SVersionNotExtendedVPaths ( const SVersion * self ) {
    assert ( self );
    return self -> version == VERSION_1_0;
}

static bool SVersionBefore3_0 ( const SVersion * self ) {
    assert ( self );
    return self -> version < VERSION_3_0;
}

static bool SVersionHasRefseqCtx ( const SVersion * self ) {
    assert ( self );
    return self -> version < VERSION_3_0;
}

static bool SVersionAccInRequest ( const SVersion * self ) {
    assert ( self );
    return self -> version < VERSION_3_0;
}

static bool SVersionTypInRequest ( const SVersion * self ) {
    assert ( self );
    return self -> version == VERSION_3_0;
}

static bool SVersionHasMultpileObjects ( const SVersion * self ) {
    assert ( self );
    return self -> version >= VERSION_3_0;
}

static bool SVersionSingleUrl ( const SVersion * self ) {
    assert ( self );
    return self -> version < VERSION_3_0;
}

static bool SVersionResponseHasMultipeUrls ( const SVersion * self ) {
    assert ( self );
    return self -> version >= VERSION_3_0;
}

static bool SVersionResponseHasTimestamp ( const SVersion * self ) {
    assert ( self );
    return self -> version >= VERSION_3_0;
}
/******************************************************************************/


/* server response header */
typedef struct {
    SRaw raw;
    SVersion version;
} SHeader;


/* number of fields in different versions of name resolver protocol */
#define N_NAMES1_0 5
#define N_NAMES1_1 10
#define N_NAMES3_0  15

/* md5 checksum */
typedef struct {
    uint8_t md5 [ 16 ];
    bool has_md5;
} SMd5;


/* response row parsed into named typed fields */
typedef struct {
    bool inited;
    uint32_t ordId;
    EObjectType objectType;
    String accession; /* versios 1.1/1.2 only */
    String objectId;
    String name;
    uint64_t osize;
    KTime_t date;
    SMd5 md5;
    String ticket;
    String url;
    String hUrl;
    String fpUrl;
    String hsUrl;
    String flUrl;
    String s3Url;
    size_t vdbcacheSize;
    KTime_t vdbcacheDate;
    SMd5 vdbcacheMd5;
    String vdbcacheUrl;
    String hVdbcacheUrl;
    String fpVdbcacheUrl;
    String hsVdbcacheUrl;
    String flVdbcacheUrl;
    String s3VdbcacheUrl;
    KTime_t expiration;
    uint32_t code;
    String message;
} STyped;


/* converter from server response string to a typed object */
typedef rc_t TConverter ( void * dest, const String * src );
typedef struct { TConverter * f; } SConverter;
typedef void * TFieldGetter ( STyped * self, int n );

typedef struct {
    int n;
    TFieldGetter * get;
    TConverter ** f;
} SConverters;


/* response row parsed into array of Strings */
typedef struct {
    int n;
    String s [ N_NAMES3_0 ];
} SOrdered;


/* server error row */
struct KSrvError {
    atomic32_t refcount;

    rc_t rc;
    uint32_t code;
    String message;

    String      objectId;
    EObjectType objectType;
};


/* EVPath is defined in ./path-priv.h */


/* a response row */
typedef struct {
    SRaw raw;
    SOrdered ordered;
    STyped typed;
    EVPath path;
    VPathSet * set;
} SRow;


/* timestamp object */
typedef struct {
    SRaw raw;
    KTime_t time;
} STimestamp;


/* server timestamp */
typedef struct {
    STimestamp server;
    STimestamp local;
} SServerTimestamp;


/* server response = header; vector of response rows; generated objects */
typedef struct {
    EServiceType serviceType;
    SHeader header;
    Vector rows;
    KSrvResponse * list;
    Kart * kart;
    SServerTimestamp timestamp;
} SResponse;


/* key-value */
typedef struct {
    String k;
    String v;
} SKV;


/* helper object to add cgi parameters to KHttpRequest */
typedef struct {
    KHttpRequest * httpReq;
    rc_t rc;
} SHttpRequestHelper;


/* cgi request info */
typedef struct {
    bool inited;
    char * cgi;
    Vector params;
} SCgiRequest;


/* request object */
typedef struct {
    char * objectId;
    EObjectType objectType;
    int ordId;
} SObject;


/* service request data ( objects to query ) */
typedef struct {
    SObject * object;
    size_t allocated;
    uint32_t objects;
    bool refseq_ctx;
} SRequestData;


typedef struct {
    BSTNode n;
    char * ticket;
    uint32_t project;
} BSTItem;


/* tickets - access authorization keys */
typedef struct {
    BSTree ticketsToProjects;
    Vector tickets;
    KDataBuffer str;
    size_t size;
    rc_t rc;
} STickets;


/* service request */
typedef struct {
    EServiceType serviceType;
    SVersion version;
    SCgiRequest cgiReq;
    SRequestData request;
    STickets tickets;
    int errorsToIgnore;
    VRemoteProtocols protocols;
} SRequest;


/* service object */
struct KService {
    SHelper helper;
    SRequest req;
    SResponse resp;

    bool resoveOidName;
};


/******************************************************************************/


/* SHelper ********************************************************************/
static rc_t SHelperInit ( SHelper * self, const KNSManager * kMgr ) {
    rc_t rc = 0;
    assert ( self );
    memset ( self, 0, sizeof * self );
    if ( kMgr == NULL ) {
        KNSManager * kns = NULL;
        rc = KNSManagerMake ( & kns );
        kMgr = kns;
    }
    else {
        rc = KNSManagerAddRef ( kMgr );
    }
    if ( rc == 0) {
        self -> kMgr = kMgr;
    }
    self -> timeoutMs = 5000;
    return rc;
}


static rc_t SHelperFini ( SHelper * self) {
    rc_t rc = 0;

    assert ( self );

    RELEASE ( KConfig       , self -> kfg );
    RELEASE ( KNSManager    , self -> kMgr );
    RELEASE ( KRepositoryMgr, self -> repoMgr );

    memset ( self, 0, sizeof * self );

    return rc;
}


static rc_t SHelperInitKfg ( SHelper * self ) {
    rc_t rc = 0;

    assert ( self );

    if ( self -> kfg == NULL )
        rc = KConfigMake ( & self -> kfg, NULL );

    return rc;
}


static rc_t SHelperInitRepoMgr ( SHelper * self ) {
    rc_t rc = 0;

    assert ( self );

    if ( self -> repoMgr == NULL ) {
        rc = SHelperInitKfg ( self );
        if ( rc != 0 )
            return rc;

        rc = KConfigMakeRepositoryMgrRead ( self -> kfg, & self -> repoMgr );
    }

    return rc;
}


/* get from kfg, otherwise use hardcoded */
static VRemoteProtocols SHelperDefaultProtocols ( SHelper * self ) {
    VRemoteProtocols protocols = DEFAULT_PROTOCOLS;

    assert ( self );

    SHelperInitKfg ( self );

    KConfigReadRemoteProtocols ( self -> kfg, & protocols );

    return protocols;
}


/* try to get cgi from kfg, otherwise use hardcoded */
static
rc_t SHelperResolverCgi ( SHelper * self, bool aProtected,
    char * buffer, size_t bsize )
{
    const char man [] = "/repository/remote/main/CGI/resolver-cgi";
    const char prt [] = "/repository/remote/protected/CGI/resolver-cgi";
    const char cgi [] = "https://www.ncbi.nlm.nih.gov/Traces/names/names.fcgi";
    rc_t rc = 0;
    const char * path = aProtected ? prt : man;
    assert ( self );
    rc = SHelperInitKfg ( self );
    if ( rc == 0 ) {
        size_t num_read = 0;
        rc = KConfigRead ( self -> kfg, path, 0, buffer, bsize,
                           & num_read, NULL );
        if ( rc != 0 ) {
            if ( buffer == NULL )
                return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
            if ( bsize < sizeof cgi )
                return RC ( rcVFS, rcQuery, rcExecuting, rcBuffer,
                            rcInsufficient );
            string_copy ( buffer, bsize, cgi, sizeof cgi );
        }
    }
    return rc;
}


static
rc_t SHelperProjectToTicket ( SHelper * self, uint32_t projectId,
    char * buffer, size_t bsize, size_t * ticket_size )
{
    rc_t rc = 0;

    const KRepository * repo = NULL;

    assert ( self );

    rc = SHelperInitRepoMgr ( self );

    rc = KRepositoryMgrGetProtectedRepository ( self -> repoMgr, projectId,
        & repo );
    if ( rc != 0 )
        return rc;

    rc = KRepositoryDownloadTicket ( repo, buffer, bsize, ticket_size );

    RELEASE ( KRepository, repo );

    return rc;
}


/* SRaw ***********************************************************************/
static void SRawInit ( SRaw * self, char * s ) {
    assert ( self );

    self -> s = s;
}


static rc_t SRawAlloc ( SRaw * self, const char * s, size_t sz ) {
    char * p = NULL;

    if ( sz == 0 )
        p = string_dup_measure ( s, NULL );
    else
        p = string_dup ( s, sz );

    if ( p == NULL )
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );

    SRawInit ( self, p );

    return 0;
}


static rc_t SRawFini ( SRaw * self ) {
    if ( self != NULL ) {
        free ( self -> s );
        self -> s = NULL;
    }

    return 0;
}


/* SVersion *******************************************************************/
static rc_t SVersionInit
    ( SVersion * self, const char * src, EServiceType serviceType )
{
    const char * s = src;

    assert ( self );

    memset ( self, 0, sizeof * self );

    if ( s == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcMessage, rcBadVersion );
    }

    if ( * s != '#' ) {
        if ( serviceType != eSTnames || * s == '\0' || ! isdigit ( * s ) )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMessage, rcBadVersion );
    }
    else
        ++ s;

    if ( serviceType == eSTsearch ) {
        const char version [] = "version ";
        size_t sz = sizeof version - 1;
        if ( string_cmp ( s, sz, version, sz, ( uint32_t ) sz ) != 0 )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMessage, rcBadVersion );
        s += sz;
    }

    if ( * s == '\0' ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcMessage, rcBadVersion );
    }

    {
        char * end = NULL;
        uint64_t l = strtoul ( s, & end, 10 );
        if ( end == NULL || * end != '.' ) {
            return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        }
        self -> major = ( uint8_t ) l;
        s = ++ end;

        l = strtoul ( s, & end, 10 );
        if ( end == NULL || * end != '\0' ) {
            return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        }
        self -> minor = ( uint8_t ) l;

        self -> version = self -> major << 24 | self -> minor << 16;

        return SRawAlloc ( & self -> raw, src, 0 );
    }
}


static rc_t SVersionFini ( SVersion * self ) {
    rc_t rc = 0;
    assert ( self );
    rc = SRawFini ( & self -> raw );
    memset ( self, 0, sizeof * self );
    return rc;
}


static rc_t SVersionToString ( const SVersion * self, char ** s ) {
    size_t num_writ = 0;
    char tmp [ 1 ];
    assert ( self && s );
    string_printf ( tmp, 1, & num_writ, "%u.%u", self -> major, self -> minor );
    ++ num_writ;
    * s = ( char * ) malloc ( num_writ );
    if ( * s == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }
    return string_printf ( * s, num_writ,
        & num_writ, "%u.%u", self -> major, self -> minor );
}


/* SHeader ********************************************************************/
static rc_t SHeaderMake
    ( SHeader * self, const String * src, EServiceType serviceType )
{
    rc_t rc = 0;

    assert ( self && src );

    memset ( self, 0, sizeof * self );

    rc = SRawAlloc ( & self -> raw, src -> addr, src -> size );

    if ( rc == 0 )
        rc = SVersionInit ( & self -> version, self -> raw . s, serviceType );

    return rc;
}


static rc_t SHeaderFini ( SHeader * self ) {
    rc_t rc = 0;
    if ( self != NULL ) {
        rc_t r2 = SRawFini ( & self -> raw );
        rc = SVersionFini ( & self -> version );
        if ( rc == 0 ) {
            rc = r2;
        }
    }
    return rc;
}


/* SProtocol ******************************************************************/
typedef struct {
    const char * text;
    VRemoteProtocols protocol;
} SProtocol;


static VRemoteProtocols SProtocolGet ( const String * url ) {
    size_t i = 0;
    SProtocol protocols [] = {
        { "http:", eProtocolHttp },
        { "fasp:", eProtocolFasp },
        { "https:",eProtocolHttps},
        { "file:", eProtocolFile },
        { "s3:"  , eProtocolS3   },
        { "gs:"  , eProtocolGS   },
    };
    if ( url == NULL || url -> addr == NULL || url -> size == 0 ) {
        return eProtocolNone;
    }
    for ( i = 0; i < sizeof protocols / sizeof protocols [ 0 ]; ++ i ) {
        uint32_t sz = string_measure ( protocols [ i ] . text, NULL );
        if ( string_cmp ( url -> addr, sz, protocols [ i ] . text, sz,
            ( uint32_t ) sz ) == 0 )
        {
            return protocols [ i ] . protocol;
        }
    }
    return eProtocolNone;
}


/* STyped *********************************************************************/
static rc_t STypedInitUrls ( STyped * self ) {
    VRemoteProtocols protocol = eProtocolNone;
    String * str = NULL;
    String * dst = NULL;
    assert ( self );
    str = & self -> url;
    while ( str -> size > 0 ) {
        size_t len = 0;
        char * n = string_chr ( str -> addr, str -> size, '$' );
        if ( n != NULL ) {
            len = n - str -> addr;
        }
        else {
            len = str -> size;
        }
        protocol = SProtocolGet ( str );
        switch ( protocol ) {
            case eProtocolFasp:
                dst = & self -> fpUrl;
                break;
            case eProtocolFile:
                dst = & self -> flUrl;
                break;
            case eProtocolHttp:
                dst = & self -> hUrl;
                break;
            case eProtocolHttps:
                dst = & self -> hsUrl;
                break;
            case eProtocolS3:
                dst = & self -> s3Url;
                break;
            default:
                return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        }
        StringInit ( dst, str -> addr, len, len );
        if ( n != NULL )
            ++ len;
        str -> addr += len;
        if ( str -> size >= len )
            str -> size -= len;
        else
            str -> size = 0;
    }
    str = & self -> vdbcacheUrl;
    while ( str -> size > 0 ) {
        size_t len = 0;
        char * n = string_chr ( str -> addr, str -> size, '$' );
        if ( n != NULL ) {
            len = n - str -> addr;
        }
        else {
            len = str -> size;
        }
        protocol = SProtocolGet ( str );
        switch ( protocol ) {
            case eProtocolFasp:
                dst = & self -> fpVdbcacheUrl;
                break;
            case eProtocolFile:
                dst = & self -> flVdbcacheUrl;
                break;
            case eProtocolHttp:
                dst = & self -> hVdbcacheUrl;
                break;
            case eProtocolHttps:
                dst = & self -> hsVdbcacheUrl;
                break;
            case eProtocolS3:
                dst = & self -> s3VdbcacheUrl;
                break;
            default:
                return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        }
        StringInit ( dst, str -> addr, len, len );
        if ( n != NULL )
            ++ len;
        str -> addr += len;
        if ( str -> size >= len )
            str -> size -= len;
        else
            str -> size = 0;
    }
    return 0;
}


static
rc_t STypedInit ( STyped * self, const SOrdered * raw, const SConverters * how,
    const SVersion * version )
{
    rc_t rc = 0;
    int i = 0;
    assert ( self && raw && how && version );
    memset ( self, 0, sizeof * self );

    if ( raw -> n != how -> n )                              /* BREAK */
        return RC ( rcVFS, rcQuery, rcResolving, rcName, rcUnexpected );

    for ( i = 0; i < raw -> n; ++i ) {
        void * dest = how -> get ( self, i );
        if ( dest == NULL ) {
            rc = RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
            break;
        }
        {
            TConverter * f = how -> f [ i ];
            if ( f == NULL ) {
                rc = RC ( rcVFS, rcQuery, rcResolving, rcFunction, rcNotFound );
                break;
            }
            rc = f ( dest, & raw -> s [ i ] );
            if ( rc != 0 )
                break;             /* BREAK */
        }
    }

    if ( rc == 0 )
        if ( SVersionResponseHasMultipeUrls ( version ) )
            rc = STypedInitUrls ( self );

    if ( rc == 0 )
        self -> inited = true;

    return rc;
}


/* CONVERTERS */
/* functions to initialize objects from response row field Strings ************/
static
bool cmpStringAndObjectType ( const String * s, const char * val )
{
    size_t sz = string_size (val );
    String v;
    StringInit ( & v, val, sz, sz );
    return StringCompare ( s, & v ) == 0;
}


/* N.B. DO NOT FREE RETURNED STRING !!! */
static const char * ObjectTypeToString ( EObjectType self ) {
    switch ( self ) {
        case eOT_undefined   : return "";
        case eOT_dbgap       : return "dbgap";
        case eOT_provisional : return "provisional";
        case eOT_srapub      : return "srapub";
        case eOT_sragap      : return "sragap";
        case eOT_srapub_source:return "srapub_source";
        case eOT_sragap_source:return "sragap_source";
        case eOT_srapub_files: return "srapub_files";
        case eOT_sragap_files: return "sragap_files";
        case eOT_refseq      : return "refseq";
        case eOT_wgs         : return "wgs";
        case eOT_na          : return "na";
        case eOT_nakmer      : return "nakmer";
        default: assert ( 0 ); return "";
    }
}


static EObjectType StringToObjectType ( const String * s ) {
    if ( cmpStringAndObjectType ( s, "" ) ) {
        return eOT_empty;
    }
    if ( cmpStringAndObjectType ( s, "dbgap" ) ) {
        return eOT_dbgap;
    }
    if ( cmpStringAndObjectType ( s, "provisional" ) ) {
        return eOT_provisional;
    }
    if ( cmpStringAndObjectType ( s, "srapub" ) ) {
        return eOT_srapub;
    }
    if ( cmpStringAndObjectType ( s, "sragap" ) ) {
        return eOT_sragap;
    }
    if ( cmpStringAndObjectType ( s, "srapub_source" ) ) {
        return eOT_srapub_source;
    }
    if ( cmpStringAndObjectType ( s, "srapub_files" ) ) {
        return eOT_srapub_files;
    }
    if ( cmpStringAndObjectType ( s, "sragap_files") ) {
        return eOT_sragap_files;
    }
    if ( cmpStringAndObjectType ( s, "refseq") ) {
        return eOT_refseq;
    }
    if ( cmpStringAndObjectType ( s, "wgs") ) {
        return eOT_wgs;
    }
    if ( cmpStringAndObjectType ( s, "na") ) {
        return eOT_na;
    }
    if ( cmpStringAndObjectType ( s, "nakmer") ) {
        return eOT_nakmer;
    }
    return eOT_undefined;
}


static rc_t EObjectTypeInit ( void * p, const String * src ) {
    EObjectType * self = ( EObjectType * ) p;
    EObjectType t = StringToObjectType ( src );
    if ( t == eOT_undefined ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcType, rcIncorrect );
    }
    assert ( self );
    * self = t;
    return 0;
}


static rc_t aStringInit ( void * p, const String * src ) {
    String * self = ( String * ) p;
    assert ( src );
    StringInit ( self, src -> addr, src -> size, src -> len );
    return 0;
}


static rc_t size_tInit ( void * p, const String * src ) {
    rc_t rc = 0;
    size_t * self = ( size_t * ) p;
    size_t s = 0;
    if ( src -> size != 0  && src -> len != 0 ) {
        s = ( size_t ) StringToU64 ( src, & rc );
    }
    if ( rc == 0 ) {
        assert ( self );
        * self = s;
    }
    return rc;
}


static rc_t uint32_tInit ( void * p, const String * src ) {
    rc_t rc = 0;
    uint32_t * self = ( uint32_t * ) p;
    uint32_t s = ( uint32_t ) StringToU64 ( src, & rc );
    if ( rc == 0 ) {
        assert ( self );
        * self = s;
    }
    return rc;
}


static rc_t uint64_tInit ( void * p, const String * src ) {
    rc_t rc = 0;
    uint64_t * self = ( uint64_t * ) p;
    uint64_t s = 0;
    assert ( src && self );
    if ( src -> size == 0 )
        * self = 0;
    else {
        s = StringToU64 ( src, & rc );
        if ( rc == 0 )
            * self = s;
    }
    return rc;
}


#if 0 && LINUX
#define TODO 1;
static
rc_t YYYY_MM_DDThh_mm_ssZToTm ( const char * src, struct tm * result )
{
/*YYYY-MM-DDThh:mm:ssTZD ISO 8601
tm_sec	int	seconds after the minute	0-61*
tm_min	int	minutes after the hour	0-59
tm_hour	int	hours since midnight	0-23
tm_mday	int	day of the month	1-31
tm_mon	int	months since January	0-11
tm_year	int	years since 1900	
tm_wday	int	days since Sunday	0-6
tm_yday	int	days since January 1	0-365
tm_isdst	int	Daylight Saving Time flag	*/
    int i = 0;
    int tmp = 0;
    char c = 0;
    assert ( src && result );
    memset ( result, 0, sizeof * result );
    for ( i = 0, tmp = 0; i < 4; ++ i ) {
        char c = src [ i ];
        if ( ! isdigit ( c ) )
            return TODO;
        tmp = tmp * 10 + c - '0';
    }
    if ( tmp < 1900 )
        return TODO;
    result -> tm_year = tmp - 1900;
    if ( src [ i ] != '-' )
        return TODO;
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = c - '0';
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 12 )
        return TODO;
    result -> tm_mon = tmp - 1;
    c = src [ ++ i ];
    if ( c != '-' )
        return TODO;
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = c - '0';
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 31 )
        return TODO;
    result -> tm_mday = tmp;
    c = src [ ++ i ];
    if ( c != 'T' )
        return TODO;
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = c - '0';
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 23 )
        return TODO;
    result -> tm_hour = tmp;
    c = src [ ++ i ];
    if ( c != ':' )
        return TODO;
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = c - '0';
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return TODO;
    result -> tm_min = tmp;
    c = src [ ++ i ];
    if ( c != ':' )
        return TODO;
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = c - '0';
    c = src [ ++ i ];
    if ( ! isdigit ( c ) )
        return TODO;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return TODO;
    result -> tm_sec = tmp;
    c = src [ ++ i ];
    if ( c != 'Z' )
        return TODO;
    /*time_t time = 0;
    struct tm * t = gmtime_r ( & time, result );*/
    return 0;
}
#endif


static rc_t KTimeInit ( void * p, const String * src ) {
    rc_t rc = 0;

    KTime_t * self = ( KTime_t * ) p;

    assert ( self && src );

    if ( src -> addr != NULL && src -> size > 0 )
        * self = StringToU64 ( src, & rc );

    return rc;
}


static rc_t KTimeInitFromIso8601 ( void * p, const String * src ) {
    rc_t rc = 0;

    KTime_t * self = ( KTime_t * ) p;

    assert ( self && src );

    if ( src -> addr != NULL && src -> size > 0 ) {
        KTime kt;
        const KTime * t = KTimeFromIso8601 ( & kt, src -> addr, src -> size );
        if ( t == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
        else
            * self = KTimeMakeTime ( & kt );
    }

    return rc;
}


static int getDigit ( char c, rc_t * rc ) {
    assert ( rc );

    if ( * rc != 0 )
        return 0;

    c = tolower ( c );
    if ( ! isdigit ( c ) && c < 'a' && c > 'f' ) {
        * rc = RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
        return 0;
    }

    if ( isdigit ( c ) )
        return c - '0';

    return c - 'a' + 10;
}


static rc_t md5Init ( void * p, const String * src ) {
    SMd5 * md5 = p;

    assert ( src && src -> addr && md5 );

    md5 -> has_md5 = false;

    switch ( src -> size ) {
        case 0:
            return 0;
        case 32: {
            rc_t rc = 0;
            int i = 0;
            for ( i = 0; i < 16 && rc == 0; ++ i ) {
                md5 -> md5 [ i ]  = getDigit ( src -> addr [ 2 * i     ], & rc )
                                    * 16;
                md5 -> md5 [ i ] += getDigit ( src -> addr [ 2 * i + 1 ],
                                                                         & rc );
            }
	        md5 -> has_md5 = rc == 0;
            return rc;
        }
        default:
            return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
    }
}


/* SConverters ****************************************************************/
/* converter from names-1.0 response row to STyped object  */
static void * STypedGetFieldNames1_0 ( STyped * self, int n ) {
    assert ( self );
    switch ( n ) {
        case  0: return & self -> accession;
        case  1: return & self -> ticket;
        case  2: return & self -> hUrl;
        case  3: return & self -> code;
        case  4: return & self -> message;
    }
    return 0;
}


static const SConverters * SConvertersNames1_0Make ( void ) {
    static TConverter * f [ N_NAMES1_0 + 1 ] = {
        aStringInit,
        aStringInit,
        aStringInit,
        uint32_tInit,
        aStringInit, NULL };
    static const SConverters c = {
        N_NAMES1_0,
        STypedGetFieldNames1_0, f };
    return & c;
}


/* converter from names-1.1 response row to STyped object  */
static void * STypedGetFieldNames1_1 ( STyped * self, int n ) {
    assert ( self);
    switch ( n ) {
        case  0: return & self -> accession;
        case  1: return & self -> objectId;
        case  2: return & self -> name;
        case  3: return & self -> osize;
        case  4: return & self -> date;
        case  5: return & self -> md5;
        case  6: return & self -> ticket;
        case  7: return & self -> hUrl;
        case  8: return & self -> code;
        case  9: return & self -> message;
    }
    return 0;
}


static const SConverters * SConvertersNames1_1Make ( void ) {
    static TConverter * f [ N_NAMES1_1 + 1 ] = {
        aStringInit,
        aStringInit,
        aStringInit,
        uint64_tInit,
        KTimeInitFromIso8601,
        md5Init,
        aStringInit,
        aStringInit,
        uint32_tInit,
        aStringInit, NULL };
    static const SConverters c = {
        N_NAMES1_1,
        STypedGetFieldNames1_1, f };
    return & c;
}


static const SConverters * SConvertersNames1_2Make ( void ) {
    static TConverter * f [ N_NAMES1_1 + 1 ] = {
        aStringInit,
        aStringInit,
        aStringInit,
        uint64_tInit,
        KTimeInitFromIso8601,
        md5Init,
        aStringInit,
        aStringInit,
        uint32_tInit,
        aStringInit, NULL };
    static const SConverters c = {
        N_NAMES1_1,
        STypedGetFieldNames1_1, f };
    return & c;
}


/* converter from names-3.0 response row to STyped object  */
static void * STypedGetFieldNames3_0 ( STyped * self, int n ) {
    assert ( self);
    switch ( n ) {
        case  0: return & self -> ordId;
        case  1: return & self -> objectType;
        case  2: return & self -> objectId;
        case  3: return & self -> osize;
        case  4: return & self -> date;
        case  5: return & self -> md5;
        case  6: return & self -> ticket;
        case  7: return & self -> url;
        case  8: return & self -> vdbcacheSize;
        case  9: return & self -> vdbcacheDate;
        case 10: return & self -> vdbcacheMd5;
        case 11: return & self -> vdbcacheUrl;
        case 12: return & self -> expiration;
        case 13: return & self -> code;
        case 14: return & self -> message;
    }
    return 0;
}


static const SConverters * SConvertersNames3_0Make ( void ) {
    static TConverter * f [ N_NAMES3_0 + 1 ] = {
        uint32_tInit,        /*  0 ord-id */
        EObjectTypeInit,     /*  1 object-type */
        aStringInit,         /*  2 object-id */
        uint64_tInit,        /*  3 osize */
        KTimeInitFromIso8601,/*  4 date */
        md5Init,             /*  5 md5 */
        aStringInit,         /*  6 ticket */
        aStringInit,         /*  7 url */
        size_tInit,          /*  8 vdbcache-size */
        KTimeInitFromIso8601,/*  9 vdbcache-date */
        md5Init,             /* 10 vdbcache-md5 */
        aStringInit,         /* 11 vdbcache-url */
        KTimeInit,           /* 12 expiration */
        uint32_tInit,        /* 13 status-code */
        aStringInit,         /* 14 message */
        NULL };
    static const SConverters c = {
        N_NAMES3_0,
        STypedGetFieldNames3_0, f };
    return & c;
}


/* converter factory function */
static
rc_t SConvertersMake ( const SConverters ** self, SHeader * header )
{
    assert ( self && header );
    switch ( header -> version. version ) {
        case VERSION_1_0:
            * self = SConvertersNames1_0Make ();
            return 0;
        case VERSION_1_1:
            * self = SConvertersNames1_1Make ();
            return 0;
        case VERSION_1_2:
            * self = SConvertersNames1_2Make ();
            return 0;
        case VERSION_3_0:
            * self = SConvertersNames3_0Make ();
            return 0;
        default:
            * self = NULL;
            return RC ( rcVFS, rcQuery, rcExecuting, rcMessage, rcBadVersion );
    }
}


/* SOrdered *******************************************************************/
static
rc_t SOrderedInit ( SOrdered * self, const SRaw * raw, int fields )
{
    assert ( self && raw );
    memset ( self, 0, sizeof * self );
    {
        const char * str = raw -> s;
        size_t size = string_size ( str );
        while ( size > 0 ) {
            size_t len = 0;
            char * n = string_chr ( str, size, '|' );
            if ( n != NULL )
                len = n - str;
            else
                len = size;
            if ( self -> n >= fields ) {
                return RC ( rcVFS, rcQuery, rcResolving, rcName, rcExcessive );
            }
            else {
                String * s = & self -> s [ self -> n ++ ];
                StringInit ( s, str, len, len );
                if ( n != NULL )
                    ++ len;
                str += len;
                if ( size >= len )
                    size -= len;
                else
                    size = 0;
            }
        }
    }
    return 0;
}


/* KSrvError ******************************************************************/
static
rc_t KSrvErrorMake ( const KSrvError ** self,
    const STyped * src, rc_t aRc )
{
    KSrvError * o = NULL;
    assert ( self && aRc );
    o = ( KSrvError * ) calloc ( 1, sizeof * o );
    if ( o == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    if ( src != NULL ) {
        o -> message . addr = string_dup ( src -> message . addr,
                                           src -> message . size );
        if ( o -> message . addr == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        o -> message . size = src -> message . size;
        o -> message . len  = src -> message . len;

        o -> objectId . addr = string_dup ( src -> objectId . addr,
                                        src -> objectId . size );
        if ( o -> objectId . addr == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        o -> objectId . size = src -> objectId . size;
        o -> objectId . len  = src -> objectId . len;

        o -> objectType = src -> objectType;

        o -> code = src -> code;
    }

    o -> rc = aRc;

    atomic32_set ( & o -> refcount, 1 );

    * self = o;

    return 0;
}


rc_t KSrvErrorAddRef ( const KSrvError * cself ) {
    KSrvError * self = ( KSrvError * ) cself;

    if ( self != NULL )
        atomic32_inc ( & ( ( KSrvError * ) self ) -> refcount );

    return 0;
}


rc_t KSrvErrorRelease ( const KSrvError * cself ) {
    rc_t rc = 0;

    KSrvError * self = ( KSrvError * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) ) {
        free ( ( void * ) self -> message  . addr );
        free ( ( void * ) self -> objectId . addr );
        memset ( self, 0, sizeof * self );
        free ( ( void * ) self );
    }

    return rc;
}


/* Rc - rc code corresponding to this Error */
rc_t KSrvErrorRc      ( const KSrvError * self, rc_t     * rc   ) {
    rc_t dummy = 0;
    if ( rc == NULL )
        rc = & dummy;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    * rc = self -> rc;
    
    return 0;
}


/* Code - Status-Code returned by server */
rc_t KSrvErrorCode    ( const KSrvError * self, uint32_t * code ) {
    uint32_t dummy = 0;
    if ( code == NULL )
        code = & dummy;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    * code = self -> code;

    return 0;
}


/*  returns pointers to internal String data
 *  Strings remain valid while "self" is valid */
/* Message - message returned by server */
rc_t KSrvErrorMessage ( const KSrvError * self, String * message ) {
    String dummy;
    if ( message == NULL )
        message = & dummy;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    * message = self -> message;

    return 0;
}


/* Object - Object-Id/Object-Type that produced this Error */
rc_t KSrvErrorObject ( const KSrvError * self,
    String * id, EObjectType * type )
{
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( id != NULL )
        * id = self -> objectId;

    if ( type != NULL )
        * type = self -> objectType;

    return 0;
}


/* EVPath *********************************************************************/
static bool VPathMakeOrNot ( VPath ** new_path, const String * src,
    const String * ticket, const STyped * typed, bool ext, rc_t * rc,
    bool useDates )
{
    String bug;
    memset ( & bug, 0, sizeof bug );

    assert ( new_path && src && typed && rc );

    if ( * rc != 0 || src -> len == 0 )
        return false;
    else {
        const String * id = & typed -> objectId;
        if ( id -> size == 0 )
            id = & typed -> accession;

        if ( id -> size == 0 ) {
/* compensate current names.cgi-v3.0 bug: it does not return id for object-id-s
 */
            if ( src -> size > 0 &&
                 isdigit ( src -> addr [ src -> size - 1 ] ) )
            {
                size_t s = 2;
                bug . addr = src -> addr + src -> size - 1;
                bug . size = 1;
                for ( s = 2; s <= src -> size
                             && isdigit ( src -> addr [ src -> size - s ] );
                    ++ s )
                {
                    -- bug . addr;
                    ++ bug . size;
                }
                bug . len = bug . size;
                id = & bug;
            }
        }

        if ( src -> size == 0 )
            assert ( src -> addr != NULL );

        * rc = VPathMakeFromUrl ( new_path, src, ticket, ext, id,
            typed -> osize,
            useDates ? typed -> date : 0,
			typed -> md5 . has_md5 ? typed -> md5 . md5 : NULL,
            useDates ? typed -> expiration : 0 );
        if ( * rc == 0 )
            VPathMarkHighReliability ( * new_path, true );

        return true;
    }
}


static rc_t EVPathInitMapping
    ( EVPath * self, const STyped * src, const SVersion * version )
{
    rc_t rc = 0;
    const VPath * vsrc = NULL;
    assert ( self && src && version );
    if ( self -> https == NULL && self -> http == NULL && self -> fasp == NULL )
        return 0;
    vsrc = self -> http ? self -> http 
	    : ( self -> https ? self -> https : self -> fasp );
    rc = VPathCheckFromNamesCGI ( vsrc, & src -> ticket,
        ( const struct VPath ** ) ( & self -> mapping ) );
    if ( rc == 0) {
        if ( SVersionBefore3_0 ( version ) ) {
            if ( src -> ticket . size != 0 ) {
                if ( src -> accession . size != 0 )
                    rc = VPathMakeFmt ( & self -> mapping, "ncbi-acc:%S?tic=%S",
                        & src -> accession, & src -> ticket );
                else if ( src -> name . size == 0 )
                    return 0;
                else
                    rc = VPathMakeFmt ( & self -> mapping,
                        "ncbi-file:%S?tic=%S", & src -> name, & src -> ticket );
            }
            else if ( src -> accession . size != 0 )
                rc = VPathMakeFmt
                    ( & self -> mapping, "ncbi-acc:%S", & src -> accession );
            else if ( src -> name . size == 0 )
                return 0;
            else
                rc = VPathMakeFmt
                    ( & self -> mapping, "ncbi-file:%S", & src -> name );
        }
        else {
            if ( src -> ticket . size != 0 ) {
                if ( src -> objectId . size != 0 &&
                     src -> objectType == eOT_sragap )
                {
                    rc = VPathMakeFmt ( & self -> mapping, "ncbi-acc:%S?tic=%S",
                        & src -> objectId, & src -> ticket );
                }
                else {
                    if ( src -> objectId . size == 0) {
                        return 0;
                    }
                    else {
                        rc = VPathMakeFmt ( & self -> mapping,
                            "ncbi-file:%S?tic=%S",
                            & src -> objectId, & src -> ticket );
                    }
                }
            }
            else
            if ( src -> objectId . size != 0 &&
                 src -> objectType == eOT_sragap )
            {
                rc = VPathMakeFmt
                    ( & self -> mapping, "ncbi-acc:%S", & src -> objectId );
            }
            else {
                if ( src -> objectId . size == 0 )
                    return 0;
                else
                    rc = VPathMakeFmt (
                        & self -> mapping, "ncbi-file:%S", & src -> objectId );
            }
        }

        if ( rc == 0 )
            return 0;

        RELEASE ( VPath, self -> http );
        RELEASE ( VPath, self -> fasp );
    }

    return rc;
}


static rc_t EVPathInit ( EVPath * self, const STyped * src, 
    const SRequest * req, rc_t * r, const char * acc )
{
    rc_t rc = 0;
    bool made = false;
    bool logError = true;
    KLogLevel lvl = klogInt;
    assert ( self && src && r );

    switch ( src -> code / 100 ) {
      case 0:
        rc = RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        break;

      case 1:
        /* informational response
           not much we can do here */
        rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
        break;

      case 2:
        /* successful response
           but can only handle 200 */
        if ( src -> code == 200 ) {
            bool ext = true;
            assert ( req );
            if ( req -> serviceType == eSTnames &&
                 SVersionNotExtendedVPaths ( & req -> version ) )
            {
                ext = false;
            }

            made |= VPathMakeOrNot ( & self -> http,
                & src -> hUrl , & src -> ticket, src, ext, & rc, true );
            made |= VPathMakeOrNot ( & self -> fasp,
                & src -> fpUrl, & src -> ticket, src, ext, & rc, true );
            made |= VPathMakeOrNot ( & self -> https,
                & src -> hsUrl, & src -> ticket, src, ext, & rc, true );
            made |= VPathMakeOrNot ( & self -> file,
                & src -> flUrl, & src -> ticket, src, ext, & rc, true );
            made |= VPathMakeOrNot ( & self -> s3,
                & src -> s3Url, & src -> ticket, src, ext, & rc, true );
            VPathMakeOrNot ( & self -> vcHttp,
                & src -> hVdbcacheUrl, & src -> ticket, src, ext, & rc, false );
            VPathMakeOrNot ( & self -> vcFasp,
                & src -> fpVdbcacheUrl,& src -> ticket, src, ext, & rc, false );
            VPathMakeOrNot ( & self -> vcHttps,
                & src -> hsVdbcacheUrl,& src -> ticket, src, ext, & rc, false );
            VPathMakeOrNot ( & self -> vcFile,
                & src -> flVdbcacheUrl,& src -> ticket, src, ext, & rc, false );
            VPathMakeOrNot ( & self -> vcS3,
                & src -> s3VdbcacheUrl,& src -> ticket, src, ext, & rc, false );

            if ( rc == 0 )
                rc = EVPathInitMapping ( self, src, & req -> version );
            return rc;
        }
        rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
        break;

	  case 3:
        /* redirection
           currently this is being handled by our request object */
        rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
        break;
      case 4:
        /* client error */
        lvl = klogErr;
        switch ( src -> code )
        {
        case 400:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcInvalid );
            break;
        case 401:
        case 403:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcQuery, rcUnauthorized );
            break;
        case 404: /* 404|no data :
          If it is a real response then this assession is not found.
          What if it is a DB failure? Will be retried if configured to do so? */
            rc = RC ( rcVFS, rcQuery, rcResolving, rcName, rcNotFound );
            logError = false;
            break;
        case 410:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcName, rcNotFound );
            break;
        default:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
        }
        break;
      case 5:
        /* server error */
        lvl = klogSys;
        switch ( src -> code )
        {
        case 503:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcDatabase, rcNotAvailable );
            break;
        case 504:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcTimeout, rcExhausted );
            break;
        default:
            rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
        }
        break;
      default:
        rc = RC ( rcVFS, rcQuery, rcResolving, rcError, rcUnexpected );
    }

    /* log message to user */
    if ( req -> errorsToIgnore == 0 && logError ) {
        if ( src -> objectId . size > 0 )
            PLOGERR ( lvl, ( lvl, rc,
                "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
                "acc=%S,msg=%S,code=%u",
                & src -> objectId, & src -> message, src -> code ) );
        else
            PLOGERR ( lvl, ( lvl, rc,
                "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
                "acc=%s,msg=%S,code=%u", acc, & src -> message, src -> code ) );
    }
    else
        -- ( ( SRequest * ) req ) -> errorsToIgnore;

    return KSrvErrorMake ( & self -> error, src, rc );
}


static rc_t EVPathFini ( EVPath * self ) {
    rc_t rc = 0;

    assert ( self );

    RELEASE ( VPath, self -> mapping );
    RELEASE ( VPath, self ->   http  );
    RELEASE ( VPath, self ->   fasp  );
    RELEASE ( VPath, self ->   https );
    RELEASE ( VPath, self ->   file  );
    RELEASE ( VPath, self ->   s3    );
    RELEASE ( VPath, self -> vcHttp  );
    RELEASE ( VPath, self -> vcFasp  );
    RELEASE ( VPath, self -> vcHttps );
    RELEASE ( VPath, self -> vcFile  );
    RELEASE ( VPath, self -> vcS3    );

    RELEASE ( KSrvError, self -> error );

    return rc;
}


/* SRow ***********************************************************************/
static rc_t SRowWhack ( void * p ) {
    rc_t rc = 0;
    rc_t r2 = 0;
    if ( p != NULL ) {
        SRow * self = ( SRow * ) p;
        r2 = EVPathFini ( & self -> path );
        if ( rc == 0) {
            rc = r2;
        }
        r2 = SRawFini ( & self -> raw );
        if ( rc == 0) {
            rc = r2;
        }
        memset ( self, 0, sizeof * self );
        free ( self );
    }
    return rc;
}


static rc_t SRowMake ( SRow ** self, const String * src, const SRequest * req,
    const SConverters * f, const SVersion * version )
{
    rc_t rc = 0;
    rc_t r2 = 0;

    SRow * p = ( SRow * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    assert ( req && version && src );

    rc = SRawAlloc ( & p -> raw, src -> addr, src -> size );
    if ( rc == 0 ) {
        assert ( f );
        rc = SOrderedInit ( & p -> ordered, & p -> raw, f -> n );
    }
    if ( rc == 0 ) {
        rc = STypedInit ( & p -> typed, & p -> ordered, f, version );
    }
    if ( rc == 0 &&
         p -> typed . code == 200 && req -> request . objects == 1 )
    {
        String acc;
        size_t l
            = string_measure ( req -> request . object [ 0 ] . objectId, NULL );
        StringInit ( & acc, req -> request . object [ 0 ] . objectId, l, l );
        assert ( acc . size == 0 || acc . addr != NULL );
        if ( acc . size > 2 && acc . addr [1] == 'R' && acc . addr [2] == 'R' &&
             ! StringEqual ( & p -> typed . accession, & acc ) &&
             ! StringEqual ( & p -> typed . objectId , & acc ) )
        {
            return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
        }
    }
    if ( rc == 0 ) {
        const char * acc = NULL;
        if ( req -> request . objects == 1 ) {
            acc = req -> request . object [ 0 ] . objectId;
        }
        rc = EVPathInit ( & p -> path, & p -> typed, req, & r2, acc );
        if ( rc == 0 ) {
            rc = r2;
        }
    }

/* compare ticket
       currently this makes sense with 1 request from a known workspace *
    if ( download_ticket . size != 0 )
    {
        if ( ticket == NULL || ! StringEqual ( & download_ticket, ticket ) )
            return RC ( rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt );
    }
*/

    if ( rc == 0 )
        rc = VPathSetMake ( & p -> set, & p -> path,
                            SVersionSingleUrl ( version ) );
    if ( rc == 0 ) {
        assert ( self );
        * self = p;
    }
    else
        SRowWhack ( p );

    return rc;
}


static void whackSRow ( void * self, void * ignore ) {
    SRowWhack ( self);
}


/* STimestamp ****************************************************************/
static rc_t STimestampInit ( STimestamp * self, const String * src ) {
    rc_t rc = 0;

    assert ( self && src );

    rc = SRawAlloc ( & self -> raw, src -> addr, src -> size );

    if ( rc == 0 )
        rc = KTimeInit ( & self -> time, src );

    return rc;
}


static rc_t STimestampInitCurrent ( STimestamp * self ) {
    assert ( self );

    self -> time = KTimeStamp ();

    if ( self -> time != 0 ) {
        const size_t s = 32;
        self -> raw . s = calloc ( 1, s );
        if ( self -> raw . s == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        else {
            size_t sz = KTimeIso8601 ( self -> time, self -> raw . s, s );
            if ( sz == 0 )
                return RC ( rcVFS, rcQuery, rcExecuting, rcMemory,
                    rcInsufficient );
            else
                return 0;
        }
    }
    else
        return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
}


static rc_t STimestampFini ( STimestamp * self ) {
    rc_t rc = 0;

    assert ( self );

    rc = SRawFini ( & self -> raw );

    memset ( self, 0, sizeof * self );

    return rc;
}


/* SServerTimestamp************************************************************/
static
rc_t SServerTimestampInit ( SServerTimestamp * self,
                            const String * src )
{
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self );

    rc = STimestampInit ( & self -> server, src );

    r2 = STimestampInitCurrent ( & self -> local );
    if ( rc == 0 )
        rc = r2;

    return rc;
}


static rc_t SServerTimestampFini ( SServerTimestamp * self ) {
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self );
    rc = STimestampFini ( & self ->server );

    r2 = STimestampFini ( & self ->local );
    if ( rc == 0 )
        rc = r2;

    return rc;
}


/* SResponse ******************************************************************/
static rc_t SResponseInit ( SResponse * self ) {
    rc_t rc = 0;
    assert ( self );
    VectorInit ( & self -> rows, 0, 5 );
    rc = KSrvResponseMake ( & self -> list );
    return rc;
}


static void whackKartItem ( void * self, void * ignore ) {
    KartItemRelease ( ( KartItem * ) self);
}


static rc_t SResponseFini ( SResponse * self ) {
    rc_t rc = 0;
    rc_t r2 = 0;
    
    assert ( self );

    {
        void ( CC * whack ) ( void *item, void *data ) = NULL;
        if ( self -> serviceType == eSTsearch )
            whack = whackKartItem;
        else
            whack = whackSRow;
        assert ( whack );
        VectorWhack ( & self -> rows, whack, NULL );
    }

    rc = SHeaderFini ( & self -> header );

    r2 = KSrvResponseRelease ( self -> list );
    if ( rc == 0 )
        rc = r2;

    r2 = KartRelease ( self -> kart );
    if ( rc == 0 )
        rc = r2;

    r2 = SServerTimestampFini ( & self -> timestamp );
    if ( rc == 0 )
        rc = r2;

    memset ( self, 0, sizeof * self );

    return rc;
}


static rc_t SResponseGetResponse
    ( const SResponse * self, const KSrvResponse ** response )
{
    rc_t rc = 0;
    assert ( self );
    rc = KSrvResponseAddRef ( self -> list );
    if ( rc == 0 ) {
        * response = self -> list;
    }
    return rc;
}


/* SKV ************************************************************************/
static
rc_t SKVMake ( const SKV ** self, const char * k, const char * v )
{
    assert ( self );
    * self = NULL;
    if ( k == NULL || * k == '\0' ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    }
    else {
        rc_t rc = 0;
        size_t num_writ = 0;
        size_t sk = string_size ( k );
        size_t sv = string_size ( v );
        size_t s  = sk + sv + 2;
        char * p = ( char * ) malloc ( s );
        if ( p == NULL ) {
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        }
        rc = string_printf ( p, s, & num_writ, "%s=%s", k, v );
        assert ( num_writ <= s );
        if ( rc != 0 ) {
            free ( p );
            p = NULL;
        }
        else {
            SKV * kv = ( SKV * ) malloc ( sizeof * kv );
            assert ( sk );
            if ( kv == NULL ) {
                free ( p );
                p = NULL;
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
            }
            else {
                StringInit ( & kv -> k, p, sk, sk );
                StringInit ( & kv -> v, p + sk + 1, sv, sv );
                * self = kv;
            }
        }
        return rc;
    }
}


static
rc_t SKVMakeObj ( const SKV ** self, const SObject * obj,
    const SVersion * version )
{
    rc_t rc = 0;
    size_t sk = 0;
    size_t num_writ = 0;
    char tmp [] = "";
    bool old = SVersionAccInRequest ( version );
    char * p = NULL;
    const char * k = "object";
    if ( old )
        k = "acc";

    sk = string_size ( k );

    assert ( self && obj );
    * self = NULL;

    if ( old )
        rc = string_printf ( tmp, 1, & num_writ, "%s=%s", k,
            obj -> objectId );
    else
        string_printf ( tmp, 1, & num_writ, "%s=%d|%s|%s", k, obj -> ordId,
            ObjectTypeToString ( obj -> objectType ), obj -> objectId );

    ++ num_writ;
    p = ( char * ) malloc ( num_writ );
    if ( p == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    if ( old )
        rc = string_printf ( p, num_writ, & num_writ, "%s=%s", k,
            obj -> objectId );
    else
        rc = string_printf ( p, num_writ, & num_writ, "%s=%d|%s|%s", k,
            obj -> ordId, ObjectTypeToString ( obj -> objectType ),
            obj -> objectId );

    if ( rc != 0 ) {
        free ( p );
        p = NULL;
    }
    else {
        SKV * kv = ( SKV * ) malloc ( sizeof * kv );
        assert ( sk );
        if ( kv == NULL ) {
            free ( p );
            p = NULL;
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        }
        else {
            -- num_writ;
            StringInit ( & kv -> k, p, sk, sk );
            StringInit ( & kv -> v, p + sk + 1, num_writ, num_writ );
            * self = kv;
        }
    }

    return rc;
}


/* SHttpRequestHelper ********************************************************/
static rc_t SHttpRequestHelperInit ( SHttpRequestHelper * self,
    const KNSManager * kMgr, const char * cgi )
{
    rc_t rc = 0;

    assert ( self );

    memset ( self, 0, sizeof * self );

    rc = KNSManagerMakeReliableClientRequest ( kMgr, & self -> httpReq,
        0x01010000, NULL, cgi );

    return rc;
}


static rc_t SHttpRequestHelperFini ( SHttpRequestHelper * self ) {
    rc_t rc = 0;

    assert ( self );

    RELEASE ( KHttpRequest, self -> httpReq );

    return rc;
}


static
void SHttpRequestHelperAddPostParam ( void * item, void * data )
{
    const SKV          * kv = ( SKV                * ) item;
    SHttpRequestHelper * p  = ( SHttpRequestHelper * ) data;

    rc_t rc = 0;

    assert ( kv && p );

    rc = KHttpRequestAddPostParam ( p -> httpReq, kv -> k . addr );
    if ( p -> rc == 0 )
        p -> rc = rc;
}


/* SCgiRequest ****************************************************************/
static
rc_t SCgiRequestInitCgi ( SCgiRequest * self, const char * cgi )
{
    assert ( self && ! self -> cgi );

    self -> cgi = string_dup_measure ( cgi, NULL );
    if ( self -> cgi == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    return 0;
}


static void whackSKV ( void * p, void * ignore ) {
    SKV * self = ( SKV * ) p;
    assert ( self );
    free ( ( void * ) self -> k . addr );
    memset ( self, 0, sizeof * self );
    free ( self );
}


static void SCgiRequestFini ( SCgiRequest * self ) {
    assert ( self );
    free ( self -> cgi );
    VectorWhack ( & self -> params, whackSKV, NULL );
    memset ( self, 0, sizeof * self );
}


static rc_t SCgiRequestPerform ( const SCgiRequest * self,
    const SHelper * helper, KStream ** response,
    const char * expected )
{
    rc_t rc = 0;

    assert ( self && helper && response );

    if ( rc == 0 ) {
        SHttpRequestHelper h;
        rc = SHttpRequestHelperInit ( & h, helper -> kMgr, self-> cgi );
        if ( rc == 0 ) {
            VectorForEach (
                & self -> params, false, SHttpRequestHelperAddPostParam, & h );
            rc = h . rc;
        }

        if ( rc == 0 ) {
            if ( expected == NULL ) {
                KHttpResult * rslt = NULL;
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), (
            ">>>>>>>>>>>>>>>> sending HTTP POST request >>>>>>>>>>>>>>>>\n" ) );
                rc = KHttpRequestPOST ( h . httpReq, & rslt );
                if ( rc == 0 ) {
                    uint32_t code = 0;
                    rc = KHttpResultStatus ( rslt, & code, NULL, 0, NULL );
                    if ( rc == 0 ) {
                        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), (
                            "  code=%d\n", code ) );
                        switch ( code ) {
                          case 200:
                            rc = KHttpResultGetInputStream ( rslt, response );
                            break;
                          case 403:
                     /* HTTP/1.1 403 Forbidden
                      - resolver CGI was called over http insted of https */
                            rc = RC ( rcVFS, rcQuery, rcExecuting, rcConnection,
                                      rcUnauthorized );
                            break;
                          case 404:
                    /* HTTP/1.1 404 Bad Request - resolver CGI was not found */
                            rc = RC ( rcVFS, rcQuery, rcExecuting, rcConnection,
                                      rcNotFound );
                            break;
                          default: /* Something completely unexpected */
                            rc = RC ( rcVFS, rcQuery, rcExecuting, rcConnection,
                                      rcUnexpected );
                            break;
                        }
                    }
                }
                RELEASE ( KHttpResult, rslt );
            }
            else {
                KStream * stream = NULL;
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( 
            "XXXXXXXXXXXX NOT sending HTTP POST request XXXXXXXXXXXXXXXX\n" ) );
                rc = KStreamMakeFromBuffer ( & stream, expected,
                                         string_size ( expected ) );
                if ( rc == 0 )
                    * response = stream;
            }
        }

        {
            rc_t r2 = SHttpRequestHelperFini ( & h );
            if ( rc == 0 )
                rc = r2;
        }
    }

    return rc;
}


/* SObject ********************************************************************/
static rc_t SObjectInit ( SObject * self,
    const char * objectId, size_t objSz, EObjectType objectType )
{
    assert ( self );
    self -> objectType = objectType;
    if ( objectId != NULL && objSz != 0 ) {
        self -> objectId = string_dup ( objectId, objSz );
        if ( self -> objectId == NULL ) {
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        }
    }
    return 0;
}


static void SObjectFini ( SObject * self ) {
    assert ( self );
    free ( self -> objectId );
    memset ( self, 0, sizeof * self );
}


/* SRequestData ***************************************************************/
static rc_t SRequestDataInit ( SRequestData * self ) {
    assert ( self );
    memset ( self, 0, sizeof * self );

    self -> allocated = 512;

    self -> object = calloc ( self -> allocated, sizeof * self -> object );
    if ( self -> object == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    return 0;
}


static void SRequestDataFini ( SRequestData * self ) {
    uint32_t i = 0;

    assert ( self );

    for ( i = 0; i < self -> objects; ++i )
        SObjectFini ( & self -> object [ i ] );

    free ( self -> object );

    memset ( self, 0, sizeof * self );
}


static
rc_t SRequestDataAppendObject ( SRequestData * self, const char * id,
    size_t id_sz, EObjectType objectType )
{
    rc_t rc = 0;

    assert ( self );

    if ( self -> objects > self -> allocated - 1 ) {
        size_t n = self -> allocated * 2;
        void * t = realloc ( self -> object, n * sizeof * self -> object );
        if ( t == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcExcessive );
        else {
            self -> object = t;
            self -> allocated = n;
        }
    }

    if ( id == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    if ( id [ 0 ] == '\0' )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcEmpty );

    if ( id_sz == 0 )
        id_sz = string_measure ( id, NULL );

    rc = SObjectInit ( & self -> object [ self -> objects ],
                       id, id_sz, objectType );

    if ( rc == 0 )
        ++ self -> objects;

    return rc;
}


/* BSTItem ********************************************************************/
static int64_t CC BSTItemCmp ( const void * item, const BSTNode * n ) {
    const String * s = item;
    const BSTItem * i = ( BSTItem * ) n;

    assert ( s && i );

    return string_cmp ( s -> addr, s -> size,
        i -> ticket, string_measure ( i -> ticket, NULL ), s -> size );
}

static
int64_t CC BSTreeSort ( const BSTNode * item, const BSTNode * n )
{
    const BSTItem * i = ( BSTItem * ) item;
    String str;
    size_t size = 0;
    uint32_t len = string_measure ( i -> ticket, & size );
    StringInit ( & str, i -> ticket, size, len );
    return BSTItemCmp ( & str, n );
}

static void BSTItemWhack ( BSTNode * n, void * ignore ) {
    BSTItem * i = ( BSTItem * ) n;
    assert ( i );
    free ( i -> ticket );
    memset ( i, 0, sizeof * i );
    free ( i );
}

/* STickets *******************************************************************/
const uint64_t BICKETS = 1024;
static rc_t STicketsAppend ( STickets * self, uint32_t project,
                             const char * ticket )
{
    rc_t rc = 0;

    const char * comma = "";

    assert ( self );

    if ( ticket == NULL )
        return 0;

    if ( rc == 0 && project > 0 && ticket [ 0 ] ) {
        BSTItem * i = NULL;

        String str;
        size_t size = 0;
        uint32_t len = string_measure ( ticket, & size );
        StringInit ( & str, ticket, size, len );

        i = ( BSTItem * ) BSTreeFind
            ( & self -> ticketsToProjects, & str, BSTItemCmp );
        if ( i != NULL )
            return 0;

        i = calloc ( 1, sizeof * i );
        if ( i != NULL )
            i -> ticket = string_dup_measure ( ticket, NULL );
        if ( i == NULL || i -> ticket == NULL ) {
            free ( i );
            return RC ( rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted );
        }

        i -> project = project;

        rc = BSTreeInsert ( & self -> ticketsToProjects, ( BSTNode * ) i,
                            BSTreeSort );
    }

    if ( self -> size > 0 )
        comma = ",";

    do {
        size_t num_writ = 0;
        char * p = ( char * ) self -> str . base;
        assert ( comma );
        rc = string_printf ( p + self -> size,
            ( size_t ) self -> str . elem_count - self -> size, & num_writ,
            "%s%s", comma, ticket );
        if ( rc == 0 ) {
            rc_t r2 = 0;
            String * s = ( String * ) malloc ( sizeof * s );
            if ( s == NULL )
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
            else {
                const char * addr = p + self -> size;
                uint32_t len = num_writ;
                if ( comma [ 0 ] != '\0' ) {
                    ++ addr;
                    -- len;
                }
                StringInit ( s, addr, len, len );
                r2 = VectorAppend ( & self -> tickets, NULL, s );
                if ( r2 != 0 ) {
                    rc = r2;
                    free ( s );
                }
                self -> size += num_writ;
                break;
            }
        }
        else if ( GetRCState ( rc ) == rcInsufficient
            && GetRCObject ( rc ) == ( enum RCObject ) rcBuffer )
        {
            size_t needed = ( size_t ) BICKETS;
            if ( self -> str . elem_count - self -> size + needed < num_writ )
                needed = num_writ;
            rc = KDataBufferResize
                ( & self -> str, self -> str . elem_count + needed );
        }
        else
            break;
    } while ( rc == 0 );

    return rc;
}


/*static void STicketsAppendFromVector ( void * item, void * data ) {
    STickets   * self   = ( STickets * ) data;
    rc_t rc = STicketsAppend ( self, ( char * ) item );
    if ( rc != 0 ) {
        self -> rc = rc;
    }
}*/


static rc_t STicketsInit ( STickets * self ) {
    assert ( self );
    memset ( self, 0, sizeof * self );
    BSTreeInit ( & self -> ticketsToProjects );
    return KDataBufferMakeBytes ( & self -> str, BICKETS );
}


static void whack_free ( void * self, void * ignore ) {
    if ( self != NULL ) {
        memset ( self, 0, sizeof ( * ( char * ) self ) );
        free ( self );
    }
}

static rc_t STicketsFini ( STickets * self ) {
    rc_t rc = 0;

    assert ( self );

    rc = KDataBufferWhack ( & self -> str );
    VectorWhack ( & self -> tickets, whack_free, NULL );
    BSTreeWhack ( & self -> ticketsToProjects, BSTItemWhack, NULL );

    memset ( self, 0 , sizeof * self );

    return rc;
}


/* Tickets ********************************************************************/
typedef struct {
    Vector * v;
    rc_t   rc;
} Tickets;


static void TicketsAppendTicket ( void * item, void * data ) {
    const String * ticket = ( String    * ) item;
    Tickets    * v      = ( Tickets * ) data;
    const SKV * kv = NULL;
    const char k [] = "tic";
    char * c = string_dup ( ticket -> addr, ticket -> size );
    if ( c == NULL ) {
        v -> rc = RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        return;
    }
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( "  %s=%s\n",
        k, c ) );
    {
        rc_t rc = SKVMake ( & kv, k, c );
        free ( c );
        if ( rc == 0 ) {
            rc = VectorAppend ( v -> v, NULL, kv );
            if ( rc != 0 && v -> rc == 0)
                v -> rc = rc;
        }
    }
}


/* SRequest *******************************************************************/
static rc_t SRequestInit ( SRequest * self ) {
    rc_t rc = 0;

    assert ( self );

    memset ( self, 0, sizeof * self );

    rc = STicketsInit ( & self -> tickets );

    if ( rc == 0 )
        rc = SRequestDataInit ( & self -> request );

    return rc;
}


static rc_t SRequestReset ( SRequest * self ) {
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self );

    r2 = SVersionFini ( & self -> version );
    if ( rc == 0 )
        rc = r2;

    SRequestDataFini ( & self -> request );
    SCgiRequestFini ( & self -> cgiReq );

    return rc;
}


static rc_t SRequestFini ( SRequest * self ) {
    rc_t r2 = 0;
    rc_t rc = SRequestReset ( self );

    assert ( self );

    r2 = STicketsFini ( & self -> tickets );
    if ( rc == 0 )
        rc = r2;

    memset ( self, 0, sizeof * self );

    return rc;
}


static rc_t SRequestAddTicket ( SRequest * self, uint32_t project,
                                const char * ticket )
{
    assert ( self );
    return STicketsAppend ( & self -> tickets, project, ticket );
}


static
rc_t SRequestInitNamesSCgiRequest ( SRequest * request, SHelper * helper,
    VRemoteProtocols protocols, const char * cgi,
    const char * version, bool aProtected )
{
    SCgiRequest * self = NULL;
    rc_t rc = 0;
    const SKV * kv = NULL;

    assert ( request );

    rc = SVersionFini ( & request -> version );
    if ( rc != 0 )
        return rc;

    rc = SVersionInit ( & request -> version, version, eSTnames );
    if ( rc != 0 )
        return rc;

    if ( protocols == eProtocolDefault )
        protocols = SHelperDefaultProtocols ( helper );
    request -> protocols = protocols;

    self = & request -> cgiReq;

    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( 
        "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n" ) );

    if ( self -> cgi == NULL ) {
        char buffer [ 1024 ] = "";
        if ( cgi == NULL ) {
            rc = SHelperResolverCgi ( helper, aProtected,
                                       buffer, sizeof buffer );
            cgi = buffer;
        }
        rc = SCgiRequestInitCgi ( self, cgi );
    }

    VectorWhack ( & self -> params, whackSKV, NULL );

    VectorInit ( & self -> params, 0, 5 );

    request -> serviceType = eSTnames;
    DBGMSG ( DBG_VFS,
        DBG_FLAG ( DBG_VFS_SERVICE ), ( "CGI = '%s'\n", self -> cgi ) );
    if ( rc == 0 ) {
        const char name [] = "version";
        char * version = NULL;
        rc = SVersionToString ( & request -> version, & version );
        if ( rc != 0 ) {
            return rc;
        }
        rc = SKVMake ( & kv, name, version );
        if ( rc == 0 ) {
            rc = VectorAppend ( & self -> params, NULL, kv );
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                ( "  %s=%s\n", name, version ) );
        }
        free ( version );
        if ( rc != 0 )
            return rc;
    }
    if ( ! SVersionHasMultpileObjects ( & request -> version ) ) {
        if ( request -> request . object [ 0 ] . objectId == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
        else {
            const char name [] = "acc";
            rc = SKVMake
                ( & kv, name, request -> request . object [ 0 ] . objectId );
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( "  %s=%s\n",
                name, request -> request . object [ 0 ] . objectId ) );
            if ( rc == 0 )
                rc = VectorAppend ( & self -> params, NULL, kv );
        }
        if ( rc != 0 )
            return rc;
    }
    else {
        uint32_t i = 0;
        for ( i = 0; i < request -> request . objects; ++i ) {
            request -> request . object [ i ] . ordId = i;
            rc = SKVMakeObj ( & kv, & request -> request . object [ i ],
                & request -> version );
            if ( rc == 0 ) {
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                    ( "  %.*s=%.*s\n", kv -> k . len, kv -> k . addr,
                                           kv -> v . len, kv -> v . addr ) );
                rc = VectorAppend ( & self -> params, NULL, kv );
            }
        }
        if ( rc != 0 )
            return rc;
    }
    if ( request -> tickets . size != 0 ) { /* optional */
        Tickets t = { & self -> params, 0 };
        VectorForEach ( & request -> tickets .tickets , false,
            TicketsAppendTicket, & t );
        rc = t . rc;
        if ( rc != 0 )
            return rc;
    }
    {
        uint32_t i = 0;
        const char * prefs [ eProtocolMaxPref ];
        const char * seps [ eProtocolMaxPref ];
        VRemoteProtocols protos = protocols;

        prefs [ 0 ] = seps [ 0 ] = NULL;
        prefs [ 1 ] = seps [ 1 ] = prefs [ 2 ] = seps [ 2 ]
                                 = prefs [ 3 ] = seps [ 3 ] = "";

        for ( i = 0; protos != 0 && i < sizeof prefs / sizeof prefs [ 0 ];
            protos >>= 3 )
        {
            /* 1.1 protocols */
            switch ( protos & eProtocolMask )
            {
            case eProtocolHttp:
                prefs [ i ] = "http";
                seps [ i ++ ] = ",";
                break;
            case eProtocolFasp:
                prefs [ i ] = "fasp";
                seps [ i ++ ] = ",";
                break;
            /* 1.2 protocols */
            case eProtocolHttps:
                prefs [ i ] = "https";
                seps [ i ++ ] = ",";
                break;
            /* 3.0 protocols */
            case eProtocolFile:
                prefs [ i ] = "file";
                seps [ i ++ ] = ",";
                break;
            default:
                assert ( 0 );
                break;
            }
        }
        if ( prefs [ 0 ] == NULL )
            rc = RC ( rcVFS, rcQuery, rcResolving, rcParam, rcInvalid );
        else
        {
            const char name [] = "accept-proto";
            size_t num_writ = 0;
            char p [ 512 ] = "";
            rc = string_printf ( p, sizeof p, & num_writ, "%s%s%s%s%s%s",
                prefs [ 0 ], seps [ 1 ], prefs [ 1 ], seps [ 2 ], prefs [ 2 ],
                                                      seps [ 3 ], prefs [ 3 ] );
            rc = SKVMake ( & kv, name, p );
            if ( rc == 0 ) {
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                    ( "  %s=%s\n", name, p ) );
                rc = VectorAppend ( & self -> params, NULL, kv );
            }
        }
        if ( rc != 0 ) {
            return rc;
        }
    }
    if ( SVersionHasRefseqCtx ( & request -> version ) &&
         request -> request . refseq_ctx )
    {
        const char name [] = "ctx";
        rc = SKVMake ( & kv, name, "refseq" );
        if ( rc == 0 ) {
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                    ( "  %s=refseq\n", name ) );
            rc = VectorAppend ( & self -> params, NULL, kv );
        }
        if ( rc != 0 ) {
            return rc;
        }
    }
    if ( SVersionTypInRequest ( & request -> version ) ) {
        if ( request -> request . object [ 0 ] . objectType !=
            eOT_undefined )
        {
            const char name [] = "typ";
            const char * v = ObjectTypeToString
                ( request -> request . object [ 0 ] . objectType );
            rc = SKVMake ( & kv, name, v );
            if ( rc == 0 ) {
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                    ( "  %s=%s\n", name, v ) );
                rc = VectorAppend ( & self -> params, NULL, kv );
            }
        }
        if ( rc != 0 ) {
            return rc;
        }
    }
    return rc;
}


static
rc_t SRequestInitSearchSCgiRequest ( SRequest * request, const char * cgi,
    const char * version )
{
    SCgiRequest * self = NULL;
    rc_t rc = 0;
    const SKV * kv = NULL;
    assert ( request );
    rc = SVersionInit ( & request -> version, version, eSTnames );
    if ( rc != 0 )
        return rc;
    self = & request -> cgiReq;
    if ( self -> cgi == NULL ) {
        if ( cgi == NULL ) {
/* try to get cgi from kfg, otherwise use hardcoded below */
            cgi =  "http://www.ncbi.nlm.nih.gov/Traces/names/search.cgi";
            cgi = "https://www.ncbi.nlm.nih.gov/Traces/names/search.cgi";
        }
        rc = SCgiRequestInitCgi ( self, cgi );
    }
    request -> serviceType = eSTsearch;
    VectorInit ( & self -> params, 0, 5 );
    DBGMSG ( DBG_VFS,
        DBG_FLAG ( DBG_VFS_SERVICE ), ( "CGI = '%s'\n", self -> cgi ) );
    if ( rc == 0 ) {
        const char name [] = "version";
        char * version = NULL;
        rc = SVersionToString ( & request -> version, & version );
        if ( rc != 0 ) {
            return rc;
        }
        rc = SKVMake ( & kv, name, version );
        if ( rc == 0 ) {
            rc = VectorAppend ( & self -> params, NULL, kv );
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                ( "  %s=%s\n", name, version ) );
        }
        free ( version );
        if ( rc != 0 ) {
            return rc;
        }
    }
    {
        const char name [] = "term";
        char * b = NULL;
        uint32_t i = 0;
        size_t l = 0;
        size_t o = 0;
        for ( i = 0; i < request -> request . objects; ++i ) {
            l += string_measure
                ( request -> request . object [ i ] . objectId, NULL ) + 1;
        }
        if ( l > 0 ) {
            b = ( char * ) malloc ( l );
            if ( b == NULL ) {
                return
                    RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
            }
            for ( i = 0; rc == 0 && i < request -> request . objects;
                ++i )
            {
                size_t num_writ = 0;
                rc = string_printf ( b + o, l - o, & num_writ,
                    "%s", request -> request . object [ i ] );
                o += num_writ;
                if ( i + 1 == request -> request . objects ) {
                    b [ o ++ ] = '\0';
                }
                else {
                    b [ o ++ ] = ',';
                }
            }
            assert ( o <= l );
            rc = SKVMake ( & kv, name, b );
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                ( "  %s=%s\n", name, b ) );
            if ( rc == 0 ) {
                rc = VectorAppend ( & self -> params, NULL, kv );
            }
            free ( b );
            if ( rc != 0 ) {
                return rc;
            }
        }
    }
    return rc;
}


/* KService *******************************************************************/
static void KServiceExpectErrors ( KService * self, int n ) {
    assert ( self );

    self -> req . errorsToIgnore = n;
}


static rc_t KServiceAddObject ( KService * self,
    const char * id, size_t id_sz, EObjectType objectType )
{
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    return SRequestDataAppendObject
        ( & self -> req . request, id, id_sz, objectType );
}


/* Add an Id ( Accession or Object-Id ) to service request */
rc_t KServiceAddId ( KService * self, const char * id ) {
    return KServiceAddObject ( self, id, 0, eOT_undefined );
}


static rc_t KServiceAddTicket ( KService * self, const char * ticket ) {
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( ticket == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    return SRequestAddTicket ( & self -> req, 0, ticket );
}


/* Add a dbGaP Project to service request */
rc_t KServiceAddProject ( KService * self, uint32_t project ) {
    rc_t rc = 0;

    char buffer [ 256 ] = "";
    size_t ticket_size = ~0;

    if ( project == 0 )
        return 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    rc = SHelperProjectToTicket ( & self -> helper, project,
        buffer, sizeof buffer, & ticket_size );
    if ( rc != 0 )
        return rc;

    assert ( ticket_size <= sizeof buffer );

    return SRequestAddTicket ( & self -> req, project, buffer );
}


static
rc_t KServiceInitNamesRequestWithVersion ( KService * self,
    VRemoteProtocols protocols,
    const char * cgi, const char * version, bool aProtected )
{
    assert ( self );

    return SRequestInitNamesSCgiRequest ( & self -> req,  & self -> helper,
        protocols, cgi, version, aProtected );
}


static
rc_t KServiceInitNamesRequest ( KService * self, VRemoteProtocols protocols,
    const char * cgi )
{
    return KServiceInitNamesRequestWithVersion ( self, protocols, cgi, "#3.0",
        false );
}


static
rc_t KServiceInitSearchRequestWithVersion ( KService * self, const char * cgi,
    const char * version )
{
    assert ( self );

    return SRequestInitSearchSCgiRequest ( & self -> req, cgi, version );
}


static rc_t KServiceInit ( KService * self, const KNSManager * mgr ) {
    rc_t rc = 0;

    assert ( self );
    memset ( self, 0, sizeof * self );

    if ( rc == 0 )
        rc = SHelperInit ( & self -> helper, mgr );

    if ( rc == 0 )
        rc = SResponseInit ( & self ->  resp );

    if ( rc == 0 )
        rc = SRequestInit ( & self -> req );

    self -> resoveOidName = DEFAULT_RESOVE_OID_NAME;

    return rc;
}


/* Initialize KService with a single "acc"/"objectType" and optional "ticket"
   in Request */
static rc_t KServiceInitNames1 ( KService * self, const KNSManager * mgr,
    const char * cgi, const char * version, const char * acc,
    size_t acc_sz, const char * ticket, VRemoteProtocols protocols,
    EObjectType objectType, bool refseq_ctx, bool aProtected )
{
    rc_t rc = 0;

    if ( rc == 0 )
        rc = KServiceInit ( self, mgr );

    if ( rc == 0 )
        rc = KServiceAddObject ( self, acc, acc_sz, objectType );
    if ( rc == 0 )
        rc = SRequestAddTicket ( & self -> req, 0, ticket );
    if ( rc == 0 )
        self -> req . request . refseq_ctx = refseq_ctx;

    if ( rc == 0 )
        rc = KServiceInitNamesRequestWithVersion
            ( self, protocols, cgi, version, aProtected );

    return rc;
}


static
rc_t KServiceMakeWithMgr ( KService ** self, const KNSManager * mgr )
{
    rc_t rc = 0;

    KService * p = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    p = ( KService * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    rc = KServiceInit ( p, mgr );
    if ( rc == 0)
        * self = p;
    else
        free ( p );

    return rc;
}


/* Make KService object */
rc_t KServiceMake ( KService ** self) {
    return KServiceMakeWithMgr ( self, NULL );
}


static rc_t KServiceFini ( KService * self ) {
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self );

    r2 = SResponseFini ( & self -> resp );
    if ( rc == 0 )
        rc = r2;

    r2 = SRequestFini ( & self -> req );
    if ( rc == 0 )
        rc = r2;

    r2 = SHelperFini ( & self -> helper );
    if ( rc == 0 )
        rc = r2;

    return rc;
}


/* Release KService object */
rc_t KServiceRelease ( KService * self ) {
    rc_t rc = 0;

    if ( self != NULL ) {
        rc = KServiceFini ( self );
        free ( self );
    }

    return rc;
}


static
rc_t KServiceProcessLine ( KService * self,
    const String * line, bool * end )
{
    rc_t rc = 0;
    assert ( self && line && end );
    if ( line -> addr [ 0 ] == '$' ) {
        * end = true;
        if ( SVersionResponseHasTimestamp
                ( & self -> resp . header . version )
            && line -> size > 2 && line -> len > 2 )
        {
            String timestamp;
            StringInit ( & timestamp, line -> addr + 2, line -> size - 2,
                                                        line -> len  - 2 );
            rc = SServerTimestampInit ( & self -> resp . timestamp,
                                        & timestamp );
        }
    }
    else if ( self -> req . serviceType == eSTsearch ) {
        const char str [] = "$end";
        size_t sz = sizeof str - 1;
        if ( string_cmp ( line -> addr, line -> size, str, sz, ( uint32_t ) sz )
            == 0)
        {
            * end = true;
        }
        else
            rc = KartAddRow ( self -> resp . kart, line -> addr, line -> size );
    }
    else {
        const SConverters * f = NULL;
        rc = SConvertersMake ( & f, & self -> resp . header );
        if ( rc == 0 ) {
            bool append = true;
            SRow * row = NULL;
            rc_t r2 = SRowMake ( & row, line, & self -> req, f,
                & self -> resp . header . version );
            uint32_t l = VectorLength ( & self -> resp . rows );
            if ( r2 == 0 ) {
                if ( SVersionHasMultpileObjects
                        ( & self -> resp . header. version )
                    || l == 0 )
                {
                    if ( l == 1 &&
                         self -> req . request . objects == 1 )
                    {
/* SRA-5283 VDB-3423: names.cgi version 3.0 incorrectly returns
           2 rows for filtered runs instead of 1: here we compensate this bug */
                        const KSrvError * error = NULL;
                        SRow * prev = VectorGet ( & self -> resp . rows, 0 );
                        assert ( prev );
                        error = row -> path . error;
                        if ( error != NULL && 
                             error -> code == 403 &&
                             error ->        objectType == eOT_sragap && 
                             prev -> typed . objectType == eOT_srapub &&
                             row -> typed . ordId == prev -> typed . ordId &&
                             StringEqual ( & row  -> typed . objectId,
                                           & prev -> typed . objectId ) )
                        {
                            append = false;
                        }
                    }
                }
                else {
/* ignore ACC.vdbcache : TODO : search for vdb.cache extension */
                    if ( l == 1 && ( row -> typed . objectId . len == 18 || 
                                     row -> typed . objectId . len == 19   ) )
                    {
                        append = false;
                    }
                }
                if ( append )
                    r2 = VectorAppend ( & self -> resp . rows, NULL, row );
                else {
                    r2 = SRowWhack ( row );
                    row = NULL;
                }
            }
            if ( r2 == 0 ) {
                if ( append && ( SVersionHasMultpileObjects
                                     ( & self -> resp . header . version)
                          || KSrvResponseLength ( self -> resp . list ) == 0 ) )
                            
                {
                    r2 = KSrvResponseAppend ( self -> resp . list, row -> set );
                }
                else
                    assert ( ! row );
            }
            if ( r2 != 0 && rc == 0 && l != 1 )
                rc = r2;
        }
    }
    return rc;
}


static
rc_t KServiceProcessStream ( KService * self, KStream * stream )
{
    bool start = true;
    char buffer [ 4096 ] = "";
    size_t num_read = 0;
    timeout_t tm;
    rc_t rc = 0;
    size_t sizeW = sizeof buffer;
    size_t sizeR = 0;
    size_t offR = 0;
    size_t offW = 0;
    char * newline = NULL;
    assert ( self );
    self -> resp . serviceType = self -> req . serviceType;
    rc = TimeoutInit ( & tm, self -> helper . timeoutMs );

    if ( rc == 0 )
        rc = SResponseFini ( & self -> resp );
    if ( rc == 0 )
        rc = SResponseInit ( & self -> resp );

    if ( rc == 0 && self -> req . serviceType == eSTsearch )
        rc = KartMake2 ( & self -> resp . kart );

    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), (
        "-----------------------------------------------------------\n" ) );

    while ( rc == 0 ) {
        if ( sizeR == 0 ) {
            rc = KStreamTimedRead
                ( stream, buffer + offW, sizeW, & num_read, & tm );
            if ( rc != 0 || num_read == 0 )
                break;
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                ( "%.*s", ( int ) num_read - 1, buffer + offW ) );
            sizeR += num_read;
            offW += num_read;
            if (sizeW >= num_read )
                sizeW -= num_read;
            else
                sizeW = 0;
        }
        newline = string_chr ( buffer + offR, sizeR, '\n' );
/* TODO different conditions: move buffer content; partial read; line > buf size
 */
        if ( newline == NULL ) {
            if ( sizeW == 0 && offR == 0 ) {
                rc = RC
                    ( rcVFS, rcQuery, rcExecuting, rcString, rcInsufficient );
                break;
            }
            else {
                memmove ( buffer, buffer + offR, sizeR );
                if ( sizeR < sizeof buffer )
                    buffer [ sizeR ] = '\0';
                sizeW = sizeof buffer - sizeR;
                offR = 0;
                offW = sizeR;
            }
            rc = KStreamTimedRead
                ( stream, buffer + offW, sizeW, & num_read, & tm );
            if ( rc != 0 ) {
                /* TBD - look more closely at rc */
                if ( num_read > 0 )
                    rc = 0;
                else
                    break;
            }
            else if ( num_read == 0 ) {
                rc = RC /* stream does not end by '\n' */
                    ( rcVFS, rcQuery, rcExecuting, rcString, rcInsufficient ); 
                break;
            }
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
                ( "%.*s", ( int ) num_read - 1, buffer + offW ) );
            sizeR += num_read;
            offW += num_read;
            if (sizeW >= num_read )
                sizeW -= num_read;
            else
                sizeW = 0;
        }
        else {
            size_t size = newline - ( buffer + offR );
            String s;
            s . addr = buffer + offR;
            s . len = s . size = size;
            if ( start ) {
                if ( size + 1 == num_read )
                    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( "\n" ) );
                rc = SHeaderMake
                    ( & self -> resp . header, & s, self -> req . serviceType );
                if ( rc != 0 )
                    break;
                start = false;
            }
            else {
                bool end = false;
                rc = KServiceProcessLine ( self, & s, & end );
                if ( end || rc != 0 ) {
                    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( "\n" ) );
                    break;
                }
            }
            ++ size;
            offR += size;
            if ( sizeR >= size )
                sizeR -= size;
            else
                sizeR = 0;
            if ( sizeR == 0 && offR == offW ) {
                offR = offW = 0;
                sizeW = sizeof buffer;
            }
        }
    }
    if ( start )
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcString, rcInsufficient );
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ),
        ( "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ "
          "rc = %R\n\n", rc ) );
    return rc;
}


static
rc_t KServiceGetResponse
    ( const KService * self, const KSrvResponse ** response )
{
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( response == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    else
        return SResponseGetResponse ( & self -> resp, response );
}


rc_t KServiceGetConfig ( struct KService * self, const KConfig ** kfg) {
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    if ( kfg == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    rc = SHelperInitKfg ( & self -> helper );
    if ( rc == 0 )
        rc = KConfigAddRef ( self -> helper . kfg );

    if ( rc == 0 )
        * kfg = self -> helper . kfg;

    return rc;
}

rc_t KServiceGetResolver ( KService * self, const String * ticket,
                           VResolver ** resolver )
{
    uint32_t project = 0;

    if ( self == NULL || ticket == NULL ||
         ticket -> addr == NULL || ticket -> size == 0 || resolver == NULL)
    {
        return 0;
    }
    else {
        const BSTItem * i = ( BSTItem * ) BSTreeFind 
            ( & self -> req . tickets . ticketsToProjects, ticket, BSTItemCmp );
        if ( i == NULL )
            return 0;

        project = i -> project;
    }

    * resolver = NULL;

    if ( project != 0 ) {
        const KRepository * r = NULL;
        rc_t rc = SHelperInitRepoMgr ( & self -> helper );
        if ( rc != 0 )
            return rc;

        rc = KRepositoryMgrGetProtectedRepository
            ( self -> helper . repoMgr,  project, & r );
        if ( rc != 0 )
            return rc;

        rc = KRepositoryMakeResolver ( r, resolver, self -> helper . kfg );

        RELEASE ( KRepository, r );

        return rc;
    }

    return 0;
}

static
rc_t KServiceNamesExecuteExtImpl ( KService * self, VRemoteProtocols protocols,
    const char * cgi, const char * version,
    const KSrvResponse ** response, const char * expected )
{
    rc_t rc = 0;

    KStream * stream = NULL;

    if ( self == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( response == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    if ( version == NULL )
        version = "#3.0";

    rc = KServiceInitNamesRequestWithVersion ( self, protocols, cgi, version,
        false );

    if ( rc == 0 )
        rc = SCgiRequestPerform ( & self -> req . cgiReq, & self -> helper,
                                  & stream, expected );

    if ( rc == 0 )
        rc = KServiceProcessStream ( self, stream );

    if ( rc == 0 )
        rc = KServiceGetResponse ( self, response );

    RELEASE ( KStream, stream );

    return rc;
}


/* Emulate Names Service Call :
   - prepare the request;
   - use "expected" instead of calling "cgi"
   - parse "expected" as "cgi" response */
rc_t KServiceTestNamesExecuteExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version,
    const struct KSrvResponse ** response, const char * expected )
{
    return KServiceNamesExecuteExtImpl ( self, protocols, cgi, version,
        response, expected );
}


/* Execute Names Service Call : extended version */
rc_t KServiceNamesExecuteExt ( KService * self, VRemoteProtocols protocols,
    const char * cgi, const char * version,
    const KSrvResponse ** response )
{
    return KServiceNamesExecuteExtImpl ( self, protocols, cgi, version,
        response, NULL );
}


/* Execute Names Service Call using current default protocol version;
   get KSrvResponse */
rc_t KServiceNamesExecute ( KService * self, VRemoteProtocols protocols,
    const KSrvResponse ** response )
{
    return KServiceNamesExecuteExt ( self, protocols, NULL, NULL, response );
}


static rc_t CC KService1NameWithVersionAndType ( const KNSManager * mgr,
    const char * url, const char * acc, size_t acc_sz, const char * ticket,
    VRemoteProtocols protocols, const VPath ** remote,  const VPath ** mapping,
    bool refseq_ctx, const char * version, EObjectType objectType,
    bool aProtected )
{
    rc_t rc = 0;

    KStream * stream = NULL;

    KService service;

    if ( acc == NULL || remote == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    rc = KServiceInitNames1 ( & service, mgr, url, version,
        acc, acc_sz, ticket, protocols, objectType, refseq_ctx, aProtected );

    protocols = service . req . protocols;

    if ( rc == 0 )
        rc = SCgiRequestPerform ( & service . req . cgiReq, & service . helper,
                                  & stream, NULL );

    if ( rc == 0 )
        rc = KServiceProcessStream ( & service, stream );

    if ( rc == 0 ) {
        if ( VectorLength ( & service . resp . rows ) != 1)
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcRow, rcIncorrect );
        else {
            uint32_t l = KSrvResponseLength ( service . resp . list );
            if ( rc == 0 ) {
                if ( l != 1 )
                    rc = RC ( rcVFS, rcQuery, rcExecuting, rcRow, rcIncorrect );
                else {
                    const KSrvError * error = NULL;
                    rc = KSrvResponseGetPath ( service . resp . list, 0, 
                        protocols, NULL, NULL, & error );
                    if ( rc == 0 && error != NULL ) {
                        KSrvErrorRc ( error, & rc );
                        KSrvErrorRelease ( error );
                    }
                    else {
                        const SRow * r =
                            ( SRow * ) VectorGet ( & service . resp . rows, 0 );
                        if ( r == NULL)
                            rc = RC
                                ( rcVFS, rcQuery, rcExecuting, rcRow, rcNull );
                        else {
                            const VPath * path = NULL;
                            VRemoteProtocols protos = protocols;
                            int i = 0;
                            for ( i = 0; protos != 0 && i < eProtocolMaxPref;
                                  protos >>= 3, ++ i )
                            {
                                switch ( protos & eProtocolMask ) {
                                    case eProtocolHttp:
                                        path = r -> path . http;
                                        break;
                                    case eProtocolFasp:
                                        path = r -> path . fasp;
                                        break;
                                    case eProtocolHttps:
                                        path = r -> path . https;
                                        break;
                                }
                                if ( path != NULL )
                                    break;
                            }

              /* in early versions of protocol only http path was initialized */
                            if ( path == NULL )
                                path = r -> path . http;

                            rc = VPathAddRef ( path );
                            if ( rc == 0 )
                                * remote = path;
                            if ( mapping ) {
                                path = r -> path . mapping;
                                rc = VPathAddRef ( path );
                                if ( rc == 0 )
                                    * mapping = path;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( rc == 0 ) {
        uint32_t l = KSrvResponseLength ( service . resp . list );
        if ( l != 1)
            rc = RC ( rcVFS, rcQuery, rcResolving, rcQuery, rcUnauthorized );
        else {
            const VPathSet * s = NULL;
            rc = KSrvResponseGet ( service . resp . list, 0, & s );
            if ( rc != 0 ) {
            }
            else if ( s == NULL )
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcRow, rcIncorrect );
            else {
                const VPath * path = NULL;
                const VPath * cache = NULL;
                rc = VPathSetGet ( s, protocols, & path, & cache );
                if ( rc == 0 ) {
                    int notequal = ~ 0;
                    assert ( remote );
                    rc = VPathEqual ( * remote, path, & notequal );
                    if ( rc == 0 )
                        rc = notequal;
                    RELEASE ( VPath, cache );
                    RELEASE ( VPath, path );
                }
            }
            RELEASE ( VPathSet, s );
        }
    }

    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 )
            rc = r2;
    }

    RELEASE ( KStream, stream );

    return rc;
}


/* make name service call : request: 1 object, response: 1 object */
LIB_EXPORT
rc_t CC KService1NameWithVersion ( const KNSManager * mgr, const char * url,
    const char * acc, size_t acc_sz, const char * ticket,
    VRemoteProtocols protocols, const VPath ** remote, const VPath ** mapping,
    bool refseq_ctx, const char * version, bool aProtected )
{
    if ( version == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    return KService1NameWithVersionAndType ( mgr, url, acc, acc_sz, ticket,
        protocols, remote, mapping, refseq_ctx, version, eOT_undefined,
        aProtected );
}


/* Execute Search Service Call : extended version */
rc_t KServiceSearchExecuteExt ( KService * self, const char * cgi,
    const char * version, const Kart ** result )
{
    rc_t rc = 0;

    KStream * stream = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( result == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    if ( version == NULL )
        version = "#1.0";

    rc = KServiceInitSearchRequestWithVersion ( self, cgi, version );

    if ( rc == 0 )
        rc = SCgiRequestPerform ( & self -> req . cgiReq, & self -> helper,
                                  & stream, NULL );

    if ( rc == 0 )
        rc = KServiceProcessStream ( self, stream );

    if ( rc == 0 ) {
        rc = KartAddRef ( self -> resp . kart );
        if ( rc == 0 )
            * result = self -> resp . kart;
    }

    RELEASE ( KStream, stream );

    return rc;
}


/* Execute Search Service Call; get Kart response */
rc_t KServiceSearchExecute ( KService * self, const Kart ** response ) {
    return KServiceSearchExecuteExt ( self, NULL, NULL, response );
}


/* Execute a simple ( one accession in request ) Search Service Call;
   get Kart response */
rc_t KService1Search ( const KNSManager * mgr, const char * cgi,
    const char * acc, const Kart ** result )
{
    rc_t rc = 0;

    KService service;

    rc = KServiceInit ( & service, mgr );

    if ( rc == 0 )
        rc = KServiceAddId ( & service, acc );

    if ( rc == 0 )
        rc = KServiceSearchExecute ( & service, result );

    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 )
            rc = r2;
    }
    return rc;
}

/* resolve mapping id -> file nime inside of VFS */
rc_t KServiceResolveName ( KService * self, int resolve ) {
    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcUpdating, rcSelf, rcNull );

    switch ( resolve ) {
        case 0 : self -> resoveOidName = DEFAULT_RESOVE_OID_NAME; break; 
        case 1 : self -> resoveOidName = true                   ; break; 
        default: self -> resoveOidName = false                  ; break; 
    }
    return 0;
}

int KServiceGetResolveName ( const KService * self ) {
    if ( self == NULL )
        return DEFAULT_RESOVE_OID_NAME;
    if ( self -> resoveOidName )
        return 1;
    else
        return 2;
}

/* TESTS **********************************************************************/
typedef struct {
    rc_t passed;
    const char * acc;
    const char * version;
    VRemoteProtocols protocols;
} SKVCheck;

static void SCgiRequestCheck ( void * item, void * data ) {
 /* const SKV * kv = ( SKV      * ) item; */
    SKVCheck  * p  = ( SKVCheck * ) data;
    assert ( p );
    p -> passed = 0;
}

static void SKVCheckInit ( SKVCheck * self, const char * acc,
    const char * version, VRemoteProtocols protocols )
{
    assert ( self );
    memset ( self, 0, sizeof * self );
    self -> acc = acc;
    self -> version = version;
    self -> protocols = protocols;
    self -> passed = -1;
}

rc_t KServiceRequestTestNames1 ( const KNSManager * mgr,
    const char * cgi, const char * version, const char * acc, size_t acc_sz,
    const char * ticket, VRemoteProtocols protocols,
    EObjectType objectType )
{
    KService service;
    rc_t rc = KServiceInitNames1 ( & service, mgr, cgi, version,
        acc, acc_sz,  ticket, protocols, objectType, false, false );
    if ( rc == 0 ) {
        SKVCheck c;
        SKVCheckInit ( & c, acc, version, protocols );
        VectorForEach
            ( & service . req . cgiReq . params, false, SCgiRequestCheck, & c );
        rc = c . passed;
    }
    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 ) {
            rc = r2;
        }
    }
    return rc;
}

rc_t KServiceNamesRequestTest ( const KNSManager * mgr, const char * b,
    const char * cgi, VRemoteProtocols protocols,
    const SServiceRequestTestData * d, ... )
{
    va_list args;
    KService * service = NULL;
    KStream * stream = NULL;
    rc_t rc = KServiceMakeWithMgr ( & service, mgr);
    va_start ( args, d );
    while ( rc == 0 && d != NULL ) {
        if ( d -> id != NULL ) {
            rc = KServiceAddObject ( service, d -> id, 0, d -> type );
        }
        if ( rc == 0 && d -> ticket != NULL ) {
            rc = KServiceAddTicket ( service, d -> ticket );
        }
        d = va_arg ( args, const SServiceRequestTestData * );
    }
    if ( rc == 0 ) {
        rc = KServiceInitNamesRequest ( service, protocols, cgi );
    }
    if ( rc == 0 ) {
        SKVCheck c;
    /*SKVCheckInit ( & c, acc, service -> req . version .raw . s, protocols );*/
        VectorForEach (
            & service -> req . cgiReq . params, false, SCgiRequestCheck, & c );
        rc = c . passed;
    }
    if ( rc == 0 ) {
        rc = KStreamMakeFromBuffer ( & stream, b, string_size ( b ) );
    }
    if ( rc == 0 ) {
        rc = KServiceProcessStream ( service, stream );
    }
    if ( rc == 0 ) {
        const KSrvResponse * l = NULL;
        rc = KServiceGetResponse ( service, & l );
        if ( rc == 0 ) {
            uint32_t i = 0;
            uint32_t n = KSrvResponseLength ( l );
            for ( i = 0; rc == 0 && i < n; ++i ) {
                const VPathSet * s = NULL;
                rc = KSrvResponseGet ( l, i, & s );
                RELEASE ( VPathSet, s );
            }
        }
        RELEASE ( KSrvResponse, l );
    }
    RELEASE ( KStream, stream );
    RELEASE ( KService, service );
    return rc;
}

rc_t KServiceFuserTest ( const KNSManager * mgr,  const char * ticket,
    const char * acc, ... )
{
    va_list args;
    KService * service = NULL;
    const KSrvResponse * response = NULL;
    rc_t rc = KServiceMake ( & service);
    va_start ( args, acc );
    while ( rc == 0 && acc != NULL ) {
        rc = KServiceAddId ( service, acc );
        acc = va_arg ( args, const char * );
    }
    if ( rc == 0 ) {
        rc = KServiceNamesQuery ( service, eProtocolDefault, & response );
    }
    if ( rc == 0 ) {
        uint32_t i = 0;
        for ( i = 0; rc == 0 && i < KSrvResponseLength ( response ); ++i ) {
            const VPath * path = NULL;
            rc = KSrvResponseGetPath ( response, i, 0, & path, NULL, NULL );
            if ( rc == 0 ) {
                rc_t r2;
/*KTime_t mod = VPathGetModDate ( path );size_t size = VPathGetSize ( path );*/
                String id;
                memset ( & id, 0, sizeof id );
                r2 = VPathGetId ( path, & id );
                if ( rc == 0 )
                    rc = r2;
            }
            RELEASE ( VPath, path );
        }
    }
    RELEASE ( KSrvResponse, response );
    RELEASE ( KService, service );
    return rc;
}

rc_t SCgiRequestPerformTestNames1 ( const KNSManager * mgr, const char * cgi,
    const char * version, const char * acc, const char * ticket,
    VRemoteProtocols protocols, EObjectType objectType )
{
    KService service;

    rc_t rc = KServiceInitNames1 ( & service, mgr, cgi, version, acc,
        string_measure ( acc, NULL ), ticket, protocols, objectType, false,
        false );

    if ( rc == 0 ) {
        KStream * response = NULL;
        rc = SCgiRequestPerform ( & service . req . cgiReq, & service . helper,
                                  & response, NULL );
        RELEASE ( KStream, response );
    }

    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 )
            rc = r2;
    }

    return rc;
}

rc_t KServiceProcessStreamTestNames1 ( const KNSManager * mgr,
    const char * b, const char * version, const char * acc,
    const VPath * exp, const char * ticket, const VPath * ex2,
    int errors )
{
    KService service;
    KStream * stream = NULL;
    rc_t rc = 0;
    if ( rc == 0 )
        rc = KServiceInitNames1 ( & service, mgr, "", version, acc,
            string_measure ( acc, NULL ), ticket, eProtocolHttps,
            eOT_undefined, false, false );
    if ( rc == 0 ) {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_SERVICE ), ( 
            "XXXXXXXXXXXX NOT sending HTTP POST request XXXXXXXXXXXXXXXX\n" ) );
        rc = KStreamMakeFromBuffer ( & stream, b, string_size ( b ) );
    }
    if ( rc == 0 )
        KServiceExpectErrors ( & service, errors );
    if ( rc == 0 )
        rc = KServiceProcessStream ( & service, stream );
    if ( rc == 0 ) {
        if ( VectorLength ( & service . resp . rows ) != 1 )
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcRow, rcExcessive );
        else {
            const VPath * path = NULL;
            const SRow * r
                = ( SRow * ) VectorGet ( & service . resp . rows, 0 );
            if ( r == NULL)
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcVector, rcEmpty );
            else
                if ( r -> path . error != NULL )
                    rc = r -> path . error -> rc;
                else
                    path = r -> path . http;
            if ( exp != NULL && rc == 0 ) {
                int notequal = ~ 0;
                rc = VPathEqual ( path, exp, & notequal );
                if ( rc == 0 )
                    rc = notequal;
            }
            if ( ex2 != NULL && rc == 0 ) {
                int notequal = ~ 0;
                rc = VPathEqual ( path, ex2, & notequal );
                if ( rc == 0 )
                    rc = notequal;
            }
        }
    }
    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 ) {
            rc = r2;
        }
    }
    RELEASE ( KStream, stream );
    return rc;
}


/* Parse "buffer" as names-3.0 response.
   Do not log "errorsToIgnore" messages during response processing */
rc_t KServiceNames3_0StreamTest ( const char * buffer,
    const KSrvResponse ** response, int errorsToIgnore )
{
    rc_t rc = 0;
    rc_t r2 = 0;

    KStream * stream = NULL;

    KService service;
    if ( rc == 0 )
        rc = KServiceInit ( & service, NULL );
    if ( rc == 0 )
        KServiceExpectErrors ( & service, errorsToIgnore );

    if ( rc == 0 )
        rc = KStreamMakeFromBuffer ( & stream, buffer, string_size ( buffer ) );

    if ( rc == 0 )
        rc = KServiceProcessStream ( & service, stream );

    if ( rc == 0 )
        rc = KServiceGetResponse ( & service , response );

    r2 = KServiceFini ( & service );
    if ( rc == 0 )
        rc = r2;

    RELEASE ( KStream, stream );

    return rc;
}

rc_t KServiceCgiTest1 ( const KNSManager * mgr, const char * cgi,
    const char * version, const char * acc, const char * ticket,
    VRemoteProtocols protocols, EObjectType objectType,
    const VPath * exp, const VPath * ex2 )
{
    const VPath * path = NULL;
    rc_t rc = KService1NameWithVersionAndType ( mgr, cgi, acc,
        string_measure ( acc, NULL ), ticket, protocols,
        & path, NULL, false, version, objectType, false );
    if ( rc == 0 ) {
        if ( exp != NULL && rc == 0 ) {
            int notequal = ~ 0;
            rc = VPathEqual ( path, exp, & notequal );
            if ( rc == 0 ) {
                rc = notequal;
            }
        }
        if ( ex2 != NULL && rc == 0 ) {
            int notequal = ~ 0;
            rc = VPathEqual ( path, ex2, & notequal );
            if ( rc == 0 ) {
                rc = notequal;
            }
        }
    }
    RELEASE ( VPath, path );
    return rc;
}

rc_t KServiceSearchTest1
    ( const KNSManager * mgr, const char * cgi, const char * acc )
{
    rc_t rc = 0;
    KService service;
    const Kart * result = NULL;
    rc = KServiceInit ( & service, mgr );
    if ( rc == 0 ) {
        rc = KServiceAddId ( & service, acc );
    }
    if ( rc == 0 )
        rc = KServiceSearchExecute ( & service, & result );
    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 )
            rc = r2;
    }
    RELEASE ( Kart, result );
    return rc;
}

rc_t KServiceSearchTest (
    const KNSManager * mgr, const char * cgi, const char * acc, ... )
{
    va_list args;
    rc_t rc = 0;
    KStream * stream = NULL;
    const Kart * result = NULL;
    KService service;
    rc = KServiceInit ( & service, mgr );
    va_start ( args, acc );
    while ( rc == 0 && acc != NULL ) {
        rc = KServiceAddObject ( & service, acc, 0, eOT_undefined);
        acc = va_arg ( args, const char * );
    }
    if ( rc == 0 ) {
        rc = KServiceSearchExecuteExt ( & service, cgi, NULL, & result );
    }
    {
        rc_t r2 = KartRelease ( result );
        if ( rc == 0 ) {
            rc = r2;
        }
    }
    {
        rc_t r2 = KServiceFini ( & service );
        if ( rc == 0 ) {
            rc = r2;
        }
    }
    RELEASE ( KStream, stream );
    return rc;
}
/******************************************************************************/
