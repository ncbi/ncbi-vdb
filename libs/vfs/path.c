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

#include "path-priv.h"

#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>

#include <sysalloc.h>

#define MAX_ACCESSION_LEN 20
#define TREAT_URI_RESERVED_AS_FILENAME 0


/*--------------------------------------------------------------------------
 * VPath
 */


/* Whack
 */
static
rc_t VPathWhack ( VPath * self )
{
    rc_t rc = VPathRelease(self->vdbcache);

    StringWhack(self->accOfParentDb);

    KDataBufferWhack ( & self -> data );
    KRefcountWhack ( & self -> refcount, "VPath" );

    free ( ( void * ) self -> id   . addr );
    free ( ( void * ) self -> name . addr );
    free ( ( void * ) self -> objectType . addr );
    free ( ( void * ) self -> service . addr );
    free ( ( void * ) self -> tick . addr );
    free ( ( void * ) self -> type . addr );

    memset ( self, 0, sizeof * self );
    free ( self );

    return rc;
}

/* ParseURI
 *  parse a flexible URI
 *  allowed to be a formal URI with scheme,
 *  a simple POSIX path, or a simple accession
 *
 * NB - the purpose is to parse, not to enforce correctness
 *  emphasis is placed on speed and leniency
 *
 *  url
 *      = accession
 *      | posix-path
 *      | scheme-spec '//' host-spec full-name-spec opt-query opt-fragment
 *      ;
 *
 *  accession
 *      = alpha-part digits-part opt-extensions
 *      | prefix alpha-part digits-part opt-extensions
 *      | prefix digits-part extensions opt-suffix
 *      ;
 *
 *  prefix
 *      = [A-Za-z]+ '_'
 *
 *  alpha-part
 *      = [A-Za-z]+
 *      ;
 *
 *  digits-part
 *      = [0-9]+
 *      ;
 *
 *  opt-extensions
 *      =
 *      | extensions
 *      ;
 *
 *  extensions
 *      = extension
 *      | extensions extension
 *      ;
 *
 *  extension
 *      = '.' [0-9]+
 *      ;
 *
 *  opt-suffix
 *      =
 *      | '_' [A-Za-z]+
 *      ;
 *
 *  obj-id
 *      = [0-9]+
 *      ;
 *
 *  posix-path
 *      = full-path
 *      | rel-path
 *      ;
 *
 *  full-path
 *      = '/' rel-path
 *      ;
 *
 *  rel-path
 *      = name
 *      | rel-path '/' name
 *      ;
 *
 *  name
 *      = [^?#/]+
 *      ;
 *
 *  scheme-spec
 *      = scheme ':'
 *      ;
 *
 *  scheme
 *      = [A-Za-z][-+A-Za-z0-9.]*
 *      ;
 *
 *  host-spec
 *      = dns-name
 *      | ipv4-spec
 *      | '[' ipv6-spec ']'
 *      ;
 *
 *  full-name-spec
 *      = '/' accession
 *      | full-path
 *      | '/' obj-id
 *      ;
 *
 *  opt-query
 *      =
 *      | '?' [^#]+
 *      ;
 *
 *  opt-fragment
 *      =
 *      | '#' any
 *      ;
 */
typedef enum VPathParseState
{
    vppStart,
    vppAccPrefixAlphaNamePathOrScheme,
    vppAccAlphaNamePath,
    vppAccDigitNamePathOrScheme,
    vppAccDigitNamePath,
    vppAccExtNamePathOrScheme,
    vppAccExtNamePath,
    vppAccSuffixNamePath,
    vppAccDotNamePathOrScheme,
    vppAccDotNamePath,
    vppAccUnderNamePath,
    vppNamePathOrScheme,
    vppAccOidRelOrSlash,
    vppAccPrefixAlphaRel,
    vppAccAlphaRel,
    vppAccDigitRel,
    vppAccExtRel,
    vppAccSuffixRel,
    vppOidRel,
    vppAccDotRel,
    vppAccUnderRel,
    vppSlash,
    vppAuthHostSpec,
    vppAuthHostNamePort,
    vppHostSpec,
    vppHostNamePort,
    vppIPv4Port,
    vppIPv4Dot,
    vppIPv6Port,
    vppIPv6Colon,
    vppPortSpecOrFullPath,
    vppPortSpec,
    vppPortName,
    vppPortNum,
    vppNamePath,
    vppUNCOrMalformedPOSIXPath,
    vppFullOrUNCPath,
    vppRelPath,
    vppFullPath,
    vppUNCPath,
    vppParamName,
    vppParamValue,
    vppFragment
}
VPathParseState;

static const struct sm
{
    const char * scheme;
    VPUri_t type;
} schemes[] =
{
    { "file", vpuri_file }, /* Most popular first */
    { "https", vpuri_https},
    { "http", vpuri_http},
    { "ftp", vpuri_ftp },
    { "fasp", vpuri_fasp },
    { "s3",vpuri_s3 },
    { "scp", vpuri_scp },
    { "sftp", vpuri_sftp },
    { "ncbi-file", vpuri_ncbi_file },
    { "ncbi-acc", vpuri_ncbi_acc },
    { "x-ncbi-legresfseq", vpuri_ncbi_legrefseq },
    { "ncbi-obj", vpuri_ncbi_obj  },
    { "gs", vpuri_google },
    { "azure", vpuri_azure }
};

static
void VPathCaptureScheme ( VPath * self, const char * uri, size_t start, size_t end )
{
    size_t size = end - start;
    char buf[64];
    size_t i, l, num_schemes;
    const char * scheme;

    StringInit ( & self -> scheme, & uri [ start ], size, ( uint32_t ) ( size ) );
    self -> from_uri = true;
    self -> scheme_type = vpuri_not_supported;

    if ( size > 0 && size < 64)
    {
        scheme = & uri [ start ];
        self -> scheme_type = vpuri_not_supported;
        tolower_copy(buf, sizeof buf, scheme, size);
        num_schemes=sizeof(schemes)/sizeof(schemes[0]);
        for (i=0; i!=num_schemes; ++i)
        {
            l=strlen(schemes[i].scheme);
            if (l==size && memcmp(buf, schemes[i].scheme, l) == 0)
            {
                self -> scheme_type = schemes[i].type;
                return;
            }
        }
    }
}

static
void VPathCaptureAccession ( VPath * self, const char * uri, size_t start, size_t end )
{
    size_t size = end - start;
    StringInit ( & self -> path, & uri [ start ], size, ( uint32_t ) size );

    switch ( self -> scheme_type )
    {
    case vpuri_none:
         self -> path_type = vpNameOrAccession;
         break;
    case vpuri_ncbi_acc:
        if ( size < MAX_ACCESSION_LEN )
        {
            self -> path_type = vpAccession;
            break;
        }
        /* no break */
    default:
         self -> path_type = vpName;
    }
}

static
void VPathCaptureAccCode ( VPath * self, uint32_t acc_prefix,
    uint32_t acc_alpha, uint32_t acc_digit, uint32_t acc_ext, uint32_t acc_suffix )
{
    self -> acc_code
        = ( acc_prefix << 16 )
        | ( acc_alpha << 12 )
        | ( acc_digit << 8 )
        | ( acc_ext << 4 )
        | ( acc_suffix << 0 )
        ;

    if ( self -> path_type == vpNameOrAccession )
    {
        switch ( self -> acc_code >> 8 )
        {
        case 0x015:
        case 0x026:
        case 0x106:
        case 0x126:
            /* refseq */
            self -> path_type = vpAccession;
            break;

        case 0x109:
            /* refseq or named annotation */
            self -> path_type = vpAccession;
            break;

        case 0x036:
        case 0x037:
        case 0x038:
        case 0x039:
            /* sra */
            break;

        case 0x042:
        case 0x048:
        case 0x049:
        case 0x142:
        case 0x148:
        case 0x149:
            /* wgs */
            self -> path_type = vpAccession;
            break;

        case 0x029:
            if ( self -> acc_code == 0x02910 )
            {
                if ( self -> path . addr [ 0 ] == 'N' &&
                     self -> path . addr [ 1 ] == 'A' )
                {
                    /* na */
                    self -> path_type = vpAccession;
                }
            }
            break;
        }
    }
}

static
void VPathCaptureOid ( VPath * self, uint64_t oid, const char * uri, size_t start, size_t oid_start, size_t end )
{
    size_t oid_size = end - oid_start;

    if ( oid == 0 || oid_size > 10 || oid > 0xFFFFFFFF )
        self -> path_type = vpName;
    else
    {
        self -> obj_id = ( uint32_t ) oid;

        if ( self -> scheme_type == vpuri_ncbi_obj )
        {
            StringInit ( & self -> path, & uri [ oid_start ], oid_size, ( uint32_t ) oid_size );
            self -> path_type = vpOID;
            return;
        }

        self -> path_type = vpNameOrOID;
    }

    StringInit ( & self -> path, & uri [ start ], ( end - start ), ( uint32_t ) ( end - start ) );
}

static
void VPathCaptureAuth ( VPath * self, const char * uri, size_t start, size_t end )
{
    StringInit ( & self -> auth, & uri [ start ], end - start, ( uint32_t ) ( end - start ) );
    self -> path_type = vpAuth;
}

static
void VPathCaptureHostName ( VPath * self, const char * uri, size_t start, size_t end )
{
    StringInit ( & self -> host, & uri [ start ], end - start, ( uint32_t ) ( end - start ) );
    self -> path_type = vpHostName;
}

static
rc_t VPathCaptureIPv4 ( VPath * self, const uint32_t ipv4 [ 4 ] )
{
    uint32_t i;

    for ( i = 0; i < 4; ++ i )
    {
        if ( ipv4 [ i ] >= 256 )
            return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );
    }

    self -> ipv4
        = ( ipv4 [ 0 ] << 24 )
        | ( ipv4 [ 1 ] << 16 )
        | ( ipv4 [ 2 ] << 8 )
        | ( ipv4 [ 3 ] << 0 )
        ;

    self -> path_type = vpEndpoint;
    self -> host_type = vhIPv4;

    return 0;
}

static
rc_t VPathCaptureIPv6 ( VPath * self, const uint32_t ipv6 [ 8 ] )
{
    uint32_t i;

    for ( i = 0; i < 8; ++ i )
    {
        if ( ipv6 [ i ] >= 0x10000 )
            return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );

        self -> ipv6 [ i ] = ( uint16_t ) ipv6 [ i ];
    }

    self -> path_type = vpEndpoint;
    self -> host_type = vhIPv6;

    return 0;
}

static
void VPathCapturePortName ( VPath * self, const char * uri, size_t start, size_t end )
{
    StringInit ( & self -> portname, & uri [ start ], end - start, ( uint32_t ) ( end - start ) );
    self -> path_type = vpEndpoint;
}

static
rc_t VPathCapturePortNum ( VPath * self, uint32_t port )
{
    if ( port >= 0x10000 )
        return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );

    self -> portnum = ( uint16_t ) port;
    self -> path_type = vpEndpoint;

    return 0;
}

static
void VPathCapturePath ( VPath * self, const char * uri, size_t start, size_t end, uint32_t count, uint32_t var )
{
    StringInit ( & self -> path, & uri [ start ], end - start, count );
    self -> path_type = ( uint8_t ) var;
}

