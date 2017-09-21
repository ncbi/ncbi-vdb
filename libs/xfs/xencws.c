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
#include <klib/log.h>
#include <kproc/lock.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/impl.h>

#include <krypto/key.h>
#include <krypto/encfile.h>

#include <xfs/doc.h>

#include <va_copy.h>

#include "schwarzschraube.h"
#include "xencws.h"
#include "zehr.h"
#include "lreader.h"
#include <xfs/path.h>

#include <sysalloc.h>

#include <ctype.h>
#include <time.h>

/*||*\
  || Lyrics:
  || 
  || There is temporary solution for WORKSPACE directory, which
  || supposes encrypting of everything.
  || Simple simulation: directory could be opened as workspace
  || if it contains special file with name _sDirectoryWsContent
  || That file is simple file in format :
  || 
  ||     type<tab>eff_name<tab>display_name
  ||     type<tab>eff_name<tab>display_name
  ||     type<tab>eff_name<tab>display_name
  || 
  || Only entries from that file are accessible throught that interface
  || There are two types of entries : file and folder
  ||
  || User should pass path and password to XFSEncDirectoryOpen
  || method to open directory. All other operations with file
  || system should be done relative to that directory.
  ||
\*||*/

#ifdef JOJOBA
#define LOG_LOC_ACQ(line,lockp) pLogMsg (   \
                        klogDebug,  \
                        " [KLockAcquire] [$(line)] [$(lockp)]", \
                        "line=%d,lockp=%p", \
                        line, ( void * )lockp   \
                        )
#define LOG_LOC_UNL(line,lockp) pLogMsg (   \
                        klogDebug,  \
                        " [KLockUnlock] [$(line)] [$(lockp)]", \
                        "line=%d,lockp=%p", \
                        line, ( void * )lockp   \
                        )
