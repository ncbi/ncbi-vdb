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

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"

#include <sysalloc.h>

#include <string.h>
#include <ctype.h>

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

    if ( Resource == NULL || Config == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Config = NULL;

        /* Trying to open resource as file */
    RCt = XFS_OpenResourceRead_MHR ( Resource, & File );
    if ( RCt == 0 ) {
        RCt = KConfigMake (
                        & TmpConfig,
                        NULL    /* We need new clear config each time */
                        );
        if ( RCt == 0 ) {
            RCt = KConfigLoadFile ( TmpConfig, Resource, File );
            if ( RCt == 0 ) {
                * Config = TmpConfig;
            }
            else {
                KConfigRelease ( TmpConfig );
            }
        }

        KFileRelease ( File );
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

