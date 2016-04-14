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
#include <kproc/lock.h>
#include <vfs/path.h>
#include <kns/http.h>

#include <kfg/repository.h>

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/file-impl.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"
#include "spen.h"
#include <xfs/path.h>

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     special structs _sP and _sPB are living here
((*/

struct XFSBurro;

struct _sPB {
    struct _sPB * prev;
    struct _sPB * next;

    struct XFSBurro * burro;
};

struct _sP {
    struct _sPB * head;
    struct _sPB * tail;

    struct KLock * mutabor;

    size_t capacity;
    size_t qty;
};

/*))
 //     XFSPen and XFSBurro are living here
((*/

struct XFSPen {
    struct BSTree tree;

    struct _sP * pen;

    struct KLock * mutabor;
};

struct XFSBurro {
    struct BSTNode node;

    struct KLock * mutabor;
    KRefcount refcount;

    const struct XFSPen * pen;
    struct _sPB * burro;

    const char * url;
    uint32_t url_hash;

    const struct KFile * file;

    size_t connect_attempts;
    XFSStatus status;
};

static const char * _sBurro_classname = "Burro";
static size_t _sDefaultConnectAttempts = 3;

static rc_t CC _CloseBurro_callback (
                                struct _sP * self,
                                struct _sPB * Burro
                                );
#ifdef NOT_NEED_YET
static rc_t CC _CloseBurroAndDelete_callback (
                                struct _sP * self,
                                struct _sPB * Burro
                                );
#endif /* NOT_NEED_YET */
static rc_t CC _sPTrimToSize_NoLock (
                                struct _sP * self,
                                size_t Size,
                                rc_t ( CC * callback ) (
                                                    struct _sP * p,
                                                    struct _sPB * b
                                                    )
                                );

static
rc_t CC
_sPDispose ( struct _sP * self )
{
    if ( self != NULL ) {
        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
        }

        // _sPTrimToSize_NoLock ( self, 0, _CloseBurroAndDelete_callback );
        _sPTrimToSize_NoLock ( self, 0, _CloseBurro_callback );

        self -> qty = 0;
        self -> capacity = 0;

        free ( self );
    }

    return 0;
}   /* _sPDispose () */

static
rc_t CC
_sPMake ( struct _sP ** Pen, size_t Capacity )
{
    rc_t RCt;
    struct _sP * DasPen;

    RCt = 0;
    DasPen = NULL;

    XFS_CSAN ( Pen )
    XFS_CAN ( Pen )

    DasPen = calloc ( 1, sizeof ( struct _sP ) );
    if ( DasPen == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( DasPen -> mutabor ) );
    if ( RCt == 0 ) {
        DasPen -> capacity = Capacity;
        DasPen -> qty = 0;

        * Pen = DasPen;
    }

    if ( RCt != 0 ) {
        * Pen = NULL;

        _sPDispose ( DasPen );
    }

    return RCt;
}   /* _sPMake () */

static
rc_t CC
_sPDisconnect_NoLock (
                    struct _sP * self,
                    struct _sPB * Burro,
                    rc_t ( CC * callback ) (
                                        struct _sP * p,
                                        struct _sPB * b
                                        )
)
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Burro )

    if ( Burro -> next == NULL && Burro -> prev == NULL ) {
        if ( self -> head == Burro ) {
            self -> head = self -> tail = NULL;

            self -> qty --;

            if ( callback != NULL ) {
                RCt = callback ( self, Burro );
            }
        }
    }
    else {
            /* Donkey is a head of household
             */
        if ( Burro -> prev == NULL ) {
            if ( self -> head != Burro ) {
                return XFS_RC ( rcInvalid );
            }

            if ( Burro -> next != NULL ) {
                Burro -> next -> prev = NULL;
                self -> head = Burro -> next;
            }
            else {
                self -> head = self -> tail = NULL;
            }
        }
        else {
                /* Donkey is a tail of household
                 */
            if ( Burro -> next == NULL ) {
                if ( self -> tail != Burro ) {
                    return XFS_RC ( rcInvalid );
                }

                if ( Burro -> prev != NULL ) {
                    Burro -> prev -> next = NULL;
                    self -> tail = Burro -> prev;
                }
                else {
                    self -> head = self -> tail = NULL;
                }
            }
            else {
                Burro -> prev -> next = Burro -> next -> prev;
                Burro -> next -> prev = Burro -> prev -> next;
            }
        }

        self -> qty --;
        Burro -> next = Burro -> prev = NULL;


        if ( callback != NULL ) {
            RCt = callback ( self, Burro );
        }
    }

    return RCt;
}   /* _sPDisconnect_NoLock () */

