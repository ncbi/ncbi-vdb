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

#include <klib/rc.h>
#include <klib/out.h>
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/time.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <vfs/path.h>
#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <vfs/services.h>

#include <atomic.h>

#include <kfg/kart.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"
#include "orz.h"
#include "proc-on.h"
#include "xgap.h" /* XFSGapRefreshKarts() */


#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) TBH - defines
 ((*/
#define     _MAX_ATTEMPTS_ALLOWED   3

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * This is asynchronous resolver implementation ... another approach
 *
 * Core of resolver is XFSGapObject, which could be edited when 
 * information for that object will became available.
 *
 * Objects kept in a cache, and indexed by accession_or_id.
 * When object is added to cache, if it is not completed, it's name
 * automatically added to queue to resolve. 
 *
 * WARNING: To avoid creation of many mutexes, I am using
 *          'pseudo_atomic' pointer swap. So, there is only one point
 *          to access object in cache for editing or reading :
 *          (( JOJOBA : add method name ))
 *
 * Resolver itself has three parts: 
 *      Queue - place where ID's to resovle are stored
 *      Cache - place where stored ID's with resolved info as
 *              XFSGapObject
 *      Processor - somethigh which get IDa from Queue, resolvers them
 *                  and put data to Cache
 *
 * There is also part which called Resolver, which start separate thread
 * which makes run all these parts run around.
 *
 * Structure naming is very simple :
 *      _GResolver - resolver
 *      _GRQue - resolver queue
 *      _GRQueO - resolver queue object
 *      _GRCache - resolver cache
 *      _GRCacheO - resolver cache object, container for XFSGapObject
 *      _GRProc - resolver processor
 *      XFSGapObject - resolved data 
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * JIPPOTAM
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
#ifdef GETTID_NID
#include <sys/syscall.h>
int gettid ()
{
/*
return syscall ( 224 );
return syscall ( 186 );
*/
return syscall ( SYS_gettid );
}
#endif /* GETTID_NID */

// #define SEC_MILLISEC
#ifdef SEC_MILLISEC
uint32_t
_sec_millisec ()
{
    struct timeval Tv;
    uint32_t BUBU = 1000;

    if ( gettimeofday ( & Tv, NULL ) == 0 ) {
        return ( ( Tv . tv_sec % BUBU ) * BUBU )
                                        + ( Tv . tv_usec / BUBU );
    }

    return 0;
}   /* _sec_millisec () */


#endif /* SEC_MILLISEC */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSGapObject
 * Tapioca ... that structure contains data for resolved item
 * This is object with state :
 *    When it is created, or after destruction it has state Invalid
 *    When it is initialized arter creation, it has state Ready,
 *         and 0 attempt to resolve
 *    When resolver resolves data for object, it get state Resolved or
 *         good
 *    When resolver fails to resolve data amd amount of attempt to 
 *         resolve less than 3, it set state Ready and ## attempts
 *         I some cases, RC and ErrMsg from last attempt would be
 *         preserved
 *    When resolver fails to resolve data and amount of attempt is 
 *         greater or equal 3, it set state Broken
 * 
 * So, there is methotd which constructs instance of Object
 *      _GapObjectMake()
 * as a method which allow to make copy of Object
 *      _GapObjectDup()
 * and methods, which allow to edit Object content and state
 *      _GapObjectSetPaths()
 *      _GapObjectAddAttempt()
 *      _GapObjectInvalidate()
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSGapObject {
    KRefcount refcount;

    const char * accession_or_id;

    const char * remote_url;
    const char * cache_path;
    KTime_t mod_time;
    uint64_t size;

    XFSGOSte state;
        /*  We store here attempt number ... will stop attempte after
         *  third one
         */
    size_t attempt_num;

        /*  We are storing here last attemp failure return code and
         *  message
         */
    rc_t rc_code;
    const char * err_msg;
};

static const char * _sGapObject_classname = "GapObject";

static
rc_t CC
_GapObjectDispose ( const struct XFSGapObject * self )
{
    struct XFSGapObject * Object = ( struct XFSGapObject * ) self;

    if ( Object != NULL ) {
        Object -> state = kgosInvalid;

        KRefcountWhack (
                        & ( Object -> refcount ),
                        _sGapObject_classname
                        );

        if ( Object -> accession_or_id != NULL ) {
            free ( ( char * ) Object -> accession_or_id );
            Object -> accession_or_id = NULL;
        }

        if ( Object -> remote_url != NULL ) {
            free ( ( char * ) Object -> remote_url );
            Object -> remote_url = NULL;
        }

        if ( Object -> cache_path != NULL ) {
            free ( ( char * ) Object -> cache_path );
            Object -> cache_path = NULL;
        }

        if ( Object -> err_msg != NULL ) {
            free ( ( char * ) Object -> err_msg );
            Object -> err_msg = NULL;
        }
        Object -> rc_code = 0;

        Object -> mod_time = 0;
        Object -> size = 0;

        free ( Object );
    }

    return 0;
}   /* _GapObjectDispose () */

static
rc_t CC
_GetValueFromPath (
                    const char ** Value,
                    const struct VPath * Path,
                    rc_t ( CC * GetterFunk ) (
                                            const struct VPath * self,
                                            struct String * str
                                            )
)
{
    rc_t RCt;
    struct String TheString;
    const char * RetVal;

    RCt = 0;
    memset ( & TheString, 0, sizeof ( TheString ) );
    RetVal = NULL;

    XFS_CSAN ( Value )
    XFS_CAN ( Path )
    XFS_CAN ( Value )
    XFS_CAN ( GetterFunk )

    RCt = GetterFunk ( Path, & TheString );
    if ( RCt == 0 ) {
        if ( TheString . len != 0 ) {
            RCt = XFS_SStrDup (
                            ( const struct String * ) & TheString,
                            & RetVal
                            );
            if ( RCt == 0 ) {
                * Value = RetVal;
            }
        }
        else {
            RCt = XFS_RC ( rcEmpty );
        }
    }

    if ( RCt != 0 ) {
        * Value = NULL;
        if ( RetVal != NULL ) {
            free ( ( char * ) RetVal );
            RetVal = NULL;
        }
    }

    return RCt;
}   /* _GetValueFromPath () */

static
rc_t CC
_ReadValueFromPath (
                    const char ** Value,
                    const struct VPath * Path,
                    rc_t ( CC * ReaderFunk ) (
                                            const struct VPath * self,
                                            char * Buffer,
                                            size_t BufferSize,
                                            const char * Filler
                                            )
)
{
    rc_t RCt;
    char Buf [ XFS_SIZE_4096 ];
    const char * RetVal;

    RCt = 0;
    * Buf = 0;
    RetVal = NULL;

    XFS_CSAN ( Value )
    XFS_CAN ( Path )
    XFS_CAN ( Value )
    XFS_CAN ( ReaderFunk )

    RCt = ReaderFunk ( Path, Buf, sizeof ( Buf ), "" );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( Buf, & RetVal );
        if ( RCt == 0 ) {
            * Value = RetVal;
        }
    }

    if ( RCt != 0 ) {
        * Value = NULL;
        if ( RetVal != NULL ) {
            free ( ( char * ) RetVal );
            RetVal = NULL;
        }
    }

    return RCt;
}   /* _ReadValueFromPath () */

/*))    Creates empty Object with state Ready and 0 attempts
 ((*/
static
rc_t CC
_GapObjectMake (
                const struct XFSGapObject ** Object,
                const char * AccessionOrId
)
{
    rc_t RCt;
    struct XFSGapObject * TheObject;

    RCt = 0;
    TheObject = NULL;

    XFS_CSAN ( Object )
    XFS_CAN ( Object )
    XFS_CAN ( AccessionOrId )

    TheObject = calloc ( 1, sizeof ( struct XFSGapObject ) );
    if ( TheObject == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        TheObject -> state = kgosInvalid;

        KRefcountInit (
                        & ( TheObject -> refcount ),
                        1,
                        _sGapObject_classname,
                        "GapObjectMake",
                        "GapObject"
                        );

        RCt = XFS_StrDup (
                        AccessionOrId,
                        & TheObject -> accession_or_id
                        );
        if ( RCt == 0 ) {
            TheObject -> state = kgosReady;

            * Object = TheObject;
        }
    }

    if ( RCt != 0 ) {
        * Object = NULL;

        if ( TheObject != NULL ) {
            _GapObjectDispose ( TheObject );
        }
    }

    return RCt;
}   /* _GapObjectMake () */

static
rc_t CC
_GapObjectDup (
                const struct XFSGapObject * self,
                const struct XFSGapObject ** Out
)
{
    rc_t RCt;
    struct XFSGapObject * Object;

    RCt = 0;
    Object = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( self )
    XFS_CAN ( Out )

    RCt = _GapObjectMake (
                        ( const struct XFSGapObject ** ) & Object,
                        self -> accession_or_id
                        );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( 
                        self -> remote_url,
                        & ( Object -> remote_url )
                        );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( 
                            self -> cache_path,
                            & ( Object -> cache_path )
                            );
            if ( RCt == 0 ) {
                if ( self -> err_msg != NULL ) {
                    RCt = XFS_StrDup ( 
                                self -> err_msg,
                                & ( Object -> err_msg )
                                );
                }
                if ( RCt == 0 ) {
                    Object -> mod_time = self -> mod_time;
                    Object -> size = self -> size;
                    Object -> state = self -> state;
                    Object -> attempt_num = self -> attempt_num;
                    Object -> rc_code = self -> rc_code;

                    * Out = Object;
                } 
            }
        }
    }

    if ( RCt != 0 ) {
        * Out = NULL;

        if ( Object != NULL ) {
            _GapObjectDispose ( Object );
        }
    }

    return RCt;
}   /* _GapObjectDup () */

