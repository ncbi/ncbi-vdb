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

#define SRC_LOC_DEFINED 1

#include <kfc/extern.h>
#include <kfc/tstate.h>
#include <kfc/rsrc.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xc.h>
#include <kfc/xcdefs.h>

#include <klib/time.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KThreadEvent KThreadEvent;
typedef struct KThreadEvtNode KThreadEvtNode;


/*--------------------------------------------------------------------------
 * ts_alloc
 */
static
void * ts_alloc ( size_t bytes, bool clear )
{
    void * mem = clear ? calloc ( 1, bytes ) : malloc ( bytes );
    if ( mem == NULL )
        exit ( -1 );
    return mem;
}


/*--------------------------------------------------------------------------
 * xc to rc
 */
static
rc_t extract_rc_xobj ( const XCObj * xobj )
{
    while ( xobj -> dad != NULL )
        xobj = xobj -> dad;
    return SILENT_RC ( 0, 0, 0, xobj -> rc_obj, 0 );
}


static
rc_t extract_rc_xstate ( const XCState * xstate )
{
    while ( xstate -> dad != NULL )
        xstate = xstate -> dad;
    return SILENT_RC ( 0, 0, 0, 0, xstate -> rc_state );
}

static
rc_t extract_rc_xerr ( const XCErr * xc )
{
    rc_t rc_obj, rc_state;

    /* get to root xc */
    while ( xc -> dad != NULL )
        xc = xc -> dad;

    /* retrieve components */
    rc_obj = extract_rc_xobj ( xc -> obj );
    rc_state = extract_rc_xstate ( xc -> state );

    return rc_obj | rc_state;
}

static
rc_t extract_rc ( const KFuncLoc * loc, const void * xc )
{
    rc_t rc_err = extract_rc_xerr ( xc );
    return loc -> rc_context | rc_err;
}


/*--------------------------------------------------------------------------
 * xc to string
 */
static
const char * extract_desc_xobj ( const XCErr * xc )
{
    const XCObj * xobj;

    while ( xc -> dad != NULL )
        xc = xc -> dad;

    xobj = xc -> obj;
    while ( xobj -> desc == NULL && xobj -> dad != NULL )
        xobj = xobj -> dad;

    return xobj -> desc;
}


static
const char * extract_desc_xstate ( const XCErr * xc )
{
    const XCState * xstate;

    while ( xc -> dad != NULL )
        xc = xc -> dad;

    xstate = xc -> state;
    while ( xstate -> desc == NULL && xstate -> dad != NULL )
        xstate = xstate -> dad;

    return xstate -> desc;
}


/*--------------------------------------------------------------------------
 * KThreadEvtNode
 *  exection state belonging to the current thread
 */
struct KThreadEvtNode
{
    KThreadEvtNode * parent;
    KThreadEvtNode * next;
    KThreadEvtNode * child;

    const KFuncLoc * loc;
    KThreadEvent * evt;
    uint32_t zdepth;              /* root node has zdepth == 0 */
};


/*--------------------------------------------------------------------------
 * KThreadEvent
 *  describes a particular event
 */
struct KThreadEvent
{
    KThreadEvtNode * node;
    KThreadEvent * next;

    const XCErr * xc;
    KTime_t timestamp;
    String message;
    xc_sev_t severity;
    xc_org_t origin;
    uint32_t lineno;
};

/* Dump
 */
