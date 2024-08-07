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

#ifndef _h_kdb_manager_
#define _h_kdb_manager_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

/* #ifndef _h_kfs_directory_ */
/* #include <kfs/directory.h> */
/* #endif */
#include <kfs/defs.h> /* kpt types */

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VPath;
struct KDirectory;

/*--------------------------------------------------------------------------
 * KDBPathType
 *  extends KPathType from <kfs/defs.h>
 */
enum
{
    /* must be handled carefully and can not go back to KDirectory
     * as this value has a different meaning in KDirectory */
    kptAny = 0,
    kptDatabase = kptLastDefined,
    kptTable,
    kptIndex,
    kptColumn,

    /* these values may not be used in KDBManagerExists below */
    kptMetadata,
    kptPrereleaseTbl
};


/*--------------------------------------------------------------------------
 * KDBManager
 *  opaque handle to library
 */
typedef struct KDBManager KDBManager;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KDBManagerAddRef ( const KDBManager *self );
KDB_EXTERN rc_t CC KDBManagerRelease ( const KDBManager *self );


/* MakeRead
 * MakeUpdate
 * MakeText (reads data from a Json string)
 *
 *  create library handle for specific use
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 */
KDB_EXTERN rc_t CC KDBManagerMakeRead ( const KDBManager **mgr, struct KDirectory const *wd );
KDB_EXTERN rc_t CC KDBManagerMakeUpdate ( KDBManager **mgr, struct KDirectory *wd );
KDB_EXTERN rc_t CC KDBManagerMakeText ( const KDBManager **mgr, const char * input, char * error, size_t error_size );

/* Version
 *  returns the library version
 */
KDB_EXTERN rc_t CC KDBManagerVersion ( const KDBManager *self, uint32_t *version );


/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *
 *  "path" [ IN ] - NUL terminated path
 *
 * DEPRECATED:
 * Should use KDBManagerPathType for reduced network thrashing.
 */
KDB_EXTERN bool CC KDBManagerExists ( const KDBManager *self, uint32_t type,
    const char *name, ... );
KDB_EXTERN bool CC KDBManagerVExists ( const KDBManager *self, uint32_t type,
    const char *name, va_list args );


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "path" [ IN ] - NUL terminated path
 */
KDB_EXTERN rc_t CC KDBManagerWritable ( const KDBManager *self,
    const char *path, ... );
KDB_EXTERN rc_t CC KDBManagerVWritable ( const KDBManager *self,
    const char *path, va_list args );

/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
KDB_EXTERN rc_t CC KDBManagerLock ( KDBManager *self, const char *path, ... );
KDB_EXTERN rc_t CC KDBManagerVLock ( KDBManager *self, const char *path, va_list args );

/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
KDB_EXTERN rc_t CC KDBManagerUnlock ( KDBManager *self, const char *path, ... );
KDB_EXTERN rc_t CC KDBManagerVUnlock ( KDBManager *self, const char *path, va_list args );


/* Drop
 *  drop an object based on its path
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to the kdb object
 */
KDB_EXTERN rc_t CC KDBManagerDrop ( KDBManager *self, uint32_t obj_type, const char *path, ... );
KDB_EXTERN rc_t CC KDBManagerVDrop ( KDBManager *self, uint32_t obj_type, const char *path, va_list args );


/* RunPeriodicTasks
 *  executes periodic tasks, such as cache flushing
 */
KDB_EXTERN rc_t CC KDBManagerRunPeriodicTasks ( const KDBManager *self );


/* PathType
 *  check the path type of an object/directory path.
 *  this is an extension of the KDirectoryPathType and will return
 *  the KDirectory values if a path type is not specifically a
 *  kdb object
 */
KDB_EXTERN int CC KDBManagerPathTypeVP ( const KDBManager * self, const struct VPath * path );
KDB_EXTERN int CC KDBManagerPathType   ( const KDBManager * self, const char *path, ... );
KDB_EXTERN int CC KDBManagerVPathType  ( const KDBManager * self, const char *path, va_list args );

typedef struct KDBContents_struct KDBContents;

struct KDBContents_struct {
    char const *name;
    KDBContents const *parent;
    KDBContents const *firstChild;  ///< owning reference
    KDBContents const *nextSibling; ///< owning reference
    KDBContents const *prevSibling;
    KPathType dbtype;               ///< apparent type from KDB's PoV, like KDBPathType would give
    KPathType fstype;               ///< from KFS's PoV, like KDirectoryPathType would give
    int attributes;
};

enum KDBContents_Common_Attributes {
    cca_HasMetadata = 1,
    cca_HasMD5_File = 2,
    cca_HasLock = 4,
    cca_HasSealed = 8,
    cca_HasErrors = (1 << 30),
};

enum KDBContents_Column_Attributes {
    cca_HasChecksum_CRC = 16,
    cca_HasChecksum_MD5 = 32,
    cca_ReversedByteOrder = 64,
    cca_IsStatic = 128,
};

enum KDBContents_Table_Attributes {
    cta_HasColumns = 16,
    cta_HasIndices = 32,
};

enum KDBContents_Database_Attributes {
    cda_HasTables = 16,
    cda_HasDatabases = 32,
};

enum KDBContents_Index_Attributes {
    cia_HasChecksum_MD5 = 16,
    cia_IsTextIndex = 32,
    cia_IsIdIndex = 64,
    cia_ReversedByteOrder = 128,
};

void KDBContentsWhack(KDBContents const *self);

#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_manager_ */
