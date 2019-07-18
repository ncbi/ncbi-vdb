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
#include <vdb/extern.h>

#include <sra/sradb.h>
#include <vdb/xform.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <kdb/meta.h>
#include <os-native.h> /* strncasecmp */
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_GROUP_COUNT (30000u)
#define STATS_NODE_NAME_ATTR "name"

static int string_buffer_add(KDataBuffer *const self,
                             unsigned const len, char const name[/* len */])
{
    size_t const rslt = self->elem_count;
    size_t const newsize = rslt + len + 1; /* +1 for nul-terminator */
    
    if (KDataBufferResize(self, newsize) != 0)
        return -1;
    else {
        char *const dst = (char *)self->base + rslt;
        unsigned i;
        
        for (i = 0; i < len; ++i)
            dst[i] = name[i];
        dst[len] = '\0';
    }
#if 1
    PLOGMSG(klogInfo, (klogInfo, "New spot group '$(NAME)'", "NAME=%.*s", (int)len, name));
#endif
    
    return (int)rslt;
}

typedef uint64_t count_t;
typedef int64_t spot_t;
#define MAX_SPOT_ID INT64_MAX
#define MIN_SPOT_ID INT64_MIN

typedef struct group_stats_s group_stats_t;
typedef struct stats_data_s stats_data_t;

struct group_stats_s {
    count_t spot_count;
    count_t base_count;
    count_t bio_base_count;
    count_t cmp_base_count;
    spot_t  spot_min;
    spot_t  spot_max;
    unsigned name_offset;
    unsigned name_len;
    unsigned node_id;
};

static group_stats_t group_stats_init(unsigned const offset,
                                      unsigned const length,
                                      unsigned const node_id)
{
    group_stats_t result = {0};
    result.name_offset = offset;
    result.name_len = length;
    result.node_id = node_id;
    result.spot_min = MAX_SPOT_ID;
    result.spot_max = MIN_SPOT_ID;
    return result;
}

enum StatsOptions {
    so_HasSpotGroup = 1,    /* group by SPOT_GROUP. IOW, there is a SPOT_GROUP column */
    so_SpotCount = 2,       /* write SPOT_COUNT, SPOT_MIN, SPOT_MAX */
    so_BaseCount = 4,       /* write BASE_COUNT */
    so_BioBaseCount = 8,    /* write BIO_BASE_COUNT */
    so_CmpBaseCount = 16    /* write CMP_BASE_COUNT */
};

#define STATS_DATA_MRU_COUNT (((size_t)256) / (sizeof(void *) * 8))
struct stats_data_s {
    VCursor const *curs;
    KMetadata *meta;
    KDataBuffer names;
    KDataBuffer group;
    group_stats_t table;
    group_stats_t deflt;
    group_stats_t *mru[STATS_DATA_MRU_COUNT];
    uint64_t options;
    uint32_t cid;
    unsigned count; /* number of spot groups */
};

static char *get_name(stats_data_t const *const self, group_stats_t const *const child)
{
    return ((char *)self->names.base) + child->name_offset;
}

static group_stats_t *get_group(stats_data_t *const self, unsigned const idx)
{
    return ((group_stats_t *)self->group.base) + idx;
}

static void stats_data_invald_mru(stats_data_t *const self) {
    memset(self->mru, 0, sizeof(self->mru));
}

static void stats_data_update_mru(stats_data_t *const self, group_stats_t *const mru)
{
    if (self->mru[0] != mru) {
        group_stats_t *newmru[STATS_DATA_MRU_COUNT];
        unsigned di;
        unsigned si;
        
        memset(newmru, 0, sizeof(newmru));
        newmru[0] = mru;
        for (di = 1, si = 0; di < STATS_DATA_MRU_COUNT && si < STATS_DATA_MRU_COUNT; ++si) {
            group_stats_t *const u = self->mru[si];
            
            if (u != mru) {
                newmru[di] = u;
                ++di;
            }
        }
        memmove(self->mru, newmru, sizeof(newmru));
    }
}

