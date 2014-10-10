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

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
