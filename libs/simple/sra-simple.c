/* ===========================================================================
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

#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <klib/ksort-macro.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/writer.h>

#include <sysalloc.h>

#include <os-native.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "sra-simple.h"
#include "cursor-helper.h"

/* TODO: generate errors */


#define COLUMN_VALUE_AS_I8(COL)  ((int8_t  const *)((COL).value))
#define COLUMN_VALUE_AS_I16(COL) ((int16_t const *)((COL).value))
#define COLUMN_VALUE_AS_I32(COL) ((int32_t const *)((COL).value))
#define COLUMN_VALUE_AS_I64(COL) ((int64_t const *)((COL).value))

#define COLUMN_VALUE_AS_U8(COL)  ((uint8_t  const *)((COL).value))
#define COLUMN_VALUE_AS_U16(COL) ((uint16_t const *)((COL).value))
#define COLUMN_VALUE_AS_U32(COL) ((uint32_t const *)((COL).value))
#define COLUMN_VALUE_AS_U64(COL) ((uint64_t const *)((COL).value))

#define READ_COLUMN(CURS, ROW, COL_ENTRY) (VCursorCellDataDirect(CURS, ROW, COL_ENTRY.id, NULL, &COL_ENTRY.value, NULL, &COL_ENTRY.elements))

enum eSequenceColumns {
    seq_PLATFORM = 0,
    seq_READ_TYPE,
    seq_READ_FILTER,
    seq_READ_LENGTH,
    seq_READ_START,
    seq_SPOT_GROUP,
    seq_READ,
    seq_QUALITY,
    seq_PRIMARY_ALIGNMENT_ID,
    seq_LAST
};

enum eAlignmentColumns {
    aln_SPOT_ID = 0,
    aln_PLATFORM,
    aln_SPOT_GROUP,
    aln_READ_NUMBER,
    aln_READ_LENGTH,
    aln_READ,
    aln_QUALITY,
    aln_REF_ID,
    aln_REF_POS,
    aln_REF_LEN,
    aln_HAS_MISMATCH,
    aln_HAS_REF_OFFSET,
    aln_REF_OFFSET,
    aln_MAPQ,
    aln_STRAND,
    aln_FLAGS,
    aln_MATE_REF_ID,
    aln_MATE_REF_POS,
    aln_MATE_STRAND,
    aln_TEMPLATE_LEN,
    aln_LAST
};

enum eSRAFileReaderTables {
    sfrt_SEQUENCE,
    sfrt_REFERENCE,
    sfrt_PRIMARY,
    sfrt_SECONDARY,
    sfrt_LAST
};

enum eCigOp {
    co_M = 0,
    co_I,
    co_D,
    co_N,
    co_S,
    co_H,
    co_P,
    co_E,
    co_X,
    co_B,
};

enum eSRAReferenceColumns {
    ref_PID = 0,
    ref_SID,
    ref_LOOKBACK,
    ref_OVERLAP_MAX,
    ref_LAST
};

typedef struct TableRowRange_s {
    int64_t  start;
    uint64_t count;
} TableRowRange;

typedef struct ReferenceEntry ReferenceEntry;
struct ReferenceEntry {
    char const *name;
    char const *accn;
    int64_t beginRow;       /* first row in table where this reference starts */
    uint32_t length;        /* length in bases of the reference */
    unsigned id;
    unsigned max_seq_len;   /* multiplier to convert from row # to base # */
    bool circular;
    bool local;
};

#define ReferenceEntryRows(ENTRY) (((ENTRY)->length + (ENTRY)->max_seq_len - 1) / (ENTRY)->max_seq_len)

struct c_SRAFileReader {
    char *accn;
    char *path;
    VTable const *tables[sfrt_LAST];
    TableRowRange rows[sfrt_LAST];
};

struct c_SRASpotEnumerator {
    SRAEnumerator e;
    column_bind_t cols[seq_LAST];
};

struct c_SRAReferenceList {
    unsigned *byName;
    unsigned *byAccn;
    ReferenceEntry **refs;
    unsigned numRefs;
};

struct c_SRAAlignmentEnumerator {
    struct SRAAlignmentEnumerator_vt const *vt;
    struct SRASimpleAlignmentEnumerator *current;
};

typedef struct SRAAlignmentEnumerator_vt SRAAlignmentEnumerator_vt;
struct SRAAlignmentEnumerator_vt {
    char const *className;
    void (*Free)(c_SRAAlignmentEnumerator *self);
    bool (*Next)(c_SRAAlignmentEnumerator *self);
    bool (*Seek)(c_SRAAlignmentEnumerator *self,
                 c_SRAReferenceList const *refList,
                 char const name[], int32_t position,
                 bool endingAfter);
};

typedef struct SRASimpleAlignmentEnumerator SRASimpleAlignmentEnumerator;
struct SRASimpleAlignmentEnumerator {
    SRAEnumerator e;
    bool primary;
    column_bind_t cols[aln_LAST];
};

typedef struct SRASingleAlignmentEnumerator SRASingleAlignmentEnumerator;
struct SRASingleAlignmentEnumerator {
    c_SRAAlignmentEnumerator dad;
    SRASimpleAlignmentEnumerator inner;
};

typedef struct SRAUnsortedAlignmentEnumerator SRAUnsortedAlignmentEnumerator;
struct SRAUnsortedAlignmentEnumerator {
    c_SRAAlignmentEnumerator dad;
    SRASimpleAlignmentEnumerator inner[2];
};

typedef struct alignment_s alignment_t;
struct alignment_s {
    int64_t rowid;
    int32_t refPos;
    int32_t refLen;
};

typedef struct alignment_list_s alignment_list_t;
struct alignment_list_s {
    alignment_t *alignments;
    unsigned current;
    unsigned count;
    unsigned allocated;
};

typedef struct SRAOrderedAlignmentEnumerator SRAOrderedAlignmentEnumerator;
struct SRAOrderedAlignmentEnumerator {
    c_SRAAlignmentEnumerator dad;
    SRAEnumerator ref;
    alignment_list_t lists[2];
    SRASimpleAlignmentEnumerator inner[2];
    column_bind_t cols[ref_LAST];
};

static TableRowRange GetRowRange(VTable const *const tbl)
{
    TableRowRange rslt = { 0, 0 };
    
    if (tbl) {
        KNamelist *names;
        rc_t rc = VTableListReadableColumns(tbl, &names);
        
        if (rc == 0) {
            VCursor const *curs;
            
            rc = VTableCreateCursorRead(tbl, &curs);
            if (rc == 0) {
                uint32_t cols = 0;
                unsigned j;
                column_bind_t *columns;
                SRAEnumerator e;
                
                KNamelistCount(names, &cols);
                columns = calloc(cols, sizeof(columns[0]));
                if (columns) {
                    for (j = 0; j != (unsigned)cols; ++j) {
                        rc = KNamelistGet(names, j, &columns[j].name);
                        if (rc)
                            columns[j].disabled = true;
                        else
                            columns[j].optional = true;
                    }
                    SRAEnumeratorInit(&e, curs, 0, 0);
                    rc = SRAEnumeratorOpen(&e, true, cols, columns);
                    if (rc == 0) {
                        for (j = 0; j != (unsigned)cols; ++j) {
                            if (columns[j].disabled == false) {
                                TableRowRange tmp = { 0, 0 };
                                
                                rc = VCursorIdRange(curs, columns[j].id, &tmp.start, &tmp.count);
                                if (rc == 0 && tmp.start && tmp.count) {
                                    if (rslt.count == 0) {
                                        rslt = tmp;
                                    }
                                    else {
                                        if (rslt.count < tmp.count)
                                            rslt.count = tmp.count;
                                        if (rslt.start > tmp.start)
                                            rslt.start = tmp.start;
                                    }
                                }
                            }
                        }
                    }
                    free(columns);
                }
                SRAEnumeratorClose(&e);
            }
            KNamelistRelease(names);
        }
    }
    return rslt;
}

static void SRAFileReaderGetRowRanges(c_SRAFileReader *const self)
{
    unsigned i;
    
    for (i = 0; i != sfrt_LAST; ++i)
        self->rows[i] = GetRowRange(self->tables[i]);
}

