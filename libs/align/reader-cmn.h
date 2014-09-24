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
#ifndef _h_align_reader_cmn_
#define _h_align_reader_cmn_

#include <insdc/insdc.h>
#include <insdc/sra.h>

enum TableReaderColumn_Array {
    ercol_Optional = 0x01,
    ercol_Skip = 0x02
};

/* use ercol_Ignore to skip optional column when cursor is created */
typedef struct TableReaderColumn_struct {
    uint32_t idx;
    const char* name;
    union {
        const void* var;
        const char* str;
        const bool* buul;
        const uint8_t* u8;
        const int16_t* i16;
        const uint16_t* u16;
        const int32_t* i32;
        const uint32_t* u32;
        const int64_t* i64;
        const uint64_t* u64;
        const INSDC_coord_one* coord1;
        const INSDC_coord_zero* coord0;
        const INSDC_coord_len* coord_len;
        const INSDC_coord_val* coord_val;
        const INSDC_SRA_xread_type* read_type;
        const INSDC_SRA_read_filter* read_filter;
    } base;
    uint32_t len;
    uint32_t flags;
} TableReaderColumn;

typedef struct TableReader TableReader;

rc_t CC TableReader_Make(const TableReader** cself, const VTable* table, TableReaderColumn* cols, size_t cache);

rc_t CC TableReader_MakeCursor(const TableReader** cself, const VCursor* cursor, TableReaderColumn* cols );

rc_t CC TableReader_OpenCursor( const TableReader* cself );

void CC TableReader_Whack(const TableReader* cself);

rc_t CC TableReader_ReadRow(const TableReader* cself, int64_t rowid);

rc_t CC TableReader_IdRange(const TableReader* cself, int64_t* first, uint64_t* count);

rc_t CC TableReader_OpenIndex(const TableReader* cself, const char* name, const KIndex** idx);

rc_t CC TableReader_PageIdRange(const TableReader *cself, int64_t rowid, int64_t *first, int64_t *last);

#endif /* _h_align_reader_cmn_ */
