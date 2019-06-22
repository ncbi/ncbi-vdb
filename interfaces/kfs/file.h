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

#ifndef _h_kfs_file_
#define _h_kfs_file_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfs_file_v1_
#include <kfs/file-v1.h>
#endif

#ifndef _h_kfs_file_v2_
#include <kfs/file-v2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KFileDesc
 *  describes basic file types
 */
enum KFileDesc
{
    kfdNull,
    kfdInvalid,
    kfdFile,
    kfdCharDev,
    kfdBlockDev,
    kfdFIFO,
    kfdSocket,

    /* the type enum may be extended */
    kfdLastDefined
};


/*--------------------------------------------------------------------------
 * KFile
 *  a file is normally created with a KDirectory
 *  optionally, the standard i/o files may be created directly
 */
#define KFileAddRef NAME_VERS ( KFileAddRef, KFILE_VERS )
#define KFileDuplicate NAME_VERS ( KFileDuplicate, KFILE_VERS )
#define KFileRelease NAME_VERS ( KFileRelease, KFILE_VERS )
#define KFileRandomAccess NAME_VERS ( KFileRandomAccess, KFILE_VERS )
#define KFileType NAME_VERS ( KFileType, KFILE_VERS )
#define KFileSize NAME_VERS ( KFileSize, KFILE_VERS ) 
#define KFileSetSize NAME_VERS ( KFileSetSize, KFILE_VERS )
#define KFileRead NAME_VERS ( KFileRead, KFILE_VERS )
#define KFileTimedRead NAME_VERS ( KFileTimedRead, KFILE_VERS )
#define KFileReadAll NAME_VERS ( KFileReadAll, KFILE_VERS )
#define KFileTimedReadAll NAME_VERS ( KFileTimedReadAll, KFILE_VERS )
#define KFileReadExactly NAME_VERS ( KFileReadExactly, KFILE_VERS )
#define KFileTimedReadExactly NAME_VERS ( KFileTimedReadExactly, KFILE_VERS )
#define KFileReadChunked KFileReadChunked_v1
#define KFileTimedReadChunked KFileTimedReadChunked_v1
#define KFileWrite NAME_VERS ( KFileWrite, KFILE_VERS )
#define KFileTimedWrite NAME_VERS ( KFileTimedWrite, KFILE_VERS )
#define KFileWriteAll NAME_VERS ( KFileWriteAll, KFILE_VERS )
#define KFileTimedWriteAll NAME_VERS ( KFileTimedWriteAll, KFILE_VERS )
#define KFileWriteExactly NAME_VERS ( KFileWriteExactly, KFILE_VERS )
#define KFileTimedWriteExactly NAME_VERS ( KFileTimedWriteExactly, KFILE_VERS )
#define KFileMakeStdIn NAME_VERS ( KFileMakeStdIn, KFILE_VERS )
#define KFileMakeStdOut NAME_VERS ( KFileMakeStdOut, KFILE_VERS )
#define KFileMakeStdErr NAME_VERS ( KFileMakeStdErr, KFILE_VERS )


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_file_ */
