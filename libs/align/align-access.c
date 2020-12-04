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

#include <align/extern.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <align/align-access.h>
#include <atomic32.h>

#include <vfs/path.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bam-priv.h"

struct AlignAccessMgr
{
    atomic32_t refcount;
};

LIB_EXPORT rc_t CC AlignAccessMgrMake(const AlignAccessMgr **mgr) {
    AlignAccessMgr *self = malloc(sizeof(*self));

    *mgr = self;
    if (self != NULL) {
        atomic32_set(&self->refcount, 1);
        return 0;
    }
    return RC(rcAlign, rcMgr, rcConstructing, rcMemory, rcExhausted);
}

LIB_EXPORT rc_t CC AlignAccessMgrAddRef( const AlignAccessMgr *self ) {
    if (self != NULL)
        atomic32_inc(&((AlignAccessMgr *)self)->refcount);
    return 0;
}

static
rc_t CC AlignAccessMgrWhack(AlignAccessMgr *self) {
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessMgrRelease(const AlignAccessMgr *cself) {
    rc_t rc = 0;
    AlignAccessMgr *self = (AlignAccessMgr *)cself;
    
    if (cself != NULL) {
        if (atomic32_dec_and_test(&self->refcount)) {
            rc = AlignAccessMgrWhack(self);
            if (rc)
                atomic32_set(&self->refcount, 1);
            else
                free(self);
        }
    }
    return rc;
}

struct AlignAccessDB {
    atomic32_t refcount;
    const AlignAccessMgr *mgr;
    const BAMFile *innerSelf;
};

LIB_EXPORT rc_t CC AlignAccessMgrMakeBAMDB(const AlignAccessMgr *self, const AlignAccessDB **db, const VPath *bam) {
    AlignAccessDB *lhs = malloc(sizeof(*lhs));
    rc_t rc;

    if (lhs == NULL)
        return RC(rcAlign, rcMgr, rcConstructing, rcMemory, rcExhausted);
    
    rc = BAMFileMakeWithVPath(&lhs->innerSelf, bam);
    if (rc) {
        free(lhs);
        return rc;
    }
    lhs->mgr = self;
    AlignAccessMgrAddRef(lhs->mgr);
    atomic32_set(&lhs->refcount, 1);
    
    *db = lhs;
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessMgrMakeIndexBAMDB(const AlignAccessMgr *self, const AlignAccessDB **db, const VPath *bam, const VPath *bam_index) {
    rc_t rc = AlignAccessMgrMakeBAMDB(self, db, bam);
    if (rc == 0) {
        rc = BAMFileOpenIndexWithVPath((**db).innerSelf, bam_index);
        if (rc == 0)
            return 0;
        AlignAccessDBRelease(*db);
        *db = NULL;
    }
    return rc;
}


/* ExportBAMFile
 *  export the BAMFile object in use by the AlignAccessDB, if any
 *  must be released via BAMFileRelease
 */
LIB_EXPORT rc_t CC AlignAccessDBExportBAMFile ( const AlignAccessDB *self, const BAMFile **result )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcAlign, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcAlign, rcTable, rcAccessing, rcSelf, rcNull );
        else if ( self -> innerSelf == NULL )
            rc = RC ( rcAlign, rcTable, rcAccessing, rcType, rcIncorrect );
        else
        {
            rc = BAMFileAddRef ( * result = self -> innerSelf );
            if ( rc == 0 )
                return 0;
        }

        * result = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC AlignAccessDBAddRef(const AlignAccessDB *cself) {
    if (cself != NULL)
        atomic32_inc(&((AlignAccessDB *)cself)->refcount);
    return 0;
}

static rc_t CC AlignAccessDBWhack(AlignAccessDB *self) {
    rc_t rc;
    
    rc = BAMFileRelease(self->innerSelf);
    if (rc)
        return rc;
    return AlignAccessMgrRelease(self->mgr);
}

LIB_EXPORT rc_t CC AlignAccessDBRelease(const AlignAccessDB *cself) {
    rc_t rc = 0;
    AlignAccessDB *self = (AlignAccessDB *)cself;
    
    if (cself != NULL) {
        if (atomic32_dec_and_test(&self->refcount)) {
            rc = AlignAccessDBWhack(self);
            if (rc)
                atomic32_set(&self->refcount, 1);
            else
                free(self);
        }
    }
    return rc;
}

struct AlignAccessRefSeqEnumerator {
    atomic32_t refcount;
    const AlignAccessDB *parent;
    int cur, end;
};

LIB_EXPORT rc_t CC AlignAccessDBEnumerateRefSequences(const AlignAccessDB *self, AlignAccessRefSeqEnumerator **refseq_enum) {
    AlignAccessRefSeqEnumerator *lhs;
    unsigned cur = 0;
    unsigned end;
    
    BAMFileGetRefSeqCount(self->innerSelf, &end);
    if (BAMFileIsIndexed(self->innerSelf)) {
        while (cur != end && BAMFileIndexHasRefSeqId(self->innerSelf, cur) == 0)
            ++cur;
        if (cur == end)
            return AlignAccessRefSeqEnumeratorEOFCode;
    }
    lhs = malloc(sizeof(*lhs));
    *refseq_enum = lhs;
    if (lhs != NULL) {
        lhs->parent = self;
        AlignAccessDBAddRef(lhs->parent);
        lhs->cur = cur;
        lhs->end = end;
        atomic32_set(&lhs->refcount, 1);
        return 0;
    }
    return RC(rcAlign, rcDatabase, rcConstructing, rcMemory, rcExhausted);
}

LIB_EXPORT rc_t CC AlignAccessRefSeqEnumeratorAddRef(const AlignAccessRefSeqEnumerator *self) {
    if (self != NULL)
        atomic32_inc(&((AlignAccessRefSeqEnumerator *)self)->refcount);
    return 0;
}

static
rc_t CC AlignAccessRefSeqEnumeratorWhack(AlignAccessRefSeqEnumerator *self) {
    return AlignAccessDBRelease(self->parent);
}

LIB_EXPORT rc_t CC AlignAccessRefSeqEnumeratorRelease(const AlignAccessRefSeqEnumerator *cself) {
    rc_t rc = 0;
    AlignAccessRefSeqEnumerator *self = (AlignAccessRefSeqEnumerator *)cself;
    
    if (cself != NULL) {
        if (atomic32_dec_and_test(&self->refcount)) {
            rc = AlignAccessRefSeqEnumeratorWhack(self);
            if (rc)
                atomic32_set(&self->refcount, 1);
            else
                free(self);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC AlignAccessRefSeqEnumeratorGetID(const AlignAccessRefSeqEnumerator *cself, char *id_buffer, size_t buffer_size, size_t *id_size) {
    rc_t rc = 0;
    const BAMRefSeq *cur;
    size_t id_act_size;
    
    if (cself == NULL)
        return 0;
    if (id_buffer == NULL && id_size == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    rc = BAMFileGetRefSeq(cself->parent->innerSelf, cself->cur, &cur);
    if (rc)
        return rc;
    id_act_size = string_size( cur->name ) + 1;
    if (id_size != NULL)
        *id_size = id_act_size;
    if (id_buffer != NULL) {
        if (buffer_size >= id_act_size)
            memmove(id_buffer, cur->name, id_act_size);
        else
            rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignAccessRefSeqEnumeratorGetLength ( const AlignAccessRefSeqEnumerator *cself,
                                           uint64_t *length )
{
    rc_t rc = 0;
    const BAMRefSeq *cur;
    
    if (cself == NULL)
        return 0;
    if (length == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    rc = BAMFileGetRefSeq(cself->parent->innerSelf, cself->cur, &cur);
    if (rc)
        return rc;
    *length = cur->length;
    
    return 0;
}

#define ENUMERATOR_IS_NOT_FILTERED_BY_INDEX 1
LIB_EXPORT rc_t CC AlignAccessRefSeqEnumeratorNext(const AlignAccessRefSeqEnumerator *cself) {
    AlignAccessRefSeqEnumerator *self = (AlignAccessRefSeqEnumerator *)cself;
    
    if (cself->cur + 1 >= cself->end)
        return AlignAccessRefSeqEnumeratorEOFCode;
    
    ++self->cur;
#if ENUMERATOR_IS_NOT_FILTERED_BY_INDEX
    return 0;
#else
    if (!BAMFileIsIndexed(cself->parent->innerSelf))
    	return 0;
	if (BAMFileIndexHasRefSeqId(cself->parent->innerSelf, cself->cur))
		return 0;
	return AlignAccessRefSeqEnumeratorNext(cself);
#endif
}

struct AlignAccessAlignmentEnumerator {
    const AlignAccessDB *parent;
    const BAMAlignment *innerSelf;
    BAMFileSlice *slice;
    uint64_t endpos;
    uint64_t startpos;
    atomic32_t refcount;
    int atend;
    int refSeqID;
};

static rc_t CC AlignAccessDBMakeEnumerator(const AlignAccessDB *self, AlignAccessAlignmentEnumerator **align_enum) {
    AlignAccessAlignmentEnumerator *lhs = malloc(sizeof(*lhs));
    
    *align_enum = lhs;
    if (lhs == NULL)
        return RC(rcAlign, rcTable, rcConstructing, rcMemory, rcExhausted);
    
    lhs->innerSelf = NULL;
    lhs->parent = self;
    AlignAccessDBAddRef(lhs->parent);
    atomic32_set(&lhs->refcount, 1);
    lhs->atend = 0;
    lhs->refSeqID = -1;
    lhs->endpos = 0;
    lhs->startpos = 0;
    
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessDBEnumerateAlignments(const AlignAccessDB *self, AlignAccessAlignmentEnumerator **align_enum) {
    rc_t rc;
    
    *align_enum = NULL;

    rc = BAMFileRewind(self->innerSelf);
    if (rc)
        return rc;
    rc = AlignAccessDBMakeEnumerator(self, align_enum);
    if (rc)
        return rc;
    return AlignAccessAlignmentEnumeratorNext(*align_enum);
}

LIB_EXPORT
rc_t CC AlignAccessDBWindowedAlignments(const AlignAccessDB *self,
                                        AlignAccessAlignmentEnumerator **align_enum,
                                        const char *refSeqName, uint64_t pos, uint64_t wsize
                                        )
{
    AlignAccessAlignmentEnumerator *lhs;
    unsigned i, n;
    const BAMRefSeq *rs;
    uint64_t endpos = pos + wsize;
    rc_t rc;
    BAMFileSlice *slice;
    
    *align_enum = NULL;

    BAMFileGetRefSeqCount(self->innerSelf, &n);
    for (i = 0; i != n; ++i) {
        BAMFileGetRefSeq(self->innerSelf, i, &rs);
        if (strcmp(refSeqName, rs->name) == 0)
            break;
    }
    if (i == n || pos >= rs->length) {
        return RC(rcAlign, rcTable, rcConstructing, rcParam, rcInvalid);
    }

    if (endpos > rs->length || wsize == 0)
        endpos = rs->length;

    rc = BAMFileMakeSlice(self->innerSelf, &slice, i, pos, endpos);
    if (rc == 0 && slice == NULL)
        return RC(rcAlign, rcTable, rcConstructing, rcMemory, rcExhausted);
    if ( rc != 0 )
    {
        if ( GetRCState( rc ) == rcNotFound && GetRCObject( rc ) == (enum RCObject)rcData )
            rc = AlignAccessAlignmentEnumeratorEOFCode;
        return rc;
    }
    rc = AlignAccessDBMakeEnumerator(self, &lhs);
    if (rc)
        return rc;

    lhs->refSeqID = i;
    lhs->endpos = endpos;
    lhs->startpos = pos;
    lhs->slice = slice;

    *align_enum = lhs;
    return AlignAccessAlignmentEnumeratorNext(*align_enum);
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorNext(const AlignAccessAlignmentEnumerator *cself) {
    rc_t rc;
    AlignAccessAlignmentEnumerator *self = (AlignAccessAlignmentEnumerator *)cself;
    int32_t refSeqID;

AGAIN:
    if (self->innerSelf != NULL) {
        BAMAlignmentRelease(self->innerSelf);
        self->innerSelf = NULL;
    }
    if (self->atend != 0)
        return AlignAccessAlignmentEnumeratorEOFCode;
    
    if (self->slice == NULL) {
        rc = BAMFileRead2(self->parent->innerSelf, &self->innerSelf);
        if (rc) {
            if (GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcRow) {
                self->atend = 1;
                rc = AlignAccessAlignmentEnumeratorEOFCode;
            }
        }
        return rc;
    }
    rc = BAMFileReadSlice(self->parent->innerSelf, &self->innerSelf, self->slice);
    if (rc) {
        if (GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcRow) {
            self->atend = 1;
            rc = AlignAccessAlignmentEnumeratorEOFCode;
        }
        return rc;
    }
    if (!BAMAlignmentIsMapped(self->innerSelf))
        goto AGAIN;

    BAMAlignmentGetRefSeqId(self->innerSelf, &refSeqID);
    if (self->refSeqID != refSeqID) {
        self->atend = 1;
        rc = AlignAccessAlignmentEnumeratorEOFCode;
    }
    else if (self->endpos != 0) {
        int64_t pos;
        uint32_t length;

        BAMAlignmentGetPosition2(self->innerSelf, &pos, &length);
        if (pos >= (int64_t)self->endpos) {
            self->atend = 1;
            rc = AlignAccessAlignmentEnumeratorEOFCode;
        }
        else {
            int64_t const endpos = pos + length;
            if (endpos <= self->startpos)
                goto AGAIN;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorAddRef ( const AlignAccessAlignmentEnumerator *self ) {
    if (self != NULL)
        atomic32_inc(&((AlignAccessAlignmentEnumerator *)self)->refcount);
    return 0;
}

static
rc_t CC AlignAccessAlignmentEnumeratorWhack(AlignAccessAlignmentEnumerator *self) {
    free(self->slice);
    if (self->innerSelf)
        BAMAlignmentRelease(self->innerSelf);
    AlignAccessDBRelease(self->parent);
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorRelease ( const AlignAccessAlignmentEnumerator *cself ) {
    rc_t rc = 0;
    AlignAccessAlignmentEnumerator *self = (AlignAccessAlignmentEnumerator *)cself;
    
    if (cself != NULL) {
        if (atomic32_dec_and_test(&self->refcount)) {
            rc = AlignAccessAlignmentEnumeratorWhack(self);
            if (rc)
                atomic32_set(&self->refcount, 1);
            else
                free(self);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqID(
                                               const AlignAccessAlignmentEnumerator *self,
                                               char *id_buffer, size_t buffer_size, size_t *id_size
) {
    rc_t rc = 0;
    int32_t id;
    const BAMRefSeq *cur;
    size_t id_act_size;
    
    if (self == NULL)
        return 0;
    if (id_buffer == NULL && id_size == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    rc = BAMAlignmentGetRefSeqId(self->innerSelf, &id);
    if (rc)
        return rc;
    if (id < 0)
        return RC(rcAlign, rcTable, rcAccessing, rcData, rcNotFound);
    rc = BAMFileGetRefSeq(self->parent->innerSelf, id, &cur);
    if (rc)
        return rc;
    id_act_size = string_size( cur->name ) + 1;
    if (id_size != NULL)
        *id_size = id_act_size;
    if (id_buffer != NULL) {
        if (buffer_size >= id_act_size)
            memmove(id_buffer, cur->name, id_act_size);
        else
            rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
    }
    return rc;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqPos(
                                                const AlignAccessAlignmentEnumerator *self,
                                                uint64_t *start_pos
) {
    rc_t rc;
    int64_t pos;
    
    rc = BAMAlignmentGetPosition(self->innerSelf, &pos);
    if (rc)
        return rc;
    if (pos < 0)
        return RC(rcAlign, rcTable, rcAccessing, rcData, rcNotFound);

    *start_pos = pos;
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqLen(
                                                              const AlignAccessAlignmentEnumerator *self,
                                                              uint32_t *reflen
                                                              )
{
    rc_t rc;
    uint32_t length;
    int64_t pos;
    
    rc = BAMAlignmentGetPosition2(self->innerSelf, &pos, &length);
    if (rc)
        return rc;
    if (pos < 0)
        return RC(rcAlign, rcTable, rcAccessing, rcData, rcNotFound);
    
    *reflen = length;
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetMapQuality(
                                                 const AlignAccessAlignmentEnumerator *self,
                                                 uint8_t *score
) {
    uint8_t y = 0;
    
    if (self && self->innerSelf) {
        uint16_t flags;
    
        BAMAlignmentGetFlags(self->innerSelf, &flags);
        if ((flags & BAMFlags_SelfIsUnmapped) == 0)
            BAMAlignmentGetMapQuality(self->innerSelf, &y);
	}
    if (score) *score = y;
    
    return 0;
}


LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetShortSeqAccessionID(
                                                        const AlignAccessAlignmentEnumerator *self,
                                                        char *id_buffer, size_t buffer_size, size_t *id_size
) {
    rc_t rc;
    size_t id_act_size;
    const char *readGroupName;
    
    if (self == NULL)
        return 0;
    if (id_buffer == NULL && id_size == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    rc = BAMAlignmentGetReadGroupName(self->innerSelf, &readGroupName);
    if (rc)
        return rc;
    if (readGroupName == 0)
        *id_size = 0;
    else {
        id_act_size = string_size( readGroupName ) + 1;
        if (id_size != NULL)
            *id_size = id_act_size;
        if (id_buffer != NULL) {
            if (buffer_size >= id_act_size)
                memmove(id_buffer, readGroupName, id_act_size);
            else
                rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
        }
    }
    return rc;    
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetShortSeqID(
                                                 const AlignAccessAlignmentEnumerator *self,
                                                 char *id_buffer, size_t buffer_size, size_t *id_size
) {
    rc_t rc;
    size_t id_act_size;
    const char *readName;
    
    if (self == NULL)
        return 0;
    if (id_buffer == NULL && id_size == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    rc = BAMAlignmentGetReadName(self->innerSelf, &readName);
    if (rc)
        return rc;

    id_act_size = string_size( readName ) + 1;
    if (id_size != NULL)
        *id_size = id_act_size;
    if (id_buffer != NULL) {
        if (buffer_size >= id_act_size)
            memmove(id_buffer, readName, id_act_size);
        else
            rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
    }
    return rc;    
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetCIGAR(
    const AlignAccessAlignmentEnumerator *self,
    uint64_t *start_pos, char *cigar_buffer, size_t buffer_size, size_t *cigar_size
) {
    int i;
    unsigned n;
    rc_t rc;
    uint32_t sp = 0;
    char *cigbuf = cigar_buffer;
    const char *const endp = cigar_buffer + buffer_size;
    
    if (cigar_buffer == NULL && cigar_size == NULL) {
        /* no result can be returned */
        rc = RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    }
    
    rc = BAMAlignmentGetCigarCount(self->innerSelf, &n);
    if (rc)
        return rc;
    if (n == 0) {
        if (cigar_size != NULL)
            *cigar_size = 0;
        if (start_pos != NULL)
            *start_pos = 0;
        cigbuf[0] = 0;
        return 0;
    }
    for (i = 0; i != n; ++i) {
        char cig1[20];
        BAMCigarType op;
        uint32_t len;
        int cig1len;
        
        BAMAlignmentGetCigar(self->innerSelf, i, &op, &len);
        if (i == 0 && op == ct_SoftClip) {
            sp = len;
            continue;
        }
        if (i == 0 && op == ct_HardClip)
            continue;
        if (i == n - 1 && (op == ct_SoftClip || op == ct_HardClip))
            continue;
        
        cig1len = sprintf(cig1, "%c%u", op, len);
        if (cigbuf + cig1len < endp) {
            if (cigar_buffer != NULL) {
                memmove(cigbuf, cig1, cig1len);
                cigbuf[cig1len] = 0;
            }
        }
        else
            rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
        cigbuf += cig1len;
    }
    if (cigar_size != NULL)
        *cigar_size = cigbuf - cigar_buffer + 1;
    if (start_pos != NULL)
        *start_pos = sp;
    return rc;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetShortSequence(
                                                    const AlignAccessAlignmentEnumerator *self,
                                                    char *seq_buffer, size_t buffer_size, size_t *seq_size
) {
    rc_t rc;
    uint32_t act_size;

    if (self == NULL)
        return 0;
    if (seq_buffer == NULL && seq_size == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    rc = BAMAlignmentGetReadLength(self->innerSelf, &act_size);
    if (rc)
        return rc;
    
    ++act_size;
    if (seq_size != NULL)
        *seq_size = act_size;
    
    if (seq_buffer != NULL) {
        if (buffer_size >= act_size) {
            rc = BAMAlignmentGetSequence(self->innerSelf, seq_buffer);
            seq_buffer[act_size - 1] = 0;
        }
        else
            rc = RC(rcAlign, rcTable, rcAccessing, rcBuffer, rcInsufficient);
    }
    return rc;    
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetStrandDirection(
                                                      const AlignAccessAlignmentEnumerator *self,
                                                      AlignmentStrandDirection *result
) {
    uint16_t flags;
    rc_t rc;
    
    if (result == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);

    *result = asd_Unknown;
    if (self == NULL)
        return 0;
    
    rc = BAMAlignmentGetFlags(self->innerSelf, &flags);
    if (rc)
        return rc;
    
    *result = ((flags & BAMFlags_SelfIsReverse) == BAMFlags_SelfIsReverse) ? asd_Reverse : asd_Forward;
    
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetIsPaired(const AlignAccessAlignmentEnumerator *self,
                                               bool *result)
{
    uint16_t flags;
    rc_t rc;
    
    if (result == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    *result = asd_Unknown;
    if (self == NULL)
        return 0;
    
    rc = BAMAlignmentGetFlags(self->innerSelf, &flags);
    if (rc)
        return rc;
    
    *result = ((flags & BAMFlags_IsMappedAsPair) == 0) ? false : true;
    
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetIsFirstInPair(const AlignAccessAlignmentEnumerator *self,
                                                    bool *result)
{
    uint16_t flags;
    rc_t rc;
    
    if (result == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    if (self == NULL)
        return 0;
    
    rc = BAMAlignmentGetFlags(self->innerSelf, &flags);
    if (rc)
        return rc;
    
    *result = ((flags & BAMFlags_IsFirst) == 0) ? false : true;
    
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetIsSecondInPair(const AlignAccessAlignmentEnumerator *self,
                                                     bool *result)
{
    uint16_t flags;
    rc_t rc;
    
    if (result == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    if (self == NULL)
        return 0;
    
    rc = BAMAlignmentGetFlags(self->innerSelf, &flags);
    if (rc)
        return rc;
    
    *result = ((flags & BAMFlags_IsSecond) == 0) ? false : true;
    
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetRecordID(const AlignAccessAlignmentEnumerator *self, uint64_t *result)
{
    if (result == NULL)
        return RC(rcAlign, rcTable, rcAccessing, rcParam, rcNull);
    
    if (self == NULL)
        return 0;
    
    *(BAMFilePosition *)result = BAMAlignmentGetFilePos(self->innerSelf);
    return 0;
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetBAMAlignment(const AlignAccessAlignmentEnumerator *self, const BAMAlignment **result)
{
    *result = self->innerSelf;
    return BAMAlignmentAddRef(self->innerSelf);
}

LIB_EXPORT rc_t CC AlignAccessAlignmentEnumeratorGetSAM(const AlignAccessAlignmentEnumerator *self, size_t *const actSize, size_t const maxsize, char *const buffer)
{
    return BAMAlignmentFormatSAM(self->innerSelf, actSize, maxsize, buffer);
}
