#ifndef WINDOWS
//#define DEPURAR 1
#endif
/*==============================================================================
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

#include "diagnose/diagnose.h" /* KDiagnoseRun */
#include "diagnose.h" /* endpoint_to_string */

#include <kfg/config.h> /* KConfigReadString */

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFile */

#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/out.h> /* KOutMsg */
#include <klib/printf.h> /* string_vprintf */
#include <klib/rc.h>
#include <klib/text.h> /* String */

#include <kns/endpoint.h> /* KNSManagerInitDNSEndpoint */
#include <kns/http.h> /* KHttpRequest */
#include <kns/manager.h> /* KNSManager */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeReliableHttpFile */
#include <kns/stream.h> /* KStream */

#include <vfs/manager.h> /* VFSManagerOpenDirectoryRead */
#include <vfs/path.h> /* VFSManagerMakePath */

#include <strtol.h> /* strtoi64 */

#include <ctype.h> /* isprint */
#include <limits.h> /* PATH_MAX */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

struct KDiagnose {
    atomic32_t refcount;

    KConfig    * kfg;
    KNSManager * kmgr;
    VFSManager * vmgr;

    int verbosity;
};

rc_t KDiagnoseSetVerbosity ( KDiagnose * self, int verbosity ) {
    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    self -> verbosity = verbosity - 1;

    return 0;
}

typedef struct {
    int n [ 6 ];
    int level;
    bool ended;
    bool started;           /*  TestStart did not terminale string by EOL */
    bool failedWhileSilent;

    int verbosity; /* -3    none  ( KVERBOSITY_NONE  )
                      -2    error ( KVERBOSITY_ERROR )
                      -1    info  ( KVERBOSITY_INFO  )
                       0... last printed index of n [] */

    int total;

    KDataBuffer msg;

    const KConfig * kfg;
    const KNSManager * kmgr;
    const VFSManager * vmgr;
} STest;

static void STestInit ( STest * self, const KDiagnose * test )
{
    assert ( self && test );

    memset ( self, 0, sizeof * self );

    self -> level = -1;

    self -> kfg = test -> kfg;
    self -> kmgr = test -> kmgr;
    self -> vmgr = test -> vmgr;

    self -> verbosity = test -> verbosity;
    if ( self -> verbosity > 0 )
        -- self -> verbosity;
    else if ( self -> verbosity == 0 ) /* max */
        self -> verbosity = sizeof self -> n / sizeof self -> n [ 0 ] - 1;
}

static void STestFini ( STest * self ) {
    assert ( self );

    if ( self -> level < KVERBOSITY_INFO )
        return;

    if ( self -> n [ 0 ] == 0 || self -> n [ 1 ] != 0 || self -> level != 0 )
        OUTMSG ( ( "= TEST WAS NOT COMPLETED\n" ) );

    OUTMSG ( ( "= %d (%d) tests performed\n",
               self -> n [ 0 ], self -> total ) );
}

static rc_t STestVStart ( STest * self, bool checking,
                          const char * fmt, va_list args  )
{
    rc_t rc = 0;
    int i = 0;
    char b [ 512 ] = "";
    KDataBuffer bf;
    memset ( & bf, 0, sizeof bf );
    rc = string_vprintf ( b, sizeof b, NULL, fmt, args );
    if ( rc != 0 ) {
        OUTMSG ( ( "CANNOT PRINT: %R\n", rc ) );
        return rc;
    }

    assert ( self );

    if ( self -> ended )
        self -> ended = false;
    else
        ++ self -> level;

    assert ( self -> level >= 0 );
    assert ( self -> level < sizeof self -> n / sizeof self -> n [ 0 ] );

    ++ self -> n [ self -> level ];

    if ( self -> msg . elem_count > 0 ) {
        assert ( self -> msg . base );
        ( ( char * ) self -> msg . base)  [ 0 ] = '\0';
        self -> msg . elem_count = 0;
    }
    rc = KDataBufferPrintf ( & self -> msg,  "< %d", self -> n [ 0 ] );
#ifdef DEPURAR
const char*c=self->msg.base;
#endif
    if ( rc != 0 )
        OUTMSG ( ( "CANNOT PRINT: %R\n", rc ) );
    else 
        for ( i = 1; rc == 0 && i <= self -> level; ++ i ) {
            rc = KDataBufferPrintf ( & self -> msg, ".%d", self -> n [ i ] );
            if ( rc != 0 )
                OUTMSG ( ( "CANNOT PRINT: %R\n", rc ) );
        }
    if ( rc == 0 )
        rc = KDataBufferPrintf ( & self -> msg, " %s ", b );
        if ( rc != 0 )
            OUTMSG ( ( "CANNOT PRINT: %R\n", rc ) );

    if ( self -> level <= self -> verbosity ) {
        OUTMSG ( ( "> %d", self -> n [ 0 ] ) );
        for ( i = 1; i <= self -> level; ++ i )
            OUTMSG ( ( ".%d", self -> n [ i ] ) );

        rc = OUTMSG ( ( " %s%s%s", checking ? "Checking " : "", b,
                        checking ? "..." : " " ) );
        if ( checking ) {
            if ( self -> level < self -> verbosity ) {
                OUTMSG ( ( "\n" ) );
                self -> started = false;
            }
            else {
                OUTMSG ( ( " " ) );
                self -> started = true;
            }
        }
    }

    return rc;
}

