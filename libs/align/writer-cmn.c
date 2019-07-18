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
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <kdb/meta.h>
#include <kapp/main.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>
#include <align/writer-cmn.h>
#include "writer-priv.h"
#include "debug.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TW_MAX_CURSORS 8

struct TableWriter {
    /* db OR mgr not both */
    VDatabase* vdb;
    VDBManager* vmgr;
    char* table_path;
    VSchema* vschema;
    bool is_update;

    VTable* vtbl;
    char* table;
    struct {
        uint32_t col_qty; /* != 0 cursor is used */
        const TableWriterColumn* cols;
        VCursor* cursor; /* != NULL cursor is opened */
        uint64_t rows;
    } *curr, cursors[TW_MAX_CURSORS];
};

static
rc_t TableWriter_Init(TableWriter** self, const char* table, const char* table_path)
{
    rc_t rc = 0;

    if( self == NULL || table == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (*self = calloc(1, sizeof(**self))) == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcMemory, rcExhausted);
    } else {
        (*self)->table = string_dup( table, string_size( table ) );
        if( (*self)->table == NULL ) {
            rc = RC(rcAlign, rcType, rcConstructing, rcMemory, rcExhausted);
        } else if( table_path != NULL ) {
            (*self)->table_path = string_dup( table_path, string_size( table_path ) );
            if( (*self)->table_path == NULL ) {
                rc = RC(rcAlign, rcType, rcConstructing, rcMemory, rcExhausted);
            }
        }
    }
    return rc;
}

static
rc_t TableWriter_OpenCursor(const TableWriter* cself, uint8_t cursor_id)
{
    rc_t rc = 0;

    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcInvalid);
    } else if( cself->cursors[cursor_id].cursor == NULL ) {
        TableWriter* self = (TableWriter*)cself;

        if( self->vtbl == NULL ) {
            if( self->vmgr != NULL ) {
                rc = VDBManagerCreateTable(self->vmgr, &self->vtbl, self->vschema, self->table, kcmCreate | kcmMD5, "%s", self->table_path);
            } else if( self->is_update ) {
                rc = VDatabaseOpenTableUpdate(self->vdb, &self->vtbl, "%s", self->table);
            } else {
                rc = VDatabaseCreateTable(self->vdb, &self->vtbl, self->table, kcmCreate | kcmMD5, "%s", self->table_path);
            }
        }

        if( rc == 0 &&
            (rc = VTableColumnCreateParams(self->vtbl, kcmCreate, kcsCRC32, 0)) == 0 &&
            (rc = VTableCreateCursorWrite(self->vtbl, &self->cursors[cursor_id].cursor, kcmInsert)) == 0 ) {
            uint32_t i;
            for(i = 0; rc == 0 && i < self->cursors[cursor_id].col_qty; i++) {
                if( !(self->cursors[cursor_id].cols[i].flags & ewcol_Ignore) ) {
                    if( (rc = VCursorAddColumn(self->cursors[cursor_id].cursor,
                                               (uint32_t*)&self->cursors[cursor_id].cols[i].idx,
                                               "%s", self->cursors[cursor_id].cols[i].name)) == 0 ) {
                        ALIGN_DBG("column %s opened", self->cursors[cursor_id].cols[i].name);
                    } else {
                        (void)PLOGERR(klogErr, (klogErr, rc, "table $(table) failed to create column '$(column)'",
                                                             "table=%s,column=%s", self->table, self->cursors[cursor_id].cols[i].name));
                    }
                }
            }
            if( rc == 0 ) {
                rc = VCursorOpen(self->cursors[cursor_id].cursor);
            }
        }
        if( rc == 0 ) {
            ALIGN_DBG("table %s opened cursor %u for writing", self->table, cursor_id);
        } else {
            ALIGN_DBGERRP("table %s failed to open cursor %u for writing", rc, self->table, cursor_id);
        }
    }
    return rc;
}

rc_t CC TableWriter_Make(const TableWriter** cself, VDatabase *db, const char* table, const char* table_path)
{
    rc_t rc = 0;
    TableWriter* obj = NULL;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (rc = TableWriter_Init(&obj, table, table_path ? table_path : table)) == 0 &&
               (rc = VDatabaseAddRef(db)) == 0 ) {
        obj->vdb = db;
    }
    if( rc == 0 ) {
        *cself = obj;
        ALIGN_DBG("table %s opened for writing", (*cself)->table);
    } else {
        TableWriter_Whack(obj, false, NULL);
        ALIGN_DBGERRP("table %s at %s", rc, table, table_path);
    }
    return rc;
}