static int stats_data_name_cmp(stats_data_t *const self,
                               group_stats_t const *const B,
                               unsigned const Alen,
                               char const Aname[])
{
    unsigned const Blen = B->name_len;
    char const *const Bname = get_name(self, B);
    unsigned Ai;
    unsigned Bi;
    
    for (Ai = Bi = 0; Ai < Alen && Bi < Blen; ++Ai, ++Bi) {
        int const a = toupper(Aname[Ai]);
        int const b = toupper(Bname[Bi]);
        int const diff = a - b;
        
        if (diff != 0)
            return diff;
    }
    return Alen - Blen;
}

static int stats_data_name_cmp_idx(stats_data_t *const self,
                                   unsigned const idx,
                                   unsigned const Alen,
                                   char const Aname[])
{
    group_stats_t const *const B = get_group(self, idx);
    return stats_data_name_cmp(self, B, Alen, Aname);
}

/* the return value is either where name was found or where it should go */
static unsigned stats_data_search(stats_data_t *const self,
                                        unsigned const len,
                                        char const name[])
{
    unsigned f = 0;
    unsigned e = self->count;
    
    while (f < e) {
        unsigned const m = f + (e - f) / 2;
        int const diff = stats_data_name_cmp_idx(self, m, len, name);
        
        if (diff < 0)
            e = m;
        else if (diff > 0)
            f = m + 1;
        else
            return m;
    }
    return f;
}

static group_stats_t *search_mru_list(stats_data_t *const self,
                                      unsigned const len,
                                      char const name[])
{
    unsigned i;
    
    for (i = 0; i < STATS_DATA_MRU_COUNT && i < self->count; ++i) {
        group_stats_t *const k = self->mru[i];
        
        if (k && stats_data_name_cmp(self, k, len, name) == 0)
            return k;
    }
    return NULL;
}

static group_stats_t *stats_data_get_group(stats_data_t *const self,
                                           unsigned const len,
                                           char const name[])
{
    group_stats_t *fnd = search_mru_list(self, len, name);
    
    if (fnd == NULL) {
        unsigned const which = stats_data_search(self, len, name);
        if (which < self->count && stats_data_name_cmp_idx(self, which, len, name) == 0)
            fnd = get_group(self, which);
        else {
            int const offset = string_buffer_add(&self->names, len, name);
            unsigned const move = self->count - which;
            
            if (offset < 0)
                return NULL;
            
            if (++self->count > MAX_GROUP_COUNT)
                return get_group(self, MAX_GROUP_COUNT);
            
            if (KDataBufferResize(&self->group, self->count) != 0)
                return NULL;
            
            fnd = get_group(self, which);
            assert(fnd + move + 1 <= get_group(self, self->count) || move == 0);
            memmove(fnd + 1, fnd, move * sizeof(*fnd));
            
            *fnd = group_stats_init(offset, len, self->count);
            stats_data_invald_mru(self);
        }
    }
    stats_data_update_mru(self, fnd);
    return fnd;
}

static rc_t group_stats_write_node(KMDataNode *const node,
                                   char const name[],
                                   void const *const value)
{
    KMDataNode *subnode;
    rc_t rc = KMDataNodeOpenNodeUpdate(node, &subnode, "%s", name);
    if (rc == 0) {
        rc = KMDataNodeWriteB64(subnode, value);
        KMDataNodeRelease(subnode);
    }
    return rc;
}

#define RC_THROW(STMT) do { rc_t const rc = (STMT); if (rc) return rc; } while(0)