static
void VPathCaptureQuery ( VPath * self, const char * uri, size_t start, size_t end, uint32_t count )
{
    StringInit ( & self -> query, & uri [ start ], end - start, count );
}

static
void VPathCaptureFragment ( VPath * self, const char * uri, size_t start, size_t end, uint32_t count )
{
    StringInit ( & self -> fragment, & uri [ start ], end - start, count );
}

#define VPathParseResetAnchor( i ) \
    do { anchor = ( i ); count = 0; } while ( 0 )

static
rc_t VPathParseInt ( VPath * self, char * uri, size_t uri_size,
                    bool uri_is_utf )
{
    rc_t rc;
    int bytes;
    uint32_t port;
    size_t i, anchor;
    uint32_t count, total;
    VPathParseState state = vppStart;

    /* for handling current accession schemes */
    uint32_t acc_prefix = 0;
    uint32_t acc_alpha = 0;
    uint32_t acc_digit = 0;
    uint32_t acc_ext = 0;
    uint32_t acc_suffix = 0;

    /* for accummulating ip addresses */
    uint32_t ip;
    uint32_t ipv4 [ 4 ];
    uint32_t ipv6 [ 8 ];

    /* for accumulating oid */
    uint64_t oid;
    uint32_t oid_anchor;
    
    bool pileup_ext_present = false;
    const char pileup_ext[] = ".pileup";
    size_t pileup_ext_size = sizeof( pileup_ext ) / sizeof( pileup_ext[0] ) - 1;
    
    bool realign_ext_present = false;
    const char realign_ext[] = ".realign";
    size_t realign_ext_size = sizeof(realign_ext) / sizeof(realign_ext[0]) - 1;

    bool vdbcache_ext_present = false;
    const char vdbcache_ext[] = ".vdbcache";
    size_t vdbcache_ext_size = sizeof(vdbcache_ext) / sizeof(vdbcache_ext[0]) - 1;

    /* remove pileup extension before parsing, so that it won't change parsing results */
    if ( uri_size > pileup_ext_size && memcmp(&uri[uri_size - pileup_ext_size], pileup_ext, pileup_ext_size) == 0)
    {
        uri_size -= pileup_ext_size;
        uri[uri_size] = '\0';
        pileup_ext_present = true;
    }

    /* remove realign extension before parsing,
       so that it won't change parsing results */
    else if (uri_size > realign_ext_size && memcmp
        (&uri[uri_size - realign_ext_size], realign_ext, realign_ext_size) == 0)
    {
        uri_size -= realign_ext_size;
        uri[uri_size] = '\0';
        realign_ext_present = true;
    }

    /* detect vdbcahde extension */
    else if (uri_size > vdbcache_ext_size && memcmp
        (&uri[uri_size - vdbcache_ext_size], vdbcache_ext, vdbcache_ext_size) == 0)
    {
        vdbcache_ext_present = true;
    }

    for ( i = anchor = 0, total = count = 0; i < uri_size; ++ total, ++ count, i += bytes )
    {
        /* read character */
        uint32_t ch = uri [ i ];

        /* assume ascii */
        bytes = 1;
        if ( ( char ) ch  < 0 )
        {
          if ( uri_is_utf ) {
/* expect uri in utf8  */
            uint32_t utf32;

            /* read into UTF-32 */
            bytes = utf8_utf32 ( & utf32, & uri [ i ], & uri [ uri_size ] );
            if ( bytes <= 0 )
            {
                if ( bytes < 0 )
                    return RC ( rcVFS, rcPath, rcParsing, rcChar, rcInvalid );
                return RC ( rcVFS, rcPath, rcParsing, rcData, rcInsufficient );
            }

            ch = utf32;
          }
          else {
/* uri is in extended ASCII  */
              ch &= 0xff;
          }
        }

        /* enter state */
        switch ( state )
        {
        case vppStart:

            if ( ch >= 128 )
                state = vppNamePath;
            else if ( isalpha ( ch ) )
            {
                acc_alpha = 1;
                state = vppAccPrefixAlphaNamePathOrScheme;
            }
            else if ( isdigit ( ch ) )
                state = vppNamePath;
            else switch ( ch )
            {
            case '/':
                state = vppFullOrUNCPath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                state = vppNamePath;
            }
            break;

        case vppAccPrefixAlphaNamePathOrScheme:

            if ( ch >= 128 )
            {
                acc_alpha = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                ++ acc_alpha;
            else if ( isdigit ( ch ) )
            {
                ++ acc_digit;
                state = vppAccDigitNamePathOrScheme;
            }
            else switch ( ch )
            {
            case '/':
                acc_alpha = 0;
                state = vppRelPath;
                break;

            case '_':
                acc_prefix = 1;
                state = vppAccAlphaNamePath;
                acc_alpha = 0;
                break;

            case '.':
            case '+':
            case '-':
                acc_alpha = 0;
                state = vppNamePathOrScheme;
                break;

            case ':':
                acc_alpha = 0;
                VPathCaptureScheme ( self, uri, anchor, i );
                state = vppAccOidRelOrSlash;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_alpha = 0;
                state = vppNamePath;
            }
            break;

        case vppAccAlphaNamePath:

            if ( ch >= 128 )
            {
                acc_prefix = acc_alpha = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                ++ acc_alpha;
            else if ( isdigit ( ch ) )
            {
                ++ acc_digit;
                state = vppAccDigitNamePath;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = 0;
                state = vppRelPath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDigitNamePathOrScheme:

            if ( ch >= 128 )
            {
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePathOrScheme;
            }
            else if ( isdigit ( ch ) )
                ++ acc_digit;
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppRelPath;
                break;

            case '.':
                state = vppAccDotNamePathOrScheme;
                break;

            case '+':
            case '-':
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePathOrScheme;
                break;

            case ':':
                acc_prefix = acc_alpha = acc_digit = 0;
                VPathCaptureScheme ( self, uri, anchor, i );
                state = vppAccOidRelOrSlash;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDigitNamePath:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
                ++ acc_digit;
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppRelPath;
                break;

            case '.':
                state = vppAccDotNamePath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            break;

        case vppAccExtNamePathOrScheme:

            if ( ch >= 128 )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePathOrScheme;
            }
            else if ( isdigit ( ch ) )
                break;
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case '.':
                state = vppAccDotNamePathOrScheme;
                break;

            case '+':
            case '-':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePathOrScheme;
                break;

            case ':':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                VPathCaptureScheme ( self, uri, anchor, i );
                state = vppAccOidRelOrSlash;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            case '_':
                if ( acc_prefix != 0 && acc_alpha == 0 && acc_digit == 9 )
                {
                    state = vppAccUnderNamePath;
                    break;
                }

                /* no break */

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccExtNamePath:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
                break;
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case '.':
                state = vppAccDotNamePath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            case '_':
                if ( acc_prefix != 0 && acc_alpha == 0 && acc_digit == 9 && acc_ext == 1 )
                {
                    state = vppAccUnderNamePath;
                    break;
                }

                /* no break */

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccSuffixNamePath:

            if ( ch >= 128 || isdigit ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = acc_suffix = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                break;
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = acc_suffix = 0;
                state = vppRelPath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = acc_suffix = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDotNamePathOrScheme:

            if ( ch >= 128 )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePathOrScheme;
            }
            else if ( isdigit ( ch ) )
            {
                ++ acc_ext;
                state = vppAccExtNamePathOrScheme;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case '.':
            case '+':
            case '-':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePathOrScheme;
                break;

            case ':':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                VPathCaptureScheme ( self, uri, anchor, i );
                state = vppAccOidRelOrSlash;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDotNamePath:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
            {
                ++ acc_ext;
                state = vppAccExtNamePath;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccUnderNamePath:

            if ( ch >= 128 || isdigit ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
            {
                ++ acc_suffix;
                state = vppAccSuffixNamePath;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppNamePathOrScheme:

            if ( ch >= 128 )
                state = vppNamePath;
            else if ( isalnum ( ch ) )
                break;
            else switch ( ch )
            {
            case '/':
                state = vppRelPath;
                break;

            case '.':
            case '+':
            case '-':
                break;

            case ':':
                VPathCaptureScheme ( self, uri, anchor, i );
                state = vppAccOidRelOrSlash;
                break;

#if ! TREAT_URI_RESERVED_AS_FILENAME
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
#endif

            default:
                state = vppNamePath;
            }
            break;

        case vppAccOidRelOrSlash:

            VPathParseResetAnchor ( i );
            acc_prefix = acc_digit = acc_ext = 0;

            if ( ch >= 128 )
                state = vppNamePath;

            else if ( isalpha ( ch ) )
            {
                acc_alpha = 1;
                state = vppAccPrefixAlphaRel;
            }

            else if ( isdigit ( ch ) )
            {
                state = vppOidRel;
                oid = ch - '0';
                oid_anchor = (uint32_t)i;
            }

            else if ( ch != '/' )
                state = vppNamePath;

            else
                state = vppSlash;

            break;

        case vppAccPrefixAlphaRel:

            if ( ch >= 128 )
            {
                acc_alpha = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                ++ acc_alpha;
            else if ( isdigit ( ch ) )
            {
                ++ acc_digit;
                state = vppAccDigitRel;
            }
            else switch ( ch )
            {
            case '_':
                acc_prefix = 1;
                acc_alpha = 0;
                state = vppAccAlphaRel;
                break;

            case '/':
                acc_alpha = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_alpha = 0;
                state = vppNamePath;
            }
            break;

        case vppAccAlphaRel:

            if ( ch >= 128 )
            {
                acc_prefix = acc_alpha = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                ++ acc_alpha;
            else if ( isdigit ( ch ) )
            {
                ++ acc_digit;
                state = vppAccDigitRel;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_prefix = acc_alpha = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDigitRel:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
                ++ acc_digit;
            else switch ( ch )
            {
            case '.':
                state = vppAccDotRel;
                break;

            case '/':
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_prefix = acc_alpha = acc_digit = 0;
                state = vppNamePath;
            }
            break;

        case vppAccExtRel:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
                break;
            else switch ( ch )
            {
            case '.':
                state = vppAccDotRel;
                break;

            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '_':
                if ( acc_prefix != 0 && acc_alpha == 0 && acc_digit == 9 && acc_ext == 1 )
                {
                    state = vppAccUnderRel;
                    break;
                }

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccSuffixRel:

            if ( ch >= 128 || isdigit ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = acc_suffix = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
                break;
            else switch ( ch )
            {
            case '.':
                state = vppAccDotRel;
                break;

            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureAccession ( self, uri, anchor, i );
                VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppOidRel:

            if ( ch >= 128 )
            {
                oid = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
            {
                if ( oid == 0 )
                    oid_anchor = (uint32_t)i;

                oid *= 10;
                oid += ch - '0';
            }
            else switch ( ch )
            {
            case '/':
                oid = 0;
                state = vppRelPath;
                break;

            case '?':
                VPathCaptureOid ( self, oid, uri, anchor, oid_anchor, i );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCaptureOid ( self, oid, uri, anchor, oid_anchor, i );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                oid = 0;
                state = vppNamePath;
            }
            break;

        case vppAccDotRel:

            if ( ch >= 128 || isalpha ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isdigit ( ch ) )
            {
                ++ acc_ext;
                state = vppAccExtRel;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppAccUnderRel:

            if ( ch >= 128 || isdigit ( ch ) )
            {
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            else if ( isalpha ( ch ) )
            {
                ++ acc_suffix;
                state = vppAccSuffixRel;
            }
            else switch ( ch )
            {
            case '/':
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppRelPath;
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            default:
                acc_prefix = acc_alpha = acc_digit = acc_ext = 0;
                state = vppNamePath;
            }
            break;

        case vppSlash:

            switch ( ch )
            {
            case '/':
                if ( self -> scheme_type == vpuri_ncbi_file )
                    state = vppUNCOrMalformedPOSIXPath;
                else
                    state = vppAuthHostSpec;
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '?':
                VPathCapturePath ( self, uri, anchor, i, count, vpFullPath );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCapturePath ( self, uri, anchor, i, count, vpFullPath );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            default:
                state = vppFullPath;
            }
            break;

        case vppAuthHostSpec:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            VPathParseResetAnchor ( i );

            if ( isalpha ( ch ) )
                state = vppAuthHostNamePort;
            else if ( isdigit ( ch ) )
            {
                ipv4 [ ip = 0 ] = ch - '0';
                state = vppIPv4Port;
            }
            else switch ( ch )
            {
            case '/':
                state = vppFullPath;
                break;
            case '[':
                ip = 0;
                memset ( ipv6, 0, sizeof ipv6 );
                state = vppIPv6Colon;
                break;
            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }

            break;

        case vppHostSpec:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            VPathParseResetAnchor ( i );

            if ( isalpha ( ch ) )
                state = vppHostNamePort;
            else if ( isdigit ( ch ) )
            {
                ipv4 [ ip = 0 ] = ch - '0';
                state = vppIPv4Port;
            }
            else switch ( ch )
            {
            case '/':
                state = vppFullPath;
                break;
            case '[':
                ip = 0;
                memset ( ipv6, 0, sizeof ipv6 );
                state = vppIPv6Colon;
                break;
            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }

            break;

        case vppAuthHostNamePort:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            if ( isalnum ( ch ) )
                break;
            else switch ( ch )
            {
            case '@':
                VPathCaptureAuth ( self, uri, anchor, i );
                state = vppHostSpec;
                break;

            case '.':
            case '+':
            case '-':
            case '_':
                break;

            case ':':
                VPathCaptureHostName ( self, uri, anchor, i );
                state = vppPortSpec;
                break;

            case '/':
                VPathCaptureHostName ( self, uri, anchor, i );
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }
            break;

        case vppHostNamePort:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            if ( isalnum ( ch ) )
                break;
            else switch ( ch )
            {
            case '.':
            case '+':
            case '-':
            case '_':
                break;

            case ':':
                VPathCaptureHostName ( self, uri, anchor, i );
                state = vppPortSpec;
                break;

            case '/':
                VPathCaptureHostName ( self, uri, anchor, i );
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }
            break;

        case vppIPv4Port:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            if ( ipv4 [ ip ] >= 256 )
                return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );
            if ( isdigit ( ch ) )
                ipv4 [ ip ] = ipv4 [ ip ] * 10 + ch - '0';
            else if ( ++ ip == 4 )
            {
                switch ( ch )
                {
                case ':':
                    rc = VPathCaptureIPv4 ( self, ipv4 );
                    state = vppPortSpec;
                    break;

                case '/':
                    rc = VPathCaptureIPv4 ( self, ipv4 );
                    state = vppFullPath;
                    VPathParseResetAnchor ( i );
                    break;

                default:
                    return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
                }

                if ( rc != 0 )
                    return rc;
            }
            else if ( ch == '.' )
                state = vppIPv4Dot;
            else
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            break;

        case vppIPv4Dot:

            if ( ch >= 128 || ! isdigit ( ch ) )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            ipv4 [ ip ] = ch - '0';
            state = vppIPv4Port;

            break;

        case vppIPv6Port:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            if ( ipv6 [ ip ] >= 0x10000 )
                return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );
            if ( isdigit ( ch ) )
                ipv6 [ ip ] = ( ipv6 [ ip ] << 4 ) + ch - '0';
            else if ( isxdigit ( ch ) )
                ipv6 [ ip ] = ( ipv6 [ ip ] << 4 ) + toupper ( ch ) - 'A' + 10;
            else
            {
                switch ( ch )
                {
                case ']':
                    rc = VPathCaptureIPv6 ( self, ipv6 );
                    state = vppPortSpecOrFullPath;
                    break;

                case ':':
                    if ( ++ ip != 8 )
                    {
                        state = vppIPv6Colon;
                        break;
                    }

                    /* no break */
                default:
                    return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
                }

                if ( rc != 0 )
                    return rc;
            }
            break;

        case vppIPv6Colon:

            if ( ch != ':' )
            {
                if ( ch >= 128 || ! isxdigit ( ch ) )
                    return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

                if ( isdigit ( ch ) )
                    ipv6 [ ip ] = ch - '0';
                else
                    ipv6 [ ip ] = toupper ( ch ) - 'A' + 10;
            }

            state = vppIPv6Port;

            break;

        case vppPortSpecOrFullPath:

            switch ( ch )
            {
            case ':':
                state = vppPortSpec;
                break;

            case '/':
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }

            break;

        case vppPortSpec:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            VPathParseResetAnchor ( i );

            if ( self -> scheme_type == vpuri_fasp ) {
                self -> missing_port = true;
                state = vppFullPath;
            }
            else if ( isalpha ( ch ) )
                state = vppPortName;
            else if ( isdigit ( ch ) )
            {
                port = ch - '0';
                state = vppPortNum;
            }
            else switch ( ch )
            {
            case '/':
                self -> missing_port = true;
                state = vppFullPath;
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }

            break;

        case vppPortName:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            if ( isalnum ( ch ) )
                break;
            else switch ( ch )
            {
            case '/':
                VPathCapturePortName ( self, uri, anchor, i );
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }
            break;

        case vppPortNum:

            if ( ch >= 128 )
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            if ( port >= 0x10000 )
                return RC ( rcVFS, rcPath, rcParsing, rcData, rcExcessive );

            if ( isdigit ( ch ) )
                port = port * 10 + ch - '0';
            else switch ( ch )
            {
            case '/':
                rc = VPathCapturePortNum ( self, port );
                if ( rc != 0 )
                    return rc;
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            default:
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }
            break;

        case vppNamePath:

            switch ( ch )
            {
            case '/':
                state = vppRelPath;
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '?':
                VPathCapturePath ( self, uri, anchor, i, count, vpName );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCapturePath ( self, uri, anchor, i, count, vpName );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;
            }

            break;

        case vppUNCOrMalformedPOSIXPath:

            switch ( ch )
            {
            case '/':
                state = vppFullPath;
                VPathParseResetAnchor ( i );
                break;

            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '?':
                VPathCapturePath ( self, uri, anchor, i, count, vpFullPath );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCapturePath ( self, uri, anchor, i, count, vpFullPath );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;

            default:
                state = vppUNCPath;
            }

            break;

        case vppFullOrUNCPath:

            if ( ch == '/' )
            {
                state = vppUNCOrMalformedPOSIXPath;
                break;
            }

            state = vppFullPath;
            /* no break */

        case vppRelPath:
        case vppFullPath:

            if(self->scheme_type != vpuri_none) switch ( ch )
            {
            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '?':
                VPathCapturePath ( self, uri, anchor, i, count,
                    ( state == vppRelPath ) ? vpRelPath : vpFullPath );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCapturePath ( self, uri, anchor, i, count,
                    ( state == vppRelPath ) ? vpRelPath : vpFullPath );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;
            }

            break;

        case vppUNCPath:

            switch ( ch )
            {
            case ':':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '?':
                VPathCapturePath ( self, uri, anchor, i, count, vpUNCPath );
                state = vppParamName;
                VPathParseResetAnchor ( i );
                break;

            case '#':
                VPathCapturePath ( self, uri, anchor, i, count, vpUNCPath );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;
            }

            break;

        case vppParamName:

            switch ( ch )
            {
            case ':':
            case '?':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '=':
                state = vppParamValue;
                break;

            case '#':
                VPathCaptureQuery ( self, uri, anchor, i, count );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;
            }

            break;

        case vppParamValue:

            switch ( ch )
            {
            case ':':
            case '?':
            case '=':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );

            case '&':
                state = vppParamName;
                break;

            case '#':
                VPathCaptureQuery ( self, uri, anchor, i, count );
                state = vppFragment;
                VPathParseResetAnchor ( i );
                break;
            }

            break;

        case vppFragment:

            switch ( ch )
            {
            case ':':
            case '?':
            case '#':
                return RC ( rcVFS, rcPath, rcParsing, rcChar, rcUnexpected );
            }

            break;
        }
    }

    /* return pileup extension back */
    if ( pileup_ext_present )
    {
        uri[uri_size] = '.';
        if ( i == uri_size ) {
            i += pileup_ext_size;
            count += pileup_ext_size;
        }
        uri_size += pileup_ext_size;
        
        if ( acc_alpha && acc_digit )
            ++acc_ext;

        self->sraClass = eSCpileup;
    }

    /* return realign extension back */
    else if (realign_ext_present)
    {
        uri[uri_size] = '.';
        if (i == uri_size) {
            i += realign_ext_size;
            count += realign_ext_size;
        }
        uri_size += realign_ext_size;

        if (acc_alpha && acc_digit)
            ++acc_ext;

        self->sraClass = eSCrealign;
    }
    /* record dbcache type */
    else if (vdbcache_ext_present)
    {
        self->sraClass = eSCvdbcache;
    }

    switch ( state )
    {
    case vppStart:
        return RC ( rcVFS, rcPath, rcParsing, rcString, rcEmpty );
    case vppAccPrefixAlphaNamePathOrScheme:
    case vppAccAlphaNamePath:
    case vppAccDigitNamePathOrScheme:
    case vppAccDigitNamePath:
    case vppAccExtNamePathOrScheme:
    case vppAccExtNamePath:
    case vppAccSuffixNamePath:
        VPathCaptureAccession ( self, uri, anchor, i );
        VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
        break;
    case vppAccDotNamePathOrScheme:
    case vppAccDotNamePath:
    case vppAccUnderNamePath:
    case vppNamePathOrScheme:
        VPathCapturePath ( self, uri, anchor, i, count, vpName );
        break;
    case vppAccOidRelOrSlash:
        return RC ( rcVFS, rcPath, rcParsing, rcData, rcInsufficient );
    case vppAccPrefixAlphaRel:
    case vppAccAlphaRel:
    case vppAccDigitRel:
    case vppAccExtRel:
    case vppAccSuffixRel:
        VPathCaptureAccession ( self, uri, anchor, i );
        VPathCaptureAccCode ( self, acc_prefix, acc_alpha, acc_digit, acc_ext, acc_suffix );
        break;
    case vppOidRel:
        VPathCaptureOid ( self, oid, uri, anchor, oid_anchor, i );
        break;
    case vppAccDotRel:
    case vppAccUnderRel:
    case vppSlash:
    case vppAuthHostSpec:
    case vppHostSpec:
        return RC ( rcVFS, rcPath, rcParsing, rcData, rcInsufficient );
    case vppAuthHostNamePort:
    case vppHostNamePort:
        VPathCaptureHostName ( self, uri, anchor, i );
        break;
    case vppIPv4Port:
        if ( ++ ip == 4 )
            return VPathCaptureIPv4 ( self, ipv4 );
    case vppIPv4Dot:
    case vppIPv6Port:
    case vppIPv6Colon:
    case vppPortSpecOrFullPath:
    case vppPortSpec:
        return RC ( rcVFS, rcPath, rcParsing, rcData, rcInsufficient );
    case vppPortName:
        VPathCapturePortName ( self, uri, anchor, i );
        break;
    case vppPortNum:
        return VPathCapturePortNum ( self, port );
    case vppNamePath:
        VPathCapturePath ( self, uri, anchor, i, count, vpName );
        break;
    case vppRelPath:
        VPathCapturePath ( self, uri, anchor, i, count, vpRelPath );
        break;
    case vppUNCOrMalformedPOSIXPath:
    case vppFullOrUNCPath:
    case vppFullPath:
        VPathCapturePath ( self, uri, anchor, i, count, vpFullPath );
        break;
    case vppUNCPath:
        VPathCapturePath ( self, uri, anchor, i, count, vpUNCPath );
        break;
    case vppParamName:
    case vppParamValue:
        VPathCaptureQuery ( self, uri, anchor, i, count );
        break;
    case vppFragment:
        VPathCaptureFragment ( self, uri, anchor, i, count );
        break;
    }

    return 0;
}

static
rc_t VPathParse ( VPath * self, char * uri, size_t uri_size )
{
    /* Parse uri as UTF-8 */
    rc_t rc = VPathParseInt ( self, uri, uri_size, true );

    if ( rc == SILENT_RC ( rcVFS, rcPath, rcParsing, rcChar, rcInvalid ) /* ||
        rc == SILENT_RC ( rcVFS, rcPath, rcParsing, rcData, rcInsufficient ) */
       )
    {
        /* Parse uri as extended ASCII */
        rc = VPathParseInt ( self, uri, uri_size, false );
    }

    return rc;
}

/* MakeFromText
 *  could be anything...
 */
static
rc_t VPathMakeFromVText ( VPath ** ppath, const char * path_fmt, va_list args )
{
    rc_t rc;
    KDataBuffer buffer;

    /* initialize buffer */
    memset ( & buffer, 0, sizeof buffer );
    buffer . elem_bits = 8;

    /* print into buffer */
    rc = KDataBufferVPrintf ( & buffer, path_fmt, args );
    if ( rc == 0 )
    {
        VPath * path = calloc ( 1, sizeof * path );
        if ( path == NULL )
            rc = RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );
        else
        {
            /* straight copy to steal contents */
            path -> data = buffer;

            /* parse into portions */
            rc = VPathParse ( path, buffer . base, ( size_t ) buffer . elem_count - 1 );
            if ( rc == 0 )
            {
                path->projectId = -1; /* public by default; 0 is valid id */

                KRefcountInit ( & path -> refcount, 1, "VPath", "make-from-text", buffer . base );
                * ppath = path;
                return 0;
            }

            free ( path );
        }

        KDataBufferWhack ( & buffer );
    }

    * ppath = NULL;

    return rc;
}

static
rc_t VPathMakeFromText ( VPath ** ppath, const char * text, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, text );

    rc = VPathMakeFromVText ( ppath, text, args );

    va_end ( args );

    return rc;
}

/* MakePathWithExtension
 *  makes a copy of original path
 *  appends an extension to path portion of orig
 */
LIB_EXPORT rc_t CC VFSManagerMakePathWithExtension ( struct VFSManager const * self,
    VPath ** new_path, const VPath * orig, const char * extension )
{
    rc_t rc;

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcPath, rcCopying, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcPath, rcCopying, rcSelf, rcNull );
        else if ( orig == NULL )
            rc = RC ( rcVFS, rcPath, rcCopying, rcParam, rcNull );
        else if ( orig -> path_type == vpInvalid || orig -> path_type > vpEndpoint )
            rc = RC ( rcVFS, rcPath, rcCopying, rcParam, rcInvalid );
        else if ( orig -> path_type > vpFullPath )
            rc = RC ( rcVFS, rcPath, rcCopying, rcParam, rcIncorrect );
#if ALLOW_ADDREF_WITHIN_MAKEPATH
        else if ( extension == NULL || extension [ 0 ] == 0 )
        {
            rc = VPathAddRef ( orig );
            if ( rc == 0 )
            {
                * new_path = ( VPath * ) orig;
                return 0;
            }
        }
#endif
        else
        {
            char path_prefix [ 4096 ];
            size_t num_writ, total = 0;

            rc = 0;

#if ! ALLOW_ADDREF_WITHIN_MAKEPATH
            if ( extension == NULL )
                extension = "";
#endif

            /* add in original scheme */
            if ( orig -> from_uri && orig -> scheme . size != 0 )
            {
                rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                    & num_writ, "%S://", & orig -> scheme );
                if ( rc == 0 )
                    total += num_writ;

                /* if there's a host spec of some sort */
                if ( rc == 0 && orig -> host . size != 0 )
                {
                    /* add in original auth */
                    if ( orig -> auth . size != 0 )
                    {
                        rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                            & num_writ, "%S@", & orig -> auth );
                        if ( rc == 0 )
                            total += num_writ;
                    }

                    /* add in host */
                    if ( rc == 0 )
                    {
                        rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                            & num_writ, "%S", & orig -> host );
                        if ( rc == 0 )
                            total += num_writ;
                    }

                    /* add in port */
                    if ( rc == 0 )
                    {
                        num_writ = 0;
                        if ( orig -> portname . size != 0 )
                        {
                            rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                                & num_writ, ":%S", & orig -> portname );
                        }
                        else if ( orig -> portnum != 0 )
                        {
                            rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                                & num_writ, ":%u", orig -> portnum );
                        }
                        else if ( orig -> missing_port )
                        {
                            rc = string_printf ( & path_prefix [ total ], sizeof path_prefix - total,
                                & num_writ, ":" );
                        }
                        if ( rc == 0 )
                            total += num_writ;
                    }
                }
            }

            if ( rc == 0 )
            {
                /* copy orig */
                switch ( orig -> path_type )
                {
                case vpOID:

                    if ( total != 0 )
                    {
                        rc = VPathMakeFromText ( new_path
                                                 , "%.*s/%u%s%S%S"
                                                 , ( uint32_t ) total, path_prefix
                                                 , orig -> obj_id
                                                 , extension
                                                 , & orig -> query
                                                 , & orig -> fragment
                            );
                    }
                    else
                    {
                        rc = VPathMakeFromText ( new_path
                                                 , "%u%s%S%S"
                                                 , orig -> obj_id
                                                 , extension
                                                 , & orig -> query
                                                 , & orig -> fragment
                            );
                    }
                    break;

                case vpAccession:
                case vpNameOrOID:
                case vpNameOrAccession:
                case vpName:
                case vpRelPath:
                case vpUNCPath:

                    /* no break */

                case vpFullPath:

                    assert ( orig -> path . size != 0 );
                    if ( total != 0 )
                    {
                        rc = VPathMakeFromText ( new_path
                                                 , "%.*s%S%s%S%S"
                                                 , ( uint32_t ) total, path_prefix
                                                 , & orig -> path
                                                 , extension
                                                 , & orig -> query
                                                 , & orig -> fragment
                            );
                    }
                    else
                    {
                        rc = VPathMakeFromText ( new_path
                                                 , "%S%s%S%S"
                                                 , & orig -> path
                                                 , extension
                                                 , & orig -> query
                                                 , & orig -> fragment
                            );
                    }
                    break;
                }

                if ( rc == 0 )
                    return 0;
            }
        }

        * new_path = NULL;
    }

    return rc;
}


/* ExtractAccessionOrOID
 *  given an arbitrary path, possibly with extensions,
 *  extract the portion of the leaf qualifying as an
 *  accession or OID
 */
LIB_EXPORT rc_t CC VFSManagerExtractAccessionOrOID ( const VFSManager * self,
    VPath ** acc_or_oid, const VPath * orig )
{
    rc_t rc;

    if ( acc_or_oid == NULL )
        rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcPath, rcConstructing, rcSelf, rcNull );
        else if ( orig == NULL )
            rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcNull );
        else if ( VPathIsAccessionOrOID ( orig ) )
        {
            rc = VPathAddRef ( orig );
            if ( rc == 0 )
            {
                * acc_or_oid = ( VPath* ) orig;
                return 0;
            }
        }
        else
        {
            String path = orig -> path;
            const char * sep, * start = path . addr;
            const char * end = path . addr + path . size;

            switch ( orig -> path_type )
            {
            case vpInvalid:
                rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcInvalid );
                break;

            case vpName:
                break;

            case vpRelPath:
            case vpUNCPath:
            case vpFullPath:
                sep = string_rchr ( start, path . size, '/' );
                if ( sep != NULL )
                    start = sep + 1;
                break;

            default:
                rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcIncorrect );
            }

            /* strip off known extensions */
            while ( 1 )
            {
                sep = string_rchr ( start, end - start, '.' );
                if ( sep == NULL )
                    break;

                switch ( end - sep )
                {
                case 4:
                    if ( strcase_cmp ( ".sra", 4, sep, 4, 4 ) == 0 ||
                         strcase_cmp ( ".wgs", 4, sep, 4, 4 ) == 0 )
                    {
                        end = sep;
                        continue;
                    }
                case 9:
                    if ( strcase_cmp ( ".vdbcache", 9, sep, 9, 9 ) == 0 ||
                         strcase_cmp ( ".ncbi_enc", 9, sep, 9, 9 ) == 0 )
                    {
                        end = sep;
                        continue;
                    }
                    break;
                }
                break;
            }

            /* create a new VPath */
            rc = VPathMakeFromText ( acc_or_oid, "%.*s", ( uint32_t ) ( end - start ), start );
            if ( rc == 0 )
            {
                const VPath * vpath = * acc_or_oid;
                if ( VPathIsAccessionOrOID ( vpath ) )
                    return 0;

                VPathRelease ( vpath );

                rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcIncorrect );
            }
        }

        * acc_or_oid = NULL;
    }

    return rc;
}