typedef enum {
    eFAIL,
    eOK,
    eMSG,
    eEndFAIL,
    eEndOK,
    eDONE,
} EOK;

static rc_t STestVEnd ( STest * self, EOK ok,
                        const char * fmt, va_list args )
{
    rc_t rc = 0;
#ifdef DEPURAR
switch(ok){
case eFAIL:
rc=0;
break;
case eOK:
rc=1;
break;
case eMSG:
rc=2;
break;
case eEndFAIL:
rc=3;
break;
case eEndOK:
rc=4;
break;
case eDONE:
rc=5;
break;
}
rc=0;
#endif
    bool failedWhileSilent = self -> failedWhileSilent;
    bool print = false;
    char b [ 512 ] = "";

    assert ( self );

    if ( ok != eMSG ) {
        if ( self -> ended )
            self -> n [ self -> level -- ] = 0;
        else {
            self -> ended = true;
            ++ self -> total;
        }
    }

    assert ( self -> level >= 0 );
#ifdef DEPURAR
const char*c=self->msg.base;
#endif
    rc = string_vprintf ( b, sizeof b, NULL, fmt, args );
    if ( rc != 0 ) {
        OUTMSG ( ( "CANNOT PRINT: %R", rc ) );
        return rc;
    }

    if ( self -> level > self -> verbosity ) {
        if ( ok == eEndFAIL || ok == eMSG ) {
            rc = KDataBufferPrintf ( & self -> msg, b );
            if ( rc != 0 )
                OUTMSG ( ( "CANNOT PRINT: %R", rc ) );
            else if ( ok == eEndFAIL ) {
                rc = KDataBufferPrintf ( & self -> msg, "\n" );
                if ( self -> started ) {
                    OUTMSG ( ( "\n" ) );
                    self -> failedWhileSilent = true;
                    self -> started = false;
                }
                if ( self -> level >= KVERBOSITY_ERROR )
                    OUTMSG ( ( self -> msg . base ) );
                assert ( self -> msg . base );
                ( ( char * ) self -> msg . base)  [ 0 ] = '\0';
                self -> msg . elem_count = 0;
            }
        }
        return rc;
    }

    print = self -> level < self -> verbosity || failedWhileSilent;
    if ( ok == eFAIL || ok == eOK || ok == eDONE ) {
        if ( print ) {
            int i = 0;
            rc = OUTMSG ( ( "< %d", self -> n [ 0 ] ) );
            for ( i = 1; i <= self -> level; ++ i )
                OUTMSG ( ( ".%d", self -> n [ i ] ) );
            OUTMSG ( ( " " ) );
        }
    }
    if ( print ||
            ( self -> level == self -> verbosity &&
              ok != eFAIL && ok != eOK ) )
    {
        OUTMSG ( ( b ) );
    }

    if ( print )
        switch ( ok ) {
            case eFAIL: OUTMSG ( ( ": FAILURE\n" ) ); break;
            case eOK  : OUTMSG ( ( ": OK\n"      ) ); break;
            case eEndFAIL:
            case eEndOK :
            case eDONE: OUTMSG ( (     "\n"      ) ); break;
            default   :                               break;
        }
    else if ( self -> level == self -> verbosity )
        switch ( ok ) {
            case eFAIL: OUTMSG ( ( "FAILURE\n" ) ); break;
            case eOK  : OUTMSG ( ( "OK\n"      ) ); break;
            case eEndFAIL:
            case eEndOK :
            case eDONE: OUTMSG ( (   "\n"      ) ); break;
            default   :                             break;
        }

    self -> failedWhileSilent = false;

    return rc;
}

static rc_t STestEnd ( STest * self, EOK ok, const char * fmt, ...  )  {
    rc_t rc = 0;

    va_list args;
    va_start ( args, fmt );

    rc = STestVEnd ( self, ok, fmt, args );

    va_end ( args );

    return rc;
}

static rc_t STestStart ( STest * self, bool checking,
                         const char * fmt, ...  )
{
    rc_t rc = 0;

    va_list args;
    va_start ( args, fmt );

    rc = STestVStart ( self, checking, fmt, args );

    va_end ( args );

    return rc;
}

typedef struct {
    VPath * vpath;
    const String * acc;
} Data;

