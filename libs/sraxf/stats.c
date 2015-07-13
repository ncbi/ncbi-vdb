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
    size_t const newsize = rslt + len;
    
    if (KDataBufferResize(self, newsize) != 0)
        return -1;
    
#if 1
    PLOGMSG(klogInfo, (klogInfo, "New spot group '$(NAME)'", "NAME=%.*s", (int)len, name));
#endif
    
    memcpy(((char *)self->base) + rslt, name, len);
    
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
    unsigned node_name;
};

static void group_stats_init(group_stats_t *const self,
                             unsigned const offset,
                             unsigned const length,
                             unsigned const node_name)
{
    memset(self, 0, sizeof(*self));
    self->name_offset = offset;
    self->name_len = length;
    self->node_name = node_name;
    self->spot_min = MAX_SPOT_ID;
    self->spot_max = MIN_SPOT_ID;
}

#define STATS_DATA_MRU_COUNT 3
struct stats_data_s {
    rc_t (*update)(stats_data_t *self,
                   int64_t spot_id,
                   uint32_t spot_len,
                   uint32_t bio_spot_len,
                   uint32_t cmp_spot_len,
                   const char *grp,
                   uint64_t grp_len);
    rc_t (*write)(group_stats_t const *const self, KMDataNode *const node);
    rc_t (*write_all)(stats_data_t *const self);
    KMetadata *meta;
    KDataBuffer names;
    KDataBuffer group;
    unsigned count;
    group_stats_t *mru[STATS_DATA_MRU_COUNT];
    group_stats_t table;
    group_stats_t deflt;
};

static char *get_name(stats_data_t *const self, unsigned const idx)
{
    return ((char *)self->names.base) + idx;
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
        
        memset(newmru, 0, sizeof(self->mru));
        newmru[0] = mru;
        for (di = 1, si = 0; di < STATS_DATA_MRU_COUNT && si < STATS_DATA_MRU_COUNT; ++si) {
            group_stats_t *const u = self->mru[si];
            
            if (u != mru) {
                newmru[di] = u;
                ++di;
            }
        }
        memcpy(self->mru, newmru, sizeof(newmru));
    }
}