/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC VPathAddRef ( const VPath *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VPath" ) )
        {
        case krefLimit:
            return RC ( rcVFS, rcPath, rcAttaching, rcRange, rcExcessive );
        }
    }

    return 0;
}

LIB_EXPORT rc_t CC VPathRelease ( const VPath *self )
{
    rc_t rc = 0;

    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VPath" ) )
        {
        case krefWhack:
            rc = VPathWhack ( ( VPath* ) self );
            break;
        case krefNegative:
            return RC ( rcVFS, rcPath, rcReleasing, rcRange, rcExcessive );
        }
    }

    return rc;
}


/* IsAcessionOrOID
 *  asks if the path pattern could possibly be an accession or oid
 */
LIB_EXPORT bool CC VPathIsAccessionOrOID ( const VPath * self )
{
    if ( self != NULL )
    {
        switch ( self -> path_type )
        {
        case vpOID:
        case vpAccession:
        case vpNameOrOID:
        case vpNameOrAccession:
            return true;
        }
    }
    return false;
}


/* IsFSCompatible
 *  asks if the path can be used with the OS' filesystems
 */
LIB_EXPORT bool CC VPathIsFSCompatible ( const VPath * self )
{
    if ( self != NULL )
    {
        switch ( self -> path_type )
        {
        case vpNameOrAccession:
        case vpName:
        case vpRelPath:
        case vpUNCPath:
        case vpFullPath:
            return true;
        }
    }
    return false;
}


