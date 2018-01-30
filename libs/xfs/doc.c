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
#include <klib/text.h>
#include <klib/printf.h>

#include <kfs/file.h>
#include <kfs/file-impl.h>

#include <xfs/doc.h>

#include <va_copy.h>

#include <string.h>    /* memmove() */

#include <sysalloc.h>

static const char * _sDoc_classname = "XFSDoc";

/*))
 //     Here is a struct buffer
((*/
struct XFSDoc {
    const union XFSDoc_vt * vt;

    KRefcount refcount;
};

LIB_EXPORT
rc_t CC
XFSDocInit ( const struct XFSDoc * self, const union XFSDoc_vt * VT )
{
    struct XFSDoc * Doc = ( struct XFSDoc * ) self;

    if ( Doc == NULL || VT == NULL ) {
        return XFS_RC ( rcNull );
    }

    KRefcountInit ( 
                    & ( Doc -> refcount ),
                    1,
                    _sDoc_classname,
                    "XFSDocInit",
                    "Doc"
                    );

    Doc -> vt = VT;

    return 0;
}   /* XFSDocInit () */

LIB_EXPORT
rc_t CC
XFSDocDispose ( const struct XFSDoc * self )
{
    rc_t RCt;
    struct XFSDoc * Doc = ( struct XFSDoc * ) self;

    RCt = 0;

    if ( Doc == NULL ) {
        return XFS_RC ( rcNull );
    }

    KRefcountWhack ( & ( Doc -> refcount ), _sDoc_classname );

    if ( Doc -> vt != NULL ) {
        switch ( Doc -> vt -> v1.maj ) {
            case 1 :
                    if ( Doc -> vt -> v1.dispose != NULL ) {
                        RCt = Doc -> vt -> v1.dispose ( Doc );
                    }
                    break;
            default :
                    RCt = XFS_RC ( rcBadVersion );
                    break;
        }
    }

    return RCt;
}   /* XFSDocDispose () */

LIB_EXPORT
rc_t CC
XFSDocAddRef ( const struct XFSDoc * self )
{
    rc_t RCt;
    int32_t RetCode;
    struct XFSDoc * Doc;

    RCt = 0;
    RetCode = 0;
    Doc = ( struct XFSDoc * ) self;

    if ( Doc != NULL ) {
        RetCode = KRefcountAdd (
                            & ( Doc -> refcount ),
                            _sDoc_classname
                            );
        switch ( RetCode ) {
            case krefOkay :
                    RCt = 0;
                    break;
            case krefZero :
            case krefLimit :
            case krefNegative :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcInvalid );
                    break;
            default :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcUnknown );
                    break;
        }
    }

    return RCt;
}   /* XFSDocAddRef () */

LIB_EXPORT
rc_t CC
XFSDocRelease ( const struct XFSDoc * self )
{
    rc_t RCt;
    int32_t RetCode;
    struct XFSDoc * Doc;

    RCt = 0;
    RetCode = 0;
    Doc = ( struct XFSDoc * ) self;

    if ( Doc != NULL ) {
        RetCode = KRefcountDrop (
                            & ( Doc -> refcount ),
                            _sDoc_classname
                            );
        switch ( RetCode ) {
            case krefOkay :
            case krefZero :
                    RCt = 0;
                    break;
            case krefWhack :
                    XFSDocDispose ( Doc );
                    break;
            case krefNegative :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcInvalid );
                    break;
            default :
                    RCt = RC ( rcFS, rcNoTarg, rcAttaching, rcRefcount, rcUnknown );
                    break;
        }
    }

    return RCt;
}   /* XFSDocRelease () */

