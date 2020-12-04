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

#include <kfg/config.h> /* KConfigMake */
#include <kfg/kart-priv.h> /* KartMake2 */
#include <kfg/keystore.h> /* KKeyStoreRelease */
#include <kfg/ngc.h> /* KNgcObj */
#include <kfg/repository.h> /* KRepositoryMgr */

#include <kfs/directory.h> /* KDirectoryOpenFileRead */
#include <kfs/file.h> /* KFile */
#include <kfs/gzip.h> /* KFileMakeGzipForRead */
#include <kfs/subfile.h> /* KFileMakeSubRead */

#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/out.h> /* OUTMSG */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h>
#include <klib/refcount.h> /* KRefcount */
#include <klib/vector.h> /* Vector */

#include <strtol.h> /* strtou64 */
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h> /* free */
#include <string.h> /* memcmp */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

typedef enum {
    eVersion1, // from kart 0x01000000
    eVersion2, // from kart 0x02000000
} Eversion;

struct KartItem {
    KRefcount refcount;

    const Kart *dad;

    Eversion version;

    String projId;
    String itemId;
    String accession; /* 1.0 */
    String name;
    String itemDesc;

    /* 2.0 */
    String objType;
    String path;
    String size;
};

static void KartItemWhack(KartItem *self) {
    assert(self);

    if ( self -> version < eVersion2 ) {
        KartRelease(self->dad);
    } else {
        free ( ( void * ) self -> projId . addr );
    }

    memset(self, 0, sizeof *self);

    free(self);
}

static rc_t KartItemMake2
    ( KartItem ** self, const char * buffer, size_t size )
{
    bool BUG = false;
    rc_t rc = 0;
    KartItem * obj = NULL;
    int i = 0;
    assert ( self );
    obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL ) {
        return RC ( rcKFG, rcData, rcAllocating, rcMemory, rcExhausted );
    }
    obj -> version = eVersion2; /* 0x02000000; */
    for ( i = 0; ; ++i ) {
        size_t l = 0;
        String * next = NULL;
        const char * p = string_chr ( buffer, size, '|' );
        if ( p == NULL ) {
            if ( i != 7 ) {
                rc = RC(rcKFG, rcFile, rcParsing, rcFile, rcInsufficient);
                break;
            }
            l = size;
        }
        else {
            l = p - buffer;
        }
        switch ( i ) {
            case 0:
                next = & obj -> projId;
                break;
            case 1:
                next = & obj -> objType;
                break;
            case 2:
                next = & obj -> itemId;
                break;
            case 3:
                next = & obj -> name;
                break;
            case 4:
                next = & obj -> path;
                break;
            case 5:
                next = & obj -> size;
                break;
            case 6:
                next = & obj -> itemDesc;
                break;
            case 7:
                BUG = true;
                break;
            default:
                rc = RC ( rcKFG, rcFile, rcParsing, rcFile, rcExcessive );
                break;
        }
        if ( ! BUG ) {
            assert ( next );
            StringInit ( next, buffer, l, ( uint32_t ) l );
            if ( l > size ) {
                rc = RC ( rcKFG, rcFile, rcParsing, rcFile, rcInvalid );
            }
        }
        if ( size == l ) {
            break;
        }
        ++ l;
        buffer += l;
        size -= l;
    }
    if (rc == 0) {
        KRefcountInit ( & obj -> refcount, 1,
            "KartItem", "KartItemMake2", "kartitem" );
        * self = obj;
    }
    else {
        free ( obj );
        obj = NULL;
    }
    return rc;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KartItemAddRef(const KartItem *self) {
    if (self != NULL) {
        switch (KRefcountAdd(&self->refcount, "KartItem")) {
            case krefLimit:
                return RC(rcKFG, rcFile, rcAttaching, rcRange, rcExcessive);
        }
    }

    return 0;
}