/* FromUri
 *  asks if the path was created from a formal URI
 */
LIB_EXPORT bool CC VPathFromUri ( const VPath * self )
{
    if ( self != NULL )
        return self -> from_uri;
    return false;
}


/* Read*
 *  read the various parts
 *  copies out data into user-supplied buffer
 */
static
rc_t VPathSetInvalidReturn ( rc_t rc,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    if ( num_read != NULL )
        * num_read = 0;
    if ( buffer != NULL && buffer_size != 0 )
        buffer [ 0 ] = 0;
    return rc;
}

static
rc_t VPathReadTestSelf ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVFS, rcPath, rcReading, rcSelf, rcNull );
    else if ( self -> path_type == vpInvalid )
        rc = RC ( rcVFS, rcPath, rcReading, rcSelf, rcInvalid );
    else
        return 0;

    return VPathSetInvalidReturn ( rc, buffer, buffer_size, num_read );
}

static
rc_t VPathReadMember ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read,
    const String * mbr, uint32_t idx )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
        rc = string_printf ( buffer, buffer_size, num_read, "%:*-$S", idx, mbr );
    return rc;
}

static
rc_t VPathGetSchemeInt ( const VPath * self, String * scheme )
{
    if ( self -> scheme . size != 0 )
        * scheme = self -> scheme;

    else switch ( self -> path_type )
    {
    case vpOID:
        CONST_STRING ( scheme, "ncbi-obj" );
        break;
    case vpAccession:
        CONST_STRING ( scheme, "ncbi-acc" );
        break;
    case vpNameOrOID:
    case vpNameOrAccession:
    case vpName:
    case vpRelPath:
    case vpFullPath:
        if ( self -> query . size != 0 || self -> fragment . size != 0 )
            CONST_STRING ( scheme, "ncbi-file" );
        else
            CONST_STRING ( scheme, "file" );
        break;
    case vpUNCPath:
        CONST_STRING ( scheme, "ncbi-file" );
        break;
    default:
        return RC ( rcVFS, rcPath, rcReading, rcType, rcIncorrect );
    }

    return 0;
}

