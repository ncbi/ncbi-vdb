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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/text.h>

#include <kfs/directory.h>

#include <va_copy.h>

#include "schwarzschraube.h"
#include "pfad.h"

#include <sysalloc.h>

#include <string.h>
#include <stdio.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static char * _sPfadEmptyName = "";

#define _STRC_INC_VAL_ 16

struct _StrC {
    char ** e;

    size_t q;
    size_t c;
};

static const char * CC _StrCGet (
                    struct _StrC * self,
                    size_t Idx
                    );
static size_t CC _StrCCount (
                    struct _StrC * self
                    );

static
void CC
_StrCClear ( struct _StrC * self )
{
    size_t Idx = 0;

    if ( self == NULL ) {
        return;
    }

    if ( self -> e != NULL ) {
        if ( self -> q != 0 ) {
            for ( Idx = 0; Idx < self -> q; Idx ++ ) {
                if ( self -> e [ Idx ] != NULL ) {
                    free ( self -> e [ Idx ] );
                    self -> e [ Idx ] = NULL;
                }
            }
        }
    }

    self -> q = 0;

}   /* _StrCClear () */

static
rc_t CC
_StrCDispose ( struct _StrC * self )
{
    if ( self != NULL ) {
        _StrCClear ( self );

        free ( self -> e );

        self -> e = NULL;

        self -> c = 0;

        free ( self );
    }

    return 0;
}   /* _StrCDispose () */

/*))    Capacity - 0 - means default value _STRC_INC_VAL_.
 ((*/
static
rc_t CC
_StrCMake ( struct _StrC ** Cont, size_t Capacity )
{
    rc_t RCt;
    struct _StrC * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CAN ( Cont )
    * Cont = NULL;

    if ( Capacity == 0 ) {
        Capacity = _STRC_INC_VAL_;
    }

    Ret = calloc ( 1, sizeof ( struct _StrC ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcNull );
    }
    else {
        Ret -> q = 0;
        Ret -> c = Capacity;
        Ret -> e = calloc ( Capacity, sizeof ( char * ) );

        * Cont = Ret;
    }

    if ( RCt != 0 ) {
        * Cont = NULL;

        _StrCDispose ( Ret );
    }

    return RCt;
}   /* _StrCMake () */

static
rc_t CC
_StrCMakeFrom ( struct _StrC * From, struct _StrC ** To )
{
    rc_t RCt;
    struct _StrC * Ret;
    size_t Idx;
    const char * Str, * Dup;

    RCt = 0;
    Ret = NULL;
    Idx = 0;
    Str = Dup = NULL;

    XFS_CSAN ( To )
    XFS_CAN ( From )
    XFS_CAN ( To )

    RCt = _StrCMake ( & Ret, From -> c );
    if ( RCt == 0 ) {

            /* Qty is set first for a case */
        Ret -> q = From -> q;

        for ( Idx = 0; Idx < From -> q; Idx ++ ) {
            Str = * ( From -> e + Idx );
            if ( Str != NULL ) {
                RCt = XFS_StrDup ( Str, & Dup );
                if ( RCt != 0 ) {
                    break;
                }

                * ( Ret -> e + Idx ) = ( char * ) Dup;
            }
        }
        if ( RCt == 0 ) {
            * To = Ret;
        }
    }

    return RCt;
}   /* _StrCMakeFrom () */

static
rc_t CC
_StrCReserve ( struct _StrC * self, size_t Amount )
{
    rc_t RCt;
    size_t NewCapas;
    char ** NewArr;

    RCt = 0;
    NewCapas = 0;

    XFS_CAN ( self );

    if ( Amount == 0 ) {
        return 0;
    }

        /* Not enough space */
    NewCapas = ( ( ( self -> q + Amount ) / _STRC_INC_VAL_ ) + 1 )
                                                    * _STRC_INC_VAL_;

    if ( self -> c < NewCapas ) {
        NewArr = calloc ( NewCapas, sizeof ( char * ) );
        if ( NewArr == NULL ) {
            return XFS_RC ( rcExhausted );
        }

        if ( self -> e != NULL ) {
            if ( self -> q != 0 ) {
                memcpy (
                        NewArr,
                        self -> e,
                        sizeof ( char * ) * self -> q
                        );
            }

            free ( self -> e );
            self -> e = NULL;
        }

        self -> c = NewCapas;
        self -> e = NewArr;
    }

    return RCt;
}   /* _StrCReserve () */

