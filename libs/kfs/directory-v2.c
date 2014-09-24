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

#define TRACK_REFERENCES 0

#include <kfs/extern.h>

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <kfs/impl.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/path.h>
#include <sysalloc.h>

#include <stdlib.h>

/*--------------------------------------------------------------------------
 * KDirectory
 *  a directory
 */

static const char classname[] = "KDirectory";

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */

// !!! Probably wrong way to implement this function
LIB_EXPORT KDirectory_v2 * CC KDirectoryDuplicate_v2 ( const KDirectory_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcCopying );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to dupliacte directory" );

    else switch ( KRefcountAdd ( & self -> refcount, classname ) )
         {
         case krefLimit:
             INTERNAL_ERROR ( xcRefcountOutOfBounds, "references to directory surpass limit" );
             break;
         default:
             INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
         }

    return NULL;
}

/* Release
 *  discard reference to directory
 *  ignores NULL references
 */
LIB_EXPORT void CC KDirectoryRelease_v2 ( const KDirectory_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcReleasing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to release directory" );

    else switch ( KRefcountDrop ( & self -> refcount, classname ) )
         {
         case krefWhack:
         {
             switch ( self -> vt -> v2 . maj )
             {
             case 2:
                 ( * self -> vt -> v2 . destroy ) ( ( KDirectory_v2 * ) self, ctx );
                 break;
             default:
                 INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
             }
             
             if ( FAILED () )
             {
                 KRefcountInit ( ( KRefcount * ) & self -> refcount, 1, classname, "failed-release", "orphan" );
                 INTERNAL_ERROR ( xcReleaseFailed, "failed to release reference, - orphan" );
             }
         }
         case krefNegative:
             INTERNAL_ERROR ( xcRefcountOutOfBounds, "references to directory cannot be negative" );
         }
}

/* List
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "." will be
 *  interpreted as format string if arguments follow
 */

LIB_EXPORT KNamelist * CC KDirectoryList_v2 ( const KDirectory_v2 *self, ctx_t ctx,
    bool ( CC * f ) ( const KDirectory_v2 *dir, const char *name, void *data ),
    void *data, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcListing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to create directory listing" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . list_dir ) ( self, ctx, f, data, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
    }

    return NULL;
}

/* Visit
 *  visit each path under designated directory,
 *  recursively if so indicated
 *
 *  "recurse" [ IN ] - if non-zero, recursively visit sub-directories
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to execute
 *  on each path. receives a base directory and relative path
 *  for each entry. if "f" returns non-zero, the iteration will
 *  terminate and that value will be returned. NB - "dir" will not
 *  be the same as "self".
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
LIB_EXPORT void CC KDirectoryVisit_v2 ( const KDirectory_v2 *self, ctx_t ctx, bool recurse,
    bool ( CC * f ) ( const KDirectory_v2 *dir, uint32_t type, const char *name, void *data ),
    void *data, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcVisiting );
    
    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to visit directory" );

    if ( f == NULL )
        INTERNAL_ERROR ( xcFunctionNull, "function pointer is null" );

    if ( path == NULL )
        /* FIX ME  path = "." */ ;

    switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . visit ) ( self, ctx, recurse, f, data, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
    }
}

/* VisitUpdate
 *  
 */
LIB_EXPORT void CC KDirectoryVisitUpdate_v2 ( KDirectory_v2 *self, ctx_t ctx, bool recurse,
    bool ( CC * f ) ( KDirectory_v2 *dir, uint32_t type, const char *name, void *data ),
    void *data, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcVisiting );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to visit directory" );
    else if ( f == NULL )
        INTERNAL_ERROR ( xcFunctionNull, "function pointer is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcFileReadOnly, "directory does not have write permission" );

    else if ( path == NULL )
        /* FIX ME path = "." */;

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . visit_update )( self, ctx, recurse, f, data, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
    }
}

/* PathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
LIB_EXPORT uint32_t CC KDirectoryPathType_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcIdentifying );

    if ( self == NULL ) 
        INTERNAL_ERROR ( xcSelfNull, "failed to get path type" );

    if ( path == NULL )
        return kptBadPath;

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . path_type ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
    }

    return kptBadPath;
}

/* ResolvePath
 *  resolves path to an absolute or directory-relative path
 *
 *  "absolute" [ IN ] - if non-zero, always give a path starting
 *  with '/'. NB - if the directory is chroot'd, the absolute path
 *  will still be relative to directory root.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character set
 *  the resolved path will be directory relative
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target path. NB - need not exist.
 */
