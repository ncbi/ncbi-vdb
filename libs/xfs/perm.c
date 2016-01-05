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
#include <klib/text.h>

#include <xfs/perm.h>

#include "zehr.h"

#include <sysalloc.h>

#include <ctype.h>  /* isspace () */
#include <os-native.h>


/*)))
 |||
 +++    XFSPerm and all about
 |||
(((*/

static char _sUserDefault [ XFS_SIZE_128 ];
static char _sGroupDefault [ XFS_SIZE_128 ];
static char _sOtherDefault [ XFS_SIZE_128 ];

/*))))))
 //////  XFSAuth
((((((*/

/*))
 ((     Structures itself
  ))
 ((*/
struct XFSAuth {
    char * Name;
    bool NameDefaulted;
    bool CanRead;
    bool CanWrite;
    bool CanExecute;
};

static
rc_t CC
_AuthDispose ( const struct XFSAuth * self )
{
    struct XFSAuth * Auth = ( struct XFSAuth * ) self;

    if ( Auth != NULL ) {
        if ( ! Auth -> NameDefaulted ) {
            if ( Auth -> Name != NULL ) {
                free ( Auth -> Name );
            }
        }
        else {
            Auth -> Name = NULL;
        }

        free ( Auth );
    }

    return 0;
}   /* _AuthDispose () */