static
rc_t CC
_StrCInsertMany (
            struct _StrC * self,
            char ** From,
            size_t Qty,
            size_t Pos
)
{
    rc_t RCt;
    size_t Idx;
    char * Str;

    RCt = 0;
    Idx = 0;
    Str = NULL;

    XFS_CAN ( self )
    XFS_CAN ( From )
    XFS_CA ( Qty, 0 )

        /* First we should check if item come to 0-q interval
         */
    if ( self -> q < Pos ) {
        return XFS_RC ( rcInvalid );
    }

        /* Second we should reserve enough space
         */
    RCt = _StrCReserve ( self, Qty );
    if ( RCt == 0 ) {
                /* Third, shifting content if it is necessary
                 */
            if ( Pos < self -> q ) {
                memmove (
                        self -> e + Pos + Qty,
                        self -> e + Pos,
                        ( self -> q - Pos ) * sizeof ( char * )
                        );
                memset ( self -> e + Pos, 0, Qty * sizeof ( char * ) );
            }

            for ( Idx = 0; Idx < Qty; Idx ++ ) {
                RCt = XFS_StrDup (
                                From [ Idx ],
                                ( const char ** ) & Str
                                );
                if ( RCt != 0 ) {
                    break;
                }

                * ( self -> e + Pos + Idx ) = Str;
            }

            self -> q += Qty;
    }

    return RCt;
}   /* _StrCInsertMany () */

static
rc_t CC
_StrCInsert (
            struct _StrC * self,
            const char * Str,
            size_t Pos
)
{
    return _StrCInsertMany ( self, ( char ** ) & Str, 1, Pos );
}   /* _StrCInsert () */

static
rc_t CC
_StrCAdd (
            struct _StrC * self,
            const char * Str,
            size_t StrLen
)
{
    rc_t RCt;
    char * TheStr;

    RCt = 0;
    TheStr = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Str )
    XFS_CA ( StrLen, 0 )

    TheStr = malloc ( sizeof ( char ) * ( StrLen + 1 ) );
    if ( TheStr == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    memcpy ( TheStr, Str, sizeof ( char ) * StrLen );
    * ( TheStr + StrLen ) = 0;

    RCt = _StrCInsert ( self, TheStr, _StrCCount ( self ) );

    free ( TheStr );

    return RCt;
}   /* _StrCAdd () */

static
rc_t CC
_StrCDeleteMany ( struct _StrC * self, size_t Qty, size_t Pos )
{
    rc_t RCt;
    size_t Idx, To;

    RCt = 0;
    Idx = To = 0;

    XFS_CAN ( self )

    if ( Qty == 0 ) {
        return 0;
    }

        /* First, we should to check upper and lower limits */
    if ( self -> q <= Pos ) {
        return XFS_RC ( rcInvalid );
    }

    To = Pos + Qty;
    if ( self -> q <= To ) {
        To = self -> q;
    }

        /* Second, we are deleting entries */
    for ( Idx = Pos; Idx < To; Idx ++ ) {
        free ( * ( self -> e + Idx ) );
        * ( self -> e + Idx ) = NULL;
    }

        /* Third, we need to move some to fill a gap
         */
    if ( To < self -> q ) {
        memmove (
                self -> e + Pos,
                self -> e + To,
                ( self -> q - To ) * sizeof ( char * )
                );

        memset (
                self -> e + self -> q - To + Pos,
                0,
                ( To - Pos ) * sizeof ( char * )
                );
    }


    self -> q -= ( To - Pos );

    return RCt;
}   /* _StrCDeleteMany () */

static
rc_t CC
_StrCDelete ( struct _StrC * self, size_t Pos )
{
    return _StrCDeleteMany ( self, 1, Pos );
}   /* _StrCDelete () */

const char * CC
_StrCGet ( struct _StrC * self, size_t Idx )
{
    if ( self != NULL ) {
        if ( 0 <= Idx && Idx < self -> q ) {
            return * ( self -> e + Idx );
        }
    }

    return NULL;
}   /* _StrCGet () */

