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
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/time.h>
#include <kproc/lock.h>

#include <atomic.h>

#include <kfg/kart.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "proc-on.h"


#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * JIPPOTAM
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
#ifdef GETTID_NID
#include <unistd.h>
#include <sys/syscall.h>
int gettid ()
{
return syscall ( SYS_gettid );
}
#endif /* GETTID_NID */

/*) Prototypes
 (*/
XFS_EXTERN void CC XFSProcOnRunOnStart ();
XFS_EXTERN void CC XFSProcOnRunOnFinish ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * Will use SLList and SLNode to store callbacks
 * Two structs : _ProcOnO ( Queue Object ) and _ProcOnQ ( Queue ).
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * _ProcOnO
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _ProcOnO {
    struct SLNode node;


    const char * name;
    XFSPOProc proc;
    const void * data;
    bool exit_on_error;

    bool on_start;
};

    /*  Note that method will not destroy it's children,
     *  only NULLed them
     */
static
rc_t CC
_ProcOnODispose ( const struct _ProcOnO * self )
{
    struct _ProcOnO * OP = ( struct _ProcOnO * ) self;

    if ( OP != NULL ) {
        if ( OP -> name != NULL ) {
            free ( ( char * ) OP -> name );
            OP -> name = NULL;
        }

        memset ( & ( OP -> node ), 0, sizeof ( struct _ProcOnO ) );

        free ( OP );
    }

    return 0;
}   /* _GRQueODispose () */

