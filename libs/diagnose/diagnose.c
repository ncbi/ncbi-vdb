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

#include <kapp/main.h> /* Quitting */

#include <kfg/config.h> /* KConfigReadString */

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFile */

#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/out.h> /* KOutMsg */
#include <klib/printf.h> /* string_vprintf */
#include <klib/rc.h>
#include <klib/text.h> /* String */
#include <klib/vector.h> /* Vector */

#include <kns/ascp.h> /* aspera_get */
#include <kns/endpoint.h> /* KNSManagerInitDNSEndpoint */
#include <kns/http.h> /* KHttpRequest */
#include <kns/manager.h> /* KNSManager */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeReliableHttpFile */
#include <kns/stream.h> /* KStream */

#include <kproc/cond.h> /* KConditionRelease */
#include <kproc/lock.h> /* KLockRelease */

#include <vfs/manager.h> /* VFSManagerOpenDirectoryRead */
#include <vfs/path.h> /* VFSManagerMakePath */
#include <vfs/resolver.h> /* VResolverRelease */

#include <strtol.h> /* strtoi64 */

#include <ctype.h> /* isprint */
#include <limits.h> /* PATH_MAX */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t CC OutMsg ( int level, unsigned type,
                        const char * fmt, va_list args )
{
    return KOutVMsg ( fmt, args );
}

static rc_t ( CC * LOGGER )
    ( int level, unsigned type, const char * fmt, va_list args );

LIB_EXPORT rc_t CC KDiagnoseLogHandlerSet ( KDiagnose * self,
        rc_t ( CC * logger ) ( int level, unsigned type,
                               const char * fmt, va_list args )
    )
{
    LOGGER = logger;
    return 0;
}

LIB_EXPORT
rc_t CC KDiagnoseLogHandlerSetKOutMsg ( KDiagnose * self )
{
    return KDiagnoseLogHandlerSet ( self, OutMsg );
}

static rc_t LogOut ( int level, unsigned type, const char * fmt, ... )
{
    rc_t rc = 0;

    va_list args;
    va_start ( args, fmt );

    if ( LOGGER != NULL )
        rc = LOGGER ( level, type, fmt, args );

    va_end ( args );

    return rc;
}

struct KDiagnose {
    atomic32_t refcount;

    KConfig    * kfg;
    KNSManager * kmgr;
    VFSManager * vmgr;

    int verbosity;

    Vector tests;
    Vector errors;

    KDiagnoseTestDesc * desc;

    enum EState {
        eRunning,
        ePaused,
        eCanceled,
    } state;
    KLock * lock;
    KCondition * condition;
};

struct KDiagnoseTest {
    struct KDiagnoseTest * parent;
    const struct KDiagnoseTest * next;
    const struct KDiagnoseTest * nextChild;
    const struct KDiagnoseTest * firstChild;
    struct KDiagnoseTest * crntChild;
    char * name;
    uint32_t level;
    char * message;
    EKDiagTestState state;

    char * number;
    char * numberNode;
};

static void KDiagnoseTestWhack ( KDiagnoseTest * self ) {
    assert ( self );
    free ( self -> name );
    free ( self -> message );
    free ( self -> number );
    free ( self -> numberNode );
    memset ( self, 0, sizeof * self );
    free ( self );
}


LIB_EXPORT rc_t CC KDiagnoseGetTests ( const KDiagnose * self,
                                       const KDiagnoseTest ** test )
{
    if ( test == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * test = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    * test = VectorGet ( & self -> tests, 0 );
    return 0;
}

LIB_EXPORT rc_t CC KDiagnoseTestNext ( const KDiagnoseTest * self,
                                       const KDiagnoseTest ** test )
{
    if ( test == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * test = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    * test = self -> next;
    return 0;
}

LIB_EXPORT rc_t CC KDiagnoseTestChild ( const KDiagnoseTest * self, 
                          uint32_t idx, const KDiagnoseTest ** test )
{
    const KDiagnoseTest * t = NULL; 
    uint32_t i = 0;
    
    if ( test == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * test = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    for ( i = i, t = self -> firstChild; i < idx && t != NULL;
          ++ i, t = t->nextChild );

    * test = t;
    return 0;
}

#define TEST_GET_INT( PROPERTY )       \
    do {                               \
        if ( PROPERTY == NULL )        \
            return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull ); \
        * PROPERTY = 0;             \
        if ( self == NULL )            \
            return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );  \
        * PROPERTY = self -> PROPERTY; \
        return 0;                      \
    } while ( 0 )

#define TEST_GET( PROPERTY )       \
    do {                               \
        if ( PROPERTY == NULL )        \
            return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull ); \
        * PROPERTY = NULL;             \
        if ( self == NULL )            \
            return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );  \
        * PROPERTY = self -> PROPERTY; \
        return 0;                      \
    } while ( 0 )

LIB_EXPORT rc_t CC KDiagnoseTestName ( const KDiagnoseTest * self,
                                       const char ** name )
{   TEST_GET ( name ); }

LIB_EXPORT rc_t CC KDiagnoseTestLevel ( const KDiagnoseTest * self,
                                        uint32_t * level )
{   TEST_GET_INT ( level ); }

LIB_EXPORT rc_t CC KDiagnoseTestNumber ( const KDiagnoseTest * self,
                                         const char ** number )
{   TEST_GET ( number ); }

LIB_EXPORT rc_t CC KDiagnoseTestMessage ( const KDiagnoseTest * self,
                                          const char ** message )
{   TEST_GET ( message ); }

LIB_EXPORT rc_t CC KDiagnoseTestState ( const KDiagnoseTest * self,
                                        EKDiagTestState * state )
{   TEST_GET_INT ( state ); }


struct KDiagnoseError {
    atomic32_t refcount;

    char * message;
};

static const char DIAGNOSERROR_CLSNAME [] = "KDiagnoseError";

LIB_EXPORT
rc_t CC KDiagnoseErrorAddRef ( const KDiagnoseError * self )
{
    if ( self != NULL )
        switch ( KRefcountAdd ( & self -> refcount, DIAGNOSERROR_CLSNAME ) ) {
            case krefLimit:
                return RC ( rcRuntime,
                            rcData, rcAttaching, rcRange, rcExcessive );
        }

    return 0;
}

static void KDiagnoseErrorWhack ( KDiagnoseError * self ) {
    assert ( self );
    free ( self -> message );
    free ( self );
    memset ( self, 0, sizeof * self );
}

LIB_EXPORT
rc_t CC KDiagnoseErrorRelease ( const KDiagnoseError * cself )
{
    rc_t rc = 0;

    KDiagnoseError * self = ( KDiagnoseError * ) cself;

    if ( self != NULL )
        switch ( KRefcountDrop ( & self -> refcount,
                                 DIAGNOSERROR_CLSNAME ) )
        {
            case krefWhack:
                KDiagnoseErrorWhack ( self );
                break;
            case krefNegative:
                return RC ( rcRuntime,
                            rcData, rcReleasing, rcRange, rcExcessive );
        }

    return rc;
}

LIB_EXPORT rc_t CC KDiagnoseErrorGetMsg ( const KDiagnoseError * self,
                                          const char ** message )
{
    if ( message == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * message = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    * message = self -> message;
    return 0;
}

static rc_t KDiagnoseErrorMake ( const KDiagnoseError ** self,
                                 const char * message )
{
    KDiagnoseError * p = NULL;

    assert ( self );

    * self = NULL;

    p = calloc ( 1, sizeof * self );
    if ( p == NULL )
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );

    p -> message = string_dup_measure ( message, NULL );
    if ( p == NULL ) {
        KDiagnoseErrorWhack ( p );
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );
    }

    KRefcountInit ( & p -> refcount, 1, DIAGNOSERROR_CLSNAME, "init", "" );

    * self = p;

    return 0;
}

static
void ( CC * CALL_BACK ) ( EKDiagTestState state, const KDiagnoseTest * test );

LIB_EXPORT rc_t CC KDiagnoseTestHandlerSet ( KDiagnose * self,
    void ( CC * callback ) ( EKDiagTestState state, const KDiagnoseTest * test )
)
{
    CALL_BACK = callback;
    return 0;
}

LIB_EXPORT
rc_t CC KDiagnoseSetVerbosity ( KDiagnose * self, int verbosity )
{
    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    self -> verbosity = verbosity - 1;

    return 0;
}