static
void KThreadEventDump ( ctx_t ctx, KTime_t timestamp, const KFuncLoc * loc,
    uint32_t lineno, xc_sev_t severity, xc_org_t origin, const void * xc_param,
    const char * msg, const KThreadEvtNode * node )
{
    const XCErr * xc = xc_param;

    size_t pre_size, sz;
    char pre [ 4096 ];

#if _DEBUGGING
    const char *fname;
#endif

    static const char * err_strings [ 3 ] [ 4 ] =
    {
        { "NOTE", "WARNING", "SYSTEM ERROR", "FATAL SYSTEM ERROR" },
        { "NOTE", "WARNING", "INTERNAL ERROR", "FATAL INTERNAL ERROR" },
        { "NOTE", "WARNING", "USER ERROR", "FATAL USER ERROR" }
    };

    KTime tm;
    KTimeLocal ( & tm, timestamp );
    string_printf ( pre, sizeof pre, & pre_size
                    , "%04u-%02u-%02uT%02u:%02u:%02u "
                    , tm . year
                    , tm . month + 1
                    , tm . day
                    , tm . hour
                    , tm . minute
                    , tm . second
        );

#if _DEBUGGING
    string_printf ( & pre [ pre_size ], sizeof pre - pre_size, & sz
                    , "%.*s/%s/%s.%s:%u:"
                    , sizeof __FILE__ -
                      sizeof __mod__  -
                      sizeof __file__ -
                      sizeof __fext__ -
                      1
                    , __FILE__
                    , loc -> src -> mod
                    , loc -> src -> file
                    , loc -> src -> ext
                    , lineno
                    , loc -> func
        );
    pre_size += sz;

    /* function name */
    fname = loc -> func;

    /* remove leading "Java_" from jni names */
    if ( memcmp ( loc -> func, "Java_", sizeof "Java_" - 1 ) == 0 )
        fname += sizeof "Java_" - 1;

    /* print it into buffer */
    string_printf ( & pre [ pre_size ], sizeof pre - pre_size, & sz
                    , "%s - "
                    , fname
        );

    /* convert jni names into Java fqn */
    if ( fname != loc -> func )
    {
        size_t i;
        for ( i = 0; i < sz; ++ i )
        {
            if ( pre [ pre_size + i ] == '_' )
                pre [ pre_size + i ] = '.';
        }
    }
    pre_size += sz;
#endif

    string_printf ( & pre [ pre_size ], sizeof pre - pre_size, & sz
                    , "%s: "
                    , err_strings [ origin ] [ severity ]
        );
    pre_size += sz;

#if _DEBUGGING
    string_printf ( & pre [ pre_size ], sizeof pre - pre_size, & sz
                    , "( %s ) "
                    , xc -> name
        );
    pre_size += sz;
#endif

    string_printf ( & pre [ pre_size ], sizeof pre - pre_size, & sz
                    , "%s %s"
                    , extract_desc_xobj ( xc )
                    , extract_desc_xstate ( xc )
        );
    pre_size += sz;

    if ( msg == NULL || msg [ 0 ] == 0 )
        fprintf ( stderr, "%s.\n", pre );
    else
        fprintf ( stderr, "%s - %s.\n", pre, msg );

#if _DEBUGGING
    if ( node != NULL )
    {
        fprintf ( stderr, "Stack Trace:\n" );
        do
        {
            fprintf ( stderr
                      , "    %s/%s.%s:%s\n"
                      , node -> loc -> src -> mod
                      , node -> loc -> src -> file
                      , node -> loc -> src -> ext
                      , node -> loc -> func
                );
            node = node -> parent;
        }
        while ( node != NULL );
    }
    else if ( ctx != NULL )
    {
        fprintf ( stderr, "Stack Trace:\n" );
        do
        {
            fprintf ( stderr
                      , "    %s/%s.%s:%s\n"
                      , ctx -> loc -> src -> mod
                      , ctx -> loc -> src -> file
                      , ctx -> loc -> src -> ext
                      , ctx -> loc -> func
                );
            ctx = ctx -> caller;
        }
        while ( ctx != NULL );
    }
#endif
}

/* Whack
 */
static
KThreadEvent * KThreadEventWhack ( KThreadEvent * self )
{
    KThreadEvent * next = self -> next;

    /* dump event to log */
    KThreadEventDump ( NULL, self -> timestamp, self -> node -> loc,
       self -> lineno, self -> severity, self -> origin,
       self -> xc, self -> message . addr, self -> node );

    free ( self );
    return next;
}

static
void KThreadEventWhackAll ( KThreadEvent * self )
{
    do
        self = KThreadEventWhack ( self );
    while ( self != NULL );
}

/* Clear
 */
static
void KThreadEventClear ( KThreadEvent * self )
{
    if ( self -> next != NULL )
        KThreadEventClear ( self -> next );
    free ( self );
}

/* Make
 */
