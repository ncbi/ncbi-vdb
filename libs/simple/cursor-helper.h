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

typedef struct column_bind_s column_bind_t;
struct column_bind_s {
    char const *type;
    char const *name;
    void const *value;
    uint32_t id;
    uint32_t elements;
    bool optional;
    bool disabled;
};

#define COLUMN_BIND_INIT_2(TYPE, NAME, OPT, DIS) { TYPE, NAME, NULL, 0, 0, OPT, DIS }
#define COLUMN_BIND_INIT_1(NAME, OPT, DIS) COLUMN_BIND_INIT_2(NULL, NAME, OPT, DIS)

typedef struct SRAEnumerator SRAEnumerator;
struct SRAEnumerator {
    VCursor const *curs;
    int64_t  start;
    uint64_t count;
    uint64_t current;
};

static
void SRAEnumeratorInit(SRAEnumerator *const self,
                       VCursor const *curs,
                       int64_t const start,
                       uint64_t const count)
{
    self->curs = curs;
    self->start = start;
    self->count = count;
    self->current = 0;
}

static
void SRAEnumeratorClose(SRAEnumerator *const self)
{
    VCursorRelease(self->curs);
    self->curs = NULL;
}

#define PRINT_ERROR 0

static
rc_t SRAEnumeratorAddColumn(SRAEnumerator *const self, column_bind_t *const column)
{
    rc_t rc;
    
    if (column->type && column->type[0]) {
        rc = VCursorAddColumn(self->curs, &column->id, "(%s)%s", column->type, column->name);
#if PRINT_ERROR
        if (rc)
            fprintf(stderr, "Failed to open '(%s)%s'; rc: %u\n", column->type, column->name, rc);
#endif
    }
    else {
        rc = VCursorAddColumn(self->curs, &column->id, column->name);
#if PRINT_ERROR
        if (rc)
            fprintf(stderr, "Failed to open '%s'; rc: %u\n", column->name, rc);
#endif
    }
    return rc;
}

static
rc_t SRAEnumeratorOpen(SRAEnumerator *const self, bool force, unsigned const columns, column_bind_t column[columns])
{
    unsigned i;
    bool have_optional = false;
    
#define RETURN_FAILURE(CALL) do { rc_t const rc__ = (CALL); if (rc__) return rc__; } while(0)
    for (i = 0; i != columns; ++i) {
        if (column[i].disabled)
            continue;
        if (column[i].optional) {
            have_optional = true;
            continue;
        }
        {
            rc_t const rc = SRAEnumeratorAddColumn(self, &column[i]);
            
            if (rc) {
                if (force) {
                    column[i].optional = true;
                    have_optional = true;
                }
                else
                    return rc;
            }
        }
    }
    if (have_optional)
        RETURN_FAILURE(VCursorPermitPostOpenAdd(self->curs));

    RETURN_FAILURE(VCursorOpen(self->curs));
#undef RETURN_FAILURE
    
    if (have_optional) {
        for (i = 0; i != columns; ++i) {
            if (column[i].disabled)
                continue;
            if (!column[i].optional)
                continue;
            column[i].disabled = SRAEnumeratorAddColumn(self, &column[i]) != 0;
        }
    }
    return 0;
}

static
void SRAEnumeratorSetCurrent(SRAEnumerator *const self, int64_t const current)
{
    self->current = current - self->start;
}

static
int64_t SRAEnumeratorCurrent(SRAEnumerator const *const self)
{
    return self->start + self->current;
}

static
rc_t SRAEnumeratorReadColumn(SRAEnumerator const *const self, column_bind_t *const column)
{
    return VCursorCellDataDirect(self->curs, SRAEnumeratorCurrent(self), column->id, NULL, &column->value, NULL, &column->elements);
}

static
bool SRAEnumeratorIsOpen(SRAEnumerator const *const self)
{
    return self->curs != NULL;
}

static
bool SRAEnumeratorIsEOF(SRAEnumerator const *const self)
{
    return self->current >= self->count;
}

static
void SRAEnumeratorNext(SRAEnumerator *const self)
{
    ++self->current;
}

static
bool SRAEnumeratorCanRead(SRAEnumerator const *const self)
{
    return SRAEnumeratorIsOpen(self) && !SRAEnumeratorIsEOF(self);
}