LIB_EXPORT rc_t CC KDiagnoseGetErrorCount ( const KDiagnose * self,
                                            uint32_t * count )
{
    if ( count == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * count = 0;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    * count = VectorLength ( & self -> errors );
    return 0;
}

LIB_EXPORT rc_t CC KDiagnoseGetError ( const KDiagnose * self, uint32_t idx,
                                       const KDiagnoseError ** error )
{
    rc_t rc = 0;

    const KDiagnoseError * e = NULL;

    if ( error == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * error = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    if ( idx >= VectorLength ( & self -> errors ) )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcInvalid );

    e = VectorGet ( & self -> errors, idx );

    rc = KDiagnoseErrorAddRef ( e );
    if ( rc == 0 )
        * error = e;

    return rc;
}

typedef struct {
    int n [ 7 ];
    int level;
    bool ended;
    bool started;           /*  TestStart did not terminale string by EOL */
    bool failedWhileSilent;

    int verbosity; /* -3    none  ( KVERBOSITY_NONE  )
                      -2    error ( KVERBOSITY_ERROR )
                      -1    info  ( KVERBOSITY_INFO  )
                       0... last printed index of n [] */

    int total;
    int failures;
    KDiagnoseTest * crnt;
    KDiagnoseTest * root;
    Vector * tests;
    Vector * errors;

    KDataBuffer msg;

    const KConfig * kfg;
    const KNSManager * kmgr;
    const VFSManager * vmgr;
    VResolver * resolver;
    VResolverEnableState cacheState;
    KDirectory * dir;

    bool ascpChecked;
    const char * ascp;
    const char * asperaKey;

    KDiagnose * boss;
} STest;

static void STestInit ( STest * self, KDiagnose * test )
{
    rc_t rc = 0;

    assert ( self && test );

    memset ( self, 0, sizeof * self );

    self -> boss = test;

    self -> level = -1;

    self -> kfg = test -> kfg;
    self -> kmgr = test -> kmgr;
    self -> vmgr = test -> vmgr;
    self -> errors = & test -> errors;
    self -> tests = & test -> tests;

    self -> verbosity = test -> verbosity;
    if ( self -> verbosity > 0 )
        -- self -> verbosity;
    else if ( self -> verbosity == 0 ) /* max */
        self -> verbosity = sizeof self -> n / sizeof self -> n [ 0 ] - 1;

    rc = KDirectoryNativeDir ( & self -> dir );
    if ( rc != 0 )
        LogOut ( KVERBOSITY_ERROR, 0, "CANNOT KDirectoryNativeDir: %R\n", rc );

    rc = VFSManagerGetResolver ( self -> vmgr, & self -> resolver);
    if ( rc != 0 )
        LogOut ( KVERBOSITY_ERROR, 0, "CANNOT GetResolver: %R\n", rc );
    else
        self -> cacheState = VResolverCacheEnable ( self -> resolver,
                                                    vrAlwaysEnable );
}

static void STestFini ( STest * self ) {
    rc_t rc = 0;

    assert ( self );

    if ( self -> level >= KVERBOSITY_INFO ) {
        if ( self -> n [ 0 ] == 0 || self -> n [ 1 ] != 0 ||
             self -> level != 0 )
        {
            LogOut ( KVERBOSITY_INFO, 0, "= TEST WAS NOT COMPLETED\n" );
        }

        LogOut ( KVERBOSITY_INFO, 0, "= %d (%d) tests performed, %d failed\n",
                   self -> n [ 0 ], self -> total, self -> failures );

        if ( self -> failures > 0 ) {
            uint32_t i = 0;
            LogOut ( KVERBOSITY_INFO, 0, "Errors:\n" );
            for ( i = 0; i < VectorLength ( self -> errors ); ++ i ) {
                const KDiagnoseError * e = VectorGet ( self -> errors, i );
                assert ( e );
                LogOut ( KVERBOSITY_INFO, 0, " %d: %s\n", i + 1, e -> message );
            }
        }
    }

    VResolverCacheEnable ( self -> resolver, self -> cacheState );

    RELEASE ( KDirectory, self -> dir );    
    RELEASE ( VResolver, self -> resolver );    

    KDataBufferWhack ( & self -> msg );

    free ( ( void * ) self -> ascp );
    free ( ( void * ) self -> asperaKey );

    memset ( self, 0, sizeof * self );
}

static rc_t KDiagnoseCheckState ( KDiagnose * self ) {
    rc_t rc = 0;

    assert ( self );

    if ( ( rc = Quitting () ) != 0 )
        if ( rc == SILENT_RC ( rcExe,
                               rcProcess, rcExecuting, rcProcess, rcCanceled ) )
        {
            LogOut ( KVERBOSITY_INFO, 0,
                     "= Signal caught: CANCELED DIAGNOSTICS\n" );
            self -> state = eCanceled;
            if ( CALL_BACK )
                CALL_BACK ( eKDTS_Canceled, NULL );
        }

    while ( self -> state != eRunning ) {
        rc_t r2;

        rc = KLockAcquire ( self -> lock );
        if ( rc == 0 )
            switch ( self -> state ) {
                case eRunning:
                    break;

                case ePaused:
                    LogOut ( KVERBOSITY_INFO, 0, "= PAUSED DIAGNOSTICS\n" );
                    if ( CALL_BACK )
                        CALL_BACK ( eKDTS_Paused, NULL );

                    rc = KConditionWait ( self -> condition, self -> lock );
                    if ( rc != 0 )
                        LogOut ( KVERBOSITY_INFO, 0,
                                 "= FAILURE DURING PAUSE: %R\n" );
                    else if ( self -> state == eRunning ) {
                        LogOut ( KVERBOSITY_INFO, 0,
                                 "= RESUMED DIAGNOSTICS\n" );
                        if ( CALL_BACK )
                            CALL_BACK ( eKDTS_Resumed, NULL );
                    }

                    break;

                case eCanceled:
                    LogOut ( KVERBOSITY_INFO, 0, "= CANCELED DIAGNOSTICS\n" );
                    if ( rc == 0 )
                        rc = RC ( rcRuntime, rcProcess, rcExecuting,
                                  rcProcess, rcCanceled );
                    if ( CALL_BACK )
                        CALL_BACK ( eKDTS_Canceled, NULL );

                    break;
            }

        r2 = KLockUnlock ( self -> lock );
        if ( rc == 0 && r2 != 0 )
            rc = r2;

        if ( self -> state == eCanceled )
            break;
    }

    return rc;
}

static rc_t STestVStart ( STest * self, bool checking,
                          const char * fmt, va_list args  )
{
    KDiagnoseTest * test = NULL;
    rc_t rc = 0;
    int i = 0;
    char b [ 512 ] = "";
    bool next = false;
    KDataBuffer bf;

//assert(self->boss);rc=KDiagnoseCheckState(self->boss);if ( rc != 0 )return rc;

    memset ( & bf, 0, sizeof bf );
    rc = string_vprintf ( b, sizeof b, NULL, fmt, args );
    if ( rc != 0 ) {
        LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R\n", rc );
        return rc;
    }

    assert ( self );

    test = calloc ( 1, sizeof * test );
    if ( test == NULL )
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );
    test -> name = strdup (b);//TODO
    if ( test -> name == NULL ) {
        free ( test );
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );
    }

    if ( self -> ended ) {
        next = true;
        self -> ended = false;
    }
    else
        ++ self -> level;

    test -> level = self -> level;
    test -> state = eKDTS_Started;

    if ( self -> crnt != NULL ) {
        if ( next ) {
            self -> crnt -> next = test;
            test -> parent = self -> crnt -> parent;
        }
        else {
            if ( self -> crnt -> firstChild == NULL )
                self -> crnt -> firstChild = test;
            else {
                KDiagnoseTest * child = self -> crnt -> crntChild;
                assert ( child );
                child -> nextChild = test;
            }
            self -> crnt -> crntChild = test;
            test -> parent = self -> crnt;
        }
    }
    else
        self -> root = test;
    self -> crnt = test;
    rc = VectorAppend ( self -> tests, NULL, test );
    if ( rc != 0 )
        return rc;

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
        LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R\n", rc );
    else {
        size_t size = 0;
        for ( i = 1; rc == 0 && i <= self -> level; ++ i ) {
            rc = KDataBufferPrintf ( & self -> msg, ".%d", self -> n [ i ] );
            if ( rc != 0 )
                LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R\n", rc );
        }
        assert ( self -> msg . base && self -> msg . elem_count > 2 );
        test -> number = string_dup_measure ( ( char * ) self -> msg . base + 2,
                                              NULL );
        if ( test -> number == NULL )
            return RC ( rcRuntime,
                        rcData, rcAllocating, rcMemory, rcExhausted );

        test -> numberNode = string_dup_measure ( test -> number, & size );
        if ( test -> numberNode == NULL )
            return RC ( rcRuntime,
                        rcData, rcAllocating, rcMemory, rcExhausted );
        else {
            while ( true ) {
                char * c = string_chr ( test -> numberNode, size, '.' );
                if ( c == NULL )
                    break;
                else
                    * c = '/';
            }
        }

    }
    if ( rc == 0 )
        rc = KDataBufferPrintf ( & self -> msg, " %s ", b );
        if ( rc != 0 )
            LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R\n", rc );

    if ( self -> level <= self -> verbosity ) {
        rc = LogOut ( self -> level, 0, "> %d", self -> n [ 0 ] );
        for ( i = 1; i <= self -> level; ++ i )
            rc = LogOut ( self -> level, 0, ".%d", self -> n [ i ] );

        rc = LogOut ( self -> level, 0, " %s%s%s",
                      checking ? "Checking " : "", b, checking ? "..." : " " );
        if ( checking ) {
            if ( self -> level < self -> verbosity ) {
                rc = LogOut ( self -> level, 0, "\n" );
                self -> started = false;
            }
            else {
                rc = LogOut ( self -> level, 0, " " );
                self -> started = true;
            }
        }
    }

    if ( CALL_BACK )
         CALL_BACK ( eKDTS_Started, test );

    return rc;
}

typedef enum {
    eFAIL,
    eOK,
    eMSG,
    eEndFAIL,
    eEndOK,
    eDONE, /* never used */
    eCANCELED,
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
rc=5; /* never used */
break;
}
rc=0;
#endif
    bool failedWhileSilent = self -> failedWhileSilent;
    bool print = false;
    char b [ 512 ] = "";
    size_t num_writ = 0;

    assert ( self );

    if ( ok != eMSG ) {
        if ( self -> ended ) {
            self -> crnt = self -> crnt -> parent;
            self -> n [ self -> level -- ] = 0;
        }
        else {
            self -> ended = true;
            ++ self -> total;
            if ( ok == eFAIL || ok == eEndFAIL )
                ++ self -> failures;
        }
    }

    assert ( self -> level >= 0 );
#ifdef DEPURAR
const char*c=self->msg.base;
#endif
    rc = string_vprintf ( b, sizeof b, & num_writ, fmt, args );
    if ( rc != 0 ) {
        LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R", rc );
        return rc;
    }

    if ( self -> crnt -> message == NULL )
        self -> crnt -> message = string_dup_measure ( b, NULL );
    else {
        size_t m = string_measure ( self -> crnt -> message, NULL);
        size_t s = m + num_writ + 1;
        char * tmp = realloc ( self -> crnt -> message, s );
        if ( tmp == NULL )
            return RC ( rcRuntime,
                        rcData, rcAllocating, rcMemory, rcExhausted );
        self -> crnt -> message = tmp;
        rc = string_printf ( self -> crnt -> message + m, s, NULL, b );
        assert ( rc == 0 );
    }
    if ( ok == eOK ) {
        free ( self -> crnt -> message );
        self -> crnt -> message = string_dup_measure ( "OK", NULL );
    }

    if ( ok == eEndFAIL || ok == eMSG ) {
        rc = KDataBufferPrintf ( & self -> msg, b );
        if ( rc != 0 )
            LogOut ( KVERBOSITY_ERROR, 0, "CANNOT PRINT: %R", rc );
        else if ( ok == eEndFAIL ) {
            const KDiagnoseError * e = NULL;
            rc = KDiagnoseErrorMake ( & e, self -> msg . base );
            if ( rc != 0 )
                return rc;
            rc = VectorAppend ( self -> errors, NULL, e );
            if ( rc != 0 ) {
                LogOut ( KVERBOSITY_ERROR, 0, "CANNOT rcRuntime: %R", rc );
                return rc;
            }
        }
    }

    if ( self -> level > self -> verbosity ) {
        if ( ok == eEndFAIL || ok == eMSG ) {
            if ( ok == eEndFAIL ) {
                rc = KDataBufferPrintf ( & self -> msg, "\n" );
                if ( self -> started ) {
                    LogOut ( KVERBOSITY_ERROR, 0,  "\n" );
                    self -> failedWhileSilent = true;
                    self -> started = false;
                }
                if ( self -> level >= KVERBOSITY_ERROR )
                    LogOut ( KVERBOSITY_ERROR, 0, self -> msg . base );
                assert ( self -> msg . base );
                ( ( char * ) self -> msg . base)  [ 0 ] = '\0';
                self -> msg . elem_count = 0;
            }
        }
    }
    else {
        print = self -> level < self -> verbosity || failedWhileSilent;
        if ( ok == eFAIL || ok == eOK || ok == eDONE || ok == eCANCELED) {
            if ( print ) {
                int i = 0;
                rc = LogOut ( self -> level, 0, "< %d", self -> n [ 0 ] );
                for ( i = 1; i <= self -> level; ++ i )
                    rc = LogOut ( self -> level, 0, ".%d", self -> n [ i ] );
                rc = LogOut ( self -> level, 0, " " );
            }
        }
        if ( print ||
                ( self -> level == self -> verbosity &&
                  ok != eFAIL && ok != eOK ) )
        {
            rc = LogOut ( self -> level, 0, b );
        }

        if ( print )
            switch ( ok ) {
                case eFAIL: rc = LogOut ( self -> level, 0, ": FAILURE\n" );
                            break;
                case eOK  : rc = LogOut ( self -> level, 0, ": OK\n"      );
                            break;
                case eCANCELED:
                case eEndFAIL:
                case eEndOK :
                case eDONE: rc = LogOut ( self -> level, 0, "\n"      );
                            break;
                default   : break;
            }
        else if ( self -> level == self -> verbosity )
            switch ( ok ) {
                case eFAIL: rc = LogOut ( self -> level, 0, "FAILURE\n" );
                            break;
                case eOK  : rc = LogOut ( self -> level, 0, "OK\n"      );
                            break;
                case eEndFAIL:
                case eEndOK :
                case eDONE: rc = LogOut ( self -> level, 0, "\n"      );
                            break;
                default   : break;
            }

        self -> failedWhileSilent = false;
    }

    if ( CALL_BACK && ok != eMSG ) {
        EKDiagTestState state = eKDTS_Succeed;
        switch ( ok ) {
            case eEndOK   : state = eKDTS_Succeed ; break;
            case eOK      : state = eKDTS_Succeed ; break;
            case eFAIL    : state = eKDTS_Failed  ; break;
            case eCANCELED: state = eKDTS_Canceled; break;
            default       : state = eKDTS_Failed  ; break;
        }
        self -> crnt -> state = state;
        CALL_BACK ( state, self -> crnt );
    }

    if ( rc == 0 ) {
        assert ( self -> boss );
        rc = KDiagnoseCheckState ( self -> boss );
    }

    return rc;
}

