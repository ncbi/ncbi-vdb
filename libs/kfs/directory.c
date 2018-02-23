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
#include <kfs/impl.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
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
LIB_EXPORT rc_t CC KDirectoryAddRef_v1 ( const KDirectory_v1 *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, classname ) )
        {
        case krefLimit:
            return RC ( rcFS, rcDirectory, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Release
 *  discard reference to directory
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KDirectoryRelease_v1 ( const KDirectory_v1 *cself )
{
    KDirectory_v1 *self = ( KDirectory* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, classname ) )
        {
        case krefWhack:
        {
            rc_t rc;

            switch ( self -> vt -> v1 . maj )
            {
            case 1:
                rc = ( * self -> vt -> v1 . destroy ) ( self );
                break;

            default:
                rc = RC ( rcFS, rcDirectory, rcReleasing, rcInterface, rcBadVersion );
            }

            if ( rc != 0 )
                KRefcountInit ( & self -> refcount, 1, classname, "failed-release", "orphan" );
            return rc;
        }
        case krefNegative:
            return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
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
LIB_EXPORT rc_t CC KDirectoryList_v1 ( const KDirectory_v1 *self, struct KNamelist **list,
    bool ( CC * f ) ( const KDirectory_v1 *dir, const char *name, void *data ),
    void *data, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVList ( self, list, f, data, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVList ( const KDirectory_v1 *self, struct KNamelist **list,
    bool ( CC * f ) ( const KDirectory_v1 *dir, const char *name, void *data ),
    void *data, const char *path, va_list args )
{
    if ( list == NULL )
        return RC ( rcFS, rcDirectory, rcListing, rcParam, rcNull );

    * list = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcListing, rcSelf, rcNull );

    if ( path == NULL || path [ 0 ] == 0 )
        path = ".";

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . list_dir ) ( self, list, f, data, path, args );
    }

    return RC ( rcFS, rcDirectory, rcListing, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryVisit_v1 ( const KDirectory_v1 *self, bool recurse,
    rc_t ( CC * f ) ( const KDirectory_v1 *dir, uint32_t type, const char *name, void *data ),
    void *data, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVVisit ( self, recurse, f, data, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVVisit ( const KDirectory_v1 *self, bool recurse,
    rc_t ( CC * f ) ( const KDirectory_v1 *dir, uint32_t type, const char *name, void *data ),
    void *data, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcVisiting, rcSelf, rcNull );
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcVisiting, rcFunction, rcNull );

    if ( path == NULL || path [ 0 ] == 0 )
        path = ".";

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . visit ) ( self, recurse, f, data, path, args );
    }

    return RC ( rcFS, rcDirectory, rcVisiting, rcInterface, rcBadVersion );
}

/* VisitUpdate
 *  
 */
LIB_EXPORT rc_t CC KDirectoryVisitUpdate_v1 ( KDirectory_v1 *self, bool recurse,
    rc_t ( CC * f ) ( KDirectory_v1 *dir, uint32_t type, const char *name, void *data ),
    void *data, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVVisitUpdate ( self, recurse, f, data, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVVisitUpdate ( KDirectory_v1 *self, bool recurse,
    rc_t ( CC * f ) ( KDirectory_v1 *dir, uint32_t type, const char *name, void *data ),
    void *data, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcVisiting, rcSelf, rcNull );
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcVisiting, rcFunction, rcNull );

    if ( path == NULL || path [ 0 ] == 0 )
        path = ".";

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcVisiting, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . visit_update )
            ( self, recurse, f, data, path, args );
    }

    return RC ( rcFS, rcDirectory, rcVisiting, rcInterface, rcBadVersion );
}

/* PathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
LIB_EXPORT uint32_t CC KDirectoryPathType_v1 ( const KDirectory_v1 *self, const char *path, ... )
{
    uint32_t type;
    va_list args;

    va_start ( args, path );
    type = KDirectoryVPathType ( self, path, args );
    va_end ( args );

    return type;
}

LIB_EXPORT uint32_t CC KDirectoryVPathType ( const KDirectory_v1 *self, const char *path, va_list args )
{
    if ( self == NULL || path == NULL || path [ 0 ] == 0 )
        return kptBadPath;

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . path_type ) ( self, path, args );
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
LIB_EXPORT rc_t CC KDirectoryResolvePath_v1 ( const KDirectory_v1 *self, bool absolute,
    char *resolved, size_t rsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVResolvePath ( self, absolute,
        resolved, rsize, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVResolvePath ( const KDirectory_v1 *self, bool absolute,
    char *resolved, size_t rsize, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcResolving, rcSelf, rcNull );

    /* allow NULL buffers of 0 size */
    if ( resolved == NULL && rsize != 0 )
        return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . resolve_path )
            ( self, absolute, resolved, rsize, path, args );
    }

    return RC ( rcFS, rcDirectory, rcResolving, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryResolveAlias_v1 ( const KDirectory_v1 *self, bool absolute,
    char *resolved, size_t rsize, const char *alias, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, alias );
    rc = KDirectoryVResolveAlias ( self, absolute,
        resolved, rsize, alias, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVResolveAlias ( const KDirectory_v1 *self, bool absolute,
    char *resolved, size_t rsize, const char *alias, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcResolving, rcSelf, rcNull );

    /* allow NULL buffers of 0 size */
    if ( resolved == NULL && rsize != 0 )
        return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcNull );

    if ( alias == NULL )
        return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcNull );
    if ( alias [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . resolve_alias )
            ( self, absolute, resolved, rsize, alias, args );
    }

    return RC ( rcFS, rcDirectory, rcResolving, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryRename_v1 ( KDirectory_v1 *self, bool force, const char *from, const char *to )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcRenaming, rcSelf, rcNull );

    if ( from == NULL || to == NULL )
        return RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcNull );
    if ( from [ 0 ] == 0 || to [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcRenaming, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . rename ) ( self, force, from, to );
    }

    return RC ( rcFS, rcDirectory, rcRenaming, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryRemove_v1 ( KDirectory_v1 *self, bool force, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVRemove ( self, force, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVRemove ( KDirectory_v1 *self, bool force, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcRemoving, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . remove ) ( self, force, path, args );
    }

    return RC ( rcFS, rcDirectory, rcRemoving, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryClearDir_v1 ( KDirectory_v1 *self, bool force, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVClearDir ( self, force, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVClearDir ( KDirectory_v1 *self, bool force, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcRemoving, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . clear_dir ) ( self, force, path, args );
    }

    return RC ( rcFS, rcDirectory, rcRemoving, rcInterface, rcBadVersion );
}

/* Access
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT rc_t CC KDirectoryAccess_v1 ( const KDirectory_v1 *self,
    uint32_t *access, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVAccess ( self, access, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVAccess ( const KDirectory_v1 *self,
    uint32_t *access, const char *path, va_list args )
{
    if ( access == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * access = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . access ) ( self, access, path, args );
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectorySetAccess_v1 ( KDirectory_v1 *self, bool recurse,
    uint32_t access, uint32_t mask, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVSetAccess ( self, recurse,
        access, mask, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVSetAccess ( KDirectory_v1 *self, bool recurse,
    uint32_t access, uint32_t mask, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        /* TBD - perhaps limit bits to lower 9 */
        return ( * self -> vt -> v1 . set_access )
            ( self, recurse, access, mask, path, args );
    }

    return RC ( rcFS, rcDirectory, rcUpdating, rcInterface, rcBadVersion );
}

/* Date
 *  get date/time to object
 *
 *  "date" [ OUT ] - return parameter
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT rc_t CC KDirectoryDate_v1 ( const KDirectory_v1 *self,
    KTime_t *date, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVDate ( self, date, path, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDirectoryVDate ( const KDirectory_v1 *self,
    KTime_t *date, const char *path, va_list args )
{
    if ( date == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * date = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 1 )
            return ( * self -> vt -> v1 . date ) ( self, date, path, args );
        break;
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectorySetDate_v1 ( KDirectory_v1 *self, bool recurse,
    KTime_t date, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVSetDate ( self, recurse,
			      date, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVSetDate ( KDirectory_v1 *self, bool recurse,
    KTime_t date, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        /* TBD - perhaps limit bits to lower 9 */
        if ( self -> vt -> v1 . min >= 1 )
            return ( * self -> vt -> v1 . setdate ) ( self, recurse, date, path, args );
        break;
    }

    return RC ( rcFS, rcDirectory, rcUpdating, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryCreateAlias_v1 ( KDirectory_v1 *self,
    uint32_t access, KCreateMode mode,
    const char *targ, const char *alias )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcSelf, rcNull );

    if ( targ == NULL || alias == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNull );
    if ( targ [ 0 ] == 0 || alias [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . create_alias )
            ( self, access, mode, targ, alias );
    }

    return RC ( rcFS, rcDirectory, rcUpdating, rcInterface, rcBadVersion );
}

/* OpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryOpenFileRead_v1 ( const KDirectory_v1 *self,
    struct KFile const **f, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenFileRead ( self, f, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVOpenFileRead ( const KDirectory_v1 *self,
    struct KFile const **f, const char *path, va_list args )
{
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcFile, rcNull );

    * f = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . open_file_read ) ( self, f, path, args );
    }

    return RC ( rcFS, rcDirectory, rcOpening, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryOpenFileWrite_v1 ( KDirectory_v1 *self,
    struct KFile **f, bool update, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenFileWrite ( self, f, update, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVOpenFileWrite ( KDirectory_v1 *self,
    struct KFile **f, bool update, const char *path, va_list args )
{
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcFile, rcNull );

    * f = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );


    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . open_file_write )
            ( self, f, update, path, args );
    }

    return RC ( rcFS, rcDirectory, rcOpening, rcInterface, rcBadVersion );
}

/* OpenFileSharedWrite ( v1.4 )
 *  opens an existing file with shared write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryOpenFileSharedWrite_v1 ( KDirectory_v1 *self,
    struct KFile **f, bool update, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenFileSharedWrite ( self, f, update, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVOpenFileSharedWrite ( KDirectory_v1 *self,
    struct KFile **f, bool update, const char *path, va_list args )
{
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcFile, rcNull );

    * f = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );


    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 4 )
        {
            return ( * self -> vt -> v1 . open_file_shared_write )
                ( self, f, update, path, args );
        }
        break;
    }

    return RC ( rcFS, rcDirectory, rcOpening, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryCreateFile_v1 ( KDirectory_v1 *self, struct KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVCreateFile ( self, f, update,
        access, mode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVCreateFile ( KDirectory_v1 *self, struct KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    if ( f == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcFile, rcNull );

    * f = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );


    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . create_file )
            ( self, f, update, access, mode, path, args );
    }

    return RC ( rcFS, rcDirectory, rcCreating, rcInterface, rcBadVersion );
}

/* FileLocator
 *  returns locator in bytes of target file
 *
 *  "locator" [ OUT ] - return parameter for file locator
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryFileLocator_v1 ( const KDirectory_v1 *self,
    uint64_t *locator, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVFileLocator ( self, locator, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVFileLocator ( const KDirectory_v1 *self,
    uint64_t *locator, const char *path, va_list args )
{
    if ( locator == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * locator = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 2 )
            return ( * self -> vt -> v1 . file_locator ) ( self, locator, path, args );
        break;
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
}

/* FileSize
 *  returns size in bytes of target file
 *
 *  "size" [ OUT ] - return parameter for file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryFileSize_v1 ( const KDirectory_v1 *self,
    uint64_t *size, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVFileSize ( self, size, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVFileSize ( const KDirectory_v1 *self,
    uint64_t *size, const char *path, va_list args )
{
    if ( size == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * size = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . file_size ) ( self, size, path, args );
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
}

/* FilePhysicalSize
 *  returns size in bytes of target file
 *
 *  "size" [ OUT ] - return parameter for file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryFilePhysicalSize_v1 ( const KDirectory_v1 *self,
    uint64_t *size, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVFilePhysicalSize ( self, size, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVFilePhysicalSize ( const KDirectory_v1 *self,
    uint64_t *size, const char *path, va_list args )
{
    if ( size == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * size = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 3 )
            return ( * self -> vt -> v1 . file_phys_size ) ( self, size, path, args );
        break;
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
}

/* SetFileSize
 *  sets size in bytes of target file
 *
 *  "size" [ IN ] - new file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectorySetFileSize_v1 ( KDirectory_v1 *self,
    uint64_t size, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVSetFileSize ( self, size, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVSetFileSize ( KDirectory_v1 *self,
    uint64_t size, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . set_size ) ( self, size, path, args );
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryFileContiguous_v1 ( const KDirectory_v1 *self,
    bool *contiguous, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVFileContiguous ( self, contiguous, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVFileContiguous ( const KDirectory_v1 *self,
    bool *contiguous, const char *path, va_list args )
{
    if ( contiguous == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * contiguous = 0;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        if ( self -> vt -> v1 . min >= 3 )
            return ( * self -> vt -> v1 . file_contiguous ) ( self, contiguous, path, args );
        break;
    }

    return RC ( rcFS, rcDirectory, rcAccessing, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryOpenDirRead_v1 ( const KDirectory_v1 *self,
    const KDirectory_v1 **sub, bool chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenDirRead ( self, sub, chroot, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVOpenDirRead ( const KDirectory_v1 *self,
    const KDirectory_v1 **sub, bool chroot, const char *path, va_list args )
{
    if ( sub == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcNull );

    * sub = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );

    if ( path == NULL || path [ 0 ] == 0 )
        path = ".";

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . open_dir_read )
            ( self, sub, chroot, path, args );
    }

    return RC ( rcFS, rcDirectory, rcOpening, rcInterface, rcBadVersion );
}

LIB_EXPORT rc_t CC KDirectoryOpenDirUpdate_v1 ( KDirectory_v1 *self,
    KDirectory_v1 **sub, bool chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenDirUpdate ( self, sub, chroot, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVOpenDirUpdate ( KDirectory_v1 *self,
    KDirectory_v1 **sub, bool chroot, const char *path, va_list args )
{
    if ( sub == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcNull );

    * sub = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        /* TODO: Maybe for writing wncbi-vdb we need to return RC */
        return SILENT_RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcReadonly );

    if ( path == NULL || path [ 0 ] == 0 )
        path = ".";

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . open_dir_update )
            ( self, sub, chroot, path, args );
    }

    return RC ( rcFS, rcDirectory, rcOpening, rcInterface, rcBadVersion );
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
LIB_EXPORT rc_t CC KDirectoryCreateDir_v1 ( KDirectory_v1 *self,
    uint32_t access, KCreateMode mode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVCreateDir ( self, access, mode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryVCreateDir ( KDirectory_v1 *self,
    uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcSelf, rcNull );

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );

    if ( self -> read_only )
        return RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcReadonly );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . create_dir ) ( self, access, mode, path, args );
    }

    return RC ( rcFS, rcDirectory, rcCreating, rcInterface, rcBadVersion );
}