rc_t CC TableWriter_MakeMgr(const TableWriter** cself, VDBManager* mgr, const char* schema_path,
                            const char* type, const char* table_path)
{
    rc_t rc = 0;
    TableWriter* obj = NULL;

    if( cself == NULL || mgr == NULL || schema_path == NULL || type == NULL || table_path == NULL) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (rc = TableWriter_Init(&obj, type, table_path)) == 0 &&
               (rc = VDBManagerMakeSchema(mgr, &obj->vschema)) == 0 &&
               (rc = VSchemaParseFile(obj->vschema, "%s", schema_path)) == 0 &&
               (rc = VDBManagerAddRef(mgr)) == 0 ) {
       obj->vmgr = mgr;
    }
    if( rc == 0 ) {
        *cself = obj;
        ALIGN_DBG("table %s at %s opened for writing", (*cself)->table, (*cself)->table_path);
    } else {
        TableWriter_Whack(obj, false, NULL);
        ALIGN_DBGERRP("table %s at %s", rc, type, table_path);
    }
    return rc;
}

rc_t CC TableWriter_MakeUpdate(const TableWriter** cself, VDatabase *db, const char* table)
{
    rc_t rc = 0;
    TableWriter* obj = NULL;

    if( cself == NULL || db == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( (rc = TableWriter_Init(&obj, table, NULL)) == 0 &&
               (rc = VDatabaseAddRef(db)) == 0 ) {
        obj->is_update = true;
        obj->vdb = db;
    }
    if( rc == 0 ) {
        *cself = obj;
        ALIGN_DBG("table %s opened for update", (*cself)->table);
    } else {
        TableWriter_Whack(obj, false, NULL);
        ALIGN_DBGERRP("table %s", rc, table);
    }
    return rc;
}

rc_t CC TableWriter_GetVTable(const TableWriter* cself, VTable** vtbl)
{
    rc_t rc = 0;

    if( cself == NULL || vtbl == NULL ) {
        rc = RC(rcAlign, rcType, rcAccessing, rcParam, rcNull);
    } else {
        *vtbl = cself->vtbl;
    }
    return rc;
}

rc_t CC TableWriter_Sign(const TableWriter *const cself,
                         const char loader_name[],
                         const ver_t loader_version,
                         const char loader_date[],
                         const char app_name[],
                         const ver_t app_version)
{
    rc_t rc = 0;
    KMetadata* meta = NULL;
    KMDataNode* root = NULL;

    if( cself == NULL || loader_name == NULL || loader_date == NULL || app_name == NULL || app_version == 0 ) {
        rc = RC(rcAlign, rcMetadata, rcWriting, rcParam, rcNull);
    } else if( (rc = VTableOpenMetadataUpdate(cself->vtbl, &meta)) == 0 &&
               (rc = KMetadataOpenNodeUpdate(meta, &root, "/")) == 0 ) {
        KMDataNode *node = NULL;
        if( (rc = KMDataNodeOpenNodeUpdate(root, &node, "SOFTWARE")) == 0 ) {
            char str_vers[64];
            KMDataNode *subNode = NULL;
            if( (rc = KMDataNodeOpenNodeUpdate(node, &subNode, "loader")) == 0 ) {
                if( (rc = string_printf(str_vers, sizeof(str_vers), NULL, "%V", loader_version)) == 0 ) {
                    rc = KMDataNodeWriteAttr(subNode, "vers", str_vers);
                }
                if(rc == 0) {
                    rc = KMDataNodeWriteAttr(subNode, "date", loader_date);
                }
                if(rc == 0) {
                    const char* tool_name = strrchr(loader_name, '/');
                    const char* r = strrchr(loader_name, '\\');
                    if( tool_name != NULL && r != NULL && tool_name < r ) {
                        tool_name = r;
                    }
                    if( tool_name++ == NULL) {
                        tool_name = loader_name;
                    }
                    rc = KMDataNodeWriteAttr(subNode, "name", tool_name);
                }
                KMDataNodeRelease(subNode);
            }
            if(rc == 0 && (rc = KMDataNodeOpenNodeUpdate(node, &subNode, "formatter")) == 0 ) {
                if( (rc = string_printf(str_vers, sizeof(str_vers), NULL, "%V", app_version)) == 0 ) {
                    rc = KMDataNodeWriteAttr(subNode, "vers", str_vers);
                }
                if(rc == 0) {
                    rc = KMDataNodeWriteAttr(subNode, "name", app_name);
                }
                KMDataNodeRelease(subNode);
            }
            KMDataNodeRelease(node);
        }
        if( rc == 0 && (rc = KMDataNodeOpenNodeUpdate(root, &node, "LOAD")) == 0 ) {
            KMDataNode *subNode = NULL;
            if( (rc = KMDataNodeOpenNodeUpdate(node, &subNode, "timestamp")) == 0 ) {
                time_t t = time(NULL);
                rc = KMDataNodeWrite(subNode, &t, sizeof(t));
                KMDataNodeRelease(subNode);
            }
            KMDataNodeRelease(node);
        }
    }
    KMDataNodeRelease(root);
    KMetadataRelease(meta);
    return rc;
}

rc_t CC TableWriter_Whack(const TableWriter* cself, bool commit, uint64_t* rows)
{
    rc_t rc = 0;

    if( rows != NULL ) {
        *rows = 0;
    }
    if( cself != NULL ) {
        TableWriter* self = (TableWriter*)cself;
        uint32_t i, j;
        
        for(i = 0; i < TW_MAX_CURSORS; i++) {
            if( self->cursors[i].col_qty > 0 ) {
                self->curr = &self->cursors[i];
                if( self->curr->cursor != NULL ) {
                    rc_t rc1 = 0, rc2;
                    if( commit ) {
                        rc1 = VCursorCommit(self->curr->cursor);
                    }
                    rc2 = VCursorRelease(self->curr->cursor);
                    rc = rc ? rc : (rc1 ? rc1 : rc2);
                    self->curr->cursor = NULL;
                }
                if( i == 0 && rows != NULL ) {
                    *rows = self->curr->rows;
                }
            }
        }
        for(i = 0; i < TW_MAX_CURSORS; i++) {
            if( self->cursors[i].col_qty > 0 ) {
                self->curr = &self->cursors[i];
                for(j = 0; j < self->curr->col_qty; j++) {
                    if( self->curr->cols[j].idx != 0 && (self->curr->cols[j].flags & ewcol_Temporary) ) {
                        rc_t rc1 = VTableDropColumn(self->vtbl, "%s", self->curr->cols[j].name);
                        if (rc1 != 0) {
                            (void)PLOGERR(klogDebug, (klogDebug, rc1,
                                "table $(table) failed to drop temporary column '$(column)'",
                                "table=%s,column=%s", self->table, self->curr->cols[j].name));
                        }
                        else {
                            ALIGN_DBG("table %s dropped tmp col %s", self->table, self->curr->cols[j].name);
                        }
                    }
                }
            }
        }
        if( commit && rc == 0 && self->vtbl != NULL ) {
            rc = VTableReindex(self->vtbl);
        }
        VTableRelease(self->vtbl);
        VSchemaRelease(self->vschema);
        VDBManagerRelease(self->vmgr);
        VDatabaseRelease(self->vdb);
        if( rc == 0 ) {
            ALIGN_DBG("table %s%s%s", self->table, self->table_path ? " at " : "", self->table_path ? self->table_path : "");
        } else {
            ALIGN_DBGERRP("table %s%s%s", rc, self->table, self->table_path ? " at " : "", self->table_path ? self->table_path : "");
        }
        free(self->table);
        free(self->table_path);
        free(self);
    }
    return rc;
}

rc_t CC TableWriter_AddCursor(const TableWriter* cself, TableWriterColumn* cols, uint32_t col_qty, uint8_t* cursor_id)
{
    rc_t rc = 0;

    if( cself == NULL || cols == NULL || col_qty == 0 || cursor_id == NULL ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
        ALIGN_DBGERR(rc);
    } else {
        uint32_t i;
        TableWriter* self = (TableWriter*)cself;

        *cursor_id = TW_MAX_CURSORS;
        for(i = 0; i < TW_MAX_CURSORS; i++) {
            if( self->cursors[i].col_qty == 0 ) {
                self->cursors[i].col_qty = col_qty;
                self->cursors[i].cols = cols;
                self->cursors[i].cursor = NULL;
                self->cursors[i].rows = 0;
                *cursor_id = i;
                break;
            }
        }
        if( *cursor_id >= TW_MAX_CURSORS ) {
            rc = RC(rcAlign, rcType, rcOpening, rcConstraint, rcExhausted);
        }
    }
    if( rc != 0 ) {
        ALIGN_DBGERRP("table %s", rc, cself->table);
    } else {
        ALIGN_DBG("table %s added cursor %hu", cself->table, *cursor_id);
    }
    return rc;
}

rc_t CC TableWriter_CloseCursor(const TableWriter* cself, uint8_t cursor_id, uint64_t* rows)
{
    rc_t rc = 0;

    assert(cself != NULL);
    assert(cursor_id < TW_MAX_CURSORS);
    if (cself == NULL) abort();
    if (cursor_id >= TW_MAX_CURSORS) abort();

    if (cself->cursors[cursor_id].cursor == NULL)
        return 0;
    else {
        rc_t rc2 = 0;
        TableWriter* self = (TableWriter*)cself;
        uint64_t r = 0;

        if( rows == NULL ) {
            rows = &r;
        }
        self->curr = &self->cursors[cursor_id];
        rc = VCursorCommit(self->curr->cursor);
        *rows = cself->curr->rows;
        rc2 = VCursorRelease(self->curr->cursor);
        self->curr->cursor = NULL;
        rc = rc ? rc : rc2;
    }
    if( rc == 0 ) {
        ALIGN_DBG("table %s closed cursor %hu rows %ld", cself->table, cursor_id, *rows);
    } else {
        ALIGN_DBGERRP("table %s cursor %hu row %ld", rc, cself->table, cursor_id, *rows);
    }
    return rc;
}

rc_t CC TableWriter_Flush(const TableWriter *cself, const uint8_t cursor_id)
{
    rc_t rc = 0;
    
    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
        ALIGN_DBGERR(rc);
    }
    else if( cself->cursors[cursor_id].cursor != NULL ) {
        rc = VCursorFlushPage(cself->cursors[cursor_id].cursor);
        if( rc == 0 ) {
            ALIGN_DBG("table %s cursor %hu flushed", cself->table, cursor_id);
        } else {
            ALIGN_DBGERRP("table %s cursor %hu flushing", rc, cself->table, cursor_id);
        }
    }
    return rc;
}