size_t CC
_StrCCount ( struct _StrC * self )
{
    return self != NULL ? self -> q : 0;
}   /* _StrCCount () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSPfad {
    KRefcount refcount;

    struct _StrC * tokens;

    const char * pfad;
    const char * orig;
    bool is_absolute;
};

static const char * _sXFSPfad_classname = "XFSPfad";

static
rc_t CC
_PfadPrune ( struct XFSPfad * self )
{
    rc_t RCt;
    size_t Idx;
    const char * Str;

    RCt = 0;
    Idx = 0;
    Str = NULL;

    XFS_CAN ( self )

    Idx = 0;
    while ( Idx < _StrCCount ( self -> tokens ) ) {
        Str = _StrCGet ( self -> tokens, Idx );

        if ( strcmp ( Str, "." ) == 0 ) {
            _StrCDelete ( self -> tokens, Idx );
            continue;
        }

        if ( strcmp ( Str, ".." ) == 0 ) {
            if ( 0 < Idx ) {
                _StrCDelete ( self -> tokens, Idx );
                Idx --;
                _StrCDelete ( self -> tokens, Idx );

                continue;
            }
        }

        Idx ++;
    }

    return RCt;
}   /* _PfadPrune () */

static
rc_t CC
_PfadCompile ( struct XFSPfad * self, const char ** Str )
{
    size_t Qty, Idx, StrSize;
    char * Pfad;

    Qty = Idx = StrSize = 0;

    XFS_CSAN ( Str )
    XFS_CAN ( self )
    XFS_CAN ( Str )

        /* First we are calculating size of string
         */
    if ( self -> is_absolute ) {        /* Trailing slash */
        StrSize += 1;
    }

    Qty = XFSPfadPartCount ( self );   /* Concatinating slashes */
    if ( 0 < Qty ) {
        StrSize += Qty - 1;

        for ( Idx = 0; Idx < Qty; Idx ++ ) {
            StrSize += string_size ( XFSPfadPartGet ( self, Idx ) );
        }
    }

    Pfad = malloc ( sizeof ( char ) * ( StrSize + 1 ) );
    if ( Pfad == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    * Pfad = 0;

    if ( self -> is_absolute ) {
        strcat ( Pfad, "/" );
    }

    for ( Idx = 0; Idx < Qty; Idx ++ ) {
        if ( Idx != 0 ) {
            strcat ( Pfad, "/" );
        }
        strcat ( Pfad, XFSPfadPartGet ( self, Idx ) );
    }

    * Str = Pfad;

    return 0;
}   /* _PfadCompile () */

static
rc_t CC
_PfadRecompile ( struct XFSPfad * self )
{
    XFS_CAN ( self )

    if ( self -> pfad != NULL ) {
        free ( ( char * ) self -> pfad );

        self -> pfad = NULL;
    }

    return _PfadCompile ( self, & ( self -> pfad ) );
}   /* _PfadRecompile () */

static
void CC
_PfadClear ( struct XFSPfad * self )
{
    if ( self == NULL ) {
        return;
    }

    _StrCClear ( self -> tokens );

    if ( self -> orig != NULL ) {
        free ( ( char * ) self -> orig );

        self -> orig = NULL;
    }

    if ( self -> pfad != NULL ) {
        free ( ( char * ) self -> pfad );

        self -> pfad = NULL;
    }
}   /* _PfadClear () */

static
rc_t CC
_PfadParse ( struct XFSPfad * self, const char * Pfad )
{
    rc_t RCt;
    const char * Bg, * Cr;
    bool Abs;

    RCt = 0;
    Bg = Cr = NULL;
    Abs = false;

    XFS_CAN ( self )
    XFS_CAN ( Pfad )

    _PfadClear ( self );

    Abs = * Pfad == '/';

    Bg = Cr = Pfad;
    while ( * Cr != 0 ) {
        if ( * Cr == '/' ) {
            if ( 0 < Cr - Bg ) {
                RCt = _StrCAdd ( self -> tokens, Bg, Cr - Bg );
                if ( RCt != 0 ) {
                    break;
                }
            }

            while ( * Cr != 0 ) {
                if ( * Cr != '/' ) {
                    Bg = Cr;
                    break;
                }
                Cr ++;
            }
        }
        Cr ++;
    }

    if ( RCt == 0 ) {
        if ( 0 < Cr - Bg ) {
            RCt = _StrCAdd ( self -> tokens, Bg, Cr - Bg );
        }

        if ( RCt == 0 ) {
            self -> is_absolute = Abs;

            RCt = XFS_StrDup ( Pfad, & ( self -> orig ) );
            if ( RCt == 0 ) {

                RCt = _PfadPrune ( self );
                if ( RCt == 0 ) {
                    RCt = _PfadRecompile ( self );
                }
            }
        }
    }

    return RCt;
}   /* _PfadParse () */

static
rc_t CC
_PfadDispose ( const struct XFSPfad * self )
{
    struct XFSPfad * Pfad = ( struct XFSPfad * ) self;

    if ( Pfad == NULL ) {
        return 0;
    }

    _PfadClear ( Pfad );

    _StrCDispose ( Pfad -> tokens );

    KRefcountWhack ( & ( Pfad -> refcount ), _sXFSPfad_classname );

    free ( Pfad );

    return 0;
}   /* _PfadDispose () */

/*) This will alloc XFSPfad structure and initialize tokens.
  | If the From parameter is not NULL, it will fill tokens wit From
  | content
  (*/
static
rc_t CC
_PfadAlloc ( struct XFSPfad ** Out, struct _StrC * From_CanBeZero )
{
    rc_t RCt;
    struct XFSPfad * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( Out )

    Ret = calloc ( 1, sizeof ( struct XFSPfad ) );
    if ( Ret == NULL ) {
        return XFS_RC ( rcExhausted );
    }


    KRefcountInit (
                & ( Ret -> refcount ),
                1,
                _sXFSPfad_classname,
                "_PfadAlloc",
                "Pfad"
                );

    RCt = From_CanBeZero == NULL
            ? _StrCMake ( & ( Ret -> tokens ), 0 ) 
            : _StrCMakeFrom ( From_CanBeZero, & ( Ret -> tokens ) )
            ;
    if ( RCt == 0 ) {
        * Out = Ret;
    }
    else {
        * Out = NULL;

        _PfadDispose ( Ret );
    }

    return RCt;
}   /* _PfadAlloc () */

LIB_EXPORT
rc_t CC
XFSPfadVMake (
            const struct XFSPfad ** Out,
            const char * Format,
            va_list Args
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_1024 ];
    size_t NumWrit;
    struct XFSPfad * Ret;
    va_list xArgs;

    RCt = 0;
    * BF = 0;
    NumWrit = 0;
    Ret = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( Format )
    XFS_CAN ( Out )

        /* First we are trying to allocate structure
         */
    RCt = _PfadAlloc ( & Ret, NULL );
    if ( RCt == 0 ) {
            /* Second we should format pfad
             */
        va_copy ( xArgs, Args );
        RCt = string_vprintf (
                            BF,
                            sizeof ( BF ),
                            & NumWrit,
                            Format,
                            xArgs
                            );
        va_end ( xArgs );
        if ( RCt == 0 ) {
               /* Third we should parse pfad
                */
            RCt = _PfadParse ( Ret, BF );
            if ( RCt == 0 ) {
                * Out = Ret;
            }
        }
    }

    if ( RCt != 0 ) {
        * Out = NULL;
        if ( Ret != NULL ) {
            _PfadDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPfadVMake () */

LIB_EXPORT
rc_t CC
XFSPfadMake ( const struct XFSPfad ** Out, const char * Format, ... )
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Format );
    RCt = XFSPfadVMake ( Out, Format, Args );
    va_end ( Args );

    return RCt;
}   /* XFSPfadMake () */

LIB_EXPORT
rc_t CC
XFSPfadVMakeAbsolute (
                const struct XFSPfad ** Out,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    const struct KDirectory * NatDir;
    char BF [ XFS_SIZE_1024 ];
    va_list xArgs;

    RCt = 0;
    NatDir = NULL;
    * BF = 0;

    XFS_CSAN ( Out )
    XFS_CAN ( Out )
    XFS_CAN ( Format )

    RCt = KDirectoryNativeDir ( ( struct KDirectory ** ) & NatDir );
    if ( RCt == 0 ) {
        va_copy ( xArgs, Args );
        RCt = KDirectoryVResolvePath (
                                    NatDir, /* Directory */
                                    true,   /* Absolute */
                                    BF,
                                    sizeof ( BF ),
                                    Format,
                                    xArgs
                                    );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = XFSPfadMake ( Out, BF );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSPfadVMakeAbsolute () */

LIB_EXPORT
rc_t CC
XFSPfadMakeAbsolute (
                const struct XFSPfad ** Out,
                const char * Format,
                ...
)
{
    rc_t RCt;
    va_list Args;

    va_start ( Args, Format );

    RCt = XFSPfadVMakeAbsolute ( Out, Format, Args );

    va_end ( Args );

    return RCt;
}   /* XFSPfadMakeAbsolute () */

LIB_EXPORT
rc_t CC
XFSPfadDup (
            const struct XFSPfad * In,
            const struct XFSPfad ** Out
)
{
    rc_t RCt;
    struct XFSPfad * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( In )
    XFS_CAN ( Out )

    RCt = _PfadAlloc ( & Ret, In -> tokens );
    if ( RCt == 0 ) {

        RCt = XFS_StrDup ( In -> orig, & ( Ret -> orig ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( In -> orig, & ( Ret -> orig ) );
            if ( RCt == 0 ) {
                Ret -> is_absolute = In -> is_absolute;
                * Out = Ret;
            }
        }
    }

    if ( RCt != 0 ) {
        * Out = NULL;
        if ( Ret != NULL ) {
            _PfadDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPfadDup () */

LIB_EXPORT
rc_t CC
XFSPfadRelease ( const struct XFSPfad * self )
{
    rc_t RCt;
    struct XFSPfad * Pfad;

    RCt = 0;
    Pfad = ( struct XFSPfad * ) self;

    XFS_CAN ( Pfad )

    switch ( KRefcountDrop ( & ( Pfad -> refcount ), _sXFSPfad_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _PfadDispose ( Pfad );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSPfadRelease () */

LIB_EXPORT
rc_t CC
XFSPfadAddRef ( const struct XFSPfad * self )
{
    rc_t RCt;
    struct XFSPfad * Pfad;

    RCt = 0;
    Pfad = ( struct XFSPfad * ) self;


    XFS_CAN ( Pfad )

    switch ( KRefcountAdd ( & ( Pfad -> refcount ), _sXFSPfad_classname )) {
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
}   /* XFSPfadAddRef () */

XFS_EXTERN
rc_t CC
XFSPfad ( const struct XFSPfad * self, const char * Format, ... )
{
    rc_t RCt;
    va_list Args;
    char BF [ XFS_SIZE_1024 ];
    size_t NumWrit;

    RCt = 0;
    * BF = 0;
    NumWrit = 0;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    va_start ( Args, Format );
    RCt = string_printf (
                        BF,
                        sizeof ( BF ),
                        & NumWrit,
                        Format,
                        Args
                        );
    va_end ( Args );
    if ( RCt == 0 ) {
        RCt = _PfadParse ( ( struct XFSPfad * ) self, BF ); 
    }

    return RCt;
}   /* XFSPfadSet () */

XFS_EXTERN
rc_t CC
XFSPfadSetPfad (
                const struct XFSPfad * self,
                const struct XFSPfad * Pfad
)
{
    rc_t RCt;
    struct XFSPfad * TheSelf;

    RCt = 0;
    TheSelf = ( struct XFSPfad * ) self;

    XFS_CAN ( TheSelf )
    XFS_CAN ( Pfad )

    RCt = _StrCMakeFrom ( Pfad -> tokens, & ( TheSelf -> tokens ) );
    if ( RCt == 0 ) {

        RCt = XFS_StrDup ( Pfad -> orig, & ( TheSelf -> orig ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Pfad -> orig, & ( TheSelf -> orig ) );
            if ( RCt == 0 ) {
                TheSelf -> is_absolute = Pfad -> is_absolute;
            }
        }
    }

    return RCt;
}   /* XFSPfadSetPfad () */

LIB_EXPORT
rc_t CC
XFSPfadAppend (
                const struct XFSPfad * self,
                const char * Format,
                ...
)
{
    rc_t RCt;
    const struct XFSPfad * ThePfad;
    va_list Args;

    RCt = 0;
    ThePfad = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    va_start ( Args, Format );
    RCt = XFSPfadVMake ( & ThePfad, Format, Args );
    va_end ( Args );

    if ( RCt == 0 ) {
        RCt = XFSPfadAppendPfad ( self, ThePfad );

        XFSPfadRelease ( ThePfad );
    }

    return RCt;
}   /* XFSPfadAppend () */

LIB_EXPORT
rc_t CC
XFSPfadAppendPfad (
                const struct XFSPfad * self,
                const struct XFSPfad * Pfad
)
{
    rc_t RCt;
    char * NewOrig;
    size_t NewLen;
    struct XFSPfad * TheSelf;

    RCt = 0;
    NewOrig = NULL;
    NewLen = 0;
    TheSelf = ( struct XFSPfad * ) self;

    XFS_CAN ( TheSelf )
    XFS_CAN ( Pfad )

    RCt = _StrCInsertMany (
                        TheSelf -> tokens,
                        Pfad -> tokens -> e, 
                        Pfad -> tokens -> q, 
                        TheSelf -> tokens -> q
                        );
    if ( RCt == 0 ) {
        NewLen = strlen ( TheSelf -> orig ) + strlen ( Pfad -> orig );
        NewOrig = malloc ( sizeof ( char ) * ( 1 + NewLen ) );
        if ( NewOrig == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            strcpy ( NewOrig, TheSelf -> orig );
            strcat ( NewOrig, Pfad -> orig );
            if ( TheSelf -> orig != NULL ) {
                free ( ( char * ) TheSelf -> orig );
            }
            TheSelf -> orig = NewOrig;

            RCt = _PfadPrune ( TheSelf );
            if ( RCt == 0 ) {
                RCt = _PfadRecompile ( TheSelf );
            }
        }
    }

    return RCt;
}   /* XFSPfadAppendPfad () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
bool CC
XFSPfadIsAbsolute ( const struct XFSPfad * self )
{
    return self == NULL ? false : self -> is_absolute;
}   /* XFSPfadIsAbsolute () */

LIB_EXPORT
uint32_t CC
XFSPfadPartCount ( const struct XFSPfad * self )
{
    return self == NULL ? 0 : _StrCCount ( self -> tokens );
}   /* XFSPfadPartCount () */

LIB_EXPORT
const char * CC
XFSPfadPartGet ( const struct XFSPfad * self, uint32_t Index )
{
    return self == NULL ? 0 : _StrCGet ( self -> tokens, Index );
}   /* XFSPfadPartGet () */

LIB_EXPORT
const char * CC
XFSPfadGet ( const struct XFSPfad * self )
{
    return self == NULL ? NULL : self -> pfad;
}   /* XFSPfadGet () */

LIB_EXPORT
const char * CC
XFSPfadOrig ( const struct XFSPfad * self )
{
    return self == NULL ? NULL : self -> orig;
}   /* XFSPfadOrig () */

LIB_EXPORT
rc_t CC
XFSPfadParent (
                const struct XFSPfad * self,
                const struct XFSPfad ** Parent
)
{
    rc_t RCt;
    const struct XFSPfad * Pfad;
    uint32_t Count;

    RCt = 0;
    Pfad = NULL;
    Count = 0;

    XFS_CSAN ( Parent )
    XFS_CAN ( self )
    XFS_CAN ( Parent )

    Count = XFSPfadPartCount ( self );
    if ( Count < 1 ) {
        RCt = _PfadAlloc ( ( struct XFSPfad ** ) & Pfad, NULL );
    }
    else {
        RCt = XFSPfadTo ( self, Count - 1, & Pfad );
    }

    if ( RCt == 0 ) {
        * Parent = Pfad;
    }
    else {
        * Parent = NULL;

        if ( Pfad != NULL ) {
            XFSPfadRelease ( Pfad );
        }
    }

    return RCt;
}   /* XFSPfadParent () */

    /*) Don't forget to free Name after using it
     (*/
LIB_EXPORT
const char * CC
XFSPfadName ( const struct XFSPfad * self )
{
    uint32_t Count;

    if ( self != NULL ) {
        Count = XFSPfadPartCount ( self );

        return Count < 1
                    ? _sPfadEmptyName
                    : XFSPfadPartGet ( self, Count - 1 )
                    ;
    }

    return NULL;
}   /* XFSPfadName () */

LIB_EXPORT
bool CC
XFSPfadEqual (
            const struct XFSPfad * Pfad1,
            const struct XFSPfad * Pfad2
)
{
    size_t s;

    if ( Pfad1 != NULL && Pfad2 != NULL ) {
        if ( Pfad1 == Pfad2 ) {
            return true;
        }

        s = string_size ( XFSPfadGet ( Pfad1 ) );

        if ( s == string_size ( XFSPfadGet ( Pfad2 ) ) ) {
            return string_cmp (
                            XFSPfadGet ( Pfad1 ),
                            s,
                            XFSPfadGet ( Pfad2 ),
                            s,
                            s
                            ) == 0;
        }

    }
    return false;
}   /* XFSPfadEqual () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSPfadSub (
            const struct XFSPfad * self,
            size_t From,
            size_t To,
            const struct XFSPfad ** Pfad
)
{
    rc_t RCt;
    size_t Idx, Qty;
    struct XFSPfad * Ret;

    RCt = 0;
    Idx = Qty = 0;
    Ret = NULL;

    XFS_CSAN ( Pfad )
    XFS_CAN ( self )
    XFS_CAN ( Pfad )

        /*) First, checking range
         (*/
    Qty = XFSPfadPartCount ( self );
    if ( Qty < From || Qty < To ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _PfadAlloc ( & Ret, NULL );
    if ( RCt == 0 ) {
        for ( Idx = From; Idx < To; Idx ++ ) {
            RCt = _StrCInsert (
                            Ret -> tokens,
                            _StrCGet ( self -> tokens, Idx ),
                            _StrCCount ( Ret -> tokens )
                            );
            if ( RCt != 0 ) {
                break;
            }
        }
        if ( RCt == 0 ) {
            Ret -> is_absolute = From == 0
                                    ? self -> is_absolute
                                    : false
                                    ;
            RCt = _PfadPrune ( Ret );
            if ( RCt == 0 ) {
                RCt = _PfadRecompile ( Ret );
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup (
                                    Ret -> pfad,
                                    & ( Ret -> orig )
                                    );
                    if ( RCt == 0 ) {
                        * Pfad = Ret;
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Pfad = NULL;

        if ( Ret != NULL ) {
            _PfadDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPfadSub () */

LIB_EXPORT
rc_t CC
XFSPfadFrom (
            const struct XFSPfad * self,
            size_t From,
            const struct XFSPfad ** Pfad
)
{
    XFS_CAN ( self )

    return XFSPfadSub ( self, From, XFSPfadPartCount ( self ), Pfad );
}   /* XFSPfadFrom () */

LIB_EXPORT
rc_t CC
XFSPfadTo (
            const struct XFSPfad * self,
            size_t To,
            const struct XFSPfad ** Pfad
)
{
    return XFSPfadSub ( self, 0, To, Pfad );
}   /* XFSPfadTo () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*) That method will check if Pfad is base pfad for 'self'. i.e.
| self could be composed as concatenation of Pfad and RelPfad
| RelPfad could be zero, in that case it will be not returned
| Apparently, that means Pfad is a sub of self, so we will call
| XFSPfadIsChild method with reversed parameters
(*/
LIB_EXPORT
bool CC
XFSPfadIsBase (
            const struct XFSPfad * self,
            const struct XFSPfad * Pfad,
            const struct XFSPfad ** RelPfad
)
{
    return XFSPfadIsChild ( Pfad, self, RelPfad );
}   /* XFSPfadIsBase () */

LIB_EXPORT
bool CC
XFSPfadSIsBase (
            const char * self,
            const char * Pfad,
            const struct XFSPfad ** RelPfad
)
{
    return XFSPfadSIsChild ( Pfad, self, RelPfad );
}   /* XFSPfadSIsBase () */

/*) That method will check if Pfad is child ( sub ) pfad for 'self'.
| i.e. Pfad could be composed as concatenation of self and RelPfad
| RelPfad could be zero, in that case it will be not returned
(*/
LIB_EXPORT
bool CC
XFSPfadIsChild (
            const struct XFSPfad * self,
            const struct XFSPfad * Pfad,
            const struct XFSPfad ** RelPfad
)
{
    rc_t RCt;
    bool RetVal;
    size_t SelfQty, PfadQty, Idx;
    const char * s1, * s2;
    const struct XFSPfad * TheRel;

    RCt = 0;
    RetVal = false;
    SelfQty = PfadQty = Idx = 0;
    s1 = s2 = NULL;
    TheRel = NULL;

    XFS_CSAN ( RelPfad )
    XFS_CAN ( self )
    XFS_CAN ( Pfad )
    XFS_CAN ( RelPfad )


    /*) Apparently we shoud check that all parts of TheSelf
      | are also parts of ThePfad
      (*/
    SelfQty = XFSPfadPartCount ( self );
    PfadQty = XFSPfadPartCount ( Pfad );

        /* We could compare only absolute pfad ... because
         | we can not tell about relative pfad how they are
         | relative ... lol
         */
    if ( self -> is_absolute && Pfad -> is_absolute && 
        SelfQty <= PfadQty ) {
        for ( Idx = 0; Idx < SelfQty; Idx ++ ) {
            s1 = XFSPfadPartGet ( self, Idx );
            s2 = XFSPfadPartGet ( Pfad, Idx );
            if ( s1 == NULL || s2 == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }

            if ( strcmp ( s1, s2 ) != 0 ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }

        if ( RCt == 0 ) {
            RetVal = true;

            if ( RelPfad != NULL ) {
                RCt = XFSPfadFrom (
                                Pfad,
                                SelfQty,
                                & TheRel
                                );
                if ( RCt == 0 ) {
                    * RelPfad = TheRel;
                }
                else {
                    if ( TheRel != NULL ) {
                        _PfadDispose ( TheRel );
                    }
                }
            }
        }
    }

    return RCt == 0 ? RetVal : false;
}   /* XFSPfadIsChild () */

LIB_EXPORT
bool CC
XFSPfadSIsChild (
            const char * self,
            const char * Pfad,
            const struct XFSPfad ** RelPfad
)
{
    rc_t RCt;
    bool RetVal;
    const struct XFSPfad * TheSelf;
    const struct XFSPfad * ThePfad;

    RCt = 0;
    RetVal = false;
    ThePfad = NULL;

    XFS_CSAN ( RelPfad )
    XFS_CAN ( self )
    XFS_CAN ( Pfad )

    RCt = XFSPfadMake ( & TheSelf, self );
    if ( RCt == 0 ) {
        RCt = XFSPfadMake ( & ThePfad, Pfad );
        if ( RCt == 0 ) {
            RetVal = XFSPfadIsChild ( TheSelf, ThePfad, RelPfad );

            XFSPfadRelease ( ThePfad );
        }

        XFSPfadRelease ( TheSelf );
    }

    return RCt == 0 ? RetVal : false;
}   /* XFSPfadSIsChild () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
void CC
XFSPfadDump ( const struct XFSPfad * self )
{
    size_t Idx = 0;

    if ( self == NULL ) {
        printf ( " [Pfad] NULL\n" );
        return;
    }

    printf ( " [Pfad] [%s]\n", self -> orig );
    printf ( "   [abs] [%s]\n", self -> is_absolute ? "yes" : "no" );
    printf ( "  [pfad] [%s]\n", self -> pfad );

    printf ( "   [qty] [%ld]\n", self -> tokens -> q );
    for ( Idx = 0; Idx < self -> tokens ->q; Idx ++ ) {
        printf ( "        [%ld] [%s]\n", Idx, self -> tokens -> e [ Idx ] );
    }

}   /* XFSPfadDump () */
