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
#ifndef _h_vfs_manager_
#define _h_vfs_manager_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif


#ifndef _h_kfs_defs_
#include <kfs/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct VPath;
struct VResolver;
struct KDirectory;
struct KNSManager;

/* temporary */


/*--------------------------------------------------------------------------
 * VFSManager
 */
typedef struct VFSManager VFSManager;

#define VFS_KRYPTO_PASSWORD_MAX_SIZE 4096


/* AddRef
 * Release
 *  ignores NULL references
 */
VFS_EXTERN rc_t CC VFSManagerAddRef ( const VFSManager *self );
VFS_EXTERN rc_t CC VFSManagerRelease ( const VFSManager *self );

/* OpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] VPath representing the path, URL or URN of the desired file
 */
VFS_EXTERN rc_t CC VFSManagerOpenFileRead ( const VFSManager *self, 
                                           struct KFile const **f,
                                           const struct VPath * path );

VFS_EXTERN rc_t CC VFSManagerOpenFileReadWithBlocksize ( const VFSManager *self, 
                                           struct KFile const **f,
                                           const struct VPath * path,
                                           uint32_t blocksize,
                                           bool promote );

VFS_EXTERN rc_t CC VFSManagerOpenDirectoryRead ( const VFSManager *self,
    struct KDirectory const **d, const struct VPath * path );

/* it forces decryption to be used for kdb */
VFS_EXTERN rc_t CC VFSManagerOpenDirectoryReadDecrypt ( const VFSManager *self,
    struct KDirectory const **d, const struct VPath * path );

/* VPath has already been resolved to a URL and an optional cache file */
VFS_EXTERN rc_t CC VFSManagerOpenDirectoryReadDecryptRemote (const struct VFSManager *self,
    struct KDirectory const **d, const struct VPath * path, const struct VPath * cache);

VFS_EXTERN rc_t CC VFSManagerOpenDirectoryUpdate ( const VFSManager *self,
    struct KDirectory **d, const struct VPath * path );

/* OpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "path" [ IN ] VPath representing the path, URL or URN of the desired file
 */
VFS_EXTERN rc_t CC VFSManagerOpenFileWrite (const VFSManager *self,
                                            struct KFile **f,
                                            bool update,
                                            const struct VPath * path);

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
 *  "path" [ IN ] VPath representing the path, URL or URN of the desired file
 */
VFS_EXTERN rc_t CC VFSManagerCreateFile (const VFSManager *self, 
                                         struct KFile **f,
                                         bool update, uint32_t access,
                                         KCreateMode mode,
                                         const struct VPath * path );


/* Remove
 *  remove an accessible object from its directory
 *
 *  "force" [ IN ] - if true and target is a directory,
 *  remove recursively
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
VFS_EXTERN rc_t CC VFSManagerRemove (const VFSManager *self, bool force,
                                     const struct VPath * path );


/* Make
 */
VFS_EXTERN rc_t CC VFSManagerMake ( VFSManager ** pmanager );

/* GetCWD
 */
VFS_EXTERN rc_t CC VFSManagerGetCWD (const VFSManager * self, struct KDirectory ** cwd);

VFS_EXTERN rc_t CC VFSManagerGetResolver ( const VFSManager * self, struct VResolver ** resolver );

VFS_EXTERN rc_t CC VFSManagerGetKNSMgr ( const VFSManager * self, struct KNSManager ** kns );


VFS_EXTERN rc_t CC VFSManagerGetKryptoPassword (const VFSManager * self, char * new_password, size_t max_size, size_t * size);

/*
  NULL value for self
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcSelf, rcNull);

  NULL value for password or 0 value for size
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcParam, rcNull);

  size greater than VFS_KRYPTO_PASSWORD_MAX_SIZE
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcSize, rcExcessive);

  illegal CR or LF (NL) in the password
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcEncryptionKey, rcInvalid);

  path/file name for password too long for function as written
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcExcessive);

  existing password path/file name is not a file
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcIncorrect);

  unknown file type for configured path/file name
  RC (rcVFS, rcEncryptionKey, rcUpdating, rcPath, rcCorrupt);

  incomplete writes to temporary password file
  RC (rcVFS, rcEncryptionKey, rcWriting, rcFile, rcInsufficient);

  other errors from KFS and KLIB
*/

/* pwd_dir[pwd_dir_size] -
    Output parameters for path to directory containing the password file.
    pwd_dir is not updated if pwd_dir == NULL or pwd_dir_size == 0
                  or pwd_dir_size is insufficient to copy the path
 */