static bool _RcCanceled ( rc_t rc ) {
    return rc == SILENT_RC ( rcExe,
                             rcProcess, rcExecuting, rcProcess, rcCanceled )
        || rc == SILENT_RC ( rcRuntime,
                             rcProcess, rcExecuting, rcProcess, rcCanceled );
}
static bool STestCanceled ( const STest * self, rc_t rc ) {
    assert ( self && self -> boss );
    return _RcCanceled ( rc ) && self -> boss -> state == eCanceled;
}

static rc_t STestFailure ( const STest * self ) {
    rc_t failure = 0;

    rc_t rc = RC ( rcRuntime, rcProcess, rcExecuting, rcProcess, rcCanceled );
    const KConfigNode * node = NULL;

    assert ( self && self -> crnt && self -> crnt -> numberNode );

    rc = KConfigOpenNodeRead ( self -> kfg, & node,
        "tools/test-sra/diagnose/%s", self -> crnt -> numberNode );
    if ( rc == 0 ) {
        uint64_t result = 0;
        rc = KConfigNodeReadU64 ( node, & result );
        if ( rc == 0 ) {
            failure = ( rc_t ) result;
// if ( _RcCanceled ( failure ) && CALL_BACK ) CALL_BACK ( eKDTS_Canceled,NULL);
        }

        KConfigNodeRelease ( node );
        node = NULL;
    }

    return failure;
}

static rc_t STestEndOr ( STest * self, rc_t * failure,
                         EOK ok, const char * fmt, ...  )
{
    rc_t rc = 0;
    bool canceled = false;

    va_list args;
    va_start ( args, fmt );

    assert ( failure );
    * failure = 0;

    rc = STestVEnd ( self, ok, fmt, args );
    canceled = STestCanceled ( self, rc );

    va_end ( args );

    if ( LOGGER == OutMsg ) {
        assert ( rc == 0 || canceled );
    }

    if ( canceled )
        * failure = rc;
    else if ( rc == 0 )
        * failure = STestFailure ( self );

    return rc;
}

static rc_t STestEnd ( STest * self, EOK ok, const char * fmt, ...  ) {
    rc_t rc = 0;

    va_list args;
    va_start ( args, fmt );

    rc = STestVEnd ( self, ok, fmt, args );

    va_end ( args );

    if ( LOGGER == OutMsg ) {
        assert ( rc == 0 || STestCanceled ( self, rc ) );
    }

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

    if ( LOGGER == OutMsg ) {
        assert ( rc == 0 );
    }

    return rc;
}