rc_t CC TableWriter_OpenRow(const TableWriter* cself, int64_t* rowid, const uint8_t cursor_id)
{
    rc_t rc = 0;

    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
        ALIGN_DBGERR(rc);
    }
    else if( cself->cursors[cursor_id].cursor != NULL || (rc = TableWriter_OpenCursor(cself, cursor_id)) == 0 ) {
        TableWriter* self = (TableWriter*)cself;

        self->curr = &self->cursors[cursor_id];
        if( (rc = VCursorOpenRow(self->curr->cursor)) == 0 && rowid != NULL ) {
            rc = VCursorRowId(self->curr->cursor, rowid);
        }
        if( rc == 0 ) {
            ALIGN_DBG("table %s cursor %hu opened row %ld", self->table, cursor_id, self->curr->rows + 1);
        } else {
            ALIGN_DBGERRP("table %s cursor %hu row %ld", rc, self->table, cursor_id, self->curr->rows + 1);
        }
    }
    return rc;
}

rc_t CC TableWriter_OpenRowId(const TableWriter* cself, const int64_t rowid, const uint8_t cursor_id)
{
    rc_t rc = 0;
    
    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
        ALIGN_DBGERR(rc);
    } else if( cself->cursors[cursor_id].cursor != NULL || (rc = TableWriter_OpenCursor(cself, cursor_id)) == 0 ) {
        TableWriter* self = (TableWriter*)cself;

        self->curr = &self->cursors[cursor_id];
        /* loop through missed rows to assign default values */
        if( rowid > self->curr->rows + 1 ) {
            int64_t r;
            for(r = self->curr->rows + 1; rc == 0 && r < rowid; r++) {
                if( (rc = VCursorOpenRow(self->curr->cursor)) == 0 &&
                    (rc = VCursorCommitRow(self->curr->cursor)) == 0 ) {
                    ALIGN_DBG("table %s written default row for cursor %hu row %ld", self->table, cursor_id, r);
                    rc = VCursorCloseRow(self->curr->cursor);
                }
            }
        }
        if( rc == 0 /* && (rc = VCursorSetRowId(cself->curr->cursor, rowid)) == 0 */ ) {
            rc = VCursorOpenRow(cself->curr->cursor);
        }
        if( rc == 0 ) {
            if( rowid > self->curr->rows + 1) {
                self->curr->rows = rowid - 1;
            }
            ALIGN_DBG("table %s cursor %hu opened row %ld", self->table, cursor_id, rowid);
        } else {
            ALIGN_DBGERRP("table %s cursor %hu row %ld", rc, self->table, cursor_id, rowid);
        }
    }
    return rc;
}

