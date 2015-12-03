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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <kproc/lock.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <krypto/key.h>
#include <krypto/encfile.h>

#include <xfs/path.h>

#include "schwarzschraube.h"
#include "xencro.h"
#include "zehr.h"
#include "lockdpt.h"

#include <sysalloc.h>

#include <ctype.h>
#include <time.h>

/*||*\
  || Lyrics:
  || That file contains interface to Encoded files based on 'krypto.
  || package. Very experimental, cuz I do not really know what to do
  || here.
\*||*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSEncEntry and XFSEndKey and _EncDpt ( EncDepot ) structs are
 ||  living here
((*/

/*))
 //  EntryDepot this is responsible for sharing Enc resources
 \\  Not sure if we need to collect keys tho
 //
((*/
struct _EncDpt {
    BSTree tree;

    KLock * mutabor;
};

/*)) 
 //  Just a path to file
((*/
struct XFSEncEntry {
    BSTNode node;

    KRefcount refcount;
    KLock * mutabor;

    const char * path;

    struct KKey key;        /* Key for decription */

    uint64_t size;          /* File size and time, I need to cache */
    KTime_t time;           /* to avoid frequent file opening */

    XFSStatus status;

    const struct KFile * file;
};

static const char * _sXFSEncEntry_classname = "XFSEncEntry";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _EncDpt Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static struct _EncDpt * _sEncDpt = NULL;

static
struct _EncDpt * CC
_Dpt ()
{
    return _sEncDpt;
}   /* _Dpt () */

static
void CC
_EncDptWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSEncEntryDispose ( ( const struct XFSEncEntry * ) Node );
    }
}   /* _EncDptWhackCallback () */

static
rc_t CC
_EncDptDisposeImpl ( struct _EncDpt * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) { 
        BSTreeWhack (
                    & ( self -> tree ),
                    _EncDptWhackCallback,
                    NULL
                    );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return RCt;
}   /* _EncDptDisposeImpl () */

LIB_EXPORT
rc_t CC
XFSEncDepotDispose ()
{
    struct _EncDpt * EncDpt;

    EncDpt = _sEncDpt;
    _sEncDpt = NULL;

    if ( EncDpt != NULL ) {
        _EncDptDisposeImpl ( EncDpt );
    }

    return 0;
}   /* XFSEncDepotDispose () */

static
rc_t CC
_EncDptMakeImpl ( struct _EncDpt ** EncDpt )
{
    rc_t RCt;
    struct _EncDpt * TheD;

    RCt = 0;
    TheD = NULL;

    if ( EncDpt != NULL ) {
        * EncDpt = NULL;
    }

    if ( EncDpt == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheD = calloc ( 1, sizeof ( struct _EncDpt ) );
    if ( TheD == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( TheD -> mutabor ) );
    if ( RCt == 0 ) {
        BSTreeInit ( & ( TheD -> tree ) );

        * EncDpt = TheD;
    }

    if ( RCt != 0 ) {
        * EncDpt = NULL;
        _EncDptDisposeImpl ( TheD );
    }

    return RCt;
}   /* _EncDptMakeImpl () */

LIB_EXPORT
rc_t CC
XFSEncDepotInit ()
{
    rc_t RCt;
    struct _EncDpt * EncDpt;

    RCt = 0;
    EncDpt = NULL;

    if ( _sEncDpt == NULL ) {

        RCt = _EncDptMakeImpl ( & EncDpt );
        if ( RCt == 0 ) {
            if ( _sEncDpt == NULL ) {
                _sEncDpt = EncDpt;
            }
            else {
                _EncDptDisposeImpl ( EncDpt );
            }
        }
    }

    return RCt;
}   /* XFSEncDepotInit () */

static
int64_t CC
_EncCmpCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                ? NULL
                : ( ( struct XFSEncEntry * ) Node ) -> path
                ;
    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _EncCmpCallback () */

static
rc_t CC
_EncDptGetNoLock (
                struct _EncDpt * self,
                const char * Path,
                const struct XFSEncEntry ** EncEntry
)
{
    const struct XFSEncEntry * RetVal = NULL;

    if ( EncEntry != NULL ) {
        * EncEntry = NULL;
    }

    if ( self == NULL || Path == NULL || EncEntry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetVal = ( const struct XFSEncEntry * ) BSTreeFind (
                                                & ( self -> tree ),
                                                Path,
                                                _EncCmpCallback
                                                );


    * EncEntry = RetVal;

    return RetVal == NULL ? XFS_RC ( rcNotFound ) : 0;
}   /* _EncDptGetNoLock () */

static
int64_t CC
_EncAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
            ( ( struct XFSEncEntry * ) N1 ) -> path,
            ( ( struct XFSEncEntry * ) N2 ) -> path
            );
}   /* _EncAddCallback () */