#else
#define LOG_LOC_ACQ(line,lockp)
#define LOG_LOC_UNL(line,lockp)
#endif


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 ||  Usefull stuff
((*/

/* static const char * _sDirectoryWsContent = ".ncbi-ws-content"; */
static const char * _sDirectoryWsContent = ".ncbi-ws-gehalt";
static const char * _sFileExt            = ".datei";
static const char * _sFolderExt          = ".mappe";
static const char * _sFileTag            = "file";
static const char * _sFolderTag          = "folder";

#define _NAME_FORMAT_LEN_   8

/* because of we unable to read encrypted file header in Write Only
 * mode we are always opening files in Update mode. I have no idea how
 * to fix that problem. Thinking about it.
 */
#define _ENCODED_FILE_UPDATE_ALWAYS_

/*\
 *  Other thoughts:
 *
 *  each directory will imediately synchronize it's content to disk.
 *  That synchronisation works only in the case of : creation, deletion
 *  and renaming file.
\*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*                                                                   */
/* As it usually happens, first is description of directory          */
/*                                                                   */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct _DirC;

struct _DirE {
    BSTNode node;

    KLock * mutabor;
    KRefcount refcount;

    const char * name;
    const char * eff_name;

    bool is_folder;
    const struct _DirC * content;     /* NULL for non-folder */
};

struct _DirC {
    BSTree tree;

    struct KKey key;

    const char * path;

    uint64_t last;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/**)))  There are too many places, where encrypted file is opened
  (((**/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static const char * _sDirE_classname = "DirE";

static rc_t CC _DirEDisposeContent ( const struct _DirE * self );

static
rc_t CC
_DirEDispose ( const struct _DirE * self )
{
    struct _DirE * Entry = ( struct _DirE * ) self;

    if ( Entry != NULL ) {

#ifdef JOJOBA
pLogMsg ( klogDebug, " [-DSP] [$(line)] [$(entry)] [$(name)]", "line=%d,entry=%p,name=%s", __LINE__, Entry, Entry -> name );
#endif /* JOJOBA */

        if ( Entry -> mutabor != NULL ) {
            KLockRelease ( Entry -> mutabor );
            Entry -> mutabor = NULL;
        }

        KRefcountWhack (
                        & ( Entry -> refcount ),
                        _sDirE_classname
                        );

        if ( Entry -> name != NULL ) {
            free ( ( char * ) Entry -> name );
            Entry -> name = NULL;
        }
        if ( Entry -> eff_name != NULL ) {
            free ( ( char * ) Entry -> eff_name );
            Entry -> eff_name = NULL;
        }

        _DirEDisposeContent ( Entry );

        free ( Entry );
    }
    return 0;
}   /* _DirEDispose () */

static
rc_t CC
_DirEMake (
            const char * Type,
            const char * EffName,
            const char * DisplayName,
            const struct _DirE ** Entry
)
{
    rc_t RCt;
    struct _DirE * RetVal;
    bool IsFolder;

    RCt = 0;
    RetVal = NULL;
    IsFolder = false;

    if ( Type != NULL && EffName != NULL && DisplayName != NULL ) {
        if ( strcmp ( Type, _sFileTag ) == 0 ) {
            IsFolder = false;
        }
        else {
            if ( strcmp ( Type, _sFolderTag ) == 0 ) {
                IsFolder = true;
            }
            else {
                RCt = XFS_RC ( rcInvalid );
            }
        }

        if ( RCt == 0 ) {
            RetVal = calloc ( 1, sizeof ( struct _DirE ) );
            if ( RetVal == NULL ) {
                RCt = XFS_RC ( rcExhausted );
            }
            else {
                RCt = KLockMake ( & ( RetVal -> mutabor ) );
                if ( RCt == 0 ) {
                    KRefcountInit (
                                & ( RetVal -> refcount ),
                                1,
                                _sDirE_classname,
                                "DirEMake",
                                "DirE"
                                );

                    RCt = XFS_StrDup (
                                    DisplayName,
                                    & ( RetVal -> name )
                                    );
                    if ( RCt == 0 ) {
                        RCt = XFS_StrDup (
                                        EffName,
                                        & ( RetVal -> eff_name )
                                        );
                        if ( RCt == 0 ) {
                            RetVal -> is_folder = IsFolder;
                            * Entry = RetVal;
#ifdef JOJOBA
pLogMsg ( klogDebug, " [-ALC] [$(line)] [$(entry)] [$(name)]", "line=%d,entry=%p,name=%s", __LINE__, RetVal, RetVal -> name );
#endif /* JOJOBA */
                        }
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        if ( RetVal != NULL ) {
            if ( RetVal -> mutabor != NULL ) {
                KLockRelease ( RetVal -> mutabor );
                RetVal -> mutabor = NULL;
            }
            if ( RetVal -> name != NULL ) {
                free ( ( char * ) RetVal -> name );
                RetVal -> name = NULL;
            }
            if ( RetVal -> eff_name != NULL ) {
                free ( ( char * ) RetVal -> eff_name );
                RetVal -> eff_name = NULL;
            }

            free ( RetVal );
        }

        * Entry = NULL;
    }

    return RCt;
}   /* _DirEMake () */

static
rc_t CC
_DirEAddRef ( const struct _DirE * self )
{
    rc_t RCt;
    struct _DirE * Entry;
    int Refc;

    RCt = 0;
    Entry = ( struct _DirE * ) self;
    Refc = 0;

    XFS_CAN ( Entry )

#ifdef JOJOBA
pLogMsg ( klogDebug, " [>ARE] [$(line)] [$(entry)] [$(name)]", "line=%d,entry=%p,name=%s", __LINE__, self, self -> name );
#endif /* JOJOBA */

    Refc = KRefcountAdd (
                    & ( Entry -> refcount ),
                    _sDirE_classname
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
}   /* _DirEAddRef () */

static
rc_t CC
_DirERelease ( const struct _DirE * self )
{
    rc_t RCt;
    struct _DirE * Entry;
    int Refc;

    RCt = 0;
    Entry = ( struct _DirE * ) self;
    Refc = 0;

    XFS_CAN ( Entry )

#ifdef JOJOBA
pLogMsg ( klogDebug, " [<ARE] [$(line)] [$(entry)] [$(name)]", "line=%d,entry=%p,name=%s", __LINE__, self, self -> name );
#endif /* JOJOBA */

    Refc = KRefcountDrop (
                    & ( Entry -> refcount ),
                    _sDirE_classname
                    );
    switch ( Refc ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _DirEDispose ( self );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* _DirERelease () */

static
rc_t CC
_NextWord (
            const char * Line,
            size_t Length,
            const char ** Word,
            const char ** NextLine
)
{
    const char * Bg, * En, * Ps;

    Bg = En = Ps = NULL;

    XFS_CSAN ( Word )
    XFS_CSAN ( NextLine )
    XFS_CA ( Length, 0 )
    XFS_CAN ( Word )
    XFS_CAN ( NextLine )
    XFS_CAN ( Line )

    Bg = Line;
    En = Line + Length;

        /* Skipping fron tabls */
    while ( Bg < En ) {
        if ( * Bg != '\t' ) {
            break;
        }
        Bg ++;
    }

    Ps = Bg;
    while ( Ps < En ) {
        if ( * Ps == '\t' ) {
            break;
        }
        Ps ++;
    }

    if ( Ps == Bg ) {
        return XFS_RC ( rcInvalid );
    }

    * Word = string_dup ( Bg, Ps - Bg );
    if ( * Word == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( Ps == En ) {
        * NextLine = NULL;
    }
    else {
        * NextLine = Ps + 1;
    }

    return 0;
}   /* _NextWord () */

static
rc_t CC
_DirEParse (
            const struct String * Line,
            const struct _DirE ** Entry
)
{
    rc_t RCt;
    const char * Start, * Name, * EffName, * Type, * NextLine;
    size_t Len, Dif;

    RCt = 0;
    Start = Name = EffName = Type = NextLine = NULL;
    Len = Dif = 0;

    XFS_CSAN ( Entry )
    XFS_CAN ( Line )
    XFS_CAN ( Entry )

    Start = Line -> addr;
    Len = Line -> len;

        /* First is Type */
    RCt = _NextWord ( Start, Len, & Type, & NextLine );
    if ( RCt == 0 ) {
        Len -= NextLine - Start;
        Start = NextLine;


            /* Second is Effective Name */
        RCt = _NextWord ( Start, Len, & EffName, & NextLine );
        if ( RCt == 0 ) {
            Len -= NextLine - Start;
            Start = NextLine;

                /* Third is display Name */
            RCt = _NextWord ( Start, Len, & Name, & NextLine );
            if ( RCt == 0 ) {
                RCt = _DirEMake ( Type, EffName, Name, Entry );

                free ( ( char * ) Name );
                Name = NULL;
            }

            free ( ( char * ) EffName );
            EffName = NULL;
        }

        free ( ( char * ) Type );
        Type = NULL;
    }

    if ( RCt != 0 ) {
        if ( * Entry != NULL ) {
            _DirEDispose ( * Entry );
        }
    }

    return RCt;
}   /* _DirEParse () */

static
rc_t CC
_OpenEncLineReader (
                const struct XFSLineReader ** Reader,
                const struct KKey * Key,
                const char * Path,
                ...
)
{
    rc_t RCt;
    struct KDirectory * NatDir;
    const struct KFile * File;
    const struct KFile * EncFile;
    va_list Args;

    RCt = 0;
    NatDir = NULL;
    File = NULL;
    EncFile = NULL;

    XFS_CSAN ( Reader )
    XFS_CAN ( Key )
    XFS_CAN ( Path )
    XFS_CAN ( Reader )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_start ( Args, Path );
        RCt = KDirectoryVOpenFileRead ( NatDir, & File, Path, Args );
        va_end ( Args );

        if ( RCt == 0 ) {
            RCt = KEncFileMakeRead ( & EncFile, File, Key );
            if ( RCt == 0 ) {
                RCt = XFSLineReaderMake ( EncFile, Reader );

                KFileRelease ( EncFile );
            }

            KFileRelease ( File );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _OpenEncLineReader () */

/*))
  ++    IMPORTANT: all modifications to Content should be made through 
  ++               corresponding Entry
  ((*/
static rc_t CC _SyncronizeDirectoryContentNoLock (
                                        const struct _DirE * self
                                        );
static rc_t CC _DirEAddEntryNoLock (
                                    const struct _DirE * self,
                                    const struct _DirE * Entry
                                    );
static
rc_t CC
_DirEReadContent ( const struct _DirE * self )
{
    rc_t RCt;
    const struct XFSLineReader * Reader;
    const struct _DirE * Entry;
    const struct String Line;
    size_t LineNo; 

    RCt = 0;
    Reader = NULL;
    Entry = NULL;
    LineNo = 0;

    XFS_CAN ( self )

    RCt = _OpenEncLineReader (
                            & Reader,
                            & ( self -> content -> key ),
                            "%s/%s",
                            self -> content -> path,
                            _sDirectoryWsContent
                            );

    if ( RCt == 0 ) {
        if ( XFSLineReaderGood ( Reader ) ) {
            do {
                RCt = XFSLineReaderGet ( Reader, & Line );
                if ( RCt == 0 ) {
                    RCt = _DirEParse ( & Line, & Entry );
                    if ( RCt == 0 ) {
                        RCt = _DirEAddEntryNoLock ( self, Entry );
                        if ( RCt == 0 ) {
/* TODO */
                        }
                    }
                }

                if ( RCt != 0 ) {
                    XFSLineReaderLineNo ( Reader, & LineNo );
pLogMsg ( klogDebug, " __DirE : invalid line no $(line)", "line=%d", ( int ) LineNo );
                    RCt = 0;
                }
            } while ( XFSLineReaderNext ( Reader ) );

        }
        else {
            /*  TODO : JUST a stub. I mean, it is not a problem. File
             *  could be lost, or directory could be just created
             */
        }

        XFSLineReaderRelease ( Reader );
    }
    else {
        if ( GetRCState ( RCt ) == rcNotFound ) {
            LogMsg ( klogDebug, " Mahindra: Syncronicytyty" );
            RCt = _SyncronizeDirectoryContentNoLock ( self );
        }
    }

    return RCt;
}   /* _DirEReadContent () */

static char _FmtStr [ 64 ];
static char * _pFmtStr = NULL;
static
const char * CC
_fFmtStr ()
{
    size_t NW;

    if ( _pFmtStr == NULL ) {
        string_printf (
                    _FmtStr,
                    sizeof ( _FmtStr ),
                    & NW,
                    ".%%0.%dd%%s",
                    _NAME_FORMAT_LEN_
                    );
        _pFmtStr = _FmtStr;
    }
    return _pFmtStr;
}   /* _fFmtStr () */

static
rc_t CC
_DirCNewName (
            const struct _DirC * self,
            bool IsFolder,
            char * Out,
            size_t Len
)
{
    rc_t RCt;
    size_t Var;
    int Series;
    const char * Ext;

    RCt = 0;
    Series = 0;
    Ext = IsFolder ? _sFolderExt : _sFileExt;
    Var =    1                     /* trailing point */
           + string_size ( Ext )   /* extention for folder or file */
           + _NAME_FORMAT_LEN_     /* name */
           + 1                     /* 0 */
           ;

    XFS_CAN ( self )
    XFS_CAN ( Out )
    if ( Len <= Var ) {
        return XFS_RC ( rcInvalid );
    }

    * Out = 0;

    Series = ( int ) ( self -> last + 1 );

    RCt = string_printf ( Out, Len, & Var, _fFmtStr (), Series, Ext );
    if ( RCt == 0 ) {
        ( ( struct _DirC * ) self ) -> last ++;
    }
    else {
        * Out = 0;
    }

    return RCt;
}   /* _DirCNewName () */

static
void CC
_MakeContentDocumentCallback ( BSTNode * Node, void * Data )
{
    struct _DirE * Entry;
    struct XFSDoc * Doc;

    Entry = ( struct _DirE * ) Node;
    Doc = ( struct XFSDoc * ) Data;

    if ( Entry != NULL && Doc != NULL ) {
        XFSTextDocAppend (
                    Doc,
                    "%s\t%s\t%s\n", 
                    ( Entry -> is_folder ? _sFolderTag : _sFileTag ),
                    Entry -> eff_name,
                    Entry -> name
                    );
    }
}   /* _MakeContentDocumentCallback () */

static
rc_t CC
_MakeContentDocument (
                    const struct _DirE * self,
                    const struct XFSDoc ** Doc
)
{
    rc_t RCt;
    struct XFSDoc * RetVal;

    RCt = 0;
    RetVal = NULL;

    XFS_CSAN ( Doc )
    XFS_CAN ( self )
    XFS_CAN ( Doc )
    XFS_CAN ( self -> content )

    RCt = XFSTextDocMake ( & RetVal );
    if ( RCt == 0 ) {
        BSTreeForEach ( 
                    & ( self -> content -> tree ),
                    false,
                    _MakeContentDocumentCallback,
                    RetVal
                    );

        * Doc = RetVal;
    }

    return RCt;
}   /* _MakeContentDocument () */

static
rc_t CC
_StoreContentDocument (
                    const struct _DirE * self,
                    const struct XFSDoc * Doc
)
{
    rc_t RCt;
    struct _DirC * Content;
    struct KFile * File, * EncFile;
    struct KDirectory * NatDir;
    const char * Text;
    uint64_t TextSize;
    size_t NumWrit;
    uint32_t PathType;

    RCt = 0;
    Content = NULL;
    File = EncFile = NULL;
    NatDir = NULL;
    Text = NULL;
    TextSize = 0;
    NumWrit = 0;
    PathType = kptNotFound;

    XFS_CAN ( self )
    XFS_CAN ( Doc )
    Content = ( struct _DirC * ) self -> content;
    XFS_CAN ( Content )

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        PathType = KDirectoryPathType (
                                    NatDir,
                                    "%s/%s",
                                    Content -> path,
                                    _sDirectoryWsContent
                                    );
        if ( PathType == kptNotFound ) {
            RCt = KDirectoryCreateFile (
                                        NatDir,
                                        & File,
                                        true,
                                        0664,
                                        kcmCreate,
                                        "%s/%s",
                                        Content -> path,
                                        _sDirectoryWsContent
                                        );
        }
        else {
            RCt = KDirectoryOpenFileWrite (
                                        NatDir,
                                        & File,
                                        true,
                                        "%s/%s",
                                        Content -> path,
                                        _sDirectoryWsContent
                                        );
        }
        if ( RCt == 0 ) {
            RCt = KFileSetSize ( File, 0 );
            if ( RCt == 0 ) {
                RCt = KEncFileMakeUpdate (
                                    & EncFile,
                                    File,
                                    & ( Content -> key )
                                    );
                KFileRelease ( File );
                if ( RCt == 0 ) {
                    RCt = XFSDocSize ( Doc, & TextSize );
                    if ( RCt == 0 ) {
                        if ( TextSize != 0 ) {
                            RCt = XFSDocGet ( Doc, & Text );
                            if ( RCt == 0 ) {
                                RCt = KFileWrite (
                                                EncFile,
                                                0,
                                                Text,
                                                ( size_t ) TextSize,
                                                & NumWrit
                                                );
                            }
                        }
                    }

                    KFileRelease ( EncFile );
                }
            }

        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _StoreContentDocument () */

/*))
 //     Method is not synchronized, so lock it before, unlock after
((*/
static
rc_t CC
_SyncronizeDirectoryContentNoLock ( const struct _DirE * self )
{
    rc_t RCt;
    const struct XFSDoc * Doc;

    RCt = 0;
    Doc = NULL;

    XFS_CAN ( self );

    if ( self -> content != NULL ) {
        RCt = _MakeContentDocument ( self, & Doc );
        if  ( RCt == 0 ) {

            RCt = _StoreContentDocument ( self, Doc );

            XFSDocRelease ( Doc );
        }
    }

    return RCt;
}   /* _SynchronizeDirectoryContentNoLock () */

static
void CC
_DirCWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        _DirERelease ( ( const struct _DirE * ) Node );
    }
}   /* _DirCWhackCallback () */

static
rc_t CC
_DirEDisposeContent ( const struct _DirE * self )
{
    struct _DirC * Content;

    if ( self != NULL ) {

        Content = ( struct _DirC * ) self -> content;
        if ( Content != NULL ) {
            BSTreeWhack (
                        & ( Content -> tree ),
                        _DirCWhackCallback,
                        NULL
                        );

            if ( Content -> path != NULL ) {
                free ( ( char * ) Content -> path );

                Content -> path = NULL;
            }

            Content -> last = 0;

            free ( Content );
        }

        ( ( struct _DirE * ) self ) -> content = NULL;
    }

    return 0;
}   /* _DirEDisposeContent () */

static
rc_t CC
_DirEVMakeContent (
            const struct _DirE * self,
            const struct KKey * Key,
            const char * Format,
            va_list Args
)
{
    rc_t RCt;
    struct _DirC * Cont;
    const struct XFSPath * Path;
    va_list xArgs;

    RCt = 0;
    Cont = NULL;
    Path = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )
    XFS_CAN ( Key )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self -> content != NULL ) {
        return XFS_RC ( rcInvalid );
    }

    va_copy ( xArgs, Args );
    RCt = XFSPathVMakeAbsolute ( & Path, false, Format, xArgs );
    va_end ( xArgs );
    if ( RCt != 0 ) {
        return XFS_RC ( rcInvalid );
    }

    Cont = calloc ( 1, sizeof ( struct _DirC ) );
    if ( Cont == NULL ) {
        XFSPathRelease ( Path );

        return XFS_RC ( rcExhausted );
    }

    ( ( struct _DirE * ) self ) -> content = Cont;

    RCt = XFS_StrDup ( XFSPathGet ( Path ), & ( Cont -> path ) );
    if ( RCt == 0 ) {
        BSTreeInit ( & ( Cont -> tree ) );

        memmove ( & ( Cont -> key ), Key, sizeof ( struct KKey ) );

        RCt = _DirEReadContent ( self );
    }

    XFSPathRelease ( Path );

    if ( RCt != 0 ) {
        _DirEDisposeContent ( self );
    }

    return RCt;
}   /* _DirEVMakeContent () */

static
rc_t CC
_DirEMakeContent (
            const struct _DirE * self,
            const struct KKey * Key,
            const char * Format,
            ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Format );
    RCt = _DirEVMakeContent ( self, Key, Format, Args );
    va_end ( Args );

    return RCt;
}   /* _DirEMakeContent () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   
 ||     NOTE: all non-locking methods which modify Content does not
 ||           store Content, while all locking methods forcely store
 ||           modified content to the disk
 ||     There are Methods : Rename, Delete, Create
 ||           All those are implemented in both locking/non-locking way
 ||           Methods are selfexplainable, except Create, which will 
 ||           return existing entry if it exists.
 ||     There is list of methods which do not modify Content : Get and
 ||           List. Those methods are implemented in both locking/non-
 ||           locking way, but locking implementation does not perform
 ||           synchronisation of Content on disk.
((*/

static
int64_t CC
_DirFindCallback ( const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                    ? NULL
                    : ( ( struct _DirE * ) Node ) -> name
                    ;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _DirFindCallback () */

static
rc_t CC
_DirEGetEntryNoLock (
                const struct _DirE * self,
                const char * Name,
                const struct _DirE ** Entry
)
{
    const struct _DirE * RetVal = NULL;

    XFS_CSAN ( Entry )
    XFS_CAN ( self )
    XFS_CAN ( self -> content )
    XFS_CAN ( Name )
    XFS_CAN ( Entry )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RetVal = ( const struct _DirE * ) BSTreeFind (
                                        & ( self -> content -> tree ),
                                        Name,
                                        _DirFindCallback
                                        );
    if ( RetVal != NULL ) {
        if ( _DirEAddRef ( RetVal ) == 0 ) {
            * Entry = RetVal;
        }
        else {
            RetVal = NULL;
        }
    }

    return RetVal == NULL ? XFS_RC ( rcNotFound ) : 0;
}   /* _DirEGetEntryNoLock () */

static
rc_t CC
_DirEGetEntry (
                const struct _DirE * self,
                const char * Name,
                const struct _DirE ** Entry
)
{
    rc_t RCt;

    RCt = 0;

    XFS_CSAN ( Entry )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Entry )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    LOG_LOC_ACQ ( __LINE__, self -> mutabor );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _DirEGetEntryNoLock ( self, Name, Entry );

        LOG_LOC_UNL ( __LINE__, self -> mutabor );
        KLockUnlock ( self -> mutabor );
    }
    return RCt;
}   /* _DirEGetEntry () */

static
void CC
_DirCModifyLast ( const struct _DirC * self, const char * Name )
{
    const char * pStart, * pEnd;
    struct _DirC * Content;
    size_t NameLen;
    uint64_t Series;

    pStart = pEnd = NULL;
    Content = ( struct _DirC * ) self;
    NameLen = 0;
    Series = 0;

    if ( Content == NULL || Name == NULL ) {
        return;
    }

    if ( string_size ( Name ) < _NAME_FORMAT_LEN_ ) {
        return;
    }

    if ( * Name == '.' ) {
        pStart = Name + 1;

        NameLen = string_size ( pStart );
        if ( _NAME_FORMAT_LEN_ <= NameLen ) {
            pEnd = pStart + _NAME_FORMAT_LEN_;

            while ( pStart < pEnd ) {
                if ( * pStart != '0' ) {
                    break;
                }

                pStart ++;

            }
            if ( pStart != pEnd ) {
                Series = string_to_U64 ( pStart, pEnd - pStart, NULL );
                if ( Content -> last <= Series ) {
                    Content -> last = Series;
                }
            }
        }
    }
}   /* _DirCModifyLast () */

static
int64_t CC
_DirEAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
                                    ( ( struct _DirE * ) N1 ) -> name,
                                    ( ( struct _DirE * ) N2 ) -> name
                                    );
}   /* _DirEAddCallback () */

static
rc_t CC
_DirEAddEntryNoLock (
                    const struct _DirE * self,
                    const struct _DirE * Entry
)
{
    rc_t RCt;

    RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> content )
    XFS_CAN ( Entry )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = BSTreeInsert (
                    ( struct BSTree * ) & ( self -> content -> tree ),
                    ( struct BSTNode * ) & ( Entry -> node ),
                    _DirEAddCallback
                    );
    if ( RCt == 0 ) {
        _DirCModifyLast ( self -> content, Entry -> eff_name );
    }

    return RCt;
}   /* _DirEAddEntryNoLock () */

static
rc_t CC
_DirEDelEntryNoLock (
            const struct _DirE * self,
            const struct _DirE * Entry
)
{
    XFS_CAN ( self )
    XFS_CAN ( self -> content )
    XFS_CAN ( Entry )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    BSTreeUnlink (
                ( struct BSTree * ) & ( self -> content -> tree ),
                ( struct BSTNode * ) & ( Entry -> node )
                );

    return _DirERelease ( Entry );
}   /* _DirEDelEntryNoLock () */

static
rc_t CC
_DirERenameEntryNoLock (
                    const struct _DirE * self,
                    const char * OldName,
                    const char * NewName
)
{
    rc_t RCt;
    struct _DirE * Entry;

    RCt = 0;
    Entry = NULL;

    XFS_CAN ( self )
    XFS_CAN ( OldName )
    XFS_CAN ( NewName )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _DirEGetEntryNoLock (
                            self,
                            OldName,
                            ( const struct _DirE ** ) & Entry
                            );
    if ( RCt == 0 ) {
        if ( Entry != NULL ) {
            RCt = _DirEDelEntryNoLock ( self, Entry );
            if ( RCt == 0 ) {
                free ( ( char * ) Entry -> name );
                Entry -> name = NULL;

                RCt = XFS_StrDup ( NewName, & ( Entry -> name ) );
                if ( RCt == 0 ) {
                    RCt = _DirEAddEntryNoLock ( self, Entry );
                }
            }
        }

        // _DirERelease ( Entry );
    }

    return RCt;
}   /* _DirERenameEntryNoLock () */

/*)) Rather long and weird method, which should be revisited TODO TODO
 ((*/
static
rc_t CC
_DirEMoveEntry (
                    const struct _DirE * OldParent,
                    const char * OldName,
                    const struct _DirE * NewParent,
                    const char * NewName
)
{
    rc_t RCt;
    struct KDirectory * NatDir;
    const struct _DirE * Entry;
    const struct _DirE * TmpEntry;
    const struct XFSPath * OldPath;
    const struct XFSPath * NewPath;
    char BF [ 32 ];

    RCt = 0;
    NatDir = NULL;
    Entry = NULL;
    TmpEntry = NULL;
    OldPath = NULL;
    NewPath = NULL;
    * BF = 0;

    XFS_CAN ( OldParent )
    XFS_CAN ( OldName )
    XFS_CAN ( NewParent )
    XFS_CAN ( NewName )

        /* Locking first and accessing Entry to move
         */
    LOG_LOC_ACQ ( __LINE__, OldParent -> mutabor );
    RCt = KLockAcquire ( OldParent -> mutabor );
    if ( RCt == 0 ) {
            /* First we are getting the entry to move 
             */
        RCt = _DirEGetEntryNoLock ( OldParent , OldName, & Entry );

        LOG_LOC_UNL ( __LINE__, OldParent -> mutabor );
        KLockUnlock ( OldParent -> mutabor );
    }

    if ( RCt == 0 ) {
            /* Locking and creating space for Entry to move
             */
        LOG_LOC_ACQ ( __LINE__, NewParent -> mutabor );
        RCt = KLockAcquire ( NewParent -> mutabor );
        if ( RCt == 0 ) {
            RCt = _DirEGetEntryNoLock (
                                     NewParent ,
                                     NewName,
                                     & TmpEntry
                                     );
            if ( RCt == 0 ) {
                RCt = _DirEDelEntryNoLock ( NewParent, TmpEntry );

                _DirERelease ( TmpEntry );
            }
            else {
                RCt = 0;
            }

            LOG_LOC_UNL ( __LINE__, NewParent -> mutabor );
            KLockUnlock ( NewParent -> mutabor );
        }
    }

    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & NatDir );

        if ( RCt == 0 ) {
                /* Here we are moving real file
                 */
            RCt = XFSPathMakeAbsolute (
                                    & OldPath,
                                    false,
                                    "%s/%s",
                                    OldParent -> content -> path,
                                    Entry -> eff_name
                                    );
            if ( RCt == 0 ) {
                RCt = _DirCNewName (
                                NewParent -> content,
                                Entry -> is_folder,
                                BF,
                                sizeof ( BF )
                                );
                if ( RCt == 0 ) {
                    RCt = XFSPathMakeAbsolute (
                                        & NewPath,
                                        false,
                                        "%s/%s",
                                        NewParent -> content -> path,
                                        BF
                                        );
                    if ( RCt == 0 ) {
                        RCt = KDirectoryRename (
                                                NatDir,
                                                true, /* FORCE */
                                                XFSPathGet ( OldPath ),
                                                XFSPathGet ( NewPath )
                                                );
                        XFSPathRelease ( NewPath );
                    }
                }

                XFSPathRelease ( OldPath );
            }

            KDirectoryRelease ( NatDir );
        }
    }

        /* Here we are deleteing old entry and synchronizing
         */
    if ( RCt == 0 ) {
        LOG_LOC_ACQ ( __LINE__, OldParent -> mutabor );
        RCt = KLockAcquire ( OldParent -> mutabor );
        if ( RCt == 0 ) {
            RCt = _DirEDelEntryNoLock ( OldParent, Entry );
            if ( RCt == 0 ) {
                RCt = _SyncronizeDirectoryContentNoLock ( OldParent );
            }
            LOG_LOC_UNL ( __LINE__, OldParent -> mutabor );
            KLockUnlock ( OldParent -> mutabor );
        }
    }

        /* Here we are adding new 'old' entry and synchronizing
         */
    if ( RCt == 0 ) {
        LOG_LOC_ACQ ( __LINE__, NewParent -> mutabor );
        RCt = KLockAcquire ( NewParent -> mutabor );
        if ( RCt == 0 ) {
                /*  First, we should setup new : Name, EffName and Path,
                 *  if it is a Folder
                 */

            free ( ( char * ) Entry -> name );
            RCt = XFS_StrDup (
                            NewName,
                            ( const char ** ) & ( Entry -> name )
                            );
            if ( RCt == 0 ) {

                free ( ( char * ) Entry -> eff_name );
                RCt = XFS_StrDup (
                            BF,
                            ( const char ** ) & ( Entry -> eff_name )
                            );
                if ( RCt == 0 ) {
                    if ( Entry -> is_folder ) {
                        RCt = XFSPathMakeAbsolute (
                                         & NewPath,
                                         false,
                                         "%s/%s",
                                         NewParent -> content -> path,
                                         BF
                                         );
                        if ( RCt == 0 ) {

                            free ( ( char * ) Entry -> content -> path );
                            XFS_StrDup (
                                    XFSPathGet ( NewPath ),
                                    ( const char ** ) & ( Entry -> content -> path )
                                    );

                            XFSPathRelease ( NewPath );
                        }

                    }

                    if ( RCt == 0 ) {
                        RCt = _DirEAddEntryNoLock ( NewParent, Entry );
                        if ( RCt == 0 ) {
                            RCt = _SyncronizeDirectoryContentNoLock (
                                                            NewParent
                                                            );
                        }
                    }
                }
            }
        }

        LOG_LOC_UNL ( __LINE__, NewParent -> mutabor );
        KLockUnlock ( NewParent -> mutabor );
    }

    return RCt;
}   /* _DirEMoveEntry () */

static
rc_t CC
_DirECreateEntryNoLock (
                const struct _DirE * self,
                const char * Name,
                bool IsFolder,
                const struct _DirE ** Entry
)
{
    rc_t RCt;
    const struct _DirE * RetVal;
    char Buf [ XFS_SIZE_64 ];

    RCt = 0;
    RetVal = NULL;
    * Buf = 0;

    XFS_CSAN ( Entry )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Entry )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _DirEGetEntryNoLock ( self, Name, & RetVal );
    if ( GetRCState ( RCt ) == rcNotFound ) {
        RCt = _DirCNewName (
                            self -> content,
                            IsFolder,
                            Buf,
                            sizeof ( Buf )
                            );
        if ( RCt == 0 ) {
            RCt = _DirEMake (
                            ( IsFolder ? _sFolderTag : _sFileTag ),
                            Buf,
                            Name,
                            & RetVal
                            );
            if ( RCt == 0 ) {
                RCt = _DirEAddEntryNoLock ( self, RetVal );
            }
        }
    }
    else {
        if ( RCt == 0 ) {
            _DirERelease ( RetVal );
        }
    }

    if ( RCt == 0 ) {
        * Entry = RetVal;
    }
    else {
        if ( RetVal != NULL ) {
            _DirEDispose ( RetVal );
        }
    }

    return RCt;
}   /* _DirECreateEntryNoLock () */

static
bool CC 
_DirEHasEntriesCallback ( BSTNode *n, void *data )
{
    return true;
}   /* _DirEHasEntriesCallback () */

static
bool CC
_DirEHasEntries ( const struct _DirE * self )
{
    if ( self == NULL ) {
        return false;
    }

    if ( ! self -> is_folder ) {
        return false;
    }

    if ( self -> content == NULL ) {
        return false;
    }

    return  BSTreeDoUntil (
                        & ( self -> content -> tree ),
                        false,
                        _DirEHasEntriesCallback,
                        NULL
                        );
}   /* _DirEHasEntries () */

static
void
_DirEListCalback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct _DirE * Entry;

    List = ( struct VNamelist * ) Data;
    Entry = ( struct _DirE * ) Node;

    if ( List != NULL && Entry != NULL ) {
        VNamelistAppend ( List, Entry -> name );
    }
}   /* _DirEListCalback () */

static
rc_t CC
_DirEListEntriesNoLock (
                const struct _DirE * self,
                struct KNamelist ** List
)
{
    rc_t RCt;
    struct VNamelist * xList;

    RCt = 0;
    xList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( self -> content )
    XFS_CAN ( List )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = VNamelistMake ( & xList, 16 );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    & ( self -> content -> tree ),
                    false,
                    _DirEListCalback,
                    xList
                    );

        RCt = VNamelistToNamelist ( xList, List );

        VNamelistRelease ( xList );
    }


    return RCt;
}   /* _DirEListEntries () */