static
KThreadEvent * KThreadEventMake ( ctx_t ctx,
    uint32_t lineno, xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char * msg, va_list args )
{
    KThreadEvent * evt;
    KTime_t ts = KTimeStamp ();

    char * c;
    size_t num_writ;
    char msg_buffer [ 4096 ];
    rc_t rc = string_vprintf ( msg_buffer, sizeof msg_buffer, & num_writ, msg, args );
    if ( rc != 0 || num_writ >= sizeof msg_buffer )
        string_printf ( msg_buffer, sizeof msg_buffer, & num_writ, "** BAD MESSAGE STRING **" );

    if ( num_writ > 0 && msg_buffer [ num_writ - 1 ] == '.' )
        msg_buffer [ -- num_writ ] = 0;

    evt = malloc ( sizeof * evt + num_writ + 1 );
    if ( evt == NULL )
    {
        /* ATTEMPT TO DUMP TO LOG */
        KThreadEventDump ( ctx, ts, ctx -> loc, lineno, severity, origin, xc, msg_buffer, NULL );
        exit ( -1 );
    }

    evt -> node = NULL;
    evt -> next = NULL;

    evt -> xc = ( const XCErr * ) xc;
    evt -> timestamp = ts;
    evt -> severity = severity;
    evt -> origin = origin;
    evt -> lineno = lineno;

    c = ( char* ) ( evt + 1 );
    memmove ( c, msg_buffer, num_writ + 1 );
    StringInit ( & evt -> message, c, num_writ, string_len ( c, num_writ ) );

    return evt;
}


/*--------------------------------------------------------------------------
 * KThreadEvtNode
 *  exection state belonging to the current thread
 */


/* Whack
 */
static
void KThreadEvtNodeWhackAll ( KThreadEvtNode * self )
{
    if ( self -> evt )
        KThreadEventWhackAll ( self -> evt );

    if ( self -> child )
        KThreadEvtNodeWhackAll ( self -> child );

    if ( self -> next )
        KThreadEvtNodeWhackAll ( self -> next );

    free ( self );
}


/* Clear
 */
static
void KThreadEvtNodeClear ( KThreadEvtNode * self )
{
    if ( self -> evt )
        KThreadEventClear ( self -> evt );

    if ( self -> child )
        KThreadEvtNodeClear ( self -> child );

    if ( self -> next )
        KThreadEvtNodeClear ( self -> next );

    free ( self );
}

/* Make
 */
static
KThreadEvtNode * KThreadEvtNodeMake ( ctx_t ctx )
{
    KThreadEvtNode * node = ts_alloc ( sizeof * node, true );
    node -> loc = ctx -> loc;
    node -> zdepth = ctx -> zdepth;
    return node;
}


/* AddChild
 */
static
KThreadEvtNode * KThreadEvtNodeAddChild ( KThreadEvtNode * self, KThreadEvtNode * child )
{
    KThreadEvtNode * existing = self -> child;
    if ( existing == NULL )
        self -> child = child;
    else
    {
        while ( existing -> next != NULL )
            existing = existing -> next;
        existing -> next = child;
    }
    child -> parent = self;
    return self;
}


/* AddEvent
 */
static
void KThreadEvtNodeAddEvent ( KThreadEvtNode * self, KThreadEvent * evt )
{
    KThreadEvent * existing = self -> evt;
    if ( existing == NULL )
        self -> evt = evt;
    else
    {
        while ( existing -> next != NULL )
            existing = existing -> next;
        existing -> next = evt;
    }
    evt -> node = self;
}


/* Unlink
 */
static
void KThreadEvtNodeUnlink ( KThreadEvtNode * self )
{
    KThreadEvtNode * par, * sib;

    par = self -> parent;
    assert ( par != NULL );
    sib = par -> child;

    if ( sib == self )
        par -> child = self -> next;
    else
    {
        while ( sib -> next != self )
        {
            assert ( sib -> next != NULL );
            sib = sib -> next;
        }
        sib -> next = self -> next;
    }

    self -> parent = NULL;
    self -> next = NULL;
}


/* Locate
 */
static
KThreadEvtNode * KThreadEvtNodeLocate ( KThreadEvtNode * self, uint32_t zdepth )
{
    assert ( zdepth <= self -> zdepth );
    while ( zdepth < self -> zdepth )
    {
        assert ( self -> parent != NULL );
        self = self -> parent;
    }
    return self;
}


/* FindDominant
 */