/* Init
 *  initialize a newly allocated directory object
 */
LIB_EXPORT rc_t CC KDirectoryInit_v1 ( KDirectory_v1 *self, const KDirectory_vt *vt,
    const char * class_name, const char * path, bool update )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcConstructing, rcSelf, rcNull );
    if ( vt == NULL )
        return RC ( rcFS, rcDirectory, rcConstructing, rcInterface, rcNull );
    switch ( vt -> v1 . maj )
    {
    case 0:
        return RC ( rcFS, rcDirectory, rcConstructing, rcInterface, rcInvalid );
    case 1:
        switch ( vt -> v1 . min )
        {
            /* ADD NEW MINOR VERSION CASES HERE */
        case 4:
#if _DEBUGGING
            if ( vt -> v1 . open_file_shared_write == NULL )
                return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            /* no break */
        case 3:
#if _DEBUGGING
            if ( vt -> v1 . file_phys_size == NULL  ||
                 vt -> v1 . file_contiguous == NULL )
                return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            /* no break */
        case 2:
#if _DEBUGGING
            if ( vt -> v1 . file_locator == NULL )
                return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            /* no break */
        case 1:
#if _DEBUGGING
            if ( vt -> v1 . get_sysdir == NULL      ||
                 vt -> v1 . setdate == NULL         ||
                 vt -> v1 . date == NULL )
                return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            /* no break */
        case 0:
#if _DEBUGGING
            /* vt -> v1 . destroy_file CAN be NULL */
            if ( vt -> v1 . create_dir == NULL      ||
                 vt -> v1 . open_dir_update == NULL ||
                 vt -> v1 . open_dir_read == NULL   ||
                 vt -> v1 . set_size == NULL        ||
                 vt -> v1 . file_size == NULL       ||
                 vt -> v1 . create_file == NULL     ||
                 vt -> v1 . open_file_write == NULL ||
                 vt -> v1 . open_file_read == NULL  ||
                 vt -> v1 . create_alias == NULL    ||
                 vt -> v1 . set_access == NULL      ||
                 vt -> v1 . access == NULL          ||
                 vt -> v1 . clear_dir == NULL       ||
                 vt -> v1 . remove == NULL          ||
                 vt -> v1 . rename == NULL          ||
                 vt -> v1 . resolve_alias == NULL   ||
                 vt -> v1 . resolve_path == NULL    ||
                 vt -> v1 . path_type == NULL       ||
                 vt -> v1 . visit_update == NULL    ||
                 vt -> v1 . visit == NULL           ||
                 vt -> v1 . list_dir == NULL        ||
                 vt -> v1 . destroy == NULL )
                return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcFS, rcDirectory, rcConstructing, rcInterface, rcBadVersion );
        }
        break;
    default:
        return RC ( rcFS, rcDirectory, rcConstructing, rcInterface, rcBadVersion );
    }

    self -> vt = vt;
    self -> read_only = ( uint8_t ) ( update == 0 );


    KRefcountInit ( &self->refcount, 1, class_name, "init", path );

    return 0;
}