static
rc_t CC
_DirEVisitNoLock (
                const struct _DirE * self, 
                const struct KDirectory * Dir,
                bool Recurse,
                rc_t ( CC * Func ) (
                            const struct KDirectory *,
                            uint32_t,
                            const char *,
                            void *
                            ), 
                void * Data,
                const char * Path
)
{
    rc_t RCt;
    struct KNamelist * List;
    uint32_t ListQty;
    uint32_t Idx;
    const struct _DirE * Entry;
    const char * Name;
    char BF [ XFS_SIZE_4096 ];
    size_t NumW;

    RCt = 0;
    List = NULL;
    ListQty = 0;
    Idx = 0;
    Entry = NULL;
    Name = NULL;
    * BF = 0;
    NumW = 0;

    XFS_CAN ( self )
    XFS_CAN ( Func )
    XFS_CAN ( Path )

    LOG_LOC_ACQ ( __LINE__, self -> mutabor );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = _DirEListEntriesNoLock ( self, & List );

        LOG_LOC_UNL ( __LINE__, self -> mutabor );
        KLockUnlock ( self -> mutabor );
    }

    if ( RCt == 0 ) {
        RCt = KNamelistCount ( List, & ListQty );
        if ( RCt == 0 ) {
            for ( Idx = 0; ( Idx < ListQty ) && ( RCt == 0 ); Idx ++ ) {
                RCt = KNamelistGet ( List, Idx, & Name );
                if ( RCt == 0 ) {
                    RCt = _DirEGetEntry ( self, Name, & Entry );
                    if ( RCt == 0 ) {

                        RCt = string_printf ( 
                                            BF,
                                            sizeof ( BF ),
                                            & NumW,
                                            "%s/%s",
                                            Path,
                                            Name
                                            );
                        if ( RCt == 0 ) {
                            RCt = Func ( 
                                        Dir,
                                        Entry -> is_folder
                                                        ? kptDir
                                                        : kptFile
                                                        ,
                                        BF,
                                        Data
                                        );
                            if ( RCt == 0 ) {
                                if ( Entry -> is_folder && Recurse ) {
                                    RCt = _DirEVisitNoLock (
                                                            Entry,
                                                            Dir,
                                                            Recurse,
                                                            Func,
                                                            Data,
                                                            BF
                                                            );
                                }
                            }
                        }

                        _DirERelease ( Entry );
                    }
                }
            }
        }

        KNamelistRelease ( List );
    }

    return RCt;
}   /* _DirEVisitNoLock () */

/*))    That method will get or create content from entry.
 ((*/
static
rc_t CC
_DirECheckLoadContent (
                const struct _DirE * self,
                const struct KKey * Key,
                const char * Format,
                ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Key )
    XFS_CAN ( Format )

    if ( ! self -> is_folder ) {
        return XFS_RC ( rcInvalid );
    }

    LOG_LOC_ACQ ( __LINE__, self -> mutabor );
    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
            /*)) Check if Content already loaded ((*/
        if ( self -> content == NULL ) {

                /*)) Tying to load Content ((*/
            va_start ( Args, Format );
            RCt = _DirEVMakeContent ( self, Key, Format, Args );
            va_end ( Args );
        }

        if ( self -> content == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }

        LOG_LOC_UNL ( __LINE__, self -> mutabor );
        KLockUnlock ( self -> mutabor );

    }

    return RCt;
}   /* _DirECheckLoadContent () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSWsDir {
    struct KDirectory dir;

    const char * passwd;
    const char * enc_type;
    bool update;

    const struct _DirE * entry;
};

static
rc_t CC
_WsDirEntry (
            const struct XFSWsDir * self,
            const struct _DirE ** Entry
)
{
    rc_t RCt = 0;

    XFS_CSAN ( Entry )
    XFS_CAN ( self )
    XFS_CAN ( Entry )

    XFS_CAN ( self -> entry ) 

    RCt = _DirEAddRef ( self -> entry );
    if ( RCt == 0 ) {
        * Entry = self -> entry;
    }

    return RCt;
}   /* _WsDirEntry () */

static
const struct KKey * CC
_WsDirKey ( const struct XFSWsDir * self )
{
    if ( self != NULL ) {
        if ( self -> entry != NULL ) {
            if ( self -> entry -> content != NULL ) {
                return & ( self -> entry -> content -> key );
            }
        }
    }
    return NULL;
}   /* _WsDirKey () */