static
rc_t VPathReadSchemeInt ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read,
    const char * suffix, bool * fail )
{
    String scheme;
    rc_t rc = VPathGetSchemeInt ( self, & scheme );
    if ( rc != 0 )
    {
        * fail = true;
        return rc;
    }
    return string_printf ( buffer, buffer_size, num_read, "%S%s", & scheme, suffix );
}

static
rc_t VPathReadAuthInt ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read,
    const char * prefix )
{
    rc_t rc = 0;

    if ( self -> auth . size == 0 )
        * num_read = 0;
    else
    {
        rc = string_printf ( buffer, buffer_size, num_read,
                             "%s%S", prefix, & self -> auth );
    }

    return rc;
}

static
rc_t VPathReadHostInt ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read,
    const char * prefix )
{
    rc_t rc = 0;
    const char * lbrack, * rbrack;

    switch ( self -> host_type )
    {
    case vhDNSName:

        rc = string_printf ( buffer, buffer_size, num_read,
                             "%s%S", prefix, & self -> host );
        break;

    case vhIPv4:

        rc = string_printf ( buffer, buffer_size, num_read
                             , "%s%u.%u.%u.%u"
                             , prefix
                             , ( self -> ipv4 >> 24 ) & 0xFF
                             , ( self -> ipv4 >> 16 ) & 0xFF
                             , ( self -> ipv4 >>  8 ) & 0xFF
                             , ( self -> ipv4 >>  0 ) & 0xFF
            );
        break;

    case vhIPv6:

        lbrack = rbrack = "";
        if ( prefix [ 0 ] != 0 )
        {
            lbrack = "[";
            rbrack = "]";
        }

        rc = string_printf ( buffer, buffer_size, num_read
                             , "%s%s%u:%u:%u:%u:%u:%u:%u:%u%s"
                             , prefix
                             , lbrack
                             , self -> ipv6 [ 0 ]
                             , self -> ipv6 [ 1 ]
                             , self -> ipv6 [ 2 ]
                             , self -> ipv6 [ 3 ]
                             , self -> ipv6 [ 4 ]
                             , self -> ipv6 [ 5 ]
                             , self -> ipv6 [ 6 ]
                             , self -> ipv6 [ 7 ]
                             , rbrack
            );
        break;
    }

    return rc;
}

static
rc_t VPathReadPathInt ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc;

    /* print remainder */
    switch ( self -> path_type )
    {
    case vpOID:
            
        rc = string_printf ( buffer, buffer_size, num_read
                             , "%u"
                             , self -> obj_id
            );
        break;

    case vpAccession:
    case vpNameOrOID:
    case vpNameOrAccession:
    case vpName:
    case vpRelPath:
    case vpUNCPath:
    case vpFullPath:

        assert ( self -> path . size != 0 );
        rc = string_printf ( buffer, buffer_size, num_read
                             , "%S"
                             , & self -> path
            );
        break;

    default:

        if ( num_read != NULL )
            * num_read = 0;

        rc = 0;
    }

    return rc;
}

static
rc_t VPathReadUriInt ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * total_read )
{
    rc_t rc;
    const char * host_prefix;
    size_t total, num_read, avail;
    bool has_auth, has_host, fail = false;

    /* print scheme plus preparation for host */
    rc = VPathReadSchemeInt ( self, buffer, buffer_size, & num_read, ":", & fail );
    if ( fail )
        return VPathSetInvalidReturn ( rc, buffer, buffer_size, total_read );
    total = num_read;
    avail = ( rc == 0 ) ? buffer_size - total : 0;

    /* print auth */
    host_prefix = "//";
    rc = VPathReadAuthInt ( self, & buffer [ total ], avail, & num_read, host_prefix );
    total += num_read;
    avail -= ( rc == 0 ) ? num_read : 0;
    has_auth = num_read > 2;
    if ( has_auth )
        host_prefix = "@";

    /* print host */
    rc = VPathReadHostInt ( self, & buffer [ total ], avail, & num_read, host_prefix );
    total += num_read;
    avail -= ( rc == 0 ) ? num_read : 0;
    has_host = num_read > ( 2U - has_auth );

    /* print port */
    if ( has_host )
    {
        if ( self -> portname . size != 0 )
            rc = string_printf ( & buffer [ total ], avail, & num_read, ":%S", & self -> portname );
        else if ( self -> portnum != 0 )
            rc = string_printf ( & buffer [ total ], avail, & num_read, ":%u", self -> portnum );
        else if ( self -> missing_port )
            rc = string_printf ( & buffer [ total ], avail, & num_read, ":" );
        else
            num_read = 0;

        total += num_read;
        avail -= ( rc == 0 ) ? num_read : 0;
    }

    /* sanity check */
    assert ( ! has_auth || has_host );

    assert ( self -> path . size == 0  || ! has_host ||
       ( self -> path . addr [ 0 ] == '/' || self -> scheme_type == vpuri_fasp )
      );

    assert ( self -> query . size == 0 || self -> query . addr [ 0 ] == '?' );
    assert ( self -> fragment . size == 0 || self -> fragment . addr [ 0 ] == '#' );

    /* print remainder */
    switch ( self -> path_type )
    {
    case vpOID:

        if ( ! has_host )
        {
            /* correct for empty host spec */
            total -= 2;
            avail += 2;
        }

        rc = string_printf ( & buffer [ total ], avail, & num_read
                             , "%s%u%S%S"
                             , has_host ? "/" : ""
                             , self -> obj_id
                             , & self -> query
                             , & self -> fragment
            );
        break;

    case vpAccession:
    case vpNameOrOID:
    case vpNameOrAccession:
    case vpName:
    case vpRelPath:
    case vpUNCPath:

        assert ( ! has_host );

        /* correct for empty host spec */
        total -= 2;
        avail += 2;

        /* no break */

    case vpFullPath:

        assert ( self -> path . size != 0 );
        rc = string_printf ( & buffer [ total ], avail, & num_read
                             , "%S%S%S"
                             , & self -> path
                             , & self -> query
                             , & self -> fragment
            );
        break;
    }

    if ( total_read != NULL )
        * total_read = total + num_read;

    if ( rc != 0 && buffer != NULL && buffer_size != 0 )
        buffer [ 0 ] = 0;

    return rc;
}

LIB_EXPORT rc_t CC VPathReadUri ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
        rc = VPathReadUriInt ( self, buffer, buffer_size, num_read );
    return rc;
}

LIB_EXPORT rc_t CC VPathReadScheme ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
    {
        bool fail = false;
        rc = VPathReadSchemeInt ( self, buffer, buffer_size, num_read, "", & fail );
    }
    return rc;
}

LIB_EXPORT rc_t CC VPathReadAuth ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
        rc = VPathReadAuthInt ( self, buffer, buffer_size, num_read, "" );
    return rc;
}

LIB_EXPORT rc_t CC VPathReadHost ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
        rc = VPathReadHostInt ( self, buffer, buffer_size, num_read, "" );
    return rc;
}

LIB_EXPORT rc_t CC VPathReadPortName ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    return VPathReadMember ( self,
        buffer, buffer_size, num_read,
        & self -> portname, 0 );
}

LIB_EXPORT rc_t CC VPathReadPath ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
        rc = VPathReadPathInt ( self, buffer, buffer_size, num_read );
    return rc;
}

LIB_EXPORT rc_t CC VPathReadQuery ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    return VPathReadMember ( self,
        buffer, buffer_size, num_read,
        & self -> query, 1 );
}