static rc_t DataInit ( Data * self, const VFSManager * mgr,
                       const char * path )
{
    rc_t rc = 0;

    assert ( self );

    memset ( self, 0, sizeof * self );

    rc = VFSManagerMakePath ( mgr, & self -> vpath, path );
    if ( rc != 0 )
        OUTMSG ( ( "VFSManagerMakePath(%s) = %R\n", path, rc ) );
    else {
        VPath * vacc = NULL;
        rc = VFSManagerExtractAccessionOrOID ( mgr, & vacc, self -> vpath );
        if ( rc != 0 )
            rc = 0;
        else {
            String acc;
            rc = VPathGetPath ( vacc, & acc );
            if ( rc == 0 )
                StringCopy ( & self -> acc, & acc );
            else
                OUTMSG ( ( "Cannot VPathGetPath"
                           "(VFSManagerExtractAccessionOrOID(%R))\n", rc ) );
        }
    }

    return rc;
}

static rc_t DataFini ( Data * self ) {
    rc_t rc = 0;

    assert ( self );

    free ( ( void * ) self -> acc );

    rc = VPathRelease ( self -> vpath );

    memset ( self, 0, sizeof * self );

    return rc;
}

static const ver_t HTTP_VERSION = 0x01010000;

static rc_t STestCheckFileSize ( STest * self, const String * path,
                                 uint64_t * sz )
{
    rc_t rc = 0;

    const KFile * file = NULL;

    assert ( self );

    STestStart ( self, false,
                 "KFile = KNSManagerMakeReliableHttpFile(%S):", path );

    rc = KNSManagerMakeReliableHttpFile ( self -> kmgr, & file, NULL,
                                          HTTP_VERSION, "%S", path );
    if ( rc != 0 )
        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    else {
        if ( rc == 0 ) {
            STestEnd ( self, eEndOK, "OK" );

            rc = STestStart ( self, false, "KFileSize(KFile(%S)) =", path );
            rc = KFileSize ( file, sz );
            if ( rc == 0 )
                STestEnd ( self, eEndOK, "%lu: OK", * sz );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }

    KFileRelease ( file );
    file = NULL;

    return rc;
}

static
rc_t STestCheckRanges ( STest * self, const Data * data, uint64_t sz )
{
    rc_t rc = STestStart ( self, true, "Support of Range requests" );
    uint64_t pos = 0;
    size_t bytes = 4096;
    uint64_t ebytes = bytes;
    bool https = false;
    char buffer [ 1024 ] = "";
    size_t num_read = 0;
    KClientHttp * http = NULL;
    KHttpRequest * req = NULL;
    KHttpResult * rslt = NULL;
    String host;
    String scheme;
    assert ( self && data );
    rc = VPathGetHost ( data -> vpath, & host );
    if ( rc != 0 )
        OUTMSG ( ( "Cannot VPathGetHost(%R)\n", rc ) );
    if ( rc == 0 )
        rc = VPathGetScheme ( data -> vpath, & scheme );
    if ( rc != 0 )
        OUTMSG ( ( "Cannot VPathGetScheme(%R)\n", rc ) );
    if ( rc == 0 ) {
        String sHttps;
        String sHttp;
        CONST_STRING ( & sHttp, "http" );
        CONST_STRING ( & sHttps, "https" );
        if ( StringEqual ( & scheme, & sHttps ) )
            https = true;
        else if ( StringEqual ( & scheme, & sHttp ) )
            https = false;
        else {
            OUTMSG ( ( "Unexpected scheme '(%S)'\n", & scheme ) );
            return 0;
        }
    }
    if ( rc == 0 ) {
        if ( https ) {
            STestStart ( self, false, "KClientHttp = "
                         "KNSManagerMakeClientHttps(%S):", & host );
            rc = KNSManagerMakeClientHttps ( self -> kmgr, & http, NULL,
                                             HTTP_VERSION, & host, 0 );
        }
        else {
            STestStart ( self, false, "KClientHttp = "
                         "KNSManagerMakeClientHttp(%S):", & host );
            rc = KNSManagerMakeClientHttp ( self -> kmgr, & http, NULL,
                                            HTTP_VERSION, & host, 0 );
        }
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        String path;
        rc = VPathGetPath ( data -> vpath, & path );
        if ( rc != 0 )
            OUTMSG ( ( "Cannot VPathGetPath(%R)\n", rc ) );
        else {
            rc = KHttpMakeRequest ( http, & req, "%S", & path );
            if ( rc != 0 )
                OUTMSG ( ( "KHttpMakeRequest(%S) = %R\n", & path, rc ) );
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false, "KHttpResult = "
            "KHttpRequestHEAD(KHttpMakeRequest(KClientHttp)):" );
        rc = KHttpRequestHEAD ( req, & rslt );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpResultGetHeader(KHttpResult, Accept-Ranges) =" );
        rc = KHttpResultGetHeader ( rslt, "Accept-Ranges",
                                    buffer, sizeof buffer, & num_read );
        if ( rc == 0 ) {
            const char bytes [] = "bytes";
            if ( string_cmp ( buffer, num_read, bytes, sizeof bytes - 1,
                              sizeof bytes - 1 ) == 0 )
            {
                STestEnd ( self, eEndOK, "'%.*s': OK",
                                        ( int ) num_read, buffer );
            }
            else {
                STestEnd ( self, eEndFAIL, "'%.*s': FAILURE",
                                        ( int ) num_read, buffer );
                rc = RC ( rcExe, rcFile, rcOpening, rcFunction, rcUnsupported );
            }
        }
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    KHttpResultRelease ( rslt );
    rslt = NULL;
    if ( sz < ebytes )
        ebytes = sz;
    if ( sz > bytes * 2 )
        pos = sz / 2;
    if ( rc == 0 ) {
        STestStart ( self, false, "KHttpResult = KHttpRequestByteRange"
                        "(KHttpMakeRequest, %lu, %zu):", pos, bytes );
        rc = KHttpRequestByteRange ( req, pos, bytes );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
            "KHttpResult = KHttpRequestGET(KHttpMakeRequest(KClientHttp)):" );
        rc = KHttpRequestGET ( req, & rslt );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        uint64_t po = 0;
        size_t byte = 0;
        rc = KClientHttpResultRange ( rslt, & po, & byte );
        if ( rc == 0 ) {
            if ( po != pos || ( ebytes > 0 && byte != ebytes ) ) {
                STestStart ( self, false,
                             "KClientHttpResultRange(KHttpResult,&p,&b):" );
                STestEnd ( self, eEndFAIL, "FAILURE: expected:{%lu,%zu}, "
                            "got:{%lu,%zu}", pos, ebytes, po, byte );
                rc = RC ( rcExe, rcFile, rcReading, rcRange, rcOutofrange );
            }
        }
        else {
            STestStart ( self, false, "KClientHttpResultRange(KHttpResult):" );
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpResultGetHeader(KHttpResult, Content-Range) =" );
        rc = KHttpResultGetHeader ( rslt, "Content-Range",
                                    buffer, sizeof buffer, & num_read );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "'%.*s': OK",
                                    ( int ) num_read, buffer );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    KHttpResultRelease ( rslt );
    rslt = NULL;
    KHttpRequestRelease ( req );
    req = NULL;
    KHttpRelease ( http );
    http = NULL;
    STestEnd ( self, rc == 0 ? eOK : eFAIL, "Support of Range requests" );
    return rc;
}

static rc_t STestCheckStreamRead ( STest * self, const KStream * stream,
    uint64_t sz, bool print, const char * exp, size_t esz )
{
    rc_t rc = 0;
    size_t total = 0;
    char buffer [ 1024 ] = "";
    STestStart ( self, false, "KStreamRead(KHttpResult):" );
    while ( rc == 0 ) {
        size_t num_read = 0;
        rc = KStreamRead ( stream, buffer, sizeof buffer, & num_read );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else if ( num_read != 0 ) {
            if ( total == 0 && esz > 0 ) {
                int i = 0;
                int s = esz;
                if ( num_read < esz )
                    s = num_read;
                STestEnd ( self, eMSG, "'" );
                for ( i = 0; i < s; ++ i ) {
                    if ( isprint ( buffer [ i ] ) )
                        STestEnd ( self, eMSG, "%c", buffer [ i ] );
                    else if ( buffer [ i ] == 0 )
                        STestEnd ( self, eMSG, "\\0" );
                    else
                        STestEnd ( self, eMSG, "\\%03o",
                                               ( unsigned char ) buffer [ i ] );
                }
                STestEnd ( self, eMSG, "': " );
                if ( string_cmp ( buffer, num_read, exp, esz, esz ) != 0 ) {
                    STestEnd ( self, eEndFAIL, " FAILURE: bad content" );
                    rc = RC ( rcExe, rcFile, rcReading, rcString, rcUnequal );
                }
            }
            total += num_read;
        }
        else {
            assert ( num_read == 0 );
            if ( total == sz ) {
                if ( print ) {
                    if ( total >= sizeof buffer )
                        buffer [ sizeof buffer - 1 ] = '\0';
                    else {
                        buffer [ total ] = '\0';
                        while ( total > 0 ) {
                            -- total;
                            if ( buffer [ total ] == '\n' )
                                buffer [ total ] = '\0';
                            else
                                break;
                        }
                    }
                    STestEnd ( self, eMSG, "%s: ", buffer );
                }
                STestEnd ( self, eEndOK, "OK" );
            }
            else
                STestEnd ( self, eEndFAIL, "%s: SIZE DO NOT MATCH (%zu)\n",
                                           total );
            break;
        }
    }
    return rc;
}

static rc_t STestCheckHttpUrl ( STest * self, const Data * data, bool print,
                                const char * exp, size_t esz )
{
    rc_t rc = 0;
    KHttpRequest * req = NULL;
    KHttpResult * rslt = NULL;
    const String * full = NULL;
    uint64_t sz = 0;
    assert ( self && data );
    rc = VPathMakeString ( data -> vpath, & full );
    if ( rc != 0 )
        OUTMSG ( ( "CANNOT VPathMakeString: %R\n", rc ) );
    if ( rc == 0 )
        rc = STestStart ( self, true, "Access to '%S'", full );
    if ( rc == 0 )
        rc = STestCheckFileSize ( self, full, & sz );
    if ( rc == 0 )
        rc = STestCheckRanges ( self, data, sz );
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpRequest = KNSManagerMakeRequest(%S):", full );
        rc = KNSManagerMakeRequest ( self -> kmgr, & req,
                                     HTTP_VERSION, NULL, "%S", full );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK"  );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpResult = KHttpRequestGET(KHttpRequest):" );
        rc = KHttpRequestGET ( req, & rslt );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        uint32_t code = 0;
        STestStart ( self, false, "KHttpResultStatus(KHttpResult) =" );
        rc = KHttpResultStatus ( rslt, & code, NULL, 0, NULL );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            STestEnd ( self, eMSG, "%u: ", code );
            if ( code == 200 )
                STestEnd ( self, eEndOK, "OK" );
            else {
                STestEnd ( self, eEndFAIL, "FAILURE" );
                rc = RC ( rcExe, rcFile, rcReading, rcFile, rcInvalid );
            }
        }
    }
    if ( rc == 0 ) {
        KStream * stream = NULL;
        rc = KHttpResultGetInputStream ( rslt, & stream );
        if ( rc != 0 )
            OUTMSG ( (
                "KHttpResultGetInputStream(KHttpResult) = %R\n", rc ) );
        else
            rc = STestCheckStreamRead ( self, stream, sz, print, exp, esz );
        KStreamRelease ( stream );
        stream = NULL;
    }
    STestEnd ( self, rc == 0 ? eOK : eFAIL, "Access to '%S'", full );
    free ( ( void * ) full );
    full = NULL;
    return rc;
}