LIB_EXPORT KPath * CC KDirectoryResolvePath_v2 ( const KDirectory_v2 *self, ctx_t ctx, 
    bool absolute, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcResolving );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to resolve path" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . resolve_path )
            ( self, ctx, absolute, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has an invalid version" );
    }

    return NULL;
}

/* ResolveAlias
 *  resolves an alias path to its immediate target
 *  NB - the resolved path may be yet another alias
 *
 *  "absolute" [ IN ] - if non-zero, always give a path starting
 *  with '/'. NB - if the directory is chroot'd, the absolute path
 *  will still be relative to directory root.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character set
 *  the resolved path will be directory relative
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting an object presumed to be an alias.
 */

LIB_EXPORT KPath * CC KDirectoryResolveAlias_v2 ( const KDirectory_v2 *self, ctx_t ctx, bool absolute,
    const KPath *alias )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcResolving );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to resolve alias" );

    else if ( alias == NULL )
        INTERNAL_ERROR ( xcPathNull, "alias is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . resolve_alias )
            ( self, ctx, absolute, alias );
    }

    return NULL;
}

/* Rename
 *  rename an object accessible from directory, replacing
 *  any existing target object of the same type
 *
 *  "from" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 *
 *  "to" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 */
LIB_EXPORT void CC KDirectoryRename_v2 ( KDirectory_v2 *self, ctx_t ctx, bool force, const KPath *from, const KPath *to )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcRenaming );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to rename directory" );

    else if ( from == NULL || to == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have write permissions" );
    
    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . rename ) ( self, ctx, force, from, to );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

}

/* Remove
 *  remove an accessible object from its directory
 *
 *  "force" [ IN ] - if non-zero and target is a directory,
 *  remove recursively
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT void CC KDirectoryRemove_v2 ( KDirectory_v2 *self, ctx_t ctx, bool force, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcRemoving );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to remove directory" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have write permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . remove ) ( self, ctx, force, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* ClearDir
 *  remove all directory contents
 *
 *  "force" [ IN ] - if non-zero and directory entry is a
 *  sub-directory, remove recursively
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 */
LIB_EXPORT void CC KDirectoryClearDir_v2 ( KDirectory_v2 *self, ctx_t ctx, bool force, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcRemoving );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to clear directory" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have write permissions" );

    switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . clear_dir ) ( self, ctx, force, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* Access
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT uint32_t CC KDirectoryAccess_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to access directory" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . access ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
    
    return 0;
}

/* SetAccess
 *  set access to object a la Unix "chmod"
 *
 *  "recurse" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 *
 *  "access" [ IN ] and "mask" [ IN ] - definition of change
 *  where "access" contains new bit values and "mask defines
 *  which bits should be changed.
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT void CC KDirectorySetAccess_v2 ( KDirectory_v2 *self, ctx_t ctx, bool recurse,
    uint32_t access, uint32_t mask, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to set access to directory" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have write permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        /* TBD - perhaps limit bits to lower 9 */
        ( * self -> vt -> v2 . set_access )
            ( self, ctx, recurse, access, mask, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* Date
 *  get date/time to object
 *
 *  "date" [ OUT ] - return parameter
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT KTime_t CC KDirectoryDate_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . date ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return 0;
}


/* SetDate
 *  set date to object
 *
 *  "recurse" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 *
 *  "date" [ IN ] - new time and date for the object
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT void CC KDirectorySetDate_v2 ( KDirectory_v2 *self, ctx_t ctx, bool recurse,
    KTime_t date, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcUpdating );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . setdate ) ( self, ctx, recurse, date, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}


/* CreateAlias
 *  creates a path alias according to create mode
 *
 *  "access" [ IN ] - standard Unix directory access mode
 *  used when "mode" has kcmParents set and alias path does
 *  not exist.
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias
 */