LIB_EXPORT
rc_t CC
XFSDocGet ( const struct XFSDoc * self, const char ** Ret )
{
    rc_t RCt;
    const char * Var1;

    RCt = 0;
    Var1 = NULL;

    if ( Ret != NULL ) {
        * Ret = NULL;
    }

    if ( self == NULL || Ret == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> vt == NULL ) { 
        return XFS_RC ( rcInvalid );
    }

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.get != NULL ) {
                RCt = self -> vt -> v1.get ( self, & Var1 );
                if ( RCt == 0 ) {
                    if ( Var1 == NULL ) {
                        RCt = XFS_RC ( rcInvalid );
                    }
                    else {
                        * Ret = Var1;
                    }
                }
            }
            else {
                RCt = XFS_RC ( rcUnsupported );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSDocGet () */

LIB_EXPORT
rc_t CC
XFSDocSize ( const struct XFSDoc * self, uint64_t * Ret )
{
    rc_t RCt;
    uint64_t Var1;

    RCt = 0;
    Var1 = 0;

    if ( Ret != NULL ) {
        * Ret = 0;
    }

    if ( self == NULL || Ret == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> vt == NULL ) { 
        return XFS_RC ( rcInvalid );
    }

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.size != NULL ) {
                RCt = self -> vt -> v1.size ( self, & Var1 );
                if ( RCt == 0 ) {
                    * Ret = Var1;
                }
            }
            else {
                RCt = XFS_RC ( rcUnsupported );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSDocSize () */

/*))
 //     Here is a file to implement
((*/
struct XFSDocFile {
    struct KFile Papahen;
    const struct XFSDoc * Doc;
};

/*\
|*| First we should make appropriate virtual table
\*/
static
rc_t CC
XFSDocFile_destroy_v1 ( struct KFile * self )
{
    struct XFSDocFile * DocFile = ( struct XFSDocFile * ) self;

    if ( DocFile != NULL ) {
        if ( DocFile -> Doc != NULL ) {
            XFSDocRelease ( DocFile -> Doc );
        }
        DocFile -> Doc = NULL;

        free ( DocFile );
    }

    return 0;
}   /* XFSDocFile_destroy_v1 () */

static
struct KSysFile * 
XFSDocFile_get_sysfile_v1 ( const struct KFile * self, uint64_t * offset )
{
    if ( offset != NULL ) {
        * offset = 0;
    }

    return NULL;
}   /* XFSDocFile_get_sysfile_v1 () */

static
rc_t CC
XFSDocFile_random_access_v1 ( const struct KFile * self )
{
    return 0;
}   /* XFSDocFile_random_access_v1 () */

static
rc_t CC
XFSDocFile_get_size_v1 ( const struct KFile * self, uint64_t * size )
{
    const struct XFSDocFile * DocFile = ( const struct XFSDocFile * ) self;

    if ( size != NULL ) {
        * size = 0;
    }

    if ( DocFile == NULL || size == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( DocFile -> Doc == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSDocSize ( DocFile -> Doc, size );
}   /* XFSDocFile_get_size_v1 () */

static
rc_t CC
XFSDocFile_set_size_v1 ( struct KFile * self, uint64_t size )
{
    return XFS_RC ( rcUnsupported );
}   /* XFSDocFile_set_size_v1 () */

static
rc_t CC
XFSDocFile_read_v1 (
            const struct KFile * self,
            uint64_t pos,
            void * buffer,
            size_t bsize,
            size_t * num_read
)
{
    rc_t RCt;
    const struct XFSDocFile * DocFile;
    uint64_t DocS;
    const char * DocB;
    size_t Siz2R;

    RCt = 0;
    DocFile = ( const struct XFSDocFile * ) self;
    DocS = 0;
    DocB = NULL;
    Siz2R = 0;

    if ( num_read != NULL ) {
        * num_read = 0;
    }

    if ( DocFile == NULL || buffer == NULL || num_read == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( bsize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    if ( DocFile -> Doc == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSDocSize ( DocFile -> Doc, & DocS );
    if ( RCt == 0 ) {
        if ( DocS < pos ) {
            return XFS_RC ( rcInvalid );
        }

        Siz2R = ( size_t ) ( DocS - pos );
        if ( bsize < Siz2R ) {
            Siz2R = bsize;
        }

        RCt = XFSDocGet ( DocFile -> Doc, & DocB );
        if ( RCt == 0 ) {
            memmove ( buffer, DocB + pos, Siz2R );
            * num_read = Siz2R;
        }
    }

    return RCt;
}   /* XFSDocFile_read_v1 () */

static
rc_t CC
XFSDocFile_write_v1 (
            struct KFile * self,
            uint64_t pos,
            const void * buffer,
            size_t bsize,
            size_t * num_writ
)
{
    if ( num_writ != NULL ) {
        * num_writ = 0;
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDocFile_write_v1 () */

static
uint32_t CC
XFSDocFile_get_type_v1 ( const struct KFile * self )
{
    return kfdFile;
}   /* XFSDocFile_get_type_v1 () */

static struct KFile_vt_v1 _sXFSDocFile_vt_v1 = {
                            1, 1,
                            XFSDocFile_destroy_v1,
                            XFSDocFile_get_sysfile_v1,
                            XFSDocFile_random_access_v1,
                            XFSDocFile_get_size_v1,
                            XFSDocFile_set_size_v1,
                            XFSDocFile_read_v1,
                            XFSDocFile_write_v1,
                            XFSDocFile_get_type_v1
                            };


/*\
|*| Here we are ugly
\*/
LIB_EXPORT
rc_t CC
XFSDocFileMake (
            const char * Name,
            const struct XFSDoc * Doc,
            struct KFile ** File
)
{
    rc_t RCt;
    struct XFSDocFile * DocFile;
    const char * xName;

    RCt = 0;
    DocFile = NULL;
    xName = Name == NULL ? "DocFile" : Name;

    if ( File != NULL ) {
        * File = NULL;
    }

    if ( Doc == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSDocAddRef ( Doc );
    if ( RCt == 0 ) {
        DocFile = calloc ( 1, sizeof ( struct XFSDocFile ) );
        if ( DocFile == NULL ) {
            return XFS_RC ( rcExhausted );
        }

        RCt = KFileInit ( 
                        & ( DocFile -> Papahen ),
                        ( const KFile_vt * ) & _sXFSDocFile_vt_v1,
                        "XFSDocFile",
                        xName,
                        true,
                        false
                        );
        if ( RCt == 0 ) {
            RCt = XFSDocAddRef ( Doc ); 
            if ( RCt == 0 ) {
                DocFile -> Doc = Doc;
                * File = & ( DocFile -> Papahen );
            }
        }

        if ( RCt != 0 ) {
            * File = NULL;

            KFileRelease ( & ( DocFile -> Papahen ) );
        }
    }

    return RCt;
}   /* XFSDocFileMake () */

/*))
 //     Here is README Doc
((*/
struct XFSTextDoc {
    struct XFSDoc Papahen;

    char * buffer;
    size_t size;
    size_t capacity;
};

static
rc_t CC
XFSTextDoc_dispose_v1 ( struct XFSDoc * self )
{
    struct XFSTextDoc * Doc = ( struct XFSTextDoc * ) self;

    if ( Doc == NULL ) {
        return 0;
    }

    if ( Doc -> buffer != NULL ) {
        free ( Doc -> buffer );
    }

    Doc -> buffer = NULL;
    Doc -> size = 0;
    Doc -> capacity = 0;

    free ( Doc );

    return 0;
}   /* XFSTextDoc_dispose_v1 () */

static
rc_t CC
XFSTextDoc_get_v1 ( const struct XFSDoc * self, const char ** B )
{
    const struct XFSTextDoc * Doc = ( const struct XFSTextDoc * ) self;
    if ( B != NULL ) {
        * B = NULL;
    }

    if ( Doc == NULL || B == NULL ) {
        return XFS_RC ( rcNull );
    }

    * B = Doc -> buffer;

    return 0;
}   /* XFSTextDoc_get_v1 () */

static
rc_t CC
XFSTextDoc_size_v1 ( const struct XFSDoc * self, uint64_t * S )
{
    const struct XFSTextDoc * Doc = ( const struct XFSTextDoc * ) self;
    if ( S != NULL ) {
        * S = 0;
    }

    if ( Doc == NULL || S == NULL ) {
        return XFS_RC ( rcNull );
    }

    * S = Doc -> size;

    return 0;
}   /* XFSTextDoc_size_v1 () */

static struct XFSDoc_vt_v1 _sTextDoc_vt_v1 = {
                                            1, 1,
                                            XFSTextDoc_dispose_v1,
                                            XFSTextDoc_get_v1,
                                            XFSTextDoc_size_v1
                                            };

LIB_EXPORT
rc_t CC
XFSTextDocMake ( struct XFSDoc ** TextDoc )
{
    rc_t RCt;
    struct XFSTextDoc * Doc;

    RCt = 0;
    Doc = NULL;

    if ( TextDoc == NULL ) {
        return XFS_RC ( rcNull );
    }
    * TextDoc = NULL;

    Doc = calloc ( 1, sizeof ( struct XFSTextDoc ) );
    if ( Doc == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSDocInit (
                    & ( Doc -> Papahen ),
                    ( const union XFSDoc_vt * ) & _sTextDoc_vt_v1
                    );
    if ( RCt == 0 ) {
        * TextDoc = & ( Doc -> Papahen );
    }
    else {
        XFSDocRelease ( & ( Doc -> Papahen ) );
    }

    return RCt;
}   /* XFSTextDocMake () */

LIB_EXPORT
rc_t CC
XFSTextDocAppend ( struct XFSDoc * self, const char * Fmt, ... )
{
    rc_t RCt;
    va_list args;

    va_start ( args, Fmt );

    RCt = XFSTextDocVAppend ( self, Fmt, args );

    va_end ( args );

    return RCt;
}   /* XFSTextDocAppend () */

static
rc_t CC
__TextDocRealloc ( struct XFSDoc * self, size_t Len )
{
    size_t NewCap;
    struct XFSTextDoc * Doc;
    char * NewBuf;

    Doc = ( struct XFSTextDoc * ) self;
    NewCap = 0;
    NewBuf = NULL;

    if ( Doc == NULL ) {
        return XFS_RC ( rcNull );
    }

    NewCap = Doc -> size + Len;

    if ( Doc -> capacity < NewCap ) {
        NewCap = ( ( NewCap / 1024 ) + 1 ) * 1024;

        NewBuf = calloc ( NewCap, sizeof ( char ) );
        if ( NewBuf == NULL ) {
            return XFS_RC ( rcExhausted );
        }

        if ( Doc -> buffer != NULL ) {
            if ( Doc -> size != 0 ) {
                memmove (
                        NewBuf,
                        Doc -> buffer,
                        Doc -> size * sizeof ( char )
                        );
                free ( Doc -> buffer );
                Doc -> buffer = NULL;
            }
        }

        Doc -> buffer = NewBuf;
        Doc -> capacity = NewCap;
    }

    return 0;
}   /* __TextDocRealloc () */

static
rc_t CC
__TextDocAppend ( struct XFSDoc * self, const char * Line, size_t Len )
{
    rc_t RCt;
    struct XFSTextDoc * Doc;

    RCt = 0;
    Doc = ( struct XFSTextDoc * ) self;

    if ( Doc == NULL || Line == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Len == 0 ) {
        return 0;
    }

    RCt = __TextDocRealloc ( self, Len );
    if ( RCt ==  0 ) {
        memmove ( Doc -> buffer + Doc -> size, Line, Len );

        Doc -> size += Len;
    }

    return RCt;
}   /* __TextDocAppend () */

LIB_EXPORT
rc_t CC
XFSTextDocVAppend ( struct XFSDoc * self, const char * Fmt, va_list args )
{
    rc_t RCt;
    char TBF [ XFS_SIZE_1024 ];
    size_t nwr;
    va_list xArgs;

    RCt = 0;
    * TBF = 0;
    nwr = 0;

    if ( self == NULL || Fmt == NULL ) {
        return XFS_RC ( rcNull );
    }

    va_copy ( xArgs, args );
    RCt = string_vprintf ( TBF, sizeof ( TBF ), & nwr, Fmt, xArgs );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        if ( nwr == 0 ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = __TextDocAppend ( self, TBF, nwr );
        }
    }

    return RCt;
}   /* XFSTextDocVAppend () */
