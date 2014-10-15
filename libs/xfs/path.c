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
#include <klib/namelist.h>
#include <vfs/path.h>

#include <xfs/path.h>

#include "schwarzschraube.h"

#include <sysalloc.h>

#include <string.h>
#include <stdio.h>


/*)))
 |||
 +++    Three and all the worst
 |||
(((*/
static const char * _sPathSeparator    = "/";

struct XFSPath {
    const struct KNamelist * Path;

    const char * Origin;    /* Don't know why? ... may be union */
};

/*))
 ((     Path make/dispose. Note, we do initialize/destroy node name
  ))    here, but Constructor could redefine it.
 ((*/
static
rc_t CC
_AddToken (
        struct VNamelist * List,
        const char * Begin,
        const char * End
)
{
    size_t Len;
    char Buf [ XFS_SIZE_512 ];

    Len = 0;

    if ( List == NULL || Begin == NULL || End == NULL ) {
        return XFS_RC ( rcNull );
    }

    Len = End - Begin;

    if ( sizeof ( Buf ) <= Len ) {
        return XFS_RC ( rcUnexpected );
    }

    if ( Len == 0 ) {
        return 0;
    }

    if ( string_copy ( Buf, sizeof ( Buf ), Begin, Len ) != Len ) {
        return XFS_RC ( rcExhausted );
    }

    Buf [ Len ] = 0;

    return VNamelistAppend ( List, Buf );
}   /* _AddToken () */

static
rc_t CC
_ParsePath ( const char * InPath, struct KNamelist ** List )
{
    rc_t RCt;
    VNamelist * TheList;
    char * pB, * pN;

    RCt = 0;
    TheList = NULL;
    pB = pN = NULL;

        /* Here and further no checks for NULL */

        /* All Path are absolute and starts from Separator */
    if ( * InPath != * _sPathSeparator ) {
        return XFS_RC ( rcUnsupported );
    }

    RCt = VNamelistMake ( & TheList, 16 /* Hu-hu */ );
    if ( RCt == 0 ) {

            /* Not sure if we should */
        RCt = VNamelistAppend ( TheList, _sPathSeparator );
        if ( RCt == 0 ) {

            pB = ( char * ) InPath;
            pN = pB;

            while ( * pN != 0 ) {
                if ( * pN == * _sPathSeparator ) {
                        /* Adding token as is */
                    RCt = _AddToken ( TheList, pB, pN );
                    if ( RCt != 0 ) {
                        break;
                    }

                        /* Skipping all separators */
                    while ( * pN == * _sPathSeparator ) {
                        pN ++;
                    }

                    pB = pN;

                    if ( * pB == 0 ) {
                        break;
                    }
                }

                pN ++;
            }

            if ( RCt == 0 ) {
                RCt = _AddToken ( TheList, pB, pN );
                if ( RCt == 0 ) {
                    RCt = VNamelistToNamelist ( TheList, List );
                }
            }
        }

        VNamelistRelease ( TheList );
    }

    return RCt;
}   /* _ParsePath () */

LIB_EXPORT
rc_t CC
XFSPathMake ( const char * InPath, const struct XFSPath ** OutPath )
{
    rc_t RCt;
    struct XFSPath * Path;
    struct KNamelist * List;

    RCt = 0;
    Path = NULL;
    List = NULL;

    if ( InPath == NULL || OutPath == NULL ) {
        return XFS_RC ( rcNull );
    }

    * OutPath = NULL;

    Path = calloc ( 1, sizeof ( struct XFSPath ) );
    if ( Path == NULL ) {
        return XFS_RC ( rcExhausted );
    }
    else {
        RCt = _ParsePath ( InPath, & List );
        if ( RCt == 0 ) {
            Path -> Path = List;

            RCt = XFS_StrDup ( InPath, & ( Path -> Origin ) );
            if ( RCt == 0 ) {
                * OutPath = Path;
            }
        }
    }

    if ( RCt != 0 ) {
        XFSPathDispose ( Path );

        Path = NULL;

        * OutPath = NULL;
    }

    return RCt;
}   /* XFSPathMake () */