/*))    Adds msg and RC
 ((*/
static
rc_t CC
_GapObjectSetRC ( const struct XFSGapObject * self, rc_t RC )
{
    rc_t RCt;
    char BB [ XFS_SIZE_1024 ];
    size_t NumWr;
    struct XFSGapObject * Object;

    RCt = 0;
    * BB = 0;
    NumWr = 0;
    Object = ( struct XFSGapObject * ) self;

    XFS_CAN ( Object )

    if ( Object -> err_msg != NULL ) {
        free ( ( char * ) Object -> err_msg );
        Object -> err_msg = NULL;
    }
    Object -> rc_code = RC;

    if ( RCt != 0 ) {
        RCt = string_printf ( BB, sizeof ( BB ), & NumWr, "%R", RC );
        if ( RCt == 0 ) {
            XFS_StrDup ( BB, & ( Object -> err_msg ) );
        }
        Object -> rc_code = RCt;
    }

    return RCt;
}   /* _GapObjectSetRC () */

/*))    Adds attempt, and if RC != 0, adds msg and RC
 ((*/
static
rc_t CC
_GapObjectAddAttempt ( const struct XFSGapObject * self, rc_t RC )
{
    rc_t RCt;
    struct XFSGapObject * Object;

    RCt = 0;
    Object = ( struct XFSGapObject * ) self;

    XFS_CAN ( Object )

    Object -> attempt_num ++;

    _GapObjectSetRC ( self, RC );

    if ( _MAX_ATTEMPTS_ALLOWED <= Object -> attempt_num ) {
        Object -> state = kgosBroken;

        RCt = XFS_RC ( rcInvalid );
    }

    return RCt;
}   /* _GapObjectAddAttempt () */

#ifdef WE_DONT_NEED_IT
static
rc_t CC
_GapObjectInvalidate ( const struct XFSGapObject * self, rc_t RC )
{
    struct XFSGapObject * Object = ( struct XFSGapObject * ) self;

    XFS_CAN ( Object )

    Object -> state = kgosBroken;

    return _GapObjectSetRC ( self, RC );
}   /* _GapObjectInvalidate () */
#endif /* WE_DONT_NEED_IT */

static
rc_t CC
_GapObjectSetPaths (
                const struct XFSGapObject * self,
                const struct VPath * Path,
                const struct VPath * Cache
)
{
    rc_t RCt;
    struct XFSGapObject * TheObject;
    const char * Aooi;

    RCt = 0;
    Aooi = NULL;

    TheObject = ( struct XFSGapObject * ) self;

    XFS_CAN ( self )
    XFS_CAN ( Path )
    XFS_CAN ( Cache )

    RCt = _GetValueFromPath ( & Aooi, Path, VPathGetId );
    if ( RCt == 0 ) {
        if ( strcmp ( TheObject -> accession_or_id, Aooi ) != 0 ) {
            RCt = XFS_RC ( rcInvalid );
        }

        free ( ( char * ) Aooi );
    }

    if ( RCt == 0 ) {
        RCt = _ReadValueFromPath (
                                & TheObject -> remote_url,
                                Path,
                                XFS_ReadVUri_ZHR
                                );
        if ( RCt == 0 ) {
            RCt = _ReadValueFromPath (
                                    & TheObject -> cache_path,
                                    Cache,
                                    XFS_ReadVPath_ZHR
                                    );
            if ( RCt == 0 ) {
                TheObject -> mod_time = VPathGetModDate ( Path );
                TheObject -> size = VPathGetSize ( Path );
                TheObject -> state = kgosGood;
            }
        }
    }

    _GapObjectSetRC ( self, RCt );

    return RCt;
}   /* _GapObjectSetPaths () */

LIB_EXPORT
rc_t CC
XFSGapObjectAddRef ( const struct XFSGapObject * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountAdd (
                        & ( self -> refcount ),
                        _sGapObject_classname
                        );
    switch ( RefC ) {
        case krefOkay :
            RCt = 0;
            break;
        case krefZero :
        case krefLimit :
        case krefNegative :
            RCt = XFS_RC ( rcInvalid );
            break;
        default :
            RCt = XFS_RC ( rcUnknown );
            break;
    }

    return RCt;
}   /* XFSGapObjectAddRef () */