static
void ExtractAccn(char accn[], char const path[], size_t length)
{
    unsigned i;
    
    if (   length > 4
        &&         path[length - 4]  == '.'
        && tolower(path[length - 3]) == 's'
        && tolower(path[length - 2]) == 'r'
        && tolower(path[length - 1]) == 'a')
    {
        length -= 4;
        if (   length > 5
            &&         path[length - 5]  == '.'
            && tolower(path[length - 4]) == 'l'
            && tolower(path[length - 3]) == 'i'
            && tolower(path[length - 2]) == 't'
            && tolower(path[length - 1]) == 'e')
        {
            length -= 5;
        }
    }
    else if (   length > 5
             &&         path[length - 5]  == '.'
             && tolower(path[length - 4]) == 'c'
             && tolower(path[length - 3]) == 's'
             && tolower(path[length - 2]) == 'r'
             && tolower(path[length - 1]) == 'a')
    {
        length -= 5;
    }
    for (i = 0; ; ++i) {
        size_t const at = length - 1 - i;
        int const ch = path[at];
        
        if (ch == '/') {
            memcpy(accn, path + at + 1, i);
            accn[i] = '\0';
            break;
        }
        else if (at == 0) {
            memcpy(accn, path, length);
            accn[length] = '\0';
            break;
        }
    }
}

void c_SRAFileReaderSetDbgFlag(char const flag[])
{
    static bool inited = false;
    
    if (!inited) {
        KWrtInit("sra-simple.jni", 0);
        KLogLibHandlerSetStdErr();
        KStsLibHandlerSetStdOut();
        (void)KLogInit();
        (void)KDbgInit();
        inited = true;
    }
    (void)KDbgSetString(flag);
}

c_SRAFileReader *c_SRAFileReaderCreate(char const path[], char const **const errmsg)
{
    c_SRAFileReader *rslt = NULL;
    size_t const length = path ? strlen(path) : 0;
    
    *errmsg = "invalid name";
    if (length) {
        *errmsg = "out of memory";
        rslt = malloc(sizeof(*rslt) + 2 + 2 * length);
        if (rslt) {
            VDBManager const *mgr;
            rc_t rc = VDBManagerMakeRead(&mgr, NULL);
            
            *errmsg = "internal vdb error";
            if (rc == 0) {
                VDatabase const *db;
                
                memset(rslt, 0, sizeof(*rslt));
                rslt->path = (char *)(rslt + 1);
                rslt->accn = rslt->path + length + 1;
                memcpy(rslt->path, path, length + 1);
                rslt->accn[0] = '\0';
                
                *errmsg = NULL;
                rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%s", path);
                if (rc == 0) {
                    VDatabaseOpenTableRead(db, rslt->tables + sfrt_SEQUENCE,  "SEQUENCE");
                    VDatabaseOpenTableRead(db, rslt->tables + sfrt_REFERENCE, "REFERENCE");
                    VDatabaseOpenTableRead(db, rslt->tables + sfrt_PRIMARY,   "PRIMARY_ALIGNMENT");
                    VDatabaseOpenTableRead(db, rslt->tables + sfrt_SECONDARY, "SECONDARY_ALIGNMENT");
                    VDatabaseRelease(db);
                    
                    if (rslt->tables[sfrt_SEQUENCE] == NULL || rslt->tables[sfrt_REFERENCE] == NULL)
                        *errmsg = "accession contains no useable data";

                    ExtractAccn(rslt->accn, path, length);
                }
                else {
                    rc = VDBManagerOpenTableRead(mgr, rslt->tables + sfrt_SEQUENCE, NULL, "%s", path);
                    if (rc == 0)
                        ExtractAccn(rslt->accn, path, length);
                    else if (GetRCState(rc) == rcNotFound)
                        *errmsg = "not found";
                    else
                        *errmsg = "couldn't open accession";
                }
                VDBManagerRelease(mgr);
                SRAFileReaderGetRowRanges(rslt);
            }
        }
    }
    return rslt;
}

void c_SRAFileReaderFree(c_SRAFileReader *const self)
{
    if (self) {
        VTableRelease(self->tables[0]);
        VTableRelease(self->tables[1]);
        VTableRelease(self->tables[2]);
        VTableRelease(self->tables[3]);
        
        free(self);
    }
}

char const *c_SRAFileReaderAccession(c_SRAFileReader const *const self)
{
    return self->accn;
}

static
void c_SRASpotEnumeratorInit(c_SRASpotEnumerator *const self,
                             VTable const *const tbl,
                             int64_t const start,
                             uint64_t const count)
{
    VCursor const *curs;
    rc_t rc = VTableCreateCursorRead(tbl, &curs);
    
    if (rc == 0) {
        static const column_bind_t columns[] = {
            COLUMN_BIND_INIT_2("U8",             "PLATFORM", 0, 0),
            COLUMN_BIND_INIT_2("U8",             "READ_TYPE", 0, 0),
            COLUMN_BIND_INIT_2("U8",             "READ_FILTER", 0, 0),
            COLUMN_BIND_INIT_2("U32",            "READ_LEN", 0, 0),
            COLUMN_BIND_INIT_2("I32",            "READ_START", 0, 0),
            COLUMN_BIND_INIT_2("utf8",           "SPOT_GROUP", 0, 0),
            COLUMN_BIND_INIT_2("INSDC:dna:text", "READ", 0, 0),
            COLUMN_BIND_INIT_2("U8",             "QUALITY", 0, 0),
            COLUMN_BIND_INIT_2("I64",            "PRIMARY_ALIGNMENT_ID", true, true)
        };
        
        memcpy(self->cols, columns, sizeof(self->cols));
        SRAEnumeratorInit(&self->e, curs, start, count);
        
        rc = SRAEnumeratorOpen(&self->e, false, seq_LAST, self->cols);
        if (rc) {
            VCursorRelease(curs);
            memset(self, 0, sizeof(*self));
        }
    }
}

c_SRASpotEnumerator *c_SRAFileReaderCreateSpotEnumerator(c_SRAFileReader *const self, char const **const errmsg)
{
    c_SRASpotEnumerator *const rslt = calloc(1, sizeof(*rslt));
    
    if (rslt && self) {
        if (self->tables[sfrt_SEQUENCE] == NULL)
            *errmsg = "no spot data in this accession";
        else
            c_SRASpotEnumeratorInit(rslt,
                                    self->tables[sfrt_SEQUENCE],
                                    self->rows[sfrt_SEQUENCE].start,
                                    self->rows[sfrt_SEQUENCE].count);
    }
    return rslt;
}

void c_SRASpotEnumeratorFree(c_SRASpotEnumerator *const self)
{
    if (self) {
        SRAEnumeratorClose(&self->e);
        free(self);
    }
}

bool c_SRASpotEnumeratorNext(c_SRASpotEnumerator *const self)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        SRAEnumeratorNext(&self->e);
        return true;
    }
    return false;
}

/* MARK: c_SRASpotEnumerator external accessors */

/*
char const *c_SRASpotEnumeratorAccession(c_SRASpotEnumerator const *const self)
{
    return self->accn;
}
*/

int64_t c_SRASpotEnumeratorReadID(c_SRASpotEnumerator const *const self)
{
    if (self && SRAEnumeratorCanRead(&self->e))
        return SRAEnumeratorCurrent(&self->e);
    return 0;
}

#define SpotEnumeratorRead(COL) SRAEnumeratorReadColumn(&self->e, (column_bind_t *)&self->cols[COL])

int32_t c_SRASpotEnumeratorPlatform(c_SRASpotEnumerator const *const self)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = SpotEnumeratorRead(seq_PLATFORM);
        if (rc == 0)
            return COLUMN_VALUE_AS_U8(self->cols[seq_PLATFORM])[0];
    }
    return 0;
}

char const *c_SRASpotEnumeratorReadGroup(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e) && length) {
        rc_t const rc = SpotEnumeratorRead(seq_SPOT_GROUP);
        if (rc == 0) {
            *length = self->cols[seq_SPOT_GROUP].elements;
            return (char const *)self->cols[seq_SPOT_GROUP].value;
        }
    }
    return NULL;
}