rc_t CC
_sPTrimToSize_NoLock (
                    struct _sP * self,
                    size_t Size,
                    rc_t ( CC * callback ) (
                                        struct _sP * p,
                                        struct _sPB * b
                                        )
)
{
    rc_t RCt;
    struct _sPB * Burro;

    RCt = 0;
    Burro = NULL;

    XFS_CAN ( self )

    if ( 0 < self -> qty ) {
        while ( Size < self -> qty ) {
            Burro = self -> tail;
            RCt = _sPDisconnect_NoLock ( self, Burro, callback );
            if ( RCt != 0 ) {
                break;
            }

            if ( Burro != NULL && Burro == self -> tail ) {
                RCt = XFS_RC ( rcInvalid );
                break;
            }
        }
    }

    return RCt;
}   /* _sPTrimToSize_NoLock () */

static
rc_t CC
_sPConnect_NoLock ( struct _sP * self, struct _sPB * Burro )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Burro )

        /* Donkey is busy already 
         */
    if ( Burro -> next != NULL || Burro -> prev != NULL ) {
        return XFS_RC ( rcInvalid );
    }

        /* Trimming pen to size
         */
    RCt = _sPTrimToSize_NoLock (
                                self,
                                self -> capacity - 1,
                                _CloseBurro_callback
                                );
    if ( RCt == 0 ) {
            /* There are no donkeys in a pen
             */
        if ( self -> head != NULL ) {
            Burro -> next = self -> head;
            self -> head -> prev = Burro;
        }
        else {
            self -> tail = Burro;
        }
        self -> head = Burro;
        self -> qty ++;
    }

    return RCt;
}   /* _sPConnect_NoLock () */

static
rc_t CC
_sPToFront ( struct _sP * self, struct _sPB * Burro )
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Burro )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {

        // if ( Burro != self -> head ) {

            RCt = _sPDisconnect_NoLock ( self, Burro, NULL );
            if ( RCt == 0 ) {
                RCt = _sPConnect_NoLock ( self, Burro );
            }
        // }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _sPToFront () */

rc_t CC
_CloseBurro_callback ( struct _sP * self, struct _sPB * Burro )
{
    rc_t RCt;
    const struct XFSBurro * TheBurro;
    struct KFile * File;

    RCt = 0;
    TheBurro = NULL;
    File = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Burro )

    TheBurro = Burro -> burro;

    XFS_CAN ( TheBurro )

    RCt = KLockAcquire ( TheBurro -> mutabor );
    if ( RCt == 0 ) {
        if ( TheBurro -> file != NULL ) {
            File = ( struct KFile * ) TheBurro -> file;
            ( ( struct XFSBurro * ) TheBurro ) -> file = NULL;
        }
        KLockUnlock ( TheBurro -> mutabor );
    }

    if ( RCt == 0 && File != 0 ) {
        KFileRelease ( File );
    }

    return RCt;
}   /* _CloseBurro_callback () */

#ifdef NOT_NEED_YET
rc_t CC
_CloseBurroAndDelete_callback ( struct _sP * self, struct _sPB * Burro )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Burro )

    RCt = _CloseBurro_callback ( self, Burro );

    free ( Burro );

    return RCt;
}   /* _CloseBurroAndDelete_callback () */
#endif /* NOT_NEED_YET */

