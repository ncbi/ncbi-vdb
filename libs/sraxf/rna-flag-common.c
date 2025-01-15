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

#include <klib/rc.h>
#include <kdb/meta.h>
#include <string.h>

#define NODE_NAME "RNA_FLAG"

struct Self;
typedef rc_t (*copy_f)(struct Self *, uint8_t *, uint8_t const *, size_t);
struct Self {
    KMetadata const *meta;
    copy_f function;
};

static void whack(void *const vp)
{
    struct Self *self = vp;
    KMetadataRelease(self->meta);
    free(self);
}

static rc_t copyDNA_to_DNA(struct Self *const self
                           , uint8_t *const dst
                           , uint8_t const *const src
                           , size_t const length)
{
    ((void)(self));
    memmove(dst, src, length);
    return 0;
}

static rc_t copyRNA_to_DNA(struct Self *const self
                           , uint8_t *const dst
                           , uint8_t const *const src
                           , size_t const length)
{
    size_t i;
    
    ((void)(self));
    for (i = 0; i < length; ++i) {
        int const base = src[i];
        dst[i] = (base != 'U') ? (uint8_t)base : (uint8_t)'T';
    }
    return 0;
}

static rc_t copyDNA_to_RNA(struct Self *const self
                           , uint8_t *const dst
                           , uint8_t const *const src
                           , size_t const length)
{
    size_t i;
    
    ((void)(self));
    for (i = 0; i < length; ++i) {
        int const base = src[i];
        dst[i] = (base != 'T') ? (uint8_t)base : (uint8_t)'U';
    }
    return 0;
}

#if WRITE_SIDE

static rc_t setFlag(KMetadata const *const cmeta, char const value)
{
    KMetadata *const meta = (KMetadata *)cmeta;
    KMDataNode *node = NULL;
    rc_t rc = KMetadataOpenNodeUpdate(meta, &node, NODE_NAME);
    if (rc == 0) {
        rc = KMDataNodeWrite(node, &value, 1);
        KMDataNodeRelease(node);
    }
    return rc;
}

static rc_t checkAndCopy(struct Self *const self
                         , uint8_t *const dst
                         , uint8_t const *const src
                         , size_t const length)
{
    // Note: the FIRST T or U wins.
    void const *const T = memchr(src, 'T', length);
    void const *const U = memchr(src, 'U', length);

    if (T != NULL && (U == NULL || T < U)) {
        rc_t rc = setFlag(self->meta, '0');
        if (rc) return rc;
        self->function = &copyDNA_to_DNA;
        return copyDNA_to_DNA(self, dst, src, length);
    }
    if (U != NULL && (T == NULL || U < T)) {
        rc_t rc = setFlag(self->meta, '1');
        if (rc) return rc;
        self->function = &copyRNA_to_DNA;
        return copyRNA_to_DNA(self, dst, src, length);
    }
    return copyDNA_to_DNA(self, dst, src, length);
}

#else

static rc_t getFlag(KMetadata const *const meta, char *const result)
{
    KMDataNode const *node = NULL;
    rc_t rc = KMetadataOpenNodeRead(meta, &node, NODE_NAME);
    if (rc == 0) {
        size_t num_read = 0;
        size_t remains = 0;
        
        rc = KMDataNodeRead(node, 0, result, 1, &num_read, &remains);
        KMDataNodeRelease(node);
    }
    return rc;
}

static rc_t checkAndCopy(struct Self *const self
                         , uint8_t *const dst
                         , uint8_t const *const src
                         , size_t const length)
{
    char rna = '\0';
    rc_t rc = getFlag(self->meta, &rna);
    if (rc == 0 && rna == '0') {
        self->function = &copyDNA_to_DNA;
        return copyDNA_to_DNA(self, dst, src, length);
    }
    if (rc == 0 && rna == '1') {
        self->function = &copyDNA_to_RNA;
        return copyDNA_to_RNA(self, dst, src, length);
    }
    return copyDNA_to_DNA(self, dst, src, length);
}

#endif

static rc_t makeSelf(VFuncDesc *rslt, KMetadata const *meta)
{
    struct Self *self = malloc(sizeof(*self));
    if (self) {
        self->meta = meta;
        self->function = &checkAndCopy;
        
        rslt->variant = vftRow;
        rslt->self = self;
        rslt->whack = whack;
        return 0;
    }
    return RC(rcXF, rcFunction, rcCreating, rcMemory, rcExhausted);
}
