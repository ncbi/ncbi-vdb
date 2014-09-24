/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was readten as part of
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
#ifndef _h_align_reader_reference_
#define _h_align_reader_reference_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifndef _h_vdb_cursor_
#include <vdb/cursor.h>
#endif

#ifndef _h_vdb_table_
#include <vdb/table.h>
#endif

#ifndef _h_vdb_database_
#include <vdb/database.h>
#endif

#ifndef _h_vdb_manager_
#include <vdb/manager.h>
#endif

#ifndef _h_align_iterator_
#include <align/iterator.h>
#endif

#ifndef _h_insdc_insdc_
#include <insdc/insdc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct VCursor;
struct PlacementIterator;

enum ReferenceList_Options {
    ereferencelist_4na = 0x01, /* READ in 4na, otherwise it is DNA */
    /* next 3 options are used only if iterator is NOT created using existing cursor */
    ereferencelist_usePrimaryIds = 0x02, /* read PRIMARY_ALIGNMENT_IDS too */
    ereferencelist_useSecondaryIds = 0x04, /* read SECONDARY_ALIGNMENT_IDS too */
    ereferencelist_useEvidenceIds = 0x08 /* use EVIDENCE_ALIGNMENT */
};

typedef struct ReferenceList ReferenceList;

/** name is used to limit only to a single sequence - for quick lookups **/
/** numbins is used to divide whole reference table in equi-row-bins and show only the bin where the name is falling to **/
ALIGN_EXTERN rc_t CC ReferenceList_MakeCursor(const ReferenceList** cself, const VCursor* cursor, uint32_t options,
                                              const char* name, const uint32_t numbins);

ALIGN_EXTERN rc_t CC ReferenceList_MakeTable(const ReferenceList** cself, const VTable* table, uint32_t options,
                                             size_t cache, const char* name, const uint32_t numbins);

ALIGN_EXTERN rc_t CC ReferenceList_MakeDatabase(const ReferenceList** cself, const VDatabase* db, uint32_t options,
                                                size_t cache, const char* name, const uint32_t numbins);

ALIGN_EXTERN rc_t CC ReferenceList_MakePath(const ReferenceList** cself, const VDBManager* vmgr, const char* dbpath,
                                            uint32_t options, size_t cache,const char *name, const uint32_t numbins);

ALIGN_EXTERN rc_t CC ReferenceList_AddRef(const ReferenceList *cself);
ALIGN_EXTERN void CC ReferenceList_Release(const ReferenceList* cself);

/* number of items in the list */
ALIGN_EXTERN rc_t CC ReferenceList_Count(const ReferenceList* cself, uint32_t* count);

typedef struct ReferenceObj ReferenceObj;

/* find object by SEQ_ID and if not found by NAME */
ALIGN_EXTERN rc_t CC ReferenceList_Find(const ReferenceList* cself, const ReferenceObj** obj, const char* key, size_t key_sz);

/* idx is 0-based */
ALIGN_EXTERN rc_t CC ReferenceList_Get(const ReferenceList* cself, const ReferenceObj** obj, uint32_t idx);

ALIGN_EXTERN rc_t ReferenceObj_AddRef(const ReferenceObj *cself);
ALIGN_EXTERN void CC ReferenceObj_Release(const ReferenceObj* cself);

/* internal idx for use in ReferenceList_Get above */
ALIGN_EXTERN rc_t CC ReferenceObj_Idx(const ReferenceObj* cself, uint32_t* idx);
/* Gets RowId range for the object */
ALIGN_EXTERN rc_t CC ReferenceObj_IdRange(const ReferenceObj* cself, int64_t* start, int64_t* stop);
/* Gets Current bin number if ReferenceList was binned*/
ALIGN_EXTERN rc_t CC ReferenceObj_Bin(const ReferenceObj* cself, uint32_t* bin);

ALIGN_EXTERN rc_t CC ReferenceObj_SeqId(const ReferenceObj* cself, const char** seqid);

ALIGN_EXTERN rc_t CC ReferenceObj_Name(const ReferenceObj* cself, const char** name);

ALIGN_EXTERN rc_t CC ReferenceObj_SeqLength(const ReferenceObj* cself, INSDC_coord_len* len);

ALIGN_EXTERN rc_t CC ReferenceObj_Circular(const ReferenceObj* cself, bool* circular);

/* check if actual refseq data is located in external table (external == true on return)
   optionally obtain its path, returned path == NULL means actual refseq is missing
   if path was requested and obtained caller must free(path)! */
ALIGN_EXTERN rc_t CC ReferenceObj_External(const ReferenceObj* cself, bool* external, char** path);

/* read a chunk of refseq into buffer from offset up to offset + len
   if offset is beyond non-circular refseq size error is returned
 */
ALIGN_EXTERN rc_t CC ReferenceObj_Read(const ReferenceObj* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                       uint8_t* buffer, INSDC_coord_len* written);

ALIGN_EXTERN rc_t CC ReferenceObj_GetIdCount( const ReferenceObj* cself, int64_t row_id, uint32_t *count );

/* return pointer to iterator for (PRIMARY|SECONDARY)_ALIGNMENT_IDS to a given range on reference,
   both cursors could be NULL
   ref_len will be truncated to seq length for non-circular references
   for other parameters see AlignMgrMakePlacementIterator
 */
ALIGN_EXTERN rc_t CC ReferenceObj_MakePlacementIterator ( const ReferenceObj* cself,
    PlacementIterator **iter,
    INSDC_coord_zero ref_window_start,
    INSDC_coord_len ref_window_len, int32_t min_mapq,
    struct VCursor const *ref_cur, struct VCursor const *align_cur, align_id_src ids,
    const PlacementRecordExtendFuncs *ext_0, const PlacementRecordExtendFuncs *ext_1,
    const char * rd_group, void * placement_ctx );

#ifdef __cplusplus
}
#endif

#endif /* _h_align_reader_reference_ */