static bool DataIsAccession ( const Data * self ) {
    assert ( self );

    if ( self -> acc == NULL )
        return false;
    else
        return self -> acc -> size != 0;
}

static rc_t STestCheckVfsUrl ( STest * self, const Data * data ) {
    rc_t rc = 0;

    const KDirectory * d = NULL;

    String path;

    assert ( self && data );

    if ( ! DataIsAccession ( data ) )
        return 0;

    rc = VPathGetPath ( data -> vpath, & path );
    if ( rc != 0 ) {
        OUTMSG ( ( "Cannot VPathGetPath(%R)", rc ) );
        return rc;
    }

    STestStart ( self, false, "VFSManagerOpenDirectoryRead(%S):", & path );
    rc = VFSManagerOpenDirectoryRead ( self -> vmgr, & d, data -> vpath );
    if ( rc == 0 )
        STestEnd ( self, eEndOK, "OK"  );
    else
        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );

    RELEASE ( KDirectory, d );

    return rc;
}

static rc_t STestCheckUrlImpl ( STest * self, const Data * data, bool print,
                                const char * exp, size_t esz )
{
    rc_t rc = STestCheckHttpUrl ( self, data, print, exp, esz );
    rc_t r2 = STestCheckVfsUrl  ( self, data );
    return rc != 0 ? rc : r2;
}

static rc_t STestCheckUrl ( STest * self, const Data * data, bool print,
                            const char * exp, size_t esz )
{
    rc_t rc = 0;

    String path;

    assert ( data );

    rc = VPathGetPath ( data -> vpath, & path );
    if ( rc != 0 ) {
        OUTMSG ( ( "Cannot VPathGetPath(%R)", rc ) );
        return rc;
    }

    if ( path . size == 0 ) /* does not exist */
        return 0;

    return STestCheckUrlImpl ( self, data, print, exp, esz );
}

static String * KConfig_Resolver ( const KConfig * self ) {
    String * s = NULL;

    rc_t rc = KConfigReadString ( self,
                                  "tools/test-sra/diagnose/resolver-cgi", & s );
    if ( rc != 0 ) {
        String str;
        CONST_STRING ( & str,
                       "https://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
        rc = StringCopy ( ( const String ** ) & s, & str );
        assert ( rc == 0 );
    }

    assert ( s );
    return s;
}

static int KConfig_Verbosity ( const KConfig * self ) {
    int64_t v = -1;

    String * s = NULL;
    rc_t rc = KConfigReadString ( self,
                                  "tools/test-sra/diagnose/verbosity", & s );
    if ( rc != 0 )
        return 0;

    assert ( s );

    if ( s -> size > 0 )
        if ( isdigit ( s -> addr [ 0 ] ) )
            v = strtoi64 ( s -> addr, NULL, 0 );

    free ( s );
    s = NULL;

    return ( int ) v;
}

static const char * STestCallCgi ( STest * self, const String * acc,
    char * response, size_t response_sz, size_t * resp_read )
{
    rc_t rc = 0;

    rc_t rs = 0;
    const char * url = NULL;
    KHttpRequest * req = NULL;
    const String * cgi = NULL;
    KHttpResult * rslt = NULL;
    KStream * stream = NULL;

    assert ( self );

    STestStart ( self, true, "Access to '%S'", acc );
    cgi = KConfig_Resolver ( self -> kfg );
    STestStart ( self, false,
        "KHttpRequest = KNSManagerMakeReliableClientRequest(%S):", cgi );
    rc = KNSManagerMakeReliableClientRequest ( self -> kmgr, & req,
        HTTP_VERSION, NULL, "%S", cgi);
    if ( rc == 0 )
        STestEnd ( self, eEndOK, "OK"  );
    else
        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    if ( rc == 0 ) {
        const char param [] = "accept-proto";
        rc = KHttpRequestAddPostParam ( req, "%s=https,http,fasp", param );
        if ( rc != 0 )
            OUTMSG ( ( "KHttpRequestAddPostParam() = %R\n", rc ) );
    }
    if ( rc == 0 ) {
        const char param [] = "object";
        rc = KHttpRequestAddPostParam ( req, "%s=0||%S", param, acc );
        if ( rc != 0 )
            OUTMSG ( ( "KHttpRequestAddPostParam() = %R\n", rc ) );
    }
    if ( rc == 0 ) {
        const char param [] = "version";
        rc = KHttpRequestAddPostParam ( req, "%s=3.0", param );
        if ( rc != 0 )
            OUTMSG ( ( "KHttpRequestAddPostParam() = %R\n", rc ) );
    }
    if ( rc == 0 ) {
        STestStart ( self, false, "KHttpRequestPOST(KHttpRequest(%S)):", cgi );
        rc = KHttpRequestPOST ( req, & rslt );
        if ( rc == 0 )
            STestEnd ( self, eEndOK, "OK"  );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        uint32_t code = 0;
        STestStart ( self, false, "KHttpResultStatus(KHttpResult(%S)) =", cgi );
        rc = KHttpResultStatus ( rslt, & code, NULL, 0, NULL );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            STestEnd ( self, eMSG, "%u: ", code );
            if ( code == 200 )
                STestEnd ( self, eEndOK, "OK" );
            else {
                STestEnd ( self, eEndFAIL, "FAILURE" );
                rs = RC ( rcExe, rcFile, rcReading, rcFile, rcInvalid );
            }
        }
    }
    if ( rc == 0 ) {
        rc = KHttpResultGetInputStream ( rslt, & stream );
        if ( rc != 0 )
            OUTMSG ( ( "KHttpResultGetInputStream() = %R\n", rc ) );
    }
    if ( rc == 0 ) {
        assert ( resp_read );
        STestStart ( self, false, "KStreamRead(KHttpResult(%S)) =", cgi );
        rc = KStreamRead ( stream, response, response_sz, resp_read );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            if ( * resp_read > response_sz - 4 ) {
                response [ response_sz - 4 ] = '.';
                response [ response_sz - 3 ] = '.';
                response [ response_sz - 2 ] = '.';
                response [ response_sz - 1 ] = '\0';
            }
            else {
                response [ * resp_read + 1 ] = '\0';
                for ( ; * resp_read > 0 && ( response [ * resp_read ] == '\n' ||
                                             response [ * resp_read ] == '\0' );
                      --  ( * resp_read ) )
                {
                    response [ * resp_read ] = '\0';
                }
            }
            STestEnd ( self, eEndOK, "'%s': OK", response );
            if ( rs == 0 ) {
                int i = 0;
                unsigned p = 0;
                for ( i = 0; p < * resp_read ; ++ i ) {
                    char * n = string_chr ( response + p,
                                            * resp_read - p, '|' );
                    if ( n != NULL )
                        p = n - response + 1;
                    if ( i == 6 ) {
                        url = n + 1;
                        break;
                    }
                }
            }
        }
    }
    if ( rc == 0 )
        rc = rs;
    KHttpResultRelease ( rslt );
    rslt = NULL;
    KHttpRequestRelease ( req );
    req = NULL;
    free ( ( void * ) cgi );
    cgi = NULL;
    return url;
}

static rc_t STestCheckAcc ( STest * self, const Data * data,
                            bool print, const char * exp, size_t esz )
{
    rc_t rc = 0;
    char response [ 4096 ] = "";
    size_t resp_len = 0;
    const char * url = NULL;
    String acc;
    bool checked = false;

    assert ( self && data );

    memset ( & acc, 0, sizeof acc );
    if ( DataIsAccession ( data ) ) {
        acc = * data -> acc;
        url = STestCallCgi ( self, & acc,
                             response, sizeof response, & resp_len );
    }
    if ( url != NULL ) {
        char * p = string_chr ( url, resp_len - ( url - response ), '|' );
        if ( p == NULL ) {
            OUTMSG (( "UNEXPECTED RESOLVER RESPONSE\n" ));
            rc = RC ( rcExe, rcString ,rcParsing, rcString, rcIncorrect );
        }
        else {
            const String * full = NULL;
            rc_t rc = VPathMakeString ( data -> vpath, & full );
            char * d = NULL;
            if ( rc != 0 )
                OUTMSG ( ( "CANNOT VPathMakeString: %R\n", rc ) );
            d = string_chr ( url, resp_len - ( url - response ), '$' );
            if ( d == NULL )
                d = p;
            while ( d != NULL && d <= p ) {
                if ( ! checked && full != NULL && string_cmp ( full -> addr,
                                    full -> size, url, d - url, d - url ) == 0 )
                {
                    checked = true;
                }
                * d = '\0';
                if ( * url == 'h' ) {
                    Data dt;
                    if ( rc == 0 )
                        rc = DataInit ( & dt, self -> vmgr, url );
                    if ( rc == 0 ) {
                        rc_t r1 = STestCheckUrl ( self, & dt, print, exp, esz );
                        if ( rc == 0 )
                            rc = r1;
                    }
                    DataFini ( & dt );
                }
                if ( d == p )
                    break;
                url = d + 1;
                d = string_chr ( d, resp_len - ( d - response ), '$' );
                if ( d > p )
                    d = p;
            }
            free ( ( void * ) full );
            full = NULL;
        }
    }
    if ( ! checked ) {
        rc_t r1 = STestCheckUrl ( self, data, print, exp, esz );
        if ( rc == 0 )
            rc = r1;
    }
    if ( acc . size != 0 )
        STestEnd ( self, rc == 0 ? eOK : eFAIL, "Access to '%S'", & acc );
    return rc;
}

