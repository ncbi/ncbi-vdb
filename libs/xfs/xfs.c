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

#include <xfs/xfs.h>
#include <klib/out.h>
#include <klib/text.h>
#include <kproc/thread.h>
#include "xfs-priv.h"

#include <sysalloc.h>
#include <stdlib.h> /* we are using calloc */
#include <string.h> /* we are using memset */

/*  Some forwards and declarations
 */
XFS_EXTERN rc_t CC XFSControlInitVT ( XFSControl * self );
static rc_t XFSVeryMainLoop ( const KThread * self, void * Data );

/*  The code will init XFSControl, it is calling external method
 *  XFSControlInitVT (), which is platform dependent
 */
LIB_EXPORT
rc_t CC
XFSControlInit ( XFSControl ** self, struct XFSPeer * Peer )
{
    rc_t RCt;
    XFSControl * TheControl;

    RCt = 0;
    TheControl = NULL;

    if ( self == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    if ( Peer == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    TheControl = *self;

    if ( TheControl != NULL ) {
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcUnexpected );
    }

    TheControl = calloc ( 1, sizeof ( XFSControl ) );
    if ( TheControl == NULL ) {
            /* MESSAGING? */
        return RC ( rcFS, rcNoTarg, rcAllocating, rcSelf, rcNull );
    }

    RCt = XFSControlInitVT ( TheControl );
    if ( RCt != 0 ) {
        free ( TheControl );

        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcUnexpected );
    }

        /*  I think that it is right place to initialize Peer field
         */
    TheControl -> Peer = Peer;
    RCt = TheControl -> vt -> v1.init ( TheControl );
    if ( RCt != 0 ) {
        TheControl -> vt -> v1.destroy ( TheControl );

        free ( TheControl );
    } else {
        * self = TheControl;
    }

    return RCt;
}   /* XFSControlInit () */

/*  The code, which is checking version is quite similar for
 *  destroy/start/stop ... and, possible for other methods,
 *  so we do that then.
 */
static
rc_t
XFSControlStandardSelfCheck( const XFSControl * self )
{
    if ( self == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    if ( self -> vt == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcInterface, rcNull );
    }

    if ( self -> Arguments == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcInterface, rcNull );
    }

        /*  Should be extended for switch later
         */
    if ( self -> vt -> v1 . maj != 1 && self -> vt -> v1 . min != 1 ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcInterface, rcInvalid );
    }

    return 0;
}   /* XFSControlStandardSelfCheck () */

/*  Control destroy.
 */
LIB_EXPORT
rc_t CC
XFSControlDestroy ( XFSControl * self )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );

    if ( RCt != 0 ) {
            /*  TODO : not sure about rc */
        return RCt;
    }

    if ( self -> Thread != NULL ) {
        KThreadRelease ( self -> Thread );

        self -> Thread = NULL;
    }

    self -> vt -> v1.destroy ( self );

    memset ( self, 0, sizeof ( XFSControl * ) );

    return RCt;
}   /* XFSControlDestroy () */

/*  Start file system
 */
LIB_EXPORT
rc_t CC
XFSStart ( XFSControl * self )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );

    if ( RCt != 0 ) {
            /*  TODO : not sure about rc */
        return RCt;
    }

    if ( self -> Thread != NULL ) {
        return RC ( rcFS, rcThread, rcConstructing, rcSelf, rcExists );
    }

        /* TODO */
    if ( self -> vt -> v1.mount == NULL ) {
        return RC ( rcFS, rcThread, rcConstructing, rcSelf, rcNull );
    }

    RCt = self -> vt -> v1.mount ( self );
    if ( RCt == 0 ) {
        RCt = KThreadMake ( & self -> Thread, XFSVeryMainLoop, self );
        if ( RCt == 0 ) {
            return 0;
        }
    }

    if ( self -> Thread != NULL ) {
        KThreadRelease ( self -> Thread );
        self -> Thread = NULL;
    }

    return RCt;
}   /* XFSStart () */

/*  Stop filesystem.
 */
LIB_EXPORT
rc_t CC
XFSStop ( XFSControl * self )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );

    if ( RCt == 0 ) {
        RCt = self -> vt -> v1.unmount ( self );
        if ( RCt == 0 ) {
            if ( self -> Thread != NULL ) {
                KThreadRelease ( self -> Thread );

                self -> Thread = NULL;
            }
        }
    }

    return RCt;
}   /* XFSStop () */

/*  Here we are goint to implement some XFSControlArgs methods
 */
LIB_EXPORT
rc_t CC
XFSControlSetMountPoint ( XFSControl * self, const char * MountPoint )
{
    rc_t RCt;

    if ( MountPoint == NULL ) { 
        return RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
    }

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt == 0 ) {
        char * TheMountPoint = self -> Arguments -> MountPoint;
        size_t SizeToCopy = sizeof ( self -> Arguments -> MountPoint );

        if ( string_copy_measure (
                            TheMountPoint,
                            SizeToCopy,
                            MountPoint
                            ) <= 0 )
        {
            RCt = RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
        }
    }

    return RCt;
}   /* XFSControlSetMountPoint () */

LIB_EXPORT
rc_t CC
XFSControlGetMountPoint (
                    const XFSControl * self,
                    char * Buffer,
                    size_t BufferSize
)
{
    rc_t RCt;

    if ( Buffer == NULL || BufferSize <= 0 ) { 
        return RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
    }

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt == 0 ) {
        const char * MountPoint = self -> Arguments -> MountPoint;

        if ( string_copy_measure (
                            Buffer,
                            BufferSize,
                            MountPoint
                            ) <= 0 )
        {
            RCt = RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
        }
    }

    return RCt;
}   /* XFSControlGetMountPoint () */

LIB_EXPORT
rc_t CC
XFSControlSetLabel ( XFSControl * self, const char * Label )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt == 0 ) {

        char * TheLabel = self -> Arguments -> Label;
        size_t SizeToCopy = sizeof ( self -> Arguments -> Label );

        size_t LabelSize = Label == NULL
                                ? 0
                                : string_size ( Label )
                                ;

        const char * SourceOfInspiration = LabelSize == 0
                                                ? "XFS"
                                                : Label
                                                ;

        if ( string_copy_measure (
                            TheLabel,
                            SizeToCopy,
                            SourceOfInspiration ) <= 0 )
        {
            RCt = RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
        }
    }

    return RCt;
}   /* XFSControlSetLabel () */

LIB_EXPORT
rc_t CC
XFSControlGetLabel (
                    const XFSControl * self,
                    char * Buffer,
                    size_t BufferSize
)
{
    rc_t RCt;

    if ( Buffer == NULL || BufferSize <= 0 ) { 
        return RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
    }

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt == 0 ) {
        const char * Label = self -> Arguments -> Label;

        if ( string_copy_measure ( Buffer, BufferSize, Label ) <= 0 ) {
            RCt = RC ( rcFS, rcString, rcCopying, rcParam, rcNull );
        }
    }

    return RCt;
}   /* XFSControlGetLabel () */

static
rc_t
XFSVeryMainLoop ( const KThread * self, void * Data )
{
    XFSControl * TheControl;

    TheControl = ( XFSControl * ) Data;

    if ( TheControl == NULL ) {
        return RC ( rcFS, rcThread, rcExecuting, rcSelf, rcNull );
    }

    if ( TheControl -> vt == NULL
        || TheControl -> vt -> v1.loop == NULL ) {
        return RC ( rcFS, rcThread, rcExecuting, rcSelf, rcNull );
    }

    return TheControl -> vt -> v1.loop ( TheControl );
}   /* XFSVeryMainLoop () */