LIB_EXPORT
rc_t CC
XFSGapObjectRelease ( const struct XFSGapObject * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountDrop (
                        & ( self -> refcount ),
                        _sGapObject_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _GapObjectDispose ( self );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* XFSGapObjectRelease () */

LIB_EXPORT
rc_t CC
XFSGapObjectId (
                const struct XFSGapObject * self,
                const char ** AccessionOrId
)
{
    XFS_CSAN ( AccessionOrId )
    XFS_CAN ( self )
    XFS_CAN ( AccessionOrId )

    * AccessionOrId = self -> accession_or_id;

    return 0;
}   /* XFSGapObjectId () */

LIB_EXPORT
rc_t CC
XFSGapObjectURL (
                const struct XFSGapObject * self,
                const char ** Url
)
{
    XFS_CSAN ( Url )
    XFS_CAN ( self )
    XFS_CAN ( Url )

    * Url = self -> remote_url;

    return 0;
}   /* XFSGapObjectUrl () */

LIB_EXPORT
rc_t CC
XFSGapObjectCachePath (
                const struct XFSGapObject * self,
                const char ** CachePath
)
{
    XFS_CSAN ( CachePath )
    XFS_CAN ( self )
    XFS_CAN ( CachePath )

    * CachePath = self -> cache_path;

    return 0;
}   /* XFSGapObjectCachePath () */

LIB_EXPORT
rc_t CC
XFSGapObjectModTime (
                const struct XFSGapObject * self,
                KTime_t * ModTime
)
{
    XFS_CSA ( ModTime, 0 )
    XFS_CAN ( self )
    XFS_CAN ( ModTime )

    * ModTime = self -> mod_time;

    return 0;
}   /* XFSGapObjectModTime () */

LIB_EXPORT
rc_t CC
XFSGapObjectSize (
                const struct XFSGapObject * self,
                uint64_t * Size
)
{
    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    * Size = self -> size;

    return 0;
}   /* XFSGapObjectSize () */

LIB_EXPORT
bool CC
XFSGapObjectGood ( const struct XFSGapObject * self )
{
    return self == NULL ? false : ( self -> state == kgosGood );
}   /* XFSGapObjectGood () */

LIB_EXPORT
bool CC
XFSGapObjectCompleted ( const struct XFSGapObject * self )
{
    return self == NULL
        ? false
        : ( self -> state == kgosGood || self -> state == kgosBroken )
        ;
}   /* XFSGapObjectCompleted () */

LIB_EXPORT
rc_t CC
XFSGapObjectState (
                    const struct XFSGapObject * self,
                    XFSGOSte * Status
)
{
    XFS_CSA ( Status, kgosInvalid )
    XFS_CAN ( self )
    XFS_CAN ( Status )

    * Status = self -> state;

    return 0;
}   /* XFSGapObjectState () */

LIB_EXPORT
rc_t CC
XFSGapObjectRcAndMsg (
                    const struct XFSGapObject * self,
                    rc_t * RC,
                    const char ** Msg
)
{
    XFS_CSA ( RC, rcInvalid )
    XFS_CSAN ( Msg )
    XFS_CAN ( self )

    if ( RC != NULL ) {
        * RC = self -> rc_code;
    }

    if ( Msg != NULL ) {
        * Msg = self -> err_msg;
    }

    return 0;
}   /* XFSGapObjectRcAndMsg () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Will use DLList and DLNode to implement FIFO
 * Two structs : _GRQueO ( Queue Object ) and _GRQue ( Queue ).
 * Thread safe ( prolly ), lockable
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * _GRQueO
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GRQueO {
    struct DLNode node;

    uint32_t project_id;
    const char * aooi;
};

    /*  Note that method will not destroy it's children,
     *  only NULLed them
     */
static
rc_t CC
_GRQueODispose ( const struct _GRQueO * self )
{
    struct _GRQueO * QO = ( struct _GRQueO * ) self;

    if ( QO != NULL ) {
        memset ( & ( QO -> node ), 0, sizeof ( QO -> node ) );

        if ( QO -> aooi != NULL ) {
            free ( ( char * ) QO -> aooi );
            QO -> aooi = NULL;
        }

        QO -> project_id = 0;

        free ( QO );
    }

    return 0;
}   /* _GRQueODispose () */

static
rc_t CC
_GRQueOMake (
            const struct _GRQueO ** QO,
            uint32_t ProjectId,
            const char * AccessionOrId
)
{
    rc_t RCt;
    struct _GRQueO * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( QO )
    XFS_CAN ( QO )
    XFS_CAN ( AccessionOrId )

    Ret = calloc ( 1, sizeof ( struct _GRQueO ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( AccessionOrId, & ( Ret -> aooi ) );
        if ( RCt == 0 ) {
            Ret -> project_id = ProjectId;

            * QO = Ret;
        }
    }

    if ( RCt != 0 ) {
        * QO = NULL;

        _GRQueODispose ( Ret );
    }

    return RCt;
}   /* _GRQueOMake () */

static
const struct _GRQueO * CC
_GRQueONext ( const struct _GRQueO * self )
{
    return self == NULL
        ? NULL
        : ( const struct _GRQueO * ) DLNodeNext ( & ( self -> node ) )
        ;
}   /* _GRQueONext () */

#ifdef WE_DONT_NEED_IT
static
const struct _GRQueO * CC
_GRQueOPrev ( const struct _GRQueO * self )
{
    return self == NULL
        ? NULL
        : ( const struct _GRQueO * ) DLNodePrev ( & ( self -> node ) )
        ;
}   /* _GRQueOPrev () */
#endif /* WE_DONT_NEED_IT */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Resolver Que ( _GRQue )
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GRQue {
    struct DLList list;

    struct KLock * mutabor;

    size_t size;
};

static
void CC
_GRQueWhackCallback ( struct DLNode * Nd, void * Data )
{
    if ( Nd != NULL ) {
        _GRQueODispose ( ( const struct _GRQueO * ) Nd );
    }
}   /* _GRQueWhackCallback () */

static
rc_t CC
_GRQueDispose ( const struct _GRQue * self )
{
    struct _GRQue * Que = ( struct _GRQue * ) self;

    if ( Que != NULL ) {
        DLListWhack ( & ( Que -> list ), _GRQueWhackCallback, NULL );

        if ( Que -> mutabor != NULL ) {
            KLockRelease ( Que -> mutabor );
            Que -> mutabor = NULL;
        }

        Que -> size = 0;

        free ( Que );
    }

    return 0;
}   /* _GRQueDispose () */

static
rc_t CC
_GRQueMake ( const struct _GRQue ** Que )
{
    rc_t RCt;
    struct _GRQue * Q;

    RCt = 0;
    Q = NULL;

    XFS_CSAN ( Que )
    XFS_CAN ( Que )

    Q = calloc ( 1, sizeof ( struct _GRQue ) );
    if ( Q == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( Q -> mutabor ) );
        if ( RCt == 0 ) {
            DLListInit ( & ( Q -> list ) );

            Q -> size = 0;

            * Que = Q;
        }
    }

    if ( RCt != 0 ) {
        * Que = NULL;

        if ( Q != NULL ) {
            _GRQueDispose ( Q );
        }
    }

    return RCt;
}   /* _GRQueMake () */

static
rc_t CC
_GRQueLock ( const struct _GRQue * self )
{
    return KLockAcquire ( self -> mutabor );
}   /* _GRQueLock () */

static
rc_t CC
_GRQueUnlock ( const struct _GRQue * self )
{
    return KLockUnlock ( self -> mutabor );
}   /* _GRQueUnlock () */

static
const struct _GRQueO * CC
_GRQueHead ( const struct _GRQue * self )
{
    return ( const struct _GRQueO * ) DLListHead ( & ( self -> list ) );
}   /* _GRQueHead () */

#ifdef WE_DONT_NEED_IT
static
const struct _GRQueO * CC
_GRQueTail ( const struct _GRQue * self )
{
    return ( const struct _GRQueO * ) DLListTail ( & ( self -> list ) );
}   /* _GRQueTail () */
#endif /* WE_DONT_NEED_IT */

/*  That will get object from que, and return it. If there is no
 *  objects - it will return NULL. User should decide what to do
 *  with object: delete or put it back :LOL:
 *  Also, it does return NULL in the case of error ... ha-ha-ha lol
 */
static
const struct _GRQueO * CC
_GRQuePull ( const struct _GRQue * self )
{
    struct DLNode * Node = NULL;

    if ( _GRQueLock ( self ) == 0 ) {
        Node = DLListPopTail ( ( struct DLList * ) & self -> list );

        if ( self -> size != 0 ) {
            ( ( struct _GRQue * ) self ) -> size --;
        }

        _GRQueUnlock ( self );
    }

    return ( const struct _GRQueO * ) Node;
}   /* _GRQuePull () */

static
rc_t CC
_GRQuePush ( const struct _GRQue * self, const struct _GRQueO * Obj )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Obj )

    RCt = _GRQueLock ( self );
    if ( RCt == 0 ) {
        DLListPushHead (
                        ( struct DLList * ) & ( self -> list ),
                        ( struct DLNode * ) & ( Obj -> node )
                        );

        ( ( struct _GRQue * ) self ) -> size ++;

        _GRQueUnlock ( self );
    }

    return RCt;
}   /* _GRQuePush () */

