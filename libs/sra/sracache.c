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

#include <sra/extern.h>
#include <sra/sradb-priv.h>

#include <klib/rc.h>
#include <kproc/lock.h>
#include <klib/refcount.h>
#include <kfg/config.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>

#include <strtol.h>

#include "sra-priv.h"

/* some of the above #defines index on sun */
#ifdef index
    #undef index
#endif

/*================================== SRACacheMetrics ==================================*/
static
void 
MetricsInit(SRACacheMetrics* self, const SRATable* table)
{
    self->bytes     = 0;/* TBD */
    self->elements  = 1;
    self->threads   = 0;/* TBD */
    self->fds       = 0;/* TBD */
}

LIB_EXPORT
bool CC
SRACacheMetricsLessThan(const SRACacheMetrics* a, const SRACacheMetrics* b)
{
    if (a->bytes >= 0 && b->bytes >= 0 && a->bytes > b->bytes)
        return false;
    if (a->elements >= 0 && b->elements >= 0 && a->elements > b->elements)
        return false;
    if (a->threads >= 0 && b->threads >= 0 && a->threads > b->threads)
        return false;
    if (a->fds >= 0 && b->fds >= 0 && a->fds > b->fds)
        return false;
        
    if (a->bytes    >= 0 && b->bytes    >= 0 && a->bytes    == b->bytes && 
        a->elements >= 0 && b->elements >= 0 && a->elements == b->elements &&
        a->threads  >= 0 && b->threads  >= 0 && a->threads  == b->threads &&
        a->fds      >= 0 && b->fds      >= 0 && a->fds      == b->fds)
        return false;
        
    return true;
}

static
void 
MetricsAdd(SRACacheMetrics* a, const SRACacheMetrics* b)
{
    a->bytes    += b->bytes;
    a->elements += b->elements;
    a->threads  += b->threads;
    a->fds      += b->fds;
}

static
void 
MetricsSubtract(SRACacheMetrics* a, const SRACacheMetrics* b)
{
    a->bytes    -= b->bytes;
    a->elements -= b->elements;
    a->threads  -= b->threads;
    a->fds      -= b->fds;
}

/*================================== SRACacheElement ==================================*/
LIB_EXPORT rc_t CC SRACacheElementMake(SRACacheElement**        self, 
                                       SRATable*                object, 
                                       struct SRACacheIndex*    index, 
                                       uint32_t                 key, 
                                       const SRACacheMetrics*   metrics)
{
    rc_t rc = 0;
    if ( self == NULL || object == NULL || index == NULL || metrics == NULL )
        return RC( rcSRA, rcNode, rcConstructing, rcParam, rcNull );
        
    *self = malloc(sizeof(**self));
    if (self == NULL)
        rc = RC ( rcSRA, rcNode, rcConstructing, rcMemory, rcExhausted );
    else
    {
        (*self)->dad.next = NULL;
        (*self)->dad.prev = NULL;
        
        (*self)->object = object;
        rc = SRATableAddRef(object);
        if ( rc != 0 )
        {
            free (*self);
        }
        else
        {
            (*self)->index = index;
        
            (*self)->key = key;
        
            memmove(&(*self)->metrics, metrics, sizeof((*self)->metrics));
        }
    }
        
    return rc;
}

LIB_EXPORT rc_t CC SRACacheElementDestroy(SRACacheElement* self)
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC( rcSRA, rcNode, rcDestroying, rcSelf, rcNull );
        
    rc = SRATableRelease(self->object);
    
    free(self);
    
    return rc;
}

/*================================== SRACacheIndex ==================================*/
LIB_EXPORT rc_t CC SRACacheIndexMake(SRACacheIndex** self, String* prefix)
{
    rc_t rc = 0;
    if ( self == NULL || prefix == NULL )
        return RC( rcSRA, rcIndex, rcConstructing, rcParam, rcNull );
        
    *self = malloc(sizeof(**self));
    if (self == NULL)
        rc = RC ( rcSRA, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        memset(*self, 0, sizeof(**self));
        rc = StringCopy((const String**)&(*self)->prefix, prefix);
        if (rc == 0)
        {
            rc = KVectorMake(&(*self)->body);
            if (rc != 0)
                StringWhack((*self)->prefix);
        }
        
        if (rc != 0)
        {
            free(*self);
            *self = NULL;
        }
    }
    
    return rc;
}