static
rc_t VPathFindParam ( const VPath * self, const char * param, String * val_str )
{
    rc_t rc;

    if ( param == NULL )
        rc = RC ( rcVFS, rcPath, rcReading, rcParam, rcNull );
    else if ( param [ 0 ] == 0 )
        rc = RC ( rcVFS, rcPath, rcReading, rcParam, rcEmpty );
    else
    {
        size_t qsize = string_size ( param );
        const char * start = self -> query . addr;
        const char * end = start + self -> query . size;
        if ( ++ start < end ) while ( 1 )
        {
            /* match as many characters as possible of param name */
            uint32_t matching = strcase_match ( start, end - start, param, qsize, -1, NULL );
            start += matching;

            if ( ( size_t ) matching == qsize )
            {
                /* may have found the parameter */
                if ( start == end || start [ 0 ] == '&' )
                {
                    /* no value */
                    StringInit ( val_str, "", 0, 0 );
                    return 0;
                }

                if ( start [ 0 ] == '=' )
                {
                    /* have value */
                    const char * val = start + 1;
                    const char * pend = string_chr ( val, end - val, '&' );
                    if ( pend == NULL )
                        pend = end;
                    StringInit ( val_str, val, pend - val, string_len ( val, pend - val ) );
                    return 0;
                }
            }

            /* find next parameter */
            start = string_chr ( start, end - start, '&' );
            if ( start == NULL )
                break;

            /* skip separator */
            ++ start;
        }

        rc = SILENT_RC ( rcVFS, rcPath, rcReading, rcParam, rcNotFound );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathReadParam ( const VPath * self, const char * param,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
    {
        String val_str;
        rc = VPathFindParam ( self, param, & val_str );
        if ( rc == 0 )
        {
            /* copy out value */
            return string_printf ( buffer, buffer_size, num_read
                                   , "%S"
                                   , & val_str
                );
        }

        VPathSetInvalidReturn ( rc, buffer, buffer_size, num_read );
    }
    return rc;
}

LIB_EXPORT rc_t CC VPathReadFragment ( const VPath * self,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    return VPathReadMember ( self,
        buffer, buffer_size, num_read,
        & self -> fragment, 1 );
}


/* MakeUri
 *  convert a VPath into a Uri
 */
LIB_EXPORT rc_t CC VPathMakeUri ( const VPath * self, const String ** uri )
{
    rc_t rc;

    if ( uri == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        /* TBD - if we build the path from a URI, we should just
           grab the text from buffer */
        size_t bytes;
        char buffer [ 8192 ];
        rc = VPathReadUri ( self, buffer, sizeof buffer, & bytes );
        if ( rc == 0 )
        {
            String uri_str;
            StringInit ( & uri_str, buffer, bytes, string_len ( buffer, bytes ) );
            return StringCopy ( uri, & uri_str );
        }

        /* TBD - if we ever have huge URIs, 8K can be insufficient */
        * uri = NULL;
    }
    return rc;
}


/* MakeSysPath
 *  convert a VPath into an OS-specific path string
 */
LIB_EXPORT rc_t CC VPathMakeSysPath ( const VPath * self, const String ** sys_path )
{
    rc_t rc;

    if ( sys_path == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        size_t bytes;
        char buffer [ 8192 ];
        rc = VPathReadSysPath ( self, buffer, sizeof buffer, & bytes );
        if ( rc == 0 )
        {
            String sys_str;
            StringInit ( & sys_str, buffer, bytes, string_len ( buffer, bytes ) );
            return StringCopy ( sys_path, & sys_str );
        }

        * sys_path = NULL;
    }
    return rc;
}


/* MakeString
 *  convert a VPath into a String
 *  respects original source of path,
 *  i.e. does not add scheme unnecessarily
 */
LIB_EXPORT rc_t CC VPathMakeString ( const VPath * self, const String ** str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcPath, rcAccessing, rcSelf, rcNull );
        else
        {
            size_t bytes, host;
            char buffer [ 8192 ];

            if ( self -> from_uri || self -> query . size != 0 || self -> fragment . size != 0 )
                rc = VPathReadUriInt ( self, buffer, sizeof buffer, & bytes );
            else switch ( self -> path_type )
            {
            case vpHostName:
                rc = VPathReadHostInt ( self, buffer, sizeof buffer, & bytes, "" );
                break;

            case vpEndpoint:
                rc = VPathReadHostInt ( self, buffer, sizeof buffer, & host, "" );
                if ( rc == 0 )
                {
                    if ( self -> portname . size != 0 )
                    {
                        rc = string_printf ( & buffer [ host ], sizeof buffer - host,
                            & bytes, ":%S", & self -> portname );
                    }
                    else
                    {
                        rc = string_printf ( & buffer [ host ], sizeof buffer - host,
                            & bytes, ":%u", & self -> portnum );
                    }

                    bytes += host;
                }
                break;

            default:
                rc = VPathReadPathInt ( self, buffer, sizeof buffer, & bytes );
            }

            if ( rc == 0 )
            {
                String local_str;
                StringInit ( & local_str, buffer, bytes, string_len ( buffer, bytes ) );
                return StringCopy ( str, & local_str );
            }
        }

        * str = NULL;
    }
    return rc;
}


/* Get*
 *  retrieves internal parts
 *  returns pointers to internal String data
 *  Strings remain valid while "self" is valid
 */
static
rc_t VPathGetTestSelf ( const VPath * self )
{
    if ( self == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcSelf, rcNull );
    if ( self -> path_type == vpInvalid )
        return RC ( rcVFS, rcPath, rcAccessing, rcSelf, rcInvalid );
    return 0;
}

LIB_EXPORT rc_t CC VPathGetScheme ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            /* work harder to fill out a scheme if not there */
            rc = VPathGetSchemeInt ( self, str );
            if ( rc == 0 )
                return 0;

            /* if the path can't have a scheme, it's okay */
            rc = 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetAuth ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> auth;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetHost ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> host;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetPortName ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> portname;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT uint16_t CC VPathGetPortNum ( const VPath * self )
{
    rc_t rc = VPathGetTestSelf ( self );
    if ( rc == 0 )
        return self -> portnum;

    return 0;
}

