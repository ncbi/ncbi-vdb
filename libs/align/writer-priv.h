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
#ifndef _h_align_writer_priv_
#define _h_align_writer_priv_

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <kdb/meta.h>

#include <align/writer-cmn.h>

enum TableWriterColumn_Array {
    ewcol_Ignore = 0x01,
    ewcol_IsArray = 0x02,
    ewcol_Temporary = 0x04
};

/* use ewcol_Ignore to skip optional column when cursor is created */
typedef struct TableWriterColumn_struct {
    uint32_t idx;
    const char* name;
    bitsz_t element_bits;
    uint32_t flags;
} TableWriterColumn;

typedef struct TableWriter TableWriter;

rc_t CC TableWriter_Make(const TableWriter** cself, VDatabase *db, const char* table, const char* table_path /* NULL */);

rc_t CC TableWriter_MakeMgr(const TableWriter** cself, VDBManager* mgr, const char* schema_path, const char* type, const char* table_path);

rc_t CC TableWriter_MakeUpdate(const TableWriter** cself, VDatabase *db, const char* table);

/* add scursor based on column list */
rc_t CC TableWriter_AddCursor(const TableWriter* cself, TableWriterColumn* cols, uint32_t col_qty, uint8_t* cursor_id);

rc_t CC TableWriter_GetVTable(const TableWriter* cself, VTable** vtbl);

rc_t CC TableWriter_Sign(const TableWriter *const cself,
                         const char loader_name[],
                         const ver_t loader_version,
                         const char loader_date[],
                         const char app_name[],
                         const ver_t app_version);

/* optionally return internal vtbl still open but committed */
rc_t CC TableWriter_Whack(const TableWriter* cself, bool commit, uint64_t* rows);

/* commits and closes cursor_id, returns number of rows written in cursor, optionally */
rc_t CC TableWriter_CloseCursor(const TableWriter* cself, uint8_t cursor_id, uint64_t* rows);

/* flushes any pending writes into the table */
rc_t CC TableWriter_Flush(const TableWriter *cself, const uint8_t cursor_id);

/* CANNOT OPEN ROWS IN MULTIPLE CURSORS AT ONCE
    Use sequence:
    OpenRow
    ColumnWrite - 0 (better set some defaults if not writing to a column!) or more times.
    CloseRow
 */
/* open a row for writing by cursor_id */
rc_t CC TableWriter_OpenRow(const TableWriter* cself, int64_t* rowid, const uint8_t cursor_id);

/* open a specific row for writing by cursor_id */
rc_t CC TableWriter_OpenRowId(const TableWriter* cself, const int64_t rowid, const uint8_t cursor_id);

/* return sequentially next rowid which will be created on OpenRow request */
rc_t CC TableWriter_GetNextRowId(const TableWriter* cself, int64_t* rowid, const uint8_t cursor_id);

/* writes cursor_id cursor default value to column
   column pointer must use same object as passed into AddCursor
 */
rc_t CC TableWriter_ColumnDefault(const TableWriter* cself, const uint8_t cursor_id,
                                  const TableWriterColumn* column, const TableWriterData *data);

/* writes value to column into cursor last passed to OpenRow*
   column pointer must use same object as passed into AddCursor
 */
rc_t CC TableWriter_ColumnWrite(const TableWriter* cself,
                                const TableWriterColumn* column, const TableWriterData *data);

/* closes row in cursor last used in OpenRow* */
rc_t CC TableWriter_CloseRow(const TableWriter* cself);

#define TW_COL_WRITE_DEF(writer, curs, col, data) \
    if( rc == 0 ) { \
        rc = TableWriter_ColumnDefault(writer, curs, &(col), &(data)); \
    }

#define TW_COL_WRITE_DEF_VAR(writer, curs, col, var) \
    if( rc == 0 ) { \
        TableWriterData dz; \
        dz.buffer = &(var); \
        dz.elements = 1; \
        TW_COL_WRITE_DEF(writer, curs, col, dz); \
    }

#define TW_COL_WRITE_DEF_BUF(writer, curs, col, buf, elems) \
    if( rc == 0 ) { \
        TableWriterData dz; \
        dz.buffer = buf; \
        dz.elements = elems; \
        TW_COL_WRITE_DEF(writer, curs, col, dz); \
    }

#define TW_COL_WRITE(writer, col, data) \
    if( rc == 0 && (col).idx != 0) { \
        rc = TableWriter_ColumnWrite(writer, &(col), &(data)); \
    }

#define TW_COL_WRITE_VAR(writer, col, var) \
    if( rc == 0 && (col).idx != 0) { \
        TableWriterData dz; \
        dz.buffer = &(var); \
        dz.elements = 1; \
        TW_COL_WRITE(writer, col, dz); \
    }

#define TW_COL_WRITE_BUF(writer, col, buf, elems) \
    if( rc == 0 && (col).idx != 0) { \
        TableWriterData dz; \
        dz.buffer = buf; \
        dz.elements = elems; \
        TW_COL_WRITE(writer, col, dz); \
    }

#endif /* _h_align_writer_priv_ */