static
const char * CC
_WsDirPath ( const struct XFSWsDir * self )
{
    if ( self != NULL ) {
        if ( self -> entry != NULL ) {
            if ( self -> entry -> content != NULL ) {
                return self -> entry -> content -> path;
            }
        }
    }
    return NULL;
}   /* _WsDirPath () */

static
rc_t CC
_WsDirMapPath (
            const struct XFSWsDir * self,
            const struct XFSPath * Path,
            const struct _DirE ** RetEntry,
            const struct XFSPath ** EffPath
)
{
    rc_t RCt;
    size_t Idx, Qty;
    const struct XFSPath * RetVal;
    const struct _DirE * Entry, * TmpEntry;

    RCt = 0;
    Idx = Qty = 0;
    RetVal = NULL;
    Entry = TmpEntry = NULL;

    XFS_CSAN ( EffPath )
    XFS_CSAN ( RetEntry )
    XFS_CAN ( self )
    XFS_CAN ( Path )
    XFS_CAN ( EffPath )
    XFS_CAN ( RetEntry )

        /*) First we are creating empty Path
         */

    RCt = XFSPathMake ( & RetVal, false, "" );
    if ( RCt == 0 ) {
        RCt = _WsDirEntry ( self, & Entry );
        if ( RCt == 0 ) {

            Qty = XFSPathPartCount ( Path );
            for ( Idx = 0; Idx < Qty; Idx ++ ) {
                RCt = _DirEGetEntry (
                                    Entry,
                                    XFSPathPartGet ( Path, Idx ),
                                    & TmpEntry
                                    );

                if ( RCt != 0 ) {
                    break;
                }

                if ( TmpEntry == NULL ) {
                    RCt = XFS_RC ( rcNotFound );
                    break;
                }

                if ( TmpEntry -> is_folder ) {
                    RCt = _DirECheckLoadContent (
                                    TmpEntry,
                                    & ( Entry -> content -> key ),
                                    "%s/%s",
                                    Entry -> content -> path,
                                    TmpEntry -> eff_name
                                    );
                    if ( RCt != 0 ) {
                        break;
                    }
                }
                else {
                    if ( ( Idx + 1 ) < Qty ) {
                        RCt = XFS_RC ( rcInvalid );
                        break;
                    }
                }

                RCt = XFSPathAppend ( RetVal, TmpEntry -> eff_name );
                if ( RCt != 0 ) {
                    break;
                }

                _DirERelease ( Entry );

                Entry = TmpEntry;
                TmpEntry = NULL;
            }

            if ( RCt == 0 ) {;
                    /* Should we check Entry for NULL ? */
                * EffPath = RetVal;

                * RetEntry = Entry;
            }
        }
    }

    if ( RCt != 0 ) {
        * EffPath = NULL;
        * RetEntry = NULL;

        if ( RetVal != NULL ) {
            XFSPathRelease ( RetVal );
        }

        if ( Entry != NULL ) {
            _DirERelease ( Entry );
        }
    }

    return RCt;
}   /* _WsDirMapPath () */

static
rc_t
_WsDirVMapIt (
            const struct XFSWsDir * self,
            const struct _DirE ** Entry,
            const struct XFSPath ** EffPath,   /* Could be NULL */
            const char * Format,
            va_list Args
)
{
    rc_t RCt;
    const struct XFSPath * WsPath;
    const struct XFSPath * DerPath;
    const struct XFSPath * TmpPath;
    const struct XFSPath * ThePath;
    const struct _DirE * TheEntry;
    va_list xArgs;

    RCt = 0;
    WsPath = NULL;
    DerPath = NULL;
    TmpPath = NULL;
    ThePath = NULL;
    TheEntry = NULL;

    XFS_CSAN( Entry )
    XFS_CSAN( EffPath )
    XFS_CAN ( self )
    XFS_CAN ( Format )

        /* First we shoud make sure that path is our.
         | I mean, path should be part of Directory path.
         */
            /*) Orig Path
             (*/
    RCt = XFSPathMakeAbsolute ( & WsPath, false, _WsDirPath ( self ) );
    if ( RCt == 0 ) {
            /*) Absolute Input Path
             (*/
        va_copy ( xArgs, Args );
        RCt = XFSPathVMakeAbsolute ( & DerPath, false, Format, xArgs );
        va_end ( xArgs );

        if ( RCt == 0 ) {
            if ( XFSPathIsChild ( WsPath, DerPath, & TmpPath ) ) {
                    /*) Here we are mapping effective path
                     (*/
                RCt = _WsDirMapPath (
                                    self,
                                    TmpPath,
                                    & TheEntry,
                                    & ThePath
                                    );
                if ( RCt == 0 ) {
                    * Entry = TheEntry;

                    if ( EffPath != NULL ) {
                        XFSPathAddRef ( ThePath );
                        * EffPath = ThePath;
                    }

                    XFSPathRelease ( ThePath );
                }

                XFSPathRelease ( TmpPath );
            }
            else {
                RCt = XFS_RC ( rcOutOfKDirectory );
            }

            XFSPathRelease ( DerPath );
        }

        XFSPathRelease ( WsPath );
    }

    if ( RCt != 0 ) {
        if ( EffPath != NULL ) {
            * EffPath = NULL;
        }
    }

    return RCt;
}   /* _WsDirVMapIt () */

static
rc_t
_WsDirMapIt (
            const struct XFSWsDir * self,
            const struct _DirE ** Entry,
            const struct XFSPath ** EffPath,   /* Could be NULL */
            const char * Format,
            ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Format );
    RCt = _WsDirVMapIt ( self, Entry, EffPath, Format, Args );
    va_end ( Args );

    return RCt;
}   /* _WsDirMapIt () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Cryptonomifile-ile-wile                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 (((    Here is some lyrics. Current version of KEncFile is not 
  )))   thread safe, so I had to add mutex here. However, the only
 (((    way to create KEncFile is call "KEncFileMakeInt ()" which
  )))   allocates KEncFile, and theree is no any 'Init" methods.
 (((    So, we can not just do it in normal 'inheritance way' That's
  )))   why there is that ugle-moogle
 (((*/
struct _xEncFile {
    struct KFile file;

    struct KFile * enc_file;

    struct KLock * mutabor;
};

static
rc_t CC
_EncFileDestroy ( struct KFile * self )
{
    struct _xEncFile * File;

    File = ( struct _xEncFile * ) self;

    if ( File != NULL ) {

        if ( File -> mutabor != NULL ) {
            KLockRelease ( File -> mutabor );

            File -> mutabor = NULL;
        }

        if ( File -> enc_file != NULL ) {
            KFileRelease ( File -> enc_file );

            File -> enc_file = NULL;
        }

        free ( File );
    }

    return 0;
}   /* _EncFileDestroy () */

static
struct KSysFile * CC
_EncFileGetSysFile ( const struct KFile * self, uint64_t * Offset )
{
    struct _xEncFile * File = ( struct _xEncFile * ) self;

    if ( File != NULL ) {
        if ( File -> enc_file != NULL ) {
            return KFileGetSysFile ( File -> enc_file, Offset );
        }
    }

    return NULL;
}   /* _EncFileGetSysFile () */

static
rc_t CC
_EncFileRandomAccess ( const struct KFile * self )
{
    const struct _xEncFile * File = ( const struct _xEncFile * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> enc_file )

    return KFileRandomAccess ( File -> enc_file );
}   /* _EncFileRandomAccess () */

static
rc_t CC
_EncFileSize ( const struct KFile * self, uint64_t * Size )
{
    rc_t RCt;
    const struct _xEncFile * File;

    RCt = 0;
    File = ( const struct _xEncFile * ) self;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Size )
    XFS_CAN ( File -> enc_file )
    XFS_CAN ( File -> mutabor )

    LOG_LOC_ACQ ( __LINE__, File -> mutabor );
    RCt = KLockAcquire ( File -> mutabor );
    if ( RCt == 0 ) {
        RCt = KFileSize ( File -> enc_file, Size );

        LOG_LOC_UNL ( __LINE__, File -> mutabor );
        KLockUnlock ( File -> mutabor );
    }

    return RCt;
}   /* _EncFileSize () */

static
rc_t CC
_EncFileSetSize ( struct KFile * self, uint64_t Size )
{
    rc_t RCt;
    const struct _xEncFile * File;

    RCt = 0;
    File = ( const struct _xEncFile * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> enc_file )
    XFS_CAN ( File -> mutabor )

    LOG_LOC_ACQ ( __LINE__, File -> mutabor );
    RCt = KLockAcquire ( File -> mutabor );
    if ( RCt == 0 ) {
        RCt = KFileSetSize ( File -> enc_file, Size );

        LOG_LOC_UNL ( __LINE__, File -> mutabor );
        KLockUnlock ( File -> mutabor );
    }

    return RCt;
}   /* _EncFileSetSize () */

static
rc_t CC
_EncFileRead (
            const struct KFile * self,
            uint64_t Pos,
            void * Bf,
            size_t BfSz,
            size_t * NumRead
)
{
    rc_t RCt;
    const struct _xEncFile * File;

    RCt = 0;
    File = ( const struct _xEncFile * ) self;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Bf )
    XFS_CAN ( NumRead )
    XFS_CAN ( File -> enc_file )
    XFS_CAN ( File -> mutabor )

    LOG_LOC_ACQ ( __LINE__, File -> mutabor );
    RCt = KLockAcquire ( File -> mutabor );
    if ( RCt == 0 ) {
        RCt = KFileRead ( File -> enc_file, Pos, Bf, BfSz, NumRead );

        LOG_LOC_UNL ( __LINE__, File -> mutabor );
        KLockUnlock ( File -> mutabor );
    }

    return RCt;
}   /* _EncFileRead () */

static
rc_t CC
_EncFileWrite (
            struct KFile * self,
            uint64_t Pos,
            const void * Bf,
            size_t BfSz,
            size_t * NumWrote
)
{
    rc_t RCt;
    struct _xEncFile * File;

    RCt = 0;
    File = ( struct _xEncFile * ) self;

    XFS_CSA ( NumWrote, 0 )
    XFS_CAN ( File )
    XFS_CAN ( Bf )
    XFS_CAN ( NumWrote )
    XFS_CAN ( File -> enc_file )
    XFS_CAN ( File -> mutabor )

    LOG_LOC_ACQ ( __LINE__, File -> mutabor );
    RCt = KLockAcquire ( File -> mutabor );
    if ( RCt == 0 ) {
        RCt = KFileWrite ( File -> enc_file, Pos, Bf, BfSz, NumWrote );

        LOG_LOC_UNL ( __LINE__, File -> mutabor );
        KLockUnlock ( File -> mutabor );
    }

    return RCt;
}   /* _EncFileWrite () */

static
uint32_t CC
_EncFileType ( const struct KFile * self )
{
    const struct _xEncFile * File = ( const struct _xEncFile * ) self;

    if ( File != NULL ) {
        if ( File -> enc_file != NULL ) {
            return KFileType ( File -> enc_file );
        }
    }

    return kptFile;
}   /* _EncFileType () */
                                
static struct KFile_vt_v1 _svxEncFile = {
                                        /* version */
                                    1, 1,
                                
                                        /* 1.0 */
                                    _EncFileDestroy,
                                    _EncFileGetSysFile,
                                    _EncFileRandomAccess,
                                    _EncFileSize,
                                    _EncFileSetSize,
                                    _EncFileRead,
                                    _EncFileWrite,
                                
                                        /* 1.1 */
                                    _EncFileType
                                    };

static
rc_t CC
_EncFileMake ( struct KFile * EncFile, struct KFile ** RetFile )
{
    rc_t RCt;
    struct _xEncFile * File;

    RCt = 0;
    File = NULL;

    XFS_CSAN ( RetFile )
    XFS_CAN ( EncFile )
    XFS_CAN ( RetFile )

    File = calloc ( 1, sizeof ( struct _xEncFile ) );
    if ( File == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KFileInit (
                        & ( File -> file ),
                        ( const KFile_vt * ) & _svxEncFile,
                        "XFSEncFile",
                        "EncFile",
                        EncFile -> read_enabled,
                        EncFile -> write_enabled
                        );
        if ( RCt == 0 ) {
            RCt = KLockMake ( & ( File -> mutabor ) );
            if ( RCt == 0 ) {
                File -> enc_file = EncFile;

                * RetFile = & ( File -> file );
            }
        }
    }

    if ( RCt != 0 ) {
        * RetFile = NULL;

        KFileRelease ( & ( File -> file ) );
    }

    return RCt;
}   /* _EncFileMake () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Cryptonomicom-om-um                                              */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_OpenEncryptedFileRead (
                const struct KFile ** File,
                struct KKey * Key,
                const char * Path,
                ...
)
{
    rc_t RCt;
    struct KDirectory * nDir;
    const struct KFile * rFile, * xFile;
    va_list Args;

    RCt = 0;
    nDir = NULL;
    rFile = xFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )
    XFS_CAN ( Key )
    XFS_CAN ( Path )

    RCt = KDirectoryNativeDir ( & nDir );
    if ( RCt == 0 ) {
        va_start ( Args, Path );
        RCt = KDirectoryVOpenFileRead ( nDir, & rFile, Path, Args );
        va_end ( Args );
        if ( RCt == 0 ) {
            RCt = KEncFileMakeRead ( & xFile, rFile, Key );

            KFileRelease ( rFile );
            if ( RCt == 0 ) {
                RCt = _EncFileMake ( ( struct KFile * ) xFile, ( struct KFile ** ) File );
            }
else { RCt = XFS_RC ( rcBusy ); pLogMsg ( klogDebug, " RET_BUSY [$(line)]", "line=%d", __LINE__ ); }
        }

        KDirectoryRelease ( nDir );
    }

    return RCt;
}   /* _OpenEncryptedFileRead () */

static
rc_t CC
_OpenVEncryptedFileWrite (
                struct KFile ** File,
                struct KKey * Key,
                bool Update,
                const char * Path,
                va_list VaList
)
{
    rc_t RCt;
    struct KDirectory * nDir;
    struct KFile * wFile, * xFile;
    va_list Args;

    RCt = 0;
    nDir = NULL;
    wFile = xFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )
    XFS_CAN ( Key )
    XFS_CAN ( Path )

/* because of we unable to read encrypted file header in Write Only
 * mode we are always opening files in Update mode. I have no idea how
 * to fix that problem. Thinking about it.
 */
#ifdef _ENCODED_FILE_UPDATE_ALWAYS_
    Update = true;
#endif /* _ENCODED_FILE_UPDATE_ALWAYS_ */

    RCt = KDirectoryNativeDir ( & nDir );
    if ( RCt == 0 ) {
        va_copy ( Args, VaList );
        RCt = KDirectoryVOpenFileWrite (
                                        nDir,
                                        & wFile,
                                        Update,
                                        Path,
                                        Args
                                        );
        va_end ( Args );
        if ( RCt == 0 ) {
            RCt = Update
                    ? KEncFileMakeUpdate ( & xFile, wFile, Key )
                    : KEncFileMakeWrite ( & xFile, wFile, Key )
                    ;
            KFileRelease ( wFile );

            if ( RCt == 0 ) {
                RCt = _EncFileMake ( ( struct KFile * ) xFile, File );
            }
else { RCt = XFS_RC ( rcBusy ); pLogMsg ( klogDebug, " RET_BUSY [$(line)]", "line=%d", __LINE__ ); }

        }

        KDirectoryRelease ( nDir );
    }

    return RCt;
}   /* _OpenVEncryptedFileWrite () */

static
rc_t CC
_OpenEncryptedFileWrite (
                struct KFile ** File,
                struct KKey * Key,
                bool Update,
                const char * Path,
                ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    va_start ( Args, Path );
    RCt = _OpenVEncryptedFileWrite ( File, Key, Update, Path, Args );
    va_end ( Args );

    return RCt;
}   /* _OpenEncryptedFileWrite () */

static
rc_t CC
_CreateEncryptedFile (
                struct KFile ** File,
                const struct KKey * Key,
                bool Update,
                uint32_t Access,
                KCreateMode Cmode,
                const char * Format,
                ...
)
{
    rc_t RCt;
    struct KDirectory * NativeDir;
    struct KFile * TheFile, * EncFile;
    va_list Args;

    RCt = 0;
    NativeDir = NULL;
    TheFile = EncFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )
    XFS_CAN ( Key )
    XFS_CAN ( Format )

/* because of we unable to read encrypted file header in Write Only
 * mode we are always opening files in Update mode. I have no idea how
 * to fix that problem. Thinking about it.
 */
#ifdef _ENCODED_FILE_UPDATE_ALWAYS_
    Update = true;
#endif /* _ENCODED_FILE_UPDATE_ALWAYS_ */

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
            /* Create File */
        va_start ( Args, Format );
        RCt = KDirectoryVCreateFile (
                                    NativeDir,
                                    & TheFile,
                                    Update,
                                    Access,
                                    Cmode,
                                    Format,
                                    Args
                                    );
        va_end ( Args );
        if ( RCt == 0 ) {
                /* Create Encrypted File */
            RCt = Update
                        ? KEncFileMakeUpdate ( & EncFile, TheFile, Key )
                        : KEncFileMakeWrite ( & EncFile, TheFile, Key )
                        ;

            KFileRelease ( TheFile );

            if ( RCt == 0 ) {
                RCt = _EncFileMake ( ( struct KFile * ) EncFile, File );
                // * File = EncFile;
            }
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _CreateEncryptedFile () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Some common things                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*||+\
 +||+   Don't forget to free 'EntryName'
\+||*/
static
rc_t CC
_GetNameAndMapParentEntryNoLock (
                        struct  XFSWsDir * self,
                        const struct _DirE ** ParentEntry,
                        const struct XFSPath ** EffPath,
                        const char ** EntryName,
                        const char * Format,
                        va_list Args
)
{
    rc_t RCt;
    const struct _DirE * RetEntry;
    const struct XFSPath * RetEffPath;
    const struct XFSPath * ThePath, * ParPath;
    va_list xArgs;

    RCt = 0;
    RetEntry = NULL;
    RetEffPath = NULL;
    ThePath = ParPath = NULL;

    XFS_CSAN ( ParentEntry )
    XFS_CSAN ( EffPath )
    XFS_CSAN ( EntryName )
    XFS_CAN ( self )
    XFS_CAN ( ParentEntry )
    XFS_CAN ( EffPath )
    XFS_CAN ( EntryName )
    XFS_CAN ( Format )

        /*) Simple : Map parent directory, create file, add record about
         (*/
    va_copy ( xArgs, Args );
    RCt = XFSPathVMakeAbsolute ( & ThePath, false, Format, xArgs );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RCt = XFSPathParent ( ThePath, & ParPath );
        if ( RCt == 0 ) {
            RCt = _WsDirMapIt (
                            self,
                            & RetEntry,
                            & RetEffPath,
                            XFSPathGet ( ParPath )
                            );
            if ( RCt == 0 ) {
                RCt = _DirECheckLoadContent (
                                            RetEntry,
                                            _WsDirKey ( self ),
                                            "%s/%s",
                                            _WsDirPath ( self ),
                                            XFSPathGet ( RetEffPath )
                                            );
                if ( RCt == 0 ) {
                    RCt = XFS_StrDup (
                                    XFSPathName ( ThePath ),
                                    EntryName
                                    );
                    if ( RCt == 0 ) {
                        * ParentEntry = RetEntry;
                        * EffPath = RetEffPath;
                    }
                }
            }

            XFSPathRelease ( ParPath );
        }

        XFSPathRelease ( ThePath );
    }

    if ( RCt != 0 ) {
        * ParentEntry = NULL;
        * EffPath = NULL;

        if ( RetEffPath != NULL ) {
            XFSPathRelease ( RetEffPath );

            RetEffPath = NULL;
        }

        if ( * EntryName != NULL ) {
            free ( ( char * ) * EntryName );

            * EntryName = NULL;
        }

        if ( RetEntry != NULL ) {
            _DirERelease ( RetEntry );
        }
    }

    return RCt;
}   /* _GetNameAndMapParentEntryNoLock () */

static
rc_t CC
_GetCNameAndMapParentEntryNoLock (
                        struct  XFSWsDir * self,
                        const struct _DirE ** ParentEntry,
                        const struct XFSPath ** EffPath,
                        const char ** EntryName,
                        const char * Format,
                        ...
)
{
    rc_t RCt;

    va_list Args;

    va_start ( Args, Format );
    RCt = _GetNameAndMapParentEntryNoLock (
                                        self,
                                        ParentEntry,
                                        EffPath,
                                        EntryName,
                                        Format,
                                        Args
                                        );
    va_end ( Args );

    return RCt;
}   /* _GetCNameAndMapParentEntryNoLock () */

static
rc_t CC
_GetContentEntryAndLock (
                        const struct _DirE * Parent,
                        const struct _DirE ** Entry,
                        const char * EntryName
)
{
    rc_t RCt;
    const struct _DirE * RetEntry;

    RCt = 0;
    RetEntry = NULL;

    XFS_CSAN ( Entry )
    XFS_CAN ( Parent )
    XFS_CAN ( Entry )
    XFS_CAN ( EntryName )

        /*) Locking
         (*/
    LOG_LOC_ACQ ( __LINE__, Parent -> mutabor );
    RCt = KLockAcquire ( Parent -> mutabor );
    if ( RCt == 0 ) {
            /*) Getting content entry for file
             (*/
        RCt = _DirEGetEntryNoLock ( Parent , EntryName, & RetEntry );
        if ( RCt == 0 ) {
            * Entry = RetEntry;
        }
        else {
            LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
            KLockUnlock ( Parent -> mutabor );
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL; 
    }

    return RCt;
}   /* _GetContentEntryAndLock () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Dirty virtual table                                              */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirDestroy
 */
static
rc_t CC
XFSWsDirDestroy ( struct KDirectory * self )
{
    struct XFSWsDir * Dir = ( struct XFSWsDir * ) self;

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirDestroy] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    if ( Dir == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Dir -> entry != NULL ) {
        _DirERelease ( Dir -> entry );
        Dir -> entry = NULL;
    }

    if ( Dir -> passwd != NULL ) {
        free ( ( char * ) Dir -> passwd );
        Dir -> passwd = NULL;
    }

    if ( Dir -> enc_type != NULL ) {
        free ( ( char * ) Dir -> enc_type );
        Dir -> enc_type = NULL;
    }

    Dir -> update = false;

    free ( Dir );

    return 0;
}   /* XFSWsDirDestroy () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirList
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "."
 *
 * [RET] rc_t		    0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]	 const XFSWsDir *	self		Object oriented C; XFSWsDir object for this method
 * [OUT] KNamelist ** listp,
 * [IN]  bool ( * f )( const KDirectory *, const char *, void * )
 * [IN]  void *	data
 * [IN]  const char * path
 * [IN]  va_list args
 */
static
rc_t CC
XFSWsDirList (
                const struct KDirectory * self,
                struct KNamelist ** List,
                bool ( CC * f ) (
                            const struct KDirectory * dir,
                            const char * name,
                            void * data
                            ),
                void * Data,
                const char * Path,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * TheEffPath;
    struct KNamelist * TheList;
    va_list xArgs;

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirList] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    TheEffPath = NULL;
    TheList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )
    XFS_CAN ( Path )

        /*) first we are trying to map content entry according to path
         (*/
    va_copy ( xArgs, Args );
    RCt = _WsDirVMapIt (
                    Dir,
                    & Entry,
                    & TheEffPath,
                    Path,
                    Args
                    );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        if ( ! Entry -> is_folder || Entry -> content == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
/*)
 || IMPORTANT: For now Filtering function is not used
 || TODO!!!!!!
 (*/
            LOG_LOC_ACQ ( __LINE__, Entry -> mutabor );
            RCt = KLockAcquire ( Entry -> mutabor );
            if ( RCt == 0 ) {

                RCt = _DirEListEntriesNoLock ( Entry, & TheList );
                if ( RCt == 0 ) {
                    * List = TheList;
                }

                LOG_LOC_UNL ( __LINE__, Entry -> mutabor );
                KLockUnlock ( Entry -> mutabor );
            }
        }

        XFSPathRelease ( TheEffPath );
        _DirERelease ( Entry );
    }

    return RCt;
}   /* XFSWsDirList () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirVisit
 *  visit each path under designated directory,
 *  recursively if so indicated
 *
 *  "recurse" [ IN ] - if non-zero, recursively visit sub-directories
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to execute
 *  on each path. receives a base directory and relative path
 *  for each entry, where each path is also given the leaf name
 *  for convenience. if "f" returns non-zero, the iteration will
 *  terminate and that value will be returned. NB - "dir" will not
 *  be the same as "self".
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
static 
rc_t CC
XFSWsDirVisit (
                const struct KDirectory * self, 
                bool Recurse,
                rc_t ( CC * Func ) (
                            const struct KDirectory *,
                            uint32_t,
                            const char *,
                            void *
                            ), 
                void * Data,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    const struct XFSWsDir * Dir;
    const struct XFSPath * Path;
    const struct XFSPath * RelPath;
    const struct _DirE * Entry;
    va_list xArgs;

    RCt = 0;
    Dir = ( const struct XFSWsDir * ) self;
    Path = NULL;
    RelPath = NULL;
    Entry = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Func )
    XFS_CAN ( Format )

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirVisit] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    va_copy ( xArgs, Args );
    RCt = XFSPathVMakeAbsolute ( & Path, false, Format, xArgs );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        if ( XFSPathSIsChild (
                            _WsDirPath ( Dir ),
                            XFSPathGet ( Path ),
                            & RelPath
                            ) ) {
                /* Mapping Node and path
                */
           va_copy ( xArgs, Args );
           RCt = _WsDirVMapIt ( Dir, & Entry, NULL, Format, xArgs ); 
           va_end ( xArgs );
           if ( RCt == 0 ) {
                /* check if it is a folder
                 */
                if ( Entry -> is_folder ) {
                    RCt = _DirEVisitNoLock (
                                    Entry,
                                    self,
                                    Recurse,
                                    Func,
                                    Data,
                                    XFSPathGet ( RelPath )
                                    );
                }
                else {
                    RCt = XFS_RC ( rcInvalid );
                }

                _DirERelease ( Entry );
            }

            XFSPathRelease ( RelPath );
        }
        else {
            RCt = XFS_RC ( rcOutOfKDirectory );
        }

        XFSPathRelease ( Path );
    }


    return RCt;
}   /* XFSWsDirVisit () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirVisitUpdate
 */
static
rc_t CC
XFSWsDirVisitUpdate (
                struct KDirectory * self,
                bool recurse,
                rc_t ( CC * f ) (
                            struct KDirectory *,
                            uint32_t,
                            const char *
                            ,void *
                            ),
                void * data,
                const char * path,
                va_list args
)
{
/*)
 || JOJOBA
 (*/
#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirVisitUpdate] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */
    return RC (rcFS, rcDirectory, rcUpdating, rcFunction, rcUnsupported);
}   /* XFSWsDirVisitUpdate () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirPathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 *
 * [RET] uint32_t
 * [IN]  const XFSWsDir *	self		Object oriented C; XFSWsDir object for this method
 * [IN]  const char *		path
 * [IN]  va_list		args
 */
static
uint32_t CC
XFSWsDirPathType (
                const struct KDirectory * self,
                const char * Path,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    uint32_t RetVal;
    va_list xArgs;

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirPathType] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    RetVal = kptNotFound;

    XFS_CAN ( self )
    XFS_CAN ( Path )

    va_copy ( xArgs, Args );
    RCt = _WsDirVMapIt (
                    Dir,
                    & Entry,
                    NULL,
                    Path,
                    xArgs
                    );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RetVal = Entry -> is_folder ? kptDir : kptFile;

        _DirERelease ( Entry );

        return RetVal;
    }
    else {
        if ( RCt == XFS_RC ( rcNotFound ) ) {
            return kptNotFound;
        }
    }

    return kptBadPath;
}   /* XFSWsDirPathType () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirResolvePath
 *
 *  resolves path to an absolute or directory-relative path
 *
 * [IN]  const XFSWsDir *self		Objected oriented self
 * [IN]	 bool 		absolute	if non-zero, always give a path starting
 *  					with '/'. NB - if the directory is 
 *					chroot'd, the absolute path
 *					will still be relative to directory root.
 * [OUT] char *		resolved	buffer for NUL terminated result path in 
 *					directory-native character set
 * [IN]	 size_t		rsize		limiting size of resolved buffer
 * [IN]  const char *	path		NUL terminated string in directory-native
 *					character set denoting target path. 
 *					NB - need not exist.
 *
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC
XFSWsDirResolvePath (
                const struct KDirectory * self,
                bool Absolute,
                char * Resolved,
                size_t Rsize,
                const char * Path,
                va_list Args
)
{
    rc_t RCt;
    va_list xArgs;

    RCt = 0;

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirResolvePath] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    va_copy ( xArgs, Args );
    RCt = XFS_VResolvePath ( Absolute, Resolved, Rsize, Path, xArgs );
    va_end ( xArgs );

    return RCt;
}   /* XFSWsDirResolvePath () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirResolveAlias
 *  resolves an alias path to its immediate target
 *  NB - the resolved path may be yet another alias
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting an object presumed to be an alias.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character set
 *
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC
XFSWsDirResolveAlias (
                const struct KDirectory * self, 
				bool absolute,
				char * resolved,
				size_t rsize,
				const char * alias,
				va_list args
)
{
/*)
 || JOJOBA
 (*/
#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirResolveAlias] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */
    return RC (rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported);
}   /* XFSWsDirResolveAlias () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirRename
 *  rename an object accessible from directory, replacing
 *  any existing target object of the same type
 *
 *  "from" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 *
 *  "to" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 */
static
rc_t CC
XFSWsDirRename (
                struct KDirectory * self,
                bool Force,
                const char * OldName,
                const char * NewName
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * OldParent, * NewParent;
    const struct XFSPath * OldEffPath, * NewEffPath;
    const char * OldEntryName, * NewEntryName;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;

    XFS_CAN ( self )
    XFS_CAN ( OldName )
    XFS_CAN ( NewName )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirRename] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

        /*||   There are two steps : move factual file if it
          ||   is necessary, and make corrections in content
          ||   files
          ||*/

        /* First we should resolve parent directory for both
         * path and decide if we should simple rename file withou
         * moving or we should move file first.
         */
    RCt = _GetCNameAndMapParentEntryNoLock (
                                            Dir,
                                            & OldParent,
                                            & OldEffPath,
                                            & OldEntryName,
                                            OldName
                                            );
    if ( RCt == 0 ) {
        RCt = _GetCNameAndMapParentEntryNoLock (
                                                Dir,
                                                & NewParent,
                                                & NewEffPath,
                                                & NewEntryName,
                                                NewName
                                                );
        if ( RCt == 0 ) {

                /*  Since we are cacheing _DirE instances, it is 
                 *  normal to compare pointers here :lol:
                 */
            if ( OldParent == NewParent ) {
                /* No need to move */
                LOG_LOC_ACQ ( __LINE__, OldParent -> mutabor );
                RCt = KLockAcquire ( OldParent -> mutabor );
                if ( RCt == 0 ) {
                    RCt = _DirERenameEntryNoLock (
                                                OldParent,
                                                OldEntryName,
                                                NewEntryName
                                                );
                    if ( RCt == 0 ) {
                        RCt = _SyncronizeDirectoryContentNoLock (
                                                            OldParent
                                                            );
                    }
                    LOG_LOC_UNL ( __LINE__, OldParent -> mutabor );
                    KLockUnlock ( OldParent -> mutabor );
                }
            }
            else {
                    /* Need to move */
                RCt = _DirEMoveEntry (
                                    OldParent,
                                    OldEntryName,
                                    NewParent,
                                    NewEntryName
                                    );
            }

            free ( ( char * ) NewEntryName );
            XFSPathRelease ( NewEffPath );
            _DirERelease ( NewParent );
        }

        free ( ( char * ) OldEntryName );
        XFSPathRelease ( OldEffPath );
        _DirERelease ( OldParent );
    }

    return RCt;
}   /* XFSWsDirRename () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirRemove
 *  remove an accessible object from its directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "force" [ IN ] - if non-zero and target is a directory,
 *  remove recursively
 */
static
rc_t CC
XFSWsDirRemove (
                struct KDirectory * self,
                bool Force,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct _DirE * Parent;
    const struct XFSPath * EffPath;
    const char * EntryName;
    struct KDirectory * NatDir;
    bool HasEntries;
    va_list xArgs;

    RCt = 0;
    Entry = NULL;
    Parent = NULL;
    EffPath = NULL;
    Dir = ( struct XFSWsDir * ) self;
    EntryName = NULL;
    NatDir = NULL;
    HasEntries = false;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirRemove] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

        /*|| Simple: map it's parent and remove entry ||*/
    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        va_copy ( xArgs, Args );
        RCt = _GetNameAndMapParentEntryNoLock (
                                        Dir,
                                        & Parent,
                                        & EffPath,
                                        & EntryName,
                                        Format,
                                        xArgs
                                        );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = _GetContentEntryAndLock ( Parent, & Entry, EntryName );
            if ( RCt == 0 ) {
                HasEntries = _DirEHasEntries ( Entry );

                if ( ( ! HasEntries ) || ( HasEntries && Force ) ) {
                    if ( RCt == 0 ) {
                /*|| IMPORTANT : we are not going to do it in right and 
                  ||             legal way by visiting each entry. Here
                  ||             could be trach remained ( not removed
                  ||             files and directories )
                  ||             Also we are forceing Force flag 8-|
                  ||*/
                        RCt = _DirEDelEntryNoLock ( Parent, Entry );
                        if ( RCt == 0 ) {
                            RCt = _SyncronizeDirectoryContentNoLock ( Parent );
                            if ( RCt == 0 ) {
                                RCt = KDirectoryRemove (
                                                NatDir,
                                                true,
                                                "%s/%s/%s",
                                                _WsDirPath ( Dir ),
                                                XFSPathGet ( EffPath ),
                                                Entry -> eff_name
                                                );
                            }
                        }
                    }
                }

                RCt = _DirERelease ( Entry );
            }

            free ( ( char * ) EntryName );

            LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
            KLockUnlock ( Parent -> mutabor );

            XFSPathRelease ( EffPath );

            _DirERelease ( Parent );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirRemove () */

static
rc_t CC
XFSWsDirRemoveU (
                struct KDirectory * self,
                bool Force,
                const char * Format,
                ...
)
{
    rc_t RCt;
    va_list Args;

    va_start ( Args, Format );
    RCt = XFSWsDirRemove ( self, Force, Format, Args );
    va_end ( Args );

    return RCt;
}   /* XFSWsDirRemoveU () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirClearDir
 *  remove all directory contents
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "force" [ IN ] - if non-zero and directory entry is a
 *  sub-directory, remove recursively
 */
static
rc_t CC
XFSWsDirClearDir (
                struct KDirectory * self,
                bool Force,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    struct KNamelist * List;
    uint32_t LQty, Idx;
    const char * SubName;
    const struct XFSPath * Path;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    List = NULL;
    LQty = Idx = 0;
    SubName = NULL;
    Path = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirClearDir] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

        /* Second we should list directory entries
         */
    va_copy ( xArgs, Args );
    RCt = XFSWsDirList ( self, & List, NULL, NULL, Format, xArgs );
    va_end ( xArgs );
    if ( RCt == 0 ) {
            /* Removing content one by one
             */
        RCt = KNamelistCount ( List, & LQty );
        if ( RCt == 0 && 0 < LQty ) {
                /* First we need path
                 */
            va_copy ( xArgs, Args );
            RCt = XFSPathVMakeAbsolute ( & Path, false, Format, xArgs );
            va_end ( xArgs );
            if ( RCt == 0 ) {
                for ( Idx = 0; Idx < LQty; Idx ++ ) {
                    RCt = KNamelistGet ( List, Idx, & SubName );
                    if ( RCt != 0 ) {
                        break;
                    }

                    RCt = XFSWsDirRemoveU (
                                            self,
                                            Force,
                                            "%s/%s",
                                            XFSPathGet ( Path ),
                                            SubName
                                            );

                    if ( RCt != 0 ) {
                        break;
                    }
                }

                XFSPathRelease ( Path );
            }
        }

        KNamelistRelease ( List );
    }

    return RCt;
}   /* XFSWsDirClearDir () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirAccess
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC
XFSWsDirVAccess (
                const struct KDirectory * self,
			    uint32_t * Access,
			    const char * Format,
			    va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;

    XFS_CSA ( Access, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Access )
    XFS_CAN ( Format )

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirVAccess] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt (
                        Dir,
                        & Entry,
                        & Path,
                        Format,
                        xArgs
                        );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = KDirectoryAccess (
                                NatDir,
                                Access,
                                "%s/%s",
                                _WsDirPath ( Dir ),
                                XFSPathGet ( Path )
                                );

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirVAccess () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirSetAccess
 *  set access to object a la Unix "chmod"
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "access" [ IN ] and "mask" [ IN ] - definition of change
 *  where "access" contains new bit values and "mask defines
 *  which bits should be changed.
 *
 *  "recurse" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 */
static
rc_t CC
XFSWsDirSetAccess (
                struct KDirectory * self,
			    bool Recurse,
			    uint32_t Access,
			    uint32_t Mask,
			    const char * Format,
			    va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirSetAccess] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt (
                        Dir,
                        & Entry,
                        & Path,
                        Format,
                        xArgs
                        );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = KDirectorySetAccess (
                                NatDir,
                                Recurse,
                                Access,
                                Mask,
                                "%s/%s",
                                _WsDirPath ( Dir ),
                                XFSPathGet ( Path )
                                );

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirSetAccess () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirCreateAlias
 *  creates a path alias according to create mode
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias
 *
 *  "access" [ IN ] - standard Unix directory access mode
 *  used when "mode" has kcmParents set and alias path does
 *  not exist.
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC
XFSWsDirCreateAlias (
                struct KDirectory * self,
			    uint32_t access,
			    KCreateMode mode,
			    const char * targ,
			    const char * alias
)
{
    assert (self != NULL);
    assert (targ != NULL);
    assert (alias != NULL);

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirCreateAlias] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

/*)
 || JOJOBA
 (*/
    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}   /* XFSWsDirCreateAlias () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirOpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC
XFSWsDirOpenFileRead (
                const struct KDirectory * self,
                const struct KFile ** File,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct _DirE * Parent;
    const char * EntryName;
    const struct XFSPath * EffPath;
    va_list xArgs;

    RCt = 0;
    Entry = NULL;
    EffPath = NULL;
    EntryName = NULL;
    Dir = ( struct XFSWsDir * ) self;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )
    XFS_CAN ( Format )

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirOpenFileRead] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

        /*) Mapping parent directory 
         (*/
    va_copy ( xArgs, Args );
    RCt = _GetNameAndMapParentEntryNoLock (
                                        Dir,
                                        & Parent,
                                        & EffPath,
                                        & EntryName,
                                        Format,
                                        xArgs
                                        );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RCt = _GetContentEntryAndLock ( Parent, & Entry, EntryName );
        if ( RCt == 0 ) {
                /*) Opening file 
                 (*/
            RCt = _OpenEncryptedFileRead (
                                    File,
                                    ( struct KKey * ) _WsDirKey ( Dir ),
                                    "%s/%s/%s",
                                    _WsDirPath ( Dir ),
                                    XFSPathGet ( EffPath ),
                                    Entry -> eff_name
                                    );

            LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
            KLockUnlock ( Parent -> mutabor );
            _DirERelease ( Entry );
        }

        free ( ( char * ) EntryName );

        XFSPathRelease ( EffPath );

        _DirERelease ( Parent );
    }

    return RCt;
}   /* XFSWsDirOpenFileRead () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirOpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 */
static
rc_t CC
XFSWsDirOpenFileWrite (
                struct KDirectory * self,
                struct KFile ** File,
                bool Update,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct _DirE * Parent;
    const char * EntryName;
    const struct XFSPath * EffPath;
    va_list xArgs;

    RCt = 0;
    Entry = NULL;
    Parent = NULL;
    EffPath = NULL;
    EntryName = NULL;
    Dir = ( struct XFSWsDir * ) self;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

// #ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirOpenFileWrite] [$(dir)]", "dir=%p", ( void * ) self );
// #endif /* JOJOBA */

        /*) Mapping parent directory 
         (*/
    va_copy ( xArgs, Args );
    RCt = _GetNameAndMapParentEntryNoLock (
                                        Dir,
                                        & Parent,
                                        & EffPath,
                                        & EntryName,
                                        Format,
                                        xArgs
                                        );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RCt = _GetContentEntryAndLock ( Parent, & Entry, EntryName );
        if ( RCt == 0 ) {
                /*) Opening file
                 (*/
            RCt = _OpenEncryptedFileWrite (
                                    File,
                                    ( struct KKey * ) _WsDirKey ( Dir ),
                                    Update,
                                    "%s/%s/%s",
                                    _WsDirPath ( Dir ),
                                    XFSPathGet ( EffPath ),
                                    Entry -> eff_name
                                    );

            LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
            KLockUnlock ( Parent -> mutabor );
            _DirERelease ( Entry );
        }

        free ( ( char * ) EntryName );

        XFSPathRelease ( EffPath );

        _DirERelease ( Parent );
    }

    return RCt;
}   /* XFSWsDirOpenFileWrite () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirCreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC
XFSWsDirCreateFile	(
                struct KDirectory * self,
                struct KFile ** File,
                bool Update,
                uint32_t Access,
                KCreateMode Cmode,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct _DirE * Parent;
    const char * EntryName;
    const struct XFSPath * EffPath;
    va_list xArgs;

    RCt = 0;
    Entry = NULL;
    Parent = NULL;
    EffPath = NULL;
    EntryName = NULL;
    Dir = ( struct XFSWsDir * ) self;

    XFS_CSAN ( File )
    XFS_CAN ( self )
    XFS_CAN ( File )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }


#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirCreateFile] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

        /*) Mapping parent directory
         (*/
    va_copy ( xArgs, Args );
    RCt = _GetNameAndMapParentEntryNoLock (
                                        Dir,
                                        & Parent,
                                        & EffPath,
                                        & EntryName,
                                        Format,
                                        xArgs
                                        );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        LOG_LOC_ACQ ( __LINE__, Parent -> mutabor );
        RCt = KLockAcquire ( Parent -> mutabor );
        if ( RCt == 0 ) {
            RCt = _DirEGetEntryNoLock (
                                    Parent,
                                    EntryName,
                                    & Entry
                                    );
            if ( GetRCState ( RCt ) == rcNotFound ) {
                    /*) Really we don't need to do it 
                     (*/
                RCt = 0;
                    /*) Making record in journal 
                     (*/
                RCt = _DirECreateEntryNoLock (
                                            Parent,
                                            EntryName,
                                            false,
                                            & Entry
                                            );
                if ( RCt == 0 ) {
                        /*) Here we are opening file 
                        (*/
                    RCt = _CreateEncryptedFile (
                                            File,
                                            _WsDirKey ( Dir ) ,
                                            Update,
                                            Access,
                                            Cmode,
                                            "%s/%s/%s",
                                            _WsDirPath ( Dir ),
                                            XFSPathGet ( EffPath ),
                                            Entry -> eff_name
                                            );
                    if ( RCt == 0 ) {
                        RCt = _SyncronizeDirectoryContentNoLock ( Parent );
                    }
                    else {
                        _DirEDelEntryNoLock ( Parent, Entry );
                    }
                }
            }
            else {
                if ( RCt == 0 ) {
                    _DirERelease ( Entry );
                }
            }

            LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
            KLockUnlock ( Parent -> mutabor );
        }

        free ( ( char * ) EntryName );

        XFSPathRelease ( EffPath );

        _DirERelease ( Parent );
    }

    return RCt;
}   /* XFSWsDirCreateFile () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirFileSize
 *  returns size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ OUT ] - return parameter for file size
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC
XFSWsDirFileSize (
                const struct KDirectory * self,
                uint64_t * Size,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    const struct KFile * File;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;
    File = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )
    XFS_CAN ( Format )

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirFileSize] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt ( Dir, & Entry, & Path, Format, xArgs );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            if ( Entry -> is_folder ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                RCt = _OpenEncryptedFileRead (
                                            & File, 
                                            ( struct KKey * ) _WsDirKey ( Dir ),
                                            "%s/%s",
                                            _WsDirPath ( Dir ),
                                            XFSPathGet ( Path )
                                            );
                if ( RCt == 0 ) {
                    RCt = KFileSize ( File, Size );

                    KFileRelease ( File );
                }
            }

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirFileSize () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirSetFileSize
 *  sets size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC
XFSWsDirSetFileSize (
                struct KDirectory * self,
                uint64_t Size,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    struct KFile * File;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;
    File = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirSetFileSize] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt ( Dir, & Entry, & Path, Format, xArgs );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            if ( Entry -> is_folder ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                RCt = _OpenEncryptedFileWrite (
                                            & File, 
                                            ( struct KKey * ) _WsDirKey ( Dir ),
                                            true,
                                            "%s/%s",
                                            _WsDirPath ( Dir ),
                                            XFSPathGet ( Path )
                                            );
                if ( RCt == 0 ) {
                    RCt = KFileSetSize ( File, Size );

                    KFileRelease ( File );
                }
            }

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirSetFileSize () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirOpenDirRead
 *
 *  opens a sub-directory
 *
 * [IN]  const XFSWsDir *	self	Object Oriented C XFSWsDir self
 * [OUT] const KDirectory **	subp	Where to put the new KDirectory/XFSWsDir
 * [IN]  bool			chroot	Create a chroot cage for this new subdirectory
 * [IN]  const char *		path	Path to the directory to open
 * [IN]  va_list		args	So far the only use of args is possible additions to path
 */
static 
rc_t CC
XFSWsDirOpenDirRead (
                const struct KDirectory * self,
                const struct KDirectory ** Subp,
                bool Chroot,
                const char * Format,
                va_list Args
)
{
    assert (self != NULL);
    assert (Subp != NULL);
    assert (Format != NULL);

/* We do not implement that method, because subdirectory workspace
 * could clash with parent workspace on file synchronizing.
 * Of course, we may hash each directory content in separate Depot,
 *            and will do it if it is necessary
 */

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirOpenDirRead] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    return RC (rcFS, rcDirectory, rcOpening, rcSelf, rcUnsupported);
}   /* XFSWsDirOpenDirRead () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirOpenDirUpdate
 *  opens a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 */
static
rc_t CC
XFSWsDirOpenDirUpdate (
                struct KDirectory * self,
                struct KDirectory ** Subp, 
                bool Chroot, 
                const char * Format, 
                va_list Args
)
{
    assert (self != NULL);
    assert (Subp != NULL);
    assert (Format != NULL);
/* We do not implement that method, because subdirectory workspace
 * could clash with parent workspace on file synchronizing.
 * Of course, we may hash each directory content in separate Depot,
 *            and will do it if it is necessary
 */

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirOpenDirUpdate] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}   /* XFSWsDirOpenDirUpdate () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirCreateDir
 *  create a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "access" [ IN ] - standard Unix directory permissions
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC
XFSWsDirCreateDir (
                struct KDirectory * self,
                uint32_t Access,
                KCreateMode CreationMode,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct _DirE * Parent;
    const struct XFSPath * EffPath;
    const char * EntryName;
    struct KDirectory * NatDir;
    va_list xArgs;

    RCt = 0;
    Entry = NULL;
    Parent = NULL;
    EffPath = NULL;
    EntryName = NULL;
    Dir = ( struct XFSWsDir * ) self;
    NatDir = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirCreateDir] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
            /*) Mapping parent
             (*/
        va_copy ( xArgs, Args );
        RCt = _GetNameAndMapParentEntryNoLock (
                                            Dir,
                                            & Parent,
                                            & EffPath,
                                            & EntryName,
                                            Format,
                                            xArgs
                                            );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            LOG_LOC_ACQ ( __LINE__, Parent -> mutabor );
            RCt = KLockAcquire ( Parent -> mutabor );
            if ( RCt == 0 ) {
                RCt = _DirEGetEntryNoLock ( Parent, EntryName, & Entry );
                if ( GetRCState ( RCt ) == rcNotFound ) {

                        /*) Really we don't need to do it
                         (*/
                    RCt = 0;

                        /*) Making record in journal
                         (*/
                    RCt = _DirECreateEntryNoLock (
                                            Parent,
                                            EntryName,
                                            true,
                                            & Entry
                                            );
                    if ( RCt == 0 ) {
                        /*) Here we are opening file
                         (*/
                        RCt = KDirectoryCreateDir (
                                                NatDir,
                                                Access,
                                                CreationMode,
                                                "%s/%s/%s",
                                                _WsDirPath ( Dir ),
                                                XFSPathGet ( EffPath ),
                                                Entry -> eff_name
                                                );
                        if ( RCt == 0 ) {
                            RCt = _SyncronizeDirectoryContentNoLock (
                                                                Parent
                                                                );
                            if ( RCt == 0 ) {
                                RCt = _SyncronizeDirectoryContentNoLock (
                                                                Entry
                                                                );
                            }
                        }
                        else {
                            _DirEDelEntryNoLock ( Parent, Entry );
                        }

                    }
                }
                else {
                    if ( RCt == 0 ) {
                        _DirERelease ( Entry );
                    }
                }

                LOG_LOC_UNL ( __LINE__, Parent -> mutabor );
                KLockUnlock ( Parent -> mutabor );
            }

            free ( ( char * ) EntryName );

            XFSPathRelease ( EffPath );
            
            _DirERelease ( Parent );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirCreateDir () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSWsDirDestroyFile
 */
static
rc_t CC
XFSWsDirDestroyFile ( struct KDirectory * self, struct KFile * File )
{
    assert (self != NULL);
    assert (File != NULL);

/* We don't really need that method
 */

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirDestroyFile] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    return RC (rcFS, rcDirectory, rcDestroying, rcSelf, rcUnsupported);
}   /* XFSWsDirDestroyFile () */

static
rc_t CC
XFSWsDirDate (
                const struct KDirectory * self,
                KTime_t * Date,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;

    XFS_CSA ( Date, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Date )
    XFS_CAN ( Format )

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirDate] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt ( Dir, & Entry, & Path, Format, xArgs );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = KDirectoryDate (
                                NatDir,
                                Date, 
                                "%s/%s",
                                _WsDirPath ( Dir ),
                                XFSPathGet ( Path )
                                );

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirDate () */


static
rc_t CC
XFSWsDirSetDate (
                struct KDirectory * self,
                bool Recurse,
                KTime_t Date,
                const char * Format,
                va_list Args
)
{
    rc_t RCt;
    struct XFSWsDir * Dir;
    const struct _DirE * Entry;
    const struct XFSPath * Path;
    struct KDirectory * NatDir;
    va_list xArgs;

    RCt = 0;
    Dir = ( struct XFSWsDir * ) self;
    Entry = NULL;
    Path = NULL;
    NatDir = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Format )

    if ( ! Dir -> update ) {
        return XFS_RC ( rcUnauthorized );
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirSetDate] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {

        va_copy ( xArgs, Args );
        RCt = _WsDirVMapIt ( Dir, & Entry, & Path, Format, xArgs );
        va_end ( xArgs );
        if ( RCt == 0 ) {
            RCt = KDirectorySetDate (
                                NatDir,
                                Recurse,
                                Date, 
                                "%s/%s",
                                _WsDirPath ( Dir ),
                                XFSPathGet ( Path )
                                );

            XFSPathRelease ( Path );
            _DirERelease ( Entry );
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* XFSWsDirSetDate () */


static
struct KSysDir * CC
XFSWsDirGetSysDir ( const struct KDirectory * self )
{

#ifdef JOJOBA
pLogMsg ( klogDebug, " <<<[XFSWsDirGetSysDir] [$(dir)]", "dir=%p", ( void * ) self );
#endif /* JOJOBA */

    return NULL;
}   /* XFSWsDirGetSysDir () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 *
 */
static struct KDirectory_vt_v1 vtXFSWsDir =
{
    /* version 1.0 */
    1, 1,

    /* start minor version 0 methods*/
    XFSWsDirDestroy,                    /* [+] */
    XFSWsDirList,                       /* [+] */
    XFSWsDirVisit,                      /* [-] */
    XFSWsDirVisitUpdate,                /* [-] */
    XFSWsDirPathType,                   /* [+] */
    XFSWsDirResolvePath,                /* [+] */
    XFSWsDirResolveAlias,               /* [-] */ /* 4 FUTURE */
    XFSWsDirRename,                     /* [+] */
    XFSWsDirRemove,                     /* [+] */
    XFSWsDirClearDir,                   /* [+] */
    XFSWsDirVAccess,                    /* [+] */
    XFSWsDirSetAccess,                  /* [+] */
    XFSWsDirCreateAlias,                /* [-] */ /* 4 FUTURE */
    XFSWsDirOpenFileRead,               /* [+] */
    XFSWsDirOpenFileWrite,              /* [+] */
    XFSWsDirCreateFile,                 /* [+] */
    XFSWsDirFileSize,                   /* [+] */
    XFSWsDirSetFileSize,                /* [+] */
    XFSWsDirOpenDirRead,                /* [-] */ /* WE DON"T DO IT */
    XFSWsDirOpenDirUpdate,              /* [-] */ /* WE DON"T DO IT */
    XFSWsDirCreateDir,                  /* [+] */
    XFSWsDirDestroyFile,                /* [-] */ /* NO NEED */
    /* end minor version 0 methods*/
    /* start minor version 1 methods*/

    XFSWsDirDate,                       /* [-] */
    XFSWsDirSetDate,                    /* [-] */
    XFSWsDirGetSysDir,                  /* [-] */
    /* end minor version 2 methods*/
#ifdef NO_JOJOBA
    NULL,
    /* end minor version 2 methods*/
    /* end minor version 3 methods*/
    NULL,
    NULL
    /* end minor version 3 methods*/
#endif /* NO_JOJOBA */
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*
 * DEPOT <<
 */
struct _WsDptE {
    BSTNode node;

    struct KDirectory * dir;
    const char * path;
};

static
rc_t CC
_WsDptEDispose ( struct _WsDptE * self )
{
    if ( self != NULL ) {
        if ( self -> dir != NULL ) {
            KDirectoryRelease ( self -> dir );

            self -> dir = NULL;
        }

        if ( self -> path != NULL ) {
            free ( ( char * ) self -> path );

            self -> path = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _WsDptEDispose () */

static
rc_t CC
_WsDptEMake (
            const char * Path,
            struct KDirectory * Dir,
            struct _WsDptE ** Entry
)
{
    rc_t RCt;
    struct _WsDptE * RetEntry;

    RCt = 0;
    RetEntry = NULL;

    RCt = 0;

    XFS_CSAN ( Entry )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )
    XFS_CAN ( Entry )

    RetEntry = calloc ( 1, sizeof ( struct _WsDptE ) );
    if ( RetEntry == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( Path, & ( RetEntry -> path ) );
        if ( RCt == 0 ) {
            RetEntry -> dir = Dir;

            * Entry = RetEntry;
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL;

        _WsDptEDispose ( RetEntry );
    }

    return RCt;
}   /* _WsDptEMake () */

struct _WsDpt {
    BSTree tree;

    struct KLock * mutabor;
};

static struct _WsDpt * _sWsDpt = NULL;

static
struct _WsDpt * CC
_Dpt ()
{
    return _sWsDpt;
}   /* _Dpt () */

static
void CC
_WsDptWhackCallback ( BSTNode * Node, void * unused )
{
    if ( Node != NULL ) {
        _WsDptEDispose ( ( struct _WsDptE * ) Node );
    }
}   /* _WsDptWhackCallback () */

static
rc_t CC
_WsDptDisposeImpl ( struct _WsDpt * self )
{
    if ( self != NULL ) {
        BSTreeWhack ( & ( self -> tree ) , _WsDptWhackCallback, NULL );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );

            self -> mutabor = NULL;
        }

        free ( self );
    }

    return 0;
}   /* _WsDptDipsoseImpl () */

static
rc_t CC
_WsDptMakeImpl ( struct _WsDpt ** Depot )
{
    rc_t RCt;
    struct _WsDpt * TheDepot;

    RCt = 0;
    TheDepot = NULL;

    XFS_CSAN ( Depot )
    XFS_CAN ( Depot )

    TheDepot = calloc ( 1, sizeof ( struct _WsDpt ) );
    if ( TheDepot != NULL ) {
        RCt = KLockMake ( & ( TheDepot -> mutabor ) );

        if ( RCt == 0 ) {
            BSTreeInit ( & ( TheDepot -> tree ) );

            * Depot = TheDepot;
        }
    }
    else {
        RCt = XFS_RC ( rcExhausted );
    }

    if ( RCt != 0 ) {
        * Depot = NULL;

        _WsDptDisposeImpl ( TheDepot );
    }

    return RCt;
}   /* _WsDptMakeImpl () */

LIB_EXPORT
rc_t CC
XFSEncDirectoryDepotInit ()
{
    rc_t RCt;
    struct _WsDpt * Depot;

    RCt = 0;
    Depot = NULL;

    if ( _sWsDpt == NULL ) {
        RCt = _WsDptMakeImpl ( & Depot );
        if ( RCt == 0 ) {
            if ( _sWsDpt == NULL ) {
                _sWsDpt = Depot;
            }
            else {
                _WsDptDisposeImpl ( Depot );
            }
        }
    }

    return RCt;
}   /* XFSEncDirectoryDepotInit () */

LIB_EXPORT
rc_t CC
XFSEncDirectoryDepotDispose ()
{
    struct _WsDpt * Depot;

    Depot = _sWsDpt;
    _sWsDpt = NULL;

    if ( Depot != NULL ) {
        _WsDptDisposeImpl ( Depot );
    }

    return 0;
}   /* XFSEncDirectoryDepotDispose () */

static
int64_t CC
_WsDptCmpCallback (const void * Item, const BSTNode * Node )
{
    const char * Str1, * Str2;

    Str1 = ( const char * ) Item;

    Str2 = Node == NULL
                    ? NULL
                    : ( ( struct _WsDptE * ) Node ) -> path
                    ;

    return XFS_StringCompare4BST_ZHR ( Str1, Str2 );
}   /* _WsDptCmpCallback () */

static
rc_t CC
_WsDptGetNoLock (
                struct _WsDpt * self,
                const char * Path,
                struct KDirectory ** Dir
)
{
    struct _WsDptE * Entry = NULL;

    XFS_CSAN ( Dir )
    XFS_CAN ( self )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )

    Entry = ( struct _WsDptE * ) BSTreeFind (
                                            & ( self -> tree ),
                                            Path,
                                            _WsDptCmpCallback
                                            );
    if ( Entry != NULL ) {
        * Dir = Entry -> dir;
    }

    return Entry == NULL ? XFS_RC ( rcNotFound ) : 0;
}   /* _WsDptGetNoLock () */

static
int64_t CC
_WsDptAddCallback ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
                                ( ( struct _WsDptE * ) N1 ) -> path,
                                ( ( struct _WsDptE * ) N2 ) -> path
                                );
}   /* _WsDptAddCallback () */

static
rc_t CC
_WsDptAddNoLock (
                struct _WsDpt * self,
                const char * Path,
                struct KDirectory * Dir
)
{
    rc_t RCt;
    struct _WsDptE * Entry;

    RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )

    RCt = _WsDptEMake ( Path, Dir, & Entry );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                            & ( self -> tree ),
                            ( struct BSTNode * ) & ( Entry -> node ),
                            _WsDptAddCallback
                            );
    }

    return RCt;
}   /* _WsDptAddNoLock () */

static
rc_t CC
_WsDptClearNoLock ( struct _WsDpt * self )
{
    XFS_CAN ( self )

    BSTreeWhack ( & ( self -> tree ), _WsDptWhackCallback, NULL );
    BSTreeInit ( & ( self -> tree ) );

    return 0;
}   /* _WsDptClearNoLock () */

LIB_EXPORT
rc_t CC
XFSEncDirectoryDepotClear ()
{
    rc_t RCt;
    struct _WsDpt * Depot;

    RCt = 0;
    Depot = _Dpt ();

    XFS_CAN ( Depot )

    LOG_LOC_ACQ ( __LINE__, Depot -> mutabor );
    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _WsDptClearNoLock ( Depot );

        LOG_LOC_UNL ( __LINE__, Depot -> mutabor );
        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* XFSEncDirectoryDepotClear () */

static
rc_t CC
_WsDptGet ( const char * Path, struct KDirectory ** Dir )
{
    rc_t RCt;
    struct _WsDpt * Depot;

    RCt = 0;
    Depot = _Dpt ();

    XFS_CAN ( Depot )

    XFS_CSAN ( Dir )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )

    LOG_LOC_ACQ ( __LINE__, Depot -> mutabor );
    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _WsDptGetNoLock ( Depot, Path, Dir );

        LOG_LOC_UNL ( __LINE__, Depot -> mutabor );
        KLockUnlock ( Depot -> mutabor );
    }

    return RCt;
}   /* _WsDptGet () */

static
rc_t CC
_WsDptAdd ( const char * Path, struct KDirectory * Dir )
{
    rc_t RCt;
    struct _WsDpt * Depot;
    struct KDirectory * TheDir;

    RCt = 0;
    Depot = _Dpt ();
    TheDir = NULL;

    XFS_CAN ( Depot )

    XFS_CAN ( Path )
    XFS_CAN ( Dir )

    LOG_LOC_ACQ ( __LINE__, Depot -> mutabor );
    RCt = KLockAcquire ( Depot -> mutabor );
    if ( RCt == 0 ) {
        RCt = _WsDptGetNoLock ( Depot, Path, & TheDir );
        if ( RCt == 0 ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = _WsDptAddNoLock ( Depot, Path, Dir );
        }

        LOG_LOC_UNL ( __LINE__, Depot -> mutabor );
        KLockUnlock ( Depot -> mutabor );
    }


    return RCt;
}   /* _WsDptAdd () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*)) That method does not do any checks
     ((*/
static
rc_t CC
_WsDirAlloc (
        const char * AbsolutePath,
        const char * DisplayName,
        const char * Password,
        const char * EncType,
        bool Update,
        struct KDirectory ** Dir
)
{
    rc_t RCt;
    struct XFSWsDir * RetDir;
    struct KKey Key;
    const struct _DirE * Entry;

    RCt = 0;
    RetDir = NULL;
    Entry = NULL;

    XFS_CSAN ( Dir )
    XFS_CAN ( AbsolutePath )
    XFS_CAN ( DisplayName )
    XFS_CAN ( Password )
    XFS_CAN ( Dir )

    RetDir = calloc ( 1, sizeof ( struct XFSWsDir ) );
    if ( RetDir == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RetDir -> update = rcUnauthorized;

            /* First - init directory
             */
    RCt = KDirectoryInit (
                        & ( RetDir -> dir ),
                        ( const KDirectory_vt * ) & vtXFSWsDir,
                        "WsDir",
                        AbsolutePath,
                        Update
                        );

    if ( RCt == 0 ) {
            /* second : Creating entry
             */
        RCt = _DirEMake (
                        _sFolderTag,
                        DisplayName,
                        DisplayName,
                        & Entry
                        );
        if ( RCt == 0 ) {
                /* third : Crating encoding key
                 */
            RCt =  XFS_InitKKey_ZHR ( Password, EncType, & Key );
            if ( RCt == 0 ) {
                    /* fourth : Trying to load content
                     */
                RCt = _DirEMakeContent ( Entry, & Key, AbsolutePath );
                if ( RCt == 0 ) {

                    RetDir -> entry = Entry;

                    RCt = XFS_StrDup (
                                    Password,
                                    & ( RetDir -> passwd )
                                    );
                    if ( RCt == 0 ) {
                        RCt = XFS_StrDup (
                                        EncType,
                                        & ( RetDir -> enc_type )
                                        );

                        if ( RCt == 0 ) {
                            * Dir = & ( RetDir -> dir );
                        }
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Dir = NULL;

        if ( RetDir != NULL ) {
            XFSWsDirDestroy ( & ( RetDir -> dir ) );
        }
    }

#ifdef JOJOBA
pLogMsg ( klogDebug, " [_WsDirAlloc] [$(line)] [$(rc)]", "line=%d,rc=%d", __LINE__, RCt );
#endif /* JOJOBA */

    return RCt;
}   /* _WsDirAlloc () */

static
rc_t CC
_WsDirMake (
                const struct KDirectory ** Dir,
                bool Update,
                const char * Password,
                const char * EncType,
                const char * Path,
                va_list Args
)
{
    rc_t RCt;
    const struct XFSPath * aPath;
    struct KDirectory * RetVal;
    const char * Name;
    va_list xArgs;

    RCt = 0;
    aPath = NULL;
    RetVal = NULL;
    Name = NULL;

    XFS_CSAN ( Dir )
    XFS_CAN ( Password )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )

        /* Creating valid path
         */
    va_copy ( xArgs, Args );
    RCt = XFSPathVMakeAbsolute ( & aPath, false, Path, xArgs );
    va_end ( xArgs );
    if ( RCt == 0 ) {
        RCt = _WsDptGet ( XFSPathGet ( aPath ), & RetVal );
        if ( RCt != 0 ) {
            RCt = 0;     /* We do not need that really */

            Name = XFSPathName ( aPath );
            if ( RCt == 0 ) {
                    /* Creating encoded directory
                     */
                RCt = _WsDirAlloc (
                                XFSPathGet ( aPath ),
                                Name,
                                Password,
                                EncType,
                                Update,
                                & RetVal
                                );
                if ( RCt == 0 ) {
                    RCt = _WsDptAdd ( XFSPathGet ( aPath ), RetVal );
                }
            }
        }

        XFSPathRelease ( aPath );
    }

    if ( RCt == 0 ) {
        if ( Dir == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            RCt = KDirectoryAddRef ( RetVal );
            if ( RCt == 0 ) {
                * Dir = RetVal;
            }
        }
    }

    if ( RCt != 0 ) {
        * Dir = NULL;

        if ( RetVal != NULL ) {
                /* Should we delete entry ? I think NO! */
        }
    }

    return RCt;
}   /* _WsDirMake () */

LIB_EXPORT
rc_t CC
XFSEncDirectoryOpen (
                const struct KDirectory ** Dir,
                bool Update,
                const char * Password,
                const char * EncType,
                const char * Path,
                ...
)
{
    rc_t RCt;
    va_list Args;

    RCt = 0;

    XFS_CSAN ( Dir )
    XFS_CAN ( Password )
    XFS_CAN ( Path )
    XFS_CAN ( Dir )

    va_start ( Args, Path );
    RCt = _WsDirMake ( Dir, Update, Password, EncType, Path, Args );
    va_end ( Args );

    return RCt;
}   /* XFSEncDirectoryOpen () */