LIB_EXPORT void CC KDirectoryCreateAlias_v2 ( KDirectory_v2 *self, ctx_t ctx,
    uint32_t access, KCreateMode mode, const KPath *targ, const KPath *alias )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcUpdating );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    if ( targ == NULL || alias == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . create_alias )( self, ctx, access, mode, targ, alias );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* OpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT const KFile_v2 * CC KDirectoryOpenFileRead_v2 ( const KDirectory_v2 *self, ctx_t ctx,
    const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcOpening );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . open_file_read ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return NULL;
}

/* OpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT KFile_v2 * CC KDirectoryOpenFileWrite_v2 ( KDirectory_v2 *self, ctx_t ctx,
    bool update, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcOpening );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . open_file_write )
            ( self, ctx, update, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return NULL;
}

/* CreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT KFile_v2 * CC KDirectoryCreateFile_v2 ( KDirectory_v2 *self, ctx_t ctx,
    bool update, uint32_t access, KCreateMode mode, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcCreating );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . create_file )
            ( self, ctx, update, access, mode, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return NULL;
}

/* FileLocator
 *  returns locator in bytes of target file
 *
 *  "locator" [ OUT ] - return parameter for file locator
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT uint64_t CC KDirectoryFileLocator_v2 ( const KDirectory_v2 *self, ctx_t ctx,
   const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . file_locator ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return 0;
}

/* FileSize
 *  returns size in bytes of target file
 *
 *  "size" [ OUT ] - return parameter for file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT uint64_t CC KDirectoryFileSize_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . file_size ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return 0;
}

/* FilePhysicalSize
 *  returns size in bytes of target file
 *
 *  "size" [ OUT ] - return parameter for file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT uint64_t CC KDirectoryFilePhysicalSize_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . file_phys_size ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return 0;
}

/* SetFileSize
 *  sets size in bytes of target file
 *
 *  "size" [ IN ] - new file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT void CC KDirectorySetFileSize_v2 ( KDirectory_v2 *self, ctx_t ctx,
    uint64_t size, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcUpdating );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . set_size ) ( self, ctx, size, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* FileContiguous
 *  returns true if the file is "contiguous".  Chunked or sparse files are not
 *  contiguous while most data files are.  Virtual generated files would likely
 *  not be contiguous.  
 *
 *  "contiguous" [ OUT ] - return parameter for file contiguous
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT bool CC KDirectoryFileContiguous_v2 ( const KDirectory_v2 *self, ctx_t ctx, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . file_contiguous ) ( self, ctx, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return false;
}

/* OpenDirRead
 * KDirectoryOpenDirUpdate
 *  opens a sub-directory
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 */
LIB_EXPORT const KDirectory_v2 * CC KDirectoryOpenDirRead_v2 ( const KDirectory_v2 *self, ctx_t ctx,
    bool chroot, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcOpening );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    if ( path == NULL )
        /* FIX ME path = "." */;

    switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . open_dir_read )
            ( self, ctx, chroot, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
    
    return NULL;
}