rc_t CC TableWriter_GetNextRowId(const TableWriter* cself, int64_t* rowid, const uint8_t cursor_id)
{
    rc_t rc = 0;
    
    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
        ALIGN_DBGERR(rc);
    } else {
        *rowid = cself->cursors[cursor_id].rows + 1;
        ALIGN_DBG("table %s next rowid %li", cself->table, *rowid);
    }
    return rc;
}

rc_t CC TableWriter_ColumnDefault(const TableWriter* cself, const uint8_t cursor_id,
                                  const TableWriterColumn* column, const TableWriterData *data)
{
    rc_t rc = 0;
    if( cself == NULL || cursor_id >= TW_MAX_CURSORS || cself->cursors[cursor_id].col_qty == 0 ||
        column == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
    } else if( !(column->flags & ewcol_IsArray) && data->elements != 1 ) {
        rc = RC( rcAlign, rcType, rcWriting, rcData, rcTooLong);
        ALIGN_DBGERRP("column %s is not an array of values", rc, column->name);
    } else if( cself->cursors[cursor_id].cursor != NULL || (rc = TableWriter_OpenCursor(cself, cursor_id)) == 0 ) {
        if( column->idx != 0 ) {
            rc = VCursorDefault(cself->cursors[cursor_id].cursor, column->idx, column->element_bits, data->buffer, 0, data->elements);
        }
    }
    if( rc != 0 ) {
        ALIGN_DBGERRP("table %s column %s default value", rc, cself->table, column->name);
    } else {
        ALIGN_DBG("table %s column %s default value %lu elements", cself->table, column->name, data->elements);
    }
    return rc;
}