static
rc_t CC
_AuthMake (
            const char * Name,
            bool NameDefaulted,
            bool CanRead,
            bool CanWrite,
            bool CanExecute,
            const struct XFSAuth ** Auth
)
{
    struct XFSAuth * TheAuth;

    if ( Name == NULL || Auth == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Auth = NULL;

    TheAuth = calloc ( 1, sizeof ( struct XFSAuth ) );
    if ( TheAuth == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( ! NameDefaulted ) {
        TheAuth -> Name = string_dup_measure ( Name, NULL );
        if ( TheAuth -> Name == NULL ) {
            _AuthDispose ( TheAuth );

            return XFS_RC ( rcExhausted );
        }
    }
    else {
        TheAuth -> Name = ( char * ) Name;
    }

    TheAuth -> NameDefaulted = NameDefaulted;
    TheAuth -> CanRead = CanRead;
    TheAuth -> CanWrite = CanWrite;
    TheAuth -> CanExecute = CanExecute;

    * Auth = TheAuth;

    return 0;
}   /* _AuthMake () */

/*))))))
 //////  XFSPerm
((((((*/
/*))
 ((     Structures itself : now it is just a three pointers, but
  ))    I will make BStree in the case of something complex
 ((*/
struct XFSPerm {
    const struct XFSAuth * U;
    const struct XFSAuth * G;
    const struct XFSAuth * O;
};

/*))
 ((     Here we are.
  ))
 ((*/
static
const char * CC
_Perm_SkipTrail ( const char * Start )
{
    if ( Start != NULL ) {
        while ( true ) {
            if ( * Start == 0 ) {
                return NULL;
            }
            if ( ! isspace ( * Start ) ) {
                return Start;
            }

            Start ++;
        }
    }

    return NULL;
}   /* _Perm_SkipTrail () */

static
rc_t CC
_Perm_ReadValue (
                const char * Start,
                const char ** Next,
                char ** Value
)
{
    const char * End;

    if ( Value == NULL || Next == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Next = NULL;
    * Value = NULL;

    if ( Start == NULL ) {
        return 0;
    }

    End = Start;

    while ( * End != 0 && * End != ':' ) { End ++; }

    if ( 0 == End - Start ) {
        if ( * End == ':' ) {
            * Next = End + 1;
        }
        return 0;
    }

    * Value = string_dup ( Start, End - Start );
    if ( * Value == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( * End == ':' ) {
        * Next = End + 1;
    }

    return 0;
}   /* _Perm_ReadString () */

static
rc_t CC
_Perm_StringParse (
            const char * PermAsString,
            char ** Perm,
            char ** User,
            char ** Group,
            char ** Other
)
{
    rc_t RCt;
    const char * pStart, * pEnd;
    char ch;
    int llp;
    const int _PLN = 9;

    RCt = 0;
    pStart = pEnd = NULL;
    ch = 0;
    llp = 0;

        /*  We should get at least permissions
         */
    if ( PermAsString == NULL || Perm == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Perm = NULL;

    if ( User != NULL )  { * User = NULL;  }
    if ( Group != NULL ) { * Group = NULL; }
    if ( Other != NULL ) { * Other = NULL; }

    pStart = ( char * ) PermAsString;


    pStart = _Perm_SkipTrail ( pStart );
    if ( pStart == NULL ) {
        return XFS_RC ( rcInvalid );
    }

        /*  second we are reading permissions 9 characters;
         */
    for ( llp = 0; llp < _PLN ; llp ++ ) {
        ch = * ( pStart + llp );
        if ( ch == 0 ) {
            return XFS_RC ( rcInvalid );
        }

        if ( ch != '-' ) {
            if ( llp % 3 == 0 && ch != 'r' ) {
                return XFS_RC ( rcInvalid );
            }
            if ( llp % 3 == 1 && ch != 'w' ) {
                return XFS_RC ( rcInvalid );
            }
            if ( llp % 3 == 2 && ch != 'x' ) {
                return XFS_RC ( rcInvalid );
            }
        }
    }

    * Perm = string_dup ( pStart, _PLN );
    if ( * Perm == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    pStart += _PLN;

        /*  skipping to the next entry
         */
    pStart = _Perm_SkipTrail ( pStart );
    if ( pStart == NULL ) {
        return 0;
    }

        /*  third we are reading user
         */
    RCt = _Perm_ReadValue ( pStart, & pEnd, User );
    if ( RCt == 0 && pEnd != NULL ) {
        RCt = _Perm_ReadValue ( pEnd, & pStart, Group );
        if ( RCt == 0 && pStart != 0 ) {
            RCt = _Perm_ReadValue ( pStart, & pEnd, Other );
        }
    }

    return RCt;
}   /* _Perm_StringParse () */

static
rc_t CC
_MakeAuth (
        XFSAType Type,
        const char * Name,
        const char * Perm,
        const struct XFSAuth ** Auth
)
{
    const char * RName;
    bool DF, CR, CW, CE;

    RName = NULL;
    DF = CR = CW = CE = false;

    if ( Auth == NULL || Perm == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Auth = NULL;

    DF = Name == NULL;
    RName = DF ? XFSPermDefaultName ( Type ) : Name;

    CR = * ( Perm + ( Type * 3 ) + 0 ) == 'r';
    CW = * ( Perm + ( Type * 3 ) + 1 ) == 'w';
    CE = * ( Perm + ( Type * 3 ) + 2 ) == 'x';

    return _AuthMake ( RName, DF, CR, CW, CE, Auth );
}   /* _MakeAuth () */

LIB_EXPORT
rc_t CC
XFSPermMake (
            const char * PermAsString,
            const struct XFSPerm ** Permissions
)
{
    rc_t RCt;
    struct XFSPerm * Perm;
    char * SPerm, * User, * Group, * Other;

    RCt = 0;
    Perm = NULL;
    SPerm = User = Group = Other = NULL;

    if ( PermAsString == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    Perm = calloc ( 1, sizeof ( struct XFSPerm ) );
    if ( Perm != NULL ) {
        RCt = _Perm_StringParse (
                            PermAsString,
                            & SPerm,
                            & User,
                            & Group,
                            & Other
                        );
        if ( RCt == 0 ) {
            RCt = _MakeAuth ( kxfsUser, User, SPerm, & ( Perm -> U ) );
            if ( RCt == 0 ) {
                RCt = _MakeAuth (
                                kxfsGroup,
                                Group,
                                SPerm,
                                & ( Perm -> G )
                                );
                if ( RCt == 0 ) {
                    RCt = _MakeAuth ( 
                                kxfsOther,
                                Other,
                                SPerm,
                                & ( Perm -> O )
                                );
                }
            }
        }
    }
    else {
        RCt = XFS_RC ( rcExhausted );
    }

    if ( SPerm != NULL ) {
        free ( SPerm );
    }
    if ( User != NULL ) {
        free ( User );
    }
    if ( Group != NULL ) {
        free ( Group );
    }
    if ( Other != NULL ) {
        free ( Other );
    }

    if ( RCt != 0 ) {
        XFSPermDispose ( Perm );
    }
    else {
        * Permissions = Perm;
    }

    return RCt;
}   /* XFSPermMake () */

LIB_EXPORT
rc_t CC
XFSPermDispose ( const struct XFSPerm * self )
{
    rc_t RCt;
    struct XFSPerm * Perm;

    RCt = 0;

    Perm = ( struct XFSPerm * ) self;

    if ( Perm != NULL ) {
        if ( Perm -> U != NULL ) {
            _AuthDispose ( Perm -> U );
            Perm -> U = NULL;
        }

        if ( Perm -> G != NULL ) {
            _AuthDispose ( Perm -> G );
            Perm -> G = NULL;
        }

        if ( Perm -> O != NULL ) {
            _AuthDispose ( Perm -> O );
            Perm -> O = NULL;
        }

        free ( Perm );
    }

    return RCt;
}   /* XFSPermDispose () */

/*)))
 ///    Defaults
(((*/
static
rc_t CC
_Perm_SetDefault (
                const char * Default,
                char * Buffer,
                size_t BufferSize
)
{
    size_t CYP;

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    if ( Default == NULL ) {
        * Buffer = 0;
        return 0;
    }

    CYP = string_size ( Default );

    if ( BufferSize <= CYP ) {
        return XFS_RC ( rcTooBig );
    }

        /* he-he ... should we check if it happens ? */
    string_copy ( Buffer, BufferSize, Default, CYP );

    return 0;
}   /* _Perm_SetDefault () */

LIB_EXPORT
const struct XFSAuth * CC
XFSPermAuth ( const struct XFSPerm * self, XFSAType Type )
{
    if ( self != NULL ) {
        switch ( Type ) {
            case kxfsUser  : return self -> U;
            case kxfsGroup : return self -> G;
            case kxfsOther : return self -> O;
        }
    }
    return NULL;
}   /* XFSPermAuth () */

LIB_EXPORT
rc_t CC
XFSPermToString (
            const struct XFSPerm * self,
            char * Buffer,
            size_t BufferSize
)
{
    if ( self == NULL || Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }
    * Buffer = 0;

    if ( self -> U == NULL || self -> G == NULL || self -> O == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    strcat ( Buffer, ( self -> U -> CanRead )    ? "r" : "-" );
    strcat ( Buffer, ( self -> U -> CanWrite )   ? "w" : "-" );
    strcat ( Buffer, ( self -> U -> CanExecute ) ? "x" : "-" );
    strcat ( Buffer, ( self -> G -> CanRead )    ? "r" : "-" );
    strcat ( Buffer, ( self -> G -> CanWrite )   ? "w" : "-" );
    strcat ( Buffer, ( self -> G -> CanExecute ) ? "x" : "-" );
    strcat ( Buffer, ( self -> O -> CanRead )    ? "r" : "-" );
    strcat ( Buffer, ( self -> O -> CanWrite )   ? "w" : "-" );
    strcat ( Buffer, ( self -> O -> CanExecute ) ? "x" : "-" );

    if (    ! self -> U -> NameDefaulted
        ||  ! self -> G -> NameDefaulted
        ||  ! self -> O -> NameDefaulted ) {

        strcat ( Buffer, " " );

        if ( ! self -> U -> NameDefaulted ) {
            strcat ( Buffer, self -> U -> Name );
        }

        if (    ! self -> G -> NameDefaulted
            ||  ! self -> O -> NameDefaulted ) {
            strcat ( Buffer, ":" );
        }

        if ( ! self -> G -> NameDefaulted ) {
            strcat ( Buffer, self -> G -> Name );
        }

        if ( ! self -> O -> NameDefaulted ) {
            strcat ( Buffer, ":" );

            strcat ( Buffer, self -> O -> Name );
        }
    }

    return 0;
}   /* XFSPermToString () */

LIB_EXPORT
const char * CC
XFSPermDefaultName ( XFSAType Type )
{
    switch ( Type ) {
        case kxfsUser :  return _sUserDefault;
        case kxfsGroup : return _sGroupDefault;
        case kxfsOther : return _sOtherDefault;
    }
    return NULL;
}   /* XFSPermDefaultName () */

LIB_EXPORT
rc_t CC
XFSPermSetDefaultName ( XFSAType Type, const char * Name )
{
    switch ( Type ) {
        case kxfsUser :
                return _Perm_SetDefault (
                                        Name,
                                        _sUserDefault,
                                        sizeof ( _sUserDefault )
                                        );
        case kxfsGroup :
                return _Perm_SetDefault (
                                        Name,
                                        _sGroupDefault,
                                        sizeof ( _sGroupDefault )
                                        );
        case kxfsOther :
                return _Perm_SetDefault (
                                        Name,
                                        _sOtherDefault,
                                        sizeof ( _sOtherDefault )
                                        );
    }
    return XFS_RC ( rcUnknown );
}   /* XFSPermSetDefaultName () */

LIB_EXPORT
const char * CC
XFSAuthName ( const struct XFSAuth * self )
{
    return self == NULL ? NULL : ( self -> Name );
}   /* XFSAuthName () */

LIB_EXPORT
bool CC
XFSAuthCanRead ( const struct XFSAuth * self )
{
    return self == NULL ? false : ( self -> CanRead );
}   /* XFSAuthCanRead () */

LIB_EXPORT
bool CC
XFSAuthCanWrite ( const struct XFSAuth * self )
{
    return self == NULL ? false : ( self -> CanWrite );
}   /* XFSAuthCanWrite () */

LIB_EXPORT
bool CC
XFSAuthCanExecute ( const struct XFSAuth * self )
{
    return self == NULL ? false : ( self -> CanExecute );
}   /* XFSAuthCanExecute () */

/*))))))
 //////  Misc
((((((*/

LIB_EXPORT
rc_t CC
XFSPermToNum ( const char * Perm, uint32_t * Num )
{
    rc_t RCt;
    uint32_t xNum;
    const struct XFSPerm * xPerm;
    const struct XFSAuth * xAuth;

    RCt = 0;
    xNum = 0;
    xPerm = NULL;
    xAuth = NULL;

    if ( Num == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Num = 0;

    if ( Perm == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPermMake ( Perm, & xPerm );
    if ( RCt == 0 ) {
        xAuth = XFSPermAuth ( xPerm, kxfsUser );
        if ( xAuth != NULL ) {
            if ( XFSAuthCanRead    ( xAuth ) ) { xNum |= 0400; }
            if ( XFSAuthCanWrite   ( xAuth ) ) { xNum |= 0200; }
            if ( XFSAuthCanExecute ( xAuth ) ) { xNum |= 0100; }
        }

        xAuth = XFSPermAuth ( xPerm, kxfsGroup );
        if ( xAuth != NULL ) {
            if ( XFSAuthCanRead    ( xAuth ) ) { xNum |= 040; }
            if ( XFSAuthCanWrite   ( xAuth ) ) { xNum |= 020; }
            if ( XFSAuthCanExecute ( xAuth ) ) { xNum |= 010; }
        }

        xAuth = XFSPermAuth ( xPerm, kxfsOther );
        if ( xAuth != NULL ) {
            if ( XFSAuthCanRead    ( xAuth ) ) { xNum |= 04; }
            if ( XFSAuthCanWrite   ( xAuth ) ) { xNum |= 02; }
            if ( XFSAuthCanExecute ( xAuth ) ) { xNum |= 01; }
        }

        * Num = xNum;

        XFSPermDispose ( xPerm );
    }

    return RCt;
}   /* XFSPermToNum () */


LIB_EXPORT
rc_t CC
XFSPermToChar ( uint32_t Num, char * Buf, size_t BufSize )
{
    rc_t RCt;

    RCt = 0;

    if ( Buf == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( BufSize <= 10 ) {
        return XFS_RC ( rcInvalid );
    }

    * Buf = 0;

        /* owner */
    Buf [ 0 ] = ( Num & 0400 ) == 0400 ? 'r' : '-';
    Buf [ 1 ] = ( Num & 0200 ) == 0200 ? 'w' : '-';
    Buf [ 2 ] = ( Num & 0100 ) == 0100 ? 'x' : '-';
        /* group */
    Buf [ 3 ] = ( Num & 040 ) == 040 ? 'r' : '-';
    Buf [ 4 ] = ( Num & 020 ) == 020 ? 'w' : '-';
    Buf [ 5 ] = ( Num & 010 ) == 010 ? 'x' : '-';
        /* others */
    Buf [ 6 ] = ( Num & 04 ) == 04 ? 'r' : '-';
    Buf [ 7 ] = ( Num & 02 ) == 02 ? 'w' : '-';
    Buf [ 8 ] = ( Num & 01 ) == 01 ? 'x' : '-';

        /* internet users */
    Buf [ 9 ] = 0;

    return RCt;
}   /* XFSPermToChar () */

/*))))))
 //////  Defaults ... sorry, hardcoding those
((((((*/
static const char * _DefROPermForContainer = "r-xr-xr-x";
static const char * _DefROPermForNode = "r--r--r--";
static uint32_t _DefROPermForContainerNum = 0;
static uint32_t _DefROPermForNodeNum = 0;

LIB_EXPORT
const char * CC
XFSPermRODefContChar ()
{
    return _DefROPermForContainer;
}   /* XFSPermRODefContChar () */

LIB_EXPORT
const char * CC
XFSPermRODefNodeChar ()
{
    return _DefROPermForNode;
}   /* XFSPermRODefNodeChar () */

LIB_EXPORT
uint32_t CC
XFSPermRODefContNum ()
{
    uint32_t T;

    if ( _DefROPermForContainerNum == 0 ) {
        XFSPermToNum ( XFSPermRODefContChar (), & T );
        _DefROPermForContainerNum = T;
    }

    return _DefROPermForContainerNum;
}   /* XFSPermRODefContNum () */

LIB_EXPORT
uint32_t CC
XFSPermRODefNodeNum ()
{
    uint32_t T;

    if ( _DefROPermForNodeNum == 0 ) {
        XFSPermToNum ( XFSPermRODefNodeChar (), & T );
        _DefROPermForNodeNum = T;
    }

    return _DefROPermForNodeNum;
}   /* XFSPermRODefNodeNum () */

/*))))))
 //////  Defaults ... sorry, hardcoding those
((((((*/
static const char * _DefRWPermForContainer = "rwxr-xr-x";
static const char * _DefRWPermForNode = "rw-r--r--";
static uint32_t _DefRWPermForContainerNum = 0;
static uint32_t _DefRWPermForNodeNum = 0;

LIB_EXPORT
const char * CC
XFSPermRWDefContChar ()
{
    return _DefRWPermForContainer;
}   /* XFSPermRWDefContChar () */

LIB_EXPORT
const char * CC
XFSPermRWDefNodeChar ()
{
    return _DefRWPermForNode;
}   /* XFSPermRWDefNodeChar () */

LIB_EXPORT
uint32_t CC
XFSPermRWDefContNum ()
{
    uint32_t T;

    if ( _DefRWPermForContainerNum == 0 ) {
        XFSPermToNum ( XFSPermRWDefContChar (), & T );
        _DefRWPermForContainerNum = T;
    }

    return _DefRWPermForContainerNum;
}   /* XFSPermRWDefContNum () */

LIB_EXPORT
uint32_t CC
XFSPermRWDefNodeNum ()
{
    uint32_t T;

    if ( _DefRWPermForNodeNum == 0 ) {
        XFSPermToNum ( XFSPermRWDefNodeChar (), & T );
        _DefRWPermForNodeNum = T;
    }

    return _DefRWPermForNodeNum;
}   /* XFSPermRWDefNodeNum () */
