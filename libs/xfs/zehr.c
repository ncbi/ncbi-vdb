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
#include <klib/container.h>
#include <klib/text.h>

#include <kfg/config.h>

#include <kfs/file.h>
#include <vfs/path.h>
#include <kns/manager.h>
#include <kns/http.h>
#include <kns/stream.h>

#include <krypto/key.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"

#include <sysalloc.h>

#include <ctype.h>
#include <os-native.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  Loading config. BTW, resource is a name of file or URL
 ((*/
LIB_EXPORT
rc_t CC
XFS_LoadConfig_ZHR (
                const char * Resource,
                const struct KConfig ** Config
)
{
    rc_t RCt;
    const struct KFile * File;
    struct KConfig * TmpConfig;

    RCt = 0;
    TmpConfig = NULL;
    File = NULL;

        /* Now Resource could be NULL
         */
    XFS_CSAN ( Config )
    XFS_CAN ( Config )

    RCt = KConfigMake ( & TmpConfig, NULL );
    if ( RCt != 0 ) {
        return RCt;
    }

    if ( Resource != NULL ) {
            /* Trying to open resource as file */
        RCt = XFS_OpenResourceRead_MHR ( Resource, & File );
        if ( RCt == 0 ) {
            RCt = KConfigLoadFile ( TmpConfig, Resource, File );

            KFileRelease ( File );
        }
    }

    if ( RCt == 0 ) {
        * Config = TmpConfig;
    }
    else {
        if ( TmpConfig != NULL ) {
            KConfigRelease ( TmpConfig );
        }
    }

    return RCt;
}   /* XFS_LoadConfig_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //  Simple string tokenizer
((*/

static
rc_t CC
_AddTrimStringToList_ZHR ( struct VNamelist * List, char * String )
{
        /*))
         ((    We suppose that String is null terminated and editable
          ))   ... i.e. it could be called by Tokenizer only
         ((*/
    char * BG, * EN;

    if ( String == NULL ) {
        return 0;
    }

    if ( List == NULL ) {
        return XFS_RC ( rcNull );
    }

    BG = String;
    EN = BG + strlen ( String );

        /*) Trim from front (*/
    while ( BG < EN ) {
        if ( isspace ( * BG ) ) {
            BG ++;
        }
        else {
            break;
        }
    }

        /*) Trim from end (*/
    while ( BG < EN ) {
        if ( isspace ( * EN ) || * EN == 0 ) {
            * EN = 0;
            EN --;
        }
        else {
            break;
        }
    }

    if ( 0 < EN - BG ) {
        return VNamelistAppend ( List, BG );
    }

    return 0;
}   /* _AddTrimStringToList_ZHR () */

LIB_EXPORT
rc_t CC
XFS_SimpleTokenize_ZHR (
                    const char * SimpleString,
                    char Separator,
                    struct KNamelist ** Tokens
)
{
    rc_t RCt;
    struct VNamelist * List;
    char LN [ XFS_SIZE_1024 ], * LNEnd;
    const char * End;

    RCt = 0;
    List = NULL;
    LNEnd = NULL;
    End = NULL;

    if ( Tokens == NULL || SimpleString == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Tokens = NULL;

    RCt = VNamelistMake ( & List, 16 /* he-he */ );
    if ( RCt == 0 ) {
        End = SimpleString;
        LNEnd = LN;

        while ( * End != 0 ) {
            * LNEnd = * End;

            if ( * End == Separator ) {
                *LNEnd = 0;

                RCt = _AddTrimStringToList_ZHR ( List, LN );
                if ( RCt != 0 ) {
                    break;
                }

                LNEnd = LN;
                End ++;
            }
            else {
                End ++;
                LNEnd ++;
            }

        }

        if ( RCt == 0 ) {
            if ( LN != LNEnd ) {
                * LNEnd = 0;
                RCt = _AddTrimStringToList_ZHR ( List, LN );
            }
        }

        if ( RCt == 0 ) {
            RCt = VNamelistToNamelist ( List, Tokens );
        }

        VNamelistRelease ( List );
    }

    return RCt;
}   /* XFS_SimpleTokenize_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //  Simple string comparator
((*/
LIB_EXPORT
int CC
XFS_StringCompare4BST_ZHR ( const char * Str1, const char * Str2 )
{
    if ( Str1 == NULL || Str2 == NULL ) {
        if ( Str1 != NULL ) {
            return 4096;
        }
        
        if ( Str2 != NULL ) {
            return 4096 * - 1;
        }

        return 0;
    }

    return strcmp ( Str1, Str2 );
}   /* XFS_StringCompare4BST_XHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_NameExtEndPosFromPath (
                        const char * Path,
                        const char ** Name,
                        const char ** Ext,  /* Will be End if No */
                        const char ** End   /* for Name + Ext */
)
{
    const char * pStart;
    const char * pEnd;
    const char * pCurr;
    const char * pExt;

    pStart = pEnd = pCurr = pExt = NULL;

    XFS_CSAN ( Name )
    XFS_CSAN ( Ext )
    XFS_CSAN ( End )
    XFS_CAN ( Path )
    XFS_CAN ( Name )
    XFS_CAN ( Ext )
    XFS_CAN ( End )

    pStart = Path;
    pEnd = Path + string_size ( Path );
    pCurr = pEnd - 1;
    pExt = pEnd;

    if ( 0 == ( pEnd - pStart ) ) {
        return XFS_RC ( rcInvalid );
    }

        /*  Lookin' for name
         */
    while ( pStart < pCurr ) {
        if ( * pCurr == '/' ) {
            pStart = pCurr + 1;
            break;
        }

        pCurr --;
    }

        /*  Now lookin' for Extention
         */
    pCurr = pEnd - 1;
    while ( pStart < pCurr ) {
        if ( * pCurr == '.' ) {
            pExt = pCurr;
            break;
        }

        pCurr --;
    }

    * Name = pStart;
    * Ext = pExt;
    * End = pEnd;

    return 0;
}   /* _NameExtEndPosFromPath () */

/*))
 ||     Will return name with/without extension
 ||     It will return new string, so don't forget to delete it
((*/
LIB_EXPORT
rc_t CC
XFS_NameFromPath_ZHR (
                    const char * Path,
                    const char ** Name,
                    bool TrimExt
)
{
    rc_t RCt;
    const char * pName;
    const char * pExt;
    const char * pEnd;
    char * RetVal;

    RCt = 0;
    pName = pExt = pEnd = NULL;
    RetVal = NULL;

    XFS_CSAN ( Name )
    XFS_CAN ( Path )
    XFS_CAN ( Name )

    RCt = _NameExtEndPosFromPath ( Path, & pName, & pExt, & pEnd );
    if ( RCt == 0 ) {

        RetVal = string_dup (
                            pName,
                            ( TrimExt ? pExt : pEnd ) - pName
                            );

        if ( RetVal == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            * Name = RetVal;
        }
    }


    return RCt;
}   /* XFS_NameFromPath_ZHR () */

/*))
 ||     Will return null if Extension does not exists
 ||     It will return new strings, so don't forget to delete them
((*/
LIB_EXPORT
rc_t CC
XFS_NameExtFromPath_ZHR (
                    const char * Path,
                    const char ** Name,
                    const char ** Ext
)
{
    rc_t RCt;
    const char * pName;
    const char * pExt;
    const char * pEnd;
    char * RetName;
    char * RetExt;

    RCt = 0;
    pName = pExt = pEnd = NULL;
    RetName = RetExt = NULL;

    XFS_CSAN ( Name )
    XFS_CSAN ( Ext )
    XFS_CAN ( Path )
    XFS_CAN ( Name )
    XFS_CAN ( Ext )

    RCt = _NameExtEndPosFromPath ( Path, & pName, & pExt, & pEnd );
    if ( RCt == 0 ) {
        RetName = string_dup ( pName, pExt - pName );
        if ( RetName == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            if ( pExt != pEnd ) {

                RetExt = string_dup ( pExt, pEnd - pExt );
                if ( RetExt == NULL ) {
                    RCt = XFS_RC ( rcExhausted );
                }
            }
            if ( RCt == 0 ) {
                * Name = RetName;
                * Ext = RetExt;
            }
        }
    }

    if ( RCt != 0 ) {
        * Name = NULL;
        * Ext = NULL;

        if ( RetName != NULL ) {
            free ( RetName );
        }

        if ( RetExt != NULL ) {
            free ( RetExt );
        }
    }

    return RCt;
}   /* XFS_NameExtFromPath_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

 /*))))
   (((( That file contains methods, which are common for all modules
   )))) but I am not sure if they were implemented right or wrong.
   ((((*/
typedef rc_t ( CC * XFS_ReadV_ZHR ) (
                                const struct VPath * self,
                                char * buffer,
                                size_t buffer_size,
                                size_t * num_read
                                );

static
rc_t CC
XFS_ReadVEverything_ZHR (
                const struct VPath * Path,
                char * Buffer,
                size_t BufferSize,
                const char * Filler,
                XFS_ReadV_ZHR Reader
)
{
    rc_t RCt;
    size_t NR;
    const char * DefaultFiller = "NULL";

    RCt = 0;

    if ( Buffer == NULL || BufferSize <= 0 ) {
        return XFS_RC ( rcNull );
    }

    * Buffer = 0;

    if ( Path != NULL ) {
        RCt = Reader ( Path, Buffer, BufferSize, & NR );
    }
    else {
        RCt = XFS_RC ( rcNull );

        string_copy_measure (
                        Buffer,
                        sizeof ( Buffer ),
                        ( Filler == NULL ? DefaultFiller : Filler )
                        );
    }

    return RCt;
}   /* XFS_ReadVEverything_ZHR () */

static
rc_t CC
XFS_ReadCEverything_ZHR (
                const char * Url,
                char * Buffer,
                size_t BufferSize,
                const char * Filler,
                XFS_ReadV_ZHR Reader
)
{
    rc_t RCt;
    size_t NR;
    const char * DefaultFiller = "NULL";
    struct VPath * Path;

    RCt = 0;
    NR = 0;
    Path = NULL;

    if ( Buffer == NULL || BufferSize <= 0 ) {
        return XFS_RC ( rcNull );
    }

    * Buffer = 0;

    if ( Url == NULL ) {
        RCt = XFS_RC ( rcNull );

        string_copy_measure (
                        Buffer,
                        sizeof ( Buffer ),
                        ( Filler == NULL ? DefaultFiller : Filler )
                        );
    }
    else {
        RCt = VFSManagerMakePath ( XFS_VfsManager (), & Path, Url );
        if ( RCt == 0 ) {
            RCt = Reader ( Path, Buffer, BufferSize, & NR );

            VPathRelease ( Path );
        }
    }

    return RCt;
}   /* XFS_ReadCEverything_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadVPath_ZHR (
                const struct VPath * Path,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadVEverything_ZHR (
                                Path,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadPath
                                );
}   /* XFS_ReadVPath_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadCPath_ZHR (
                const char * Url,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadCEverything_ZHR (
                                Url,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadPath
                                );
}   /* XFS_ReadCPath_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadVUri_ZHR (
                const struct VPath * Path,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadVEverything_ZHR (
                                Path,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadUri
                                );
}   /* XFS_ReadVUri_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadCUri_ZHR (
                const char * Url,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadCEverything_ZHR (
                                Url,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadUri
                                );
}   /* XFS_ReadCUri_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadVHost_ZHR (
                const struct VPath * Path,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadVEverything_ZHR (
                                Path,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadHost
                                );
}   /* XFS_ReadVHost_ZHR () */

LIB_EXPORT
rc_t CC
XFS_ReadCHost_ZHR (
                const char * Url,
                char * Buffer,
                size_t BufferSize,
                const char * Filler
)
{
    return XFS_ReadCEverything_ZHR (
                                Url,
                                Buffer,
                                BufferSize,
                                Filler,
                                VPathReadHost
                                );
}   /* XFS_ReadCHost_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

LIB_EXPORT
const char * CC
XFS_SkipSpaces_ZHR ( const char * Start, const char * End )
{
    if ( Start != NULL && End != NULL ) {
        while ( Start < End ) {
            if ( ! isspace ( * Start ) ) {
                return Start;
            }

            Start ++;
        }
    }
    return NULL;
}   /* XFS_SkipSpaces_ZHR () */

LIB_EXPORT
const char * CC
XFS_SkipLetters_ZHR ( const char * Start, const char * End )
{
    if ( Start != NULL && End != NULL ) {
        while ( Start < End ) {
            if ( isspace ( * Start ) ) {
                return Start;
            }

            Start ++;
        }
    }
    return NULL;
}   /* XFS_SkipLetters_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSHttpStream {
    KClientHttp * http;
    KHttpRequest * req;
    KHttpResult * res;
    KStream * str;

    rc_t last_error;
    bool completed;
};

LIB_EXPORT
rc_t CC
XFS_HttpStreamMake_ZHR (
                    const char * Url,
                    const struct XFSHttpStream ** Stream
)
{
    rc_t RCt;
    struct VPath * Path;
    struct String Host;
    uint32_t Port;
    struct XFSHttpStream * TheStream;

    RCt = 0;
    Path = NULL;
    TheStream = NULL;

    if ( Stream != NULL ) {
        * Stream = NULL;
    }

    if ( Url == NULL || Stream == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = VFSManagerMakePath ( XFS_VfsManager (), & Path, Url );
    if ( RCt == 0 ) {
        RCt = VPathGetHost ( Path, & Host );
        if ( RCt == 0 ) {
            Port = VPathGetPortNum ( Path );

            TheStream = calloc ( 1, sizeof ( struct XFSHttpStream ) );
            if ( TheStream == NULL ) {
                RCt = XFS_RC ( rcExhausted );
            }
            else {
                RCt = KNSManagerMakeHttp (
                                    XFS_KnsManager(),
                                    & ( TheStream -> http ),
                                    NULL,
                                    0x01010000,
                                    & Host,
                                    Port
                                    );
                if ( RCt == 0 ) {
                    RCt = KHttpMakeRequest (
                                        TheStream -> http,
                                        & ( TheStream -> req ),
                                        Url
                                        );
                    if ( RCt == 0 ) {
                        RCt = KHttpRequestGET (
                                        TheStream -> req,
                                        & ( TheStream -> res )
                                        );
                        if ( RCt == 0 ) {
                            RCt = KHttpResultGetInputStream (
                                                TheStream -> res,
                                                & ( TheStream -> str )
                                                );
                            if ( RCt == 0 ) {
                                TheStream -> completed = false;
                                TheStream -> last_error = 0;

                                * Stream = TheStream;
                            }
                        }
                    }
                }
            }
        }

        VPathRelease ( Path );
    }

    if ( RCt != 0 ) {
        * Stream = NULL;

        if ( TheStream != NULL ) {
            XFS_HttpStreamDispose_ZHR ( TheStream );
        }
    }

    return RCt;
}   /* XFS_HttpStreamMake_ZHR () */

static
rc_t CC
_HttpStreamCloseInternals_ZHR ( const struct XFSHttpStream * self )
{
    struct XFSHttpStream * Stream = ( struct XFSHttpStream * ) self;

    if ( Stream == 0 ) {
        return 0;
    }

    if ( Stream -> str != NULL ) {
        KStreamRelease ( Stream -> str );

        Stream -> str = NULL;
    }

    if ( Stream -> res != NULL ) {
        KClientHttpResultRelease ( Stream -> res );

        Stream -> res = NULL;
    }

    if ( Stream -> req != NULL ) {
        KClientHttpRequestRelease ( Stream -> req );

        Stream -> req = NULL;
    }

    if ( Stream -> http != NULL ) {
        KClientHttpRelease ( Stream -> http );

        Stream -> http = NULL;
    }

    return 0;
}   /* _HttpStreamCloseInternals_ZHR () */

LIB_EXPORT
rc_t CC
XFS_HttpStreamDispose_ZHR ( const struct XFSHttpStream * self )
{
    struct XFSHttpStream * Stream = ( struct XFSHttpStream * ) self;

    if ( Stream == NULL ) {
        return 0;
    }

    Stream -> completed = false;
    Stream -> last_error = XFS_RC ( rcInvalid );

    _HttpStreamCloseInternals_ZHR ( self );

    free ( Stream );

    return 0;
}   /* XFS_HttpStreamDispose_ZHR () */

LIB_EXPORT
bool CC
XFS_HttpStreamGood_ZHR ( const struct XFSHttpStream * self )
{
    if ( self != NULL ) {
        return self -> last_error == 0;
    }
    return false;
}   /* XFS_HttpStreamGood_ZHR () */

LIB_EXPORT
bool CC
XFS_HttpStreamCompleted_ZHR ( const struct XFSHttpStream * self )
{
    if ( self != NULL ) {
        return self -> completed;
    }
    return true;
}   /* XFS_HttpStreamCompleted_ZHR () */

LIB_EXPORT
rc_t CC
XFS_HttpStreamRead_ZHR (
                    const struct XFSHttpStream * self,
                    void * Buffer,
                    size_t Size,
                    size_t * NumRead
)
{
    struct XFSHttpStream * Stream = ( struct XFSHttpStream * ) self;

    if ( NumRead != NULL ) {
        * NumRead = 0;
    }

    if ( Stream == NULL || Buffer == NULL || Size == 0 || NumRead == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Stream -> last_error != 0 ) {
        return Stream -> last_error;
    }

    if ( Stream -> completed ) {
        * NumRead = 0;

        return 0;
    }

    if ( Stream -> str == NULL ) {
        _HttpStreamCloseInternals_ZHR ( Stream );

        Stream -> last_error = XFS_RC ( rcInvalid );

        return Stream -> last_error;
    }

    Stream -> last_error = KStreamRead ( 
                                    Stream -> str,
                                    Buffer,
                                    Size,
                                    NumRead
                                    );
    if ( Stream -> last_error == 0 ) {
        if ( * NumRead == 0 ) {
            _HttpStreamCloseInternals_ZHR ( Stream );

            Stream -> completed = true;
        }
    }
    else {
        _HttpStreamCloseInternals_ZHR ( Stream );
    }

    return self -> last_error;
}   /* XFS_HttpStreamRead_ZHR */

LIB_EXPORT
rc_t CC
XFS_HttpStreamTimedRead_ZHR (
                    const struct XFSHttpStream * self,
                    void * Buffer,
                    size_t Size,
                    size_t * NumRead,
                    struct timeout_t * Tm
)
{
    struct XFSHttpStream * Stream = ( struct XFSHttpStream * ) self;

    if ( NumRead != NULL ) {
        * NumRead = 0;
    }

    if ( Stream == NULL || Buffer == NULL || Size == 0 || NumRead == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Stream -> last_error != 0 ) {
        return Stream -> last_error;
    }

    if ( Stream -> completed ) {
        * NumRead = 0;

        return 0;
    }

    if ( Stream -> str == NULL ) {
        _HttpStreamCloseInternals_ZHR ( Stream );

        Stream -> last_error = XFS_RC ( rcInvalid );

        return Stream -> last_error;
    }

    Stream -> last_error = KStreamTimedRead ( 
                                    Stream -> str,
                                    Buffer,
                                    Size,
                                    NumRead,
                                    Tm
                                    );
    if ( Stream -> last_error == 0 ) {
        if ( * NumRead == 0 ) {
            _HttpStreamCloseInternals_ZHR ( Stream );

            Stream -> completed = true;
        }
    }
    else {
        _HttpStreamCloseInternals_ZHR ( Stream );
    }

    return self -> last_error;
}   /* XFS_HttpStreamTimedRead_ZHR */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
XFS_KeyType_ZHR ( const char * EncType, KKeyType * Type )
{
    size_t B;
    const char * C;

    B = 0;
    C = NULL;

    XFS_CAN ( Type )
    * Type = kkeyNone;

    if ( EncType == NULL ) {
        * Type = KKeyTypeDefault;
        return 0;
    }

    B = string_size ( EncType );

    C = "AES128";
    if ( strcase_cmp ( C, string_size ( C ), EncType, B, B ) == 0 ) {
        * Type = kkeyAES128;
        return 0;
    }

    C = "AES192";
    if ( strcase_cmp ( C, string_size ( C ), EncType, B, B ) == 0 ) {
        * Type = kkeyAES192;
        return 0;
    }

    C = "AES256";
    if ( strcase_cmp ( C, string_size ( C ), EncType, B, B ) == 0 ) {
        * Type = kkeyAES256;
        return 0;
    }

    return XFS_RC ( rcInvalid );
}   /* XFS_KeyType_ZHR () */

LIB_EXPORT
rc_t CC
XFS_InitKKey_ZHR (
            const char * EncPass,
            const char * EncType,
            struct KKey * Key
)
{
    rc_t RCt;
    KKeyType Type;

    RCt = 0;
    Type = kkeyNone;

    XFS_CAN ( EncPass )
    XFS_CAN ( Key )

    RCt = XFS_KeyType_ZHR ( EncType, & Type );
    if ( RCt == 0 ) {
        RCt = KKeyInitRead (
                            Key,
                            Type,
                            EncPass,
                            string_size ( EncPass )
                            );
    }

    return RCt;
}   /* XFS_InitKey_ZHR () */

LIB_EXPORT
rc_t CC
XFS_CopyKKey_ZHR ( const struct KKey * Src, struct KKey * Dst )
{
    XFS_CAN ( Src )
    XFS_CAN ( Dst )

    memmove ( Dst, Src, sizeof ( struct KKey ) );

    return 0;
}   /* XFS_CopyKKey_ZHR () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

