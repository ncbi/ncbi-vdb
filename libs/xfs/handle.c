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
#include <klib/refcount.h>
#include <klib/text.h>

#include <kfs/file.h>
#include <vfs/path.h>

#include <xfs/handle.h>
#include <xfs/node.h>

#include <sysalloc.h>

static const char * XFSHandle_classname = "XFSHandle";

struct XFSHandle {
    KRefcount refcount;

    const struct XFSNode * Node;

    void * Handle;
};

/*))
 // Creating Handle from scratch, OMG!
((*/
LIB_EXPORT
rc_t CC
XFSHandleMake (
                const struct XFSNode * Node,
                const struct XFSHandle ** Handle
)
{
    rc_t RCt;
    struct XFSHandle * RetVal;

    RCt = 0;
    RetVal = NULL;

    if ( Node == NULL || Handle == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Handle = NULL;

    RetVal = ( struct XFSHandle * ) calloc (
                                        1,
                                        sizeof ( struct XFSHandle )
                                        );
    if ( RetVal == NULL ) { 
        return XFS_RC ( rcNull );
    }
    
    RCt = XFSNodeAddRef ( Node );
    if ( RCt == 0 ) {
        KRefcountInit ( 
                    & ( RetVal -> refcount ),
                    1,
                    XFSHandle_classname,
                    "XFSHandleMake",
                    "Node Handle"
                    );

        RetVal -> Node = Node;
    }

    if ( RCt != 0 ) {
        free ( RetVal );
    }
    else {
        * Handle = RetVal;
    }

    return RCt;
}   /* XFSHandleMake () */

LIB_EXPORT
rc_t CC
XFSHandleDestroy ( const struct XFSHandle * self )
{
    struct XFSHandle * Handle = ( struct XFSHandle * ) self;
    if ( Handle != NULL ) { 
        KRefcountWhack ( & ( Handle -> refcount ), XFSHandle_classname );

        if ( Handle -> Node != NULL ) {
            XFSNodeRelease ( Handle -> Node );

            Handle -> Node = NULL;
        }

        if ( Handle -> Handle != NULL ) {
            /* TODO !!! Warning about active handle */

            Handle -> Handle = NULL;
        }

        free ( Handle );
    }

    return 0;
}   /* XFSHandleDestroy () */

LIB_EXPORT
rc_t CC
XFSHandleAddRef ( const struct XFSHandle * self )
{
    rc_t RCt;
    int32_t RetCode;
    struct XFSHandle * Handle;

    RCt = 0;
    RetCode = 0;
    Handle = ( struct XFSHandle * ) self;
    if ( Handle != NULL ) {
        RetCode = KRefcountAdd (
                        & ( Handle -> refcount ),
                        XFSHandle_classname
                        );
        switch ( RetCode ) {
            case krefOkay :
                    RCt = 0;
                    break;
            case krefZero :
            case krefLimit :
            case krefNegative :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcInvalid );
                    break;
            default :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcUnknown );
                    break;
        }
    }

    return RCt;
}   /* XFSHandleAddRef () */

LIB_EXPORT
rc_t CC
XFSHandleRelease ( const struct XFSHandle * self )
{
    rc_t RCt;
    int32_t RetCode;
    struct XFSHandle * Handle;

    RCt = 0;
    RetCode = 0;
    Handle = ( struct XFSHandle * ) self;
    if ( Handle != NULL ) {
        RetCode = KRefcountDrop (
                        & ( Handle -> refcount ),
                        XFSHandle_classname
                        );
        switch ( RetCode ) {
            case krefOkay :
            case krefZero :
                    RCt = 0;
                    break;
            case krefWhack :
                    XFSHandleDestroy ( Handle );
                    break;
            case krefNegative :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcInvalid );
                    break;
            default :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcUnknown );
                    break;
        }
    }

    return RCt;
}   /* XFSHandleRelease () */

LIB_EXPORT
void * CC
XFSHandleGet ( const struct XFSHandle * self )
{
    return self == NULL ? NULL : ( self -> Handle );
}   /* XFSHandleGetType () */

LIB_EXPORT
rc_t CC
XFSHandleSet ( const struct XFSHandle * self, void * Handle )
{
    struct XFSHandle * Self;

    Self = ( struct XFSHandle * ) self;

    if ( Self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Self -> Handle = Handle;

    return 0;
}   /* XFSHandleSet () */

LIB_EXPORT
const struct XFSNode * CC
XFSHandleNode ( const struct XFSHandle * self )
{
    return self == NULL ? NULL : ( self -> Node );
}   /* XFSHandleNode () */