static rc_t group_stats_write_stats(group_stats_t const *const self,
                                stats_data_t const *const parent,
                                KMDataNode *const node)
{
    if ((parent->options & so_SpotCount) != 0) {
        RC_THROW(group_stats_write_node(node, "SPOT_COUNT"    , &self->spot_count    ));
        RC_THROW(group_stats_write_node(node, "SPOT_MIN"      , &self->spot_min      ));
        RC_THROW(group_stats_write_node(node, "SPOT_MAX"      , &self->spot_max      ));
    }
    if ((parent->options & so_BaseCount) != 0) {
        RC_THROW(group_stats_write_node(node, "BASE_COUNT"    , &self->base_count    ));
    }
    if ((parent->options & so_BioBaseCount) != 0) {
        RC_THROW(group_stats_write_node(node, "BIO_BASE_COUNT", &self->bio_base_count));
    }
    if ((parent->options & so_CmpBaseCount) != 0) {
        RC_THROW(group_stats_write_node(node, "CMP_BASE_COUNT", &self->cmp_base_count));
    }

    return 0;
}

static rc_t group_stats_write_with_name(group_stats_t const *const self,
                                stats_data_t const *const parent,
                                KMDataNode *const node)
{
    RC_THROW(KMDataNodeWriteAttr(node, STATS_NODE_NAME_ATTR, get_name(parent, self)));
    RC_THROW(group_stats_write_stats(self, parent, node));

    return 0;
}