static rc_t STestFail ( STest * self, rc_t failure,
                        const char * start,  ...  )
{
    va_list args;

    rc_t rc = 0;

    rc_t r2 = 0;

    va_start ( args, start );

    rc = STestVStart ( self, false, start, args );

    r2 = STestEnd ( self, eEndFAIL, "FAILURE: %R", failure );
    if ( rc == 0 && r2 != 0 )
        rc = r2;

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
        LogOut ( KVERBOSITY_ERROR, 0,
                 "VFSManagerMakePath(%s) = %R\n", path, rc );
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
                LogOut ( KVERBOSITY_ERROR, 0, "Cannot VPathGetPath"
                           "(VFSManagerExtractAccessionOrOID(%R))\n", rc );
            RELEASE ( VPath, vacc );
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

static rc_t STestCheckFile ( STest * self, const String * path,
                             uint64_t * sz, rc_t * rc_read )
{
    rc_t rc = 0;

    const KFile * file = NULL;

    assert ( self && sz && rc_read );

    STestStart ( self, false,
                 "KFile = KNSManagerMakeReliableHttpFile(%S):", path );

    rc = KNSManagerMakeReliableHttpFile ( self -> kmgr, & file, NULL,
                                          HTTP_VERSION, "%S", path );
    if ( rc != 0 )
        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    else {
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
        else {
            STestStart ( self, false, "KFileSize(KFile(%S)) =", path );
            rc = KFileSize ( file, sz );
            if ( rc == 0 )
                STestEndOr ( self, & rc, eEndOK, "%lu: OK", * sz );
            if ( rc != 0 ) {
                if ( _RcCanceled ( rc ) )
                    STestEnd ( self, eCANCELED, "CANCELED" );
                else
                    STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
            }
        }
    }

    if ( rc == 0 ) {
        char buffer [ 304 ] = "";
        uint64_t pos = 0;
        size_t bsize = sizeof buffer;
        size_t num_read = 0;
        if ( * sz < 256 ) {
            pos = 0;
            bsize = ( size_t ) * sz;
        }
        else
            pos = ( * sz - sizeof buffer ) / 2;
        STestStart ( self, false,
                    "KFileRead(%S,%lu,%zu):", path, pos, bsize );
        * rc_read = KFileRead ( file, pos, buffer, bsize, & num_read );
        if ( * rc_read == 0 )
            STestEndOr ( self, rc_read, eEndOK, "OK" );
        if ( * rc_read != 0 ) {
            if ( _RcCanceled ( * rc_read ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", * rc_read );
        }
    }

    KFileRelease ( file );
    file = NULL;

    return rc;
}

static
rc_t STestCheckRanges ( STest * self, const Data * data, uint64_t sz )
{
    rc_t rc = 0;
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
    STestStart ( self, true, "Support of Range requests" );
    rc = VPathGetHost ( data -> vpath, & host );
    if ( rc != 0 )
        STestFail ( self, rc, "VPathGetHost" );
    if ( rc == 0 )
        rc = VPathGetScheme ( data -> vpath, & scheme );
    if ( rc != 0 )
        STestFail ( self, rc, "VPathGetScheme" );
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
            LogOut ( KVERBOSITY_ERROR, 0,
                     "Unexpected scheme '(%S)'\n", & scheme );
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
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        String path;
        rc = VPathGetPath ( data -> vpath, & path );
        if ( rc != 0 )
            STestFail ( self, rc, "VPathGetPath" );
        else {
            rc = KHttpMakeRequest ( http, & req, "%S", & path );
            if ( rc != 0 )
                STestFail ( self, rc, "KHttpMakeRequest(%S)", & path );
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false, "KHttpResult = "
            "KHttpRequestHEAD(KHttpMakeRequest(KClientHttp)):" );
        rc = KHttpRequestHEAD ( req, & rslt );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
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
                rc = STestEnd ( self, eEndOK, "'%.*s': OK",
                                        ( int ) num_read, buffer );
            }
            else {
                STestEnd ( self, eEndFAIL, "'%.*s': FAILURE",
                                        ( int ) num_read, buffer );
                rc = RC ( rcRuntime,
                          rcFile, rcOpening, rcFunction, rcUnsupported );
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
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
            "KHttpResult = KHttpRequestGET(KHttpMakeRequest(KClientHttp)):" );
        rc = KHttpRequestGET ( req, & rslt );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        uint64_t po = 0;
        size_t byte = 0;
        rc = KClientHttpResultRange ( rslt, & po, & byte );
        if ( rc == 0 ) {
            if ( po != pos || ( ebytes > 0 && byte != ebytes ) ) {
                rc = RC ( rcRuntime, rcFile, rcReading, rcRange, rcOutofrange );
                STestFail ( self, rc,
                    "KClientHttpResultRange(KHttpResult,&p,&b): "
                    "got:{%lu,%zu}", pos, ebytes, po, byte );
/*              STestStart ( self, false,
                             "KClientHttpResultRange(KHttpResult,&p,&b):" );
                STestEnd ( self, eEndFAIL, "FAILURE: expected:{%lu,%zu}, "
                            "got:{%lu,%zu}", pos, ebytes, po, byte );*/
            }
        }
        else {
            STestFail ( self, rc, "KClientHttpResultRange(KHttpResult)" );
/*          STestStart ( self, false, "KClientHttpResultRange(KHttpResult):" );
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );*/
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpResultGetHeader(KHttpResult, Content-Range) =" );
        rc = KHttpResultGetHeader ( rslt, "Content-Range",
                                    buffer, sizeof buffer, & num_read );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "'%.*s': OK",
                                    ( int ) num_read, buffer );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    KHttpResultRelease ( rslt );
    rslt = NULL;
    KHttpRequestRelease ( req );
    req = NULL;
    KHttpRelease ( http );
    http = NULL;

    if ( rc == 0 ) {
        rc_t r2 = STestEnd ( self, eOK, "Support of Range requests" );
        if ( r2 != 0 && rc == 0 )
            rc = r2;
    }
    else if ( _RcCanceled ( rc ) )
        STestEnd ( self, eCANCELED, "Support of Range requests: CANCELED" );
    else
        STestEnd ( self, eFAIL, "Support of Range requests" );

    return rc;
}

static KPathType STestRemoveCache ( STest * self, const char * cache ) {
    KPathType type = kptNotFound;

    assert ( self );

    type = KDirectoryPathType ( self -> dir, cache );

    if ( type != kptNotFound ) {
        if ( ( type & ~ kptAlias ) == kptFile ) {
            rc_t rc = KDirectoryRemove ( self -> dir, false, cache );
            if ( rc != 0 )
                STestFail ( self, rc, "KDirectoryRemove(%s)", cache );
            else
                type = kptNotFound;
        }
        else
            LogOut ( KVERBOSITY_ERROR, 0,
                     "UNEXPECTED FILE TYPE OF '%s': %d\n", cache, type );
    }

    return type;
}

static rc_t STestCheckStreamRead ( STest * self, const KStream * stream,
    const char * cache, uint64_t * cacheSize, uint64_t sz, bool print,
    const char * exp, size_t esz )
{
    rc_t rc = 0;
    size_t total = 0;
    char buffer [ 1024 ] = "";
    KFile * out = NULL;
    rc_t rw = 0;
    uint64_t pos = 0;
    assert ( cache && cacheSize );
    if ( cache [ 0 ] != '\0' ) {
        if ( STestRemoveCache ( self, cache ) == kptNotFound ) {
            rw = KDirectoryCreateFile ( self -> dir, & out, false,  0664,
                                        kcmCreate | kcmParents, cache );
            if ( rw != 0 )
                LogOut ( KVERBOSITY_ERROR, 0,
                         "CANNOT CreateFile '%s': %R\n", cache, rw );
        }
    }
    STestStart ( self, false, "KStreamRead(KHttpResult):" );
    while ( rc == 0 ) {
        size_t num_read = 0;
        rc = KStreamRead ( stream, buffer, sizeof buffer, & num_read );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else if ( num_read != 0 ) {
            if ( rw == 0 && out != NULL ) {
                size_t num_writ = 0;
                rw = KFileWriteAll ( out, pos, buffer, num_read, & num_writ );
                if ( rw == 0 ) {
                    assert ( num_writ == num_read );
                    pos += num_writ;
                }
                else
                    LogOut ( KVERBOSITY_ERROR, 0,
                             "CANNOT WRITE TO '%s': %R\n", cache, rw );
            }
            if ( total == 0 && esz > 0 ) {
                int i = 0;
                int s = esz;
                if ( num_read < esz )
                    s = num_read;
                rc = STestEnd ( self, eMSG, "'" );
                if ( rc != 0 ) {
                    if ( STestCanceled ( self, rc ) )
                        STestEnd ( self, eCANCELED, "CANCELED" );
                    else
                        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
                    break;
                }
                for ( i = 0; i < s && rc == 0; ++ i ) {
                    if ( isprint ( ( unsigned char ) buffer [ i ] ) )
                        rc = STestEnd ( self, eMSG, "%c", buffer [ i ] );
                    else if ( buffer [ i ] == 0 )
                        rc = STestEnd ( self, eMSG, "\\0" );
                    else
                        rc = STestEnd ( self, eMSG, "\\%03o",
                                               ( unsigned char ) buffer [ i ] );
                }
                if ( rc == 0 )
                    rc = STestEnd ( self, eMSG, "': " );
                if ( rc != 0 ) {
                    if ( STestCanceled ( self, rc ) )
                        STestEnd ( self, eCANCELED, "CANCELED" );
                    else
                        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
                    break;
                }
                if ( string_cmp ( buffer, num_read, exp, esz, esz ) != 0 ) {
                    STestEnd ( self, eEndFAIL, " FAILURE: bad content" );
                    rc = RC ( rcRuntime,
                              rcFile, rcReading, rcString, rcUnequal );
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
                    rc = STestEnd ( self, eMSG, "%s: ", buffer );
                }
                if ( rc == 0 )
                    rc = STestEnd ( self, eEndOK, "OK" );
                if ( rc != 0 ) {
                    if ( STestCanceled ( self, rc ) )
                        STestEnd ( self, eCANCELED, "CANCELED" );
                    else
                        STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
                }
            }
            else
                STestEnd ( self, eEndFAIL, "%s: SIZE DO NOT MATCH (%zu)\n",
                                           total );
            break;
        }
    }
    {
        rc_t r2 = KFileRelease ( out );
        if ( rw == 0 )
            rw = r2;
    }
    if ( rw == 0 )
        * cacheSize = pos;
    return rc;
}

static rc_t STestCheckHttpUrl ( STest * self, const Data * data,
    const char * cache, uint64_t * cacheSize,
    bool print, const char * exp, size_t esz )
{
    rc_t rc = 0;
    rc_t rc_read = 0;
    rc_t r2 = 0;
    KHttpRequest * req = NULL;
    KHttpResult * rslt = NULL;
    const String * full = NULL;
    uint64_t sz = 0;
    assert ( self && data );
    rc = VPathMakeString ( data -> vpath, & full );
    if ( rc != 0 )
        STestFail ( self, rc, "VPathMakeString" );
    if ( rc == 0 )
        STestStart ( self, true, "Access to '%S'", full );
    if ( rc == 0 )
        rc = STestCheckFile ( self, full, & sz, & rc_read );
    r2 = STestCheckRanges ( self, data, sz );
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpRequest = KNSManagerMakeRequest(%S):", full );
        rc = KNSManagerMakeRequest ( self -> kmgr, & req,
                                     HTTP_VERSION, NULL, "%S", full );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK"  );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        STestStart ( self, false,
                     "KHttpResult = KHttpRequestGET(KHttpRequest):" );
        rc = KHttpRequestGET ( req, & rslt );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK" );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        uint32_t code = 0;
        STestStart ( self, false, "KHttpResultStatus(KHttpResult) =" );
        rc = KHttpResultStatus ( rslt, & code, NULL, 0, NULL );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            rc = STestEnd ( self, eMSG, "%u: ", code );
            if ( rc == 0 )
                if ( code == 200 )
                    STestEnd ( self, eEndOK, "OK" );
                else {
                    STestEnd ( self, eEndFAIL, "FAILURE" );
                    rc = RC ( rcRuntime, rcFile, rcReading, rcFile, rcInvalid );
                }
            else  if ( STestCanceled ( self, rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        KStream * stream = NULL;
        rc = KHttpResultGetInputStream ( rslt, & stream );
        if ( rc != 0 )
            STestFail ( self, rc, "KHttpResultGetInputStream(KHttpResult)" );
        else
            rc = STestCheckStreamRead ( self, stream, cache, cacheSize,
                                        sz, print, exp, esz );
        KStreamRelease ( stream );
        stream = NULL;
    }
    if ( rc == 0 && r2 != 0 )
        rc = r2;
    if ( rc == 0 && rc_read != 0 )
        rc = rc_read;
    KHttpRequestRelease ( req );
    req = NULL;
    KHttpResultRelease ( rslt );
    rslt = NULL;

    if ( rc == 0 )
        STestEnd ( self, eOK, "Access to '%S'", full );
    else if ( _RcCanceled ( rc ) )
        STestEnd ( self, eCANCELED, "Access to '%S': CANCELED", full );
    else
        STestEnd ( self, eFAIL, "Access to '%S'", full );

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
        STestFail ( self, rc, "VPathGetPath" );
        return rc;
    }

    STestStart ( self, false, "VFSManagerOpenDirectoryRead(%S):", & path );
    rc = VFSManagerOpenDirectoryRead ( self -> vmgr, & d, data -> vpath );
    if ( rc == 0 )
        STestEndOr ( self, & rc, eEndOK, "OK"  );
    if ( rc != 0 ) {
        if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "CANCELED" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }

    RELEASE ( KDirectory, d );

    return rc;
}

static rc_t STestCheckUrlImpl ( STest * self, const Data * data,
    const char * cache, uint64_t * cacheSize,
    bool print, const char * exp, size_t esz )
{
    rc_t rc = STestCheckHttpUrl ( self, data,
                                  cache, cacheSize, print, exp, esz );
    rc_t r2 = STestCheckVfsUrl  ( self, data );
    return rc != 0 ? rc : r2;
}

static rc_t STestCheckUrl ( STest * self, const Data * data, const char * cache,
    uint64_t * cacheSize, bool print, const char * exp, size_t esz )
{
    rc_t rc = 0;

    String path;

    assert ( data );

    rc = VPathGetPath ( data -> vpath, & path );
    if ( rc != 0 ) {
        STestFail ( self, rc, "VPathGetPath" );
        return rc;
    }

    if ( path . size == 0 ) /* does not exist */
        return 0;

    return STestCheckUrlImpl ( self, data, cache, cacheSize, print, exp, esz );
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

typedef struct {
    KDataBuffer response;
    uint32_t code;
} Abuse;

static void AbuseInit ( Abuse * self ) {
    assert ( self );
    memset ( self, 0, sizeof * self );
}

static rc_t AbuseFini ( Abuse * self ) {
    rc_t rc = 0;
    assert ( self );
    rc = KDataBufferWhack ( & self -> response );
    memset ( self, 0, sizeof * self );
    return rc;
}

static void AbuseSetStatus ( Abuse * self, uint32_t code ) {
    assert ( self );
    self -> code = code;
}

static rc_t AbuseAdd ( Abuse * self, const char * txt, int sz ) {
    rc_t rc = 0;
    assert ( self );
    if ( rc == 0 )
        return KDataBufferPrintf ( & self -> response,  "%s", txt );
    else
        return KDataBufferPrintf ( & self -> response,  "%.*s", sz, txt );
}

static rc_t TestAbuse ( STest * self, Abuse * test,
                          bool * ok, bool * abuse )
{
    size_t i = 0;
    const char * s = NULL;
    String misuse;
    CONST_STRING ( & misuse,
        "https://misuse.ncbi.nlm.nih.gov/error/abuse.shtml" );
    assert ( self && test && ok && abuse );
    * ok = * abuse = false;
    if ( test -> code == 200 ) {
        * ok = true;
        return 0;
    }
    if ( test -> code != 302 )
        return 0;
    s = test -> response . base;
    while ( true ) {
        const char * h = NULL;
        assert ( test -> response . elem_count >= i );
        h = string_chr ( s + i, ( size_t ) test -> response . elem_count - i,
                         'h' );
        if ( h == NULL )
            break;
        i = h - s;
        if ( i < misuse . size )
            break;
        if ( string_cmp ( h, misuse . size,
                          misuse . addr, misuse . size, misuse . size ) == 0 )
        {
            rc_t rc = 0;
            KHttpRequest * req = NULL;
            KHttpResult * rslt = NULL;
            KStream * stream = NULL;
            KDataBuffer buffer;
            size_t total = 0;
            char * base = NULL;
            rc = KDataBufferMakeBytes ( & buffer, 4096 );
            if ( rc == 0 )
                rc = KNSManagerMakeRequest ( self -> kmgr, & req, HTTP_VERSION,
                                             NULL, "%S", & misuse );
            if ( rc == 0 )
                rc = KHttpRequestGET ( req, & rslt );
            if ( rc == 0 )
                rc = KHttpResultGetInputStream ( rslt, & stream );
            while ( rc == 0 ) {
                size_t num_read = 0;
                uint64_t avail = buffer . elem_count - total;
                if ( avail == 0 ) {
                    rc = KDataBufferResize ( & buffer,
                                             buffer . elem_count + 1024 );
                    if ( rc != 0 )
                        break;
                }
                base = buffer . base;
                rc = KStreamRead ( stream, & base [ total ],
                    ( size_t ) buffer . elem_count - total, & num_read );
                if ( num_read == 0 )
                    break;
                if ( rc != 0 ) /* TBD - look more closely at rc */
                    rc = 0;
                total += num_read;
            }
            RELEASE ( KStream, stream );
            RELEASE ( KHttpResult, rslt );
            RELEASE ( KHttpRequest, req );
            * abuse = true;
            KDataBufferWhack ( & buffer );
            return rc;
        }
        ++ i;
    }
    return 0;
}

static rc_t STestCallCgi ( STest * self, const String * acc, char * response,
    size_t response_sz, size_t * resp_read,
    const char ** url, Abuse * test, bool http )
{
    rc_t rc = 0;

    rc_t rs = 0;
    KHttpRequest * req = NULL;
    const String * cgi = NULL;
    KHttpResult * rslt = NULL;
    KStream * stream = NULL;

    assert ( self && url );

    STestStart ( self, true,
                 "Resolution of %s path to '%S'", http ? "HTTPS": "FASP", acc );

    * url = NULL;

    cgi = KConfig_Resolver ( self -> kfg );
    STestStart ( self, false,
        "KHttpRequest = KNSManagerMakeReliableClientRequest(%S):", cgi );
    rc = KNSManagerMakeReliableClientRequest ( self -> kmgr, & req,
        HTTP_VERSION, NULL, "%S", cgi);
    if ( rc == 0 )
        STestEndOr ( self, & rc, eEndOK, "OK"  );
    if ( rc != 0 ) {
        if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "CANCELED" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }
    if ( rc == 0 ) {
        const char param [] = "accept-proto";
        const char * v = "http,fasp";
        if ( http )
            v = "http,https";
        rc = KHttpRequestAddPostParam ( req, "%s=%s", param, v );
        if ( rc != 0 )
            STestFail ( self, rc,
                "KHttpRequestAddPostParam(%s=%s)", param, v );
    }
    if ( rc == 0 ) {
        const char param [] = "object";
        rc = KHttpRequestAddPostParam ( req, "%s=0||%S", param, acc );
        if ( rc != 0 )
            STestFail ( self, rc,
                        "KHttpRequestAddPostParam(%s=0||%S)", param, acc );
    }
    if ( rc == 0 ) {
        const char param [] = "version";
        rc = KHttpRequestAddPostParam ( req, "%s=3.0", param );
        if ( rc != 0 )
            STestFail ( self, rc, "KHttpRequestAddPostParam(%s=3.0)", param );
    }
    if ( rc == 0 ) {
        STestStart ( self, false, "KHttpRequestPOST(KHttpRequest(%S)):", cgi );
        rc = KHttpRequestPOST ( req, & rslt );
        if ( rc == 0 )
            STestEndOr ( self, & rc, eEndOK, "OK"  );
        if ( rc != 0 ) {
            if ( _RcCanceled ( rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        uint32_t code = 0;
        STestStart ( self, false, "KHttpResultStatus(KHttpResult(%S)) =", cgi );
        rc = KHttpResultStatus ( rslt, & code, NULL, 0, NULL );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        else {
            rc = STestEnd ( self, eMSG, "%u: ", code );
            if ( rc == 0 )
                if ( code == 200 )
                    STestEnd ( self, eEndOK, "OK" );
                else {
                    STestEnd ( self, eEndFAIL, "FAILURE" );
                    rs = RC ( rcRuntime, rcFile, rcReading, rcFile, rcInvalid );
                }
            else  if ( STestCanceled ( self, rc ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
        }
    }
    if ( rc == 0 ) {
        const char name [] = "Location";
        rc_t r2 = 0;
        char buffer [ PATH_MAX ] = "";
        size_t num_read = 0;
        STestStart ( self, false, "KClientHttpResultGetHeader(%s)", name );
        r2 = KClientHttpResultGetHeader ( rslt, name,
                                          buffer, sizeof buffer, & num_read );
        if ( r2 != 0 ) {
            if ( r2 == SILENT_RC ( rcNS,rcTree,rcSearching,rcName,rcNotFound ) )
                rc = STestEnd ( self, eEndOK, ": not found: OK" );
            else
                STestEnd ( self, eEndFAIL, "FAILURE: %R", r2 );
        }
        else
            STestEnd ( self, eEndFAIL, "= '%.*s'", ( int ) num_read, buffer );
    }
    if ( rc == 0 ) {
        rc = KHttpResultGetInputStream ( rslt, & stream );
        if ( rc != 0 )
            STestFail ( self, rc, "KHttpResultGetInputStream" );
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
            rc = STestEnd ( self, eEndOK, "'%s': OK", response );
            if ( rs == 0 ) {
                int i = 0;
                unsigned p = 0;
                for ( i = 0; p < * resp_read ; ++ i ) {
                    char * n = string_chr ( response + p,
                                            * resp_read - p, '|' );
                    if ( n != NULL )
                        p = n - response + 1;
                    if ( i == 6 ) {
                        * url = n + 1;
                        break;
                    }
                }
            }
        }
    }
    if ( rc == 0 && rs != 0 )
        rc = rs;
    KStreamRelease ( stream );
    stream = NULL;
    KHttpResultRelease ( rslt );
    rslt = NULL;
    KHttpRequestRelease ( req );
    req = NULL;
    free ( ( void * ) cgi );
    cgi = NULL;
AbuseSetStatus(test,302);
AbuseAdd(test,"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
"<html><head>\n"
"<title>302 Found</title>\n"
"</head><body>\n"
"<h1>Found</h1>\n"
"<p>The document has moved <a href=\"https://misuse.ncbi.nlm.nih.gov/error/abuse.shtml\">here</a>.</p>\n"
"</body></html>",0);
{bool ok, abuse;
TestAbuse(self,test,&ok,&abuse);}
    if ( rc == 0 )
        rc = STestEnd ( self, eOK,  "Resolution of %s path to '%S'",
                                    http ? "HTTPS": "FASP", acc );
    else if ( _RcCanceled ( rc ) )
        STestEnd ( self, eCANCELED, "Resolution of %s path to '%S': CANCELED",
                                    http ? "HTTPS": "FASP", acc );
    else
        STestEnd ( self, eFAIL,     "Resolution of %s path to '%S'",
                                    http ? "HTTPS": "FASP", acc );
    return rc;
}

static rc_t STestCheckFaspDownload ( STest * self, const char * url,
                             const char * cache, uint64_t * cacheSz )
{
    rc_t rc = 0;

    uint32_t m = 0;
    String fasp;
    String schema;

    assert ( self );

    if ( ! self -> ascpChecked ) {
        ascp_locate ( & self -> ascp, & self -> asperaKey, true, true);
        self -> ascpChecked = true;

        if ( self -> ascp == NULL ) {
            STestStart ( self, false, "ascp download test:" );
            rc = STestEnd ( self, eEndOK, "skipped: ascp not found" );
        }
    }

    if ( self -> ascp == NULL )
        return rc;

    STestStart ( self, false, "ascp download test:" );

    CONST_STRING ( & fasp, "fasp://" );

    m = string_measure ( url, NULL );
    if ( m < fasp . size ) {
        LogOut ( KVERBOSITY_ERROR, 0, "UNEXPECTED SCHEMA IN '%s'", url );
        return 0;
    }

    StringInit( & schema, url, fasp . size, fasp . len );
    if ( ! StringEqual ( & schema, & fasp ) ) {
        LogOut ( KVERBOSITY_ERROR, 0, "UNEXPECTED SCHEMA IN '%s'", url );
        return 0;
    }

    if ( rc == 0 ) {
        STestRemoveCache ( self, cache );
        rc = aspera_get ( self -> ascp, self -> asperaKey,
                          url + fasp . size, cache, 0 );
    }

    if ( rc == 0 )
        rc = KDirectoryFileSize ( self -> dir, cacheSz, cache );
    if ( rc == 0 )
        STestEndOr ( self, & rc, eEndOK, "OK" );
    if ( rc != 0 ) {
        if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "CANCELED" );
        else
            STestEnd ( self, eEndFAIL, "FAILURE: %R", rc );
    }

    return rc;
}

/******************************************************************************/

/*
static rc_t _STestCheckAcc ( STest * self, const Data * data, bool print,
                            const char * exp, size_t esz )
{
    rc_t rc = 0;
    char response [ 4096 ] = "";
    size_t resp_len = 0;
    const char * url = NULL;
    String acc;
    bool checked = false;

    const VPath * vcache = NULL;
    char faspCache [ PATH_MAX ] = "";
    uint64_t faspCacheSize = 0;
    char httpCache [ PATH_MAX ] = "";
    uint64_t httpCacheSize = 0;

    assert ( self && data );

    memset ( & acc, 0, sizeof acc );
    if ( DataIsAccession ( data ) ) {
        Abuse test;
        AbuseInit ( & test );
        acc = * data -> acc;
        rc = STestCallCgi ( self, & acc, response, sizeof response,
                            & resp_len, & url, & test, true | false );
        AbuseFini ( & test );
    }
    if ( acc . size != 0 ) {
        String cache;
        VPath * path = NULL;
        rc_t r2 = VFSManagerMakePath ( self -> vmgr, & path,
                                       "%S", data -> acc );
        if ( r2 == 0 )
            r2 = VResolverQuery ( self -> resolver, eProtocolFasp,
                                  path, NULL, NULL, & vcache);
// TODO: find another cache location if r2 != 0
        if ( r2 == 0 )
            r2 = VPathGetPath ( vcache, & cache );
        if ( r2 == 0 ) {
            rc_t r1 = string_printf ( faspCache, sizeof faspCache, NULL,
                                      "%S.fasp", & cache );
            r2      = string_printf ( httpCache, sizeof httpCache, NULL,
                                      "%S.http", & cache );
            if ( r2 == 0 )
                r2 = r1;
        }
        RELEASE ( VPath, path );
        if ( rc == 0 && r2 != 0 )
            rc = r2;
    }
    if ( url != NULL ) {
        char * p = string_chr ( url, resp_len - ( url - response ), '|' );
        if ( p == NULL ) {
            rc = RC ( rcRuntime, rcString ,rcParsing, rcString, rcIncorrect );
            STestFail ( self, rc, "UNEXPECTED RESOLVER RESPONSE" );
        }
        else {
            const String * full = NULL;
            rc_t r2 = VPathMakeString ( data -> vpath, & full );
            char * d = NULL;
            if ( r2 != 0 )
                LogOut ( KVERBOSITY_ERROR, 0,
                         "CANNOT VPathMakeString: %R\n", r2 );
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
                switch ( * url ) {
                    case 'h': {
                        Data dt;
                        if ( rc == 0 )
                            rc = DataInit ( & dt, self -> vmgr, url );
                        if ( rc == 0 ) {
                            rc_t r1 = STestCheckUrl ( self, & dt,
                                httpCache, & httpCacheSize, print, exp, esz );
                            if ( rc == 0 && r1 != 0 )
                                rc = r1;
                        }
                        DataFini ( & dt );
                        break;
                    }
                    case 'f': {
                        rc_t r1 = STestCheckFaspDownload ( self, url,
                                                   faspCache, & faspCacheSize );
                        if ( rc == 0 && r1 != 0 )
                            rc = r1;
                        break;
                    }
                    default:
                        break;
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
        rc_t r1 = STestCheckUrl ( self, data, httpCache, & httpCacheSize,
                                  print, exp, esz );
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }
    if ( faspCacheSize != 0 && httpCacheSize != 0 ) {
        uint64_t pos = 0;
        rc_t r1 = 0;
        STestStart ( self, false, "HTTP vs ASCP download:" );
        if ( faspCacheSize != httpCacheSize ) {
            r1 = RC ( rcRuntime, rcFile, rcComparing, rcSize, rcUnequal );
            STestEnd ( self, eEndFAIL, "FAILURE: size does not match: "
                       "ascp(%lu)/http(%lu)", faspCacheSize, httpCacheSize );
        }
        else {
            const KFile * ascp = NULL;
            const KFile * http = NULL;
            rc_t r1 = KDirectoryOpenFileRead ( self -> dir, & ascp, faspCache );
            if ( r1 != 0 )
                LogOut ( KVERBOSITY_ERROR, 0,
                         "KDirectoryOpenFileRead(%s)=%R\n", faspCache, r1 );
            else {
                r1 = KDirectoryOpenFileRead ( self -> dir, & http, httpCache );
                if ( r1 != 0 )
                    LogOut ( KVERBOSITY_ERROR, 0,
                             "KDirectoryOpenFileRead(%s)=%R\n", httpCache, r1 );
            }
            if ( r1 == 0 ) {
                char bAscp [ 1024 ] = "";
                char bHttp [ 1024 ] = "";
                size_t ascp_read = 0;
                size_t http_read = 0;
                while ( r1 == 0 ) {
                    r1 = KFileReadAll ( ascp, pos, bAscp, sizeof bAscp,
                                        & ascp_read );
                    if ( r1 != 0 ) {
                        STestEnd ( self, eEndFAIL, "FAILURE to read '%s': %R",
                                                   faspCache, r1 );
                        break;
                    }
                    r1 = KFileReadAll ( http, pos, bHttp, sizeof bHttp,
                                        & http_read );
                    if ( r1 != 0 ) {
                        STestEnd ( self, eEndFAIL, "FAILURE to read '%s': %R",
                                                   httpCache, r1 );
                        break;
                    }
                    else if ( ascp_read != http_read ) {
                        r1 = RC (
                            rcRuntime, rcFile, rcComparing, rcSize, rcUnequal );
                        STestEnd ( self, eEndFAIL,
                            "FAILURE to read the same amount from files" );
                        break;
                    }
                    else if ( ascp_read == 0 )
                        break;
                    else {
                        pos += ascp_read;
                        if ( string_cmp ( bAscp, ascp_read,
                                          bHttp, http_read, ascp_read ) != 0 )
                        {
                            r1 = RC ( rcRuntime,
                                      rcFile, rcComparing, rcData, rcUnequal );
                            STestEnd ( self, eEndFAIL,
                                       "FAILURE: files are different" );
                            break;
                        }
                    }
                }
            }
            RELEASE ( KFile, ascp );
            RELEASE ( KFile, http );
        }
        if ( r1 == 0 ) {
            rc_t r2 = 0;
            r1 = KDirectoryRemove ( self -> dir, false, faspCache );
            if ( r1 != 0 )
                STestEnd ( self, eEndFAIL, "FAILURE: cannot remove '%s': %R",
                                           faspCache, r1 );
            r2 = KDirectoryRemove ( self -> dir, false, httpCache );
            if ( r2 != 0 ) {
                if ( r1 == 0 ) {
                    r1 = r2;
                    STestEnd ( self, eEndFAIL,
                        "FAILURE: cannot remove '%s': %R", httpCache, r1 );
                }
                else
                    LogOut ( KVERBOSITY_ERROR, 0,
                             "Cannot remove '%s': %R\n", httpCache, r2 );
            }
            if ( r1 == 0 )
                rc = STestEnd ( self, eEndOK, "%lu bytes compared: OK", pos );
            else if ( rc == 0 )
                rc = r1;
        }
    }

    if ( acc . size != 0 ) {
        if ( rc == 0 )
            rc = STestEnd ( self, eOK, "Access to '%S'", & acc );
        else if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "Access to '%S': CANCELED", & acc );
        else
            STestEnd ( self, eFAIL, "Access to '%S'", & acc );
    }

    RELEASE ( VPath, vcache );
    return rc;
}

static rc_t _STestCheckNetwork ( STest * self, const Data * data,
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
        STestFail ( self, rc, "CANNOT PREPARE MESSAGE" );
    va_end ( args );

    assert ( self && data );

    STestStart ( self, true, b );
    rc = VPathGetHost ( data -> vpath, & host );
    if ( rc != 0 )
        STestFail ( self, rc, "VPathGetHost" );
    else {
        rc_t r1 = 0;
        uint16_t port = 443;
        STestStart ( self, false, "KNSManagerInitDNSEndpoint(%S:%hu)",
                                  & host, port );
        rc = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep, & host, port );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, ": FAILURE: %R", rc );
        else {
            char endpoint [ 1024 ] = "";
            rc_t rx = endpoint_to_string ( endpoint, sizeof endpoint, & ep );
            if ( rx == 0 )
                STestEndOr ( self, & rx, eEndOK, "= '%s': OK", endpoint );
            if ( rx != 0 ) {
                if ( _RcCanceled ( rx ) )
                    STestEnd ( self, eCANCELED, "CANCELED" );
                else
                    STestEnd ( self, eEndFAIL,
                               "CANNOT CONVERT TO STRING: %R", rx );
            }
        }
        port = 80;
        STestStart ( self, false, "KNSManagerInitDNSEndpoint(%S:%hu)",
                                  & host, port );
        r1 = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep,
                                              & host, port );
        if ( r1 != 0 )
            STestEnd ( self, eEndFAIL, "FAILURE: %R", r1 );
        else {
            char endpoint [ 1024 ] = "";
            rc_t rx = endpoint_to_string ( endpoint, sizeof endpoint, & ep );
            if ( rx == 0 )
                STestEndOr ( self, & rx, eEndOK, "= '%s': OK", endpoint );
            if ( rx != 0 ) {
                if ( _RcCanceled ( rx ) )
                    STestEnd ( self, eCANCELED, "CANCELED" );
                else
                    STestEnd ( self, eEndFAIL,
                               "CANNOT CONVERT TO STRING: %R", rx );
            }
        }
        rc = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep, & host, port );
        if ( rc == 0 ) {
            rc = _STestCheckAcc ( self, data, false, exp, esz );
            if ( data2 != NULL ) {
                rc_t r2 = _STestCheckAcc ( self, data2, true, 0, 0 );
                if ( rc == 0 && r2 != 0 )
                    rc = r2;
            }
        }
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }

    if ( rc == 0 )
        rc = STestEnd ( self, eOK, b );
    else  if ( _RcCanceled ( rc ) )
        STestEnd ( self, eCANCELED, "%s: CANCELED", b );
    else
        STestEnd ( self, eFAIL, b );
    return rc;
}
*/

struct KDiagnoseTestDesc {
    const char * name;
    const char * desc;
    uint64_t code;
    uint32_t level;
    KDiagnoseTestDesc * next;
    KDiagnoseTestDesc * child;
    const KDiagnoseTestDesc * depends;
};

static rc_t KDiagnoseTestDescRelease ( KDiagnoseTestDesc * self ) {
    if ( self != NULL ) {
        if ( self -> child )
            KDiagnoseTestDescRelease ( self -> child );

        if ( self -> next )
            KDiagnoseTestDescRelease ( self -> next );

        memset ( self, 0, sizeof * self );

        free ( self );
    }

    return 0;
}

static rc_t KDiagnoseTestDescMake ( KDiagnoseTestDesc ** self,
    uint32_t level, const char * name, uint64_t code )
{
    assert ( self );

    * self = calloc ( 1, sizeof ** self );

    if ( * self == NULL )
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );
    else {
        ( * self ) -> name  = name;
        ( * self ) -> desc  = "";
        ( * self ) -> code  = code;
        ( * self ) -> level = level;

        return 0;
    }
}

static rc_t KDiagnoseMakeDesc ( KDiagnose * self ) {
    rc_t rc = 0;
    KDiagnoseTestDesc * root = NULL;
    KDiagnoseTestDesc * kfg = NULL;
    KDiagnoseTestDesc * kfgCommon = NULL;
    KDiagnoseTestDesc * net = NULL;
    KDiagnoseTestDesc * netNcbi = NULL;
    KDiagnoseTestDesc * netHttp = NULL;
    KDiagnoseTestDesc * netAscp = NULL;
    assert ( self );
    if ( rc == 0 )
        rc = KDiagnoseTestDescMake ( & root, 0, "System",
                                     DIAGNOSE_CONFIG | DIAGNOSE_NETWORK );
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & kfg, 1, "Configuration",
                                     DIAGNOSE_CONFIG );
        if ( rc == 0 )
            root -> child = kfg;
    }
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & kfgCommon, 2, "Common configuration",
                                     DIAGNOSE_CONFIG_COMMON );
        if ( rc == 0 )
            kfg -> child = kfgCommon;
    }
    if ( rc == 0 ) {
        KDiagnoseTestDesc * kfgGap = NULL;
        rc = KDiagnoseTestDescMake ( & kfgGap, 2, "DbGaP configuration",
                                     DIAGNOSE_CONFIG_DB_GAP );
        if ( rc == 0 ) {
            kfgCommon -> next = kfgGap;
            kfgGap -> depends = kfgCommon;
        }
    }
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & net, 1, "Network", DIAGNOSE_NETWORK );
        if ( rc == 0 )
            kfg -> next = net;
    }
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & netNcbi, 2, "Access to NCBI",
                                     DIAGNOSE_NETWORK_NCBI );
        if ( rc == 0 ) {
            net -> child = netNcbi;
            netNcbi -> depends = kfgCommon;
        }
    }
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & netHttp, 2, "HTTPS download",
                                     DIAGNOSE_NETWORK_HTTPS );
        if ( rc == 0 ) {
            netNcbi -> next = netHttp;
            netHttp -> depends = netNcbi;
        }
    }
    if ( rc == 0 ) {
        rc = KDiagnoseTestDescMake ( & netAscp, 2, "Aspera download",
                                     DIAGNOSE_NETWORK_ASPERA );
        if ( rc == 0 ) {
            netHttp -> next = netAscp;
            netAscp -> depends = netNcbi;
        }
    }
    if ( rc == 0 ) {
        KDiagnoseTestDesc * gap = NULL;
        rc = KDiagnoseTestDescMake ( & gap, 2, "Access to dbGaP",
                                     DIAGNOSE_NETWORK_DB_GAP );
        if ( rc == 0 ) {
            netAscp -> next = gap;
            gap -> depends = netNcbi;
        }
    }
    if ( rc != 0 )
        KDiagnoseTestDescRelease ( root );
    else
        self -> desc = root;
    return rc;
}

