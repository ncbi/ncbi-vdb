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
#include <kns/manager.h>
#include <kns/http.h>
#include <kns/stream.h>

#include <xfs/path.h>

#include "schwarzschraube.h"
#include "xhttp.h"
#include "zehr.h"
#include "lockdpt.h"

#include <sysalloc.h>

#include <ctype.h>
#include <os-native.h>
#include <time.h>
#include <stdio.h> /* sscanf */

/*||*\
  || Lyrics:
  || That file contains interface to SRA archive at
  ||
  ||     https://sra-download.ncbi.nlm.nih.gov/sra
  ||
  || If we will enter 'DRR000001' subdirectory, we will file here :
  ||
  || (https://sra-download.ncbi.nlm.nih.gov/srapub/DRR000001/)
  || 
  ||     DRR000001.sra           26-May-2014 07:22  569M  
  ||
  || So, code could looks little weird, parsing and serving that format
  ||
\*||*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) XFSHttp and XFSHttpEntry and _HttpED ( EntryDepot ) structs are
 ||  living here
((*/

/*))
 //  Status we will use in that module
((*/

/*)) Forward
 ((*/
struct _HttpHs;

/*))
 //  EntryDepot this is responsible for sharing HTTP resources
((*/
struct _HttpED {
    BSTree tree;
    KRefcount refcount;
    KLock * mutabor;

    struct XFSLockDepot * mutabors;
    struct _HttpHs * http_hs;
};

/*)) 
 //  Just a URL base to access entry by relative path.
((*/
struct XFSHttp {
    KRefcount refcount;

    const char * base_url;
};

/*)) 
 //  Just a URL base to access entry by relative path.
((*/
struct XFSHttpEntry {
    BSTNode node;
    KRefcount refcount;

    const char * name;
    const char * url;
    uint32_t url_hash;

    bool is_folder;

    struct VNamelist * list;    /* is null if not a folder */
    uint64_t size;              /* is ZERO if a folder */
    KTime_t time;

    XFSStatus status;
};

struct XFSHttpReader {
    KRefcount refcount;

    const struct XFSHttpEntry * entry;
};

/*) Usefuls
 (*/
static const char * _sXFSHttpED_classname = "XFSHttpEntryDepot";
static const char * _sXFSHttp_classname = "XFSHttp";
static const char * _sXFSHttpEntry_classname = "XFSHttpEntry";
static const char * _sXFSHttpReader_classname = "XFSHttpReader";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _HttpHs/_HttpHsEn Methods ...                                     */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#define DEFAULT_PIPIRDA_SIZE    64

struct _HttpHsEn {
    struct _HttpHsEn * next;
    struct _HttpHsEn * prev;

    const struct XFSHttpEntry * entry;
    const struct KFile * file;
};

struct _HttpHs {
    KLock * mutabor;

    uint32_t size;
    uint32_t qty;

    struct _HttpHsEn * kopf;
    struct _HttpHsEn * arse;
};