static
KThreadEvent * KThreadEvtNodeFindDominant ( const KThreadEvtNode * self, const KThreadEvtNode * prune, KThreadEvent * dom )
{
    /* scan events within this node */
    KThreadEvent * evt = self -> evt;
    while ( evt != NULL )
    {
        if ( dom == NULL )
            dom = evt;
        else if ( evt -> severity > dom -> severity )
            dom = evt;
        evt = evt -> next;
    }

    /* scan child events */
    self = self -> child;
    while ( self != NULL )
    {
        if ( self != prune )
            dom = KThreadEvtNodeFindDominant ( self, NULL, dom );
        self = self -> next;
    }

    return dom;
}


/*--------------------------------------------------------------------------
 * KThreadState
 *  exection state belonging to the current thread
 */
struct KThreadState
{
    KThreadEvtNode * eroot;
};


/* Whack
 */
void KThreadStateWhack ( KThreadState * self )
{
    if ( self != NULL )
    {
        if ( self -> eroot != NULL )
            KThreadEvtNodeWhackAll ( self -> eroot );
        free ( self );
    }
}


/* MakeThreadState
 *  creates state for a newly created thread
 *  called from the new thread
 */
KThreadState * KProcMgrMakeThreadState ( struct KProcMgr const * self )
{
    if ( self != NULL )
    {
        KThreadState * tstate = ts_alloc ( sizeof * tstate, true );
        return tstate;
    }

    return NULL;
}


/* CaptureEvent
 *  records an event from the exception mechanism
 */
void KThreadStateEvent ( KThreadState * self, ctx_t ctx,
    uint32_t lineno, xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char * msg, va_list args )
{
    KThreadEvtNode * node, * par;
    KThreadEvent * evt = KThreadEventMake ( ctx, lineno, severity, origin, xc, msg, args );

    /* prepare an RC for this event */
    rc_t rc = 0;
    if ( severity > xc_sev_warn )
    {
        rc = extract_rc ( ctx -> loc, xc );
        assert ( rc != 0 );
    }

    /* handle early errors before managers */
    if ( self == NULL )
    {
        KThreadEventDump ( ctx, evt -> timestamp, ctx -> loc,
            lineno, severity, origin, xc, evt -> message . addr, NULL );
        free ( evt );
        return;
    }

    /* CASES:

       1. the thread has no existing event state
          - create new node for call stack
          - add event to the node
          - propagate the event and rc up the ctx stack
          - capture stack trace as node chain
          - record root node

       2. the current stack frame already has an event
          - find the corresponding node in stack trace
          - add event to this node
          - propagate up call chain while severity > caller.severity

       3. the current stack frame is clear, but some caller has state
          - create new node for call stack
          - add event to the node
          - propagate the event and rc up the ctx stack until frame has event
          - capture stack trace as node chain
          - find the corresponding node in stack trace
          - add node from child frame to common parent node
          - continue to propagate up call chain while severity > caller.severity

    */


    /* case 1 - just propagate up the stack */
    if ( self -> eroot == NULL )
    {
        assert ( ctx -> evt == NULL );
        assert ( ctx -> rc == 0 );

        /* create a new event node */
        node = KThreadEvtNodeMake ( ctx );

        /* add the event */
        KThreadEvtNodeAddEvent ( node, evt );
        ( ( KCtx * ) ctx ) -> evt = evt;
        ( ( KCtx * ) ctx ) -> rc = rc;

        /* propagate up the stack */
        while ( ctx -> caller != NULL )
        {
            /* bump up */
            ctx = ctx -> caller;
            par = KThreadEvtNodeMake ( ctx );

            /* link */
            node = KThreadEvtNodeAddChild ( par, node );

            /* record the event */
            ( ( KCtx * ) ctx ) -> evt = evt;
            ( ( KCtx * ) ctx ) -> rc = rc;
        }

        /* record root node */
        self -> eroot = node;
    }

    /* cases 2 & 3 - will need to merge */
    else
    {
        KThreadEvent * existing = ctx -> evt;

        /* case 2 - current frame has an event */
        if ( ctx -> evt != NULL )
        {
            /* NB - this may not be so simple, if
               a canonical ordering is to be achieved.
               this code falls through to normal propagation,
               that only favors the new event over existing
               if it is more severe in nature.

               canonical ordering would be:
               a) events originating in a frame always
                  order before child events
               b) dominant event should be selected
                  first by severity and second by order

               as it stands, the code allows a previous child
               event of the same severity to remain dominant.

            */

            node = KThreadEvtNodeLocate ( existing -> node, ctx -> zdepth );
            KThreadEvtNodeAddEvent ( node, evt );
        }

        /* case 3 - current frame is clear */
        else
        {
            assert ( ctx -> rc == 0 );

            /* create a new event node */
            node = KThreadEvtNodeMake ( ctx );

            /* add the event */
            KThreadEvtNodeAddEvent ( node, evt );
            ( ( KCtx * ) ctx ) -> evt = evt;
            ( ( KCtx * ) ctx ) -> rc = rc;

            /* bump up */
            assert ( ctx -> caller != NULL );
            ctx = ctx -> caller;

            /* propagate up the stack */
            while ( ctx -> evt == NULL )
            {
                /* make node for this frame */
                par = KThreadEvtNodeMake ( ctx );

                /* link */
                node = KThreadEvtNodeAddChild ( par, node );

                /* record the event */
                ( ( KCtx * ) ctx ) -> evt = evt;
                ( ( KCtx * ) ctx ) -> rc = rc;

                /* bump up */
                assert ( ctx -> caller != NULL );
                ctx = ctx -> caller;
            }

            /* found nearest existing event */
            existing = ctx -> evt;

            /* locate common parent node */
            par = KThreadEvtNodeLocate ( existing -> node, ctx -> zdepth );
            node = KThreadEvtNodeAddChild ( par, node );
        }

        /* cases 2 & 3 - propagate event while severity > existing */
        while ( severity > existing -> severity )
        {
            do
            {
                /* overwrite prior */
                ( ( KCtx * ) ctx ) -> evt = evt;
                ( ( KCtx * ) ctx ) -> rc = rc;

                /* bump up */
                ctx = ctx -> caller;
                if ( ctx == NULL )
                    return;
            }
            while ( ctx -> evt == ( void * ) existing );

            assert ( ctx -> evt != NULL );
            existing = ctx -> evt;
        }
    }

    if ( severity == xc_sev_fatal )
        exit ( -1 );
}


