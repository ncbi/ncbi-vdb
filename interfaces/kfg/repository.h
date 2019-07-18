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

#ifndef _h_kfg_repository_
#define _h_kfg_repository_


#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif
 
#ifndef _h_klib_vector_
#include <klib/vector.h> /* Vector */
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KConfig;
struct KNgcObj;

/*--------------------------------------------------------------------------
 * KRepository
 *  presents structured access to a storage repository
 *  as modeled in KConfig.
 *
 *  all objects are obtained via KRepositoryMgr ( see below )
 */
typedef struct KRepository KRepository;


/* AddRef
 * Release
 */
KFG_EXTERN rc_t CC KRepositoryAddRef ( const KRepository *self );
KFG_EXTERN rc_t CC KRepositoryRelease ( const KRepository *self );


/* Category
 * SubCategory
 *  tells what the repository category or sub-category are
 *  or returns "bad" if the repository object is not usable.
 */
typedef uint32_t KRepCategory;
enum
{
    krepBadCategory,
    krepUserCategory,
    krepSiteCategory,
    krepRemoteCategory
};

typedef uint32_t KRepSubCategory;
enum
{
    krepBadSubCategory,
    krepMainSubCategory,
    krepAuxSubCategory,
    krepProtectedSubCategory
};

KFG_EXTERN KRepCategory CC KRepositoryCategory ( const KRepository *self );
KFG_EXTERN KRepSubCategory CC KRepositorySubCategory ( const KRepository *self );


/* Name
 *  get the repository name
 *  attempts to copy NUL-terminated name into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - name output parameter
 *
 *  "name_size" [ OUT, NULL OKAY ] - returns the name size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryName ( const KRepository *self,
    char *buffer, size_t bsize, size_t *name_size );


/* DisplayName
 *  get the repository display name,
 *  if different from its actual name
 *
 *  attempts to copy NUL-terminated name into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - name output parameter
 *
 *  "name_size" [ OUT, NULL OKAY ] - returns the name size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryDisplayName ( const KRepository *self,
    char *buffer, size_t bsize, size_t *name_size );


/* Root
 *  read the root path as a POSIX path or URL
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "root_size" [ OUT, NULL OKAY ] - returns the path size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryRoot ( const KRepository *self,
    char *buffer, size_t bsize, size_t *root_size );


/* SetRoot
 *  set the root path
 *
 *  "root" [ IN ] and "root_size" [ IN ] - path input parameter
 */
KFG_EXTERN rc_t CC KRepositorySetRoot( KRepository *self,
    const char *root, size_t root_size );


/* RootHistory
 *  read the root-history as a semicolon separated list of POSIX paths
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "roothistory_size" [ OUT, NULL OKAY ] - returns the root-history
 *  size in bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryRootHistory ( const KRepository *self,
    char *buffer, size_t bsize, size_t *roothistory_size );


/* SetRootHistory
 *  set the root-history list of paths
 *
 *  "roothistory" [ IN ] and "roothistory_size" [ IN ] - path input parameter
 */
KFG_EXTERN rc_t CC KRepositorySetRootHistory( KRepository *self,
    const char *roothistory, size_t roothistory_size  );


/* AppendToRootHistory
 *  append to the root-history
 *
 *  "roothistory" [ IN ] and "roothistory_size" [ IN ] - path input parameter
 *  if item == NULL, add the current root to the root-history
 */
KFG_EXTERN rc_t CC KRepositoryAppendToRootHistory( KRepository *self,
    const char *item  );


/* Resolver
 *  read the url of the CGI-resolver
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "written" [ OUT, NULL OKAY ] - returns the url size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryResolver ( const KRepository *self,
    char *buffer, size_t bsize, size_t *written );


/* Disabled
 *  discover whether the repository is enabled
 */
KFG_EXTERN bool CC KRepositoryDisabled ( const KRepository *self );


/* Set Disabled
 *  changes the status of a repository, writes status into kfg-file on disk
 *  disabled = true  ... disables the repository
 *  disabled = false ... enables the repository
 */
KFG_EXTERN rc_t CC KRepositorySetDisabled ( const KRepository *self, bool disabled );


/* CacheEnabled
 *  discover whether the repository supports caching
 */
KFG_EXTERN bool CC KRepositoryCacheEnabled ( const KRepository *self );


/* DownloadTicket
 *  return any associated download ticket
 *
 *  attempts to copy NUL-terminated ticket into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - ticket output parameter
 *
 *  "ticket_size" [ OUT, NULL OKAY ] - returns the ticket size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryDownloadTicket ( const KRepository *self,
    char *buffer, size_t bsize, size_t *ticket_size );


/* EncryptionKey
 *  return any associated encryption key
 *
 *  attempts to copy NUL-terminated key into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - encryption key output parameter
 *
 *  "key_size" [ OUT, NULL OKAY ] - returns the key size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryEncryptionKey ( const KRepository *self,
    char *buffer, size_t bsize, size_t *key_size );


/* EncryptionKeyFile
 *  return path to any associated encryption key file
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - key file path output parameter
 *
 *  "path_size" [ OUT, NULL OKAY ] - returns the path size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryEncryptionKeyFile ( const KRepository *self,
    char *buffer, size_t bsize, size_t *path_size );


/* Description
 *  return any associated descriptive text
 *
 *  attempts to copy NUL-terminated description into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - description text output parameter
 *
 *  "desc_size" [ OUT, NULL OKAY ] - returns the text size in
 *  bytes, excluding any NUL termination.
 */