uint32_t c_SRASpotEnumeratorNumberOfReads(c_SRASpotEnumerator const *const self)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = SpotEnumeratorRead(seq_READ_LENGTH);
        if (rc == 0)
            return self->cols[seq_READ_LENGTH].elements;
    }
    return 0;
}

uint8_t const *c_SRASpotEnumeratorReadTypeArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e) && length) {
        rc_t const rc = SpotEnumeratorRead(seq_READ_TYPE);
        if (rc == 0) {
            *length = self->cols[seq_READ_TYPE].elements;
            return COLUMN_VALUE_AS_U8(self->cols[seq_READ_TYPE]);
        }
    }
    return NULL;
}

uint8_t const *c_SRASpotEnumeratorReadFilterArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e) && length) {
        rc_t const rc = SpotEnumeratorRead(seq_READ_FILTER);
        if (rc == 0) {
            *length = self->cols[seq_READ_FILTER].elements;
            return COLUMN_VALUE_AS_U8(self->cols[seq_READ_FILTER]);
        }
    }
    return NULL;
}

uint32_t const *c_SRASpotEnumeratorReadLengthArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e) && length) {
        rc_t const rc = SpotEnumeratorRead(seq_READ_LENGTH);
        if (rc == 0) {
            *length = self->cols[seq_READ_LENGTH].elements;
            return COLUMN_VALUE_AS_U32(self->cols[seq_READ_LENGTH]);
        }
    }
    return NULL;
}

int32_t const *c_SRASpotEnumeratorReadStartArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e) && length) {
        rc_t const rc = SpotEnumeratorRead(seq_READ_START);
        if (rc == 0) {
            *length = self->cols[seq_READ_START].elements;
            return COLUMN_VALUE_AS_I32(self->cols[seq_READ_START]);
        }
    }
    return NULL;
}

char const *c_SRASpotEnumeratorReadSequenceArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = SpotEnumeratorRead(seq_READ);
        if (rc == 0) {
            if (length)
                *length = self->cols[seq_READ].elements;
            return (char const *)self->cols[seq_READ].value;
        }
    }
    return NULL;
}

uint8_t const *c_SRASpotEnumeratorReadQualityArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = SpotEnumeratorRead(seq_QUALITY);
        if (rc == 0) {
            if (length)
                *length = self->cols[seq_QUALITY].elements;
            return COLUMN_VALUE_AS_U8(self->cols[seq_QUALITY]);
        }
    }
    return NULL;
}

int64_t const *c_SRASpotEnumeratorAlignIDArray(c_SRASpotEnumerator const *const self, uint32_t *const length)
{
    if (self && SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = SpotEnumeratorRead(seq_PRIMARY_ALIGNMENT_ID);
        if (rc == 0) {
            if (length)
                *length = self->cols[seq_PRIMARY_ALIGNMENT_ID].elements;
            return COLUMN_VALUE_AS_I64(self->cols[seq_PRIMARY_ALIGNMENT_ID]);
        }
    }
    return NULL;
}

#define REFS_CMP(FLD, A, B) strcmp(self->refs[*(unsigned const *)(A)]->FLD, self->refs[*(unsigned const *)(B)]->FLD)
static
void SortRefEntriesByName(c_SRAReferenceList *const self)
{
#define SWAP(A, B, D1, D2) KSORT_TSWAP(unsigned, A, B)
#define CMP(A, B) REFS_CMP(name, A, B)
    KSORT(self->byName, self->numRefs, sizeof(self->byName[0]), 0, sizeof(self->byName[0]));
#undef SWAP
#undef CMP
}

static
void SortRefEntriesByAccn(c_SRAReferenceList *const self)
{
#define SWAP(A, B, D1, D2) KSORT_TSWAP(unsigned, A, B)
#define CMP(A, B) REFS_CMP(accn, A, B)
    KSORT(self->byAccn, self->numRefs, sizeof(self->byAccn[0]), 0, sizeof(self->byAccn[0]));
#undef SWAP
#undef CMP
}

void c_SRAReferenceListFree(c_SRAReferenceList *const self)
{
    unsigned i;
    
    for (i = 0; i != self->numRefs; ++i)
        free((void *)self->refs[i]);

    free(self->refs);
    free(self);
}

c_SRAReferenceList const *c_SRAFileReaderGetReferenceList(c_SRAFileReader *const self, char const **const errmsg)
{
    c_SRAReferenceList *rslt = NULL;
    
    if (self) {
        rslt = calloc(1, sizeof(*rslt));
        
        if (rslt && self->tables[sfrt_REFERENCE]) {
            VCursor const *curs;
            rc_t rc = VTableCreateCursorRead(self->tables[sfrt_REFERENCE], &curs);
            
            *errmsg = "internal vdb error";
            if (rc == 0) {
                enum eRefListColumns {
                    rlc_NAME,
                    rlc_SEQ_ID,
                    rlc_CIRCULAR,
                    rlc_MAX_SEQ_LEN,
                    rlc_SEQ_LEN,
                    rlc_NAME_RANGE
                };
                column_bind_t cols[] = {
                    COLUMN_BIND_INIT_2("utf8", "NAME", 0, 0),
                    COLUMN_BIND_INIT_2("utf8", "SEQ_ID", 0, 0),
                    COLUMN_BIND_INIT_2("bool", "CIRCULAR", 0, 0),
                    COLUMN_BIND_INIT_2("U32",  "MAX_SEQ_LEN", 0, 0),
                    COLUMN_BIND_INIT_2("U32",  "SEQ_LEN", 0, 0),
                    COLUMN_BIND_INIT_1("NAME_RANGE", 0, 0)
                };
                SRAEnumerator e;
                struct VCursorParams const *const params = (struct VCursorParams const *)curs;

                SRAEnumeratorInit(&e, curs, self->rows[sfrt_REFERENCE].start, self->rows[sfrt_REFERENCE].count);
                *errmsg = "required columns not found";
                rc = SRAEnumeratorOpen(&e, false, sizeof(cols)/sizeof(cols[0]), cols);
                if (rc == 0) {
                    while (!SRAEnumeratorIsEOF(&e)) {
                        struct {
                            int64_t first;
                            int64_t last;
                        } range;
                        
                        *errmsg = "error reading table";
                        rc = SRAEnumeratorReadColumn(&e, &cols[rlc_NAME]); if (rc) break;
                        
                        *errmsg = "error reading index";
                        rc = VCursorParamsSet(params, "QUERY_SEQ_NAME", "%.*s",
                                              cols[rlc_NAME].elements, cols[rlc_NAME].value);
                        if (rc) break;
                        
                        rc = SRAEnumeratorReadColumn(&e, &cols[rlc_NAME_RANGE]); if (rc) break;
                        memcpy(&range, cols[rlc_NAME_RANGE].value, sizeof(range));
                        
                        SRAEnumeratorSetCurrent(&e, range.last);
                        ++rslt->numRefs;
                        SRAEnumeratorNext(&e);
                        *errmsg = NULL;
                    }
                    *errmsg = "not enough memory to load reference information";
                    rslt->refs = calloc(rslt->numRefs, sizeof(rslt->refs[0]) + 2 * sizeof(rslt->byName[0]));
                    if (rslt->refs) {
                        unsigned i;
                        
                        rslt->byName = (unsigned *)(rslt->refs + rslt->numRefs);
                        rslt->byAccn = rslt->byName + rslt->numRefs;
                        
                        SRAEnumeratorSetCurrent(&e, self->rows[sfrt_REFERENCE].start);
                        for (i = 0; i != rslt->numRefs; ++i) {
                            struct {
                                int64_t first;
                                int64_t last;
                            } range;
                            
                            rslt->byName[i] = rslt->byAccn[i] = i;
                            
                            *errmsg = "error reading table";
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_NAME]); if (rc) break;
                            
                            *errmsg = "error reading index";
                            rc = VCursorParamsSet(params, "QUERY_SEQ_NAME", "%.*s",
                                                  cols[rlc_NAME].elements, cols[rlc_NAME].value);
                            if (rc) break;
                            
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_NAME_RANGE]); if (rc) break;
                            memcpy(&range, cols[rlc_NAME_RANGE].value, sizeof(range));
                            
                            *errmsg = "error reading table";
                            SRAEnumeratorSetCurrent(&e, range.last);
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_SEQ_ID]); if (rc) break;
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_CIRCULAR]); if (rc) break;
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_MAX_SEQ_LEN]); if (rc) break;
                            rc = SRAEnumeratorReadColumn(&e, &cols[rlc_SEQ_LEN]); if (rc) break;
                            *errmsg = "not enough memory to load reference information";
                            rslt->refs[i] = malloc(sizeof(ReferenceEntry) + cols[rlc_NAME].elements + 1 + cols[rlc_SEQ_ID].elements + 1);
                            if (rslt->refs[i]) {
                                ReferenceEntry *const entry = rslt->refs[i];
                                unsigned const last_seq_len = COLUMN_VALUE_AS_U32(cols[rlc_SEQ_LEN])[0];
                                unsigned const max_seq_len = COLUMN_VALUE_AS_U32(cols[rlc_MAX_SEQ_LEN])[0];
                                unsigned const row_count = (unsigned)(range.last + 1 - range.first);
                                unsigned const base_count = (row_count - 1) * max_seq_len + last_seq_len;
                                bool const circular = ((bool const *)cols[rlc_CIRCULAR].value)[0];
                                char *const name = (char *)(entry + 1);
                                unsigned const namelen = cols[rlc_NAME].elements;
                                char *const accn = name + namelen + 1;
                                unsigned const accnlen = cols[rlc_SEQ_ID].elements;
                                
                                memcpy(name, cols[rlc_NAME].value, namelen); name[namelen] = '\0';
                                memcpy(accn, cols[rlc_SEQ_ID].value, accnlen); accn[accnlen] = '\0';
                                
                                memset(entry, 0, sizeof(*entry));
                                entry->name = name;
                                entry->accn = accn;
                                entry->beginRow = range.first;
                                entry->length = base_count;
                                entry->id = i;
                                entry->max_seq_len = max_seq_len;
                                entry->circular = circular;

                                SRAEnumeratorNext(&e);
                                *errmsg = NULL;
                            }
                            else {
                                rc = RC(rcSRA, rcName, rcAllocating, rcMemory, rcInsufficient);
                                break;
                            }
                        }
                        if (rc == 0) {
                            SortRefEntriesByName(rslt);
                            SortRefEntriesByAccn(rslt);
                        }
                    }
                    else
                        rc = RC(rcSRA, rcName, rcAllocating, rcMemory, rcInsufficient);
                }
                SRAEnumeratorClose(&e);
            }
        }
    }
    return rslt;
}