static int stats_data_name_cmp(stats_data_t *const self,
                               group_stats_t const *const B,
                               unsigned const Alen,
                               char const Aname[])
{
    unsigned const Blen = B->name_len;
    char const *const Bname = get_name(self, B->name_offset);
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

static group_stats_t *stats_data_get_group(stats_data_t *const self,
                                           unsigned const len,
                                           char const name[])
{
    unsigned i;
    unsigned which;
    group_stats_t *fnd;
    
    for (i = 0; i < STATS_DATA_MRU_COUNT && i < self->count; ++i) {
        group_stats_t *const k = self->mru[i];
        
        if (k && stats_data_name_cmp(self, k, len, name) == 0) {
            fnd = k;
            goto UPDATE_MRU;
        }
    }
    which = stats_data_search(self, len, name);
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
        
        group_stats_init(fnd, offset, len, self->count);
        stats_data_invald_mru(self);
    }
UPDATE_MRU:
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

static rc_t group_stats_write_name(KMDataNode *const node,
                                   unsigned const namelen,
                                   char const name[/* namelen */])
{
    char        sbuf[4096];
    char *const hbuf   = (namelen + 1) < sizeof(sbuf) ? NULL : malloc(namelen + 1);
    char *const buffer = (namelen + 1) < sizeof(sbuf) ? sbuf : hbuf;
    
    if (buffer == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);

    memcpy(buffer, name, namelen);
    buffer[namelen] = '\0';
    {
        rc_t const rc = KMDataNodeWriteAttr(node, STATS_NODE_NAME_ATTR, buffer);
            
        free(hbuf);
        return rc;
    }
}

#define RC_THROW(STMT) do { rc_t const rc = (STMT); if (rc) return rc; } while(0)

static rc_t group_stats_write_no_compressed(group_stats_t const *const self,
                                            KMDataNode *const node)
{
    RC_THROW(group_stats_write_node(node, "SPOT_COUNT"    , &self->spot_count    ));
    RC_THROW(group_stats_write_node(node, "BASE_COUNT"    , &self->base_count    ));
    RC_THROW(group_stats_write_node(node, "BIO_BASE_COUNT", &self->bio_base_count));
    RC_THROW(group_stats_write_node(node, "SPOT_MIN"      , &self->spot_min      ));
    RC_THROW(group_stats_write_node(node, "SPOT_MAX"      , &self->spot_max      ));
    
    return 0;
}

static rc_t group_stats_write_compressed(group_stats_t const *const self,
                                         KMDataNode *const node)
{
    RC_THROW(group_stats_write_no_compressed(self, node));
    RC_THROW(group_stats_write_node(node, "CMP_BASE_COUNT", &self->cmp_base_count));
    
    return 0;
}

static rc_t group_stats_write_1(group_stats_t const *const self,
                                stats_data_t const *const parent,
                                unsigned const namelen,
                                char const name[/* namelen */],
                                KMDataNode *const node)
{
    RC_THROW(group_stats_write_name(node, namelen, name));
    RC_THROW(parent->write(self, node));

    return 0;
}

static rc_t group_stats_write(group_stats_t const *const self,
                              stats_data_t const *const parent,
                              char const strings[])
{
    static char const namebase[] = "STATS/SPOT_GROUP/";
    char namepath[sizeof(namebase) + 3]; /* sizeof(namebase) includes terminator */
    char *const name = namepath + sizeof(namebase) - 1;
    unsigned nodeid = self->node_name - 1;
    KMDataNode *node;
    rc_t rc;

    memcpy(namepath, namebase, sizeof(namebase));
    name[4] = '\0';
    name[3] = nodeid % 26 + 'A'; nodeid /= 26;
    name[2] = nodeid % 26 + 'A'; nodeid /= 26;
    name[1] = nodeid % 26 + 'A'; nodeid /= 26;
    name[0] = nodeid % 26 + 'A'; nodeid /= 26;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, "%s", namepath));

    rc = group_stats_write_1(self, parent,
                             self->name_len,
                             strings + self->name_offset,
                             node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t group_stats_write_default(group_stats_t const *const self,
                                      stats_data_t const *const parent)
{
    KMDataNode *node;
    rc_t rc;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, "STATS/SPOT_GROUP/default"));

    rc = parent->write(self, node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t group_stats_write_table(group_stats_t const *const self,
                                    stats_data_t const *const parent)
{
    KMDataNode *node;
    rc_t rc;
    
    RC_THROW(KMetadataOpenNodeUpdate(parent->meta, &node, "STATS/TABLE"));

    rc = parent->write(self, node);
    KMDataNodeRelease(node);
    
    return rc;
}

static rc_t stats_data_write_table(stats_data_t *const self)
{
    RC_THROW(group_stats_write_table(&self->table, self));
    return 0;
}

static rc_t stats_data_write_all(stats_data_t *const self)
{
    unsigned i;

    RC_THROW(stats_data_write_table(self));
    if (self->count <= MAX_GROUP_COUNT) {
        if (self->deflt.spot_count != 0)
            RC_THROW(group_stats_write_default(&self->deflt, self));
        RC_THROW(group_stats_write_table(&self->table, self));
        for (i = 0; i < self->count; ++i) {
            RC_THROW(group_stats_write(get_group(self, i), self, self->names.base));
        }
    }
    return 0;
}

static
void CC stats_data_whack(void *const data)
{
    stats_data_t *const self = data;
    
    self->write_all(self);
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
    if (spot_id) {
        ++self->spot_count;
        if (self->spot_max < spot_id)
            self->spot_max = spot_id;
        if (self->spot_min > spot_id)
            self->spot_min = spot_id;
    }
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
                             uint64_t grp_len);

static
rc_t stats_data_update_all(stats_data_t *self,
                           int64_t spot_id,
                           uint32_t spot_len,
                           uint32_t bio_spot_len,
                           uint32_t cmp_spot_len,
                           char const grp[],
                           uint64_t grp_len);

static void stats_data_init_funcs(stats_data_t *const self, bool has_spot_groups)
{
    self->update    = has_spot_groups ? stats_data_update_all : stats_data_update_table;
    self->write_all = has_spot_groups ? stats_data_write_all  : stats_data_write_table;
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
            stats_data_init_funcs(self, false);
            (void)PLOGMSG(klogWarn, (klogWarn, "Too many spot groups ($(count)); dropping group stats", "count=%u", (unsigned)(self->count)));
        }
    }
    return 0;
}