LIB_EXPORT rc_t CC VPathGetPath ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> path;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetQuery ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            StringSubstr ( & self -> query, str, 1, 0 );
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetParam ( const VPath * self, const char * param, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            rc = VPathFindParam ( self, param, str );
            if ( rc == 0 )
                return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetFragment ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            /* returning a NULL String when there is no fragment */
            StringInit ( str, NULL, 0, 0 );

            StringSubstr ( & self -> fragment, str, 1, 0 );
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

/* TEMPORARY */
LIB_EXPORT uint32_t CC VPathGetOid ( const VPath * self )
{
    if ( self != NULL )
        return self -> obj_id;
    return 0;
}


LIB_EXPORT rc_t CC VPathGetId ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> id;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetTicket ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> tick;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetService ( const VPath * self, String * str )
{
    rc_t rc;

    if ( str == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * str = self -> service;
            return 0;
        }

        StringInit ( str, "", 0, 0 );
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetType(const VPath * self, String * str)
{
    rc_t rc;

    if (str == NULL)
        rc = RC(rcVFS, rcPath, rcAccessing, rcParam, rcNull);
    else
    {
        rc = VPathGetTestSelf(self);
        if (rc == 0)
        {
            *str = self->type;
            return 0;
        }

        StringInit(str, "", 0, 0);
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetName(const VPath * self, String * str)
{
    rc_t rc;

    if (str == NULL)
        rc = RC(rcVFS, rcPath, rcAccessing, rcParam, rcNull);
    else
    {
        rc = VPathGetTestSelf(self);
        if (rc == 0)
        {
            *str = self->name;
            return 0;
        }

        StringInit(str, "", 0, 0);
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetNameExt(const VPath * self, String * str)
{
    rc_t rc;

    if (str == NULL)
        rc = RC(rcVFS, rcPath, rcAccessing, rcParam, rcNull);
    else
    {
        rc = VPathGetTestSelf(self);
        if (rc == 0)
        {
            *str = self->nameExtension;
            return 0;
        }

        StringInit(str, "", 0, 0);
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetObjectType(const VPath * self, String * str)
{
    rc_t rc;

    if (str == NULL)
        rc = RC(rcVFS, rcPath, rcAccessing, rcParam, rcNull);
    else
    {
        rc = VPathGetTestSelf(self);
        if (rc == 0)
        {
            *str = self->objectType;
            return 0;
        }

        StringInit(str, "", 0, 0);
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetCeRequired(const VPath * self, bool * required)
{
    rc_t rc;

    if (required == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * required = self -> ceRequired;
            return 0;
        }

        * required = false;
    }

    return rc;
}

LIB_EXPORT rc_t CC VPathGetPayRequired(const VPath * self, bool * required)
{
    rc_t rc;

    if (required == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        rc = VPathGetTestSelf ( self );
        if ( rc == 0 )
        {
            * required = self -> payRequired;
            return 0;
        }

        * required = false;
    }

    return rc;
}


LIB_EXPORT KTime_t CC VPathGetModDate ( const VPath * self  )
{
    if ( self != NULL )
        return self -> modification;
    return 0;
}

LIB_EXPORT uint64_t CC VPathGetSize ( const VPath * self )
{
    if ( self != NULL )
        return self -> osize;
    return 0;
}

LIB_EXPORT const uint8_t * CC VPathGetMd5 ( const VPath * self )
{
    if ( self == NULL )
        return NULL;
    if ( ! self -> has_md5 )
        return NULL;

    return self -> md5;
}


/* MarkHighReliability
 *  mark a path as representing either a reliable URL
 *  or one where the reliability is unknown.
 *
 *  a highly-reliable URL is one that should be expected
 *  to work. this property makes errors more suspicious,
 *  and more likely to be temporary, leading to harder work
 *  within the networking module.
 *
 *  "high_reliability" [ IN ] - set to true for high reliability
 */
LIB_EXPORT rc_t CC VPathMarkHighReliability ( VPath * self, bool high_reliability )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVFS, rcPath, rcUpdating, rcSelf, rcNull );
    else
    {
        self -> highly_reliable = high_reliability;
        rc = 0;
    }
    return rc;
}

/* IsHighlyReliable
 *  returns "true" if the path represents a high-reliability URL
 */
LIB_EXPORT bool CC VPathIsHighlyReliable ( const VPath * self )
{
    return self != NULL && self -> highly_reliable;
}


/*--------------------------------------------------------------------------
 * VFSManager
 */


/* MakePath
 *  make a path object from a string conforming to
 *  either a standard POSIX path or a URI
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "path_str" [ IN ] - a UTF-8 NUL-terminated string
 *  representing a POSIX path or URI, or
 *  a string_printf compatible format string
 *
 *  "path_fmt" [ IN ] and "args" [ IN ] - a UTF-8 NUL-terminated fmt string
 *  compatible with string_vprintf, plus argument list
 */
LIB_EXPORT rc_t CC VFSManagerVMakePath ( const VFSManager * self,
    VPath ** new_path, const char * path_fmt, va_list args )
{
    rc_t rc;

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcSelf, rcNull );
        else if ( path_fmt == NULL )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcNull );
        else if ( path_fmt [ 0 ] == 0 )
            rc = RC ( rcVFS, rcMgr, rcConstructing, rcPath, rcEmpty );
        else
        {
            rc = VPathMakeFromVText ( new_path, path_fmt, args );
            if ( rc == 0 )
            {
                VPath * path = * new_path;

                if ( ! path -> from_uri )
                {
                    String scheme;
                    rc = VPathGetSchemeInt ( path, & scheme );
                    if ( rc == 0 )
                        path -> scheme = scheme;
                }

                return 0;
            }
        }

        * new_path = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VFSManagerMakePath ( const VFSManager * self,
    VPath ** new_path, const char * path_str, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path_str );

    rc = VFSManagerVMakePath ( self, new_path, path_str, args );

    va_end ( args );

    return rc;
}

/* MakeAccPath - TEMPORARY
 *  takes a textual accession representation
 *  creates a VPath representing an accession
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "acc" [ IN ] - a NUL-terminated ASCII fmt string
 */
LIB_EXPORT rc_t CC VFSManagerVMakeAccPath ( const VFSManager * self,
    VPath ** new_path, const char * fmt, va_list args )
{
    rc_t rc = VFSManagerVMakePath ( self, new_path, fmt, args );
    if ( rc == 0 )
    {
        VPath * path = * new_path;
        switch ( path -> path_type )
        {
        case vpAccession:
            break;
        case vpNameOrAccession:
            path -> path_type = vpAccession;
            break;
        default:
            VPathRelease ( path );
            * new_path = NULL;
            return RC ( rcVFS, rcMgr, rcConstructing, rcToken, rcIncorrect );
        }

        if ( path -> scheme . size == 0 )
            VPathCaptureScheme ( path, "ncbi-acc", 0, 8 );
    }
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerMakeAccPath ( const VFSManager * self,
    VPath ** new_path, const char * acc, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, acc );

    rc = VFSManagerVMakeAccPath ( self, new_path, acc, args );

    va_end ( args );

    return rc;
}


/* MakeOidPath - TEMPORARY
 *  takes an integer oid
 *  creates a VPath representing an obj-id
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "oid" [ IN ] - a non-zero object id
 */
LIB_EXPORT rc_t CC VFSManagerMakeOidPath ( const VFSManager * self,
    VPath ** new_path, uint32_t oid )
{
    char oid_str [ 32 ];
    rc_t rc = string_printf ( oid_str, sizeof oid_str, NULL, "%u", oid );
    assert ( rc == 0 );

    rc = VFSManagerMakePath ( self, new_path, "%s", oid_str );
    if ( rc == 0 )
    {
        VPath * path = * new_path;
        switch ( path -> path_type )
        {
        case vpInvalid:
        case vpNameOrAccession:
        case vpName:
            break;
        default:
            VPathRelease ( path );
            * new_path = NULL;
            return RC ( rcVFS, rcMgr, rcConstructing, rcToken, rcIncorrect );
        }

        VPathCaptureScheme ( path, "ncbi-obj", 0, 8 );
        path -> obj_id = oid;
        path -> path_type = vpOID;
    }
    return rc;
}


/* ==========================================
             HACK O' MATIC
 */

#include <vfs/path-priv.h>
#include <vfs/manager-priv.h>
#include <kfs/directory.h>

/* MakeDirectoryRelative
 *  apparently the idea was to interpret "posix_path" against
 *  "dir" to come up with a stand-alone path that could be used
 *  without "dir" in the future. this is a total misunderstanding
 *  of the filesystem.
 *
 *  the old code would call create system path from the posix_path,
 *  which seems to make little sense on Windows... then it would
 *  decide whether "posix_path" was standalone or directory relative.
 */
static
rc_t LegacyVPathResolveAccession ( VPath ** new_path, const VPath * path )
{
    rc_t rc;
    VFSManager * mgr;

    * new_path = NULL;

    rc = VFSManagerMake ( & mgr );
    if ( rc == 0 )
    {
        VResolver * resolver;
        rc = VFSManagerGetResolver ( mgr, & resolver );
        if ( rc == 0 )
        {
            rc = VResolverLocal ( resolver, path, ( const VPath** ) new_path );
            if ( GetRCState ( rc ) == rcNotFound )
                rc = VResolverRemote ( resolver, 0, path, ( const VPath** ) new_path );

            VResolverRelease ( resolver );
        }

        VFSManagerRelease ( mgr );
    }

    return rc;
}

static
rc_t LegacyVPathMakeKDirRelative ( VPath ** new_path, const KDirectory * dir, const VPath * path )
{
    /* resolve the provided path against directory */
    char resolved [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( dir, true, resolved, sizeof resolved,
        "%.*s", ( uint32_t ) path -> path . size, path -> path . addr );
    if ( rc == 0 )
    {
        /* re-evaluate, adding in any query and fragment */
        rc = VPathMakeFromText ( new_path, "%s%S%S", resolved, & path -> query, & path -> fragment );
        if ( rc == 0 )
        {
            VPath * relpath = * new_path;

            /* reassess the scheme */
            String scheme;
            rc = VPathGetSchemeInt ( relpath, & scheme );
            if ( rc == 0 )
                relpath -> scheme = scheme;

            return 0;
        }
    }

    * new_path = NULL;
    return rc;
}

LIB_EXPORT rc_t CC VPathMakeDirectoryRelativeVPath ( VPath ** new_path,
    const KDirectory * dir, const char * posix_path, const VPath * vpath )
{
    rc_t rc = 0;

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( dir == NULL )
            rc = RC ( rcVFS, rcMgr, rcResolving, rcDirectory, rcNull );
        else
        {
            if ( vpath == NULL )
                /* first, try to get a VPath from "posix_path" */
                rc = LegacyVPathMakeFmt ( new_path, posix_path );
            else
                * new_path = ( VPath * ) vpath;
            if ( rc == 0 )
            {
                VPath * path = * new_path;

                /* now try to interpret the thing */
                if ( path -> from_uri )
                {
                    /* look at scheme */
                    switch ( LegacyVPathGetUri_t ( path ) )
                    {
                    case vpuri_ncbi_acc:
                    case vpuri_ncbi_obj:
                    case vpuri_ncbi_legrefseq:
                        /* try to resolve using VResolver */
                        rc = LegacyVPathResolveAccession ( new_path, path );
                        break;

                    case vpuri_ncbi_vfs:
                    case vpuri_file:
                        /* test for full path */
                        if ( path -> path_type == vpFullPath || path -> path_type == vpUNCPath )
                            return 0;

                        /* interpret relative to given directory */
                        rc = LegacyVPathMakeKDirRelative ( new_path, dir, path );
                        break;

                    case vpuri_http:
                    case vpuri_https:
                    case vpuri_ftp:
                    case vpuri_fasp:
                        /* calling code would know how to handle these */
                        return 0;

                    default:
                        /* while apparently valid URIs, callers would not know */
                        * new_path = NULL;
                    }
                }

                else
                {
                    /* look at deduced path type */
                    switch ( path -> path_type )
                    {
                    case vpOID:
                    case vpAccession: /* this is not expected not be set... */
                    case vpNameOrOID:
                    case vpNameOrAccession:
                        /* try to resolve using VResolver */
                        rc = LegacyVPathResolveAccession ( new_path, path );
                        if ( rc == 0 )
                            break;

                        /* no break */

                    case vpName:
                    case vpRelPath:
                        rc = LegacyVPathMakeKDirRelative ( new_path, dir, path );
                        break;

                    case vpUNCPath:
                    case vpFullPath:
                        return 0;

                    default:
                        * new_path = NULL;
                    }
                }

                /* clean up path */
                assert ( * new_path != path );
                if ( vpath == NULL )
                    VPathRelease ( path );
                return rc;
            }
        }

        * new_path = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC LegacyVPathMakeDirectoryRelative ( VPath ** new_path,
    const KDirectory * dir, const char * posix_path )
{
    return VPathMakeDirectoryRelativeVPath(new_path, dir, posix_path, NULL);
}

/* Option
 *  rc == 0 if the option has been specified
 *  for options with a parameter, the value of the parameter is copied to buffer
 */
LIB_EXPORT rc_t CC VPathOption ( const VPath * self, VPOption_t option,
    char * buffer, size_t buffer_size, size_t * num_read )
{
    rc_t rc = VPathReadTestSelf ( self, buffer, buffer_size, num_read );
    if ( rc == 0 )
    {
        const char * param1, * param2 = NULL;

        switch ( option )
        {
        case vpopt_encrypted:
            param1 = "enc";
            param2 = "encrypt";
            break;
        case vpopt_pwpath:
            param1 = "pwfile";
            break;
        case vpopt_pwfd:
            param1 = "pwfile";
            break;
        case vpopt_readgroup:
            param1 = "readgroup";
            break;
#if 0            
        case vpopt_temporary_pw_hack:
            param1 = "temporary_pw_hack";
            break;
#endif            
        case vpopt_vdb_ctx:
            param1 = "vdb-ctx";
            break;
        case vpopt_gap_ticket: 
            param1 = "tic";
            break;
        case vpopt_gap_prjId:
            param1 = "pId";
            break;
        default:
            return RC ( rcVFS, rcPath, rcReading, rcToken, rcUnrecognized );
        }

        rc = VPathReadParam ( self, param1, buffer, buffer_size, num_read );
        if ( param2 != NULL && GetRCState ( rc ) == rcNotFound )
            rc = VPathReadParam ( self, param2, buffer, buffer_size, num_read );
    }
    return rc;
}


LIB_EXPORT rc_t LegacyVPathMake ( VPath ** new_path, const char * posix_path )
{
    return LegacyVPathMakeFmt ( new_path, posix_path );
}

rc_t LegacyVPathMakeFmt ( VPath ** new_path, const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = LegacyVPathMakeVFmt ( new_path, fmt, args );

    va_end ( args );

    return rc;
}

static
rc_t VPathMakeVFmtExt ( EVPathType ext, VPath ** new_path, const String * id,
    const String * tick, uint64_t osize, KTime_t date, const uint8_t md5 [ 16 ],
    KTime_t exp_date, const char * service, const String * objectType,
    const String * type, bool ceRequired, bool payRequired, const char * name,
    int64_t projectId, uint32_t version, const char * fmt, va_list args )
{
    rc_t rc;

    if ( new_path == NULL )
        rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcNull );
    else
    {
        if ( fmt == NULL )
            rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcNull );
        else if ( fmt [ 0 ] == 0 )
            rc = RC ( rcVFS, rcPath, rcConstructing, rcParam, rcEmpty );
        else
        {
            rc = VPathMakeFromVText ( new_path, fmt, args );
            if ( rc == 0 )
            {
                VPath * path = * new_path;

                if ( ! path -> from_uri )
                {
                    String scheme;
                    rc = VPathGetSchemeInt ( path, & scheme );
                    if ( rc == 0 )
                        path -> scheme = scheme;
                }

                path -> ext = ext;
                path -> osize = osize;
                path -> projectId = projectId;
                path -> version = version;
                path -> modification = date;
                path -> expiration = exp_date;

                if ( md5 != NULL ) {
                    int i = 0;
                    for ( i = 0; i < 16; ++ i )
                        path -> md5 [ i ] = md5 [ i ];
                    path -> has_md5 = true;
                }

                if ( tick != NULL && tick -> size > 0 ) {
                    StringInit ( & path -> tick,
                        string_dup ( tick -> addr, tick -> size ),
                        tick -> size, tick -> len );
                    if ( path -> tick . addr == NULL )
                        return RC ( rcVFS,
                            rcPath, rcAllocating, rcMemory, rcExhausted );
                }

                if ( service != NULL ) {
                    size_t size = 0;
                    char * srv = string_dup_measure ( service, & size );
                    if ( srv == NULL )
                        return RC ( rcVFS,
                            rcPath, rcAllocating, rcMemory, rcExhausted );
                    StringInit ( & path -> service, srv, size, size );
                }

                if (objectType != NULL && objectType->size > 0) {
                    StringInit(&path->objectType,
                        string_dup(objectType->addr, objectType->size),
                        objectType->size, objectType->len);
                    if (path->objectType.addr == NULL)
                        return RC(rcVFS,
                            rcPath, rcAllocating, rcMemory, rcExhausted);
                }

                if (type != NULL && type->size > 0) {
                    StringInit(&path->type,
                        string_dup(type->addr, type->size),
                        type->size, type->len);
                    if (path->type.addr == NULL)
                        return RC(rcVFS,
                            rcPath, rcAllocating, rcMemory, rcExhausted);
                }

                if (name != NULL) {
                    size_t size = 0;
                    char * c = string_dup_measure(name, &size);
                    if (c == NULL)
                        return RC(rcVFS,
                            rcPath, rcAllocating, rcMemory, rcExhausted);
                    StringInit(&path->name, c, size, size);

                    c = string_chr(path->name.addr, path->name.size, '.');
                    if (c == NULL)
                        size = 0;
                    else
                        size = path->name.size - (++c - path->name.addr);
                    StringInit(&path->nameExtension, c, size, size);
                }

                rc = VPathSetId(path, id);
                if ( rc != 0 )
                    return rc;

                path->ceRequired = ceRequired;
                path->payRequired = payRequired;

                return 0;
            }
        }

        * new_path = NULL;
    }

    return rc;
}

rc_t VPathSetId(VPath * self, const String * id) {
    assert(self);

    if (self->id.addr != NULL) {
        free((void*)self->id.addr);
        memset(&self->id, 0, sizeof self->id);
    }

    if (id != NULL && id->size > 0) {
        StringInit(&self->id,
            string_dup(id->addr, id->size),
            id->size, id->len);
        if (self->id.addr == NULL)
            return RC(rcVFS,
                rcPath, rcAllocating, rcMemory, rcExhausted);
    }

    return 0;
}

rc_t VPathSetMagic(VPath * self, bool magic) {
    assert(self);

    self->magic = magic;

    return 0;
}

static
rc_t VPathMakeFmtExt ( VPath ** new_path, bool ext, const String * id,
	const String * tick, uint64_t osize, KTime_t date, const uint8_t md5 [ 16 ],
	KTime_t exp_date, const char * service, const String * objectType,
    const String * type, bool ceRequired, bool payRequired, const char * name,
    int64_t projectId, uint32_t version, const char * fmt, ... )
{
    EVPathType t = ext ? eVPext : eVPWithId; 
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = VPathMakeVFmtExt ( t, new_path, id, tick, osize, date, md5, exp_date,
        service, objectType, type, ceRequired, payRequired, name, projectId,
        version, fmt, args );

    va_end ( args );

    return rc;
}

rc_t VPathMakeFromUrl ( VPath ** new_path, const String * url,
    const String * tick, bool ext, const String * id, uint64_t osize,
    KTime_t date, const uint8_t md5 [ 16 ], KTime_t exp_date,
    const char * service, const String * objectType, const String * type,
    bool ceRequired, bool payRequired, const char * name,
    int64_t projectId, uint32_t version)
{
    if ( tick != NULL && tick -> addr != NULL && tick -> size != 0 ) {
        const char * fmt = NULL;
        assert(url);
        if (string_chr(url->addr, url->size, '?') == NULL)
            fmt = "%S?tic=%S";
        else
            fmt = "%S&tic=%S";
        return VPathMakeFmtExt(new_path, ext, id, tick, osize, date, md5,
            exp_date, service, objectType, type, ceRequired, payRequired, name,
            projectId, version, fmt, url, tick);
    }
    else if (projectId >= 0) {
        const char * fmt = NULL;
        assert(url);
        if (string_chr(url->addr, url->size, '?') == NULL)
            fmt = "%S?pId=%d";
        else
            fmt = "%S&pId=%d";
        return VPathMakeFmtExt(new_path, ext, id, tick, osize, date, md5,
            exp_date, service, objectType, type, ceRequired, payRequired, name,
            projectId, version, fmt, url, projectId);
    }
    else
        return VPathMakeFmtExt ( new_path, ext, id, tick, osize, date, md5,
		    exp_date, service, objectType, type, ceRequired, payRequired, name,
            projectId, version, "%S", url );
}

rc_t LegacyVPathMakeVFmt ( VPath ** new_path, const char * fmt, va_list args )
{
    return VPathMakeVFmtExt ( false, new_path, NULL, NULL, 0, 0, NULL, 0,
        NULL, NULL, NULL, false, false, NULL, -1, 0, fmt, args );
}

rc_t VPathAttachVdbcache(VPath * self, const VPath * vdbcache) {
    rc_t rc = 0;

    if (self != NULL) {
        if (vdbcache != NULL) {
            rc = VPathAddRef(vdbcache);
            if (rc == 0) {
                rc = VPathRelease(self->vdbcache);
                self->vdbcache = vdbcache;
            }
        }

        if (rc == 0)
            self->vdbcacheChecked = true;
    }

    return rc;
}

/* GetVdbcache
 *  return attached vdbcahe
 * and boolian trating that there is no neew to check remove vdbcache URL
 */
LIB_EXPORT rc_t CC VPathGetVdbcache(const VPath * self,
    const VPath ** vdbcache, bool * vdbcacheChecked)
{
    rc_t rc = 0;

    bool dummy = true;
    if (vdbcacheChecked == NULL)
        vdbcacheChecked = &dummy;

    if (vdbcache == NULL)
        return RC(rcVFS, rcPath, rcAccessing, rcParam, rcNull);
    if (self == NULL)
        return RC(rcVFS, rcPath, rcAccessing, rcSelf, rcNull);

    *vdbcacheChecked = false;

    rc = VPathAddRef(self->vdbcache);

    if (rc == 0) {
        *vdbcache = self->vdbcache;
        *vdbcacheChecked = self->vdbcacheChecked;
    }

    return rc;
}

LIB_EXPORT rc_t CC LegacyVPathGetScheme_t ( const VPath * self, VPUri_t * uri_type )
{
    rc_t rc = 0;

    if ( uri_type == NULL )
        rc = RC ( rcVFS, rcPath, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcPath, rcAccessing, rcSelf, rcNull );
        else if ( self -> path_type != vpInvalid )
        {
            if ( ! self -> from_uri )
            {
                * uri_type = vpuri_none;
                return 0;
            }

            * uri_type = self -> scheme_type;
            return 0;
        }

        /* object was invalid */
        * uri_type = vpuri_invalid;
    }

    return rc;
}

VPUri_t LegacyVPathGetUri_t ( const VPath * self )
{
    VPUri_t uri_type;
    LegacyVPathGetScheme_t ( self, & uri_type );
    return uri_type;
}

bool VPathGetProjectId(const VPath * self, uint32_t * projectId) {
    uint32_t dummy = 0;
    if (projectId == NULL)
        projectId = &dummy;

    assert(self);

    *projectId = 0;

    if (self->projectId < 0)
        return false;

    *projectId = self->projectId;
    return true;
}

rc_t VPathClose ( const VPath * l, const VPath * r, int * notequal,
                  KTime_t expirationRange )
{
    rc_t rc = 0;
    char pbuffer [ 999 ] = "";
    size_t pnumred = 0;
    char ebuffer [ 999 ] = "";
    size_t end = 0;
    rc_t rp = 0;
    rc_t re = 0;
    String pstr = { 0, 0, 0 };
    String estr = { 0, 0, 0 };
    VPUri_t puri_type;
    VPUri_t euri_type;
    int dummy = 0;
    if ( notequal == NULL )
        notequal = & dummy;

    * notequal = 0;

    if ( l == r ) {
        return 0;
    }

    if ( ( l == NULL && r != NULL ) || ( r == NULL && l != NULL ) ) {
        * notequal = 1;
        return 0;
    }

    rp = VPathReadUri ( l, pbuffer, sizeof pbuffer, & pnumred );
    re = VPathReadUri ( r, ebuffer, sizeof ebuffer, & end );
    if ( rp == 0 && re == 0 ) {
        if ( pnumred != end )
            * notequal |= 2;
        else if ( string_cmp ( pbuffer, pnumred, ebuffer, end,
                               ( uint32_t ) end ) != 0 )
        {
            * notequal |= 4;
        }
    }
    else if ( rc == 0 ) {
        if ( rp != 0 )
            rc = rp;
        else
            rc = re;
    }

    rp = VPathGetAuth ( l, & pstr );
    re = VPathGetAuth ( r, & estr );
    if ( rp == 0 && re == 0 ) {
        if ( ! StringEqual ( & pstr, & estr ) )
            * notequal |= 8;
    }
    else if ( rc == 0 ) {
        if ( rp != 0 )
            rc = rp;
        else
            rc = re;
    }

    if ( VPathFromUri ( l ) != VPathFromUri ( r ) )
        * notequal |= 0x10;

    rp = VPathGetScheme_t ( l, & puri_type );
    re = VPathGetScheme_t ( r, & euri_type );
    if ( rp == 0 && re == 0 ) {
        if ( memcmp ( & puri_type, & euri_type, sizeof euri_type) != 0 )
            * notequal |= 0x20;
    }
    else if ( rc == 0 ) {
        if ( rp != 0 )
            rc = rp;
        else
            rc = re;
    }

    if ( VPathIsHighlyReliable ( l ) != VPathIsHighlyReliable ( r ) )
        * notequal |= 0x40;

    if ( l -> ext && r -> ext ) {
        rp = VPathGetId ( l, & pstr );
        re = VPathGetId ( r, & estr );
        if ( rp == 0 && re == 0 ) {
            if ( ! StringEqual ( & pstr, & estr ) )
                * notequal |= 0x80;
        }
        else if ( rc == 0 ) {
            if ( rp != 0 )
                rc = rp;
            else
                rc = re;
        }

        rp = VPathGetTicket ( l, & pstr );
        re = VPathGetTicket ( r, & estr );
        if ( rp == 0 && re == 0 ) {
            if ( ! StringEqual ( & pstr, & estr ) )
                * notequal |= 0x100;
        }
        else if ( rc == 0 ) {
            if ( rp != 0 )
                rc = rp;
            else
                rc = re;
        }

        if ( l -> ext == eVPext && r -> ext == eVPext ) {
            {
                KTime_t tp = VPathGetModDate ( l );
                KTime_t te = VPathGetModDate ( r );
                if ( tp != te )
                    * notequal |= 0x200;
            }
            {
                uint64_t p = VPathGetSize ( l );
                uint64_t e = VPathGetSize ( r );
                if ( p != e )
                    * notequal |= 0x400;
            }
            {
                const uint8_t * p = VPathGetMd5 ( l );
                const uint8_t * e = VPathGetMd5 ( r );
                if ( ( p == NULL && e != NULL ) || ( e == NULL && p != NULL ) )
                    * notequal |= 0x800;
                else if ( p != NULL ) {
                int i = 0;
                for ( i = 0; i < 16; ++i )
                    if ( p [ i ] != e [ i ] ) {
                        * notequal |= 0x1000;
                        break;
                    }
                }
            }
            {
                KTime_t tp = l -> expiration;
                KTime_t te = r -> expiration;
                if ( tp != te ) {
/* TO IMPLEMENT     KTime_t diff = abs ( l -> expiration - r -> expiration );*/
                    * notequal |= 0x2000;
                }
            }
        }
    }

    return rc;
}

rc_t VPathEqual ( const VPath * l, const VPath * r, int * notequal ) {
    return VPathClose ( l, r, notequal, 0 );
}

rc_t VPathSetAccOfParentDb(VPath * self, const String * acc) {
    rc_t rc = 0;

    if (self != NULL && acc != NULL)
        rc = StringCopy(&self->accOfParentDb, acc);

    return rc;
}