static const char DIAGNOSE_CLSNAME [] = "KDiagnose";

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
        if ( rc == 0 && r2 != 0 )
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
        if ( rc == 0 && r2 != 0 )
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
        if ( rc == 0 && r2 != 0 )
            rc = r2;
    }
    else {
        rc_t r2 = VFSManagerAddRef ( vmgr );
        if ( r2 == 0 )
            p -> vmgr = vmgr;
        else if ( rc == 0 )
            rc = r2;
    }

    if ( rc == 0 )
        rc = KLockMake ( & p -> lock );
    if ( rc == 0 )
        rc = KConditionMake ( & p -> condition );

    if ( rc == 0 )
        rc = KDiagnoseMakeDesc ( p );

    if ( rc == 0 ) {
        p -> verbosity = KConfig_Verbosity ( p -> kfg );
        KRefcountInit ( & p -> refcount, 1, DIAGNOSE_CLSNAME, "init", "" );
        * test = p;
    }
    else
        KDiagnoseRelease ( p );

    return rc;
}

LIB_EXPORT rc_t CC KDiagnoseAddRef ( const KDiagnose * self ) {
    if ( self != NULL )
        switch ( KRefcountAdd ( & self -> refcount, DIAGNOSE_CLSNAME ) ) {
            case krefLimit:
                return RC ( rcRuntime,
                            rcData, rcAttaching, rcRange, rcExcessive );
        }

    return 0;
}