/* ClearEvents
 *  clears events from a particular point in the callchain
 *  down toward lower points.
 */
void KThreadStateClearEvents ( KThreadState * self, ctx_t ctx )
{
    if ( self != NULL )
    {
        assert ( ctx != NULL );
        if ( ctx -> evt != NULL )
        {
            /* the recorded event */
            KThreadEvent * evt = ctx -> evt;

            /* locate the node for this frame */
            KThreadEvtNode * node = KThreadEvtNodeLocate ( evt -> node, ctx -> zdepth );

            /* douse it and everything below it */
            if ( node == self -> eroot )
            {
                self -> eroot = NULL;
                KThreadEvtNodeClear ( node );

                do
                {
                    ( ( KCtx * ) ctx ) -> evt = NULL;
                    ( ( KCtx * ) ctx ) -> rc = 0;

                    ctx = ctx -> caller;
                }
                while ( ctx != NULL );
            }
            else
            {
                KThreadEvtNode * par = node -> parent;

                KThreadEvtNodeUnlink ( node );
                KThreadEvtNodeClear ( node );

                /* WARNING - "evt" and "node" are now dangling! */

                ( ( KCtx * ) ctx ) -> evt = NULL;
                ( ( KCtx * ) ctx ) -> rc = 0;

                assert ( ctx -> caller != NULL );
                ctx = ctx -> caller;

                /* look for nodes with no more events */
                for ( node = par; node != NULL; node = par )
                {
                    if ( node -> child != NULL || node -> evt != NULL )
                        break;

                    par = node -> parent;
                    if ( par == NULL )
                        self -> eroot = NULL;
                    else
                        KThreadEvtNodeUnlink ( node );
                    KThreadEvtNodeClear ( node );

                    assert ( ctx != NULL );
                    assert ( ctx -> evt == ( void * ) evt );
                    ( ( KCtx * ) ctx ) -> evt = NULL;
                    ( ( KCtx * ) ctx ) -> rc = 0;

                    ctx = ctx -> caller;
                }

                assert ( node == NULL || ctx != NULL );

                /* see if it was cleared all the way to root */
                if ( node == NULL )
                    self -> eroot = NULL;

                /* see if the dominant evt from the cleared node
                   had propagated higher up and needs to be reset */
                else if ( ctx -> evt == ( void* ) evt )
                {
                    rc_t rc = 0;
                    KThreadEvent * dom;
                    KThreadEvtNode * par;

                    assert ( ctx != NULL );

                    /* AT THIS POINT:

                       the current "node", a parent of the node
                       originally being cleared, either has its
                       own event(s) or has children that do.

                       the dominant event of the node we just
                       cleared ( dangling pointer in "evt" ) had
                       been propagated as the dominant event of
                       this ( and possibly further ) parents.

                       we need to establish a new dominant event.
                       this will be accomplished by visiting the
                       current node and all of its children to find
                       the node with the highest severity as first
                       key, and first in order as second key.

                       NB - we may need to review the code to ensure
                       there is a canonical ordering in all cases.

                     */

                    dom = KThreadEvtNodeFindDominant ( node, NULL, NULL );
                    assert ( dom != NULL );
                    if ( dom -> severity > xc_sev_warn )
                        rc = extract_rc ( node -> loc, dom -> xc );

                    ( ( KCtx * ) ctx ) -> evt = dom;
                    ( ( KCtx * ) ctx ) -> rc = rc;

                    par = node -> parent;
                    ctx = ctx -> caller;

                    while ( par != NULL )
                    {
                        KThreadEvent * dom2;

                        /* early exit optimization */
                        if ( ctx -> evt != ( void * ) evt )
                            break;

                        /* compete for dominance */
                        dom2 = KThreadEvtNodeFindDominant ( par, node, dom );
                        if ( dom2 != dom )
                        {
                            rc = 0;
                            if ( dom2 -> severity > xc_sev_warn )
                                rc = extract_rc ( node -> loc, dom2 -> xc );
                            dom = dom2;
                        }

                        /* update context */
                        ( ( KCtx * ) ctx ) -> evt = dom;
                        ( ( KCtx * ) ctx ) -> rc = rc;

                        /* bump up */
                        par = node -> parent;
                        ctx = ctx -> caller;
                    }
                }
            }
        }
    }
}


/* GetMessage
 *  retrieve current event message
 */
const char * KThreadStateGetMessage ( const KThreadState * self, ctx_t ctx )
{
    if ( self != NULL )
    {
        assert ( ctx != NULL );
        if ( ctx -> evt != NULL )
        {
            KThreadEvent * evt = ctx -> evt;
            return evt -> message . addr;
        }
    }

    return "";
}


/* IsXCErr
 */
bool KThreadStateIsXCErr ( const KThreadState * self, ctx_t ctx, xc_t xc )
{
    if ( self != NULL )
    {
        assert ( ctx != NULL );
        if ( ctx -> evt != NULL )
        {
            KThreadEvent * evt = ctx -> evt;
            const XCErr * err = evt -> xc;
            do
            {
                if ( err == ( const XCErr * ) xc )
                    return true;

                err = err -> dad;
            }
            while ( err != NULL );
        }
    }

    return false;
}

/* IsXCObj
 */
bool KThreadStateIsXCObj ( const KThreadState * self, ctx_t ctx, xobj_t xo )
{
    if ( self != NULL )
    {
        assert ( ctx != NULL );
        if ( ctx -> evt != NULL )
        {
            KThreadEvent * evt = ctx -> evt;
            const XCErr * err = evt -> xc;
            const XCObj * obj;

            while ( err -> dad != NULL )
                err = err -> dad;

            obj = err -> obj;

            do
            {
                if ( obj == ( const XCObj * ) xo )
                    return true;

                obj = obj -> dad;
            }
            while ( obj != NULL );
        }
    }

    return false;
}

/* IsXCState
 */
bool KThreadStateIsXCState ( const KThreadState * self, ctx_t ctx, xstate_t xs )
{
    if ( self != NULL )
    {
        assert ( ctx != NULL );
        if ( ctx -> evt != NULL )
        {
            KThreadEvent * evt = ctx -> evt;
            const XCErr * err = evt -> xc;
            const XCState * state;

            while ( err -> dad != NULL )
                err = err -> dad;

            state = err -> state;

            do
            {
                if ( state == ( const XCState * ) xs )
                    return true;

                state = state -> dad;
            }
            while ( state != NULL );
        }
    }

    return false;
}
