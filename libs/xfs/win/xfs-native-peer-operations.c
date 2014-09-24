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

 /* Something unusual: Lyrics
  *
  * That file contains only one useful function:
  *
  *                       XFS_Private_InitOperations
  *
  * it needed to fill _DOKAN_OPERATIONS structure.
  *
  * I put here all possible stubs to _DOKAN_OPERATIONS
  * functions. However, I will use only several of them while 
  * structure initialisation. So, if You want to extend functionality,
  * please edit already ready stub and add new function to structure
  * initialized .
  *
  */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <string.h> /* we are using memset() */

#include "xfs-native-peer-operations.h"

rc_t
XFS_Private_InitOperations ( struct _DOKAN_OPERATIONS * Ops )
{
    if ( Ops == NULL ) {
        return RC ( rcExe, rcData, rcConstructing, rcParam, rcNull );
    }

    memset ( Ops, 0, sizeof( struct _DOKAN_OPERATIONS ) );


    return 0;
}   /* XFS_Private_InitOperations() */


int DOKAN_CALLBACK
CreateFile (
            LPCWSTR FileName,
            DWORD DesiredAccess,
            DWORD ShareMode,
            DWORD CreationDisposition,
            DWORD FlagsAndAttributes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* CreateFile() */

int DOKAN_CALLBACK
OpenDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* OpenDirectory() */

int DOKAN_CALLBACK
CreateDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* CreateDirectory() */

int DOKAN_CALLBACK
Cleanup (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* Cleanup() */

int DOKAN_CALLBACK
CloseFile (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* CloseFile() */

int DOKAN_CALLBACK
ReadFile (
            LPCWSTR FileName,
            LPVOID Buffer,
            DWORD NumberOfBytesToRead,
            LPDWORD NumberOfBytesRead,
            LONGLONG Offset,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* ReadFile() */
       

int DOKAN_CALLBACK
WriteFile (
            LPCWSTR FileName,
            LPCVOID Buffer,
            DWORD NumberOfBytesToWrite,
            LPDWORD NumberOfBytesWritten,
            LONGLON Offset,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* WriteFile() */


int DOKAN_CALLBACK
FlushFileBuffers (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* FlushFileBuffers() */


int DOKAN_CALLBACK
GetFileInformation (
            LPCWSTR FileName,
            LPBY_HANDLE_FILE_INFORMATION Buffer,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* GetFileInformation() */
       

int DOKAN_CALLBACK
FindFiles (
            LPCWSTR PathName,
            PFillFindData FindDataCallback,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* FindFiles() */


int DOKAN_CALLBACK
FindFilesWithPattern (
            LPCWSTR PathName,
            LPCWSTR SearchPattern,
            PFillFindData FindDataCallback,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* FindFilesWithPattern() */


int DOKAN_CALLBACK
SetFileAttributes (
            LPCWSTR FileName,
            DWORD FileAttributes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* SetFileAttributes() */


int DOKAN_CALLBACK
SetFileTime (
            LPCWSTR FileName,
            CONST FILETIME * CreationTime,
            CONST FILETIME * LastAccessTime,
            CONST FILETIME * LastWriteTime,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* SetFileTime() */

int DOKAN_CALLBACK
DeleteFile (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* DeleteFile() */

int DOKAN_CALLBACK
DeleteDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* DeleteDirectory() */


int DOKAN_CALLBACK
MoveFile (
            LPCWSTR ExistingFileName,
            LPCWSTR NewFileName,
            BOOL ReplaceExisiting,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* MoveFile() */


int DOKAN_CALLBACK
SetEndOfFile (
            LPCWSTR FileName,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* SetEndOfFile() */


int DOKAN_CALLBACK
SetAllocationSize (
            LPCWSTR FileName,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* SetAllocationSize() */

int DOKAN_CALLBACK
LockFile (
            LPCWSTR FileName,
            LONGLONG ByteOffset,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* LockFile() */

int DOKAN_CALLBACK
UnlockFile (
            LPCWSTR FileName,
            LONGLONG ByteOffset,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* UnlockFile() */

int DOKAN_CALLBACK
GetDiskFreeSpace (
            PULONGLONG FreeBytesAvailable,
            PULONGLONG TotalNumberOfBytes,
            PULONGLONG TotalNumberOfFreeBytes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* GetDiskFreeSpace() */

int DOKAN_CALLBACK
GetVolumeInformation (
            LPWSTR VolumeNameBuffer,
            DWORD VolumeNameSize,
            LPDWORD VolumeSerialNumber,
            LPDWORD MaximumComponentLength,
            LPDWORD FileSystemFlags,
            LPWSTR FileSystemNameBuffer,
            DWORD FileSystemNameSize,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* GetVolumeInformation() */


int DOKAN_CALLBACK
Unmount (
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* Unmount() */


int DOKAN_CALLBACK
GetFileSecurity (
            LPCWSTR FileName,
            PSECURITY_INFORMATION SecurityInformation,
            PSECURITY_DESCRIPTOR SecurityDescriptor,
            ULONG SecurityDescriptorLength,
            PULONG LengthNeeded,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* GetFileSecurity() */

int DOKAN_CALLBACK
SetFileSecurity (
            LPCWSTR FileName,
            PSECURITY_INFORMATION SecurityInformation,
            PSECURITY_DESCRIPTOR SecurityDescriptor,
            ULONG SecurityDescriptorLength,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    return -EPERM;
}   /* SetFileSecurity() */
