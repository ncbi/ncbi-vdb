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

#ifndef _hpp_kfs_directory_
#define _hpp_kfs_directory_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

/*--------------------------------------------------------------------------
 * KDirectory
 *  a container node within file system
 */
struct KDirectory {
    /* AddRef
     * Release
     *  ignores NULL references
     */
    inline rc_t AddRef ( void ) const throw ()
    { return KDirectoryAddRef ( this ); }
    inline rc_t Release ( void ) const throw ()
    { return KDirectoryRelease ( this ); }


    /* List
     *  create a directory listing
     *
     *  "list" [ OUT ] - return parameter for list object
     *
     *  "f" [ IN, NULL OKAY ] and "data" [ IN, OPAQUE ] - optional
     *  filter function to execute on each path. receives a base directory
     *  and relative path for each entry. if "f" returns true, the name will
     *  be added to the list.
     *
     *  "path" [ IN, NULL OKAY ] - optional parameter for target
     *  directory. if NULL, interpreted to mean "."
     */
    inline rc_t List ( struct KNamelist **list,
        bool ( CC * f ) ( const KDirectory *dir, const char *name, void *data ),
        void *data,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVList ( this, list, f, data, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t List ( struct KNamelist **list,
        bool ( CC * f ) ( const KDirectory *dir, const char *name, void *data ),
        void *data,
        const char *path, va_list args ) const throw ()
    { return KDirectoryVList ( this, list, f, data, path, args ); }


    /* Visit
     *  visit each path under designated directory
     *
     *  "recurse" [ IN ] - if true, recursively visit sub-directories
     *
     *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to execute
     *  on each path. receives a base directory and relative path
     *  for each entry. if "f" returns true, the iteration will
     *  terminate and that value will be returned. NB - "dir" will not
     *  be the same as "self".
     *
     *  "path" [ IN ] - NUL terminated string in directory-native character set
     */
    inline rc_t Visit ( bool recurse,
        rc_t ( CC * f ) ( const KDirectory *dir, uint32_t type, const char *name, void *data ),
        void *data,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVVisit ( this, recurse, f, data, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Visit ( bool recurse,
        rc_t ( CC * f ) ( const KDirectory *dir, uint32_t type, const char *name, void *data ),
        void *data,
        const char *path, va_list args ) const throw ()
    { return KDirectoryVVisit ( this, recurse, f, data, path, args ); }


    /* VisitUpdate
     *  like Visit except that the directory passed back to "f"
     *  is available for update operations
     */
    inline rc_t VisitUpdate ( bool recurse,
        rc_t ( CC * f ) ( KDirectory *dir, uint32_t type, const char *name, void *data ),
        void *data,
        const char *path, ... ) throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVVisitUpdate ( this, recurse, f, data, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t VisitUpdate ( bool recurse,
        rc_t ( CC * f ) ( KDirectory *dir, uint32_t type, const char *name, void *data ),
        void *data,
        const char *path, va_list args ) throw ()
    { return KDirectoryVVisitUpdate ( this, recurse, f, data, path, args ); }


    /* PathType
     *  returns a KPathType
     *
     *  "path" [ IN ] - NUL terminated string in directory-native character set
     */
    inline uint32_t PathType ( const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        uint32_t r = KDirectoryVPathType ( this, path, args );
        va_end ( args );
        return r;
    }
    inline uint32_t PathType ( const char *path, va_list args ) const throw ()
    { return KDirectoryVPathType ( this, path, args ); }


    /* ResolvePath
     *  resolves path to an absolute or directory-relative path
     *
     *  "absolute" [ IN ] - if true, always give a path starting
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
    inline rc_t ResolvePath ( bool absolute, char *resolved, size_t rsize,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVResolvePath ( this, absolute, resolved, rsize, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t ResolvePath ( bool absolute, char *resolved, size_t rsize,
        const char *path, va_list args ) const throw ()
    { return KDirectoryVResolvePath ( this, absolute, resolved, rsize, path, args ); }


    /* ResolveAlias
     *  resolves an alias path to its immediate target
     *  NB - the resolved path may be yet another alias
     *
     *  "absolute" [ IN ] - if true, always give a path starting
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
     inline rc_t ResolveAlias ( bool absolute, char *resolved, size_t rsize, 
        const char *alias, ... ) const throw ()
     {
        va_list args;
        va_start ( args, alias );
        rc_t rc = KDirectoryVResolveAlias ( this, absolute, resolved, rsize, alias, args );
        va_end ( args );
        return rc;
     }
     inline rc_t ResolveAlias ( bool absolute, char *resolved, size_t rsize, 
        const char *alias, va_list args ) const throw ()
     { return KDirectoryVResolveAlias ( this, absolute, resolved, rsize, alias, args ); }


     /* Rename
     *  rename an object accessible from directory, replacing
     *  any existing target object of the same type
     *
     *  "from" [ IN ] - NUL terminated string in directory-native
     *  character set denoting existing object
     *
     *  "to" [ IN ] - NUL terminated string in directory-native
     *  character set denoting existing object
     *
     *  "force" [ IN ] - not false means try to do more if it fails internally
     */
     inline rc_t Rename ( bool force, const char *from, const char *to ) throw ()
     {  return KDirectoryRename ( this, force, from, to );  }


     /* Remove
     *  remove an accessible object from its directory
     *
     *  "force" [ IN ] - if true and target is a directory,
     *  remove recursively
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target object
     */
     inline rc_t Remove ( bool force, const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVRemove ( this, force, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t Remove ( bool force, const char *path, va_list args ) throw ()
     { return KDirectoryVRemove ( this, force, path, args ); }


     /* ClearDir
     *  remove all directory contents
     *
     *  "force" [ IN ] - if true and directory entry is a
     *  sub-directory, remove recursively
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target directory
     */
     inline rc_t ClearDir ( bool force, const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVClearDir ( this, force, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t ClearDir ( bool force, const char *path,
        va_list args ) throw ()
     { return KDirectoryVClearDir ( this, force, path, args ); }


     /* Access
     *  get access to object
     *
     *  "access" [ OUT ] - return parameter for Unix access mode
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target object
     */
     inline rc_t Access ( uint32_t *access,
        const char *path, ... ) const throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVAccess ( this, access, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t Access ( uint32_t *access, const char *path, 
        va_list args ) const throw ()
     { return KDirectoryVAccess ( this, access, path, args ); }


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
     inline rc_t SetAccess ( bool recurse, uint32_t access, uint32_t mask,
        const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVSetAccess ( this, recurse, access, mask, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t SetAccess ( bool recurse, uint32_t access, uint32_t mask,
        const char *path, va_list args ) throw ()
     { return KDirectoryVSetAccess ( this, recurse, access, mask, path, args ); }


     /* Date
     *  get date/time to object
     *
     *  "date" [ OUT ] - return parameter
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target object
     */
     inline rc_t Date ( KTime_t *date, const char *path, ... ) const throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVDate ( this, date, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t Date ( KTime_t *date, const char *path,
        va_list args ) const throw ()
     { return KDirectoryVDate ( this, date, path, args ); }


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
     inline rc_t SetDate ( bool recurse, KTime_t date,
        const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVSetDate ( this, recurse, date, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t SetDate ( bool recurse, KTime_t date, const char *path,
        va_list args ) throw ()
     { return KDirectoryVSetDate ( this, recurse, date, path, args ); }


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
     inline rc_t CreateAlias ( uint32_t access, KCreateMode mode,
        const char *targ, const char *alias ) throw ()
     { return KDirectoryCreateAlias ( this, access, mode, targ, alias ); }


    /* OpenFileRead
     *  opens an existing file with read-only access
     *
     *  "f" [ OUT ] - return parameter for newly opened file
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target file
     */
    inline rc_t OpenFileRead ( struct KFile const **f,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVOpenFileRead ( this, f, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenFileRead ( struct KFile const **f, const char *path,
        va_list args ) const throw ()
    { return KDirectoryVOpenFileRead ( this, f, path, args ); }


    /* OpenFileWrite
     *  opens an existing file with write access
     *
     *  "f" [ OUT ] - return parameter for newly opened file
     *
     *  "update" [ IN ] - if true, open in read/write mode
     *  otherwise, open in write-only mode
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target file
     */
    inline rc_t OpenFileWrite ( struct KFile **f, bool update,
        const char *path, ... ) throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVOpenFileWrite ( this, f, update, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenFileWrite ( struct KFile **f, bool update,
        const char *path, va_list args ) throw ()
    { return KDirectoryVOpenFileWrite ( this, f, update, path, args ); }


    /* CreateFile
     *  opens a file with write access
     *
     *  "f" [ OUT ] - return parameter for newly opened file
     *
     *  "update" [ IN ] - if true, open in read/write mode
     *  otherwise, open in write-only mode
     *
     *  "access" [ IN ] - standard Unix access mode, e.g. 0664
     *
     *  "mode" [ IN ] - a creation mode ( see explanation above ).
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target file
     */
     inline rc_t CreateFile ( struct KFile **f, bool update, uint32_t access,
        KCreateMode mode, const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVCreateFile ( this, f, update, access, mode, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t CreateFile ( struct KFile **f, bool update, uint32_t access,
        KCreateMode mode, const char *path, va_list args ) throw ()
     { return KDirectoryVCreateFile ( this, f, update, access, mode, path, args ); }


     /* FileSize
     *  returns size in bytes of target file
     *
     *  "size" [ OUT ] - return parameter for file size
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target file
     */
     inline rc_t FileSize ( uint64_t *size, 
        const char *path, ... ) const throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVFileSize ( this, size, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t FileSize ( uint64_t *size, const char *path,
        va_list args ) const throw ()
     { return KDirectoryVFileSize ( this, size, path, args ); }


     /* SetFileSize
     *  sets size in bytes of target file
     *
     *  "size" [ IN ] - new file size
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target file
     */
     inline rc_t SetFileSize ( uint64_t size, const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVSetFileSize ( this, size, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t SetFileSize ( uint64_t size, const char *path,
        va_list args ) throw ()
     { return KDirectoryVSetFileSize ( this, size, path, args ); }


     /* OpenDirRead
     * OpenDirUpdate
     *  opens a sub-directory
     *
     *  "chroot" [ IN ] - if true, the new directory becomes
     *  chroot'd and will interpret paths beginning with '/'
     *  relative to itself.
     *
     *  "path" [ IN ] - NUL terminated string in directory-native
     *  character set denoting target directory
     */
     inline rc_t OpenDirRead ( const KDirectory **sub, bool chroot,
        const char *path, ... ) const throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVOpenDirRead ( this, sub, chroot, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t OpenDirRead ( const KDirectory **sub, bool chroot,
        const char *path, va_list args ) const throw ()
     { return KDirectoryVOpenDirRead ( this, sub, chroot, path, args ); }

     inline rc_t OpenDirUpdate ( KDirectory **sub, bool chroot,
        const char *path, ... )
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVOpenDirUpdate ( this, sub, chroot, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t OpenDirUpdate ( KDirectory **sub, bool chroot,
        const char *path, va_list args ) throw ()
     { return KDirectoryVOpenDirUpdate ( this, sub, chroot, path, args ); }

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
     inline rc_t CreateDir ( uint32_t access, KCreateMode mode,
        const char *path, ... ) throw ()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KDirectoryVCreateDir ( this, access, mode, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t CreateDir ( uint32_t access, KCreateMode mode,
        const char *path, va_list args ) throw ()
     { return  KDirectoryVCreateDir ( this, access, mode, path, args ); }


    /* NativeDir
     *  returns a native file-system directory node reference
     *  the directory root will be "/" and set to the native
     *  idea of current working directory
     *
     *  NB - the returned reference will be non-const, allowing
     *  modification operations to be attempted. these operations
     *  may still fail if the underlying FS disallows them.
     *
     *  "dir" [ OUT ] - return parameter for native directory
     */
    inline static rc_t NativeDir ( KDirectory **dir ) throw ()
    { return KDirectoryNativeDir ( dir ); }

private:
    KDirectory ();
    ~ KDirectory ();
    KDirectory ( const KDirectory& );
    KDirectory &operator = ( const KDirectory& );
};

#endif /* _hpp_kfs_directory_ */