static
rc_t stats_data_update_all(stats_data_t *self,
                           int64_t spot_id,
                           uint32_t spot_len,
                           uint32_t bio_spot_len,
                           uint32_t cmp_spot_len,
                           const char *grp,
                           uint64_t grp_len)
{
    stats_data_update_table(self, spot_id, spot_len, bio_spot_len, cmp_spot_len, 0, 0);
    return stats_data_update_group(self, spot_id, spot_len, bio_spot_len, cmp_spot_len, grp, grp_len);
}

static rc_t stats_data_init(stats_data_t *const self,
                            VTable *const tbl,
                            bool has_spot_group,
                            bool compressed)
{
    stats_data_init_funcs(self, has_spot_group);
    self->write  = compressed ? group_stats_write_compressed : group_stats_write_no_compressed;
    
    group_stats_init(&self->table, 0, 0, 0);
    group_stats_init(&self->deflt, 0, 0, 0);

    RC_THROW(VTableOpenMetadataUpdate(tbl, &self->meta));
    if (has_spot_group) {
        RC_THROW(KDataBufferMakeBytes(&self->names, 0));
        RC_THROW(KDataBufferMake(&self->group, sizeof(group_stats_t) * 8, 0));
    }
    return 0;
}

static
rc_t stats_data_make(stats_data_t **const pself,
                     VTable *const tbl,
                     bool has_spot_group,
                     bool compressed)
{
    rc_t rc = 0;
    stats_data_t *const self = calloc(1, sizeof(*self));
    
    if (self) {
        rc = stats_data_init(self, tbl, has_spot_group, compressed);
        if (rc == 0) {
            *pself = self;
            return 0;
        }
        KMetadataRelease(self->meta);
        free(self);
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}

static
rc_t CC stats_data_update(stats_data_t* self,
    const int64_t spot_id, const uint32_t spot_len,
    const uint32_t bio_spot_len, const uint32_t cmp_spot_len,
    bool has_grp, const char* grp, uint64_t grp_len)
{
    return self->update(self, spot_id, spot_len, bio_spot_len, cmp_spot_len, grp, grp_len);
}

static
rc_t CC stats_data_trigger(void *data, const VXformInfo *info, int64_t row_id,
                               VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    uint32_t i, bio_spot_len;
    const char* grp = NULL;
    uint64_t len = 0;

    uint32_t spot_len = argv[0].u.data.elem_count;
    /* take nreads from read_len */
    uint32_t nreads = argv[1].u.data.elem_count;
    /* get read_len and read_type */
    const INSDC_coord_len *read_len = argv[1].u.data.base;
    const INSDC_SRA_xread_type *read_type = argv[2].u.data.base;
    read_len += argv[1].u.data.first_elem;
    read_type += argv[2].u.data.first_elem;

    assert(argc >= 3 && argc <= 4);
    assert(nreads == argv[2].u.data.elem_count);

    for(i = bio_spot_len = 0; i < nreads; i++) {
        if( (read_type[i] & SRA_READ_TYPE_BIOLOGICAL) != 0 ) {
            bio_spot_len += read_len[i];
        }
    }
    if( argc == 4 ) {
        /* get group name and length */
        grp = argv[3].u.data.base;
        len = argv[3].u.data.elem_count;
        grp += argv[3].u.data.first_elem;
    }
    return stats_data_update(data, row_id, spot_len, bio_spot_len, 0, argc == 4, grp, len);
}

static
rc_t CC stats_data_cmp_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                   VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    uint32_t i, bio_spot_len;
    const char* grp = NULL;
    uint64_t len = 0;

    uint32_t cmp_spot_len = argv[0].u.data.elem_count;
    uint32_t spot_len = argv[1].u.data.elem_count;
    /* take nreads from read_len */
    uint32_t nreads = argv[2].u.data.elem_count;
    /* get read_len and read_type */
    const INSDC_coord_len *read_len = argv[2].u.data.base;
    const INSDC_SRA_xread_type *read_type = argv[3].u.data.base;
    read_len += argv[2].u.data.first_elem;
    read_type += argv[3].u.data.first_elem;

    assert(data != NULL);
    assert(argc >= 4 && argc <= 5);
    assert(nreads == argv[3].u.data.elem_count);

    for(i = bio_spot_len = 0; i < nreads; i++) {
        if( (read_type[i] & SRA_READ_TYPE_BIOLOGICAL) != 0 ) {
            bio_spot_len += read_len[i];
        }
    }
    if( argc == 5 ) {
        /* get group name and length */
        grp = argv[4].u.data.base;
        len = argv[4].u.data.elem_count;
        grp += argv[4].u.data.first_elem;
    }
    return stats_data_update(data, row_id, spot_len, bio_spot_len, cmp_spot_len, argc == 5, grp, len);
}