static
rc_t CC
_ProcOnOMake (
            const struct _ProcOnO ** OP,
            const char * ProcName,
            XFSPOProc Proc,
            const void * Data,
            bool ExitOnError,
            bool RunOnStart
)
{
    rc_t RCt;
    struct _ProcOnO * Ret;
    const char * Ctx;

    RCt = 0;
    Ret = NULL;
    Ctx = RunOnStart ? "ON START" : "ON FINISH";

    XFS_CSAN ( OP )
    XFS_CAN ( OP )
    XFS_CAN ( ProcName )
    XFS_CAN ( Proc )

    pLogMsg ( klogDebug, "PROC-ON(): ADD $(ctx) [$(name)]\n", "ctx=%s,name=%s", Ctx, ProcName );

    Ret = calloc ( 1, sizeof ( struct _ProcOnO ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( ProcName, & ( Ret -> name ) );
        if ( RCt == 0 ) {
            Ret -> proc = Proc;
            Ret -> data = Data;
            Ret -> exit_on_error = ExitOnError;
            Ret -> on_start = RunOnStart;

            * OP = Ret;
        }
    }

    if ( RCt != 0 ) {
        * OP = NULL;

        _ProcOnODispose ( Ret );
    }

    return RCt;
}   /* _ProcOnOMake () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * ProcOn Que ( _ProcOnQ )
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct _ProcOnQ {
    struct SLList list;
};

static const struct _ProcOnQ * _sProcOnQ = 0;

static
void CC
_ProcOnQWhackCallback ( struct SLNode * Nd, void * Data )
{
    if ( Nd != NULL ) {
        _ProcOnODispose ( ( const struct _ProcOnO * ) Nd );
    }
}   /* _ProcOnQWhackCallback () */

LIB_EXPORT
rc_t CC
XFSProcOnDispose ()
{
    struct _ProcOnQ * Q = ( struct _ProcOnQ * ) _sProcOnQ;

    if ( Q != NULL ) {
        _sProcOnQ = NULL;

        SLListWhack ( & ( Q -> list ), _ProcOnQWhackCallback, NULL );

        free ( Q );
    }

    return 0;
}   /* XFSProcOnDispose () */

LIB_EXPORT
rc_t CC
XFSProcOnInit ()
{
    rc_t RCt;
    struct _ProcOnQ * Q;

    RCt = 0;
    Q = ( struct _ProcOnQ * ) _sProcOnQ;

    if ( Q == NULL ) {
        Q = calloc ( 1, sizeof ( struct _ProcOnQ ) );
        if ( Q == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            SLListInit ( & ( Q -> list ) );

            _sProcOnQ = Q;
        }
    }

    return RCt;
}   /* XFSProcOnInit () */

static
rc_t CC
_ProcOnAddEvent (
                const char * ProcName,
                XFSPOProc Proc,
                const void * ProcData,
                bool ExitOnError,
                bool RunAtStart
)
{
    rc_t RCt;
    struct _ProcOnO * Object;
    struct _ProcOnQ * Que;

    RCt = 0;
    Object = NULL;
    Que = ( struct _ProcOnQ * ) _sProcOnQ;

    XFS_CAN ( Que )
    XFS_CAN ( ProcName )
    XFS_CAN ( Proc )

    RCt = _ProcOnOMake ( 
                        ( const struct _ProcOnO ** ) & Object,
                        ProcName,
                        Proc,
                        ProcData,
                        ExitOnError,
                        RunAtStart
                        );
    if ( RCt == 0 ) {
        SLListPushTail ( & ( Que -> list ), & ( Object -> node ) );
    }

    return RCt;
}   /* _ProcOnAddEvent () */

LIB_EXPORT
rc_t CC
XFSProcOnAddStart (
                    const char * Name,
                    XFSPOProc Proc,
                    const void * Data,
                    bool ExitOnError
)
{
    return _ProcOnAddEvent ( Name, Proc, Data, ExitOnError, true );
}   /* XFSProcOnAddStart () */

LIB_EXPORT
rc_t CC
XFSProcOnAddFinish (
                    const char * Name,
                    XFSPOProc Proc,
                    const void * Data,
                    bool ExitOnError
)
{
    return _ProcOnAddEvent ( Name, Proc, Data, ExitOnError, false );
}   /* XFSProcOnAddFinish () */

static
void CC
_ProcOnRunCallback ( SLNode * Node, void * Data )
{
    rc_t RCt;
    struct _ProcOnO * Object;
    bool RunOnStart;
    const char * Ctx;

    RCt = 0;
    Object = NULL;
    RunOnStart = false;
    Ctx = NULL;

    if ( Node != NULL && Data != NULL ) {
        Object = ( struct _ProcOnO * ) Node;
        RunOnStart = * ( ( bool * ) Data );

        Ctx = RunOnStart ? "STARTING" : "FINISHING";

        if ( RunOnStart == Object -> on_start ) {
            pLogMsg ( klogDebug, "PROC-ON(): $(ctx) [$(name)]\n", "ctx=%s,name=%s", Ctx, Object -> name  );
            RCt = ( Object -> proc ) ( Object -> data );
            if ( RCt != 0 ) {
                if ( Object -> exit_on_error ) {
                    pLogMsg ( klogDebug, "PROC-ON(): $(ctx) [$(name)] failed with code [$(rc)]. EXITING by request\n", "ctx=%s,name=%s,rc=%d", Ctx, Object -> name, RCt  );
                    exit ( 3 );
                }
                else {
                    pLogMsg ( klogDebug, "PROC-ON(): $(ctx) [$(name)] failed with code [$(rc)]\n", "ctx=%s,name=%s,rc=%d", Ctx, Object -> name, RCt  );
                }
            }
        }
    }
}   /* _ProcOnRunCallback () */

/*) That method will return nothing, cuz there is nothing to do on fail
 (*/
static
void CC
_ProcOnRun ( bool RunOnStart )
{
    struct _ProcOnQ * Que;
    bool ROS;
    const char * Msg;

    Que = ( struct _ProcOnQ * ) _sProcOnQ;
    ROS = RunOnStart;
    Msg = RunOnStart ? "STARTING" : "FINISHING";

    if ( Que == NULL ) {
        pLogMsg ( klogDebug, "PROC-ON(): $(ctx): [NULL] que, exiting ...\n", "ctx=%s", Msg );
        return;
    }

    pLogMsg ( klogDebug, "PROC-ON(): $(ctx)\n", "ctx=%s", Msg );

    SLListForEach ( & ( Que -> list ), _ProcOnRunCallback, & ROS );
}   /* _ProcOnRun () */

LIB_EXPORT
void CC
XFSProcOnRunOnStart ()
{
    _ProcOnRun ( true );
}   /* XFSProcOnRunOnStart () */

LIB_EXPORT
void CC
XFSProcOnRunOnFinish ()
{
    _ProcOnRun ( false );
}   /* XFSProcOnRunOnFinish () */
