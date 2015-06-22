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
#include <klib/printf.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>

#include <va_copy.h>

#include "lreader.h"
#include "schwarzschraube.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    Struct itself
 ((*/
struct XFSLineReader {
    KRefcount refcount;

    const struct KMMap * map;

    size_t size;
    const char * start;
    const char * curr_line;
    size_t curr_len;
    size_t curr_no;
};

static const char * _sXFSLineReader_classname = "XFSLineReader";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) Kinda basic method
 ((*/
static
rc_t CC
_LineLen ( const char * Begin, const char * End, size_t * Len )
{
    const char * Pos = Begin;

    if ( Len != NULL ) {
        * Len = 0;
    }

    if ( Pos == NULL || End == NULL || Len == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( End <= Pos ) {
        return XFS_RC ( rcInvalid );
    }

    while ( Pos < End ) {
        if ( * Pos == '\n' ) {
            break;
        }

        Pos ++;
    }

    * Len = Pos - Begin;

    return 0;
}   /* _LineLen () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_LineReaderDispose ( const struct XFSLineReader * self )
{
    struct XFSLineReader * Reader = ( struct XFSLineReader * ) self;

    if ( Reader != NULL ) {
        KRefcountWhack (
                    & ( Reader -> refcount ),
                    _sXFSLineReader_classname
                    );

        if ( Reader -> map != NULL ) {
            KMMapRelease ( Reader -> map );
            Reader -> map = NULL;
        }

        Reader -> size = 0;
        Reader -> start = Reader -> curr_line = 0;
        Reader -> curr_len = Reader -> curr_no = 0;

        free ( Reader );
    }

    return 0;
}   /* XFSLineReaderDispose () */

LIB_EXPORT
rc_t CC
XFSLineReaderOpen (
                const struct XFSLineReader ** Reader,
                const char * Path,
                ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Path );

    RCt = XFSLineReaderVOpen ( Reader, Path, Args );

    va_end ( Args );

    return RCt;
}   /* XFSLineReaderOpen () */

LIB_EXPORT
rc_t CC
XFSLineReaderVOpen (
                const struct XFSLineReader ** Reader,
                const char * Path,
                va_list Args
)
{
    rc_t RCt;
    struct KDirectory * NatDir;
    const struct KFile * File;
    va_list xArgs;

    RCt = 0;
    NatDir = NULL;
    File = NULL;

    XFS_CSAN ( Reader )
    XFS_CAN ( Path )
    XFS_CAN ( Reader )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = KDirectoryVOpenFileRead ( NatDir, & File, Path, xArgs );
        va_end ( xArgs );
        if ( RCt == 0 ) {

            RCt = XFSLineReaderMake ( File, Reader );

            KFileRelease ( File );  /* Will release file here */
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSLineReaderVOpen () */

LIB_EXPORT
rc_t CC
XFSLineReaderMake (
                const struct KFile * File,
                const struct XFSLineReader ** Reader
)
{
    rc_t RCt;
    struct XFSLineReader * RetVal;
    const struct KMMap * Map;
    size_t MapSize, LineLen;
    const void * Addr;

    RCt = 0;
    RetVal = NULL;
    Map = NULL;
    MapSize = LineLen = 0;
    Addr = NULL;

    XFS_CSAN ( Reader )
    XFS_CAN ( File )
    XFS_CAN ( Reader )

    RetVal = calloc ( 1, sizeof ( struct XFSLineReader ) );
    if ( RetVal == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KMMapMakeRead ( & Map, File );
    if ( RCt == 0 ) {
        RCt = KMMapSize ( Map, & MapSize );
        if ( RCt == 0 ) {
            if ( 0 < MapSize ) {
                RCt = KMMapAddrRead ( Map, & Addr );
            }
            else {
                Addr = NULL;
            }

            if ( RCt == 0 ) {
                KRefcountInit (
                            & ( RetVal -> refcount ),
                            1,
                            _sXFSLineReader_classname,
                            "LineReaderMake",
                            "LineReader"
                            );


                RetVal -> map = Map;
                RetVal -> size = MapSize;
                RetVal -> start = ( const char * ) Addr;

                XFSLineReaderRewind ( RetVal );

                * Reader = RetVal;
            }
        }
    }

    if ( RCt != 0 ) {
        * Reader = NULL;

        if ( RetVal != NULL ) {
            _LineReaderDispose ( RetVal );
        }
    }

    return RCt;
}   /* XFSLineReaderMake () */

LIB_EXPORT
rc_t CC
XFSLineReaderAddRef ( const struct XFSLineReader * self )
{
    rc_t RCt;
    struct XFSLineReader * Reader;
    int Refc;

    RCt = 0;
    Reader = ( struct XFSLineReader * ) self;
    Refc = 0;

    if ( Reader == NULL ) {
        return XFS_RC ( rcNull );
    }

    Refc = KRefcountAdd (
                        & ( Reader -> refcount ),
                        _sXFSLineReader_classname
                        );
    switch ( Refc ) {
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
}   /* XFSLineReaderAddRef () */

LIB_EXPORT
rc_t CC
XFSLineReaderRelease ( const struct XFSLineReader * self )
{
    rc_t RCt;
    int Refc;
    struct XFSLineReader * Reader;

    RCt = 0;
    Refc = 0;
    Reader = ( struct XFSLineReader * ) self;

    if ( Reader == NULL ) {
        return XFS_RC ( rcNull );
    }

    Refc = KRefcountDrop (
                        & ( Reader -> refcount ),
                        _sXFSLineReader_classname
                        );
    switch ( Refc ) {
        case krefOkay :
        case krefZero :
                    RCt = 0;
                    break;
        case krefWhack :
                    RCt = _LineReaderDispose ( self );
                    break;
        case krefNegative :
                    RCt = XFS_RC ( rcInvalid );
                    break;
        default :
                    RCt = XFS_RC ( rcUnknown );
                    break;
    }

    return RCt;
}   /* XFSLineReaderRelease () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
bool CC
XFSLineReaderGood ( const struct XFSLineReader * self )
{
    if ( self != NULL ) {
        if ( self -> start != NULL ) {
            return true;
        }
    }
    return false;
}   /* XFSLineReaderGood () */

LIB_EXPORT
bool CC
XFSLineReaderNext ( const struct XFSLineReader * self )
{
    const char * NewLineStart, * LineEnd;
    size_t NewLineLen;
    struct XFSLineReader * Reader;

    NewLineStart = LineEnd = NULL;
    NewLineLen = 0;
    Reader = ( struct XFSLineReader * ) self;

    if ( ! XFSLineReaderGood ( self ) ) {
        return false;
    }

    LineEnd = self -> start + self -> size;
    NewLineStart = self -> curr_line + Reader -> curr_len + 1;

    if ( NewLineStart < LineEnd ) {
        if ( _LineLen ( NewLineStart, LineEnd, & NewLineLen ) == 0 ) {
            Reader -> curr_line = NewLineStart;
            Reader -> curr_len = NewLineLen;
            Reader -> curr_no ++;

            return true;
        }
    }

    return false;
}   /* XFSLineReaderNext () */

LIB_EXPORT
rc_t CC
XFSLineReaderGet (
            const struct XFSLineReader * self,
            const struct String * OutLine
)
{
    struct String * TheOutLine = ( struct String * ) OutLine;

    XFS_CAN ( self )
    XFS_CAN ( OutLine )

    if ( ! XFSLineReaderGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    StringInit (
                TheOutLine,
                self -> curr_line,
                self -> curr_len,
                self -> curr_len
                );

    return 0;
}   /* XFSLineReaderGet () */

LIB_EXPORT
rc_t CC
XFSLineReaderLineNo (
            const struct XFSLineReader * self,
            size_t * LineNo
)
{
    XFS_CSA ( LineNo, 0 )
    XFS_CAN ( self )
    XFS_CAN ( LineNo )

    if ( ! XFSLineReaderGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    * LineNo = self -> curr_no;

    return 0;
}   /* XFSLineReaderLineNo () */

LIB_EXPORT
rc_t CC
XFSLineReaderRewind ( const struct XFSLineReader * self )
{
    rc_t RCt;
    size_t Len;
    struct XFSLineReader * Reader;

    RCt = 0;
    Len = 0;
    Reader = ( struct XFSLineReader * ) self;

    XFS_CAN ( self );

    if ( ! XFSLineReaderGood ( self ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _LineLen (
                    self -> start,
                    self -> start + self -> size,
                    & Len
                    );
    if ( RCt == 0 ) {
        Reader -> curr_line = self -> start;
        Reader -> curr_len = Len;
        Reader -> curr_no = 0;
    }

    return RCt;
}   /* XFSLineReaderRewind () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
