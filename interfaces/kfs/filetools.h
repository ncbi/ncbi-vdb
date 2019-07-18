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

#ifndef _h_kfs_filetools_
#define _h_kfs_filetools_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfs_file_
#include <kfs/file.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* PLEASE DO NOT VERSION these functions, they use only the standard KFile-Interface */


/* LoadKFileToNameList
 * parses the content of a file as text, decomposes this text into lines,
 * and adds the lines to the namelist. Because VNamelist cannot contain empty lines,
 * empty lines are translated into a string containing a single space-character.
 * The parsing accepts POSIX/Mac and Windows line endings.
 */
KFS_EXTERN rc_t CC LoadKFileToNameList( struct KFile const * self, struct VNamelist * namelist );


/* LoadFileByNameToNameList
 * calls internally LoadKFileToNameList(), but does construct the KFile internally from the given path
 */
KFS_EXTERN rc_t CC LoadFileByNameToNameList( struct VNamelist * namelist, const char * filename );


/* Walks a file by calling the on_line for each line
  */

KFS_EXTERN rc_t CC ProcessFileLineByLine( struct KFile const * self,
    rc_t ( CC * on_line )( const String * line, void * data ), void * data );

/* WriteNameListToKFile
 * Writes the lines of namelist into the given KFile.
 * Uses the string given in delim as line-ending
 */
KFS_EXTERN rc_t CC WriteNameListToKFile( struct KFile * self, const VNamelist * namelist, 
                                         const char * delim );

/* WriteNamelistToFileByName
 * calls internally WriteNameListToKFile(), but does construct the KFile internally from the given path
 * if the file exists, it will be overwritten. if the files does not exist it will be created
 */
KFS_EXTERN rc_t CC WriteNamelistToFileByName( const VNamelist * namelist,
     const char * filename, const char * delim );

/* ReadDirEntriesIntoToNamelist
 * creates a VNamelist-instance, iterates over the entries of the given KDirectory,
 * and enters them into the created Namelist ( sorted on request )
 */
KFS_EXTERN rc_t CC ReadDirEntriesIntoToNamelist( VNamelist ** namelist, const KDirectory * dir,
    bool perform_sort, bool add_files, bool add_dirs, const char * path );
     
#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_filetools_ */