/* DestroyFile
 *  does whatever is necessary with an unreferenced file
 */
LIB_EXPORT rc_t CC KDirectoryDestroyFile_v1 ( const KDirectory_v1 *self, KFile *f )
{
    const KDirectory_vt *vt = self -> vt;
    switch ( vt -> v1 . maj )
    {
    case 1:
        if ( vt -> v1 . destroy_file != NULL )
            return ( * vt -> v1 . destroy_file ) ( ( KDirectory_v1* ) self, f );
        return KFileDestroy_v1 ( f );
    }

    return RC ( rcFS, rcDirectory, rcDestroying, rcInterface, rcBadVersion );
}

/* GetSysDir
 *  returns an underlying system file object
 */
LIB_EXPORT struct KSysDir_v1* CC KDirectoryGetSysDir_v1 ( const KDirectory_v1 *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            if ( self -> vt -> v1 . min > 0 )
                return ( * self -> vt -> v1 . get_sysdir ) ( self );
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


LIB_EXPORT rc_t CC KDirectoryCopyPath ( const KDirectory_v1 *src_dir,
    KDirectory_v1 *dst_dir, const char *src_path, const char * dst_path )
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


static rc_t build_obj_path( char **s, const char *path, const char * objname )
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


LIB_EXPORT rc_t CC KDirectoryCopyPaths( const KDirectory_v1 * src_dir,
    KDirectory_v1 * dst_dir, bool recursive, const char *src, const char *dst )
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


LIB_EXPORT rc_t CC KDirectoryCopy( const KDirectory_v1 * src_dir,
    KDirectory_v1 * dst_dir, bool recursive, const char *src, const char *dst )
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