static
rc_t CC
_HttpHsEnMake ( struct XFSHttpEntry * Entry, struct _HttpHsEn ** HsEn )
{
    struct _HttpHsEn * TheSi = NULL;

    if ( HsEn != NULL ) {
        * HsEn = NULL;
    }

    if ( Entry == NULL || HsEn == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheSi = calloc ( 1, sizeof ( struct _HttpHsEn ) );
    if ( TheSi == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    TheSi -> entry = Entry;

    * HsEn = TheSi;

    return 0;
}   /* _HttpHsEnMake () */

static
rc_t CC
_HttpHsEnDispose ( struct _HttpHsEn * self )
{
    if ( self == NULL ) {
        return 0;
    }

    if ( self -> file != NULL ) {
        KFileRelease ( self -> file );

        self -> file = NULL;
    }

    self -> next = self -> prev = NULL;
    self -> entry = NULL;

    free ( self );

    return 0;
}   /* _HttpHsEnDispose () */

static
rc_t CC
_HttpHsEnConnect ( struct _HttpHsEn * self )
{
    rc_t RCt;
    const struct KNSManager * Manager;
    const struct KFile * File;

    RCt = 0;
    Manager = NULL;
    File = NULL;

    if ( self == NULL ) {
        return XFS_RC ( rcNull ) ;
    }

    if ( self -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self -> entry -> url == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self -> entry -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

        /*)) Not sure about it ((*/
    if ( self -> file != NULL ) {
        return 0;
    }

        /*] Here we are [*/
    Manager = XFS_KnsManager ();
    if ( Manager == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KNSManagerMakeHttpFile (
                                    Manager,
                                    & File,
                                    NULL,   /* no open connections */
                                    0x01010000,
                                    self -> entry -> url
                                    );
        if ( RCt == 0 ) {
            self -> file = File;
        }
    }

    return RCt;
}   /* _HttpHsEnConnect () */

static
void CC
_HttpHsRemNoLock ( struct _HttpHs * self, struct _HttpHsEn * HsEn )
{
    if ( self == NULL || HsEn == NULL ) {
        return;
    }

    if ( HsEn -> prev == NULL ) {       /* at the kopf */
        if ( HsEn -> next == NULL ) {
            self -> kopf = self -> arse = NULL;
        }
        else {
            self -> kopf = HsEn -> next;
            self -> kopf -> prev = NULL;
        }
    }
    else {
        if ( HsEn -> next == NULL ) {   /* at the arse */
            self -> arse = HsEn -> prev;
            self -> arse -> next = NULL;
        }
        else {                          /* in between */
            HsEn -> prev -> next = HsEn -> next;
            HsEn -> next -> prev = HsEn -> prev;
        }
    }

    HsEn -> prev = HsEn -> next = NULL;

    self -> qty ++;
}   /* _HttpHsRemNoLock () */

static
void CC
_HttpHsAddNoLock ( struct _HttpHs * self, struct _HttpHsEn * HsEn )
{
    if ( self == NULL || HsEn == NULL ) {
        return;
    }

        /*)) Note, addidng always to head
         ((*/
    if ( self -> kopf == NULL ) {
        HsEn -> next = HsEn -> prev = NULL;
        self -> kopf = self -> arse = HsEn;
    }
    else {
        self -> kopf -> prev = HsEn;
        HsEn -> next = self -> kopf;
        self -> kopf = HsEn;
    }

    self -> qty ++;
}   /* _HttpHsAddNoLock () */

static
void CC
_HttpHsMakeSlotNoLock ( const struct _HttpHs * self )
{
    struct _HttpHsEn * HsEn;

    if ( self == NULL ) {
        return;
    }

    if ( self -> arse == NULL ) {
        return;
    }

    while ( self -> size <= self -> qty ) {
        HsEn = self -> arse;
        if ( HsEn == NULL ) {
            break;
        }

        _HttpHsRemNoLock ( ( struct _HttpHs * ) self, HsEn );
        _HttpHsEnDispose ( HsEn );
    }
}   /* _HttpHsMakeSlotNoLock () */

static
struct _HttpHsEn * CC
_HttpHsFindNoLock (
                const struct _HttpHs * self,
                const struct XFSHttpEntry * Entry
)
{
    struct _HttpHsEn * HsEn = NULL;

    if ( self == NULL || Entry == NULL ) {
        return NULL;
    }

    HsEn = self -> kopf;

    while ( HsEn != NULL ) {
        if ( HsEn -> entry == Entry ) {
            _HttpHsRemNoLock ( ( struct _HttpHs * ) self, HsEn );
            _HttpHsAddNoLock ( ( struct _HttpHs * ) self, HsEn );
            return HsEn;
        }

        HsEn = HsEn -> next;
    }

    return NULL;
}   /* _HttpHsFindNoLock () */

static 
rc_t CC
_HttpHsMake ( uint32_t Size, struct _HttpHs ** Hs )
{
    rc_t RCt;
    struct _HttpHs * TheHs;

    RCt = 0;
    TheHs = NULL;

    if ( Hs == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Hs = NULL;

    TheHs = calloc ( 1, sizeof ( struct _HttpHs ) );
    if ( TheHs == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( TheHs -> mutabor ) );
    if ( RCt == 0 ) {
        TheHs -> size = Size == 0 ? DEFAULT_PIPIRDA_SIZE : Size ;
        TheHs -> qty = 0;
        * Hs = TheHs;
    }
    else {
        free ( TheHs );
    }

    return RCt;
}   /* _HttpHsMake () */

static
rc_t CC
_HttpHsDispose ( struct _HttpHs * self )
{
    bool AbleToLock;
    struct _HttpHsEn * HsEn;

    AbleToLock = false;
    HsEn = NULL;

    if ( self == NULL ) {
        return 0;
    }

    if ( self -> mutabor != NULL ) {
        AbleToLock = KLockAcquire ( self -> mutabor ) == 0;
    }

    while ( self -> kopf != NULL ) {
        HsEn = self -> kopf;

        _HttpHsRemNoLock ( ( struct _HttpHs * ) self, HsEn );
        _HttpHsEnDispose ( HsEn );
    }

    self -> kopf = self -> arse = NULL;
    self -> size = self -> qty = 0;

    if ( self -> mutabor != NULL ) {
        if ( AbleToLock ) {
            KLockUnlock ( self -> mutabor );
        }
        KLockRelease ( self -> mutabor );
    }

    free ( self );

    return 0;
}   /* _HttpHsDispose () */

static
rc_t CC
_HttpHsFindOrCreate (
                    const struct _HttpHs * self,
                    const struct XFSHttpEntry * Entry,
                    const struct _HttpHsEn ** HsEn
)
{
    rc_t RCt;
    struct _HttpHsEn * TheSi;

    RCt = 0;
    TheSi = NULL;

    if ( HsEn != NULL ) {
        * HsEn = NULL;
    }

    if ( self == NULL || Entry == NULL || HsEn == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
            /* First we are trying to find appropriate HttpHsEn
             */
        TheSi = _HttpHsFindNoLock ( self, Entry );
        if ( TheSi == NULL ) {
            RCt = _HttpHsEnMake (
                            ( struct XFSHttpEntry * ) Entry,
                            & TheSi
                            );
            if ( RCt == 0 ) {
                _HttpHsMakeSlotNoLock ( self );
                _HttpHsAddNoLock ( ( struct _HttpHs * ) self, TheSi );
            }
            else {
                TheSi = NULL;
                _HttpHsEnDispose ( TheSi );
            }
        }

        KLockUnlock ( self -> mutabor );
    }

    if ( RCt == 0 ) {
        if ( TheSi == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = _HttpHsEnConnect ( TheSi );
            if ( RCt == 0 ) {
                * HsEn = TheSi;
            }
        }
    }

    return RCt;
}   /* _HttpHsFindOrCreate () */

static
rc_t CC
_HttpHsDelete (
            const struct _HttpHs * self,
            const struct XFSHttpEntry * Entry
)
{
    rc_t RCt;
    struct _HttpHsEn * HsEn;

    RCt = 0;
    HsEn = NULL;

    if ( self == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        HsEn = _HttpHsFindNoLock ( self, Entry );
        if ( HsEn != NULL ) {
            _HttpHsRemNoLock ( ( struct _HttpHs * ) self, HsEn );
            _HttpHsEnDispose ( HsEn );
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* _HttpHsDelele () */

static
rc_t CC
_HttpHsGetKFile (
            const struct _HttpHs * self,
            const struct XFSHttpEntry * Entry,
            const struct KFile ** File
)
{
    rc_t RCt;
    const struct _HttpHsEn * HsEn;

    RCt = 0;
    HsEn = NULL;

    RCt = _HttpHsFindOrCreate ( self, Entry, & HsEn );
    if ( RCt == 0 ) {
        RCt = KFileAddRef ( HsEn -> file );
        if ( RCt == 0 ) {
            * File = HsEn -> file;
        }
    }

    return RCt;
}   /* _HttpHsGetKFile () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* _HttpED Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static struct _HttpED * _sHttpED = NULL;

static
struct _HttpED * CC
_ED ()
{
    return _sHttpED;
}   /* _ED () */

static
void CC
_HttpEDWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        XFSHttpEntryRelease ( ( struct XFSHttpEntry * ) Node );
    }
}   /* _HttpEDWhackCallback () */

static
rc_t CC
_HttpEDDisposeImpl ( struct _HttpED * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) { 
        BSTreeWhack (
                    & ( self -> tree ),
                    _HttpEDWhackCallback,
                    NULL
                    );

        KRefcountWhack (
                    & ( self -> refcount ),
                    _sXFSHttpED_classname
                    );

        XFSLockDepotDispose ( self -> mutabors );
        _HttpHsDispose ( self -> http_hs );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        free ( self );
    }

    return RCt;
}   /* _HttpEDDisposeImpl () */

static
rc_t CC
_HttpEDMakeImpl ( struct _HttpED ** HttpED )
{
    rc_t RCt;
    struct _HttpED * TheED;

    RCt = 0;
    TheED = NULL;

    if ( HttpED != NULL ) {
        * HttpED = NULL;
    }

    if ( HttpED == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheED = calloc ( 1, sizeof ( struct _HttpED ) );
    if ( TheED == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSLockDepotMake ( 0, & ( TheED -> mutabors ) );
    if ( RCt == 0 ) {

        RCt = _HttpHsMake ( 0, & ( TheED -> http_hs ) );
        if ( RCt == 0 ) {

            RCt = KLockMake ( & ( TheED -> mutabor ) );
            if ( RCt == 0 ) {
                BSTreeInit ( & ( TheED -> tree ) );

                KRefcountInit (
                            & ( TheED -> refcount ),
                            1,
                            _sXFSHttpED_classname,
                            "_HttpEDMake",
                            "HttpED"
                            );

                * HttpED = TheED;
            }
        }
    }

    if ( RCt != 0 ) {
        * HttpED = NULL;
        _HttpEDDisposeImpl ( TheED );
    }

    return RCt;
}   /* _HttpEDMakeImpl () */

static
rc_t CC
_HttpEDDispose ()
{
    struct _HttpED * HttpED;

         /* Sorry, there is no any mutex available, so dummy
          * TODO ... think about global lock ...
          */
    HttpED = _sHttpED;
    _sHttpED = NULL;

    if ( HttpED != NULL ) {
        _HttpEDDisposeImpl ( HttpED );
    }

    return 0;
}   /* _HttpEDDispose () */

static
rc_t CC
_HttpEDMake ()
{
    rc_t RCt;
    struct _HttpED * HttpED;

    RCt = 0;
    HttpED = NULL;

    if ( _sHttpED == NULL ) {

        RCt = _HttpEDMakeImpl ( & HttpED );
        if ( RCt == 0 ) {
                /* Sorry, there is no any mutex available, so dummy
                 * TODO ... think about global lock ...
                 */
            if ( _sHttpED == NULL ) {
                _sHttpED = HttpED;
            }
            else {
                _HttpEDDisposeImpl ( HttpED );
            }
        }
    }

    return RCt;
}   /* _HttpEDMake () */

static
rc_t CC
_HttpEDAddRef ()
{
    rc_t RCt;
    struct _HttpED * HttpED;

    RCt = 0;
    HttpED = _ED ();

    if ( HttpED == NULL ) {
        RCt = _HttpEDMake ( );
        if ( RCt == 0 ) {
        }
    }
    else {
        RCt = KLockAcquire ( HttpED -> mutabor );
        if ( RCt == 0 ) {
            switch ( KRefcountAdd ( & ( HttpED -> refcount ), _sXFSHttpED_classname ) ) {
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
            KLockUnlock ( HttpED -> mutabor );
        }
    }

    return RCt;
}   /* _HttpEDAddRef () */

static
rc_t CC
_HttpEDRelease ()
{
    rc_t RCt;
    struct _HttpED * HttpED;
    bool DisposeED;

    RCt = 0;
    HttpED = _ED ();
    DisposeED = false;

    if ( HttpED != NULL ) {
        RCt = KLockAcquire ( HttpED -> mutabor );
        if ( RCt == 0 ) {
            switch ( KRefcountDrop ( & ( HttpED -> refcount ), _sXFSHttpED_classname ) ) {
                case krefOkay :
                case krefZero :
                            RCt = 0;
                            break;
                case krefWhack :
                            RCt = 0;
                            DisposeED = true;
                            break;
                case krefNegative :
                            RCt = XFS_RC ( rcInvalid );
                            break;
                default :
                            RCt = XFS_RC ( rcUnknown );
                            break;
            }

            KLockUnlock ( HttpED -> mutabor );
        }
        if ( DisposeED ) {
            _HttpEDDispose ();
        };
    }

    return RCt;
}   /* _HttpEDRelease () */

static
int64_t CC
_HttpEntryCmpCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                ? NULL
                : ( ( struct XFSHttpEntry * ) Node ) -> url
                ;
    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _HttpEntryCmpCallback () */

static
rc_t CC
_HttpEDGetNoLock (
                struct _HttpED * self,
                const char * Url,
                const struct XFSHttpEntry ** Entry
)
{
    const struct XFSHttpEntry * RetVal = NULL;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Url == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetVal = ( const struct XFSHttpEntry * ) BSTreeFind (
                                                & ( self -> tree ),
                                                Url,
                                                _HttpEntryCmpCallback
                                                );


    * Entry = RetVal;

    return 0;
}   /* _HttpEDGetNoLock () */

static
rc_t CC
_HttpEDGet ( const char * Url, const struct XFSHttpEntry ** Entry )
{
    rc_t RCt;
    struct _HttpED * ED;

    RCt = 0;
    ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( ED -> mutabor );
    if ( RCt == 0 ) {
        RCt = _HttpEDGetNoLock ( ED, Url, Entry );

        KLockUnlock ( ED -> mutabor );
    }

    return RCt;
}   /* _HttpEDGet () */

static
int64_t CC
_HttpEntryAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
            ( ( struct XFSHttpEntry * ) N1 ) -> url,
            ( ( struct XFSHttpEntry * ) N2 ) -> url
            );
}   /* _HttpEntryAddCallback () */

static
rc_t CC
_HttpEDAddNoLock (
                struct _HttpED * self,
                const char * Url,
                const struct XFSHttpEntry * Entry
)
{
    rc_t RCt;
    const struct XFSHttpEntry * TheEntry;

    RCt = 0;
    TheEntry = NULL;

    if ( self == NULL || Url == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _HttpEDGetNoLock ( self, Url, & TheEntry );
    if ( RCt == 0 ) {
        if ( TheEntry == NULL ) {
            RCt = XFSHttpEntryAddRef ( Entry );
            if ( RCt == 0 ) {
                RCt = BSTreeInsert (
                            & ( self -> tree ),
                            ( struct BSTNode * ) & ( Entry -> node ),
                            _HttpEntryAddCallback
                            );
                if ( RCt != 0 ) {
                    XFSHttpEntryRelease ( Entry );
                }
            }
        }
    }

    return RCt;
}   /* _HttpEDAddNoLock () */

static
rc_t CC
_HttpEDAdd ( const struct XFSHttpEntry * Entry )
{
    rc_t RCt;
    struct _HttpED * ED;

    RCt = 0;
    ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( ED -> mutabor );
    if ( RCt == 0 ) {
        RCt = _HttpEDAddNoLock ( ED, Entry -> url, Entry );

        KLockUnlock ( ED -> mutabor );
    }
    return RCt;
}   /* _HttpEDAdd () */

#ifdef NOT_NEED_YET
static
rc_t CC
_HttpEDDelNoLock ( struct _HttpED * self, const char * Url )
{
    rc_t RCt;
    const struct XFSHttpEntry * Entry;
    bool DelSucc;

    RCt = 0;
    Entry = NULL;
    DelSucc = false;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _HttpEDGetNoLock ( self, Url, & Entry );
    if ( RCt == 0 ) {
        if ( Entry != NULL ) {
            DelSucc = BSTreeUnlink (
                                & ( self -> tree ),
                                ( struct BSTNode * ) & ( Entry -> node )
                                ); 
            if ( DelSucc ) {
                _HttpHsDelete ( self -> http_hs, Entry );
                XFSHttpEntryRelease ( Entry );
            }
        }
    }

    return RCt;
}   /* _HttpEDDelNoLock () */

static
rc_t CC
_HttpEDDel ( const char * Url )
{
    rc_t RCt;
    struct _HttpED * ED;

    RCt = 0;
    ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( ED -> mutabor );
    if ( RCt == 0 ) {
        RCt = _HttpEDDelNoLock ( ED, Url );

        KLockUnlock ( ED -> mutabor );
    }

    return RCt;
}   /* _HttpEDGet () */
#endif /* NOT_NEED_YET */

static
rc_t CC
_HttpEDClearNoLock ( struct _HttpED * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeWhack ( & ( self -> tree ), _HttpEDWhackCallback, NULL );
    BSTreeInit ( & ( self -> tree ) );

    return 0;
}   /* _HttpEDClearNoLock () */

static
rc_t CC
_HttpEDClear ()
{
    rc_t RCt;
    struct _HttpED * ED;

    RCt = 0;
    ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockAcquire ( ED -> mutabor );
    if ( RCt == 0 ) {
        RCt = _HttpEDClearNoLock ( ED );

        KLockUnlock ( ED -> mutabor );
    }

    return RCt;
}   /* _HttpEDClear () */

static 
rc_t CC
_HttpEDLock ( uint32_t HashValue )
{
    struct _HttpED * ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ED -> mutabors == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSLockDepotAcquire ( ED -> mutabors, HashValue );
}   /* _HttpEDLock () */

static 
rc_t CC
_HttpEDUnlock ( uint32_t HashValue )
{
    struct _HttpED * ED = _ED ();

    if ( ED == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ED -> mutabors == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSLockDepotUnlock ( ED -> mutabors, HashValue );
}   /* _HttpEDUnlock () */

static
rc_t CC
_HttpEDGetFileForEntry (
                    const struct XFSHttpEntry * Entry,
                    const struct KFile ** File
)
{
    struct _HttpED * ED = _ED ();

    if ( File != NULL ) {
        * File = NULL;
    }

    if ( ED == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ED -> http_hs == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Entry == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    return _HttpHsGetKFile ( ED -> http_hs, Entry, File );
}   /* _HttpEDHttpHs () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* Misk Methods ...                                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //   MISKA 1: Will concatenate base_url and Path.
((*/
static
rc_t CC
_HttpMakeUrlFromPath (
                    const char * BaseUrl,
                    const char * Path,
                    const char ** Url
)
{
    size_t PathSize, BaseSize, RetSize;
    char * RetUrl;

    PathSize = BaseSize = RetSize = 0;
    RetUrl = NULL;

    if ( Url != NULL ) {
        * Url = NULL;
    }

    if ( BaseUrl == NULL || Path == NULL || Url == NULL ) {
        return XFS_RC ( rcNull );
    }

    PathSize = string_size ( Path );
    BaseSize = string_size ( BaseUrl );

        /*  First we are calculating string size with some extra
         */
    RetSize = PathSize
                + 1 /* slash */
                + BaseSize
                + 1 /* 0 at the end of string */
                ;
        /*  Second we are allocating string
         */
    RetUrl = calloc ( RetSize, sizeof ( char ) );
    if ( RetUrl == NULL ) {
        return XFS_RC ( rcExhausted );
    }

        /*  Third we are copying base_url and removing exess of slashes
         */
    string_copy ( RetUrl, RetSize, BaseUrl, BaseSize );
    if ( RetUrl [ BaseSize - 1 ] == '/' ) {
        BaseSize --;
        RetUrl [ BaseSize ] = 0;
    }

        /*  Fourth, adding trailing slash if it is necessary
         */
    if ( * Path != '/' ) {
        RetUrl [ BaseSize ] = '/';
        BaseSize ++;
    }

        /*  Fith we are copying Path
         */
    string_copy ( RetUrl + BaseSize, RetSize - BaseSize, Path, PathSize );

    * Url = RetUrl;

    return 0;
}   /* _HttpMakeUrlFromPath () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSHttpEntry Make/Dispose Methods ...                             */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static rc_t CC _HttpEntryDispose ( struct XFSHttpEntry * self );

static
rc_t CC
_HttpCreateEntry (
            const char * BaseUrl,
            const char * Path,
            bool IsFolder,
            uint64_t Size,
            KTime_t Time,
            struct XFSHttpEntry ** Entry
)
{
    rc_t RCt;
    struct XFSHttpEntry * RetEntry;
    size_t PathSize;

    RCt = 0;
    RetEntry = NULL;
    PathSize = 0;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( BaseUrl == NULL || Path == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }
    PathSize = string_size ( Path );
    if ( PathSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RetEntry = calloc ( 1, sizeof ( struct XFSHttpEntry ) );
    if ( RetEntry == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RetEntry -> status = kxfsInvalid;

    RCt = VNamelistMake ( & ( RetEntry -> list ), 128 );
    if ( RCt == 0 ) { 
        RCt = _HttpMakeUrlFromPath ( BaseUrl, Path, & ( RetEntry -> url ) );
        if ( RCt == 0 ) {
            RCt = XFS_NameFromPath_ZHR ( Path, & ( RetEntry -> name ), false );
            if ( RCt == 0 ) {
                KRefcountInit (
                            & ( RetEntry -> refcount ),
                            1,
                            _sXFSHttpEntry_classname,
                            "_HttpCreateEntry",
                            "HttpEntry"
                            );
                RetEntry -> is_folder = IsFolder;
                RetEntry -> size = IsFolder ? 0 : Size;
                RetEntry -> time = Time;
                RetEntry -> url_hash = string_hash (
                                            RetEntry -> url,
                                            string_size ( RetEntry -> url )
                                            );
                    /* Last thing to do, it is Ready, not Complete !!! */
                RetEntry -> status = kxfsReady;

                * Entry = RetEntry;
            }
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL;

        if ( RetEntry != NULL ) {
            _HttpEntryDispose ( RetEntry );
        }
    }

    return RCt;
}   /* _HttpCreateEntry () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSHttp Methods ...                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSHttpMake ( const char * BaseUrl, const struct XFSHttp ** TheHttp )
{
    rc_t RCt;
    struct XFSHttp * RetHttp;
    struct XFSHttpEntry * Entry;
    size_t UrlSize;

    RCt = 0;
    RetHttp = NULL;

    if ( TheHttp != NULL ) {
        * TheHttp = NULL;
    }

    if ( TheHttp == NULL || BaseUrl == NULL ) {
        return XFS_RC ( rcNull );
    }

        /* First we should be sure that HttpED is good and trusty
         */
    RCt = _HttpEDAddRef ();
    if ( RCt != 0 ) {
        return RCt;
    }

    RetHttp = calloc ( 1, sizeof ( struct XFSHttp ) );
    if ( RetHttp == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    KRefcountInit (
                & ( RetHttp -> refcount ),
                1,
                _sXFSHttp_classname,
                "XFSHttpMake",
                "Http"
                );

    RCt = XFS_StrDup ( BaseUrl, & ( RetHttp -> base_url ) );
    if ( RCt == 0 ) {
            /* Here we are stripping last 'slash' if it is
             */
        UrlSize = string_size ( RetHttp -> base_url );
        while ( 0 < UrlSize ) {
            if ( RetHttp -> base_url [ UrlSize - 1 ] == '/' ) {
                ( ( char * ) RetHttp -> base_url ) [ UrlSize - 1 ] = 0;
                UrlSize --;
            }
            else {
                break;
            }
        }

        RCt = _HttpCreateEntry (
                            RetHttp -> base_url,
                            "/",
                            true,
                            0,
                            0,
                            & Entry
                            );
        if ( RCt == 0 ) {
            RCt = _HttpEDAdd ( Entry );

            if ( RCt != 0 ) {
                _HttpEntryDispose ( Entry );

                    /*  Really it is not error, and something else
                     *  already created that entry
                     */
                RCt = 0;
            }
            * TheHttp = RetHttp;
        }
    }

    if ( RCt != 0 ) {
        * TheHttp = NULL;

        XFSHttpDispose ( RetHttp );
    }

    return RCt;
}   /* XFSHttpMake () */

LIB_EXPORT
rc_t CC
XFSHttpDispose ( const struct XFSHttp * self )
{
    struct XFSHttp * Http = ( struct XFSHttp * ) self;

    if ( Http != NULL ) {
        if ( Http -> base_url != NULL ) {
            free ( ( char * ) Http -> base_url );
            Http -> base_url = NULL;
        }

        KRefcountWhack ( & ( Http -> refcount ), _sXFSHttp_classname );

        free ( Http );

        _HttpEDRelease ();
    }

    return 0;
}   /* XFSHttpDispose () */

LIB_EXPORT
rc_t CC
XFSHttpAddRef ( const struct XFSHttp * self )
{
    rc_t RCt;
    struct XFSHttp * Http;

    RCt = 0;
    Http = ( struct XFSHttp * ) self;

    if ( Http == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( Http -> refcount ), _sXFSHttp_classname ) ) {
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
}   /* XFSHttpAddRef () */

LIB_EXPORT
rc_t CC
XFSHttpRelease ( const struct XFSHttp * self )
{
    rc_t RCt;
    struct XFSHttp * Http;

    RCt = 0;
    Http = ( struct XFSHttp * ) self;

    if ( Http == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( Http -> refcount ), _sXFSHttp_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = XFSHttpDispose ( ( struct XFSHttp * ) Http );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSHttpRelease () */

LIB_EXPORT
const char * CC 
XFSHttpBaseUrl ( const struct XFSHttp * self )
{
    if ( self != NULL ) {
        return self -> base_url;
    }
    return NULL;
}   /* XFSHttpBaseUrl () */

LIB_EXPORT
bool CC
XFSHttpHasEntry ( const struct XFSHttp * self, const char * Path )
{
    return XFSHttpGetEntry ( self, Path ) != NULL;
}   /* XFSHttpHasEntry () */

LIB_EXPORT
const struct XFSHttpEntry * CC
XFSHttpGetEntry ( const struct XFSHttp * self, const char * EntryPath )
{
    rc_t RCt;
    const char * Url;
    const struct XFSHttpEntry * RetEntry;

    RCt = 0;
    Url = NULL;
    RetEntry = NULL;

    if ( self != NULL && EntryPath != NULL ) {
        RCt = _HttpMakeUrlFromPath ( self -> base_url, EntryPath, & Url );

        if ( RCt == 0 ) {
            RCt = _HttpEDGet ( Url, & RetEntry );
            if ( RCt != 0 ) {
                RetEntry = NULL;
            }
            free ( ( char * ) Url );
        }
    }

    return RetEntry;
}   /* XFSHttpGetEntry () */

static rc_t CC _HttpLoadDirEntry ( const struct XFSHttpEntry * self );

static
rc_t CC
_MakeValidPath (
            const char * Path,
            const struct XFSPath ** xPath,
            uint32_t * xPathQty
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_4096 ];
    const char * P;

    RCt = 0;
    * BF = 0;
    P = NULL;

    if ( xPath != NULL ) {
        * xPath = NULL;
    }

    if ( xPathQty != NULL ) {
        * xPathQty = 0;
    }

    if ( xPath == NULL || Path == NULL || xPathQty == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( * Path == '/' ) {
        P = Path;
    }
    else {
        * BF = '/';
        string_copy_measure ( BF + 1, sizeof ( BF ) - 1, Path );

        P = BF;
    }

    RCt = XFSPathMake ( xPath, true, P );
    if ( RCt == 0 ) {
        * xPathQty = XFSPathPartCount ( * xPath );
    }

    return RCt;
}   /* _MakeValidPath () */

static
rc_t CC
_HttpOrCreateEntry (
            const struct XFSHttp * self,
            const char * Path,
            const struct XFSHttpEntry ** Entry
)
{
    rc_t RCt;
    const struct XFSPath * xPath;
    char BF [ XFS_SIZE_1024 ];
    uint32_t xQty;
    const struct XFSHttpEntry * Parent;
    const struct XFSPath * xParent;

    RCt = 0;
    xPath = NULL;
    * BF = 0;
    xQty = 0;
    Parent = NULL;
    xParent = NULL;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Path == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _MakeValidPath ( Path, & xPath, & xQty );
    if ( RCt == 0 ) { 
        if ( xQty < 2 ) {
            /* Not an error ???
                RCt = XFS_RC ( rcInvalid );
            */
        }
        else {
                /* Path to parent directory */
            RCt = XFSPathParent ( xPath, & xParent );
            if ( RCt == 0 ) {
                    /* Trying to get ready one */
                Parent = XFSHttpGetEntry ( self, XFSPathGet ( xParent ) );
                if ( Parent == NULL ) {
                        /* Creating new one */
                    RCt = _HttpCreateEntry (
                                    self -> base_url,
                                    XFSPathGet ( xParent ),
                                    true,
                                    0,
                                    0,
                                    ( struct XFSHttpEntry ** ) & Parent
                                    );
                    if ( RCt == 0 ) {
                        RCt = _HttpEDAdd ( Parent );
                    }
                }

                if ( RCt == 0 ) {
                        /* Checking and loading parent entry */
                    if ( Parent -> status == kxfsReady ) {
                        RCt = _HttpLoadDirEntry ( Parent );
                    }

                        /* Check if we obtained necessary node */
                    if ( RCt == 0 ) {
                        Parent = XFSHttpGetEntry ( self, Path );
                        if ( Parent == NULL ) {
                            RCt = XFS_RC ( rcNull );
                        }
                        else {
                            * Entry = Parent;
                        }

                    }
                }

                XFSPathRelease ( xParent );
            }
        }

        XFSPathRelease ( xPath );
    }

    return RCt;
}   /* _HttpOrCreateEntry () */

LIB_EXPORT
rc_t CC
XFSHttpGetOrCreateEntry (
            const struct XFSHttp * self,
            const char * Path,
            const struct XFSHttpEntry ** Entry
)
{
    rc_t RCt;
    const struct XFSHttpEntry * TheEntry;

    RCt = 0;
    TheEntry = NULL;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Entry == NULL || Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheEntry = XFSHttpGetEntry ( self, Path );
    if ( TheEntry != NULL ) {
        * Entry = TheEntry;
    }
    else {
        /*)) we should find an parent directory and load it.
          || after that we should search throught that directory
          ((*/
        RCt = _HttpOrCreateEntry ( self, Path , & TheEntry );
        if ( RCt == 0 ) {
            * Entry = TheEntry;
        }
    }

    return RCt;
}   /* XFSHttpGetOrCreateEntry () */

static
rc_t CC
_HttpEntryDispose ( struct XFSHttpEntry * self )
{
    if ( self != NULL ) {
        self -> status = kxfsInvalid;

        KRefcountWhack (
                    & ( self -> refcount ),
                    _sXFSHttpEntry_classname
                    ); 
        if ( self -> name != NULL ) {
            free ( ( char * ) self -> name ) ;
            self -> name = NULL;
        }
        if ( self -> url != NULL ) {
            free ( ( char * ) self -> url ) ;
            self -> url = NULL;
        }
        self -> url_hash = 0;
        self -> is_folder = false;

        if ( self -> list != NULL ) {
            VNamelistRelease ( self -> list );
            self -> list = NULL;
        }

        self -> size = 0;
        self -> time = 0;

        free ( self );
    }

    return 0;
}   /* _HttpEntryDispose () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSHttpEntry Methods ...                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSHttpEntryAddRef ( const struct XFSHttpEntry * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountAdd ( & ( self -> refcount ), _sXFSHttpEntry_classname ) ) {
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
}   /* XFSHttpEntryAddRef () */

LIB_EXPORT
rc_t CC
XFSHttpEntryRelease ( const struct XFSHttpEntry * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    switch ( KRefcountDrop ( & ( self -> refcount ), _sXFSHttpEntry_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _HttpEntryDispose ( ( struct XFSHttpEntry * ) self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSHttpEntryRelease () */

LIB_EXPORT
bool CC
XFSHttpEntryIsFolder ( const struct XFSHttpEntry * self )
{
    return self == NULL ? false : ( self -> is_folder );
}   /* XFSHttpEntryIsFolder () */

LIB_EXPORT
const char * CC
XFSHttpEntryName ( const struct XFSHttpEntry * self )
{
    return self == NULL ? NULL : ( self -> name );
}   /* XFSHttpEntryName () */

LIB_EXPORT
const char * CC
XFSHttpEntryUrl ( const struct XFSHttpEntry * self )
{
    return self == NULL ? NULL : ( self -> url );
}   /* XFSHttpEntryUrl () */

LIB_EXPORT
uint32_t CC
XFSHttpEntryUrlHash ( const struct XFSHttpEntry * self )
{
    return self == NULL ? 0 : ( self -> url_hash );
}   /* XFSHttpEntryUrlHash () */

static
rc_t CC
_HttpCheckLoadList ( const struct XFSHttpEntry * self )
{
    rc_t RCt;
    struct XFSHttpEntry * Entry;

    RCt = 0;
    Entry = ( struct XFSHttpEntry * ) self;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> is_folder ) {
        if ( self -> status == kxfsReady ) {
            RCt = _HttpLoadDirEntry ( self );

            Entry -> status = ( RCt == 0 ? kxfsComplete : kxfsBroken );
        }
    }

    if ( self -> status != kxfsComplete
        && self -> status != kxfsReady ) {
        RCt = XFS_RC ( rcInvalid );
    }

    return RCt;
}   /* _HttpCheckLoadList () */

LIB_EXPORT
rc_t CC
XFSHttpEntryList (
                const struct XFSHttpEntry * self,
                struct KNamelist ** List
)
{
    rc_t RCt;

    RCt = 0;

    if ( List != NULL ) {
        * List = NULL;
    }

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _HttpCheckLoadList ( self );
    if ( RCt == 0 ) {
            /* TODO : think where to keep empty list ?
             */
        RCt = VNamelistToNamelist ( self -> list, List );
    }

    return RCt;
}   /* XFSHttpEntryList () */

LIB_EXPORT
rc_t CC
XFSHttpEntrySize ( const struct XFSHttpEntry * self, uint64_t * Size )
{
    if ( Size != NULL ) {
        * Size = 0;
    }

    if ( self == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Size = self -> size;

    return 0;
}   /* XFSHttpEntrySize () */

LIB_EXPORT
rc_t CC
XFSHttpEntryTime ( const struct XFSHttpEntry * self, KTime_t * Time )
{
    if ( Time != NULL ) {
        * Time = 0;
    }

    if ( self == NULL || Time == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Time = self -> time;

    return 0;
}   /* XFSHttpEntryTime () */

LIB_EXPORT
bool CC
XFSHttpEntryGood ( const struct XFSHttpEntry * self )
{
    if ( self != NULL ) {
        return self -> status == kxfsReady
                    || self -> status == kxfsComplete;
    }
    return false;
}   /* XFSHttpEntryGood () */

LIB_EXPORT
bool CC
XFSHttpEntryHas (
                const struct XFSHttpEntry * self,
                const char * ChildName
)
{
    rc_t RCt;
    uint32_t Cnt, llp;
    const char * Name;
    size_t CSz, NSz;

    RCt = 0;
    Cnt = llp = 0;
    Name = NULL;
    CSz = NSz = 0;

    if ( self != NULL && ChildName != NULL ) {
        CSz = string_size ( ChildName );

        if ( self -> list != NULL ) {
            RCt = VNameListCount ( self -> list, & Cnt );
            if ( RCt == 0 ) {
                for ( llp = 0; llp < Cnt; llp ++ ) {
                    RCt = VNameListGet ( self -> list, llp, & Name );
                    if ( RCt == 0 ) {
                        NSz = string_size ( Name );
                        if ( NSz == CSz ) {
                            if ( string_cmp ( ChildName, CSz, Name, CSz, CSz ) == 0 ) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}   /* XFSHttpEntryHas () */

LIB_EXPORT
rc_t CC
XFSHttpEntryGet (
                const struct XFSHttpEntry * self,
                const char * ChildName,
                const struct XFSHttpEntry ** Child
)
{
    rc_t RCt;
    const char * ChildPath;
    const struct XFSHttpEntry * RetChild;

    RCt = 0;
    ChildPath = NULL;

    if ( Child != NULL ) {
        * Child = NULL;
    }

    if ( self == NULL || ChildName == NULL || Child == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _HttpMakeUrlFromPath ( self -> url, ChildName, & ChildPath );
    if ( RCt == 0 ) {
        RCt = _HttpEDGet ( ChildPath, & RetChild );
        if ( RCt == 0 ) {
            * Child = RetChild;
        }

        free ( ( char * ) ChildPath );
        ChildPath = NULL;
    }

    return RCt;
}   /* XFSHttpEntryGet () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
rc_t CC
XFSHttpRehash ()
{
    return _HttpEDClear () ;
}   /* XFSHttpRehash () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSHttpReader Methods ...                                         */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSHttpReaderMake (
                const struct XFSHttpEntry * Entry,
                const struct XFSHttpReader ** Reader
)
{
    rc_t RCt;
    struct XFSHttpReader * RetReader;

    RCt = 0;
    RetReader = NULL;

    if ( Reader != NULL ) {
        * Reader = NULL;
    }

    if ( Entry == NULL || Reader == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetReader = calloc ( 1, sizeof ( struct XFSHttpReader ) );
    if ( RetReader == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSHttpEntryAddRef ( Entry );
    if ( RCt == 0 ) {
        RetReader -> entry = Entry;

        KRefcountInit (
                    & ( RetReader -> refcount ),
                    1,
                    _sXFSHttpReader_classname,
                    "XFSHttpReaderMake",
                    "HttpReader"
                    );

        * Reader = RetReader;
    }
    else {
        free ( RetReader );
    }

    return RCt;
}   /* XFSHttpReaderMake () */

LIB_EXPORT
rc_t CC
XFSHttpReaderAddRef ( const struct XFSHttpReader * self )
{
    rc_t RCt;
    struct XFSHttpReader * Reader;
    uint32_t hash;

    RCt = 0;
    Reader = ( struct XFSHttpReader * ) self;
    hash = 0;

    if ( Reader == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Reader -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    hash = Reader -> entry -> url_hash;
    RCt = _HttpEDLock ( hash );
    if ( RCt == 0 ) {
        switch ( KRefcountAdd ( & ( Reader -> refcount ), _sXFSHttpReader_classname ) ) {
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

        RCt = _HttpEDUnlock ( hash );
    }

    return RCt;
}   /* XFSHttpReaderAddRef () */

static
rc_t CC
_HttpReaderDispose ( const struct XFSHttpReader * self )
{
    struct XFSHttpReader * Reader;

    Reader = ( struct XFSHttpReader * ) self;

    if ( Reader == NULL ) {
        return 0;
    }

        /* TODO We are not removing data from queue, it will die itself
         */

    KRefcountWhack (
                & ( Reader -> refcount ),
                _sXFSHttpReader_classname
                );

    if ( Reader -> entry != NULL ) {
        _HttpHsDelete ( _ED () -> http_hs, Reader -> entry );

        XFSHttpEntryRelease ( Reader -> entry );

        Reader -> entry = NULL;
    }

    free ( Reader );

    return 0;
}   /* _HttpReaderDispose () */

LIB_EXPORT
rc_t CC
XFSHttpReaderRelease ( const struct XFSHttpReader * self )
{
    rc_t RCt;
    struct XFSHttpReader * Reader;
    uint32_t hash;

    RCt = 0;
    Reader = ( struct XFSHttpReader * ) self;
    hash = 0;

    if ( Reader == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Reader -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    hash = Reader -> entry -> url_hash;
    RCt = _HttpEDLock ( hash );
    if ( RCt == 0 ) {
        switch ( KRefcountDrop ( & ( Reader -> refcount ), _sXFSHttpReader_classname ) ) {
            case krefOkay :
            case krefZero :
                        RCt = 0;
                        break;
            case krefWhack :
                        RCt = 0;
                        _HttpReaderDispose ( Reader );
                        break;
            case krefNegative :
                        RCt = XFS_RC ( rcInvalid );
                        break;
            default :
                        RCt = XFS_RC ( rcUnknown );
                        break;
        }

        RCt = _HttpEDUnlock ( hash );
    }

    return RCt;
}   /* XFSHttpReaderRelease () */

static
rc_t CC
_CheckResoveOpen (
            const struct XFSHttpEntry * self,
            const struct KFile ** File
)
{
    rc_t RCt;
    struct XFSHttpEntry * Entry;
    const struct KFile * TheFile;
    uint64_t Size;

    RCt = 0;
    Entry = ( struct XFSHttpEntry * ) self;
    TheFile = NULL;
    Size = 0;

    if ( File != NULL ) {
        * File = NULL;
    }

    if ( File == NULL || self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> status == kxfsInvalid
        || self -> status == kxfsBroken ) {
        return XFS_RC ( rcInvalid );
    }


    RCt = _HttpEDGetFileForEntry ( self, & TheFile );
    if ( RCt != 0 ) {
        ( ( struct XFSHttpEntry * ) self ) -> status = kxfsBroken;
    }
    else {
        if ( self -> status == kxfsReady ) {
            if ( ! self -> is_folder ) {
                    /* Here we are reading real file size */
                RCt = KFileSize ( TheFile, & Size );
                if ( RCt == 0 ) {
                    Entry -> size = Size;
                }
                else {
                    RCt = 0; // TODO: not sure about it
                }
            }
        }

        ( ( struct XFSHttpEntry * ) self ) -> status = kxfsComplete;
        * File = TheFile;
    }

    return RCt;
}   /* _CheckResoveOpen () */


LIB_EXPORT
rc_t CC
XFSHttpReaderRead (
                const struct XFSHttpReader * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
    rc_t RCt;
    const struct KFile * File;

    RCt = 0;
    File = NULL;

    if ( self == NULL || Buffer == NULL || NumRead == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( BufferSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _HttpEDLock ( self -> entry -> url_hash );
    if ( RCt == 0 ) {

        RCt = _CheckResoveOpen ( self -> entry, &File );
        if ( RCt == 0 ) {
            RCt = KFileRead (
                            File,
                            Offset,
                            Buffer,
                            BufferSize,
                            NumRead
                            );
            KFileRelease ( File );
        }
        _HttpEDUnlock ( self -> entry -> url_hash );
    }

    return RCt;
}   /* XFSHttpReaderRead () */

static
rc_t CC
_HttpMakeBuffer ( uint64_t Size, void ** Buffer )
{
    char * BF;

    BF = NULL;

    if ( Buffer == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Buffer = NULL;

    if ( Size == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    BF = calloc ( ( size_t ) Size, sizeof ( char ) );
    if ( BF == NULL ) {
        return XFS_RC ( rcExhausted );
    }
    BF [ Size - 1 ] = 0;

    * Buffer = ( void * ) BF;

    return 0;
}   /* _HttpMakeBuffer () */

/*))
 // We do suppose that size of readed data will not exceed 128K
((*/
#define XFS_MAX_DIR_BUFFER 131072

static
rc_t CC
_xStreamReadAll ( const char * Url, void ** Buffer, size_t * NumRead )
{
    rc_t RCt;
    const struct XFSHttpStream * Stream;
    char * BF;
    uint64_t SZ, Off;
    size_t NumR;

    RCt = 0;
    Stream = NULL;
    BF = NULL;
    SZ = XFS_MAX_DIR_BUFFER;
    Off = 0;
    NumR = 0;

    if ( Buffer != NULL ) {
        * Buffer = NULL;
    }

    if ( NumRead != NULL ) {
        * NumRead = 0;
    }

    if ( Url == NULL || Buffer == NULL || NumRead == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFS_HttpStreamMake_ZHR ( Url, & Stream );
    if ( RCt == 0 ) {
        RCt = _HttpMakeBuffer ( SZ, ( void ** ) & BF );
        if ( RCt == 0 ) {
            while ( ! XFS_HttpStreamCompleted_ZHR ( Stream ) ) {
                RCt = XFS_HttpStreamRead_ZHR (
                                            Stream,
                                            BF + Off,
                                            ( size_t ) ( SZ - Off ),
                                            & NumR
                                            );
                if ( RCt != 0 ) {
                    break;
                }

                if ( NumR != 0 && SZ <= ( Off + NumR ) ) {
                    RCt = XFS_RC ( rcInvalid );

                    break;
                }

                Off += NumR;
                NumR = 0;
            }

            if ( RCt == 0 ) {
                * Buffer = BF;
                * NumRead = ( size_t ) Off;
            }
        }

        XFS_HttpStreamDispose_ZHR ( Stream );
    }

    if ( RCt != 0 ) {
        * Buffer = NULL;
        * NumRead = 0;

        if ( BF != NULL ) {
            free ( BF );
        }
    }

    return RCt;
}   /* _xStreamReadAll () */

struct _BufferLR {
    const char * buffer;
    size_t length;

    size_t line_start;
    size_t line_end;
    size_t line_no;
};

static
rc_t CC
_BufferLRInit (
            struct _BufferLR * self,
            const char * Buffer,
            size_t Length
)
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    self -> buffer = Buffer;
    self -> length = Length;
    self -> line_start = 0;
    self -> line_end = 0;
    self -> line_no = 0;

    return 0;
}   /* _BufferLRInit () */

static
rc_t CC
_BufferLRWhack ( struct _BufferLR * self )
{
    if ( self != NULL ) {
        self -> buffer = NULL;
        self -> length = 0;
        self -> line_start = 0;
        self -> line_end = 0;
        self -> line_no = 0;
    }
    return 0;
}   /* _BufferLRWhack () */

static
bool CC
_BufferLRNext (
            struct _BufferLR * self,
            const char ** NextLine,
            size_t * Length
)
{
    const char * start, * end, * kirdyk;

    start = end = kirdyk = NULL;

    if ( NextLine != NULL ) {
        * NextLine = NULL;
    }

    if ( Length != NULL ) {
        * Length = 0;
    }

    if ( self == NULL || NextLine == NULL || Length == NULL ) {
        return false;
    }

        /* Line number, just for case */
    self -> line_no ++;

    kirdyk = self -> buffer + self -> length;
    start = self -> buffer + self -> line_start;
    end = self -> buffer + self -> line_end;

        /* First, we are looking for start of new line */
    start = NULL;
    while ( end < kirdyk ) {
        if ( isalnum ( * end ) || * end == '<' ) {
            start = end;
            break;
        }
        end ++;
    }
    if ( start == NULL ) {
        return false;
    }

        /* Second we are looking for end of new line */
    end ++;
    while ( end < kirdyk ) {
        if ( * end == '\n' ) {
            break;
        }

        end ++;
    }

    self -> line_start = start - self -> buffer;
    self -> line_end = end - self -> buffer;

    * NextLine = start;
    * Length = end - start;

    return true;
}   /* _BufferLRNext () */

static
bool CC
_GetAnchor (
        const char * Start,
        const char * End,
        const char ** AncStart,
        const char ** AncEnd
)
{
    const char * AncS, * AncE, * AncP;

    AncS = AncE = AncP = NULL;

    /* We are looking for "<a href ... </a>" tag */

    AncP = Start;
    while ( AncP < End ) {
        if ( * AncP == '<' ) {
            if ( 2 <= ( End - AncP ) ) {
                if ( * ( AncP + 1 ) == 'a' && * ( AncP + 2 ) == ' ' ) {
                    AncS = AncP;
                    AncP += 8;

                    break;
                }
            }
        }

        AncP ++;
    }

    if ( AncS == NULL ) {
        return false;
    }

    while ( AncP < End ) {
        if ( * AncP == '<' ) {
            if ( 3 <= ( End - AncP ) ) {
                if ( * ( AncP + 1 ) == '/' && * ( AncP + 2 ) == 'a' && * ( AncP + 3 ) == '>' ) {
                    AncE = AncP + 4;

                    break;
                }
            }
        }

        AncP ++;
    }

    if ( AncE == NULL ) {
        return false;
    }

    * AncStart = AncS;
    * AncEnd = AncE;

    return true;
}   /* _GetAnchor () */

static
bool CC
_GetAnchorName (
            const char * Start,
            const char * End,
            char ** Name,
            bool * IsDir
)
{
    const char * nS, * nE, * nP;
    bool Dir;

    nS = nE = nP = NULL;
    Dir = false;

    * Name = NULL;
    * IsDir = false;

    nP = Start;
    while ( nP < End ) {
        if ( * nP == '"' ) {
            if ( 1 <= ( End - nP ) ) {
                nP ++;
                nS = nP;
            }
            break;
        }

        nP ++;
    }
    if ( nS == NULL ) {
        return false;
    }

    while ( nP < End ) {
        if ( * nP == '"' ) {
            nE = nP;

            break;
        }

        nP ++;
    }
    if ( nE == NULL ) {
        return false;
    }

    if ( * ( nE - 1 ) == '/' ) {
        Dir = true;
        nE --;
    }

    if ( nE == nS ) {
        return false;
    }

    * Name = string_dup ( nS, nE - nS );
    * IsDir = Dir;

    return * Name != NULL;
}   /* _GetAnchorName () */

static
rc_t CC
_GetNameFrom (
            const char * Start,
            const char * End,
            char ** Name,
            bool * IsDir,
            const char ** Next
)
{
    char * TheName;
    const char * aS, * aE;

    TheName = NULL;
    aS = aE = NULL;

    if ( Name != NULL ) {
        * Name = NULL;
    }

    if ( Next != NULL ) {
        * Next = NULL;
    }

    if ( IsDir != NULL ) {
        * IsDir = false;
    }

    if ( Start == NULL || End == NULL || Name == NULL || Next == NULL || IsDir == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( ! _GetAnchor ( Start, End, & aS, & aE ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! _GetAnchorName ( aS, aE, & TheName, IsDir ) ) {
        return XFS_RC ( rcInvalid );
    }

    * Name = TheName;
    * Next = aE + 1;

    return 0;
}   /* _GetNameFrom () */

static
rc_t CC
_GetTimeFrom (
            const char * Start,
            const char * End,
            int64_t * Time,
            const char ** Next
)
{
    static const char * MNT [ 12 ] = {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                        };
    const char * Pos;
    int Day, Year, Hour, Minute, Ret;
    char Month [ 8 ];
    struct tm Tm;

    Pos = NULL;
    Day = Year = Hour = Minute = Ret = 0;
    * Month = 0;
    memset ( & Tm, 0, sizeof ( struct tm ) );

    if ( Time != NULL ) {
        * Time = 0;
    }
    if ( Next != NULL ) {
        * Next = NULL;
    }

    if ( Start == NULL || End == NULL || Time == NULL || Next == NULL ) {
        return XFS_RC ( rcNull );
    }

        /* We are parsing format "dd-mmm-YYYY HH:MM"
         */
    Start = XFS_SkipSpaces_ZHR ( Start, End );
    if ( Start == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Pos = Start + 17;
    Pos = XFS_SkipSpaces_ZHR ( Pos, End );

    Ret = sscanf (
            Start,
            "%2d-%3s-%4d %2d:%2d",
            & Day,
            Month,
            & Year,
            & Hour,
            & Minute
            ); 
    if ( Ret != 5 ) {
        return XFS_RC ( rcInvalid );
    }

    for ( Ret = 0; Ret < 12; Ret ++ ) {
        if ( strncmp ( MNT [ Ret ], Month, 3 ) == 0 ) {
            break;
        }
    }

    if ( Ret == 12 ) {
        return XFS_RC ( rcInvalid );
    }

    Tm . tm_min = Minute;
    Tm . tm_hour = Hour;
    Tm . tm_year = Year - 1900;
    Tm . tm_mon = Ret;
    Tm . tm_mday = Day;

    * Time = mktime ( & Tm );
    * Next = Pos;

    return 0;
}   /* _GetTimeFrom () */

static
rc_t CC
_GetSizeFrom (
            const char * Start,
            const char * End,
            uint64_t * Size,
            const char ** Next
)
{
    const char * Pos;
    uint64_t TheSize;
    float PP;
    int Ret;

    Pos = NULL;
    TheSize = 0;
    PP = 0;
    Ret = 0;

    if ( Size != NULL ) {
        * Size = 0;
    }

    if ( Next != NULL ) {
        * Next = NULL;
    }

    if ( Start == NULL || End == NULL || Size == NULL || Next == NULL ) {
        return XFS_RC ( rcNull );
    }

    Start = XFS_SkipSpaces_ZHR ( Start, End );
    if ( Start == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Pos = XFS_SkipLetters_ZHR ( Start, End );
    if ( Pos == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Ret = sscanf ( Start, "%f", & PP );
    if ( Ret != 1 ) {
        return XFS_RC ( rcInvalid );
    }

    switch ( * ( Pos - 1 ) ) {
        case 'K' : TheSize = ( uint64_t ) ( PP * 1024.0 ); break;
        case 'M' : TheSize = ( uint64_t ) ( PP * 1048576.0 ); break;
        case 'G' : TheSize = ( uint64_t ) ( PP * 1073741824.0 ); break;
        default  : TheSize = ( uint64_t ) ( PP * 1.0 ); break;
    }

    * Size = TheSize;
    * Next = Pos;

    return 0;
}   /* _GetSizeFrom () */

/*|
|*  Very simple format of line. For file :
 *|
|*      name     dd-MMM-YYYY HH:MM   size
 *|
|*  For directory :
 *|
|*      name/    dd-MMM-YYYY HH:MM     -
 *|
|*  Bad thing : ther is no method to convert string to time
 *|
|*/

static
rc_t CC
_HttpParseLine (
            const char * Line,
            size_t Length,
            char ** Name,
            uint64_t * Size,
            int64_t * Time,
            bool * IsDir
)
{
    rc_t RCt;
    const char * start, * end, * pos;
    char * TheName;
    bool TheIsDir;


    RCt = 0;
    start = end = pos = NULL;
    TheName = NULL;
    TheIsDir = false;

    if ( Name != NULL ) {
        * Name = NULL;
    }

    if ( Size != NULL ) {
        * Size = 0;
    }

    if ( Time != NULL ) {
        * Time = 0;
    }

    if ( IsDir != NULL ) {
        * IsDir = false;
    }

    if ( Line == NULL || Length == 0 || Name == NULL || Size == NULL || Time == NULL || IsDir == NULL ) {
        return XFS_RC ( rcNull );
    }

        /* first we are extracting Name of entry */
    start = Line;
    end = start + Length;
    RCt = _GetNameFrom ( start, end, & TheName, & TheIsDir, & start );
    if ( RCt == 0 ) {

        RCt = _GetTimeFrom ( start, end, Time, & start );
        if ( RCt == 0 ) {

            if ( ! TheIsDir ) {
                RCt = _GetSizeFrom ( start, end, Size, & start );
            }

        }
    }

    if ( RCt == 0 ) {
        * Name = TheName;
        * IsDir = TheIsDir;
    }
    else {
        free ( TheName );
    }

    return RCt;
}   /* _HttpParseLine () */

static
rc_t CC
_HttpParseEntry (
                const struct XFSHttpEntry * self,
                const char * Line,
                size_t Length
)
{
    rc_t RCt;
    struct XFSHttpEntry * NewEntry;
    char * Name;
    uint64_t Size;
    int64_t Time;
    bool IsDir;

    RCt = 0;
    NewEntry = NULL;
    Name = NULL;
    Size = 0;
    Time = 0;
    IsDir = false;

    if ( self == NULL || Line == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Length == 0 ) {
        return XFS_RC ( rcInvalid );
    } 

    RCt = _HttpParseLine (
                        Line,
                        Length,
                        & Name,
                        & Size,
                        & Time,
                        & IsDir
                        );
    if ( RCt == 0 ) {
            /*) First we should add name to list of items
             (*/
        RCt = VNamelistAppend ( self -> list, Name );
        if ( RCt == 0 ) {
                /*) Second we should add new entry to ED
                 (*/
            RCt = _HttpCreateEntry (
                                self -> url,
                                Name,
                                IsDir,
                                Size,
                                Time,
                                & NewEntry
                                );
            if ( RCt == 0 ) {
                RCt = _HttpEDAdd ( NewEntry );
                if ( RCt != 0 ) {
                    _HttpEntryDispose ( NewEntry );
                }
            }
        }

        free ( Name );
    }

    return 0;
}   /* _HttpParseEntry () */

static
rc_t CC
_HttpParseFolder (
            const struct XFSHttpEntry * self,
            char * Buffer,
            size_t BufferSize
)
{
    rc_t RCt;
    struct _BufferLR LineReader;
    const char * Line;
    size_t LineLength;

    RCt = 0;
    Line = NULL;
    LineLength = 0;

    RCt = _BufferLRInit ( & LineReader, Buffer, BufferSize );
    if ( RCt == 0 ) {
        while ( _BufferLRNext ( & LineReader, & Line, & LineLength ) ) {
            if ( Line != NULL && LineLength != 0 ) {
                RCt = _HttpParseEntry ( self, Line, LineLength );
                if ( RCt != 0 ) {
                        /*) TODO: It is OK, we will report on that only
                         (*/
                    RCt = 0;
                }
            }
        }
        _BufferLRWhack ( & LineReader );
    }

    return RCt;
}   /* _HttpParseFolder () */

rc_t CC
_HttpLoadDirEntry ( const struct XFSHttpEntry * self )
{
    rc_t RCt;
    char * Buffer;
    size_t NumRead;

    RCt = 0;
    Buffer = NULL;
    NumRead = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _xStreamReadAll (
                        self -> url,
                        ( void ** ) & Buffer,
                        & NumRead
                        );
    if ( RCt == 0 ) {
        if ( Buffer != NULL ) {
            RCt = _HttpParseFolder ( self, Buffer, NumRead );
            if ( RCt != 0 ) {
                    /* I am not sure, but I will do it */
                RCt = 0;

                ( ( struct XFSHttpEntry * ) self ) -> status
                                                        = kxfsBroken;
            }
            else {
                ( ( struct XFSHttpEntry * ) self ) -> status
                                                        = kxfsComplete;
            }

            free ( Buffer );
        }
    }

    return RCt;
}   /* _HttpLoadDirEntry () */