static void CC whack ( void * item, void * data ) { free ( item ); }
static void CC testsWhack ( void * item, void * data )
{   KDiagnoseTestWhack ( item ); }

LIB_EXPORT rc_t CC KDiagnoseRelease ( const KDiagnose * cself ) {
    rc_t rc = 0;

    KDiagnose * self = ( KDiagnose * ) cself;

    if ( self != NULL )
        switch ( KRefcountDrop ( & self -> refcount, DIAGNOSE_CLSNAME ) ) {
            case krefWhack:
                RELEASE ( KConfig   , self -> kfg );
                RELEASE ( KNSManager, self -> kmgr );
                RELEASE ( VFSManager, self -> vmgr );
                RELEASE ( KLock     , self -> lock );
                RELEASE ( KCondition, self -> condition );
                VectorWhack ( & self -> tests , & testsWhack, NULL );
                VectorWhack ( & self -> errors, & whack     , NULL );

                RELEASE ( KDiagnoseTestDesc, self -> desc  );

                free ( self );
                break;
            case krefNegative:
                return RC ( rcRuntime,
                            rcData, rcReleasing, rcRange, rcExcessive );
        }

    return rc;
}

static rc_t _KDiagnoseSetState ( KDiagnose * self, enum EState state ) {
    rc_t rc = 0;
    rc_t r2 = 0;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    rc = KLockAcquire ( self -> lock );

    if ( rc == 0 ) {
        self -> state = state;

        rc = KConditionSignal ( self -> condition );
    }

    r2 = KLockUnlock ( self -> lock );
    if ( rc == 0 && r2 != 0 )
        rc = r2;

    return rc;
}