LIB_EXPORT KDirectory_v2 * CC KDirectoryOpenDirUpdate_v2 ( KDirectory_v2 *self, ctx_t ctx,
    bool chroot, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcOpening );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    if ( path == NULL )
        /* FIX ME path = "." */;

    switch ( self -> vt -> v2 . maj )
    {
    case 2:
        return ( * self -> vt -> v2 . open_dir_update )
            ( self, ctx, chroot, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    return NULL;
}

/* CreateDir
 *  create a sub-directory
 *
 *  "access" [ IN ] - standard Unix directory mode, e.g.0775
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 */
LIB_EXPORT void CC KDirectoryCreateDir_v2 ( KDirectory_v2 *self, ctx_t ctx,
    uint32_t access, KCreateMode mode, const KPath *path )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get directory date" );

    else if ( path == NULL )
        INTERNAL_ERROR ( xcPathNull, "path is null" );

    if ( self -> read_only )
        INTERNAL_ERROR ( xcDirectoryWriteOnly, "directory does not have read permissions" );

    else switch ( self -> vt -> v2 . maj )
    {
    case 2:
        ( * self -> vt -> v2 . create_dir ) ( self, ctx, access, mode, path );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

/* Init
 *  initialize a newly allocated directory object
 */
LIB_EXPORT void CC KDirectoryInit_v2 ( KDirectory_v2 *self, ctx_t ctx, const KDirectory_vt *vt,
    const char * class_name, const char * path, bool update )
{
    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get initialize directory" );

    else if ( vt == NULL )
        INTERNAL_ERROR ( xcInterfaceNull, "null vtable" );

    else switch ( vt -> v2 . maj )
    {
    case 2:
#if _DEBUGGING
        if ( vt -> v2 . file_phys_size == NULL  ||
             vt -> v2 . file_locator == NULL    ||
             vt -> v2 . get_sysdir == NULL      ||
             vt -> v2 . setdate == NULL         ||
             vt -> v2 . date == NULL            ||
             vt -> v2 . create_dir == NULL      ||
             vt -> v2 . open_dir_update == NULL ||
             vt -> v2 . open_dir_read == NULL   ||
             vt -> v2 . set_size == NULL        ||
             vt -> v2 . file_size == NULL       ||
             vt -> v2 . create_file == NULL     ||
             vt -> v2 . open_file_write == NULL ||
             vt -> v2 . open_file_read == NULL  ||
             vt -> v2 . create_alias == NULL    ||
             vt -> v2 . set_access == NULL      ||
             vt -> v2 . access == NULL          ||
             vt -> v2 . clear_dir == NULL       ||
             vt -> v2 . remove == NULL          ||
             vt -> v2 . rename == NULL          ||
             vt -> v2 . resolve_alias == NULL   ||
             vt -> v2 . resolve_path == NULL    ||
             vt -> v2 . path_type == NULL       ||
             vt -> v2 . visit_update == NULL    ||
             vt -> v2 . visit == NULL           ||
             vt -> v2 . list_dir == NULL        ||
             vt -> v2 . destroy == NULL )
            
            INTERNAL_ERROR ( xcInterfaceNull, "null vtable" );
#endif
        break;
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }

    self -> vt = vt;
    self -> read_only = ( uint8_t ) ( update == 0 );


    KRefcountInit ( &self->refcount, 1, class_name, "init", path );
}

/* DestroyFile
 *  does whatever is necessary with an unreferenced file
 */
LIB_EXPORT void CC KDirectoryDestroyFile_v2 ( const KDirectory_v2 *self, ctx_t ctx, KFile_v2 *f )
{
    FUNC_ENTRY ( ctx, rcFS, rcDirectory, rcDestroying );

    const KDirectory_vt *vt = self -> vt;
    switch ( vt -> v2 . maj )
    {
    case 2:
        if ( vt -> v2 . destroy_file != NULL )
            ( * vt -> v2 . destroy_file ) ( ( KDirectory_v2 * ) self, ctx, f );
        else
            KFileDestroy_v2 ( f, ctx );
    default:
        INTERNAL_ERROR ( xcDirectoryInvalidVersion, "directory has invalid version" );
    }
}

#if 0

/* GetSysDir
 *  returns an underlying system file object
 */
LIB_EXPORT struct KSysDir* CC KDirectoryGetSysDir_v2 ( const KDirectory_v2 *self, ctx_t ctx )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v2 . maj )
        {
        case 1:
            if ( self -> vt -> v2 . min > 0 )
                return ( * self -> vt -> v2 . get_sysdir ) ( self );
            break;
        }
    }
    return NULL;
}