LIB_EXPORT rc_t CC SRACacheIndexDestroy(SRACacheIndex* self)
{
    rc_t rc;
    if ( self == NULL  )
        return RC( rcSRA, rcIndex, rcDestroying, rcSelf, rcNull );
        
    rc = KVectorRelease(self->body);
    StringWhack(self->prefix);
    free(self);
    return rc;
}

/*================================== SRACache ==================================*/
static
rc_t
ReadValue(struct KConfig* kfg, const char* path, uint64_t* value, uint64_t dflt)
{
    rc_t rc = 0;
    *value = dflt;
    if ( kfg )
    {   /* read configuration values */
        const KConfigNode* node;
        rc = KConfigOpenNodeRead( kfg, &node, "%s", path );
        if (rc == 0)
        {
            uint64_t temp;
            rc = KConfigNodeReadU64( node, &temp );
            if (rc == 0)
                *value = temp;
            rc = KConfigNodeRelease ( node );
        }
        else if (rc == RC ( rcKFG, rcNode, rcOpening, rcPath, rcNotFound ))
            rc = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC SRACacheInit(SRACache** self, struct KConfig* kfg)
{
    rc_t rc = 0;
    if (self == NULL)
        return RC( rcSRA, rcData, rcConstructing, rcSelf, rcNull );
        
    *self = malloc(sizeof(**self));
    if (*self == NULL)
        rc = RC ( rcSRA, rcData, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint64_t v;
#define LOAD_VALUE(target, path, default)                       \
        if (rc == 0)                                            \
        {                                                       \
            rc = ReadValue(kfg, path, &v, default);             \
            if ( rc == 0 )                                      \
                target = v;                                    \
        }

        LOAD_VALUE((*self)->softThreshold.bytes,    "/openserver/thresholds/soft/bytes",    SRACacheThresholdSoftBytesDefault);
        LOAD_VALUE((*self)->softThreshold.elements, "/openserver/thresholds/soft/elements", SRACacheThresholdSoftElementsDefault);
        LOAD_VALUE((*self)->softThreshold.threads,  "/openserver/thresholds/soft/threads",  SRACacheThresholdSoftThreadsDefault);
        LOAD_VALUE((*self)->softThreshold.fds,      "/openserver/thresholds/soft/fds",      SRACacheThresholdSoftFdsDefault);
            
        LOAD_VALUE((*self)->hardThreshold.bytes,    "/openserver/thresholds/hard/bytes",    SRACacheThresholdHardBytesDefault);
        LOAD_VALUE((*self)->hardThreshold.elements, "/openserver/thresholds/hard/elements", SRACacheThresholdHardElementsDefault);
        LOAD_VALUE((*self)->hardThreshold.threads,  "/openserver/thresholds/hard/threads",  SRACacheThresholdHardThreadsDefault);
        LOAD_VALUE((*self)->hardThreshold.fds,      "/openserver/thresholds/hard/fds",      SRACacheThresholdHardFdsDefault);
#undef LOAD_VALUE
    
        if (rc == 0)
        {
            BSTreeInit( & (*self)->indexes );
            DLListInit( & (*self)->lru );
            
            memset(&(*self)->current, 0, sizeof(*self)->current);
            
            (*self)->requests         = 0;
            (*self)->hits             = 0;
            (*self)->misses           = 0;
            (*self)->busy             = 0;
            
            rc = KLockMake(&(*self)->mutex);
        }
    }        
    
    return rc;
}

LIB_EXPORT rc_t CC SRACacheGetSoftThreshold(SRACache* self, SRACacheMetrics* metrics)
{
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
    if (metrics == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcParam, rcNull );
    memmove(metrics, &self->softThreshold, sizeof(self->softThreshold));
    return 0;
}

LIB_EXPORT rc_t CC SRACacheSetSoftThreshold(SRACache* self, const SRACacheMetrics* metrics)
{
    if (self == NULL)
        return RC( rcSRA, rcData, rcUpdating, rcSelf, rcNull );
    if (metrics == NULL)
        return RC( rcSRA, rcData, rcUpdating, rcParam, rcNull );
    memmove(&self->softThreshold, metrics, sizeof(self->softThreshold));
    return 0;
}

LIB_EXPORT rc_t CC SRACacheGetHardThreshold(SRACache* self, SRACacheMetrics* metrics)
{
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
    if (metrics == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcParam, rcNull );
    memmove(metrics, &self->hardThreshold, sizeof(self->hardThreshold));
    return 0;
}

LIB_EXPORT rc_t CC SRACacheSetHardThreshold(SRACache* self, const SRACacheMetrics* metrics)
{
    if (self == NULL)
        return RC( rcSRA, rcData, rcUpdating, rcSelf, rcNull );
    if (metrics == NULL)
        return RC( rcSRA, rcData, rcUpdating, rcParam, rcNull );
    memmove(&self->hardThreshold, metrics, sizeof(self->hardThreshold));
    return 0;
}

static void CC SRACacheIndexDestructor(BSTNode *n, void *data)
{
    SRACacheIndexDestroy((SRACacheIndex*)n);
}
static void CC SRACacheElementDestructor(DLNode *n, void *data)
{
    SRACacheElementDestroy((SRACacheElement*)n);
}

LIB_EXPORT rc_t CC SRACacheWhack(SRACache* self)
{
    rc_t rc = 0;
    if (self == NULL)
        return RC( rcSRA, rcData, rcDestroying, rcSelf, rcNull );

    BSTreeWhack(&self->indexes, SRACacheIndexDestructor, NULL);
    DLListWhack(&self->lru, SRACacheElementDestructor, NULL);
    
    rc = KLockRelease(self->mutex);
    free(self);
    return rc;
}

LIB_EXPORT rc_t CC SRACacheGetUsage(SRACache* self, SRACacheUsage* usage)
{
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
    if (usage == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcParam, rcNull );
    usage->soft_threshold   = self->softThreshold.elements;
    usage->hard_threshold   = self->hardThreshold.elements;
    usage->elements         = self->current.elements;
    usage->requests         = self->requests;
    usage->hits             = self->hits;
    usage->misses           = self->misses;
    usage->busy             = self->busy;
    return 0;
}

static
int64_t CC IndexCmp (const BSTNode * item, const BSTNode * n)
{
    return StringCompare( ((const SRACacheIndex *)item)->prefix, ((const SRACacheIndex *)n)->prefix );
}
    
static
int64_t CC PrefixCmp (const void * item, const BSTNode * n)
{
    return StringCompare( (const String*)item, ((const SRACacheIndex *)n)->prefix );
}
    
static
rc_t
AddNewIndex(BSTree* indexes, String* prefix, SRACacheIndex** newIdx)
{
    rc_t rc = SRACacheIndexMake(newIdx, prefix);
    if (rc == 0)
    {
        rc = BSTreeInsert(indexes, &(*newIdx)->dad, IndexCmp);
        if (rc != 0)
            SRACacheIndexDestroy(*newIdx);
    }
    return rc;
}

static
rc_t
ParseAccessionName(const char *acc, String* prefix, uint32_t* key)
{
    rc_t rc = 0;
    
    /*  extract the alphabetic prefix from the accession's name */
    size_t prefLen = 0;
    while (acc[prefLen] != 0 && isalpha(acc[prefLen]))
        ++prefLen;
    StringInit(prefix, acc, prefLen, prefLen);
    
    {   /* the numeric key will be the rest of the accession's name, if fully numeric */
        char *end;
        *key = strtou32 ( acc + prefLen, & end, 10 );
        if ( end [ 0 ] != 0 )
            rc = RC( rcSRA, rcData, rcAccessing, rcName, rcInvalid );
    }
    return rc;
}

LIB_EXPORT rc_t CC SRACacheAddTable(SRACache* self, const char* acc, SRATable* table)
{ /* if crosses soft threshold, signal BG thread to wake up */
    rc_t rc = 0;
    String prefix;
    uint32_t key;
    
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
    if (acc == NULL || acc[0] == 0 || table == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcParam, rcNull );

    rc = ParseAccessionName(acc, &prefix, &key);
    if (rc == 0)
    {
        rc = KLockAcquire(self->mutex);
        if (rc == 0)
        {
            SRACacheIndex* index = (SRACacheIndex*) BSTreeFind ( &self->indexes, &prefix, PrefixCmp );
            if (index == NULL)
                rc = AddNewIndex( &self->indexes, &prefix, &index );
                
            if (rc == 0)
            {
                SRACacheElement* elem = NULL;
                rc = KVectorGetPtr ( index->body, key, (void**)&elem );   
                if (rc == 0 && elem != NULL) /* table already in cache */
                    rc = RC( rcSRA, rcData, rcInserting, rcParam, rcExists );
                else if (GetRCState(rc) == rcNotFound)
                {
                    SRACacheMetrics metrics;
                    MetricsInit(&metrics, table);
                    rc = SRACacheElementMake(&elem, table, index, key, &metrics);
                    if (rc == 0)
                    {
                        rc = KVectorSetPtr( index->body, key, elem );
                        if (rc != 0)
                            SRACacheElementDestroy(elem);
                        else
                        {   /* success - update global usage and time-sorted list */
                            MetricsAdd(&self->current, &metrics);
                            DLListPushTail( &self->lru, (DLNode*)elem );
                        }
                    }
                }
            }
                
            {
                rc_t rc2 = KLockUnlock(self->mutex);
                if (rc == 0)
                    rc = rc2;
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC SRACacheGetTable(SRACache* self, const char* acc, const SRATable** object)
{
    rc_t rc = 0;
    String prefix;
    uint32_t key;
    
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
    if (acc == NULL || acc[0] == 0 || object == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcParam, rcNull );

    *object = NULL;
    
    rc = ParseAccessionName(acc, &prefix, &key);
    if (rc == 0)
    {
        ++ self->requests;
        
        rc = KLockAcquire(self->mutex);
        if (rc == 0)
        {
            SRACacheIndex* index = (SRACacheIndex*) BSTreeFind ( &self->indexes, &prefix, PrefixCmp );
            if (index != NULL)
            {
                SRACacheElement* elem = NULL;
                rc = KVectorGetPtr ( index->body, key, (void**)&elem );   
                if (rc == 0 && elem != NULL) 
                {   
                    if (atomic32_read(&elem->object->refcount) == 1)    /* owned by cache, not used elsewhere */
                    {
                        ++ self->hits;
                        
                        /* move to the least recently used position */
                        DLListUnlink    ( &self->lru, &elem->dad );
                        DLListPushTail  ( &self->lru, &elem->dad );
                        
                        *object = elem->object;
                        rc = SRATableAddRef(*object);
                    }
                    else
                    {
                        ++ self->busy;
                        rc = RC( rcSRA, rcData, rcAccessing, rcParam, rcBusy);
                    }
                }
                else if (GetRCState(rc) == rcNotFound || elem == NULL)
                {
                    ++ self->misses;
                    rc = 0;
                }
            }
            else
                ++ self->misses;
                
            {
                rc_t rc2 = KLockUnlock(self->mutex);
                if (rc == 0)
                {
                    rc = rc2;
                    if (rc != 0)
                        SRATableRelease(*object);
                }
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC SRACacheFlush(SRACache* self)
{
    rc_t rc = 0;
    if (self == NULL)
        return RC( rcSRA, rcData, rcAccessing, rcSelf, rcNull );
        
    rc = KLockAcquire(self->mutex);
    if (rc == 0)
    {
        /* use the lower of the two thresholds */
        const SRACacheMetrics* thr = &self->softThreshold;
        if ( SRACacheMetricsLessThan( &self->hardThreshold, thr ) )
            thr = &self->hardThreshold;
            
        while ( ! SRACacheMetricsLessThan( &self->current, thr ) )
        {
            SRACacheElement* toFlush = (SRACacheElement*) DLListPopHead( &self->lru );
            if (toFlush == NULL)
                break;
            rc = KVectorUnset( toFlush->index->body, toFlush->key );
            if (rc != 0)
                break; /* something is badly wrong */
            MetricsSubtract( &self->current, &toFlush->metrics );
            rc = SRACacheElementDestroy(toFlush);
            if (rc != 0)
                break;
        }
        
        {
            rc_t rc2 = KLockUnlock(self->mutex);
            if (rc == 0)
                rc = rc2;
        }
    }

    return rc;
}
