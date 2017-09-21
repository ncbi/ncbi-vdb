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
#include <klib/log.h>

#include <kfs/directory.h>

#include <va_copy.h>

#include <xfs/path.h>
#include "schwarzschraube.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static char * _sPathEmptyName = "";

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
                memmove (
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

    TheStr = calloc ( StrLen + 1, sizeof ( char ) );
    if ( TheStr == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    memmove ( TheStr, Str, sizeof ( char ) * StrLen );
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
        if ( Idx < self -> q ) {
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

struct XFSPath {
    KRefcount refcount;

    struct _StrC * tokens;

    const char * path;
    const char * orig;
    bool is_absolute;
};

static const char * _sXFSPath_classname = "XFSPath";

static
rc_t CC
_PathPrune ( struct XFSPath * self )
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
}   /* _PathPrune () */

static
rc_t CC
_PathCompile ( struct XFSPath * self, const char ** Str )
{
    size_t Qty, Idx, StrSize;
    char * Path;
    const char * Token;

    Qty = Idx = StrSize = 0;
    Path = NULL;
    Token = NULL;

    XFS_CSAN ( Str )
    XFS_CAN ( self )
    XFS_CAN ( Str )

        /* First we are calculating size of string
         */
    if ( self -> is_absolute ) {        /* Trailing slash */
        StrSize += 1;
    }

    Qty = XFSPathPartCount ( self );   /* Concatinating slashes */
    if ( 0 < Qty ) {
        StrSize += Qty - 1;

        for ( Idx = 0; Idx < Qty; Idx ++ ) {
            StrSize += string_size ( XFSPathPartGet ( self, Idx ) );
        }
    }

    Path = calloc ( StrSize + 1, sizeof ( char ) );
    if ( Path == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    * Path = 0;

    if ( self -> is_absolute ) {
        strcat ( Path, "/" );
    }

    for ( Idx = 0; Idx < Qty; Idx ++ ) {
        Token = XFSPathPartGet ( self, Idx );
        if ( strcmp ( Token, "/" ) != 0 ) {
            if ( Idx != 0 ) {
                strcat ( Path, "/" );
            }
            strcat ( Path, Token );
        }
    }

    * Str = Path;

    return 0;
}   /* _PathCompile () */

static
rc_t CC
_PathRecompile ( struct XFSPath * self )
{
    XFS_CAN ( self )

    if ( self -> path != NULL ) {
        free ( ( char * ) self -> path );

        self -> path = NULL;
    }

    return _PathCompile ( self, & ( self -> path ) );
}   /* _PathRecompile () */

static
void CC
_PathClear ( struct XFSPath * self )
{
    if ( self == NULL ) {
        return;
    }

    _StrCClear ( self -> tokens );

    if ( self -> orig != NULL ) {
        free ( ( char * ) self -> orig );

        self -> orig = NULL;
    }

    if ( self -> path != NULL ) {
        free ( ( char * ) self -> path );

        self -> path = NULL;
    }
}   /* _PathClear () */

static
rc_t CC
_PathParse (
            struct XFSPath * self,
            bool AddPrecedingSlash,
            const char * Path
)
{
    rc_t RCt;
    const char * Bg, * Cr, * En;
    bool Abs;

    RCt = 0;
    Bg = NULL;
    Cr = NULL;
    En = NULL;
    Abs = false;

    XFS_CAN ( self )
    XFS_CAN ( Path )

    _PathClear ( self );

    Abs = * Path == '/';

    if ( Abs && AddPrecedingSlash ) {
        RCt = _StrCAdd ( self -> tokens, "/", 1 );
    }

    if ( RCt == 0 ) {
        Bg = Path;
        Cr = Path;
        En = Bg + string_size ( Path );
        while ( Cr < En ) {
            if ( * Cr == '/' ) {
                if ( 0 < Cr - Bg ) {
                    RCt = _StrCAdd ( self -> tokens, Bg, Cr - Bg );
                    if ( RCt != 0 ) {
                        break;
                    }
                }

        	    while ( Cr < En ) {
                    if ( * Cr != '/' ) {
                        Bg = Cr;
                        break;
                    }
                    Cr ++;
                }
            }
            Cr ++;
        }
    }

    if ( RCt == 0 ) {
        if ( 0 < Cr - Bg && * Bg != '/' ) {
            RCt = _StrCAdd ( self -> tokens, Bg, Cr - Bg );
        }

        if ( RCt == 0 ) {
            self -> is_absolute = Abs;

            RCt = XFS_StrDup ( Path, & ( self -> orig ) );
            if ( RCt == 0 ) {

                RCt = _PathPrune ( self );
                if ( RCt == 0 ) {
                    RCt = _PathRecompile ( self );
                }
            }
        }
    }

    return RCt;
}   /* _PathParse () */

static
rc_t CC
_PathDispose ( const struct XFSPath * self )
{
    struct XFSPath * Path = ( struct XFSPath * ) self;

    if ( Path == NULL ) {
        return 0;
    }

    _PathClear ( Path );

    _StrCDispose ( Path -> tokens );

    KRefcountWhack ( & ( Path -> refcount ), _sXFSPath_classname );

    free ( Path );

    return 0;
}   /* _PathDispose () */

/*) This will alloc XFSPath structure and initialize tokens.
  | If the From parameter is not NULL, it will fill tokens wit From
  | content
  (*/
static
rc_t CC
_PathAlloc ( struct XFSPath ** Out, struct _StrC * From_CanBeZero )
{
    rc_t RCt;
    struct XFSPath * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( Out )

    Ret = calloc ( 1, sizeof ( struct XFSPath ) );
    if ( Ret == NULL ) {
        return XFS_RC ( rcExhausted );
    }


    KRefcountInit (
                & ( Ret -> refcount ),
                1,
                _sXFSPath_classname,
                "_PathAlloc",
                "Path"
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

        _PathDispose ( Ret );
    }

    return RCt;
}   /* _PathAlloc () */

LIB_EXPORT
rc_t CC
XFSPathVMake (
            const struct XFSPath ** Out,
            bool AddPrecedingSlash,
            const char * Format,
            va_list Args
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_1024 ];
    size_t NumWrit;
    struct XFSPath * Ret;
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
    RCt = _PathAlloc ( & Ret, NULL );
    if ( RCt == 0 ) {
            /* Second we should format path
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
               /* Third we should parse path
                */
            RCt = _PathParse ( Ret, AddPrecedingSlash, BF );
            if ( RCt == 0 ) {
                * Out = Ret;
            }
        }
    }

    if ( RCt != 0 ) {
        * Out = NULL;
        if ( Ret != NULL ) {
            _PathDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPathVMake () */

LIB_EXPORT
rc_t CC
XFSPathMake (
            const struct XFSPath ** Out,
            bool AddPrecedingSlash,
            const char * Format,
            ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Format );
    RCt = XFSPathVMake ( Out, AddPrecedingSlash, Format, Args );
    va_end ( Args );

    return RCt;
}   /* XFSPathMake () */

LIB_EXPORT
rc_t CC
XFSPathVMakeAbsolute (
                const struct XFSPath ** Out,
                bool AddPrecedingSlash,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_1024 ];
    va_list xArgs;

    RCt = 0;
    * BF = 0;

    XFS_CSAN ( Out )
    XFS_CAN ( Out )
    XFS_CAN ( Format )

    va_copy ( xArgs, Args );
    RCt = XFS_VResolvePath (
                            true,   /* Absolute */
                            BF,
                            sizeof ( BF ),
                            Format,
                            xArgs
                            );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RCt = XFSPathMake ( Out, AddPrecedingSlash, BF );
    }

    return RCt;
}   /* XFSPathVMakeAbsolute () */

LIB_EXPORT
rc_t CC
XFSPathMakeAbsolute (
                const struct XFSPath ** Out,
                bool AddPrecedingSlash,
                const char * Format,
                ...
)
{
    rc_t RCt;
    va_list Args;

    va_start ( Args, Format );

    RCt = XFSPathVMakeAbsolute ( Out, AddPrecedingSlash, Format, Args );

    va_end ( Args );

    return RCt;
}   /* XFSPathMakeAbsolute () */

LIB_EXPORT
rc_t CC
XFSPathDup (
            const struct XFSPath * In,
            const struct XFSPath ** Out
)
{
    rc_t RCt;
    struct XFSPath * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Out )
    XFS_CAN ( In )
    XFS_CAN ( Out )

    RCt = _PathAlloc ( & Ret, In -> tokens );
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
            _PathDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPathDup () */

LIB_EXPORT
rc_t CC
XFSPathRelease ( const struct XFSPath * self )
{
    rc_t RCt;
    struct XFSPath * Path;

    RCt = 0;
    Path = ( struct XFSPath * ) self;

    XFS_CAN ( Path )

    switch ( KRefcountDrop ( & ( Path -> refcount ), _sXFSPath_classname ) ) {

        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _PathDispose ( Path );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSPathRelease () */

LIB_EXPORT
rc_t CC
XFSPathAddRef ( const struct XFSPath * self )
{
    rc_t RCt;
    struct XFSPath * Path;

    RCt = 0;
    Path = ( struct XFSPath * ) self;


    XFS_CAN ( Path )

    switch ( KRefcountAdd ( & ( Path -> refcount ), _sXFSPath_classname )) {
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
}   /* XFSPathAddRef () */

XFS_EXTERN
rc_t CC
XFSPathSet (
            const struct XFSPath * self,
            bool AddPrecedingSlash,
            const char * Format,
            ...
)
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
        RCt = _PathParse ( ( struct XFSPath * ) self, AddPrecedingSlash, BF ); 
    }

    return RCt;
}   /* XFSPathSet () */

XFS_EXTERN
rc_t CC
XFSPathSetPath (
                const struct XFSPath * self,
                const struct XFSPath * Path
)
{
    rc_t RCt;
    struct XFSPath * TheSelf;

    RCt = 0;
    TheSelf = ( struct XFSPath * ) self;

    XFS_CAN ( TheSelf )
    XFS_CAN ( Path )

    RCt = _StrCMakeFrom ( Path -> tokens, & ( TheSelf -> tokens ) );
    if ( RCt == 0 ) {

        RCt = XFS_StrDup ( Path -> orig, & ( TheSelf -> orig ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Path -> orig, & ( TheSelf -> orig ) );
            if ( RCt == 0 ) {
                TheSelf -> is_absolute = Path -> is_absolute;
            }
        }
    }

    return RCt;
}   /* XFSPathSetPath () */

LIB_EXPORT
rc_t CC
XFSPathAppend (
                const struct XFSPath * self,
                const char * Format,
                ...
)
{
    rc_t RCt;
    const struct XFSPath * ThePath;
    va_list Args;

    RCt = 0;
    ThePath = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    va_start ( Args, Format );
    RCt = XFSPathVMake ( & ThePath, false, Format, Args );
    va_end ( Args );

    if ( RCt == 0 ) {
        RCt = XFSPathAppendPath ( self, ThePath );

        XFSPathRelease ( ThePath );
    }

    return RCt;
}   /* XFSPathAppend () */

LIB_EXPORT
rc_t CC
XFSPathAppendPath (
                const struct XFSPath * self,
                const struct XFSPath * Path
)
{
    rc_t RCt;
    char * NewOrig;
    size_t NewLen;
    struct XFSPath * TheSelf;

    RCt = 0;
    NewOrig = NULL;
    NewLen = 0;
    TheSelf = ( struct XFSPath * ) self;

    XFS_CAN ( TheSelf )
    XFS_CAN ( Path )

    RCt = _StrCInsertMany (
                        TheSelf -> tokens,
                        Path -> tokens -> e, 
                        Path -> tokens -> q, 
                        TheSelf -> tokens -> q
                        );
    if ( RCt == 0 ) {
        NewLen = strlen ( TheSelf -> orig ) + strlen ( Path -> orig );
        NewOrig = calloc ( 1 + NewLen, sizeof ( char ) );
        if ( NewOrig == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            strcpy ( NewOrig, TheSelf -> orig );
            strcat ( NewOrig, Path -> orig );
            if ( TheSelf -> orig != NULL ) {
                free ( ( char * ) TheSelf -> orig );
            }
            TheSelf -> orig = NewOrig;

            RCt = _PathPrune ( TheSelf );
            if ( RCt == 0 ) {
                RCt = _PathRecompile ( TheSelf );
            }
        }
    }

    return RCt;
}   /* XFSPathAppendPath () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
bool CC
XFSPathIsAbsolute ( const struct XFSPath * self )
{
    return self == NULL ? false : self -> is_absolute;
}   /* XFSPathIsAbsolute () */

LIB_EXPORT
uint32_t CC
XFSPathPartCount ( const struct XFSPath * self )
{
    return self == NULL ? 0 : _StrCCount ( self -> tokens );
}   /* XFSPathPartCount () */

LIB_EXPORT
const char * CC
XFSPathPartGet ( const struct XFSPath * self, uint32_t Index )
{
    return self == NULL ? 0 : _StrCGet ( self -> tokens, Index );
}   /* XFSPathPartGet () */

LIB_EXPORT
const char * CC
XFSPathGet ( const struct XFSPath * self )
{
    return self == NULL ? NULL : self -> path;
}   /* XFSPathGet () */

LIB_EXPORT
const char * CC
XFSPathOrig ( const struct XFSPath * self )
{
    return self == NULL ? NULL : self -> orig;
}   /* XFSPathOrig () */

LIB_EXPORT
rc_t CC
XFSPathParent (
                const struct XFSPath * self,
                const struct XFSPath ** Parent
)
{
    rc_t RCt;
    const struct XFSPath * Path;
    uint32_t Count;

    RCt = 0;
    Path = NULL;
    Count = 0;

    XFS_CSAN ( Parent )
    XFS_CAN ( self )
    XFS_CAN ( Parent )

    Count = XFSPathPartCount ( self );
    if ( Count < 1 ) {
        RCt = _PathAlloc ( ( struct XFSPath ** ) & Path, NULL );
    }
    else {
        RCt = XFSPathTo ( self, Count - 1, & Path );
    }

    if ( RCt == 0 ) {
        * Parent = Path;
    }
    else {
        * Parent = NULL;

        if ( Path != NULL ) {
            XFSPathRelease ( Path );
        }
    }

    return RCt;
}   /* XFSPathParent () */

    /*) Don't forget to free Name after using it
     (*/
LIB_EXPORT
const char * CC
XFSPathName ( const struct XFSPath * self )
{
    uint32_t Count;

    if ( self != NULL ) {
        Count = XFSPathPartCount ( self );

        return Count < 1
                    ? _sPathEmptyName
                    : XFSPathPartGet ( self, Count - 1 )
                    ;
    }

    return NULL;
}   /* XFSPathName () */

LIB_EXPORT
bool CC
XFSPathEqual (
            const struct XFSPath * Path1,
            const struct XFSPath * Path2
)
{
    size_t s;

    if ( Path1 != NULL && Path2 != NULL ) {
        if ( Path1 == Path2 ) {
            return true;
        }

        s = string_size ( XFSPathGet ( Path1 ) );

        if ( s == string_size ( XFSPathGet ( Path2 ) ) ) {
            return string_cmp (
                            XFSPathGet ( Path1 ),
                            s,
                            XFSPathGet ( Path2 ),
                            s,
                            s
                            ) == 0;
        }

    }
    return false;
}   /* XFSPathEqual () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSPathSub (
            const struct XFSPath * self,
            size_t From,
            size_t To,
            const struct XFSPath ** Path
)
{
    rc_t RCt;
    size_t Idx, Qty;
    struct XFSPath * Ret;

    RCt = 0;
    Idx = Qty = 0;
    Ret = NULL;

    XFS_CSAN ( Path )
    XFS_CAN ( self )
    XFS_CAN ( Path )

        /*) First, checking range
         (*/
    Qty = XFSPathPartCount ( self );
    if ( Qty < From || Qty < To ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _PathAlloc ( & Ret, NULL );
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
            RCt = _PathPrune ( Ret );
            if ( RCt == 0 ) {
                RCt = _PathRecompile ( Ret );
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup (
                                    Ret -> path,
                                    & ( Ret -> orig )
                                    );
                    if ( RCt == 0 ) {
                        * Path = Ret;
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Path = NULL;

        if ( Ret != NULL ) {
            _PathDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSPathSub () */

LIB_EXPORT
rc_t CC
XFSPathFrom (
            const struct XFSPath * self,
            size_t From,
            const struct XFSPath ** Path
)
{
    XFS_CAN ( self )

    return XFSPathSub ( self, From, XFSPathPartCount ( self ), Path );
}   /* XFSPathFrom () */

LIB_EXPORT
rc_t CC
XFSPathTo (
            const struct XFSPath * self,
            size_t To,
            const struct XFSPath ** Path
)
{
    return XFSPathSub ( self, 0, To, Path );
}   /* XFSPathTo () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*) That method will check if Path is base path for 'self'. i.e.
| self could be composed as concatenation of Path and RelPath
| RelPath could be zero, in that case it will be not returned
| Apparently, that means Path is a sub of self, so we will call
| XFSPathIsChild method with reversed parameters
(*/
LIB_EXPORT
bool CC
XFSPathIsBase (
            const struct XFSPath * self,
            const struct XFSPath * Path,
            const struct XFSPath ** RelPath
)
{
    return XFSPathIsChild ( Path, self, RelPath );
}   /* XFSPathIsBase () */

LIB_EXPORT
bool CC
XFSPathSIsBase (
            const char * self,
            const char * Path,
            const struct XFSPath ** RelPath
)
{
    return XFSPathSIsChild ( Path, self, RelPath );
}   /* XFSPathSIsBase () */

/*) That method will check if Path is child ( sub ) path for 'self'.
| i.e. Path could be composed as concatenation of self and RelPath
| RelPath could be zero, in that case it will be not returned
(*/
LIB_EXPORT
bool CC
XFSPathIsChild (
            const struct XFSPath * self,
            const struct XFSPath * Path,
            const struct XFSPath ** RelPath
)
{
    rc_t RCt;
    bool RetVal;
    size_t SelfQty, PathQty, Idx;
    const char * s1, * s2;
    const struct XFSPath * TheRel;

    RCt = 0;
    RetVal = false;
    SelfQty = PathQty = Idx = 0;
    s1 = s2 = NULL;
    TheRel = NULL;

    XFS_CSAN ( RelPath )
    XFS_CAN ( self )
    XFS_CAN ( Path )
    XFS_CAN ( RelPath )


    /*) Apparently we shoud check that all parts of TheSelf
      | are also parts of ThePath
      (*/
    SelfQty = XFSPathPartCount ( self );
    PathQty = XFSPathPartCount ( Path );

        /* We could compare only absolute path ... because
         | we can not tell about relative path how they are
         | relative ... lol
         */
    if ( self -> is_absolute && Path -> is_absolute && 
        SelfQty <= PathQty ) {
        for ( Idx = 0; Idx < SelfQty; Idx ++ ) {
            s1 = XFSPathPartGet ( self, Idx );
            s2 = XFSPathPartGet ( Path, Idx );
            if ( s1 == NULL || s2 == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }

            if ( strcmp ( s1, s2 ) != 0 ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }

        if ( RCt == 0 ) {
            RetVal = true;

            if ( RelPath != NULL ) {
                RCt = XFSPathFrom (
                                Path,
                                SelfQty,
                                & TheRel
                                );
                if ( RCt == 0 ) {
                    * RelPath = TheRel;
                }
                else {
                    if ( TheRel != NULL ) {
                        _PathDispose ( TheRel );
                    }
                }
            }
        }
    }

    return RCt == 0 ? RetVal : false;
}   /* XFSPathIsChild () */

LIB_EXPORT
bool CC
XFSPathSIsChild (
            const char * self,
            const char * Path,
            const struct XFSPath ** RelPath
)
{
    rc_t RCt;
    bool RetVal;
    const struct XFSPath * TheSelf;
    const struct XFSPath * ThePath;

    RCt = 0;
    RetVal = false;
    ThePath = NULL;

    XFS_CSAN ( RelPath )
    XFS_CAN ( self )
    XFS_CAN ( Path )

    RCt = XFSPathMake ( & TheSelf, true, self );
    if ( RCt == 0 ) {
        RCt = XFSPathMake ( & ThePath, true, Path );
        if ( RCt == 0 ) {
            RetVal = XFSPathIsChild ( TheSelf, ThePath, RelPath );

            XFSPathRelease ( ThePath );
        }

        XFSPathRelease ( TheSelf );
    }

    return RCt == 0 ? RetVal : false;
}   /* XFSPathSIsChild () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
void CC
XFSPathDump ( const struct XFSPath * self )
{
    size_t Idx = 0;

    if ( self == NULL ) {
        LogMsg ( klogDebug, "  [Path] NULL" );
        return;
    }

    pLogMsg ( klogDebug, "  [Path] [$(orig)]", "orig=%s", self -> orig );
    pLogMsg ( klogDebug, "   [abs] [$(abs)]", "abs=%s", self -> is_absolute ? "yes" : "no" );
    pLogMsg ( klogDebug, "  [path] [$(path)]", "path=%s", self -> path );

    pLogMsg ( klogDebug, "   [qty] [$(qty)]", "qty=%ld", self -> tokens -> q );
    for ( Idx = 0; Idx < self -> tokens ->q; Idx ++ ) {
        pLogMsg ( klogDebug, "        [$(idx)] [$(token)]", "idx=%ld,token=%s", Idx, self -> tokens -> e [ Idx ] );
    }

}   /* XFSPathDump () */