static
rc_t CC
_EncDptAddNoLock (
                struct _EncDpt * self,
                const struct XFSEncEntry * EncEntry
)
{
    rc_t RCt = 0;

    if ( self == NULL || EncEntry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = BSTreeInsert (
                    & ( self -> tree ),
                    ( struct BSTNode * ) & ( EncEntry -> node ),
                    _EncAddCallback
                    );

    return RCt;
}   /* _EncDptAddNoLock () */

static
rc_t CC
_EncDptDelNoLock (
            struct _EncDpt * self,
            const struct XFSEncEntry * EncEntry
)
{
    if ( self == NULL || EncEntry == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeUnlink (
                & ( self -> tree ),
                ( struct BSTNode * ) & ( EncEntry -> node )
                );
    XFSEncEntryDispose ( EncEntry );

    return 0;
}   /* _EncDptDelNoLock () */

static
rc_t CC
_EncDptClearNoLock ( struct _EncDpt * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeWhack ( & ( self -> tree ), _EncDptWhackCallback, NULL );
    BSTreeInit ( & ( self -> tree ) );

    return 0;
}   /* _EncDptClearNoLock () */

LIB_EXPORT
rc_t CC
XFSEncDepotClear ()
{
    rc_t RCt;
    struct _EncDpt * D;

    RCt = 0;
    D = _Dpt ();

    if ( D == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( D -> mutabor );
    if ( RCt == 0 ) {
        RCt = _EncDptClearNoLock ( D );

        KLockUnlock ( D -> mutabor );
    }

    return RCt;
}   /* XFSEncDepotClear () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSEncEntry Methods ...                                           */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_EncEntryMake (
            const char * Path,
            const char * Passwd,
            const char * EncodeType,
            const struct XFSEncEntry ** EncEntry
)
{
    rc_t RCt;
    struct XFSEncEntry * TheEntry;

    RCt = 0;
    TheEntry = NULL;

    XFS_CSAN ( EncEntry )

    XFS_CAN ( Path )
    XFS_CAN ( Passwd )
    XFS_CAN ( EncEntry )

    TheEntry = calloc ( 1, sizeof ( struct XFSEncEntry ) );
    if ( TheEntry == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    TheEntry -> status = kxfsInvalid;

    RCt = XFS_InitKKey_ZHR (
                        Passwd,
                        EncodeType,
                        & ( TheEntry -> key )
                        );
    if ( RCt == 0 ) {
        RCt = KLockMake ( & ( TheEntry -> mutabor ) );
        if ( RCt == 0 ) {
            KRefcountInit (
                        & ( TheEntry -> refcount ),
                        1,
                        _sXFSEncEntry_classname,
                        "EncEntryMake",
                        "EncEntry"
                        );

            RCt = XFS_StrDup ( Path, & ( TheEntry -> path ) );
            if ( RCt == 0 ) {
                TheEntry -> status = kxfsReady;

                * EncEntry = TheEntry;
            }
        }
    }

    if ( RCt != 0 ) {
        * EncEntry = NULL;

        if ( TheEntry != NULL ) {
            XFSEncEntryDispose ( TheEntry );
        }
    }

    return RCt;
}   /* _EncEntryMake () */

static
rc_t CC
_EncDptFindOrCreateNoLock (
                struct _EncDpt * self,
                const char * Path,
                const char * Passwd,
                const char * EncodeType,
                const struct XFSEncEntry ** EncEntry
)
{
    rc_t RCt;
    const struct XFSEncEntry * Entry;

    RCt = 0;

    XFS_CSAN ( EncEntry )

    XFS_CAN ( Path )
    XFS_CAN ( Passwd )
    XFS_CAN ( EncEntry )

    RCt = _EncDptGetNoLock ( self, Path, & Entry );
    if ( RCt == 0 ) {
        RCt = XFSEncEntryAddRef ( Entry );
    }
    else {
        RCt = _EncEntryMake ( Path, Passwd, EncodeType, & Entry );
        if ( RCt == 0 ) {
            RCt = _EncDptAddNoLock ( self, Entry );
        }
    }

    if ( RCt == 0 ) {
        * EncEntry = Entry;
    }
    else {
        * EncEntry = NULL;

        if ( Entry != NULL ) {
            _EncDptDelNoLock ( self, Entry );
        }
    }

    return RCt;
}   /* _EncDptFindOrCreateNoLock () */

LIB_EXPORT
rc_t CC
XFSEncEntryFindOrCreate (
                const char * Path,
                const char * Passwd,
                const char * EncodeType,
                const struct XFSEncEntry ** EncEntry
)
{
    rc_t RCt;
    const struct XFSEncEntry * TheEntry;
    struct _EncDpt * Dpt;

    RCt = 0;
    TheEntry = NULL;
    Dpt = _Dpt ();

    XFS_CAN ( Dpt )

    XFS_CSAN ( EncEntry )

    RCt = KLockAcquire ( Dpt -> mutabor );
    if ( RCt == 0 ) {

        RCt = _EncDptFindOrCreateNoLock (
                                        Dpt,
                                        Path,
                                        Passwd,
                                        EncodeType,
                                        & TheEntry
                                        );
        if ( RCt == 0 ) {
            /* Here add to ... */

            * EncEntry = TheEntry;
        }

        KLockUnlock ( Dpt -> mutabor );
    }

    return RCt;
}   /* XFSEncEntryFindOrCreate () */

LIB_EXPORT
rc_t CC
XFSEncEntryDispose (
    const struct XFSEncEntry * self
)
{
    struct XFSEncEntry * Entry = ( struct XFSEncEntry * ) self;

    if ( Entry == NULL ) {
        return 0;
    }

    if ( XFSEncEntryIsOpen ( Entry ) ) {
        XFSEncEntryClose ( Entry );
    }

    Entry -> status = kxfsInvalid;

    if ( Entry -> file != NULL ) {
        KFileRelease ( Entry -> file );

        Entry -> file = NULL;
    }

    if ( Entry -> path != NULL ) {
        free ( ( char * ) Entry -> path );

        Entry -> path = NULL;
    }

    if ( Entry -> mutabor != NULL ) {
        KLockRelease ( Entry -> mutabor );

        Entry -> mutabor = NULL;
    }

    Entry -> size = 0;
    Entry -> time = 0;
    ( Entry -> key ) . type = kkeyTypeCount;

    KRefcountWhack ( & ( Entry -> refcount ), _sXFSEncEntry_classname );

    free ( Entry );

    return 0;
}   /* XFSEncEntryDispose () */


LIB_EXPORT
rc_t CC
XFSEncEntryAddRef ( const struct XFSEncEntry * self )
{
    rc_t RCt;
    struct XFSEncEntry * EncEntry;

    RCt = 0;
    EncEntry = ( struct XFSEncEntry * ) self;

    if ( EncEntry == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( EncEntry -> refcount ), _sXFSEncEntry_classname ) ) {
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
}   /* XFSEncEntryAddRef () */

LIB_EXPORT
rc_t CC
XFSEncEntryRelease ( const struct XFSEncEntry * self )
{
    rc_t RCt;
    struct XFSEncEntry * EncEntry;

    RCt = 0;
    EncEntry = ( struct XFSEncEntry * ) self;

    if ( EncEntry == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( EncEntry -> refcount ), _sXFSEncEntry_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSEncEntryClose (
                                    ( struct XFSEncEntry * ) EncEntry
                                    );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSEncEntryRelease () */

static
rc_t CC
_OpenEncFileNoLock ( const struct XFSEncEntry * self )
{
    rc_t RCt;
    struct XFSEncEntry * Entry;
    struct KDirectory * NatD;
    const struct KFile * File, * EncFile;
    uint32_t PathType;

    RCt = 0;
    Entry = ( struct XFSEncEntry * ) self;
    NatD = NULL;
    File = EncFile = NULL;
    PathType = kptNotFound;

    XFS_CAN ( Entry )

        /*  First we should check that entry is valid, or it's state
         *  is Ready or Complete
         */
    if ( Entry -> status != kxfsReady
        && Entry -> status != kxfsComplete ) {
        return XFS_RC ( rcInvalid );
    }

        /*  Second it is not open already
         */
    if ( Entry -> file != NULL ) {
        return 0;
    }

    RCt = KDirectoryNativeDir ( & NatD );
    if ( RCt == 0 ) {
            /*  Third if it does exist and it is a FILE
             */
        PathType = KDirectoryPathType ( NatD, Entry -> path );
        if ( PathType == kptFile ) {
            RCt = KDirectoryOpenFileRead (
                                        NatD,
                                        & File,
                                        Entry -> path
                                        );
            if ( RCt == 0 ) {
                RCt = KEncFileMakeRead (
                                        & EncFile,
                                        File,
                                        & Entry -> key
                                        );
                if ( RCt == 0 ) {
                        /*  Here we are reading missing data
                         */
                    if ( Entry -> status == kxfsReady ) {
                        RCt = KDirectoryDate (
                                            NatD,
                                            & ( Entry -> time ),
                                            Entry -> path
                                            );
                        if ( RCt == 0 ) {
                            RCt = KFileSize (
                                            EncFile,
                                            & ( Entry -> size )
                                            );
                            if ( RCt == 0 ) {
                                Entry -> status = kxfsComplete;
                            }
                        }
                    }
                }

                if ( RCt == 0 ) {
                    Entry -> file = EncFile;
                }

                KFileRelease ( File );
            }
        }
        else {
            Entry -> status = kxfsBroken;
            RCt = XFS_RC ( rcInvalid );
        }

        KDirectoryRelease ( NatD );
    }

    return RCt;
}   /* _OpenEncFileNoLock () */

static 
rc_t CC
_CloseEncFileNoLock ( const struct XFSEncEntry * self )
{
    rc_t RCt;
    struct XFSEncEntry * Entry;

    RCt = 0;
    Entry = ( struct XFSEncEntry * ) self;

    XFS_CAN ( Entry )

    if ( Entry -> file != NULL ) {
        KFileRelease ( Entry -> file );
        Entry -> file = NULL;
    }

    return RCt;
}   /* _CloseEncFileNoLock () */

static
rc_t CC
_CompleteEncEntryNoLock ( const struct XFSEncEntry * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    if ( self -> status == kxfsComplete ) {
        return 0;
    }

    if ( self -> status != kxfsReady ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _OpenEncFileNoLock ( self );
    if ( RCt == 0 ) {
        RCt = _CloseEncFileNoLock ( self );
    }

    return RCt;
}   /* _CompleteEncEntryNoLock () */

XFS_EXTERN
const char * CC
XFSEncEntryPath ( const struct XFSEncEntry * self )
{
    return self == NULL
                    ? NULL
                    : self -> path
                    ;
}   /* XFSEncEntryPath () */

XFS_EXTERN
rc_t CC
XFSEncEntrySize ( const struct XFSEncEntry * self, uint64_t * Size )
{
    rc_t RCt = 0;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) { 
            /* Here we are ... if it is in Ready state, we should open
               and read it. If it is in Complete state, we should just
               return result
             */
        RCt = _CompleteEncEntryNoLock ( self );
        if ( RCt == 0 ) {
            * Size = self -> size;
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSEncEntrySize () */

XFS_EXTERN
rc_t CC
XFSEncEntryTime ( const struct XFSEncEntry * self, KTime_t * Time )
{
    rc_t RCt = 0;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Time )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) { 
            /* Here we are ... if it is in Ready state, we should open
               and read it. If it is in Complete state, we should just
               return result
             */
        RCt = _CompleteEncEntryNoLock ( self );
        if ( RCt == 0 ) {
            * Time = self -> time;
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSEncEntryTime () */

XFS_EXTERN
bool CC
XFSEncEntryGood ( const struct XFSEncEntry * self )
{
    bool RetVal = false;

    if ( self != NULL ) {
        if ( KLockAcquire ( self -> mutabor ) == 0 ) {
            RetVal = self -> status == kxfsGood;

            KLockUnlock ( self -> mutabor );
        }
    }

    return RetVal;
}   /* XFSEncEntryGood () */

XFS_EXTERN
bool CC
XFSEncEntryIsOpen ( const struct XFSEncEntry * self )
{
    bool RetVal = false;

    if ( self != NULL ) {
        if ( KLockAcquire ( self -> mutabor ) == 0 ) {
            RetVal = self -> file != NULL;

            KLockUnlock ( self -> mutabor );
        }
    }

    return RetVal;
}   /* XFSEncEntryIsOpen () */

XFS_EXTERN
rc_t CC
XFSEncEntryOpen ( const struct XFSEncEntry * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _OpenEncFileNoLock ( self );

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSEncEntryOpen () */

XFS_EXTERN
rc_t CC
XFSEncEntryClose ( const struct XFSEncEntry * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _CloseEncFileNoLock ( self );

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSEncEntryClose () */

XFS_EXTERN
rc_t CC
XFSEncEntryRead (
            const struct XFSEncEntry * self,
            uint64_t Offset,
            void * Buffer,
            size_t BufferSize,
            size_t * NumRead
)
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )

    if ( BufferSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        if ( self -> file != NULL ) {
            if ( self -> status == kxfsComplete ) {
                RCt = KFileRead (
                                self -> file,
                                Offset,
                                Buffer,
                                BufferSize,
                                NumRead
                                );
            }
            else {
                RCt = XFS_RC ( rcInvalid );
            }
        }
        else {
            RCt = XFS_RC ( rcInvalid );
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSEncEntryReady () */
