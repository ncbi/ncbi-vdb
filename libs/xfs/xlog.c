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
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/status.h>
#include <kproc/lock.h>
#include <kfs/file.h>
#include <kfs/directory.h>

#include <xfs/xlog.h>
#include "schwarzschraube.h"

#include <sysalloc.h>

 /*))))
   |||| That file contains unsoted methods
   ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 || Setting log file is tricky and platform dependent
((*/

struct _LWr {
    struct KLock * mutabor;

    const char * path;
    struct KFile * file;

    uint64_t pos;
};

static struct _LWr * _sLWr = NULL;

static
rc_t CC
_LWrClose ( struct _LWr * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        if ( self -> file != NULL ) {
            KFileRelease ( self -> file );
            self -> file = NULL;
        }

        KLockUnlock ( self -> mutabor );
    }

    return 0;
}   /* _LWrClose () */

static
rc_t CC
_LWrOpen ( struct _LWr * self )
{
    rc_t RCt;
    struct KDirectory * NatDir;

    RCt = 0;
    NatDir = NULL;

    XFS_CAN ( self )

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        if ( self -> file == NULL ) {
            RCt = KDirectoryNativeDir ( & NatDir );
            if ( RCt == 0 ) {
                RCt = KDirectoryCreateFile (
                                            NatDir,
                                            & ( self -> file ),
                                            false,
                                            0664,
                                            kcmSharedAppend,
                                            self -> path
                                            );
                if ( RCt == 0 ) {
                    RCt = KFileSize (
                                    self -> file,
                                    & ( self -> pos )
                                    );
                }
                KDirectoryRelease ( NatDir );
            }

        }
        KLockUnlock ( self -> mutabor );
    }

    if ( RCt != 0 ) {
        _LWrClose ( self );
    }

    return 0;
}   /* _LWrOpen () */

static
rc_t CC
_LWrDispose ( struct _LWr * self )
{
    if ( self != NULL ) {
        _LWrClose ( self );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( self -> mutabor );
            self -> mutabor = NULL;
        }

        if ( self -> path != NULL ) {
            free ( ( char * ) self -> path );
            self -> path = NULL;
        }

        self -> pos = 0;

        free ( self );
    }

    return 0;
}   /* _LWrDispose () */

static 
rc_t CC
_LWrMake ( struct _LWr ** Writer, const char * LogFile )
{
    rc_t RCt;
    struct _LWr * TheWriter;

    RCt = 0;
    TheWriter = NULL;

    XFS_CSAN ( Writer )
    XFS_CAN ( Writer )
    XFS_CAN ( LogFile )

    TheWriter = calloc ( 1, sizeof ( struct _LWr ) );
    if ( TheWriter == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KLockMake ( & ( TheWriter -> mutabor ) );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( LogFile, & ( TheWriter -> path ) );
            if ( RCt == 0 ) {
                TheWriter -> pos = 0;

                * Writer = TheWriter;
            }
        }
    }

    if ( RCt != 0 ) {
        * Writer = NULL;

        if ( TheWriter != NULL ) {
            _LWrDispose ( TheWriter );
        }
    }

    return RCt;
}   /* _LWrMake () */

static
rc_t CC
_LWrWriter ( void * self, const char * Bf, size_t BfS, size_t * NWr )
{
    rc_t RCt;
    struct _LWr * Writer;

    RCt = 0;
    Writer = ( struct _LWr * ) self;

    XFS_CSA ( NWr, 0 )
    XFS_CAN ( Bf )
    XFS_CAN ( NWr )

    if ( self != NULL && Writer -> file != NULL ) {
        RCt = KFileWriteAll ( Writer -> file, Writer -> pos, Bf, BfS, NWr );
        if ( RCt == 0 ) {
            Writer -> pos += * NWr;
        }
    }

    return RCt;
}   /* _LWrWriter () */

static
rc_t CC
_LWrWriterDummy ( void * s, const char * b, size_t bs, size_t * w )
{
    if ( w != NULL ) {
        * w = bs;
    }

    return 0;
}   /* _LWrWriterDummy () */

LIB_EXPORT
rc_t CC
XFSLogInit ( const char * LogFile )
{
    rc_t RCt;
    struct _LWr * Writer;

    RCt = 0;
    Writer = NULL;

    if ( LogFile != NULL ) {
        if ( _sLWr == NULL ) {
            RCt = _LWrMake ( & Writer, LogFile );
            if ( RCt == 0 ) {
                RCt = _LWrOpen ( Writer );
                if ( RCt == 0 ) {
                    _sLWr = Writer;

                    KOutHandlerSet( _LWrWriter, Writer );
                    KDbgHandlerSet( _LWrWriter, Writer );
                    KLogHandlerSet( _LWrWriter, Writer );
                    KLogLibHandlerSet( _LWrWriter, Writer );
                    KStsHandlerSet( _LWrWriter, Writer );
                    KStsLibHandlerSet( _LWrWriter, Writer );
                }
            }
        }
    }

    if ( RCt != 0 || LogFile == NULL ) {
        _sLWr = NULL;

        if ( Writer != NULL ) {
            _LWrDispose ( Writer );
        }
        KOutHandlerSet( _LWrWriterDummy, NULL );
        KDbgHandlerSet( _LWrWriterDummy, NULL );
        KLogHandlerSet( _LWrWriterDummy, NULL );
        KLogLibHandlerSet( _LWrWriterDummy, NULL );
        KStsHandlerSet( _LWrWriterDummy, NULL );
        KStsLibHandlerSet( _LWrWriterDummy, NULL );
    }

    return 0;
    return RCt;
}   /* XFSLogInit () */

LIB_EXPORT
rc_t CC
XFSLogDestroy ( )
{
    struct _LWr * Writer = _sLWr;
    if ( Writer != NULL ) {
        _sLWr = NULL;

        _LWrClose ( Writer );
        _LWrDispose ( Writer );
    }

    return 0;
}   /* XFSLogDestroy () */