uint32_t c_SRAReferenceListGetCount(c_SRAReferenceList const *const self)
{
    if (self)
        return self->numRefs;
    return 0;
}

int32_t c_SRAReferenceListGetIndexForName(c_SRAReferenceList const *const self, char const name[])
{
    if (self) {
        unsigned f = 0;
        unsigned e = self->numRefs;
        
        while (f < e) {
            unsigned const m = (f + e) / 2;
            unsigned const M = self->byName[m];
            int const diff = strcmp(name, self->refs[M]->name);
            
            if (diff == 0)
                return M;
            if (diff < 0)
                e = m;
            else
                f = m + 1;
        }
    }
    return -1;
}

int32_t c_SRAReferenceListGetIndexForRowID(c_SRAReferenceList const *const self, int64_t const rowid)
{
    if (self) {
        unsigned f = 0;
        unsigned e = self->numRefs;
        
        while (f < e) {
            unsigned const m = (f + e) / 2;
            int64_t begRow = self->refs[m]->beginRow;
            int64_t endRow = begRow + ReferenceEntryRows(self->refs[m]);
            if (rowid >= endRow)
                f = m + 1;
            else if (begRow > rowid)
                e = m;
            else
                return m;
        }
    }
    return -1;
}

char const *c_SRAReferenceListGetName      (c_SRAReferenceList const *const self, uint32_t const i)
{
    if (self && i < self->numRefs)
        return self->refs[i]->name;
    return NULL;
}

char const *c_SRAReferenceListGetAccession (c_SRAReferenceList const *const self, uint32_t const i)
{
    if (self && i < self->numRefs)
        return self->refs[i]->accn;
    return NULL;
}

uint32_t c_SRAReferenceListGetBaseCount (c_SRAReferenceList const *const self, uint32_t const i)
{
    if (self && i < self->numRefs)
        return self->refs[i]->length;
    return 0;
}

bool c_SRAReferenceListGetIsCircular(c_SRAReferenceList const *const self, uint32_t const i)
{
    if (self && i < self->numRefs)
        return self->refs[i]->circular;
    return false;
}

void c_SRAAlignmentEnumeratorFree(c_SRAAlignmentEnumerator *const self)
{
    if (self && self->vt && self->vt->Free) {
        self->vt->Free(self);
        free(self);
    }
}

bool c_SRAAlignmentEnumeratorNext(c_SRAAlignmentEnumerator *const self)
{
    if (self && self->vt && self->vt->Next)
        return self->vt->Next(self);
    return false;
}

bool c_SRAAlignmentEnumeratorSeek(c_SRAAlignmentEnumerator *const self,
                                  c_SRAReferenceList const *const refList,
                                  char const name[], int32_t const position,
                                  bool const endingAfter)
{
    if (self && self->vt && self->vt->Seek && refList && name && position >= 0)
        return self->vt->Seek(self, refList, name, position, endingAfter);
    return false;
}

#define AlignmentEnumeratorRead(COL) SRAEnumeratorReadColumn(&self->e, (column_bind_t *)&self->cols[COL])

#define SRAAlignmentEnumeratorInnerAccessorDef(TYPE, FUNC, COL, DEFAULT) TYPE SRASimpleAlignmentEnumerator ## FUNC(SRASimpleAlignmentEnumerator *const self) \
{   if (SRAEnumeratorCanRead(&self->e)) \
        { rc_t const rc = AlignmentEnumeratorRead(COL); if (rc == 0) return ((TYPE const *)(self->cols[COL].value))[0]; }\
    return DEFAULT; }

#define SRAAlignmentEnumeratorInnerAccesLenDef(TYPE, FUNC, COL, DEFAULT) TYPE SRASimpleAlignmentEnumerator ## FUNC(SRASimpleAlignmentEnumerator *const self, uint32_t *const length) \
{ if (SRAEnumeratorCanRead(&self->e)) { \
    rc_t const rc = AlignmentEnumeratorRead(COL); if (rc == 0) { \
        if (length) *length = self->cols[COL].elements; \
        return self->cols[COL].value; }} \
    return DEFAULT; }

#define SRAAlignmentEnumeratorInnerAccessor(FUNC, DEFAULT) ((self && self->current) ? SRASimpleAlignmentEnumerator ## FUNC(self->current) : DEFAULT)
#define SRAAlignmentEnumeratorInnerAccesLen(FUNC, DEFAULT) ((self && self->current) ? SRASimpleAlignmentEnumerator ## FUNC(self->current, length) : DEFAULT)

/* MARK: c_SRAAlignmentEnumerator inner accessors */

static SRAAlignmentEnumeratorInnerAccessorDef(int64_t, SpotID, aln_SPOT_ID, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, Platform, aln_PLATFORM, 0)
static SRAAlignmentEnumeratorInnerAccesLenDef(char const *, ReadGroup, aln_SPOT_GROUP, NULL)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, ReadNumber, aln_READ_NUMBER, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, ReadLength, aln_READ_LENGTH, 0)

static
char const *SRASimpleAlignmentEnumeratorReadSequence(SRASimpleAlignmentEnumerator *const self, uint32_t *length)
{
    if (SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = AlignmentEnumeratorRead(aln_READ);
        if (rc == 0) {
            *length = self->cols[aln_READ].elements;
            return (char const *)self->cols[aln_READ].value;
        }
    }
    return NULL;
}

static
uint8_t const *SRASimpleAlignmentEnumeratorReadQuality(SRASimpleAlignmentEnumerator *const self, uint32_t *length)
{
    if (SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = AlignmentEnumeratorRead(aln_QUALITY);
        if (rc == 0) {
            *length = self->cols[aln_QUALITY].elements;
            return (uint8_t const *)self->cols[aln_QUALITY].value;
        }
    }
    return NULL;
}