LIB_EXPORT
rc_t CC
XFSPathDispose ( const struct XFSPath * self )
{
    struct XFSPath * Path;

    Path = ( struct XFSPath * ) self;

    if ( Path == NULL ) {
        return 0;
    }

    if ( Path -> Path != NULL ) {
        KNamelistRelease ( Path -> Path );

        Path -> Path = NULL;
    }

    if ( Path -> Origin != NULL ) {
        free ( ( char * ) Path -> Origin );

        Path -> Origin = NULL;
    }

    free ( Path );

    return 0;
}   /* XFSPathDispose () */

LIB_EXPORT
uint32_t CC
XFSPathCount ( const struct XFSPath * self )
{
    uint32_t Count = 0;

    if ( self != NULL ) {
        if ( self -> Path != NULL ) {
            if ( KNamelistCount ( self -> Path, & Count ) == 0 ) {
                return Count;
            }
        }
    }
    return 0;
}   /* XFSPathCount () */

LIB_EXPORT
const char * CC
XFSPathGet ( const struct XFSPath * self, uint32_t Index )
{
    uint32_t Count;
    const char * RetVal;

    RetVal = NULL;

    if ( self != NULL ) {
        if ( self -> Path != NULL ) {
            Count = XFSPathCount ( self );
            if ( Index < Count ) {
                if ( KNamelistGet ( self -> Path, Index, & RetVal ) == 0 ) {
                    return RetVal;
                }
            }
        }
    }

    return NULL;
}   /* XFSPathGet () */

LIB_EXPORT
const char * CC
XFSPathName ( const struct XFSPath * self )
{
    return XFSPathGet ( self, XFSPathCount ( self ) - 1 );
}   /* XFSPathName () */

LIB_EXPORT
const char * CC
XFSPathGetOrigin ( const struct XFSPath * self )
{
    if ( self != NULL ) {
        if ( self -> Path != NULL ) {
            return self -> Origin;
        }
    }

    return NULL;
}   /* XFSPathGetOrigin () */

/*))
 //     if 'To == 0' it will be treated as largest path item
((*/
static
rc_t CC
_CompilePath (
        const struct XFSPath * self,
        uint32_t From,
        uint32_t To,
        char * Buffer,
        size_t BufferSize
)
{
    uint32_t Count;
    const char * pS;

    Count = 0;

    if ( self == NULL || Buffer == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    * Buffer = 0;

    Count = XFSPathCount ( self );

    if ( To == ~0 ) {
        To = Count;
    }

    if ( Count < From || Count < To || To < From ) {
        return XFS_RC ( rcInvalid );
    }

    pS = XFSPathGet ( self, 0 );

        /*) Here we are 
         (*/
    * Buffer = 0;
    for ( Count = From; Count < To; Count ++ ) {
        if ( Count != From && 1 < Count ) {
            strcat ( Buffer, pS );
        }
        strcat ( Buffer, XFSPathGet ( self, Count ) );
    }

    return 0;
}   /* _CompilePath () */

LIB_EXPORT
rc_t CC
XFSPathTo (
        const struct XFSPath * self,
        uint32_t Index,
        char * Buffer,
        size_t BufferSize
)
{
    return _CompilePath ( self, 0, Index, Buffer, BufferSize );
}   /* XFSPathTo () */

LIB_EXPORT
rc_t CC
XFSPathFrom (
        const struct XFSPath * self,
        uint32_t Index,
        char * Buffer,
        size_t BufferSize
)
{
    return _CompilePath ( self, Index, ~0, Buffer, BufferSize );
}   /* XFSPathTo () */

LIB_EXPORT
rc_t CC
XFSPathDump ( const struct XFSPath * self )
{
    rc_t RCt = 0;
    uint32_t Count, llp;
    const char * Entry;

    if ( self == NULL ) {
        printf ( "XPTH [ NULL PATH ]\n" );
        return XFS_RC ( rcNull );
    }

    RCt = KNamelistCount ( self -> Path, & Count );
    if ( RCt == 0 ) {
        printf ( "XPTH [%s]\n     [", self -> Origin );
        for ( llp = 0; llp < Count; llp ++ ) {
            RCt = KNamelistGet ( self -> Path, llp, & Entry );
            if ( RCt != 0 ) {
                break;
            }

            printf ( " \"%s\"(%d)", Entry, llp );
        }

        if ( RCt == 0 ) { 
            printf ( " ]\n" );
        }
        else {
            printf ( " INVALID PATH! ]\n" );
        }
    }

    return RCt;
}   /* XFSPathDump () */