static
rc_t CC
_GRQueAdd (
            const struct _GRQue * self,
            uint32_t ProjectId,
            const char * AccessionOrId
)
{
    rc_t RCt;
    const struct _GRQueO * QO;

    RCt = 0;
    QO = NULL;

    XFS_CAN ( self )
    XFS_CAN ( AccessionOrId )

    RCt = _GRQueOMake ( & QO, ProjectId, AccessionOrId );
    if ( RCt == 0 ) {
        RCt = _GRQuePush ( self, QO );
    }

    return RCt;
}   /* _GRQueAdd () */

/*
 *  For Successiful Clipping
 */
struct _GRCache;
static bool CC _GRCacheHasToBeResolved (
                        const struct _GRCache * self,
                        const char * AccessionOrId
                        );

/*  That method will create new GRQue object and attach to it 
 *  ClipSize or less GRQueO objects from original GRQue object
 *  If ClipSize == 0, all object will be moved to new GRQueO object
 *  Container - is needed to check if object already resolved.
 *  Container could be NULL
 */
static
rc_t CC
_GRQueClip (
            const struct _GRQue * self,
            const struct _GRQue ** Clip,
            const struct _GRCache * Container,
            uint32_t ClipSize
)
{
    rc_t RCt;
    const struct _GRQue * Ret;
    const struct _GRQueO * QO;

    RCt = 0;
    Ret = NULL;
    QO = NULL;

    XFS_CSAN ( Clip )
    XFS_CAN ( self )
    XFS_CAN ( Clip )

    RCt = _GRQueMake ( & Ret );
    if ( RCt == 0 ) {
        if ( ClipSize == 0 ) {
            ClipSize = Ret -> size;
        }

        while ( Ret -> size < ClipSize ) {
            QO = _GRQuePull ( self );
            if ( QO == NULL ) {
                break;
            }

            if ( Container != NULL ) {
                if ( ! _GRCacheHasToBeResolved ( Container, QO -> aooi ) ) {
                    _GRQueODispose ( QO );
                    continue;
                }
            }

            RCt = _GRQuePush ( Ret, QO );
            if ( RCt != 0 ) {
                break;
            }
        }

        if ( RCt == 0 ) {
            * Clip = Ret;
        }
    }

    if ( RCt != 0 ) {
        * Clip = NULL;

        if ( Ret != NULL ) {
            _GRQueDispose ( Ret );
        }
    }

    return RCt;
}   /* _GRQueClip () */

/*) That method will dispose added clip
 (*/
static
rc_t CC
_GRQueRecycleClip (
                const struct _GRQue * self,
                const struct _GRQue * Clip,
                const struct _GRCache * Cache
)
{
    rc_t RCt;
    const struct _GRQueO * ClipObj;

    RCt = 0;
    ClipObj = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Clip )

    while ( ( ClipObj = _GRQuePull ( Clip ) ) != NULL ) {
        if ( _GRCacheHasToBeResolved ( Cache, ClipObj -> aooi ) ) {
            RCt = _GRQuePush ( self, ClipObj );
            if ( RCt != 0 ) {
                pLogMsg ( klogDebug, "RESOLVER: really something bad happens while recycling que [$(rc)]", "rc=%d", RCt );
                break;
            }
        }
        else {
            _GRQueODispose ( ClipObj );
        }
    }

        /*  Yes, we are doing that here
         */
    if ( RCt == 0 ) {
        _GRQueDispose ( Clip );
    }

    return RCt;
}   /* _GRQueRecycleClip () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Placeholder for XFSGapObject 
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GRCacheO {
    struct BSTNode node;

    atomic_ptr_t holder;
};

static const struct XFSGapObject * CC _GRCacheOGet (
                            const struct _GRCacheO * self
                            );
static rc_t CC _GRCacheOSet (
                            const struct _GRCacheO * self,
                            const struct XFSGapObject * Object
                            );

static 
rc_t CC
_GRCacheODispose ( const struct _GRCacheO * self )
{
    if ( self != NULL ) {
        _GRCacheOSet ( self, NULL );

        free ( ( struct _GRCacheO * ) self );
    }

    return 0;
}   /* _GRCacheODispose () */

static
rc_t CC
_GRCacheOMake (
            const struct _GRCacheO ** CO,
            const struct XFSGapObject * Object
)
{
    rc_t RCt;
    struct _GRCacheO * Obj;

    RCt = 0;
    Obj = NULL;

    XFS_CSAN ( CO )
    XFS_CAN ( Object )
    XFS_CAN ( CO )

    Obj = calloc ( 1, sizeof ( struct _GRCacheO ) );
    if ( Obj == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        _GRCacheOSet ( Obj, Object );

        * CO = Obj;
    }

    if ( RCt != 0 ) {
        * CO = NULL;

        if ( Obj != NULL ) {
            _GRCacheODispose ( Obj );
        }
    }

    return RCt;
}   /* _GRCacheOMake () */