LIB_EXPORT rc_t CC KDiagnosePause  ( KDiagnose * self ) {
    return _KDiagnoseSetState ( self, ePaused );
}

LIB_EXPORT rc_t CC KDiagnoseResume ( KDiagnose * self ) {
    return _KDiagnoseSetState ( self, eRunning );
}

LIB_EXPORT rc_t CC KDiagnoseCancel ( KDiagnose * self ) {
    return _KDiagnoseSetState ( self, eCanceled );
}

LIB_EXPORT rc_t CC KDiagnoseGetDesc ( const KDiagnose * self,
    const KDiagnoseTestDesc ** desc )
{
    if ( desc == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcParam, rcNull );

    * desc = NULL;

    if ( self == NULL )
        return RC ( rcRuntime, rcData, rcAccessing, rcSelf, rcNull );

    * desc = self -> desc;

    return 0;
}

static rc_t STestKNSManagerInitDNSEndpoint ( STest * self, const String * host,
                                             uint16_t port )
{
    rc_t rc = 0;

    KEndPoint ep;

    STestStart ( self, false, "KNSManagerInitDNSEndpoint(%S:%hu)",
                              host, port );
    rc = KNSManagerInitDNSEndpoint ( self -> kmgr, & ep, host, port );
    if ( rc != 0 )
        STestEnd ( self, eEndFAIL, ": FAILURE: %R", rc );
    else {
        char endpoint [ 1024 ] = "";
        rc_t rx = endpoint_to_string ( endpoint, sizeof endpoint, & ep );
        if ( rx == 0 )
            STestEndOr ( self, & rx, eEndOK, "= '%s': OK", endpoint );
        if ( rx != 0 ) {
            if ( _RcCanceled ( rx ) )
                STestEnd ( self, eCANCELED, "CANCELED" );
            else
                STestEnd ( self, eEndFAIL, "CANNOT CONVERT TO STRING: %R", rx );
        }
    }

    return rc;
}

static rc_t STestCheckNcbiAccess ( STest * self ) {
    rc_t rc = 0;

    String www;
    CONST_STRING ( & www, "www.ncbi.nlm.nih.gov" );

    {
        rc_t r1 = STestKNSManagerInitDNSEndpoint ( self, & www, 80 );
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }

    {
        rc_t r1 = STestKNSManagerInitDNSEndpoint ( self, & www, 443 );
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }

    {
        rc_t r1 = 0;
        String ftp;
#define FTP "ftp-trace.ncbi.nlm.nih.gov"
        CONST_STRING ( & ftp, FTP );
        r1 = STestKNSManagerInitDNSEndpoint ( self, & ftp, 443 );
        if ( r1 == 0 ) {
            Data v;
            r1 = DataInit ( & v, self -> vmgr, "https://" FTP
                            "/sra/sdk/current/sratoolkit.current.version" );
            if ( r1 == 0 ) {
                uint64_t s = 0;
                r1 = STestCheckUrl ( self, & v, "", & s, true, 0, 0 );
            }
            DataFini ( & v );
            if ( rc == 0 && r1 != 0 )
                rc = r1;
        }
        else if ( rc == 0 )
            rc = r1;
    }

    return rc;
}

static rc_t STestCheckHttp ( STest * self, const String * acc, bool print,
    char * downloaded, size_t sDownloaded, uint64_t * downloadedSize,
    const char * exp, size_t esz )
{
    rc_t rc = 0;
    char response [ 4096 ] = "";
    size_t resp_len = 0;
    const char * url = NULL;
    bool failed = false;
    Abuse test;
    AbuseInit ( & test );
    STestStart ( self, true, "HTTPS access to '%S'", acc );
    rc = STestCallCgi ( self, acc, response, sizeof response,
                        & resp_len, & url, & test, true );
    AbuseFini ( & test );
    if ( rc == 0 ) {
        String cache;
        VPath * path = NULL;
        rc_t r2 = VFSManagerMakePath ( self -> vmgr, & path, "%S", acc );
        const VPath * vcache = NULL;
        if ( r2 == 0 )
            r2 = VResolverQuery ( self -> resolver, eProtocolHttps,
                                  path, NULL, NULL, & vcache);
// TODO: find another cache location if r2 != 0
        if ( r2 == 0 )
            r2 = VPathGetPath ( vcache, & cache );
        if ( r2 == 0 ) {
            r2      = string_printf ( downloaded, sDownloaded, NULL,
                                      "%S.http", & cache );
            RELEASE ( VPath, vcache );
        }
        RELEASE ( VPath, path );
        if ( rc == 0 && r2 != 0 )
            rc = r2;
    }
    if ( rc == 0 && url != NULL ) {
        char * p = string_chr ( url, resp_len - ( url - response ), '|' );
        if ( p == NULL ) {
            rc = RC ( rcRuntime, rcString ,rcParsing, rcString, rcIncorrect );
            STestFail ( self, rc, "UNEXPECTED RESOLVER RESPONSE" );
            failed = true;
        }
        else {
            Data dt;
            * p = '\0';
            rc = DataInit ( & dt, self -> vmgr, url );
            if ( rc == 0 ) {
                rc_t r1 = STestCheckUrl ( self, & dt,
                    downloaded, downloadedSize, print, exp, esz );
                if ( rc == 0 && r1 != 0 ) {
                    assert ( downloaded );
                    * downloaded = '\0';
                    rc = r1;
                }
            }
            DataFini ( & dt );
        }
    }
    if ( ! failed ) {
        if ( rc == 0 )
            rc = STestEnd ( self, eOK,  "HTTPS access to '%S'", acc );
        else if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "HTTPS access to '%S': CANCELED", acc );
        else
            STestEnd ( self, eFAIL,     "HTTPS access to '%S'", acc );
    }
    return rc;
}

static rc_t STestCheckFasp ( STest * self, const String * acc, bool print,
    char * downloaded, size_t sDownloaded, uint64_t * downloadedSize,
    const char * exp, size_t esz )
{
    rc_t rc = 0;
    char response [ 4096 ] = "";
    size_t resp_len = 0;
    const char * url = NULL;
    bool failed = false;
    Abuse test;
    AbuseInit ( & test );
    STestStart ( self, true, "Aspera access to '%S'", acc );
    rc = STestCallCgi ( self, acc, response, sizeof response,
                        & resp_len, & url, & test, false );
    AbuseFini ( & test );
    if ( rc == 0 ) {
        String cache;
        VPath * path = NULL;
        rc_t r2 = VFSManagerMakePath ( self -> vmgr, & path, "%S", acc );
        const VPath * vcache = NULL;
        if ( r2 == 0 )
            r2 = VResolverQuery ( self -> resolver, eProtocolFasp,
                                  path, NULL, NULL, & vcache);
// TODO: find another cache location if r2 != 0
        if ( r2 == 0 )
            r2 = VPathGetPath ( vcache, & cache );
        if ( r2 == 0 ) {
            r2 = string_printf ( downloaded, sDownloaded, NULL,
                                 "%S.fasp", & cache );
            RELEASE ( VPath, vcache );
        }
        RELEASE ( VPath, path );
        if ( rc == 0 && r2 != 0 )
            rc = r2;
    }
    if ( rc == 0 && url != NULL ) {
        char * p = string_chr ( url, resp_len - ( url - response ), '|' );
        if ( p == NULL ) {
            rc = RC ( rcRuntime, rcString ,rcParsing, rcString, rcIncorrect );
            STestFail ( self, rc, "UNEXPECTED RESOLVER RESPONSE" );
            failed = true;
        }
        else {
            Data dt;
            * p = '\0';
            rc = DataInit ( & dt, self -> vmgr, url );
            if ( rc == 0 ) {
                rc_t r1 = STestCheckFaspDownload ( self, url, downloaded,
                                                   downloadedSize );
                if ( rc == 0 && r1 != 0 ) {
                    assert ( downloaded );
                    * downloaded = '\0';
                    rc = r1;
                }
            }
            DataFini ( & dt );
        }
    }
    if ( ! failed ) {
        if ( rc == 0 )
            rc = STestEnd ( self, eOK,  "Aspera access to '%S'", acc );
        else if ( _RcCanceled ( rc ) )
            STestEnd ( self, eCANCELED, "Aspera access to '%S': CANCELED",
                                        acc );
        else
            STestEnd ( self, eFAIL,     "Aspera access to '%S'", acc );
    }
    return rc;
}