static rc_t group_stats_write(group_stats_t const *const self,
                              stats_data_t const *const parent)
{
    static char const namebase[] = "STATS/SPOT_GROUP/";
    char namepath[sizeof(namebase) + 3]; /* sizeof(namebase) includes terminator */
    char *const name = namepath + sizeof(namebase) - 1;
    unsigned nodeid = self->node_id - 1;
    KMDataNode *node;
    rc_t rc;

    memmove(namepath, namebase, sizeof(namebase));
    name[4] = '\0';
    name[3] = nodeid % 26 + 'A'; nodeid /= 26;
    name[2] = nodeid % 26 + 'A'; nodeid /= 26;
    name[1] = nodeid % 26 + 'A'; nodeid /= 26;
    name[0] = nodeid % 26 + 'A'; nodeid /= 26;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, namepath)); /* namepath is safe (we just generated it) */

    rc = group_stats_write_with_name(self, parent, node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t group_stats_write_default(group_stats_t const *const self,
                                      stats_data_t const *const parent)
{
    KMDataNode *node;
    rc_t rc;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, "STATS/SPOT_GROUP/default"));

    rc = group_stats_write_stats(self, parent, node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t group_stats_write_table(group_stats_t const *const self,
                                    stats_data_t const *const parent)
{
    KMDataNode *node;
    rc_t rc;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, "STATS/TABLE"));

    rc = group_stats_write_stats(self, parent, node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t stats_data_write_all(stats_data_t *const self)
{
    unsigned i;

    RC_THROW(group_stats_write_table(&self->table, self));
    if ((self->options & so_HasSpotGroup) == 0)
        return 0;
    
    if (self->deflt.spot_count != 0)
        RC_THROW(group_stats_write_default(&self->deflt, self));

    RC_THROW(group_stats_write_table(&self->table, self));

    for (i = 0; i < self->count; ++i) {
        RC_THROW(group_stats_write(get_group(self, i), self));
    }
    return 0;
}

static
void CC stats_data_whack(void *const data)
{
    stats_data_t *const self = data;

    VCursorRelease(self->curs);
    
    stats_data_write_all(self);
    KDataBufferWhack(&self->group);
    KDataBufferWhack(&self->names);
    KMetadataRelease(self->meta);
    free(self);
}

static void group_stats_update(group_stats_t *const self,
                               spot_t const spot_id,
                               INSDC_coord_len const spot_len,
                               INSDC_coord_len const bio_spot_len,
                               INSDC_coord_len const cmp_spot_len
                               )
{
    ++self->spot_count;
    if (self->spot_max < spot_id)
        self->spot_max = spot_id;
    if (self->spot_min > spot_id)
        self->spot_min = spot_id;
    self->base_count     += spot_len;
    self->bio_base_count += bio_spot_len;
    self->cmp_base_count += cmp_spot_len;
}

static
rc_t stats_data_update_table(stats_data_t *self,
                             int64_t spot_id,
                             uint32_t spot_len,
                             uint32_t bio_spot_len,
                             uint32_t cmp_spot_len,
                             char const grp[],
                             uint64_t grp_len)
{
    group_stats_update(&self->table, spot_id, spot_len, bio_spot_len, cmp_spot_len);
    return 0;
}

static
rc_t stats_data_update_group(stats_data_t *self,
                             int64_t spot_id,
                             uint32_t spot_len,
                             uint32_t bio_spot_len,
                             uint32_t cmp_spot_len,
                             char const grp[],
                             uint64_t grp_len)
{
    if (grp_len == 0 || grp == NULL || grp[0] == '\0' ||
        (grp_len == 7 && strncasecmp("default", grp, 7) == 0))
    {
        group_stats_update(&self->deflt, spot_id, spot_len, bio_spot_len, cmp_spot_len);
    }
    else
    {
        group_stats_t *const fnd = stats_data_get_group(self, (unsigned)grp_len, grp);
        
        if (fnd == NULL)
            return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
        
        if (fnd - get_group(self, 0) < MAX_GROUP_COUNT)
            group_stats_update(fnd, spot_id, spot_len, bio_spot_len, cmp_spot_len);
        else {
            KDataBufferWhack(&self->group);
            KDataBufferWhack(&self->names);
            self->options &= ~so_HasSpotGroup;
            (void)PLOGMSG(klogWarn, (klogWarn, "Too many spot groups ($(count)); dropping group stats", "count=%u", (unsigned)(self->count)));
        }
    }
    return 0;
}

static KMetadata *openMetadataUpdate(VTable const *const tbl, rc_t *const rc)
{
    KMetadata *meta = NULL;
    
    *rc = VTableOpenMetadataUpdate((VTable *)tbl, &meta);
    assert(*rc == 0 && meta != NULL);
    if (*rc != 0 || meta == NULL)
        return NULL;
    return meta;
}

static
stats_data_t *stats_data_make(VTable const *const tbl, int64_t const options, VCursor const *const curs, uint32_t const cid, rc_t *const rc)
{
    stats_data_t *const self = calloc(1, sizeof(*self));
    assert(self != NULL);
    if (self == NULL) {
        *rc = RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
        return NULL;
    }
    self->table = self->deflt = group_stats_init(0, 0, 0);
    self->curs = curs;
    self->cid = cid;
    self->meta = openMetadataUpdate(tbl, rc);
    self->names.elem_bits = 8;
    self->group.elem_bits = sizeof(group_stats_t) * 8;
    self->options = options | (curs != NULL ? so_HasSpotGroup : 0);
    return self;
}

static rc_t stats_data_update(stats_data_t* self,
    const int64_t spot_id, const uint32_t spot_len,
    const uint32_t bio_spot_len, const uint32_t cmp_spot_len,
    const char* grp, uint64_t grp_len)
{
    rc_t const rc = stats_data_update_table(self, spot_id, spot_len, bio_spot_len, cmp_spot_len, 0, 0);
    if ((self->options & so_HasSpotGroup) == 0 || rc != 0)
        return rc;
    return stats_data_update_group(self, spot_id, spot_len, bio_spot_len, cmp_spot_len, grp, grp_len);
}

static unsigned bioSpotLength(unsigned const nreads, INSDC_SRA_xread_type const *const read_type, INSDC_coord_len const *const read_len)
{
    unsigned i;
    unsigned len;
    for (len = i = 0; i < nreads; ++i) {
        if ((read_type[i] & SRA_READ_TYPE_BIOLOGICAL) != 0)
            len += read_len[i];
    }
    return len;
}

#define SAFE_BASE(ELEM, DTYPE) ((ELEM < argc && sizeof(DTYPE) * 8 == (size_t)argv[ELEM].u.data.elem_bits) ? (((DTYPE const *)argv[ELEM].u.data.base) + argv[ELEM].u.data.first_elem) : ((DTYPE const *)NULL))
#define BIND_COLUMN(ELEM, DTYPE, POINTER) DTYPE const *const POINTER = SAFE_BASE(ELEM, DTYPE)
#define SAFE_COUNT(ELEM) (ELEM < argc ? argv[ELEM].u.data.elem_count : 0)

static
rc_t CC stats_data_trigger(void *data, const VXformInfo *info, int64_t row_id,
                               VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    enum COLUMNS {
        col_READ,
        col_READ_LEN,
        col_READ_TYPE,
        col_SPOT_GROUP
    };
    unsigned const spot_len = SAFE_COUNT(col_READ);
    unsigned const nreads = SAFE_COUNT(col_READ_LEN);
    BIND_COLUMN(col_READ_LEN, INSDC_coord_len, read_len);
    BIND_COLUMN(col_READ_TYPE, INSDC_SRA_xread_type, read_type);
    BIND_COLUMN(col_SPOT_GROUP, char, grp);
    unsigned const len = SAFE_COUNT(col_SPOT_GROUP);

    return stats_data_update(data, row_id, spot_len, bioSpotLength(nreads, read_type, read_len), 0, grp, len);
}

static
rc_t CC stats_data_cmp_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                   VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    enum COLUMNS {
        col_CMP_READ,
        col_QUALITY,
        col_READ_LEN,
        col_READ_TYPE,
        col_SPOT_GROUP
    };
    unsigned const cmp_spot_len = SAFE_COUNT(col_CMP_READ);
    unsigned const spot_len = SAFE_COUNT(col_QUALITY);
    unsigned const nreads = SAFE_COUNT(col_READ_LEN);
    BIND_COLUMN(col_READ_LEN, INSDC_coord_len, read_len);
    BIND_COLUMN(col_READ_TYPE, INSDC_SRA_xread_type, read_type);
    BIND_COLUMN(col_SPOT_GROUP, char, grp);
    unsigned const len = SAFE_COUNT(col_SPOT_GROUP);

    return stats_data_update(data, row_id, spot_len, bioSpotLength(nreads, read_type, read_len), cmp_spot_len, grp, len);
}