/*))    That always add reffcount, You should release
 ((*/
const struct XFSGapObject * CC
_GRCacheOGet ( const struct _GRCacheO * self )
{
    const struct XFSGapObject * Ret = NULL;

    if ( self != NULL ) {
        Ret = self -> holder . ptr;

        XFSGapObjectAddRef ( Ret );
    }

    return Ret;
}   /* _GRCacheOGet () */

/*))    That will Release previous object if it is not null, and add
 //     reference to new one if it is not a null
((*/
rc_t CC
_GRCacheOSet (
            const struct _GRCacheO * self,
            const struct XFSGapObject * Object
)
{
    void * Read, * Test;

    XFS_CAN ( self )

    Test = Read = self -> holder . ptr;

    if ( Object != NULL ) {
        XFSGapObjectAddRef ( Object );
    }

    do {
        Read = atomic_test_and_set_ptr (
                        ( struct atomic_ptr_t * ) & ( self -> holder ),
                        ( void * ) Object,
                        Test
                        );
    } while ( Read != Test );

    if ( Read != NULL ) {
        XFSGapObjectRelease ( Read );
    }

    return 0;
}   /* _GRGacheOSet () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * There where resolved data stored
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GRCache {
    struct BSTree tree;

    struct KLock * mutabor;
};

static
void CC
_GRCacheWhackCallback ( struct BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        _GRCacheODispose ( ( struct _GRCacheO * ) Node );
    }
}   /* _GRCacheWhackCallback () */

static
rc_t CC
_GRCacheDispose ( const struct _GRCache * self )
{
    struct _GRCache * C = ( struct _GRCache * ) self;

    if ( C != NULL ) {
        BSTreeWhack ( & ( C -> tree ), _GRCacheWhackCallback, NULL );

        if ( C -> mutabor != NULL ) {
            KLockRelease ( C -> mutabor );
            C -> mutabor = NULL;
        }

        free ( C );
    }

    return 0;
}   /* _GRCacheDispose () */

static
rc_t CC
_GRCacheMake ( const struct _GRCache ** Cont )
{
    rc_t RCt;
    struct _GRCache * C;

    RCt = 0;
    C = NULL;

    XFS_CSAN ( Cont )
    XFS_CAN ( Cont )

    C = calloc ( 1, sizeof ( struct _GRCache ) );
    if ( C == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( C -> mutabor ) );
        if ( RCt == 0 ) {
            BSTreeInit ( & ( C -> tree ) );

            * Cont = C;
        }
    }

    if ( RCt != 0 ) {
        * Cont = NULL;

        if ( C != NULL ) {
            _GRCacheDispose ( C );
        }
    }

    return RCt;
}   /* _GRCacheMake () */

static
rc_t CC
_GRCacheLock ( const struct _GRCache * self )
{
    XFS_CAN ( self )

    return KLockAcquire ( self -> mutabor );
}   /* _GRCacheLock () */

static
rc_t CC
_GRCacheUnlock ( const struct _GRCache * self )
{
    XFS_CAN ( self )

    return KLockUnlock ( self -> mutabor );
}   /* _GRCacheUnLock () */

static
int64_t CC
_GRCacheFindCallback ( const void * Item, const struct BSTNode * Node )
{
    const char * Str1;
    const struct XFSGapObject * Object;
    const char * Str2;



    Str1 = ( const char * ) Item;
    Object = Node == NULL
                    ? NULL
                    :
                    ( ( ( struct _GRCacheO * ) Node ) -> holder ) . ptr
                    ;
    Str2 = Object == NULL ? "" : Object -> accession_or_id;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _GRCacheFindCallback () */

/*))    NOTE : release Object after use
 ((*/
static
rc_t CC
_GRCacheFind_NoLock (
                    const struct _GRCache * self,
                    const struct _GRCacheO ** Object,
                    const char * AccessionOrId
)
{
    rc_t RCt;
    const struct _GRCacheO * COFound;

    RCt = 0;
    COFound = NULL;

    XFS_CSAN ( Object )
    XFS_CAN ( self )
    XFS_CAN ( Object )
    XFS_CAN ( AccessionOrId )

    COFound = ( const struct _GRCacheO * ) BSTreeFind (
                                                    & ( self -> tree ),
                                                    AccessionOrId,
                                                    _GRCacheFindCallback
                                                    );

    if ( COFound == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        * Object = COFound;
    }

    return RCt;
}   /* _GRCacheFind_NoLock () */

static
rc_t CC
_GRCacheFind (
            const struct _GRCache * self,
            const struct XFSGapObject ** Object,
            const char * AccessionOrId
)
{
    rc_t RCt;
    const struct _GRCacheO * Holder;

    RCt = 0;
    Holder = NULL;

    RCt = _GRCacheLock ( self );
    if ( RCt == 0 ) {
        RCt = _GRCacheFind_NoLock ( self, & Holder, AccessionOrId );
        if ( RCt == 0 ) {
            * Object = _GRCacheOGet ( Holder );
        }

        _GRCacheUnlock ( self );
    }

    return RCt;
}   /* _GRCacheFind () */

static
bool CC
_GRCacheHas ( const struct _GRCache * self, const char * AccessionOrId )
{
    const struct _GRCacheO * Holder;

    return _GRCacheFind_NoLock ( self, & Holder, AccessionOrId );
}   /* _GRCacheHas () */

static
int64_t CC
_GRCacheAddObjectCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    const struct XFSGapObject * O1, * O2;

    O1 = N1 == NULL
                ? NULL : ( ( struct _GRCacheO * ) N1 ) -> holder . ptr;
    O2 = N2 == NULL
                ? NULL : ( ( struct _GRCacheO * ) N2 ) -> holder . ptr;
    return XFS_StringCompare4BST_ZHR (
            ( O1 == NULL ? "" : O1 -> accession_or_id ),
            ( O2 == NULL ? "" : O2 -> accession_or_id )
            );
}   /* _GRCacheAddObjectCallback () */

static
rc_t CC
_GRCacheAddObject_NoLock (
                        const struct _GRCache * self,
                        const struct XFSGapObject * Object
)
{
    rc_t RCt;
    const struct _GRCacheO * CO;

    RCt = 0;
    CO = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Object )

    RCt = _GRCacheFind_NoLock (
                            self,
                            & CO,
                            Object -> accession_or_id
                            );
    if ( RCt == 0 ) {
        XFSGapObjectRelease ( Object );
        RCt = XFS_RC ( rcExists );
    }
    else {
        RCt = _GRCacheOMake ( & CO, Object );
        if ( RCt == 0 ) {
            RCt = BSTreeInsert (
                                ( BSTree * ) & ( self -> tree ),
                                ( struct BSTNode * ) CO,
                                _GRCacheAddObjectCallback
                                );
        }
    }

    return RCt;
}   /* _GRCacheAddObject_NoLock () */