LIB_EXPORT rc_t CC KartItemRelease(const KartItem *self) {
    if (self != NULL) {
        switch (KRefcountDrop(&self -> refcount, "KartItem")) {
            case krefWhack:
                KartItemWhack((KartItem*)self);
                break;
            case krefLimit:
                return RC(rcKFG, rcFile, rcReleasing, rcRange, rcExcessive);
        }
    }

    return 0;
}

static rc_t StringAsUint64(const String *self, uint64_t *pid) {
    uint64_t id = 0;

    char buffer[21] = "";
    size_t bytes = 0;
    char *end = NULL;

    assert(self);

    if (pid == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcParam, rcNull);
    }

    *pid = 0;

    if (sizeof buffer - 1 < self->len) {
        return RC(rcKFG, rcFile, rcAccessing, rcBuffer, rcInsufficient);
    }

    if (self->len == 0 || self->size == 0) {
        return RC(rcKFG, rcFile, rcAccessing, rcItem, rcEmpty);
    }

    bytes = string_copy(buffer, sizeof buffer, self->addr, self->len);
    if (bytes != self->len) {
        return RC(rcKFG, rcFile, rcAccessing, rcBuffer, rcInsufficient);
    }

    id = strtou64(buffer, &end, 0);
    if (end[0] != 0) {
        return RC(rcKFG, rcFile, rcAccessing, rcParam, rcInvalid);
    }

    *pid = id;

    return 0;
}

LIB_EXPORT
rc_t CC KartItemProjIdNumber(const KartItem *self, uint64_t *pid)
{
    if (self == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcSelf, rcNull);
    }
    return StringAsUint64(&self->projId, pid);
}

LIB_EXPORT
rc_t CC KartItemItemIdNumber(const KartItem *self, uint64_t *pid)
{
    if (self == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcSelf, rcNull);
    }
    return StringAsUint64(&self->itemId, pid);
}

static rc_t KartItemCheck(const KartItem *self, const String **elem) {
    if (elem == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcParam, rcNull);
    }

    *elem = NULL;

    if (self == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcSelf, rcNull);
    }

    return 0;
}