VFS_EXTERN rc_t CC VFSManagerUpdateKryptoPassword (const VFSManager * self, 
                                                   const char * password,
                                                   size_t size,
                                                   char * pwd_dir,
                                                   size_t pwd_dir_size);


VFS_EXTERN rc_t CC VFSManagerResolveSpec ( const VFSManager * self,
                                           const char * spec,
                                           struct VPath ** path_to_build,
                                           const struct KFile ** remote_file,
                                           const struct VPath ** local_cache,
                                           bool resolve_acc );

struct KDirectory;

VFS_EXTERN rc_t CC VFSManagerResolveSpecIntoDir ( const VFSManager * self,
                                                  const char * spec,
                                                  const struct KDirectory ** dir,
                                                  bool resolve_acc );

/*--------------------------------------------------------------------------
 * KConfig
 *  placing some KConfig code that relies upon VFS here
 */
struct KConfig;
struct KConfigNode;

/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
VFS_EXTERN rc_t CC KConfigReadVPath ( struct KConfig const* self, const char* path, struct VPath** result );

/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
VFS_EXTERN rc_t CC KConfigNodeReadVPath ( struct KConfigNode const *self, struct VPath** result );

/* ResolvePath
 *
 * take a VPath and resolve to a final form apropriate for KDB
 *
 * that is take a relative path and resolve it against the CWD
 * or take an accession and resolve into the local or remote 
 * VResolver file based on config. It is just a single resolution percall
 *
 * flags
 *      can disable all Accession resolution
 *      can let VPath With no scheme be treated as a possible accession
 *
 */

/* bit values for flags */
    /* allow no local accession resolution */
#define vfsmgr_rflag_no_acc_local (1<<0)
    /* allow no remote accession resolution */
#define vfsmgr_rflag_no_acc_remote (1<<1)
    /* never do VResolver Accession resolution */
#define vfsmgr_rflag_no_acc  (vfsmgr_rflag_no_acc_local|vfsmgr_rflag_no_acc_remote)
    /* use VResolver Accession resolution for simple names with no scheme */

#define vfsmgr_rflag_kdb_acc (1<<2)
    /* over ridden by vfsmgr_rflag_no_acc */


VFS_EXTERN rc_t CC VFSManagerResolvePath (const VFSManager * self,
                                          uint32_t flags,
                                          const struct  VPath * in_path,
                                          struct VPath ** out_path);

VFS_EXTERN rc_t CC VFSManagerResolvePathRelative (const VFSManager * self,
                                                  uint32_t flags,
                                                  const struct  VPath * base_path,
                                                  const struct  VPath * in_path,
                                                  struct VPath ** out_path);

/*
 * Registering bindings between dbGaP object Ids and object names
 */                                                  

/* VFSManagerRegisterObject
 *  registers a binding between an object Id and an object name (object = accession or dbGaP file) 
 * 
 * self [ IN ] - VFSManager object
 * oid [ IN ] - object id
 * obj [ IN ] - Vpath representing the object's name (scheme is "ncbi-acc" for accessions, "ncbi-file" for dbGaP files; 
 *              the name itself is the 'path' component
 */
VFS_EXTERN rc_t CC VFSManagerRegisterObject(struct VFSManager* self, uint32_t oid, const struct VPath* obj);

/* VFSManagerGetObjectId
 *  look up an object id by an object name 
 * 
 * self [ IN ] - VFSManager object
 * obj [ IN ] - Vpath representing the object's name (scheme is "ncbi-acc" for accessions, "ncbi-file" for dbGaP files; 
 *              the name itself is the 'path' component
 * oid [ OUT ] - object id
 */
VFS_EXTERN rc_t CC VFSManagerGetObjectId(const struct VFSManager* self, const struct VPath* obj, uint32_t* oid);

/* VFSManagerGetObject
 *  look up an object name by an object id
 * 
 * self [ IN ] - VFSManager object
 * oid [ IN ] - object id
 * obj [ OUT ] - Vpath representing the object's name (scheme is "ncbi-acc" for accessions, "ncbi-file" for dbGaP files; 
 *              the name itself is the 'path' component
 */
VFS_EXTERN rc_t CC VFSManagerGetObject(const struct VFSManager* self, uint32_t oid, struct VPath** obj);

/* SetAdCaching
 *  Enable Caching to Accession as Directory in cwd
 */
VFS_EXTERN
rc_t CC VFSManagerSetAdCaching(struct VFSManager* self, bool enabled);
                                                  
#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_manager_ */