static
rc_t CC
_sPBMake ( struct _sPB ** RetVal, struct XFSBurro * Burro )
{
    struct _sPB * TheRet = NULL;

    XFS_CSAN ( RetVal )
    XFS_CAN ( RetVal )
    XFS_CAN ( Burro )

    TheRet = calloc ( 1, sizeof ( struct _sPB ) );
    if ( TheRet == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    TheRet -> burro = Burro;
    * RetVal = TheRet;

    return 0;
}   /* _sPBMake () */

static
rc_t CC
_sPBDispose ( struct _sPB * self )
{
    if ( self != NULL ) {
            /* We do not check if Burro was not disconnected
               cuz it is too much to expect from us
               Just shut them down
             */
        self -> prev = NULL;
        self -> next = NULL;
        self -> burro = NULL;

        free ( self );
    }
    return 0;
}   /* _sPBDispose () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSBurro                                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_BurroDispose ( const struct XFSBurro * self )
{
    struct XFSBurro * Burro = ( struct XFSBurro * ) self;

    if ( Burro != NULL ) {
        KRefcountWhack ( & ( Burro -> refcount ), _sBurro_classname );

        if ( Burro -> mutabor !=  NULL ) {
            KLockRelease ( Burro -> mutabor );
            Burro -> mutabor = NULL;
        }

        if ( Burro -> burro != NULL ) {
            _sPBDispose ( Burro -> burro );
            Burro -> burro = NULL;
        }

        if ( Burro -> url != NULL ) {
            free ( ( char * ) Burro -> url );
            Burro -> url = NULL;
        }

        Burro -> url_hash = 0;

        if ( Burro -> file != NULL ) {
            KFileRelease ( Burro -> file );
            Burro -> file = NULL;
        }

        Burro -> connect_attempts = _sDefaultConnectAttempts;
        Burro -> status = kxfsInvalid;
        Burro -> pen = NULL;

        free ( Burro );
    }

    return 0;
}   /* _BurroDispose () */

static rc_t CC _PenAdd_NoLock (
                            const struct XFSPen * self,
                            const struct XFSBurro * Burro
                            );

static
rc_t CC
_BurroMake (
            const struct XFSPen * Pen,
            const struct XFSBurro ** Burro,
            const char * Url
)
{
    rc_t RCt;
    struct XFSBurro * TheBurro;

    RCt = 0;
    TheBurro = NULL;

    XFS_CSAN ( Burro )
    XFS_CAN ( Burro )
    XFS_CAN ( Url )

    TheBurro = calloc ( 1, sizeof ( struct XFSBurro ) );
    if ( TheBurro == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( TheBurro -> refcount ),
                1,
                _sBurro_classname,
                "BurroMake",
                "Burro"
                );

    RCt = KLockMake ( & ( TheBurro -> mutabor ) );
    if ( RCt == 0 ) {
        RCt = _sPBMake ( & ( TheBurro -> burro ), TheBurro );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Url, & ( TheBurro -> url ) );
            if ( RCt == 0 ) {
                RCt = _PenAdd_NoLock ( Pen, TheBurro );
                if ( RCt == 0 ) {
                    TheBurro -> url_hash = string_hash (
                                        TheBurro -> url,
                                        string_size ( TheBurro -> url )
                                        );
                    TheBurro -> file = NULL;
                    TheBurro -> connect_attempts =
                                            _sDefaultConnectAttempts;
                    TheBurro -> status = kxfsReady;
                    TheBurro -> pen = Pen;

                    * Burro = TheBurro;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Burro = NULL;

        if ( TheBurro != NULL ) {
            _BurroDispose ( TheBurro );
        }
    }

    return RCt;
}   /* _BurroMake () */

rc_t CC
XFSBurroAddRef ( const struct XFSBurro * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountAdd ( & ( self -> refcount ), _sBurro_classname );
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
}   /* XFSBurroAddRef () */

rc_t CC
XFSBurroRelease ( const struct XFSBurro * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    XFS_CAN ( self );

    RefC = KRefcountDrop ( & ( self -> refcount ), _sBurro_classname );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _BurroDispose ( self );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* XFSBurroRelease () */

bool CC
XFSBurroGood ( const struct XFSBurro * self )
{
    if ( self != NULL ) {
        return self -> status == kxfsReady
                || self -> status == kxfsGood;
    }

    return false;
}   /* XFSBurroGood () */

static
rc_t CC
_BurroCheckMakeFile_NoLock ( const struct XFSBurro * self )
{
    rc_t RCt;
    const struct KFile * File;

    RCt = 0;
    File = NULL;

    XFS_CAN ( self )

    if ( self -> status != kxfsReady && self -> status != kxfsComplete ) {
        return XFS_RC ( rcInvalid );
    }

    File = self -> file;
    if ( File == NULL ) {
        while ( 0 < self -> connect_attempts ) {
            ( ( struct XFSBurro * ) self ) -> status = kxfsReady;
            RCt = KNSManagerMakeHttpFile (
                                    XFS_KnsManager (),
                                    & File,
                                    NULL,
                                    0x01010000,
                                    self -> url
                                    );
            if ( RCt == 0 ) {
                ( ( struct XFSBurro * ) self ) -> status = kxfsComplete;
                break;
            }

            ( ( struct XFSBurro * ) self ) -> status = kxfsBroken;
            ( ( struct XFSBurro * ) self ) -> connect_attempts --;
        }
    }

    if ( RCt == 0 ) {
        ( ( struct XFSBurro * ) self ) -> connect_attempts =
                                            _sDefaultConnectAttempts;
        ( ( struct XFSBurro * ) self ) -> file = File;
    }
    else {
        ( ( struct XFSBurro * ) self ) -> file = NULL;
    }

    return RCt;
}   /* _BurroCheckMakeFile_NoLock () */

static
rc_t CC
_BurroGetFile (
                const struct XFSBurro * self,
                const struct KFile ** File,
                bool DropOld
)
{
    rc_t RCt;

    RCt = 0;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( self -> pen )
    XFS_CAN ( self -> burro )
    XFS_CAN ( File )

        /* Check if we are OK */
    if ( self -> status != kxfsReady && self -> status != kxfsComplete ) {
        return XFS_RC ( rcInvalid );
    }

        /* Read procedure is :
         *    Take Burro from a line and put it on the top
         *    Connect if necessary
         *    Read, or return Error
         */
    RCt = _sPToFront ( self -> pen -> pen, self -> burro );
    if ( RCt == 0 ) {
        RCt = KLockAcquire ( self -> mutabor );
        if ( RCt == 0 ) {
            if ( DropOld ) {
                if ( self -> file != NULL ) {
                    KFileRelease ( self -> file );
                    ( ( struct XFSBurro * ) self ) -> file = NULL;
                }
            }

            RCt = _BurroCheckMakeFile_NoLock ( self );
            if ( RCt == 0 ) {
                RCt = KFileAddRef ( self -> file );
                if ( RCt == 0 ) {
                    * File = self -> file;
                }
            }

            KLockUnlock ( self -> mutabor );
        }
    }

    return RCt;
}   /* _BurroGetFile () */

rc_t CC
XFSBurroCachedFile (
                const struct XFSBurro * self,
                const struct KFile ** File
)
{
    return _BurroGetFile ( self, File, false );
}   /* XFSBurroCachedFile () */

rc_t CC
XFSBurroRecachedFile (
                const struct XFSBurro * self,
                const struct KFile ** File
)
{
    return _BurroGetFile ( self, File, true );
}   /* XFSBurroRecachedFile () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSPen                                                            */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

rc_t CC
XFSPenMake ( const struct XFSPen ** Pen, size_t Capacity )
{
    rc_t RCt;
    struct XFSPen * ThePen;

    RCt = 0;
    ThePen = NULL;

    XFS_CSAN ( Pen )
    XFS_CAN ( Pen )

    ThePen = calloc ( 1, sizeof ( struct XFSPen ) );
    if ( ThePen == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    BSTreeInit ( & ( ThePen -> tree ) );

    RCt = KLockMake ( & ( ThePen -> mutabor ) );
    if ( RCt == 0 ) {
        RCt = _sPMake ( & ( ThePen -> pen ), Capacity );
        if ( RCt == 0 ) {
            * Pen = ThePen;
        }
    }

    if ( RCt != 0 ) {
        * Pen = NULL;

        if ( ThePen != NULL ) {
            XFSPenDispose ( ThePen );
        }
    }

    return 0;
}   /* XFSPenMake () */

static
void CC
_PenTreeWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSBurroRelease ( ( const struct XFSBurro * ) Node );
    }
}   /* _PenTreeWhackCallback () */

rc_t CC
XFSPenDispose ( const struct XFSPen * self )
{
    struct XFSPen * Pen = ( struct XFSPen * ) self;

    if ( Pen != NULL ) {
        if ( Pen -> mutabor != NULL ) {
            KLockRelease ( Pen -> mutabor );
            Pen -> mutabor = NULL;
        }

        if ( Pen -> pen != NULL ) {
            _sPDispose ( Pen -> pen );
            Pen -> pen = NULL;
        }

        BSTreeWhack ( & ( Pen -> tree ), _PenTreeWhackCallback, NULL );

        free ( Pen );
    }

    return 0;
}   /* XFSPenDispose () */

static
int64_t CC
_PenFindCallback ( const void * Item, const struct BSTNode * Node )
{
    const char * ItemUrl, * NodeUrl;

    ItemUrl = ( const char * ) Item;
    NodeUrl = Node == NULL ? 0 : ( ( struct XFSBurro * ) Node ) -> url;

    return XFS_StringCompare4BST_ZHR ( ItemUrl, NodeUrl );
}   /* _PenFindCallback () */

static
rc_t CC
_PenFind_NoLock (
            const struct XFSPen * self,
            const struct XFSBurro ** Burro,
            const char * Url
)
{
    rc_t RCt;
    const struct XFSBurro * TheBurro;

    RCt = 0;
    TheBurro = NULL;

    XFS_CSAN ( Burro )
    XFS_CAN ( self )
    XFS_CAN ( Burro )
    XFS_CAN ( Url )

    TheBurro = ( struct XFSBurro * ) BSTreeFind ( 
                                            & ( self -> tree ),
                                            Url,
                                            _PenFindCallback
                                            );
    if ( TheBurro == NULL ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        * Burro = TheBurro;
    }

    return RCt;
}   /* _PenFind_NoLock () */

static
int64_t CC
_BurroAddCallback (
const struct BSTNode * N1,
const struct BSTNode * N2
)
{
    return XFS_StringCompare4BST_ZHR (
                                    ( ( struct XFSBurro * ) N1 ) -> url,
                                    ( ( struct XFSBurro * ) N2 ) -> url
                                    );
}   /* _BurroAddCallback () */

rc_t CC
_PenAdd_NoLock (
                const struct XFSPen * self,
                const struct XFSBurro * Burro
)
{
    XFS_CAN ( self )
    XFS_CAN ( Burro )

    return BSTreeInsert (
                        & ( ( ( struct XFSPen * ) self ) -> tree ),
                        ( struct BSTNode * ) Burro,
                        _BurroAddCallback
                        );
}   /* _PenAdd_NoLock () */

rc_t CC
XFSPenGet ( 
            const struct XFSPen * self,
            const struct XFSBurro ** Burro,
            const char * Url
)
{
    rc_t RCt;
    const struct XFSBurro * TheBurro;

    RCt = 0;
    TheBurro = NULL;

    XFS_CSAN ( Burro )
    XFS_CAN ( self )
    XFS_CAN ( Burro )
    XFS_CAN ( Url )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _PenFind_NoLock ( self, & TheBurro, Url );
        if ( GetRCState ( RCt ) == rcNotFound ) {
            RCt = _BurroMake ( self, & TheBurro, Url );
        }

        if ( RCt == 0 ) {
            RCt = XFSBurroAddRef ( TheBurro );
            if ( RCt == 0 ) {
                * Burro = TheBurro;
            }
        }

        KLockUnlock ( self -> mutabor );
    }

    if ( RCt != 0 ) {
        * Burro = NULL;

        if ( TheBurro != NULL ) {
            _BurroDispose ( TheBurro );
            TheBurro = NULL;
        }
    }

    return RCt;
}   /* XFSPenGet () */