static rc_t copy_loop( const KFile *src, KFile *dst, size_t bsize )
{
    rc_t rc = 0;
    uint64_t pos = 0;
    size_t num_read = 1;

    char * buffer = malloc( bsize );
    if ( buffer == NULL )
        return RC( rcFS, rcDirectory, rcCopying, rcMemory, rcExhausted );

    while ( rc == 0 && num_read > 0 )
    {
        rc = KFileRead ( src, pos, buffer, bsize, &num_read );
        if ( rc == 0 && num_read > 0 )
        {
            size_t num_writ;
            rc = KFileWrite ( dst, pos, buffer, num_read, &num_writ );
            pos += num_read;
        }
    }
    free( buffer );
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryCopyPath ( const KDirectory_v2 *src_dir,
    KDirectory_v2 *dst_dir, const char *src_path, const char * dst_path )
{
    rc_t rc;
    struct KFile const *f_src;

    if ( src_dir == NULL || dst_dir == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcSelf, rcNull );
    if ( src_path == NULL || dst_path == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcParam, rcNull );

    rc = KDirectoryOpenFileRead ( src_dir, &f_src, "%s", src_path );
    if ( rc == 0 )
    {
        uint32_t pt = KDirectoryPathType ( dst_dir, "%s", dst_path );
        switch( pt )
        {
            case kptFile : ; /* intentional fall through! */
            case kptDir  : rc = KDirectoryRemove ( dst_dir, true, "%s", dst_path ); break;
        }
        if ( rc == 0 )
        {
            struct KFile *f_dst;
            uint32_t access = 0664;
            rc = KDirectoryCreateFile ( dst_dir, &f_dst, false, access, kcmCreate, "%s", dst_path );
            if ( rc == 0 )
            {
                rc = copy_loop( f_src, f_dst, 1024 * 16 );
            }
        }
    }
    return rc;
}


static rc_t build_obj_path( char **s, const KPath *path, const char * objname )
{
    rc_t rc;
    size_t lp = string_size( path );
    size_t l = lp + string_size( objname ) + 2;
    *s = malloc( l );
    if ( *s == NULL )
        rc = RC( rcFS, rcDirectory, rcCopying, rcMemory, rcExhausted );
    else
    {
        size_t written;
        const char * concat = ( ( path[ lp - 1 ] == '/' ) ? "%s%s" : "%s/%s" );
        rc = string_printf( *s, l, &written, concat, path, objname );
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryCopyPaths( const KDirectory_v2 * src_dir,
    KDirectory_v2 * dst_dir, bool recursive, const char *src, const char *dst )
{
    rc_t rc;
    struct KNamelist *list;

    if ( src_dir == NULL || dst_dir == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcSelf, rcNull );
    if ( src == NULL || dst == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcParam, rcNull );

    rc = KDirectoryList ( src_dir, &list, NULL, NULL, "%s", src );
    if ( rc == 0 )
    {
        uint32_t pt = KDirectoryPathType ( dst_dir, "%s", dst );
        /* if the output-directory does not exist: create it! */
        switch( pt )
        {
            case kptFile : rc = KDirectoryRemove ( dst_dir, true, "%s", dst );
                            /* intentially no break ! */

            case kptNotFound : if ( rc == 0 )
                                    rc = KDirectoryCreateDir ( dst_dir, 0775, kcmCreate | kcmParents, "%s", dst );
                               break;
        }
        if ( rc == 0 )
        {
            uint32_t i, n;
            rc = KNamelistCount ( list, &n );
            for ( i = 0; i < n && rc == 0; ++i )
            {
                const char *name;
                rc = KNamelistGet ( list, i, &name );
                if ( rc == 0 )
                {
                    char *src_obj;
                    rc = build_obj_path( &src_obj, src, name );
                    if ( rc == 0 )
                    {
                        char *dst_obj;
                        rc = build_obj_path( &dst_obj, dst, name );
                        if ( rc == 0 )
                        {
                            pt = KDirectoryPathType ( src_dir, "%s", src_obj );
                            switch( pt )
                            {
                                case kptFile : rc = KDirectoryCopyPath ( src_dir, dst_dir, src_obj, dst_obj );
                                               break;

                                case kptDir  : if ( recursive )
                                                    rc = KDirectoryCopyPaths( src_dir, dst_dir, true, src_obj, dst_obj );
                                               break;
                            }
                            free( dst_obj );
                        }
                        free( src_obj );
                    }
                }
            }
        }
        KNamelistRelease ( list );
    }
    return rc;
}


LIB_EXPORT rc_t CC KDirectoryCopy( const KDirectory_v2 * src_dir,
    KDirectory_v2 * dst_dir, bool recursive, const char *src, const char *dst )
{
    rc_t rc = 0;
    uint32_t pt;

    if ( src_dir == NULL || dst_dir == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcSelf, rcNull );
    if ( src == NULL || dst == NULL )
        return RC ( rcFS, rcDirectory, rcCopying, rcParam, rcNull );

    pt = KDirectoryPathType ( src_dir, "%s", src );
    switch( pt )
    {
        case kptFile : rc = KDirectoryCopyPath ( src_dir, dst_dir, src, dst );
                        break;
        case kptDir  : rc = KDirectoryCopyPaths ( src_dir, dst_dir, recursive, src, dst );
                        break;
    }
    return rc;
}
#endif
