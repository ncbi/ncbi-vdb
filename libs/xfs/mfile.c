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

#include <kfs/file.h>
#include <kfs/file-impl.h>
#include <kfs/defs.h>

#include "mfile.h"

#include <sysalloc.h>

#include <string.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    XFSMFilePod and others
 ((*/

struct XFSMFilePod {
    KRefcount refcount;

    char * data;
    uint64_t capacity;
    uint64_t size;
    uint32_t block_size;
};

static const char * _sMFilePod_classname = "MFilePod";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSMFilePod ...                                                   */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_MFilePodDispose ( struct XFSMFilePod * self )
{
    if ( self != NULL ) {
        KRefcountWhack (
                    & ( self -> refcount ),
                    _sMFilePod_classname
                    );

        if ( self -> data != NULL ) {
            free ( self -> data );
            self -> data = NULL;
        }

        self -> capacity = 0;
        self -> size = 0;
        self -> block_size = 0;
    }

    return 0;
}   /* _MFilePodDispose () */

static rc_t CC _MFilePodSetDataSize (
                                const struct XFSMFilePod * self,
                                uint64_t Size
                                );

LIB_EXPORT
rc_t CC
XFSMFilePodMake (
                struct XFSMFilePod ** Pod,
                uint64_t InitialSize,
                uint32_t BlockSize
)
{
    rc_t RCt;
    struct XFSMFilePod * ThePod;

    RCt = 0;
    ThePod = NULL;

    XFS_CSAN ( Pod )
    XFS_CAN ( Pod )

    ThePod = calloc ( 1, sizeof ( struct XFSMFilePod ) );
    if ( ThePod == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        ThePod -> block_size  = BlockSize == 0
                                            ? XFS_SIZE_8192
                                            : BlockSize
                                            ;
        ThePod -> size = 0;
        ThePod -> capacity = 0;
        ThePod -> data = NULL;

        KRefcountInit (
                    & ( ThePod -> refcount ),
                    1,
                    _sMFilePod_classname,
                    "MFilePod",
                    "MFilePod"
                    );

        if ( InitialSize != 0 ) {
            RCt = _MFilePodSetDataSize ( ThePod, InitialSize );
        }

        if ( RCt == 0 ) {
            * Pod = ThePod;
        }
    }

    if ( RCt != 0 ) {
        * Pod = NULL;

        if ( ThePod != NULL ) {
            _MFilePodDispose ( ThePod );
        }
    }

    return RCt;
}   /* XFSMFilePodMake () */

LIB_EXPORT
rc_t CC
XFSMFilePodAddRef ( const struct XFSMFilePod * self )
{
    rc_t RCt;
    struct XFSMFilePod * Pod;
    int RefC;

    RCt = 0;
    Pod = ( struct XFSMFilePod * ) self;
    RefC = 0;

    XFS_CAN ( self )

    RefC = KRefcountAdd (
                        & ( Pod -> refcount ),
                        _sMFilePod_classname
                        );
    switch ( RefC ) {
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
}   /* XFSMFilePodAddRef () */

LIB_EXPORT
rc_t CC
XFSMFilePodRelease ( const struct XFSMFilePod * self )
{
    rc_t RCt;
    struct XFSMFilePod * Pod;
    int RefC;

    RCt = 0;
    RefC = 0;
    Pod = ( struct XFSMFilePod  * ) self;

    XFS_CAN ( Pod )

    RefC = KRefcountDrop (
                        & ( Pod -> refcount ),
                        _sMFilePod_classname
                        );
    switch ( RefC ) {
        case krefOkay :
        case krefZero :
                RCt = 0;
                break;
        case krefWhack :
                RCt = _MFilePodDispose ( Pod );
                break;
        case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
        default :
                RCt = XFS_RC ( rcUnknown );
                break;
    }

    return RCt;
}   /* XFSMFilePodRelease () */

LIB_EXPORT
rc_t CC
XFSMFilePodData ( const struct XFSMFilePod * self, const char ** Data )
{
    XFS_CSAN ( Data )
    XFS_CAN ( self )
    XFS_CAN ( Data )

    * Data = self -> data;

    return 0;
}   /* XFSMFilePodData () */

LIB_EXPORT
rc_t CC
XFSMFilePodDataSize ( const struct XFSMFilePod * self, uint64_t * Size )
{
    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    * Size = self -> data == NULL ? 0 : self -> size;

    return 0;
}   /* XFSMFilePodDataSize () */

/*  That will realloc each time when it will be asked to set smaller
 *  size, unless size will be ZERO
 */
rc_t CC
_MFilePodSetDataSize ( const struct XFSMFilePod * self, uint64_t Size )
{
    struct XFSMFilePod * Pod;
    uint64_t NewCapacity;
    char * Data;

    Pod = ( struct XFSMFilePod * ) self;
    NewCapacity = 0;
    Data = NULL;

    XFS_CAN ( Pod )

    NewCapacity = Size == 0
                    ? 0
                    : (( ( Size / Pod -> block_size ) + 1 ) * Pod -> block_size )
                    ;

    if ( NewCapacity != Pod -> capacity ) {
        if ( NewCapacity != 0 ) {
            Data = calloc ( NewCapacity, sizeof ( char ) );
            if ( Data == NULL ) {
                return XFS_RC ( rcExhausted );
            }
        }

        if ( Pod -> data != NULL ) {
            if ( Pod -> size != 0 ) {
                memcpy (
                        Data,
                        Pod -> data,
                        sizeof ( char ) * Pod -> size
                        );
            }

            free ( Pod -> data );
        }

        Pod -> capacity = NewCapacity;
        Pod -> size = Size;
        Pod -> data = Data;
    }

    return 0;
}   /* _MFilePodSetDataSize () */

static
rc_t CC
_MFilePodDataRead (
                const struct XFSMFilePod * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
        /* There are some things about KFile:
         * On the end of file it returns 0 status and 0 NumRead
         * On all other stuff it return non zero status if there
         * is something missed or wrong.
         */
    size_t Size2Read = 0;

    XFS_CSA ( NumRead, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Buffer )
    XFS_CAN ( NumRead )

    if ( self -> size < Offset ) {
        return XFS_RC ( rcInvalid );
    }

    Size2Read = BufferSize;
    if ( self -> size < ( Offset + Size2Read ) ) {
        Size2Read = self -> size - Offset;
    }

    * NumRead = Size2Read;

    if ( Size2Read != 0 ) {
        memcpy (
                Buffer,
                self -> data + Offset,
                sizeof ( char ) * Size2Read
                );
    }

    return 0;
}   /* _MFilePodDataRead () */

static
rc_t CC
_MFilePodDataWrite (
                struct XFSMFilePod * self,
                uint64_t Offset,
                const void * Buffer,
                size_t BufferSize,
                size_t * NumWrite
)
{
    /* Here we are
     */
    rc_t RCt;
    uint64_t WholeSize;

    RCt = 0;
    WholeSize = Offset + BufferSize;

    if ( self -> size < WholeSize ) {
        RCt = _MFilePodSetDataSize ( self, WholeSize );
        if ( RCt != 0 ) {
            return RCt;
        }
        self -> size = WholeSize;
    }

    memcpy (
            self -> data + Offset,
            Buffer,
            sizeof ( char ) * BufferSize
            );

    return 0;
}   /* _MFilePodDataWrite () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))    _MFile and others
 ((*/

struct _MFile {
    struct KFile papahen;

    struct XFSMFilePod * pod;
};

/*))
 || _MFile_Temp virtual table
((*/

static rc_t CC _MFile_destroy (
                                struct KFile * self
                                );
static struct KSysFile_v1 * CC _MFile_get_sysfile (
                                const struct KFile * self,
                                uint64_t * Offset
                                );
static rc_t CC _MFile_random_access (
                                const struct KFile * self
                                );
static rc_t CC _MFile_get_size (
                                const struct KFile * self,
                                uint64_t * Size
                                );
static rc_t CC _MFile_set_size (
                                struct KFile * self,
                                uint64_t Size
                               );
static rc_t CC _MFile_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead
                                );
static rc_t CC _MFile_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrite
                                );
static uint32_t CC _MFile_get_type (
                                const struct KFile * self
                                );

#ifdef MINOR_2_JOJOBA
static rc_t CC _MFile_timed_read (
                                const struct KFile * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t BufferSize,
                                size_t * NumRead,
                                struct timeout_t * Tm
                                );
static rc_t CC _MFile_timed_write (
                                struct KFile * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t BufferSize,
                                size_t * NumWrit,
                                struct timeout_t * Tm
                                );
#endif /* MINOR_2_JOJOBA */

static struct KFile_vt_v1 _svtMFile = {
                                1,  /* maj */
                                1,  /* min */

                            /* start minor version == 0 */
                                _MFile_destroy,
                                _MFile_get_sysfile,
                                _MFile_random_access,
                                _MFile_get_size,
                                _MFile_set_size,
                                _MFile_read,
                                _MFile_write,
                            /* end minor version == 0 */

                            /* start minor version == 1 */
                                _MFile_get_type,
                            /* end minor version == 1 */

#ifdef MINOR_2_JOJOBA
                            /* start minor version == 2 */
                                _MFile_timed_read,
                                _MFile_timed_write,
                            /* end minor version == 2 */
#endif /* MINOR_2_JOJOBA */
};


/*))
 || _MFile constructor
((*/
LIB_EXPORT
rc_t CC
XFSMFileMake ( struct KFile ** File, struct XFSMFilePod * Pod )
{
    rc_t RCt;
    struct _MFile * TheFile;

    RCt = 0;
    TheFile = NULL;

    XFS_CSAN ( File )
    XFS_CAN ( File )
    XFS_CAN ( Pod )

    TheFile = calloc ( 1, sizeof ( struct _MFile ) );
    if ( TheFile == NULL ) {
        RCt = XFS_RC ( rcNull );
    }
    else {
        RCt = KFileInit (
                & ( TheFile -> papahen ),
                ( const KFile_vt * ) & _svtMFile,
                "MFile",
                "MFile",
                true,
                false
                );
        if ( RCt == 0 ) {
            RCt = XFSMFilePodAddRef ( Pod );
            if ( RCt == 0 ) {
                TheFile -> pod = Pod;

                * File = & ( TheFile -> papahen );
            }
        }
    }

    if ( RCt != 0 ) {
        * File = NULL;
        KFileRelease ( & ( TheFile -> papahen ) );
    }

    return RCt;
}   /* XFSMFileMake () */

/*))
 || _MFile virtual table implementation
((*/
rc_t CC
_MFile_destroy ( struct KFile * self )
{
    struct _MFile * File = ( struct _MFile * ) self;

    if ( File != NULL ) {
        if ( File -> pod != NULL ) {
            XFSMFilePodRelease ( File -> pod );
            File -> pod = NULL;
        }

        free ( File );
    }

    return 0;
}   /* _MFile_destroy () */

struct KSysFile_v1 * CC
_MFile_get_sysfile ( const struct KFile * self, uint64_t * Offset )
{
    return NULL;
}   /* _MFile_get_sysfile () */

rc_t CC
_MFile_random_access ( const struct KFile * self )
{
    XFS_CAN ( self )

    return 0;
}   /* _MFile_random_access () */

rc_t CC
_MFile_get_size ( const struct KFile * self, uint64_t * Size )
{
    const struct _MFile * File = ( const struct _MFile * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> pod )

    return XFSMFilePodDataSize ( File -> pod, Size );
}   /* _MFile_get_size () */

rc_t CC
_MFile_set_size ( struct KFile * self, uint64_t Size )
{
    struct _MFile * File = ( struct _MFile * ) self;

    XFS_CAN ( File )
    XFS_CAN ( File -> pod )

    return _MFilePodSetDataSize ( File -> pod, Size );
}   /* _MFile_set_size () */

rc_t CC
_MFile_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead
)
{
     const struct _MFile * File = ( const struct _MFile * ) self;

     XFS_CAN ( File )
     XFS_CAN ( File -> pod )

     return _MFilePodDataRead (
                            File -> pod,
                            Offset,
                            Buffer,
                            BufferSize,
                            NumRead
                            );
}   /* _MFile_read () */