rc_t CC TableWriter_ColumnWrite(const TableWriter* cself,
    const TableWriterColumn* column, const TableWriterData *data)
{
    rc_t rc = 0;

    if( cself == NULL || column == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcInvalid);
    }
    else if( cself->curr == NULL || cself->curr->cursor == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcMode, rcInvalid);
    }
    else if( !(column->flags & ewcol_IsArray) && data->elements > 1 ) {
        rc = RC( rcAlign, rcType, rcWriting, rcData, rcTooLong);
        ALIGN_DBGERRP("column %s is not an array of values", rc, column->name);
    }
    else if (column->idx != 0) {
        const char platform[] = "PLATFORM";
        /* Do not try to write PLATFORM column when it is empty
          (e.g. cg-load writes PLATFORM defult) */
        if (data->buffer != NULL || data-> elements != 0 ||
            string_cmp(platform, sizeof platform - 1, column->name,
                string_measure(column->name, NULL), sizeof platform - 1) != 0)
        {
            rc = VCursorWrite(cself->curr->cursor, column->idx,
                column->element_bits, data->buffer, 0, data->elements);
        }
    }

    if( rc != 0 ) {
        ALIGN_DBGERRP("table %s column %s row %ld", rc, cself->table, column->name, cself->curr->rows + 1);
    }
    else {
        ALIGN_DBG("table %s column %s value %lu elements", cself->table, column->name, data->elements);
    }

    return rc;
}

rc_t CC TableWriter_CloseRow(const TableWriter* cself)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcType, rcClosing, rcSelf, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->curr == NULL || cself->curr->cursor == NULL ) {
        rc = RC( rcAlign, rcType, rcClosing, rcMode, rcInvalid);
    } else {
        if( (rc = VCursorCommitRow(cself->curr->cursor)) == 0 ) {
            rc = VCursorCloseRow(cself->curr->cursor);
        }
        if( rc == 0 ) {
            cself->curr->rows++;
        } else {
            ALIGN_DBGERRP("table %s row %ld", rc, cself->table, cself->curr->rows + 1);
        }
        ((TableWriter*)cself)->curr = NULL;
    }
    return rc;
}