#ifdef WE_DONT_NEED_IT
static
rc_t CC
_GRCacheAddObject (
                const struct _GRCache * self,
                const struct XFSGapObject * Obj
)
{
    rc_t RCt = 0;

    RCt = _GRCacheLock ( self );
    if ( RCt == 0 ) {
        RCt = _GRCacheAddObject_NoLock ( self, Obj );

        _GRCacheUnlock ( self );
    }

    return 0;
}   /* _GRCacheAddObject () */
#endif /* WE_DONT_NEED_IT */

static
rc_t CC
_GRCacheFindOrCreate (
            const struct _GRCache * self,
            const struct XFSGapObject ** Object,
            const char * AccessionOrId
)
{
    rc_t RCt;
    const struct _GRCacheO * Holder;
    const struct XFSGapObject * TheObject;

    RCt = 0;
    Holder = NULL;
    TheObject = NULL;

    RCt = _GRCacheLock ( self );
    if ( RCt == 0 ) {
        RCt = _GRCacheFind_NoLock ( self, & Holder, AccessionOrId );
        if ( RCt == 0 ) {
            TheObject = _GRCacheOGet ( Holder );
        }
        else {
            RCt = _GapObjectMake ( & TheObject, AccessionOrId );
            if ( RCt == 0 ) {
                RCt = _GRCacheAddObject_NoLock ( self, TheObject );
            }
        }

        _GRCacheUnlock ( self );
    }

    if ( RCt == 0 ) {
        * Object = TheObject;
    }

    return RCt;
}   /* _GRCacheFindOrCreate () */

static
rc_t CC
_GRCacheFindAndCopy (
            const struct _GRCache * self,
            const struct XFSGapObject ** Object,
            const char * AccessionOrId
)
{
    rc_t RCt;
    const struct XFSGapObject * TheObject;

    RCt = 0;
    TheObject = NULL;

    RCt = _GRCacheFindOrCreate ( self, & TheObject, AccessionOrId );
    if ( RCt == 0 ) {

        RCt = _GapObjectDup ( TheObject, Object );

        XFSGapObjectRelease ( TheObject );
    }

    return RCt;
}   /* _GRCacheFindAndCopy () */

static
rc_t CC
_GRCacheSetObject (
                const struct _GRCache * self,
                const struct XFSGapObject * Object
)
{
    rc_t RCt;
    const struct _GRCacheO * CO;

    RCt = 0;
    CO = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Object )

    RCt = _GRCacheLock ( self );
    if ( RCt == 0 ) {
        RCt = _GRCacheFind_NoLock (
                                self,
                                & CO,
                                Object -> accession_or_id
                                );
        if ( RCt == 0 ) {
            RCt = _GRCacheOSet ( CO, Object );
        }
        else {
            RCt = BSTreeInsert (
                                ( BSTree * ) & ( self -> tree ),
                                ( struct BSTNode * ) CO,
                                _GRCacheAddObjectCallback
                                );
        }

        _GRCacheUnlock ( self );
    }

    return RCt;
}   /* _GRCacheSetObject () */

static
bool CC
_GRCacheObjectHasToBeResolved ( const struct XFSGapObject * Object )
{
    bool Ret = true;

    if ( Object != NULL ) {
        Ret =      ( Object -> state != kgosGood )
                && ( Object -> state != kgosBroken )
                && ( Object -> attempt_num < _MAX_ATTEMPTS_ALLOWED )
                ;
    }

    return Ret;
}   /* _GRCacheObjectHasToBeResolved () */

static
bool CC
_GRCacheHasToBeResolved (
                        const struct _GRCache * self,
                        const char * AccessionOrId
)
{
    bool Ret;
    const struct XFSGapObject * Object;

    Ret = true;
    Object = NULL;

    if ( _GRCacheFind ( self, & Object, AccessionOrId ) == 0 ) {

        Ret = _GRCacheObjectHasToBeResolved ( Object );

        XFSGapObjectRelease ( Object );
    }

    return Ret;
}   /* _GRCacheHasToBeResolved () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * There where tre resolving is taking place
 * How it is Works?
 *   1 ) Call _GRProcMake 
 *   2 ) Call _GRProcProcess
 *   3 ) Call _GRProcDispose
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct _GRProc {
    struct KService * service;
};

static
rc_t CC
_GRProcDispose ( const struct _GRProc * self )
{
    struct _GRProc * P = ( struct _GRProc * ) self;

    if ( P != NULL ) {
        if ( P -> service != NULL ) {
            KServiceRelease ( P -> service );
            P -> service = NULL;
        }

        free ( P );
    }

    return 0;
}   /* _GRProcDispose () */

static
rc_t CC
_GRProcMake ( const struct _GRProc ** Processor )
{
    rc_t RCt;
    struct _GRProc * P;

    RCt = 0;
    P = NULL;

    XFS_CSAN ( Processor )
    XFS_CAN ( Processor )

    P = calloc ( 1, sizeof ( struct _GRProc ) );
    if ( P == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KServiceMake ( & ( P -> service ) );
        if ( RCt == 0 ) {
            * Processor = P;
        }
    }

    if ( RCt != 0 ) {
        * Processor = NULL;

        if ( P != NULL ) {
            _GRProcDispose ( P );
        }
    }

    return RCt;
}   /* _GRProcMake () */

static
rc_t CC
_GRProcPrepareIds (
                    const struct _GRProc * self,
                    const struct _GRQue * Input
)
{
    rc_t RCt;
    const struct _GRQueO * DasOB;

    RCt = 0;
    DasOB = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Input )

    DasOB = _GRQueHead ( Input );

    do {
        RCt = KServiceAddProject (
                                self -> service,
                                DasOB -> project_id
                                );
        if ( RCt == 0 ) {
            RCt = KServiceAddId ( self -> service, DasOB -> aooi );
        }

        if ( RCt != 0 ) {
            break;
        }

        DasOB = _GRQueONext ( DasOB );
    } while ( DasOB != NULL );

    return RCt;
}   /* _GRProcPrepareIds () */

/*
 *  Editing is : check if paths are OK, and set state to Resolved
 *  otherwise add another attempt.
 */