rc_t CC
_MFile_write (
                struct KFile * self,
                uint64_t Offset,
                const void * Buffer,
                size_t BufferSize,
                size_t * NumWrite
)
{
     struct _MFile * File = ( struct _MFile * ) self;

     XFS_CAN ( File )
     XFS_CAN ( File -> pod )

     return _MFilePodDataWrite (
                            File -> pod,
                            Offset,
                            Buffer,
                            BufferSize,
                            NumWrite
                            );
}   /* _MFile_write () */

uint32_t CC
_MFile_get_type ( const struct KFile * self )
{
    return kptFile;
}   /* _MFile_get_type () */


#ifdef MINOR_2_JOJOBA
rc_t CC
_MFile_timed_read (
                const struct KFile * self,
                uint64_t Offset,
                void * Buffer,
                size_t BufferSize,
                size_t * NumRead,
                struct timeout_t * Tm
)
{
     const struct _MFile * File = ( const struct _MFile * ) self;

     XFS_CAN ( File )
     XFS_CAN ( File -> pod )

     return _MFilePodDataRead (
                            File -> pod,
                            Offset,
                            Buffer,
                            BufferSize,
                            NumRead
                            );
}   /* _MFile_timed_read () */

rc_t CC
_MFile_timed_write (
                struct KFile * self,
                uint64_t Offset,
                const void * Buffer,
                size_t BufferSize,
                size_t * NumWrite,
                struct timeout_t * Tm
)
{
     struct _MFile * File = ( struct _MFile * ) self;

     XFS_CAN ( File )
     XFS_CAN ( File -> pod )

     return _MFilePodDataWrite (
                            File -> pod,
                            Offset,
                            Buffer,
                            BufferSize,
                            NumWrite
                            );
}   /* _MFile_timed_write () */

#endif /* MINOR_2_JOJOBA */