/******************************************************************************/

static rc_t STestCheckNetwork ( STest * self, const Data * data,
    const char * exp, size_t esz, const Data * data2,
    const char * fmt, ... )
{
    rc_t rc = 0;
    KEndPoint ep;
    char b [ 512 ] = "";
    String host;

    va_list args;
    va_start ( args, fmt );
    rc = string_vprintf ( b, sizeof b, NULL, fmt, args );
    if ( rc != 0 )
        OUTMSG ( ( "CANNOT PREPARE MEGGAGE: %R\n", rc ) );
    va_end ( args );

    assert ( self && data );

    STestStart ( self, true, b );
    rc = VPathGetHost ( data -> vpath, & host );
    if ( rc != 0 )
        OUTMSG ( ( "Cannot VPathGetHost(%R)", rc ) );
    else {
        rc_t r1 = 0;
        uint16_t port = 443;
        STestStart ( self, false, "KNSManagerInitDNSEndpoint(%S:%hu) =",
                                  & host, port );
        rc = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep, & host, port );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            char endpoint [ 1024 ] = "";
            rc_t rx = endpoint_to_string ( endpoint, sizeof endpoint, & ep );
            if ( rx != 0 )
                STestEnd ( self, eEndFAIL, "CANNOT CONVERT TO STRING" );
            else
                STestEnd ( self, eEndOK, "'%s': OK", endpoint );
        }
        port = 80;
        STestStart ( self, false, "KNSManagerInitDNSEndpoint(%S:%hu) =",
                                  & host, port );
        r1 = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep,
                                              & host, port );
        if ( r1 != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", r1 );
        else {
            char endpoint [ 1024 ] = "";
            rc_t rx = endpoint_to_string ( endpoint, sizeof endpoint, & ep );
            if ( rx != 0 )
                STestEnd ( self, eEndFAIL, "CANNOT CONVERT TO STRING" );
            else
                STestEnd ( self, eEndOK, "'%s': OK", endpoint );
        }
        if ( rc == 0 ) {
            rc = STestCheckAcc ( self, data, false, exp, esz );
            if ( data2 != NULL ) {
                rc_t r2 = STestCheckAcc ( self, data2, true, 0, 0 );
                if ( rc == 0 )
                    rc = r2;
            }
        }
        if ( rc == 0 )
            rc = r1;
    }
    STestEnd ( self, rc == 0 ? eOK : eFAIL, b );
    return rc;
}

static const char CLASSNAME [] = "KDirectory";