LIB_EXPORT
rc_t CC KartItemProjId(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->projId;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemItemId(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->itemId;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemAccession(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->accession;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemName(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->name;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemItemDesc(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->itemDesc;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemObjType (const KartItem *self, const String **elem )
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->objType;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemPath (const KartItem *self, const String **elem )
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->path;
    }
    return rc;
}
LIB_EXPORT
rc_t CC KartItemSize (const KartItem *self, const String **elem )
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->size;
    }
    return rc;
}
/*LIB_EXPORT rc_t CC KartItemTypeId(const KartItem *self, const String **elem)
{
    rc_t rc = KartItemCheck(self, elem);
    if (rc == 0) {
        *elem = &self->typeId;
    }
    return rc;
}*/

/** Print KartItem using OUTMSG; if (self == NULL) then print the header */
LIB_EXPORT rc_t CC KartItemPrint(const KartItem *self) { /* AA-833 */
    if (self != NULL) {
        return OUTMSG(("'%S'\t'%S'\t'%S'\t'%S'\t'%S'\n", &self->projId,
            &self->itemId, &self->accession, &self->name, &self->itemDesc));
    }
    return 0;
}

struct Kart {
    KRefcount refcount;

    const KRepositoryMgr *mgr;

    Eversion version;

    /* version eVersion1 0x01000000 */
    KDataBuffer mem;
    const char *text;
    uint64_t len;
    uint16_t itemsProcessed;

    KKeyStore *keystore;

    /* version eVersion2 0x02000000 */
    Vector rows;

    const KNgcObj *ngcObj;
};

static void whackKartItem ( void * self, void * ignore ) {
    KartItemRelease ( ( KartItem * ) self);
}

static void KartWhack(Kart *self) {
    assert(self);

    if ( self -> version < eVersion2 ) {
        KDataBufferWhack(&self->mem);
    } else {
        VectorWhack ( & self -> rows, whackKartItem, NULL );
    }

    KKeyStoreRelease ( self -> keystore );

    KRepositoryMgrRelease ( self -> mgr );

    KNgcObjRelease ( self -> ngcObj );

    memset(self, 0, sizeof *self);

    free(self);
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KartAddRef(const Kart *self) {
    if (self != NULL) {
        switch (KRefcountAdd(&self->refcount, "Kart")) {
            case krefLimit:
                return RC(rcKFG, rcFile, rcAttaching, rcRange, rcExcessive);
        }
    }

    return 0;
}

LIB_EXPORT rc_t CC KartRelease(const Kart *self) {
    if (self != NULL) {
        switch (KRefcountDrop(&self -> refcount, "Kart")) {
            case krefWhack:
                KartWhack((Kart*)self);
                break;
            case krefLimit:
                return RC(rcKFG, rcFile, rcReleasing, rcRange, rcExcessive);
        }
    }

    return 0;
}

LIB_EXPORT
rc_t CC KartItemsProcessed(const Kart *self, uint16_t *number)
{
    if (number == NULL) {
        return RC(rcKFG, rcFile, rcLoading, rcParam, rcNull);
    }

    *number = 0;

    if (self == NULL) {
        return RC(rcKFG, rcFile, rcLoading, rcSelf, rcNull);
    }

    *number = self->itemsProcessed;

    return 0;
}

static rc_t KartItemInitFromKartRow(const Kart *self, const KartItem **item,
    const char *line, size_t len)
{
    rc_t rc = 0;
    int i = 0;
    KartItem *obj = NULL;
    assert(self && item && line && len);
    obj = calloc(1, sizeof *obj);
    if (obj == NULL) {
        return RC(rcKFG, rcData, rcAllocating, rcMemory, rcExhausted);
    }
    for (i = 0; ; ++i) {
        size_t l = 0;
        String *next = NULL;
        const char *p = string_chr(line, len, '|');
        if (p == NULL) {
            if (i != 4) {
                rc = RC(rcKFG, rcFile, rcParsing, rcFile, rcInsufficient);
                break;
            }
            l = len;
        }
        else {
            l = p - line;
        }
        switch (i) { /* AA-833 */
            case 0:
                next = &obj->projId;
                break;
            case 1:
                next = &obj->itemId;
                break;
            case 2:
                next = &obj->accession;
                break;
            case 3:
                next = &obj->name;
                break;
            case 4:
                next = &obj->itemDesc;
                break;
            default:
                rc = RC(rcKFG, rcFile, rcParsing, rcFile, rcExcessive);
                break;
        }
        assert(next);
        StringInit(next, line, l, (uint32_t)l);
        if (l > len) {
            rc = RC(rcKFG, rcFile, rcParsing, rcFile, rcInvalid);
        }
        if (len == l) {
            break;
        }
        ++l;
        line += l;
        len -= l;
    }
    if (rc == 0) {
        rc = KartAddRef(self);
    }
    if (rc == 0) {
        KRefcountInit(&obj->refcount, 1, "KartItem", "KartItemInitFromKartRow", "kartitem");
        ++((Kart*)self)->itemsProcessed;
        obj->dad = self;
        *item = obj;
    }
    else {
        free(obj);
        obj = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC KartPrint(const Kart *self) {
    if (self == NULL)
        return RC(rcKFG, rcFile, rcLoading, rcSelf, rcNull);

    if ( self -> version == eVersion1 ) {
        uint32_t l = ( uint32_t ) self -> mem . elem_count;
        OUTMSG ( ( "%.*s", l, self -> mem.base ) );
    }
    else {
        uint32_t i = 0;
        uint32_t l = VectorLength ( & self -> rows );
        for ( i = 0; i < l; ++ i ) {
            KartItem * result = VectorGet ( & self -> rows, i );
            assert ( result );
            OUTMSG ( ( "%S|%S|%S|%S|%S|%S|%S\n", & result -> projId,
                & result -> objType, & result -> itemId, & result -> name,
                & result -> path, & result -> size, & result -> itemDesc ) );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KartPrintNumbered(const Kart *self) {
    rc_t rc = 0;
    rc_t rc2 = 0;
    const char *start = NULL;
    int32_t remaining = 0;
    int32_t len = 0;
    uint32_t i = 0;
    const char *next = NULL;
    bool done = false;

    if (self == NULL) {
        return RC(rcKFG, rcFile, rcAccessing, rcSelf, rcNull);
    }

    if ( self -> version > eVersion1 )
        return RC ( rcKFG, rcFile, rcAccessing, rcInterface, rcBadVersion );

    remaining = (uint32_t)self->mem.elem_count;
    start = self->mem.base;

    {
        const char version[] = "version ";
        size_t l = sizeof version - 1;
        if (string_cmp(version, l, start, remaining, (uint32_t)l) != 0) {
            return RC(rcKFG, rcFile, rcAccessing, rcFormat, rcUnrecognized);
        }
    }

    next = string_chr(start, remaining, '\n');
    if (next == NULL) {
        len = remaining;
    }
    else {
        ++next;
        len = next - start;
    }
    remaining -= len;
    rc2 = OUTMSG(("%.*s", len, start));
    if (rc2 != 0 && rc == 0) {
        rc = rc2;
    }
    start = next;

    rc2 = OUTMSG(("row\tproj-id|item-id|accession|name|item-desc\n"));
    if (rc2 != 0 && rc == 0) {
        rc = rc2;
    }

    for (i = 1; remaining > 0; ++i) {
        if (*start == '$') {
            const char end[] = "$end";
            size_t l = sizeof end - 1;
            if (string_cmp(end, l, start, remaining, (uint32_t)l) != 0) {
                return RC(rcKFG, rcFile, rcAccessing, rcFormat, rcUnrecognized);
            }
            else {
                done = true;
            }
        }
        next = string_chr(start, remaining, '\n');
        if (next == NULL) {
            len = remaining;
        }
        else {
            ++next;
            len = next - start;
        }
        remaining -= len;
        if (done) {
            rc2 = OUTMSG(("%.*s", len, start));
        }
        else {
            rc2 = OUTMSG(("%d\t%.*s", i, len, start));
        }
        if (rc2 != 0 && rc == 0) {
            rc = rc2;
        }
        start = next;
    }

    return rc;
}

LIB_EXPORT rc_t CC KartItemGetTicket(const KartItem *self,
    char * ticket, size_t ticket_size, size_t * written)
{
    if (self == NULL)
        return RC(rcKFG, rcFile, rcAccessing, rcSelf, rcNull);
    return KNgcObjGetTicket(self->dad->ngcObj, ticket, ticket_size, written);
}

static
rc_t KartRegisterObject ( const Kart * self, const KartItem * item )
{
    rc_t rc = 0;

    const KRepository * repo = NULL;

    uint64_t itemId = 0;
    uint64_t projId = 0;
    const String * acc = NULL;
    const String * name = NULL;

    char ticket [ 4096 ] = "";

    char b [ 4096 ] = "";
    String id = { b, 0, 0 }; 

    if ( item == NULL )
        return 0;

    rc = KartItemItemIdNumber ( item, & itemId );
    if ( rc == SILENT_RC ( rcKFG, rcFile, rcAccessing, rcItem, rcEmpty ) ||

        /* not a number (accession) */
         rc == SILENT_RC ( rcKFG, rcFile, rcAccessing, rcParam, rcInvalid ) )
        
    {   return 0; }

    assert ( self );

    if ( rc == 0 )
        rc = KartItemProjIdNumber ( item, & projId );

    if ( rc == 0 )
        rc = KartItemAccession ( item, & acc );

    if ( rc == 0 )
        rc = KartItemName ( item, & name );

    if ( rc == 0 ) {
        rc = KRepositoryMgrGetProtectedRepository ( self -> mgr, projId,
                                                    & repo );
        if ( GetRCModule ( rc ) == rcKFG && GetRCState ( rc ) == rcNotFound )
            rc = RC ( rcKFG, rcNode, rcAccessing, rcNode, rcNotFound );

        if ( rc == 0 ) {
            rc = KRepositoryDownloadTicket( repo, ticket, sizeof ticket, NULL );
            if ( GetRCState ( rc ) == rcNotFound )
                rc = RC ( rcKFG, rcNode, rcAccessing, rcNode, rcNotFound );
        }
        else if (self->ngcObj != NULL)
            rc = KNgcObjGetTicket(self->ngcObj, ticket, sizeof ticket, NULL);

        if ( rc == 0 ) {
            if ( acc != NULL && acc -> size != 0 )
                rc = string_printf ( b, sizeof b, & id . size,
                                 "ncbi-acc:%S?tic=%s", acc, ticket );
            else
                rc = string_printf ( b, sizeof b, & id . size,
                                 "ncbi-file:%S?tic=%s", name, ticket );
            id . len = id . size;
        }
    }

    if ( rc == 0 )
        rc = KKeyStoreRegisterObject ( self->keystore, itemId, & id );

    RELEASE ( KRepository, repo );
    
    return rc;
}

LIB_EXPORT rc_t CC KartReset(const Kart *cself) {
    Kart * self = (Kart *)cself;
    if (self == NULL)
        return RC(rcKFG, rcFile, rcLoading, rcSelf, rcNull);

    if (self->version < eVersion2) {
        self->text = self->mem.base;
        self->len = self->mem.elem_count;
    }
    else
        self->len = 0;

    return 0;
}

LIB_EXPORT
rc_t CC KartMakeNextItem ( const Kart * cself, const KartItem **item )
{
    rc_t rc = 0;

    Kart * self = ( Kart * ) cself;
    size_t len = 0;
    const char *line = NULL;
    const char *next = NULL;
    const KartItem * result = NULL;

    if (item == NULL) {
        return RC(rcKFG, rcFile, rcLoading, rcParam, rcNull);
    }
    *item = NULL;
    if (self == NULL) {
        return RC(rcKFG, rcFile, rcLoading, rcSelf, rcNull);
    }

    if ( self -> version < eVersion2 ) {
        while (self->len > 0
            && (self->text[0] == '\r' || self->text[0] == '\n'))
        {
            ++self->text;
            --self->len;
        }

        line = self->text;
        next = string_chr(self->text, ( size_t ) self->len, '\n');
        if (next == NULL) {
            return RC(rcKFG, rcFile, rcLoading, rcFile, rcInsufficient);
        }

        len = next - self->text;
        if (*(next - 1) == '\r') {
            --len;
        }

        if (self->len >= (uint64_t) (next - self->text + 1) ){
            self->len -= next - self->text + 1;
        }
        else {
            OUTMSG(("WARNING: STRING OVERFLOW DURING KART ROW PARSING"));
            self->len = 0;
        }

        self->text = next + 1;

        {
            const char end[] = "$end";
            if (string_cmp(line, len, end, sizeof end - 1, sizeof end - 1) == 0)
            {
                return 0;
            }
        }

        rc = KartItemInitFromKartRow(self, & result, line, len);
    }
    else {
        uint32_t l = VectorLength ( & self -> rows );
        if ( self -> len < l ) {
            result = VectorGet ( & self -> rows,
                                            ( uint32_t ) self -> len ++ );
            if ( result != NULL ) {
                rc = KartItemAddRef ( result );
            }
        }
    }

    if ( rc == 0 )
        rc = KartRegisterObject ( self, result );

    if ( rc == 0 )
        * item = result;
    else
        KartItemRelease ( result );

    return rc;
}

static
rc_t decode_kart(KDataBuffer *mem, const KFile *orig, size_t hdr_sz)
{
    rc_t rc = 0;
    size_t num_read;
    uint64_t eof;
    assert(mem && orig && hdr_sz);
    rc = KFileSize ( orig, & eof );
    if ( rc == 0 )
    {
        const KFile *sub;
        rc = KFileMakeSubRead(&sub, orig, hdr_sz, eof - hdr_sz);
        if ( rc == 0 )
        {
            const KFile *gzip;
            rc = KFileMakeGzipForRead ( & gzip, sub );
            if ( rc == 0 )
            {
                rc = KDataBufferMakeBytes ( mem, 0 );
                if ( rc == 0 )
                {
                    size_t total, to_read;

                    /* after all of that, we're ready to decompress */
                    for ( total = 0; ; )
                    {
                        char *buff;

                        rc = KDataBufferResize ( mem,
                            total + 32 * 1024 );
                        if ( rc != 0 )
                            break;

                        buff = mem -> base;
                        to_read = ( size_t ) mem -> elem_count - total;

                        rc = KFileReadAll ( gzip, total,
                            & buff [ total ], to_read, & num_read );
                        if ( rc != 0 )
                            break;

                        total += num_read;
                        
                        if ( num_read < to_read )
                        {
                            buff [ total ] = 0;
                            mem -> elem_count = total;
                            break;
                        }
                    }
                }

                KFileRelease ( gzip );
            }

            KFileRelease ( sub );
        }
    }

    return rc;
}

static rc_t KartProcessHeader(Kart *self) {
    assert(self);

    self->text = self->mem.base;
    self->len = self->mem.elem_count;

    {
        const char version[] = "version ";
        size_t l = sizeof version - 1;
        if (string_cmp(version, l,
                       self->text, ( size_t ) self->len, (uint32_t)l) != 0)
        {
            return RC(rcKFG, rcFile, rcAccessing, rcFormat, rcUnrecognized);
        }

        self->text += l;
        self->len -= l;
    }

    {
        const char version[] = "1.0";
        size_t l = sizeof version - 1;
        if (string_cmp(version, l, self->text, l, (uint32_t)l) != 0) {
            return RC(rcKFG, rcFile, rcAccessing, rcFormat, rcUnsupported);
        }

        self->text += l;
        self->len -= l;
    }

    while (self->len > 0 && (self->text[0] == '\r' || self->text[0] == '\n')) {
        ++self->text;
        --self->len;
    }

    return 0;
}

#ifdef _DEBUGGING
static rc_t read_textkart(KDataBuffer *mem, const KFile *orig) {
    rc_t rc = 0;
    size_t num_read;
    uint64_t eof;
    assert(mem && orig);
    rc = KFileSize ( orig, & eof );
    if ( rc == 0 )
    {
        rc = KDataBufferMakeBytes ( mem, 0 );
        if ( rc == 0 ) {
            /* after all of that, we're ready to read */
            rc = KDataBufferResize(mem, eof);
            if ( rc != 0 )
                return rc;
            rc = KFileReadAll ( orig, 0, mem -> base, ( size_t ) eof,
                                & num_read );
            if ( rc != 0 )
                return rc;
        }
    }
    return rc;
}
KFG_EXTERN rc_t CC KartMakeText(const struct KDirectory *dir, const char *path,
    Kart **kart, bool *isKart)
{
    rc_t rc = 0;
    const KFile *f = NULL;

    if (dir == NULL || path == NULL || kart == NULL || isKart == NULL) {
        return RC(rcKFG, rcFile, rcReading, rcParam, rcNull);
    }

    *isKart = false;
    *kart = NULL;

    rc = KDirectoryOpenFileRead(dir, &f, "%s", path);
    if (rc != 0) {
        return rc;
    }

    {
        Kart *obj = NULL;

        *isKart = true;

        obj = calloc(1, sizeof *obj);
        if (obj == NULL) {
            return RC(rcKFG, rcData, rcAllocating, rcMemory, rcExhausted);
        }

        rc = read_textkart(&obj->mem, f);
        if (rc == 0) {
            rc = KartProcessHeader(obj);
        }
        if (rc == 0) {
            KRefcountInit(&obj->refcount, 1, "Kart", "MakeText", "kart");
            *kart = obj;
        }
        else {
            KartWhack(obj);
        }
    }

    RELEASE(KFile, f);
    return rc;
}
#endif

LIB_EXPORT rc_t CC KartMake2 ( Kart ** kart ) {
    Kart * obj = NULL;

    if ( kart == NULL )
        return RC ( rcKFG, rcFile, rcReading, rcParam, rcNull );

    obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL )
        return RC ( rcKFG, rcData, rcAllocating, rcMemory, rcExhausted );

    obj -> version = eVersion2;

    KRefcountInit ( & obj->refcount, 1, "Kart", "KartMake2", "kart" );

    * kart = obj;

    return 0;
}

LIB_EXPORT rc_t CC KartAddRow ( Kart * self,
    const char * row, size_t size )
{
    if ( self == NULL )
        return RC ( rcKFG, rcFile, rcUpdating, rcSelf, rcNull );
    if ( row == NULL )
        return RC ( rcKFG, rcFile, rcUpdating, rcParam, rcNull );

    if ( self -> version < eVersion2 )
        return RC ( rcKFG, rcFile, rcUpdating, rcInterface, rcBadVersion );

    {
        rc_t rc = 0;

        KartItem * item = NULL;

        const char * p = string_dup ( row, size );
        if ( p == NULL )
            return RC ( rcKFG, rcFile, rcUpdating, rcMemory, rcExhausted );

        rc = KartItemMake2 ( & item, p, size );
        if ( rc == 0 ) {
            rc = VectorAppend ( & self -> rows, NULL, item );

            if ( rc != 0 )
                KartItemRelease ( item );
        }

        return rc;
    }
}

static rc_t KartNgcInit(Kart * self,
    const char *ngcPath, const KDirectory *dir)
{
    if (ngcPath == NULL)
        return 0;

    else {
        const KFile * f = NULL;
        rc_t rc = KDirectoryOpenFileRead(dir, &f, "%s", ngcPath);

        if (rc == 0) {
            assert(self);
            rc = KNgcObjMakeFromFile(&self->ngcObj, f);
        }

        RELEASE(KFile, f);

        return rc;
    }
}

LIB_EXPORT rc_t CC KartMakeWithNgc(const KDirectory *dir, const char *path,
    Kart **kart, bool *isKart, const char *ngcPath)
{
    rc_t rc = 0;
    const KFile *f = NULL;
    char hdr[8] = "";
    size_t num_read = 0;

    if (dir == NULL || path == NULL || kart == NULL || isKart == NULL) {
        return RC(rcKFG, rcFile, rcReading, rcParam, rcNull);
    }

    *isKart = false;
    *kart = NULL;

    rc = KDirectoryOpenFileRead(dir, &f, "%s", path);
    if (rc != 0) {
        return rc;
    }

    rc = KFileReadAll(f, 0, hdr, sizeof hdr, &num_read);
    if (rc == 0 && num_read == sizeof hdr &&
        memcmp(hdr, "ncbikart", sizeof hdr) == 0)
    {
        KConfig * kfg = NULL;

        Kart *obj = NULL;

        *isKart = true;

        obj = calloc(1, sizeof *obj);
        if (obj == NULL) {
            return RC(rcKFG, rcData, rcAllocating, rcMemory, rcExhausted);
        }

        rc = decode_kart(&obj->mem, f, sizeof hdr);
        if (rc == 0) {
            rc = KartProcessHeader(obj);
        }

        if (rc == 0)
            rc = KConfigMake ( & kfg , NULL );
        if ( rc == 0 )
            rc = KKeyStoreMake ( & obj -> keystore, kfg );
        if ( rc == 0 )
            rc = KConfigMakeRepositoryMgrRead ( kfg, & obj -> mgr );
        RELEASE ( KConfig, kfg );

        if (rc == 0)
            rc = KartNgcInit(obj, ngcPath, dir);

        if (rc == 0) {
            KRefcountInit(&obj->refcount, 1, "Kart", "Make", "kart");
            *kart = obj;
        }
        else {
            KartWhack(obj);
        }
    }

    RELEASE(KFile, f);

    return rc;
}

LIB_EXPORT rc_t CC KartMake(const KDirectory *dir, const char *path,
    Kart **kart, bool *isKart)
{
    return KartMakeWithNgc(dir, path, kart, isKart, NULL);
}