static SRAAlignmentEnumeratorInnerAccessorDef(int64_t, RefRowID, aln_REF_ID, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, AlignmentStart, aln_REF_POS, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, ReferenceLength, aln_REF_LEN, 0)
static SRAAlignmentEnumeratorInnerAccesLenDef(int32_t const *, RefOffset, aln_REF_OFFSET, NULL)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, MappingQuality, aln_MAPQ, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(uint32_t, Flags, aln_FLAGS, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(bool, Strand, aln_STRAND, false)
static SRAAlignmentEnumeratorInnerAccessorDef(bool, MateRefRowID, aln_MATE_REF_ID, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, MateAlignmentStart, aln_MATE_REF_POS, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(int32_t, TemplateLength, aln_TEMPLATE_LEN, 0)
static SRAAlignmentEnumeratorInnerAccessorDef(bool, MateStrand, aln_MATE_STRAND, false)

static
bool const *SRASimpleAlignmentEnumeratorHasMismatch(SRASimpleAlignmentEnumerator *const self)
{
    if (SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = AlignmentEnumeratorRead(aln_HAS_MISMATCH);
        if (rc == 0)
            return (bool const *)self->cols[aln_HAS_MISMATCH].value;
    }
    return NULL;
}

static
bool const *SRASimpleAlignmentEnumeratorHasRefOffset(SRASimpleAlignmentEnumerator *const self)
{
    if (SRAEnumeratorCanRead(&self->e)) {
        rc_t const rc = AlignmentEnumeratorRead(aln_HAS_REF_OFFSET);
        if (rc == 0)
            return (bool const *)self->cols[aln_HAS_REF_OFFSET].value;
    }
    return NULL;
}

static unsigned GenerateBAMCigar(int const style,
                                 unsigned const readLen,
                                 unsigned const refLen,
                                 uint32_t rslt[],
                                 bool const hmm[readLen],
                                 bool const hro[readLen],
                                 unsigned const offsets,
                                 int32_t const offset[offsets])
{
    int ri;
    unsigned si = 0;
    unsigned di;
    unsigned cur_off = 0;
    unsigned op_len = 0;
    int op_code = 0;
    int const opM = style == 1 ? co_E : co_M;
    int const opX = style == 1 ? co_X : co_M;
    
    for (di = 0; si < readLen && ri < (int)refLen; ) {
        if (hro[si]) {
            int offs;
            
            if (op_len > 0) {
                rslt[di++] = (op_len << 4) | op_code;
                op_len = 0;
            }
            if (cur_off >= offsets)
                return 0;
            offs = offset[cur_off++];
            if (offs < 0) {
                unsigned j;
                
                for (j = 1; j < -offs && (si + j) < readLen; ) {
                    if (hro[si + j]) {
                        rslt[di++] = (j << 4) | (si ? co_I : co_S);
                        offs += j;
                        si += j;
                        j = 1;
                    }
                    else
                        ++j;
                }
                if (offs < 0) {
                    rslt[di++] = ((-offs) << 4) | (si ? co_I : co_S);
                    si += -offs;
                }
                continue;
            }
            else if (offs > 0) {
                rslt[di++] = (offs << 4) | co_D;
                ri += offs;
            }
            else
                return 0;
        }
        if (ri < (int)refLen) {
            int const op_nxt = hmm[si] ? opX : opM;
            
            if (op_len == 0 || op_code == op_nxt)
                ++op_len;
            else {
                rslt[di++] = (op_len << 4) | op_code;
                op_len = 1;
            }
            op_code = op_nxt;
        }
        else
            break;
        ++si;
        ++ri;
    }
    rslt[di++] = (op_len << 4) | op_code;
    if (si < readLen) {
        op_len = readLen - si;
        if (cur_off + 1 < offsets)
            op_code = co_I;
        else
            op_code = co_S;
        rslt[di++] = (op_len << 4) | op_code;
    }
    return di;
}

static unsigned GenerateSimpleBAMCigar(unsigned const readLen,
				       unsigned const refLen,
				       uint32_t rslt[],
				       bool const hro[readLen],
				       unsigned const offsets,
				       int32_t const offset[offsets])
{
    int ri;
    unsigned si = 0;
    unsigned di;
    unsigned cur_off = 0;
    unsigned op_len = 0;
    int op_code = 0;
    
    for (di = 0; si < readLen && ri < (int)refLen; ) {
        if (hro[si]) {
            int offs;
            
            if (op_len > 0) {
                rslt[di++] = (op_len << 4) | op_code;
                op_len = 0;
            }
            if (cur_off >= offsets)
                return 0;
            offs = offset[cur_off++];
            if (offs < 0) {
                unsigned j;
                
                for (j = 1; j < -offs && (si + j) < readLen; ) {
                    if (hro[si + j]) {
                        rslt[di++] = (j << 4) | co_I;
                        offs += j;
                        si += j;
                        j = 1;
                    }
                    else
                        ++j;
                }
                if (offs < 0) {
                    rslt[di++] = ((-offs) << 4) | co_I;
                    si += -offs;
                }
                continue;
            }
            else if (offs > 0) {
                rslt[di++] = (offs << 4) | co_D;
                ri += offs;
            }
            else
                return 0;
        }
        if (ri < (int)refLen) {
            if (op_len == 0 || op_code == 0)
                ++op_len;
            else {
                rslt[di++] = (op_len << 4) | op_code;
                op_len = 1;
		op_code = 0;
            }
        }
        else
            break;
        ++si;
        ++ri;
    }
    rslt[di++] = (op_len << 4) | op_code;
    if (si < readLen) {
        op_len = readLen - si;
        rslt[di++] = (op_len << 4) | co_I;
    }
    return di;
}

static void CondSetOpCode(uint32_t *const pvalue, int const oldCode, int const newCode)
{
    uint32_t const value = *pvalue;
    int const opCode = value & 0xF;

    if (opCode == oldCode)
        *pvalue = value ^ oldCode ^ newCode;
}

static
uint32_t const *SRASimpleAlignmentEnumeratorBAMCigar(SRASimpleAlignmentEnumerator *const self, unsigned *const length)
{
    unsigned const readLen = SRASimpleAlignmentEnumeratorReadLength(self);
    unsigned const refLen = SRASimpleAlignmentEnumeratorReferenceLength(self);
    bool const *hro = SRASimpleAlignmentEnumeratorHasRefOffset(self);
    uint32_t offsets;
    int32_t const *offset = SRASimpleAlignmentEnumeratorRefOffset(self, &offsets);
    uint32_t *rslt = NULL;
    unsigned actlen = 0;
    
    if (readLen == 0 && refLen > 0) {
        rslt = malloc(2 * sizeof(rslt[0]));
        if (rslt) {
            rslt[0] = (refLen << 4) | co_D;
            rslt[1] = 0;
        }
    }
    else if (hro && offset) {
        rslt = malloc((2 * offsets + 1) * sizeof(rslt[0]));
        if (rslt) {
            actlen = GenerateSimpleBAMCigar(readLen, refLen, rslt, hro, offsets, offset);
            if (actlen) {
                CondSetOpCode(&rslt[0], co_I, co_S);
                CondSetOpCode(&rslt[actlen-1], co_I, co_S);
            }
            else {
                free(rslt);
                rslt = NULL;
            }
        }
    }
    if (length)
        *length = actlen;
    return rslt;
}

/*
static unsigned decimal_digits(unsigned const n)
{
    unsigned digits = 1;
    unsigned scale;

    for (scale = 10; scale <= n; scale *= 10)
        ++digits;
    return digits;
}

static
unsigned op2s(char dst[], uint32_t const op)
{
    static char const opcodes[16] = "MIDNSHP=XB??????";
    unsigned oplen = op >> 4;
    unsigned const digits = decimal_digits(oplen);
    unsigned i;
    
    dst[digits] = opcodes[op & 0xF];
    for (i = 1; ; ++i) {
        int const digit = (oplen % 10) + '0';
        
        dst[digits - i] = digit;
        if (i == digits)
            break;
        oplen /= 10;
    }
    return digits + 1;
}

static
unsigned GenerateCigarString(char rslt[], unsigned const ops, uint32_t const op[ops])
{
    unsigned i;
    unsigned j;
    
    for (i = j = 0; i != ops; ++i)
        j += op2s(rslt + j, op[i]);

    return j;
}

static
char const *SRASimpleAlignmentEnumeratorCigarString(SRASimpleAlignmentEnumerator *const self, uint32_t *const length)
{
    unsigned ops;
    uint32_t const *op = SRASimpleAlignmentEnumeratorBAMCigar(self, &ops);
    char *rslt = NULL;
    unsigned len = 0;
    
    if (op) {
        rslt = malloc(11 * ops);
        if (rslt) {
            unsigned const actlen = GenerateCigarString(rslt, ops, op);
            void *const tmp = realloc(rslt, actlen + 1);
            if (tmp) {
                rslt = tmp;
                rslt[actlen] = '\0';
                len = actlen;
            }
            else {
                free(rslt);
                rslt = NULL;
            }
        }
    }
    if (length)
        *length = len;
    return NULL;
}
 */

/* MARK: c_SRAAlignmentEnumerator external accessors */

int64_t c_SRAAlignmentEnumeratorReadID(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(SpotID, 0);
}

int32_t c_SRAAlignmentEnumeratorPlatform(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(Platform, 0);
}

char const *c_SRAAlignmentEnumeratorReadGroup(c_SRAAlignmentEnumerator *const self, uint32_t *const length)
{
    return SRAAlignmentEnumeratorInnerAccesLen(ReadGroup, NULL);
}

int32_t c_SRAAlignmentEnumeratorReadNumber(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(ReadNumber, 0);
}

int32_t c_SRAAlignmentEnumeratorReadLength(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(ReadLength, 0);
}

char const *c_SRAAlignmentEnumeratorReadSequence(c_SRAAlignmentEnumerator *const self, uint32_t *length)
{
    return SRAAlignmentEnumeratorInnerAccesLen(ReadSequence, NULL);
}

uint8_t const *c_SRAAlignmentEnumeratorReadQuality(c_SRAAlignmentEnumerator *const self, uint32_t *length)
{
    return SRAAlignmentEnumeratorInnerAccesLen(ReadQuality, NULL);
}

int64_t c_SRAAlignmentEnumeratorRefRowID(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(RefRowID, 0);
}

int32_t c_SRAAlignmentEnumeratorAlignmentStart(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(AlignmentStart, 0);
}

int32_t c_SRAAlignmentEnumeratorReferenceLength(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(ReferenceLength, 0);
}

uint32_t const *c_SRAAlignmentEnumeratorCigar(c_SRAAlignmentEnumerator *const self, uint32_t *const length)
{
    return SRAAlignmentEnumeratorInnerAccesLen(BAMCigar, length);
}

int32_t c_SRAAlignmentEnumeratorMappingQuality(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(MappingQuality, 0);
}

static
uint32_t c_SRAAlignmentEnumeratorFlags(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(Flags, 0);
}

bool c_SRAAlignmentEnumeratorReadPaired(c_SRAAlignmentEnumerator *const self)
{
    return (c_SRAAlignmentEnumeratorFlags(self) & 1) == 0 ? false : true;
}

bool c_SRAAlignmentEnumeratorProperPair(c_SRAAlignmentEnumerator *const self)
{
    return (c_SRAAlignmentEnumeratorFlags(self) & 2) == 0 ? false : true;
}

/*
bool c_SRAAlignmentEnumeratorReadUnmapped(c_SRAAlignmentEnumerator *const self)
{
    return false;
}

bool c_SRAAlignmentEnumeratorFirstOfPair(c_SRAAlignmentEnumerator *const self)
{
    return c_SRAAlignmentEnumeratorReadNumber(self) == 1 ? true : false;
}

bool c_SRAAlignmentEnumeratorSecondOfPair(c_SRAAlignmentEnumerator *const self)
{
    return c_SRAAlignmentEnumeratorReadNumber(self) == 2 ? true : false;
}
*/

bool c_SRAAlignmentEnumeratorReadNegativeStrand(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(Strand, false);
}

bool c_SRAAlignmentEnumeratorNotPrimaryAlignment (c_SRAAlignmentEnumerator *const self)
{
    if (self && self->current)
        return !self->current->primary;
    return false;
}

bool c_SRAAlignmentEnumeratorReadFailsVendorQC(c_SRAAlignmentEnumerator *const self)
{
    return (c_SRAAlignmentEnumeratorFlags(self) & 0x200) == 0 ? false : true;
}

bool c_SRAAlignmentEnumeratorReadIsDuplicate(c_SRAAlignmentEnumerator *const self)
{
    return (c_SRAAlignmentEnumeratorFlags(self) & 0x400) == 0 ? false : true;
}

bool c_SRAAlignmentEnumeratorMateUnmapped(c_SRAAlignmentEnumerator *const self)
{
    return (c_SRAAlignmentEnumeratorFlags(self) & 8) == 0 ? false : true;
}

bool c_SRAAlignmentEnumeratorMateNegativeStrand(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(MateStrand, false);
}

int64_t c_SRAAlignmentEnumeratorMateRefRowID(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(MateRefRowID, 0);
}

int32_t c_SRAAlignmentEnumeratorMateAlignmentStart(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(MateAlignmentStart, 0);
}

int32_t c_SRAAlignmentEnumeratorTemplateLength(c_SRAAlignmentEnumerator *const self)
{
    return SRAAlignmentEnumeratorInnerAccessor(TemplateLength, 0);
}


static
rc_t SRASimpleAlignmentEnumeratorInit(SRASimpleAlignmentEnumerator *const self,
                                      VTable const *const tbl,
                                      int64_t const start,
                                      uint64_t const count)
{
    rc_t rc = -1;
    
    if (tbl) {
        VCursor const *curs;
        rc = VTableCreateCursorRead(tbl, &curs);
        
        if (rc == 0) {
            static const column_bind_t columns[] = {
                COLUMN_BIND_INIT_2("I64",            "SEQ_SPOT_ID", 0, 0),
                COLUMN_BIND_INIT_2("U8",             "PLATFORM", 0, 0),
                COLUMN_BIND_INIT_2("utf8",           "SPOT_GROUP", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "SEQ_READ_ID", 0, 0),
                COLUMN_BIND_INIT_2("U32",            "READ_LEN", 0, 0),
                COLUMN_BIND_INIT_2("INSDC:dna:text", "READ", 0, 0),
                COLUMN_BIND_INIT_2("U8",             "QUALITY", 0, 0),
                
                COLUMN_BIND_INIT_2("I64",            "REF_ID", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "REF_POS", 0, 0),
                COLUMN_BIND_INIT_2("U32",            "REF_LEN", 0, 0),
                COLUMN_BIND_INIT_2("bool",           "HAS_MISMATCH", 0, 0),
                COLUMN_BIND_INIT_2("bool",           "HAS_REF_OFFSET", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "REF_OFFSET", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "MAPQ", 0, 0),
                COLUMN_BIND_INIT_2("bool",           "REF_ORIENTATION", 0, 0),
                COLUMN_BIND_INIT_2("U32",            "SAM_FLAGS", 0, 0),
                
                COLUMN_BIND_INIT_2("I64",            "MATE_REF_ID", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "MATE_REF_POS", 0, 0),
                COLUMN_BIND_INIT_2("bool",           "MATE_REF_ORIENTATION", 0, 0),
                COLUMN_BIND_INIT_2("I32",            "TEMPLATE_LEN", 0, 0)
            };
            
            memcpy(self->cols, columns, sizeof(self->cols));
            self->e.curs = curs;
            self->e.start = start;
            self->e.count = count;
            
            rc = SRAEnumeratorOpen(&self->e, false, aln_LAST, self->cols);
            if (rc) {
                VCursorRelease(curs);
                memset(self, 0, sizeof(*self));
            }
        }
    }
    return rc;
}

static
void SRAUnsortedAlignmentEnumeratorFree(c_SRAAlignmentEnumerator *const dad)
{
    SRAUnsortedAlignmentEnumerator *const self = (SRAUnsortedAlignmentEnumerator *)dad;
    
    SRAEnumeratorClose(&self->inner[0].e);
    SRAEnumeratorClose(&self->inner[1].e);
}

static
bool SRAUnsortedAlignmentEnumeratorNext(c_SRAAlignmentEnumerator *const dad)
{
    SRAUnsortedAlignmentEnumerator *const self = (SRAUnsortedAlignmentEnumerator *)dad;
    
    while (self->dad.current && SRAEnumeratorIsOpen(&self->dad.current->e)) {
        SRAEnumeratorNext(&self->dad.current->e);
        if (!SRAEnumeratorIsEOF(&self->dad.current->e))
            return true;
        if (self->dad.current == &self->inner[1])
            break;
        self->dad.current = &self->inner[1];
    }
    self->dad.current = NULL;
    return false;
}

/*
static
bool SRAUnsortedAlignmentEnumeratorSeek(c_SRAAlignmentEnumerator *self,
                                        c_SRAReferenceList const *refList,
                                        char const name[], int32_t position,
                                        bool endingAfter)
{
    return false;
}
*/

c_SRAAlignmentEnumerator *c_SRAFileReaderCreateAlignmentEnumerator(c_SRAFileReader *const self,
                                                                   bool primary,
                                                                   bool secondary,
                                                                   char const **const errmsg)
{
    static SRAAlignmentEnumerator_vt const vt = {
        "SRAUnsortedAlignmentEnumerator",
        SRAUnsortedAlignmentEnumeratorFree,
        SRAUnsortedAlignmentEnumeratorNext,
        NULL
    };
    SRAUnsortedAlignmentEnumerator *const rslt = calloc(1, sizeof(*rslt));
    
    if (rslt && self) {
        rc_t rc;
        
        if (secondary) {
            rc = SRASimpleAlignmentEnumeratorInit(&rslt->inner[1],
                                                  self->tables[sfrt_SECONDARY],
                                                  self->rows[sfrt_SECONDARY].start,
                                                  self->rows[sfrt_SECONDARY].count);
            if (rc == 0)
                rslt->dad.current = &rslt->inner[1];
        }
        if (primary) {
            rslt->inner[0].primary = true;
            rc = SRASimpleAlignmentEnumeratorInit(&rslt->inner[0],
                                                  self->tables[sfrt_PRIMARY],
                                                  self->rows[sfrt_PRIMARY].start,
                                                  self->rows[sfrt_PRIMARY].count);
            if (rc == 0)
                rslt->dad.current = &rslt->inner[0];
        }
        rslt->dad.vt = &vt;
    }
    return &rslt->dad;
}

static
void SRAOrderedAlignmentEnumeratorFree(c_SRAAlignmentEnumerator *const dad)
{
    SRAOrderedAlignmentEnumerator *const self = (SRAOrderedAlignmentEnumerator *)dad;
    
    SRAEnumeratorClose(&self->ref);
    SRAEnumeratorClose(&self->inner[0].e);
    SRAEnumeratorClose(&self->inner[1].e);
    free(self->lists[0].alignments);
    free(self->lists[1].alignments);
}

#define AlignmentCmp(FLD, A, B) (((alignment_t const *)(A))->FLD < ((alignment_t const *)(B))->FLD ? -1 : ((alignment_t const *)(A))->FLD == ((alignment_t const *)(B))->FLD ? 0 : 1)

static void AlignmentListSortByRowID(alignment_list_t *const self)
{
#define SWAP(A, B, C, D) KSORT_TSWAP(alignment_t, A, B)
#define CMP(A, B) AlignmentCmp(rowid, A, B)
    KSORT(self->alignments, self->count, sizeof(alignment_t), 0, sizeof(alignment_t));
#undef CMP
#undef SWAP
}

static int AlignmentCmpRefPos(void const *const A, void const *const B)
{
    int const diffPos = AlignmentCmp(refPos, A, B);
    if (diffPos)
        return diffPos;
    else {
        int const diffLen = AlignmentCmp(refLen, A, B);
        if (diffLen)
            return diffLen;
    }
    return A < B ? -1 : A == B ? 0 : 1;
}

static void AlignmentListSortByRefPos(alignment_list_t *const self)
{
#define SWAP(A, B, C, D) KSORT_TSWAP(alignment_t, A, B)
#define CMP(A, B) AlignmentCmpRefPos(A, B)
    KSORT(self->alignments, self->count, sizeof(alignment_t), 0, sizeof(alignment_t));
#undef CMP
#undef SWAP
}

static
void SRAOrderedAlignmentEnumeratorLoadList(SRAOrderedAlignmentEnumerator *const self, unsigned const which)
{
    SRASimpleAlignmentEnumerator *const e = &self->inner[which];
    alignment_list_t *const list = &self->lists[which];
    column_bind_t *IDs = &self->cols[which ? ref_SID : ref_PID];
    rc_t rc = SRAEnumeratorReadColumn(&self->ref, IDs);
    unsigned i;
    
    list->current = list->count = 0;
    if (rc) return;
    if (IDs->elements > list->allocated) {
        void *const tmp = realloc(list->alignments, sizeof(list->alignments[0]) * IDs->elements);
        
        if (tmp == NULL)
            return;
        list->allocated = IDs->elements;
        list->alignments = tmp;
    }
    list->count = IDs->elements;
    for (i = 0; i != list->count; ++i) {
        list->alignments[i].rowid = COLUMN_VALUE_AS_I64(*IDs)[i];
    }
    AlignmentListSortByRowID(list);
    for (i = 0; i < list->count; ++i) {
        alignment_t *const cur = &list->alignments[i];
        
        SRAEnumeratorSetCurrent(&e->e, cur->rowid);
        cur->refPos = SRASimpleAlignmentEnumeratorAlignmentStart(e);
        cur->refLen = SRASimpleAlignmentEnumeratorReferenceLength(e);
        if (cur->refLen == 0) {
            cur->refLen = 0;
            cur->refPos = -1;
        }
    }
    AlignmentListSortByRefPos(list);
    while (list->current < list->count && list->alignments[list->current].refPos == -1)
        ++list->current;
}

static
bool SRAOrderedAlignmentEnumeratorNext(c_SRAAlignmentEnumerator *const dad)
{
    SRAOrderedAlignmentEnumerator *const self = (SRAOrderedAlignmentEnumerator *)dad;

    for ( ; ; )  {
        if (self->lists[0].current < self->lists[0].count && self->lists[1].current < self->lists[1].count) {
            unsigned const pbeg = self->lists[0].alignments[self->lists[0].current].refPos;
            unsigned const sbeg = self->lists[1].alignments[self->lists[1].current].refPos;
            unsigned const pend = pbeg + self->lists[0].alignments[self->lists[0].current].refLen;
            unsigned const send = sbeg + self->lists[1].alignments[self->lists[1].current].refLen;
            unsigned const iter = (pbeg < sbeg || (pbeg == sbeg && pend < send)) ? 0 : 1;

            SRAEnumeratorSetCurrent(&self->inner[iter].e, self->lists[iter].alignments[self->lists[iter].current].rowid);
            ++self->lists[iter].current;
            dad->current = &self->inner[iter];
            return true;
        }
        if (self->lists[0].current < self->lists[0].count) {
            unsigned const iter = 0;
            
            SRAEnumeratorSetCurrent(&self->inner[iter].e, self->lists[iter].alignments[self->lists[iter].current].rowid);
            ++self->lists[iter].current;
            dad->current = &self->inner[iter];
            return true;
        }
        if (self->lists[1].current < self->lists[1].count) {
            unsigned const iter = 1;
            
            SRAEnumeratorSetCurrent(&self->inner[iter].e, self->lists[iter].alignments[self->lists[iter].current].rowid);
            ++self->lists[iter].current;
            dad->current = &self->inner[iter];
            return true;
        }
        SRAEnumeratorNext(&self->ref);
        if (SRAEnumeratorIsEOF(&self->ref))
            return false;
        if (self->cols[ref_PID].id)
            SRAOrderedAlignmentEnumeratorLoadList(self, 0);
        if (self->cols[ref_SID].id)
            SRAOrderedAlignmentEnumeratorLoadList(self, 1);
    }
}

static
void SRAOrderedAlignmentEnumeratorSeekList(alignment_list_t *const list, unsigned const position, bool const endingAfter)
{
    while (list->current < list->count) {
        unsigned const beg = list->alignments[list->current].refPos;
        unsigned const goal = beg + (endingAfter ? list->alignments[list->current].refLen: 0);

        if (goal >= position)
            break;
        ++list->current;
    }
}

static
bool SRAOrderedAlignmentEnumeratorSeek(c_SRAAlignmentEnumerator *const dad,
                                       c_SRAReferenceList const *refList,
                                       char const name[], int32_t const position,
                                       bool endingAfter)
{
    SRAOrderedAlignmentEnumerator *const self = (SRAOrderedAlignmentEnumerator *)dad;
    int const refNo = c_SRAReferenceListGetIndexForName(refList, name);
    
    if (refNo >= 0) {
        ReferenceEntry const *const entry = refList->refs[refNo];
        
        if (position < entry->length) {
            unsigned const max_seq_len = entry->max_seq_len;
            int64_t const last_row = (entry->beginRow + entry->length + max_seq_len - 1) / max_seq_len;
            int64_t row = entry->beginRow + position / max_seq_len;
            
            if (endingAfter) {
                if (self->cols[ref_LOOKBACK].id) {
                    int const inner_pos = position % max_seq_len;
                    
                    if (inner_pos < COLUMN_VALUE_AS_I32(self->cols[ref_OVERLAP_MAX])[0])
                        row -= COLUMN_VALUE_AS_I32(self->cols[ref_LOOKBACK])[0];
                }
                else {
                    row -= 10;
                    if (row < entry->beginRow)
                        row = entry->beginRow;
                }
            }
            SRAEnumeratorSetCurrent(&self->ref, row);
            for ( ; ; ) {
                if (self->cols[ref_PID].id)
                    SRAOrderedAlignmentEnumeratorLoadList(self, 0);
                if (self->cols[ref_SID].id)
                    SRAOrderedAlignmentEnumeratorLoadList(self, 1);
                if (endingAfter) {
                    unsigned last_end = 0;
                    
                    if (self->cols[ref_PID].id)
                        last_end = self->lists[0].alignments[self->lists[0].count - 1].refPos + self->lists[0].alignments[self->lists[0].count - 1].refLen;
                    if (self->cols[ref_SID].id) {
                        unsigned const le = self->lists[1].alignments[self->lists[1].count - 1].refPos + self->lists[1].alignments[self->lists[1].count - 1].refLen;
                        
                        if (last_end < le)
                            last_end = le;
                    }
                    if (last_end >= position)
                        break;
                }
                else {
                    unsigned last_start = 0;
                    
                    if (self->cols[ref_PID].id)
                        last_start = self->lists[0].alignments[self->lists[0].count - 1].refPos;
                    if (self->cols[ref_SID].id) {
                        unsigned const ls = self->lists[1].alignments[self->lists[1].count - 1].refPos;
                        
                        if (last_start < ls)
                            last_start = ls;
                    }
                    if (last_start >= position)
                        break;
                }
                SRAEnumeratorNext(&self->ref);
                if (SRAEnumeratorCurrent(&self->ref) >= last_row)
                    return false;
            }
            if (self->cols[ref_PID].id)
                SRAOrderedAlignmentEnumeratorSeekList(&self->lists[0], position, endingAfter);
            if (self->cols[ref_SID].id)
                SRAOrderedAlignmentEnumeratorSeekList(&self->lists[1], position, endingAfter);
            return true;
        }
    }
    return false;
}

c_SRAAlignmentEnumerator *c_SRAFileReaderCreateOrderedAlignmentEnumerator(c_SRAFileReader *self,
                                                                          bool primary,
                                                                          bool secondary,
                                                                          char const **const errmsg)
{
    static SRAAlignmentEnumerator_vt const vt = {
        "SRAOrderedAlignmentEnumerator",
        SRAOrderedAlignmentEnumeratorFree,
        SRAOrderedAlignmentEnumeratorNext,
        SRAOrderedAlignmentEnumeratorSeek,
    };
    static const column_bind_t cols[] = {
        COLUMN_BIND_INIT_2("I64", "PRIMARY_ALIGNMENT_IDS", 0, 0),
        COLUMN_BIND_INIT_2("I64", "SECONDARY_ALIGNMENT_IDS", 0, 0),
        COLUMN_BIND_INIT_2("I32", "OVERLAP_REF_POS", 1, 0),
        COLUMN_BIND_INIT_2("I32", "OVERLAP_REF_LEN", 1, 0),
    };
    SRAOrderedAlignmentEnumerator *rslt = NULL;
    
    if (self && self->tables[sfrt_REFERENCE]) {
        if (self->tables[sfrt_PRIMARY] == NULL) primary = false;
        if (self->tables[sfrt_SECONDARY] == NULL) secondary = false;

        rslt = calloc(1, sizeof(*rslt));
        if (rslt && (primary || secondary)) {
            VCursor const *curs;
            rc_t rc = VTableCreateCursorRead(self->tables[sfrt_REFERENCE], &curs);

            if (rc == 0) {
                memcpy(rslt->cols, cols, sizeof(rslt->cols));

                rslt->cols[ref_PID].disabled = !primary;
                rslt->cols[ref_SID].disabled = !secondary;
                
                SRAEnumeratorInit(&rslt->ref, curs, self->rows[sfrt_REFERENCE].start, self->rows[sfrt_REFERENCE].count);
                rc = SRAEnumeratorOpen(&rslt->ref, false, sizeof(rslt->cols)/sizeof(rslt->cols[0]), rslt->cols);
                if (rc == 0) {
                    if (secondary) {
                        rc = SRASimpleAlignmentEnumeratorInit(&rslt->inner[1],
                                                              self->tables[sfrt_SECONDARY],
                                                              self->rows[sfrt_SECONDARY].start,
                                                              self->rows[sfrt_SECONDARY].count);
                        if (rc == 0)
                            rslt->dad.current = &rslt->inner[1];
                    }
                    if (primary) {
                        rslt->inner[0].primary = true;
                        rc = SRASimpleAlignmentEnumeratorInit(&rslt->inner[0],
                                                              self->tables[sfrt_PRIMARY],
                                                              self->rows[sfrt_PRIMARY].start,
                                                              self->rows[sfrt_PRIMARY].count);
                        if (rc == 0)
                            rslt->dad.current = &rslt->inner[0];
                    }
                    rslt->dad.vt = &vt;
                }
                else
                    memset(rslt, 0, sizeof(*rslt));
            }
        }
    }
    return &rslt->dad;
}

static
void SRASingleAlignmentEnumeratorFree(c_SRAAlignmentEnumerator *const dad)
{
    SRASingleAlignmentEnumerator *const self = (SRASingleAlignmentEnumerator *)dad;
    
    SRAEnumeratorClose(&self->inner.e);
}

c_SRAAlignmentEnumerator *c_SRAFileReaderCreateAlignmentByIDEnumerator(c_SRAFileReader *self,
                                                                       int64_t id,
                                                                       char const **const errmsg)
{
    static SRAAlignmentEnumerator_vt const vt = {
        "SRASingleAlignmentEnumerator",
        SRASingleAlignmentEnumeratorFree,
        NULL,
        NULL,
    };
    SRASingleAlignmentEnumerator *rslt = NULL;
    
    *errmsg = "not found";
    if (self && self->tables[sfrt_REFERENCE] && self->tables[sfrt_PRIMARY]
        && id >= self->rows[sfrt_PRIMARY].start
        && id < self->rows[sfrt_PRIMARY].count + self->rows[sfrt_PRIMARY].count)
    {
        rslt = calloc(1, sizeof(rslt));
        if (rslt) {
            rc_t rc = SRASimpleAlignmentEnumeratorInit(&rslt->inner, self->tables[sfrt_PRIMARY], id, 1);

            if (rc == 0) {
                rslt->dad.vt = &vt;
                *errmsg = NULL;
            }
        }
    }
    return &rslt->dad;
}