static
rc_t CC
_GRProcMakeEditObject (
                    const struct KSrvResponse * Response,
                    uint32_t ResponseNo,
                    const struct _GRCache * Output
)
{
    rc_t RCt;
    const struct VPath * Path;
    const struct VPath * Cache;
    const struct VPath * VdbCache;
    const struct KSrvError * Error;
    const struct XFSGapObject * Object;
    const char * Aooi;
    rc_t ErrRc;
    EObjectType ObjectType;
    struct String StringId;

    RCt = 0;
    Path = NULL;
    Cache = NULL;
    VdbCache = NULL;
    Error = NULL;
    Object = NULL;
    Aooi = NULL;
    ErrRc = 0;

        /*) First we should find object to edit ... or create it
         (*/

    RCt = KSrvResponseGetPath (
                                Response,
                                ResponseNo,
                                eProtocolHttps,
                                & Path,
                                & VdbCache,
                                & Error
                                );
    if ( RCt == 0 ) {
        if ( Error != NULL ) {
                /*)  Taking Accession or Object Id from Error
                 (*/
            RCt = KSrvErrorObject ( Error, & StringId, & ObjectType );
            if ( RCt == 0 ) {
                RCt = XFS_SStrDup (
                                ( const struct String * ) & StringId,
                                & Aooi
                                );
            }
        }
        else {
                /*) Taking Accession or Object Id from path
                 (*/
            RCt = _GetValueFromPath ( & Aooi, Path, VPathGetId );
        }
        if ( RCt == 0 ) {
                /*) Here we trying to find or create object to edit
                 (*/
            RCt = _GRCacheFindAndCopy ( Output, & Object, Aooi );
        }

            /*) So, if we are gucci, we set paths, or set errors if not 
             (*/
        if ( Error == NULL ) {
            if ( RCt == 0 ) {
                    /*) First, there is no error and path extractable
                     (*/
                RCt = KSrvResponseGetCache (
                                            Response,
                                            ResponseNo,
                                            & Cache
                                            );
                if ( RCt == 0 ) {
                    RCt = _GapObjectSetPaths ( Object, Path, Cache );
// printf ( "[MEO] [%d] [GOOD] [%s]\n", __LINE__, Aooi );
                }
            }
        }
        else {
            KSrvErrorRc ( Error, & ErrRc );
             _GapObjectAddAttempt ( Object, ErrRc );
        }

        if ( Object != NULL ) {
            RCt = _GRCacheSetObject ( Output, Object );
        }

        XFSGapObjectRelease ( Object );

        if ( Error != NULL ) {
            KSrvErrorRelease ( Error );
        }

        if ( Path != NULL ) {
            VPathRelease ( Path );
        }

        if ( Cache != NULL ) {
            VPathRelease ( Cache );
        }

        if ( VdbCache != NULL ) {
            VPathRelease ( VdbCache );
        }

        if ( Aooi != NULL ) {
            free ( ( char * ) Aooi );
        }
    }

    return RCt;
}   /* _GRProcMakeEditObject () */

static
rc_t CC
_GRProcProcess (
                const struct _GRProc * self,
                const struct _GRQue * Input,
                const struct _GRCache * Output
)
{
    rc_t RCt;
    const struct KSrvResponse * Response;
    uint32_t ResponseLen;
    uint32_t llp;

    RCt = 0;
    Response = NULL;
    ResponseLen = 0;
    llp = 0;

    XFS_CAN ( self )
    XFS_CAN ( Input )
    XFS_CAN ( Output )

    RCt = _GRProcPrepareIds ( self, Input );
    if ( RCt == 0 ) {
        RCt = KServiceNamesQuery (
                                    self -> service,
                                    eProtocolHttps,
                                    & Response
                                    );
    }

    if ( RCt == 0 ) {
        ResponseLen = KSrvResponseLength ( Response );
        for ( llp = 0; llp < ResponseLen; llp ++ ) {
            RCt = _GRProcMakeEditObject ( Response, llp, Output );
            if ( RCt != 0 ) {
                pLogMsg ( klogDebug, "RESOLVER: really something bad happens while editing object [$(rc)]", "rc=%d", RCt );
                continue; /* JOJOBA: break; ??? */
            }
        }

        KSrvResponseRelease ( Response );
    }

    return RCt;
}   /* _GRProcProcess () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * There is a place where updating thread is started
 * Lyrics - that is endless loop with 1 second delay ... should work
 * I think.
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _GResolver {
    struct KThread * thread;

    const struct _GRCache * container;
    const struct _GRQue * queue;

    atomic32_t stop_request;

    size_t wait_time_millisec;

        /*)  we are planning to do it adaptive later
         (*/
    size_t qty_to_process;
    size_t desired_time_to_process;
    float average_processing_speed;
};

static const struct _GResolver * _sGResolver = NULL;

static
struct _GResolver *
_Rsl ()
{
    return ( struct _GResolver * ) _sGResolver;
}   /* _Rsl () */

static
rc_t CC
_GResolverDispose ( const struct _GResolver * self )
{
    rc_t RCt;
    struct _GResolver * Resolver;

    RCt = 0;
    Resolver = ( struct _GResolver * ) self;

    if ( Resolver != NULL ) {
        if ( Resolver -> thread != NULL ) {
            KThreadWait ( Resolver -> thread, & RCt );

            pLogMsg ( klogDebug, "RESOLVER: while disposing resolver thread status is [$(rc)]", "rc=%d", RCt );
            KThreadRelease ( Resolver -> thread );
            Resolver -> thread = NULL;
        }

        if ( Resolver -> queue != NULL ) {
            _GRQueDispose ( Resolver -> queue );
            Resolver -> queue = NULL;
        }

        if ( Resolver -> container != NULL ) {
            _GRCacheDispose ( Resolver -> container );
            Resolver -> container = NULL;
        }

        atomic32_set ( & ( Resolver -> stop_request ), 0 );
        Resolver -> qty_to_process = 100;
        Resolver -> desired_time_to_process = 0;
        Resolver -> average_processing_speed = 0;
        Resolver -> wait_time_millisec = 300;

        free ( Resolver );
    }

    return 0;
}   /* _GResolverDispose () */

static
rc_t CC
_GResolverMake ( const struct _GResolver ** Resolver )
{
    rc_t RCt;
    struct _GResolver * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Resolver )
    XFS_CAN ( Resolver )

    Ret = calloc ( 1, sizeof ( struct _GResolver ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = _GRCacheMake ( & ( Ret -> container ) );
        if ( RCt == 0 ) {
            RCt = _GRQueMake ( & ( Ret -> queue ) );
            if ( RCt == 0) {
                atomic32_set ( & ( Ret -> stop_request ), 0 );
                Ret -> qty_to_process = 100;
                Ret -> desired_time_to_process = 0;
                Ret -> average_processing_speed = 0;
                Ret -> wait_time_millisec = 300;

                * Resolver = Ret;
            }
        }
    }

    if ( RCt != 0 ) {
        * Resolver = NULL;

        if ( Ret != NULL ) {
            _GResolverDispose ( Ret );
        }
    }

    return RCt;
}   /* _GResolverMake () */

static
bool CC
_GapResolverStopRequested ( const struct _GResolver * self )
{
    return self == NULL
                ? true
                : atomic32_read ( & ( self -> stop_request ) )
                ;
}   /* _GapResolverStopRequested () */

static
rc_t CC
_GapResolverSetStopRequested (
                            const struct _GResolver * self,
                            bool Flag
)
{
    int32_t Value;
    atomic32_t * Atomic;

    XFS_CAN ( self )

    Value = Flag ? 1 : 0;
    Atomic = & ( ( ( struct _GResolver * ) self ) -> stop_request );

    atomic32_set ( Atomic, Value );

    return 0;
}   /* _GapResolverSetStopRequested () */