static
rc_t CC stats_data_cmpf_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                   VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    enum COLUMNS {
        col_CMP_READ,
        col_SPOT_LEN,
        col_READ_LEN,
        col_READ_TYPE,
        col_SPOT_GROUP
    };
    unsigned const cmp_spot_len = SAFE_COUNT(col_CMP_READ);
    unsigned const nreads = SAFE_COUNT(col_READ_LEN);
    BIND_COLUMN(col_SPOT_LEN, uint32_t, spot_len);
    BIND_COLUMN(col_READ_LEN, INSDC_coord_len, read_len);
    BIND_COLUMN(col_READ_TYPE, INSDC_SRA_xread_type, read_type);
    BIND_COLUMN(col_SPOT_GROUP, char, grp);
    unsigned const len = SAFE_COUNT(col_SPOT_GROUP);

    return stats_data_update(data, row_id, spot_len[0], bioSpotLength(nreads, read_type, read_len), cmp_spot_len, grp, len);
}

static
rc_t CC stats_data_cmpb_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    enum COLUMNS {
        col_CMP_READ,
        col_SPOT_GROUP
    };
    unsigned const cmp_spot_len = SAFE_COUNT(col_CMP_READ);
    BIND_COLUMN(col_SPOT_GROUP, char, phys_grp);
    unsigned const phys_len = SAFE_COUNT(col_SPOT_GROUP);
    stats_data_t *const self = data;
    char const *grp = phys_grp;
    unsigned len = phys_len;
    
    if (grp == NULL && self->curs != NULL) {
        void const *base;
        uint32_t count;
        uint32_t boff;
        uint32_t elem_bits;
        rc_t rc;
        
        rc = VCursorCellDataDirect(self->curs, row_id, self->cid, &elem_bits, &base, &boff, &count);
        grp = base;
        len = count;
        if (rc) return rc;
    }
    return stats_data_update(self, 0, 0, 0, cmp_spot_len, grp, len);
}