static
rc_t STestHttpVsFasp ( STest * self, const char * http, uint64_t httpSize,
                       const char * fasp, uint64_t faspSize )
{
    rc_t rc = 0;
    uint64_t pos = 0;
    const KFile * ascpF = NULL;
    const KFile * httpF = NULL;
    STestStart ( self, false, "HTTP vs ASCP download:" );
    if ( httpSize != faspSize ) {
        rc = RC ( rcRuntime, rcFile, rcComparing, rcSize, rcUnequal );
        STestEnd ( self, eEndFAIL, "FAILURE: size does not match: "
                   "http(%lu)/ascp(%lu)", httpSize, faspSize );
    }
    else {
        rc = KDirectoryOpenFileRead ( self -> dir, & httpF, http );
        if ( rc != 0 )
            STestEnd ( self, eEndFAIL, "cannot open '%s'; %R", http, rc );
        else {
            rc = KDirectoryOpenFileRead ( self -> dir, & ascpF, fasp );
            if ( rc != 0 )
                STestEnd ( self, eEndFAIL, "cannot open '%s'; %R", fasp, rc );
        }
    }
    if ( rc == 0 ) {
        char bAscp [ 1024 ] = "";
        char bHttp [ 1024 ] = "";
        size_t ascp_read = 0;
        size_t http_read = 0;
        while ( rc == 0 ) {
            rc = KFileReadAll ( ascpF, pos, bAscp, sizeof bAscp, & ascp_read );
            if ( rc != 0 ) {
                STestEnd ( self, eEndFAIL, "FAILURE to read '%s': %R",
                                          fasp, rc );
                break;
            }
            rc = KFileReadAll ( httpF, pos, bHttp, sizeof bHttp, & http_read );
            if ( rc != 0 ) {
                STestEnd ( self, eEndFAIL, "FAILURE to read '%s': %R",
                                          http, rc );
                break;
            }
            else if ( ascp_read != http_read ) {
                rc = RC ( rcRuntime, rcFile, rcComparing, rcSize, rcUnequal );
                STestEnd ( self, eEndFAIL,
                           "FAILURE to read the same amount from files" );
                break;
            }
            else if ( ascp_read == 0 )
                break;
            else {
                pos += ascp_read;
                if ( string_cmp ( bAscp, ascp_read,
                                  bHttp, http_read, ascp_read ) != 0 )
                {
                    rc = RC ( rcRuntime,
                              rcFile, rcComparing, rcData, rcUnequal );
                    STestEnd ( self, eEndFAIL, "FAILURE: files are different" );
                    break;
                }
            }
        }
    }
    RELEASE ( KFile, ascpF );
    RELEASE ( KFile, httpF );
    if ( rc == 0 )
        rc = STestEnd ( self, eEndOK, "%lu bytes compared: OK", pos );
    return rc;
}

LIB_EXPORT rc_t CC KDiagnoseRun ( KDiagnose * self, uint64_t tests,
                                  uint32_t projectId, ... )
{
    rc_t rc = 0;

    const char exp [] = "NCBI.sra\210\031\003\005\001\0\0\0";
    STest t;

    if ( self == NULL )
        rc = KDiagnoseMakeExt ( & self, NULL, NULL, NULL );
    else
        rc = KDiagnoseAddRef ( self );
    if ( rc != 0 )
        return rc;

    assert ( self );

    STestInit ( & t, self );

    STestStart ( & t, true, "System" );

    if ( tests & DIAGNOSE_NETWORK_HTTPS  ||
         tests & DIAGNOSE_NETWORK_ASPERA ||
         tests & DIAGNOSE_NETWORK_DB_GAP )
    {   tests |= DIAGNOSE_NETWORK_NCBI; }
    if ( tests & DIAGNOSE_NETWORK_NCBI )
        tests |= DIAGNOSE_CONFIG_COMMON;

    if ( tests & DIAGNOSE_CONFIG ) {
        rc_t r1 = 0;
        if ( tests & DIAGNOSE_CONFIG_DB_GAP )
            tests |= DIAGNOSE_CONFIG_COMMON;
        STestStart ( & t, true, "Configuration" );
        if ( tests & DIAGNOSE_CONFIG_COMMON && ! _RcCanceled ( r1 ) ) {
            rc_t r2 = 0;
            STestStart ( & t, true,        "Common configuration" );
            if ( r2 == 0 )
                r2 = STestEnd ( & t, eOK,  "Common configuration" );
            else {
                if ( _RcCanceled ( r2 ) )
                    STestEnd ( & t, eCANCELED,
                                           "Common configuration: CANCELED" );
                else
                    STestEnd ( & t, eFAIL, "Common configuration" );
            }
            if ( r1 == 0 && r2 != 0 )
                r1 = r2;
        }
        if ( tests & DIAGNOSE_CONFIG_DB_GAP && ! _RcCanceled ( r1 ) ) {
            rc_t r2 = 0;
            STestStart ( & t, true,        "DbGaP configuration" );
            if ( r2 == 0 )
                r2 = STestEnd ( & t, eOK,  "DbGaP configuration" );
            else {
                if ( _RcCanceled ( r2 ) )
                    STestEnd ( & t, eCANCELED,
                                           "DbGaP configuration: CANCELED" );
                else
                    STestEnd ( & t, eFAIL, "DbGaP configuration" );
            }
            if ( r1 == 0 && r2 != 0 )
                r1 = r2;
        }
        if ( r1 == 0 )
            r1 = STestEnd ( & t, eOK,      "Configuration" );
        else {
            if ( _RcCanceled ( r1 ) )
                STestEnd ( & t, eCANCELED, "Configuration: CANCELED" );
            else
                STestEnd ( & t, eFAIL,     "Configuration" );
        }
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }

    if ( tests & DIAGNOSE_NETWORK && ! _RcCanceled ( rc ) ) {
        rc_t r1 = 0;
        String run;
        char http [ PATH_MAX ] = "";
        uint64_t httpSize = 0;
        CONST_STRING ( & run, "SRR029074" );
        if ( tests & DIAGNOSE_NETWORK_HTTPS || tests & DIAGNOSE_NETWORK_ASPERA )
            tests |= DIAGNOSE_NETWORK_NCBI;
        STestStart ( & t, true, "Network" );
        if ( tests & DIAGNOSE_NETWORK_NCBI && ! _RcCanceled ( r1 ) ) {
            rc_t r2 = 0;
            STestStart ( & t, true,        "Access to NCBI" );
            r2 = STestCheckNcbiAccess ( & t );
            if ( r2 == 0 )
                r2 = STestEnd ( & t, eOK,      "Access to NCBI" );
            else {
                if ( _RcCanceled ( r2 ) )
                    STestEnd ( & t, eCANCELED, "Access to NCBI: CANCELED" );
                else
                    STestEnd ( & t, eFAIL,     "Access to NCBI" );
            }
            if ( r1 == 0 && r2 != 0 )
                r1 = r2;
        }
        if ( tests & DIAGNOSE_NETWORK_HTTPS && ! _RcCanceled ( r1 ) ) {
            rc_t r2 = 0;
            STestStart ( & t, true,        "HTTPS download" );
            r2 = STestCheckHttp ( & t, & run, false, http, sizeof http,
                                  & httpSize, exp, sizeof exp - 1 );
            if ( r2 == 0 )
                r2 = STestEnd ( & t, eOK,      "HTTPS download" );
            else {
                if ( _RcCanceled ( r2 ) )
                    STestEnd ( & t, eCANCELED, "HTTPS download: CANCELED" );
                else
                    STestEnd ( & t, eFAIL,     "HTTPS download" );
            }
            if ( r1 == 0 && r2 != 0 )
                r1 = r2;
        }
        if ( tests & DIAGNOSE_NETWORK_ASPERA && ! _RcCanceled ( r1 ) ) {
            rc_t r2 = 0;
            char fasp [ PATH_MAX ] = "";
            uint64_t faspSize = 0;
            STestStart ( & t, true,        "Aspera download" );
            r2 = STestCheckFasp ( & t, & run, false, fasp, sizeof fasp,
                                  & faspSize, exp, sizeof exp - 1 );
            if ( r2 == 0 )
                r2 = STestEnd ( & t, eOK,      "Aspera download" );
            else {
                if ( _RcCanceled ( r2 ) )
                    STestEnd ( & t, eCANCELED, "Aspera download: CANCELED" );
                else
                    STestEnd ( & t, eFAIL,     "Aspera download" );
            }
            if ( r1 == 0 && r2 != 0 )
                r1 = r2;
            if ( r2 == 0 && httpSize != 0 && faspSize != 0 ) {
                r2 = STestHttpVsFasp ( & t, http, httpSize, fasp, faspSize );
                if ( r1 == 0 && r2 != 0 )
                    r1 = r2;
            }
            if ( * fasp != '\0' ) {
                rc_t r2 = KDirectoryRemove ( t . dir, false, fasp );
                if ( r2 != 0 ) {
                    STestFail ( & t, r2, "FAILURE: cannot remove '%s': %R",
                                          fasp, r2 );
                    if ( r1 == 0 && r2 != 0 )
                        r1 = r2;
                }
                else
                    * fasp = '\0';
            }
        }
        if ( * http != '\0' ) {
            rc_t r2 = KDirectoryRemove ( t . dir, false, http );
            if ( r2 != 0 ) {
                STestFail ( & t, r2, "FAILURE: cannot remove '%s': %R",
                                      http, r2 );
                if ( r1 == 0 && r2 != 0 )
                    r1 = r2;
            }
            else
                * http = '\0';
        }
        if ( r1 == 0)
            r1 = STestEnd ( & t, eOK,  "Network" );
        else  if ( _RcCanceled ( r1 ) )
            STestEnd ( & t, eCANCELED, "Network: CANCELED" );
        else
            STestEnd ( & t, eFAIL,     "Network" );
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }

#if 0
    if ( tests & DIAGNOSE_CONFIG ) {
        rc_t r1 = 0;
        STestStart ( & t, true, "Configuration" );
        STestStart ( & t, false, "node" );
        STestEndOr ( & t, & r1, eEndOK, "OK" );
        if ( r1 == 0 )
            r1 = STestEnd ( & t, eOK, "Configuration" );
        else {
            if ( _RcCanceled ( r1 ) )
                STestEnd ( & t, eCANCELED, "Configuration: CANCELED" );
            else
                STestEnd ( & t, eFAIL, "Configuration" );
        }
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }
#endif
#if 0
    if ( tests & DIAGNOSE_NETWORK && ! _RcCanceled ( rc ) ) {
        rc_t r1 = 0;
        STestStart ( & t, true, "Network" );
#if 0
        {
#undef  HOST
#define HOST "www.ncbi.nlm.nih.gov"
            String h;
            Data d;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr, "https://" HOST );
            if ( r2 == 0 )
                r2 = _STestCheckNetwork ( & t, & d, 0, 0,
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
                            "https://" HOST "/srapub/SRR029074" );
            if ( r2 == 0 )
                r2 = _STestCheckNetwork ( & t, & d, exp, sizeof exp - 1,
                                         NULL, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
#endif
#if 1
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
                r2 = _STestCheckNetwork ( & t, & d, exp, sizeof exp - 1,
                                         & v, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & v );
            DataFini ( & d );
        }
#endif
#if 0
        {
#undef  HOST
#define HOST "gap-download.ncbi.nlm.nih.gov"
            String h;
            Data d;
            CONST_STRING ( & h, HOST );
            r2 = DataInit ( & d, self -> vmgr, "https://" HOST );
            if ( r2 == 0 )
                r2 = _STestCheckNetwork ( & t, & d, NULL, 0, 
                                         NULL, "Access to '%S'", & h );
            if ( r1 == 0 )
                r1 = r2;
            DataFini ( & d );
        }
#endif
        if ( r1 == 0)
            r1 = STestEnd ( & t, eOK, "Network" );
        else  if ( _RcCanceled ( r1 ) )
            STestEnd ( & t, eCANCELED, "Network: CANCELED" );
        else
            STestEnd ( & t, eFAIL, "Network" );
        if ( rc == 0 && r1 != 0 )
            rc = r1;
    }
#endif

    if ( rc == 0 && tests & DIAGNOSE_FAIL )
      rc = 1;

    if ( rc == 0)
        STestEnd ( & t, eOK,       "System" );
    else  if ( _RcCanceled ( rc ) )
        STestEnd ( & t, eCANCELED, "System: CANCELED" );
    else
        STestEnd ( & t, eFAIL,     "System" );

    STestFini ( & t );
    KDiagnoseRelease ( self );
    return rc;
}
