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

#ifndef _h_sra_wsradb_
#define _h_sra_wsradb_

#ifndef _h_sra_extern_
#include <sra/extern.h>
#endif

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct KDirectory;


/*--------------------------------------------------------------------------
 * SRAMgr
 *  opaque handle to SRA library
 */


/* MakeUpdate
 *  create library handle for read/write access
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 *
 *  NB - not implemented in read-only library,
 *  and the read-only library may not be mixed with read/write
 */
SRA_EXTERN rc_t CC SRAMgrMakeUpdate ( SRAMgr **mgr, struct KDirectory *wd );


/* SetMD5Mode
 *  sets default MD5 file mode for all objects
 *  opened for update under manager
 */
SRA_EXTERN rc_t CC SRAMgrSetMD5Mode ( SRAMgr *self, bool useMD5 );


/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
SRA_EXTERN rc_t CC SRAMgrLock ( SRAMgr *self, const char *path, ... );
SRA_EXTERN rc_t CC SRAMgrVLock ( SRAMgr *self, const char *path, va_list args );


/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
SRA_EXTERN rc_t CC SRAMgrUnlock ( SRAMgr *self, const char *path, ... );
SRA_EXTERN rc_t CC SRAMgrVUnlock ( SRAMgr *self, const char *path, va_list args );


/* DropTable
 *  drop an existing table
 *
 *  "force" [ IN ] - if true, make every attempt to remove table
 *
 *  "path" [ IN ]  - NUL terminated table name
 */
 SRA_EXTERN rc_t CC SRAMgrDropTable ( SRAMgr *self, bool force, const char *path, ... );
 SRA_EXTERN rc_t CC SRAMgrVDropTable ( SRAMgr *self, bool force, const char *path, va_list args );


/*--------------------------------------------------------------------------
 * SRATable
 *  a collection of spots with several data series, minimally including
 *  base or color calls and their quality ( confidence ) values, and
 *  optionally signal-related values ( signal, intensity, noise, ... ).
 */

/* Create
 *  creates a new table
 *
 *  "tbl" [ OUT ] - return parameter for table
 *
 *  "typespec" [ IN ] - type and optionally version of table schema,
 *  e.g. 'MY_NAMESPACE:MyTable' or 'MY_NAMESPACE:MyTable#1.1'
 *
 *  "path" [ IN ] - NUL terminated table name
 */
SRA_EXTERN rc_t CC SRAMgrCreateTable ( SRAMgr *self, SRATable **tbl,
    const char *typespec, const char *path, ... );
SRA_EXTERN rc_t CC SRAMgrVCreateTable ( SRAMgr *self, SRATable **tbl,
    const char *typespec, const char *path, va_list args );


/* OpenUpdate
 *  open an existing table
 *
 *  "run" [ OUT ] - return parameter for table
 *
 *  "path" [ IN ] - NUL terminated table name
 */
SRA_EXTERN rc_t CC SRAMgrOpenTableUpdate ( SRAMgr *self,
    SRATable **tbl, const char *path, ... );
SRA_EXTERN rc_t CC SRAMgrVOpenTableUpdate ( SRAMgr *self,
    SRATable **tbl, const char *path, va_list args );


/* NewSpot
 *  creates a new spot record, returns spot id.
 *
 *  "id" [ OUT ] - return parameter for id of newly created spot
 */
SRA_EXTERN rc_t CC SRATableNewSpot ( SRATable *self, spotid_t *id );


/* OpenSpot
 *  opens an existing spot record from id
 *
 *  "id" [ IN ] - 1-based spot id
 */
SRA_EXTERN rc_t CC SRATableOpenSpot ( SRATable *self, spotid_t id );


/* CloseSpot
 *  closes a spot opened with either NewSpot or OpenSpot
 */
SRA_EXTERN rc_t CC SRATableCloseSpot ( SRATable *self );


/* Commit
 *  commit all changes
 */
SRA_EXTERN rc_t CC SRATableCommit ( SRATable *self );


/* OpenColumnWrite
 *  open a column for write
 *
 *  "idx" [ OUT ] - return parameter for 1-based column index.
 *
 *  "col" [ OUT, NULL OKAY ] - optional return parameter for
 *  newly opened column.
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
 *
 *  "datatype" [ IN ] - NUL terminated string in ASCII
 *   describing fully qualified column data type
 */
SRA_EXTERN rc_t CC SRATableOpenColumnWrite ( SRATable *self, uint32_t *idx,
    SRAColumn **col, const char *name, const char *datatype );


/* SetIdxColumnDefault
 *  give a default value for column
 *
 *  if no value gets written to a column within an open spot,
 *  this value is substituted.
 *
 *  "idx" [ IN ] - 1-based column index
 *
 *  "base" [ IN ] and "offset" [ IN ] - pointer and bit offset
 *  to start of row data
 *
 *  "size" [ IN ] - size in bits of row data
 */
SRA_EXTERN rc_t CC SRATableSetIdxColumnDefault ( SRATable *self, uint32_t idx,
    const void *base, bitsz_t offset, bitsz_t size );


/* WriteIdxColumn
 *  write row data to an indexed column
 *
 *  "idx" [ IN ] - 1-based column index
 *
 *  "base" [ IN ] and "offset" [ IN ] - pointer and bit offset
 *  to start of row data
 *
 *  "size" [ IN ] - size in bits of row data
 */
SRA_EXTERN rc_t CC SRATableWriteIdxColumn ( SRATable *self, uint32_t idx,
    const void *base, bitsz_t offset, bitsz_t size );


/* MetaFreeze
 *  freezes current metadata revision
 *  further modification will begin on a copy
 */
SRA_EXTERN rc_t CC SRATableMetaFreeze ( SRATable *self );


/* OpenMDataNode
 *  open a metadata node
 */
SRA_EXTERN rc_t CC SRATableOpenMDataNodeUpdate ( SRATable *self,
    struct KMDataNode **node, const char *path, ... );
SRA_EXTERN rc_t CC SRATableVOpenMDataNodeUpdate ( SRATable *self,
    struct KMDataNode **node, const char *path, va_list args );


#ifdef __cplusplus
}
#endif

#endif /* _h_sra_wsradb_ */