LIB_EXPORT rc_t CC KDiagnoseMakeExt ( KDiagnose ** test, KConfig * kfg,
    KNSManager * kmgr, VFSManager * vmgr )
{
    rc_t rc = 0;

    KDiagnose * p = NULL;

    if ( test == NULL )
        return  RC ( rcRuntime, rcData, rcCreating, rcParam, rcNull );

    p = calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );

    if ( kfg == NULL ) {
        rc_t r2 = KConfigMake ( & p -> kfg, NULL );
        if ( rc == 0 )
            rc = r2;
    }
    else {
        rc_t r2 = KConfigAddRef ( kfg );
        if ( r2 == 0 )
            p -> kfg = kfg;
        else if ( rc == 0 )
            rc = r2;
    }

    if ( kmgr == NULL ) {
        rc_t r2 = KNSManagerMake ( & p -> kmgr );
        if ( rc == 0 )
            rc = r2;
    }
    else {
        rc_t r2 = KNSManagerAddRef ( kmgr );
        if ( r2 == 0 )
            p -> kmgr = kmgr;
        else if ( rc == 0 )
            rc = r2;
    }

    if ( vmgr == NULL ) {
        rc_t r2 = VFSManagerMake ( & p -> vmgr );
        if ( rc == 0 )
            rc = r2;
    }
    else {
        rc_t r2 = VFSManagerAddRef ( vmgr );
        if ( r2 == 0 )
            p -> vmgr = vmgr;
        else if ( rc == 0 )
            rc = r2;
    }

    if ( rc == 0 ) {
        p -> verbosity = KConfig_Verbosity ( p -> kfg );
        KRefcountInit ( & p -> refcount, 1, CLASSNAME, "init", "" );
        * test = p;
    }
    else
        KDiagnoseRelease ( p );

    return rc;
}

LIB_EXPORT rc_t CC KDiagnoseAddRef ( const KDiagnose * self ) {
    if ( self != NULL )
        switch ( KRefcountAdd ( & self -> refcount, CLASSNAME ) ) {
            case krefLimit:
                return RC ( rcRuntime,
                            rcData, rcAttaching, rcRange, rcExcessive );
        }

    return 0;
}

LIB_EXPORT rc_t CC KDiagnoseRelease ( const KDiagnose * cself ) {
    rc_t rc = 0;

    KDiagnose * self = ( KDiagnose * ) cself;

    if ( self != NULL )
        switch ( KRefcountDrop ( & self -> refcount, CLASSNAME ) ) {
            case krefWhack:
                RELEASE ( KConfig   , self -> kfg );
                RELEASE ( KNSManager, self -> kmgr );
                RELEASE ( VFSManager, self -> vmgr );
                free ( self );
                break;
            case krefNegative:
                return RC ( rcRuntime,
                            rcData, rcReleasing, rcRange, rcExcessive );
        }

    return rc;
}

LIB_EXPORT rc_t CC KDiagnoseRun ( KDiagnose * self, uint64_t tests ) {
    rc_t rc = 0;

    const char exp [] = "NCBI.sra\210\031\003\005\001\0\0\0";
    rc_t r2 = 0;
    STest t;

    if ( self == NULL )
        rc = KDiagnoseMakeExt ( & self, NULL, NULL, NULL );
    else
        rc = KDiagnoseAddRef ( self );
    if ( rc != 0 )
        return rc;

    assert ( self );

    if ( tests == DIAGNOSE_ALL )
        tests = ~ 0;

    STestInit ( & t, self );

    if ( tests & DIAGNOSE_CONFIG ) {
        rc_t r1 = STestStart ( & t, true, "Configuration" );
        STestEnd ( & t, r1 == 0 ? eOK : eFAIL, "Configuration" );
        if ( rc == 0 )
            rc = r1;
    }

    if ( tests & DIAGNOSE_NETWORK ) {
        rc_t r1 = STestStart ( & t, true, "Network" );
        {
#undef  HOST
#define HOST "www.ncbi.nlm.nih.gov"
            String h;
            Data d;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr, "https://" HOST );
            if ( r2 == 0 )
                r2 = STestCheckNetwork ( & t, & d, 0, 0,
                                         NULL, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
        {
#undef  HOST
#define HOST "sra-download.ncbi.nlm.nih.gov"
            String h;
            Data d;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr,
                            "https://" HOST "/srapub/SRR042846" );
            if ( r2 == 0 )
                r2 = STestCheckNetwork ( & t, & d, exp, sizeof exp - 1,
                                         NULL, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
        {
#undef  HOST
#define HOST "ftp-trace.ncbi.nlm.nih.gov"
            String h;
            Data d;
            Data v;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr,
                            "https://" HOST "/sra/refseq/KC702174.1" );
            if ( r2 == 0 )
                r2 = DataInit ( & v, self -> vmgr, "https://" HOST
                                "/sra/sdk/current/sratoolkit.current.version" );
            if ( r2 == 0 )
                r2 = STestCheckNetwork ( & t, & d, exp, sizeof exp - 1,
                                         & v, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
        {
#undef  HOST
#define HOST "gap-download.ncbi.nlm.nih.gov"
            String h;
            Data d;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr, "https://" HOST );
            if ( r2 == 0 )
                r2 = STestCheckNetwork ( & t, & d, NULL, 0, 
                                         NULL, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
        STestEnd ( & t, r1 == 0 ? eOK : eFAIL, "Network" );
        if ( rc == 0 )
            rc = r1;
    }

    STestFini ( & t );
    KDiagnoseRelease ( self );
    return rc;
}