static
rc_t CC
_GResolverResolve ( struct _GResolver * Resolver )
{
    rc_t RCt;
    const struct _GRProc * Proc;
    const struct _GRQue * Clip;

    RCt = 0;
    Proc = NULL;
    Clip = NULL;

        /*  Clipping out queue
         */
    RCt = _GRQueClip (
                    Resolver -> queue,
                    & Clip,
                    Resolver -> container,
                    Resolver ->  qty_to_process
                    );
    if ( RCt == 0 ) {
        if ( 0 < Clip -> size ) {
                /*  Makin' processor
                 */
            RCt = _GRProcMake ( & Proc );
            if ( RCt == 0 ) {
                    /*  Runnin' resolver
                     */
                RCt = _GRProcProcess ( Proc, Clip, Resolver -> container );

                _GRProcDispose ( Proc );

            }
        }

            /*))     Put unresolved items back to queue,
             //     Clip will be disposed here
            ((*/
        _GRQueRecycleClip (
                            Resolver -> queue,
                            Clip,
                            Resolver -> container
                            );
    }

    return RCt;
}   /* _GResolverResolve () */

/*  Here we think that Data is a instanse of  _GResolver
 */
static
rc_t CC
_GResolverRun ( const struct KThread * self, void * Data )
{
    rc_t RCt;
    size_t llp;
    struct _GResolver * Res;

    RCt = 0;
    llp = 0;
    Res = ( struct _GResolver * ) Data;
    if ( Res == NULL ) {
        Res = _Rsl ();
    }

    XFS_CAN ( self )
    XFS_CAN ( Res )

        /*)) OK, here we are :^|
         ((*/
    while ( true ) {
            /*  Check if we need to stop
             */
        if ( _GapResolverStopRequested ( Res ) ) {
            break;
        }

            /* JOJOBA: not really elegant solution :LOL: */
        llp ++;
        if ( llp % 4 == 0 ) {
            XFSGapRefreshKarts ();
        }

        RCt = _GResolverResolve ( Res );

            /*  Here we do sleep
             */
        KSleepMs ( Res -> wait_time_millisec );
    }

    return RCt;
}   /* _GResolverRun () */

/*) Thread unsafe ... so think
 (*/
static
rc_t CC
_GapResolverStart ( const void * Data )
{
    rc_t RCt;
    struct KThread * Thread;
    struct _GResolver * Rsl;

    RCt = 0;
    Thread = NULL;
    Rsl = ( struct _GResolver * ) Data;

    XFS_CAN ( Rsl )

    if ( Rsl -> thread == 0 ) {
        RCt = KThreadMake ( & Thread, _GResolverRun, Rsl );
        if ( RCt == 0 ) {
            Rsl -> thread = Thread;
        }
        else {
            pLogMsg ( klogDebug, "RESOLVER: while starting resolver was not able to start thread: RC is [$(rc)]", "rc=%d", RCt );
        }
    }

    return RCt;
}   /* _GapResolverStart () */

LIB_EXPORT
rc_t CC
_GapResolverStop ( const void * Data )
{
    rc_t RCt;
    rc_t RCt2;
    struct _GResolver * Rsl;

    RCt = 0;
    RCt2 = 0;
    Rsl = ( struct _GResolver * ) Data;

    XFS_CAN ( Rsl )

        /*  Simple : set flag and wait
         */
    if ( Rsl -> thread != NULL ) {
        RCt = _GapResolverSetStopRequested ( Rsl, true );
        if ( RCt == 0 ) {

            RCt = KThreadWait ( Rsl -> thread, & RCt2 );
            if ( RCt != 0 ) {
                KThreadCancel ( Rsl -> thread );
            }
        }
        if ( RCt != 0 ) {
            KThreadCancel ( Rsl -> thread );
        }

        KThreadRelease ( Rsl -> thread );

        Rsl -> thread = NULL;
    }

    return RCt;
}   /* _GapResolverStop () */

/*) Thread unsafe ... so think
 (*/
LIB_EXPORT
rc_t CC
XFSGapResolverInit ()
{
    rc_t RCt;
    const struct _GResolver * Rsl;

    RCt = 0;
    Rsl = _Rsl ();

    if ( Rsl == NULL ) {
        RCt = _GResolverMake ( & Rsl );
        _sGResolver = Rsl;

        if ( Rsl != NULL ) {
            XFSProcOnAddStart (
                            "Resolver Thread",
                            _GapResolverStart,
                            Rsl,
                            false
                            );
            XFSProcOnAddFinish (
                            "Resolver Thread",
                            _GapResolverStop,
                            Rsl,
                            false
                            );
        }
    }
    else {
        pLogMsg ( klogDebug, "RESOLVER: can not init resolver, it is already inited", "" );
    }

    return RCt;
}   /* XFSGapResolverInit () */

LIB_EXPORT
rc_t CC
XFSGapResolverDispose ()
{
    rc_t RCt;
    const struct _GResolver * Rsl;

    RCt = 0;
    Rsl = NULL;

    Rsl = _sGResolver;
    _sGResolver = NULL;

    if ( Rsl == NULL ) {
        pLogMsg ( klogDebug, "RESOLVER: can not dispose resolver, it is already disposed", "" );
    }
    else {
            /*)     Nothing wrong to call it twice :LOLL:
             (*/
        _GapResolverStop ( Rsl );
        _GResolverDispose ( Rsl );
    }

    return RCt;
}   /* XFSGapResolverDispose () */

    /*  GapObjects are distincts by AccessionOrId ...
     */
LIB_EXPORT
rc_t CC
XFSGapResolverGetObject (
                        const struct XFSGapObject ** Object,
                        const char * AccessionOrId
)
{
    struct _GResolver * Rsl = _Rsl ();

    XFS_CSAN ( Object )
    XFS_CAN ( Rsl )
    XFS_CAN ( Object )
    XFS_CAN ( AccessionOrId )

    return _GRCacheFind ( Rsl -> container, Object, AccessionOrId );
}   /* XFSGapResolverGetObject () */

LIB_EXPORT
bool CC
XFSGapResolverHasObject ( const char * AccessionOrId )
{
    struct _GResolver * Rsl = _Rsl ();
    if ( Rsl != NULL ) {
        return _GRCacheHas ( Rsl -> container, AccessionOrId );
    }
    return false;
}   /* XFSGapResolverHasObject () */

/*  Putting accesstion to a Id and ProjectId
*/
LIB_EXPORT
rc_t CC
XFSGapResolverAddToResolve (
                            uint32_t ProjectId,
                            const char * AccessionOrId
)
{
    rc_t RCt;
    const struct XFSGapObject * Object;
    struct _GResolver * Rsl;

    RCt = 0;
    Object = NULL;
    Rsl = _Rsl ();

    XFS_CAN ( AccessionOrId )
    XFS_CAN ( Rsl )

    RCt = _GRCacheFindOrCreate (
                                Rsl -> container,
                                & Object,
                                AccessionOrId
                                );
    if ( RCt == 0 ) {
        if ( _GRCacheObjectHasToBeResolved ( Object ) ) {
            RCt = _GRQueAdd ( Rsl -> queue, ProjectId, AccessionOrId );
        }

        XFSGapObjectRelease ( Object );
    }

    return RCt;
}   /* XFSGapResolverAddToResolve () */