static
rc_t CC stats_data_cmpf_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                   VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    uint32_t i, bio_spot_len;
    const char* grp = NULL;
    uint64_t len = 0;

    uint32_t cmp_spot_len = argv[0].u.data.elem_count;
    const uint32_t* spot_len = argv[1].u.data.base;
    /* take nreads from read_len */
    uint32_t nreads = argv[2].u.data.elem_count;
    /* get read_len and read_type */
    const INSDC_coord_len *read_len = argv[2].u.data.base;
    const INSDC_SRA_xread_type *read_type = argv[3].u.data.base;
    spot_len += argv[1].u.data.first_elem;
    read_len += argv[2].u.data.first_elem;
    read_type += argv[3].u.data.first_elem;

    assert(data != NULL);
    assert(argc >= 4 && argc <= 5);
    assert(nreads == argv[3].u.data.elem_count);

    for(i = bio_spot_len = 0; i < nreads; i++) {
        if( (read_type[i] & SRA_READ_TYPE_BIOLOGICAL) != 0 ) {
            bio_spot_len += read_len[i];
        }
    }
    if( argc == 5 ) {
        /* get group name and length */
        grp = argv[4].u.data.base;
        len = argv[4].u.data.elem_count;
        grp += argv[4].u.data.first_elem;
    }
    return stats_data_update(data, row_id, *spot_len, bio_spot_len, cmp_spot_len, argc == 5, grp, len);
}

static
rc_t CC stats_data_csra2_trigger(void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    uint32_t bio_spot_len;
    const char* grp = NULL;
    uint64_t len = 0;
    
    uint32_t read_len = argv[0].u.data.elem_count;
    bio_spot_len = read_len;
    
    assert(argc >= 1 && argc <= 2);
    
    if( argc == 2 ) {
        assert(1 == argv[1].u.data.elem_count);
        
        /* get group name and length */
        grp = argv[1].u.data.base;
        len = argv[1].u.data.elem_count;
        grp += argv[1].u.data.first_elem;
    }
    return stats_data_update(data, row_id, read_len, bio_spot_len, 0, argc == 2, grp, len);
}


VTRANSFACT_IMPL ( NCBI_SRA_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    stats_data_t *data;

    assert(dp->argc >= 3 && dp->argc <= 4);

    if( (rc = stats_data_make(&data, (VTable*)info->tbl, dp->argc > 3, false)) == 0 ) {
        rslt->self = data;
        rslt->whack = stats_data_whack;
        rslt->variant = vftNonDetRow;
        rslt->u.rf = stats_data_trigger;
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_cmp_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    stats_data_t *data;

    assert(dp->argc >= 4 && dp->argc <= 5);

    if( (rc = stats_data_make(&data, (VTable*)info->tbl, dp->argc > 4, true)) == 0 ) {
        rslt->self = data;
        rslt->whack = stats_data_whack;
        rslt->variant = vftNonDetRow;
        rslt->u.rf = stats_data_cmp_trigger;
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_cmpf_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    stats_data_t *data;

    assert(dp->argc >= 4 && dp->argc <= 5);

    if( (rc = stats_data_make(&data, (VTable*)info->tbl, dp->argc > 4, true)) == 0 ) {
        rslt->self = data;
        rslt->whack = stats_data_whack;
        rslt->variant = vftNonDetRow;
        rslt->u.rf = stats_data_cmpf_trigger;
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_csra2_stats_trigger, 1, 0, 0 )
    ( const void * self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    stats_data_t *data;
    
    assert(dp->argc >= 1 && dp->argc <= 2);
    
    if( (rc = stats_data_make(&data, (VTable*)info->tbl, dp->argc > 1, false)) == 0 ) {
        rslt->self = data;
        rslt->whack = stats_data_whack;
        rslt->variant = vftNonDetRow;
        rslt->u.rf = stats_data_csra2_trigger;
    }
    return rc;
}
