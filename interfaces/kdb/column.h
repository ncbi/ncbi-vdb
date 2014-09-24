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

#ifndef _h_kdb_column_
#define _h_kdb_column_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct KDBManager;


/*--------------------------------------------------------------------------
 * KChecksum
 *  describes blob checksum
 */
typedef uint8_t KChecksum;
enum
{
    kcsNone,
    kcsCRC32,
    kcsMD5
};


/*--------------------------------------------------------------------------
 * KColumn
 *  a collection of blobs indexed by oid
 */
typedef struct KColumn KColumn;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KColumnAddRef ( const KColumn *self );
KDB_EXTERN rc_t CC KColumnRelease ( const KColumn *self );


/* CreateColumn
 * VCreateColumn
 *  create a new or open an existing column
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "checksum" [ IN ] - the type of checksum information to
 *  apply when writing blobs
 *
 *  "pgsize" [ IN, DEFAULT ZERO ] - size of internal column "pages"
 *  the default value is indicated by 0 ( zero ).
 *  NB - CURRENTLY THE ONLY SUPPORTED PAGE SIZE IS 1 ( ONE ) BYTE.
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
KDB_EXTERN rc_t CC KDBManagerCreateColumn ( struct KDBManager *self,
    KColumn **col, KCreateMode cmode, KChecksum checksum,
    size_t pgsize, const char *path, ... );
KDB_EXTERN rc_t CC KTableCreateColumn ( struct KTable *self,
    KColumn **col, KCreateMode cmode, KChecksum checksum,
    size_t pgsize, const char *path, ... );

KDB_EXTERN rc_t CC KDBManagerVCreateColumn ( struct KDBManager *self,
    KColumn **col, KCreateMode cmode, KChecksum checksum,
    size_t pgsize, const char *path, va_list args );
KDB_EXTERN rc_t CC KTableVCreateColumn ( struct KTable *self,
    KColumn **col, KCreateMode cmode, KChecksum checksum,
    size_t pgsize, const char *path, va_list args );


/* OpenColumnRead
 * VOpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
KDB_EXTERN rc_t CC KDBManagerOpenColumnRead ( struct KDBManager const *self,
    const KColumn **col, const char *path, ... );
KDB_EXTERN rc_t CC KTableOpenColumnRead ( struct KTable const *self,
    const KColumn **col, const char *path, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenColumnRead ( struct KDBManager const *self,
    const KColumn **col, const char *path, va_list args );
KDB_EXTERN rc_t CC KTableVOpenColumnRead ( struct KTable const *self,
    const KColumn **col, const char *path, va_list args );


/* OpenColumnUpdate
 * VOpenColumnUpdate
 *  open a column for read/write
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
KDB_EXTERN rc_t CC KDBManagerOpenColumnUpdate ( struct KDBManager *self,
    KColumn **col, const char *path, ... );
KDB_EXTERN rc_t CC KTableOpenColumnUpdate ( struct KTable *self,
    KColumn **col, const char *path, ... );

KDB_EXTERN rc_t CC KDBManagerVOpenColumnUpdate ( struct KDBManager *self,
    KColumn **col, const char *path, va_list args );
KDB_EXTERN rc_t CC KTableVOpenColumnUpdate ( struct KTable *self,
    KColumn **col, const char *path, va_list args );


/* Locked
 *  returns true if locked
 */
KDB_EXTERN bool CC KColumnLocked ( const KColumn *self );


/* Version
 *  returns the format version
 */
KDB_EXTERN rc_t CC KColumnVersion ( const KColumn *self, uint32_t *version );


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
KDB_EXTERN rc_t CC KColumnByteOrder ( const KColumn *self, bool *reversed );


/* IdRange
 *  returns id range for column
 *
 *  "first" [ OUT ] - first id in column
 *
 *  "count" [ OUT ] - number of ids represented by this column
 */
KDB_EXTERN rc_t CC KColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count );


/* Reindex
 *  optimize indices
 */
KDB_EXTERN rc_t CC KColumnReindex ( KColumn *self );


/* CommitFreq
 * SetCommitFreq
 *  manage frequency of commits
 */
KDB_EXTERN rc_t CC KColumnCommitFreq ( KColumn *self, uint32_t *freq );
KDB_EXTERN rc_t CC KColumnSetCommitFreq ( KColumn *self, uint32_t freq );


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KColumnOpenManagerRead ( const KColumn *self, struct KDBManager const **mgr );
KDB_EXTERN rc_t CC KColumnOpenManagerUpdate ( KColumn *self, struct KDBManager **mgr );


/* OpenParent
 *  duplicate reference to parent table
 *  NB - returned reference must be released
 */
KDB_EXTERN rc_t CC KColumnOpenParentRead ( const KColumn *self, struct KTable const **tbl );
KDB_EXTERN rc_t CC KColumnOpenParentUpdate ( KColumn *self, struct KTable **tbl );


/*--------------------------------------------------------------------------
 * KColumnBlob
 *  one or more rows of column data
 */
typedef struct KColumnBlob KColumnBlob;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KColumnBlobAddRef ( const KColumnBlob *self );
KDB_EXTERN rc_t CC KColumnBlobRelease ( const KColumnBlob *self );


/* CreateBlob
 *  creates a new, unassigned blob
 */
KDB_EXTERN rc_t CC KColumnCreateBlob ( KColumn *self, KColumnBlob **blob );


/* OpenBlobRead
 * OpenBlobUpdate
 *  opens an existing blob containing row data for id
 */
KDB_EXTERN rc_t CC KColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blob, int64_t id );
KDB_EXTERN rc_t CC KColumnOpenBlobUpdate ( KColumn *self, KColumnBlob **blob, int64_t id );


/* Read
 *  read data from blob
 *
 *  "offset" [ IN ] - starting offset into blob
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read. specifically,
 *  "offset" + "num_read" + "remaining" == sizeof blob
 */
KDB_EXTERN rc_t CC KColumnBlobRead ( const KColumnBlob *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining );


/* Append
 *  append data to open blob
 *
 *  "buffer" [ IN ] and "size" [ IN ] - blob data
 */
KDB_EXTERN rc_t CC KColumnBlobAppend ( KColumnBlob *self, const void *buffer, size_t size );


/* Validate
 *  runs checksum validation on unmodified blob
 */
KDB_EXTERN rc_t CC KColumnBlobValidate ( const KColumnBlob *self );


/* IdRange
 * AssignRange
 *  access id range for blob
 *
 *  "first" [ OUT ] and  "count" [ OUT ] - return parameters for IdRange

 *  "first" [ IN ] and "count" [ IN ] - range parameters for assign
 */
KDB_EXTERN rc_t CC KColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );
KDB_EXTERN rc_t CC KColumnBlobAssignRange ( KColumnBlob *self, int64_t first, uint32_t count );


/* Commit
 *  commit changes to blob
 *  close to further updates
 */
KDB_EXTERN rc_t CC KColumnBlobCommit ( KColumnBlob *self );


#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_column_ */