static
rc_t CC stats_data_csra2_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    enum COLUMNS {
        col_READ,
        col_SPOT_GROUP
    };
    unsigned const read_len = SAFE_COUNT(col_READ);
    unsigned const len = SAFE_COUNT(col_SPOT_GROUP);
    BIND_COLUMN(col_SPOT_GROUP, char, grp);

    return stats_data_update(data, row_id, read_len, read_len, 0, grp, len);
}

VTRANSFACT_IMPL ( NCBI_SRA_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    int64_t const options = so_SpotCount
                          | so_BaseCount
                          | so_BioBaseCount
                          | (dp->argc > 3 ? so_HasSpotGroup : 0);
    rc_t rc = 0;

    rslt->self = stats_data_make(info->tbl, options, NULL, 0, &rc);
    rslt->whack = stats_data_whack;
    rslt->variant = vftNonDetRow;
    rslt->u.rf = stats_data_trigger;

    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_cmp_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    int64_t const options = so_SpotCount
                          | so_BaseCount
                          | so_BioBaseCount
                          | so_CmpBaseCount
                          | (dp->argc > 4 ? so_HasSpotGroup : 0);
    rc_t rc = 0;
    
    rslt->self = stats_data_make(info->tbl, options, NULL, 0, &rc);
    rslt->whack = stats_data_whack;
    rslt->variant = vftNonDetRow;
    rslt->u.rf = stats_data_cmp_trigger;
    
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_cmpf_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    int64_t const options = so_SpotCount
                          | so_BaseCount
                          | so_BioBaseCount
                          | so_CmpBaseCount
                          | (dp->argc > 4 ? so_HasSpotGroup : 0);
    rc_t rc = 0;
    
    rslt->self = stats_data_make(info->tbl, options, NULL, 0, &rc);
    rslt->whack = stats_data_whack;
    rslt->variant = vftNonDetRow;
    rslt->u.rf = stats_data_cmpf_trigger;
    
    return rc;
}

VTRANSFACT_IMPL ( NCBI_csra2_stats_trigger, 1, 0, 0 )
    ( const void * self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    int64_t const options = so_SpotCount
                          | so_BaseCount
                          | so_BioBaseCount
                          | (dp->argc > 1 ? so_HasSpotGroup : 0);
    rc_t rc = 0;
    
    rslt->self = stats_data_make(info->tbl, options, NULL, 0, &rc);
    rslt->whack = stats_data_whack;
    rslt->variant = vftNonDetRow;
    rslt->u.rf = stats_data_csra2_trigger;
    
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_cmpb_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    int64_t const options = so_CmpBaseCount
                          | (dp->argc > 1 ? so_HasSpotGroup : 0);
    rc_t rc = 0;
    VCursor const *curs = NULL;
    uint32_t cid = 0;

    if (dp->argc == 1) {
        rc = VTableCreateCursorRead(info->tbl, &curs);
        if (rc == 0)
            rc = VCursorAddColumn(curs, &cid, "SPOT_GROUP");
        if (rc == 0)
            rc = VCursorOpen(curs);
        if (rc != 0 && curs != NULL) {
            VCursorRelease(curs);
            curs = NULL;
        }
    }
    rslt->self = stats_data_make(info->tbl, options, curs, cid, &rc);
    rslt->whack = stats_data_whack;
    rslt->variant = vftNonDetRow;
    rslt->u.rf = stats_data_cmpb_trigger;
    
    return rc;
}