KFG_EXTERN rc_t CC KRepositoryDescription ( const KRepository *self,
    char *buffer, size_t bsize, size_t *desc_size );


/* ProjectId
 *  return project id for protected user repository
 *  return RC when repository is not user protected
 *
 *  "projectId" [ OUT ] - returns the project id
 */
KFG_EXTERN rc_t CC KRepositoryProjectId
    ( const KRepository * self, uint32_t * projectId );


/* Description
 *  register an encrypted object in association with the repository's encryption key
 */
KFG_EXTERN rc_t CC KRepositoryRegisterObject ( const KRepository *self, const char* object_id );


/*--------------------------------------------------------------------------
 * KRepositoryVector
 *  uses Vector API
 *  holds zero or more KRepository objects
 */
typedef struct Vector KRepositoryVector;


/* Whack
 *  destroy your vector
 */
KFG_EXTERN rc_t CC KRepositoryVectorWhack ( KRepositoryVector *self );


/*--------------------------------------------------------------------------
 * KRepositoryMgr
 *  manages structured access to repositories
 */
typedef struct KRepositoryMgr KRepositoryMgr;

typedef uint32_t RepositorySelect;
enum
{
    user = 1,
    site = 2,
    remote = 4,
};

/* Make
 *  create a repository manager
 *  uses values from "self"
 *
 *  mgr [ OUT ] 
 */
KFG_EXTERN rc_t CC KConfigMakeRepositoryMgrRead ( struct KConfig const *self, const KRepositoryMgr **mgr );
KFG_EXTERN rc_t CC KConfigMakeRepositoryMgrUpdate ( struct KConfig *self, KRepositoryMgr **mgr );


/* AddRef
 * Release
 */
KFG_EXTERN rc_t CC KRepositoryMgrAddRef ( const KRepositoryMgr *self );
KFG_EXTERN rc_t CC KRepositoryMgrRelease ( const KRepositoryMgr *self );


/* UserRepositories
 *  retrieve all user repositories in a Vector
 */
KFG_EXTERN rc_t CC KRepositoryMgrUserRepositories ( const KRepositoryMgr *self,
    KRepositoryVector *user_repositories );


/* SiteRepositories
 *  retrieve all site repositories in a Vector
 */
KFG_EXTERN rc_t CC KRepositoryMgrSiteRepositories ( const KRepositoryMgr *self,
    KRepositoryVector *site_repositories );


/* RemoteRepositories
 *  retrieve all remote repositories in a Vector
 */
KFG_EXTERN rc_t CC KRepositoryMgrRemoteRepositories ( const KRepositoryMgr *self,
    KRepositoryVector *remote_repositories );

/* CurrentProtectedRepository
 *  returns the currently active user protected repository
 */
KFG_EXTERN rc_t CC KRepositoryMgrCurrentProtectedRepository ( const KRepositoryMgr *self,
    const KRepository **p_protected );

/* GetProtectedRepository
 *  retrieves a (read-only) user protected repository by its associated project-id
 */
KFG_EXTERN rc_t CC KRepositoryMgrGetProtectedRepository ( const KRepositoryMgr *self, 
    uint32_t projectId, 
    const KRepository **p_protected );


/* Getter/Setter for the global disabled-node on the main-category
 */
KFG_EXTERN bool CC KRepositoryMgrCategoryDisabled ( const KRepositoryMgr *self, KRepCategory category );
KFG_EXTERN rc_t CC KRepositoryMgrCategorySetDisabled ( const KRepositoryMgr *self, KRepCategory category, bool disabled );


/* ImportNgc
 *  import ngc file into current configuration
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the ngc file
 * pathToProtectedRepository [ IN, NULL OKAY ] - optional - the
 *  special value NULL is interpreted as ${HOME}/ncbi/dbGaP-$(Project #)
 * newRepoParentPath [ OUT, NULL OKAY ] - optional - 
 *  path to the new protected repository: should not be released!
 */
KFG_EXTERN rc_t CC KConfigImportNgc( struct KConfig * self,
    const char *ngcPath, const char *pathToProtectedRepository,
    const char **newRepoParentPath);


enum
{
    /* on input, these bits represent authority to perform operation.
       on success, they represent operations performed.
       on failure, they represent reason for failure. */

    INP_CREATE_REPOSITORY  = ( 1 << 0 ),   /* permission to create a new entry         */
    INP_UPDATE_DNLD_TICKET = ( 1 << 1 ),   /* permission to modifify a download ticket */
    INP_UPDATE_ENC_KEY     = ( 1 << 2 ),   /* permission to modify an encryption key   */
    INP_UPDATE_DESC        = ( 1 << 3 ),   /* permission to modify a description       */
    INP_UPDATE_ROOT        = ( 1 << 4 ),   /* permission to modify root path */

    INP_UPDATE_APPS        = ( 1 << 5 ),   /* modify apps :
         is used as return value only:
         permission is not required - apps update it will be always performed */
};

/* ImportNgcObj
 *   import of a KNgcObj
 *
 *  the KNgcObj has to be created with KNgcObjMakeFromFile() kfg/ngc.h
 *  the permission-flags define what the functions is allowd to do
 *  result flags show what was done according to the permissions
 */
KFG_EXTERN rc_t CC KRepositoryMgrImportNgcObj( KRepositoryMgr *self,
    const struct KNgcObj * ngc, const char * location, uint32_t permissions, uint32_t * result_flags );


/* HasRemoteAccess
 *  whether remote access is available (has an enabled remote repository)
 */
KFG_EXTERN bool CC KRepositoryMgrHasRemoteAccess ( const KRepositoryMgr *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_repository_ */
