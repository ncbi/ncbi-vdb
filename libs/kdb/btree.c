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

#define BTREE_KEY2ID 1

#include <kdb/extern.h>
#include <kdb/btree.h>
#include <kfs/file.h>
#include <kfs/pagefile.h>
#include <klib/refcount.h>
#include <klib/btree.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KBTree
 *  this implementation is an extremely simplified structure
 *  meant to provide the ability to create an index for temporary use
 */
#define eByteOrderTag 0x05031988
#define eByteOrderReverse 0x88190305


/* v3 does not store values, but stores keys in node pages */
typedef struct KBTreeHdr_v3 KBTreeHdr_v3;
struct KBTreeHdr_v3
{
    /* last entry id */
    uint32_t id_seq;

    /* key min/max */
    uint16_t key_min, key_max;

    /* type [ 0 ] is type
       type [ 1 ] is non-zero if comparison function was used
       rest are for alignment */
    uint8_t type [ 4 ];

    /* tree root */
    uint32_t root;

    /* next to last */
    uint32_t version;

    /* last */
    uint32_t endian;
};

struct Pager {
    KPageFile *pager;
    rc_t rc;
};

static void const *PagerAlloc(Pager *self, uint32_t *newid)
{
    KPage *page = NULL;
    self->rc = KPageFileAlloc(self->pager, &page, newid);
    return (void const *)page;
}

static void const *PagerUse(Pager *self, uint32_t pageid)
{
    KPage *page = NULL;
    self->rc = KPageFileGet(self->pager, &page, pageid);
    return (void const *)page;
}

static void const *PagerAccess(Pager *self, void const *page)
{
    void const *mem = NULL;
    self->rc = KPageAccessRead((KPage const *)page, &mem, NULL);
    return mem;
}

static void *PagerUpdate(Pager *self, void const *page)
{
    void *mem = NULL;
    self->rc = KPageAccessUpdate((KPage *)page, &mem, NULL);
    return mem;
}

static void PagerUnuse(Pager *self, void const *page)
{
    KPageRelease((KPage const *)page);
}

static Pager_vt const KPageFile_vt = {
    PagerAlloc,
    PagerUse,
    PagerAccess,
    PagerUpdate,
    PagerUnuse
};

typedef struct KBTreeHdr_v3 KBTreeHdr;

static
rc_t KBTreeReadHeader ( KBTreeHdr *hdr, const KFile *f )
{
    uint64_t eof;
    rc_t rc = KFileSize ( f, & eof );
    if ( rc == 0 )
    {
        size_t num_read;

        /* this would be an empty file */
        if ( eof == 0 )
        {
            memset ( hdr, 0, sizeof * hdr );
            return RC ( rcDB, rcTree, rcConstructing, rcData, rcNotFound );
        }

        if ( eof < sizeof * hdr )
            return RC ( rcDB, rcTree, rcConstructing, rcData, rcCorrupt );

        rc = KFileReadAll ( f, eof - sizeof * hdr, hdr, sizeof * hdr, & num_read );
        if ( rc == 0 && num_read != sizeof * hdr )
            rc = RC ( rcDB, rcTree, rcConstructing, rcData, rcInsufficient );
        if ( rc == 0 )
        {
            if ( hdr -> endian != eByteOrderTag )
            {
                if ( hdr -> endian == eByteOrderReverse )
                    return RC ( rcDB, rcTree, rcConstructing, rcByteOrder, rcIncorrect );
                return RC ( rcDB, rcTree, rcConstructing, rcData, rcCorrupt );
            }
            if ( hdr -> version != 2 )
                return RC ( rcDB, rcTree, rcConstructing, rcHeader, rcBadVersion );
        }
    }
    return rc;
}

struct KBTree
{
    /* file itself */
    KFile *file;

    /* page cache layered on top */
    Pager pgfile;

    /* "header" is stored at end */
    KBTreeHdr hdr;

    KRefcount refcount;

    bool read_only;
};

/* Whack
 */
static
rc_t KBTreeWhack ( KBTree *self )
{
    if ( self -> read_only || self -> file == NULL )
        KPageFileRelease ( self -> pgfile.pager );
    else
    {
        size_t num_writ;

        /* request page file size */
        uint64_t eof;
        rc_t rc = KPageFileSize ( self -> pgfile.pager, & eof, NULL, NULL );
        if ( rc != 0 )
            return rc;

        /* drop the page file and its cache */
        KPageFileRelease ( self -> pgfile.pager );

        /* write header to tail */        
        rc = KFileWrite ( self -> file, eof, & self -> hdr, sizeof self -> hdr, & num_writ );
        if ( rc == 0 && num_writ != sizeof self -> hdr )
            rc = RC ( rcDB, rcTree, rcPersisting, rcTransfer, rcIncomplete );
        if ( rc == 0 )
            rc = KFileSetSize ( self -> file, eof + sizeof self -> hdr );
        if ( rc != 0 )
        {
            /* TBD - can issue a warning here */
        }
    }

    KFileRelease ( self -> file );
    free ( self );
    return 0;
}


/* MakeRead
 * MakeUpdate
 *  make a b-tree object backed by supplied KFile
 *
 *  "backing" [ IN ] - open file with read permissions.
 *   NB - a reference will be attached to this file.
 *
 *  "climit" [ IN ] - cache limit in bytes. the internal cache will
 *   retain UP TO ( but not exceeding ) the limit specified. a value
 *   of 0 ( zero ) will disable caching.
 *
 *  "cmp" [ IN, NULL OKAY ] - optional comparison callback function for opaque keys.
 *   specific key types will use internal comparison functions. for opaque keys, a
 *   NULL function pointer will cause ordering by size and binary comparison.
 */
LIB_EXPORT rc_t CC KBTreeMakeRead_1 ( const KBTree **btp,
    const KFile *backing, size_t climit )
{
    rc_t rc;

    if ( btp == NULL )
        rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcNull );
    else
    {
        if ( backing == NULL )
            rc = RC ( rcDB, rcTree, rcConstructing, rcFile, rcNull );
        else
        {
            KBTree *bt = malloc ( sizeof * bt );
            if ( bt == NULL )
                rc = RC ( rcDB, rcTree, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KBTreeReadHeader ( & bt -> hdr, backing );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( backing );
                    if ( rc == 0 )
                    {
                        /* create page file */
                        rc = KPageFileMakeRead ( ( const KPageFile** ) & bt -> pgfile.pager, backing, climit );
                        if ( rc == 0 )
                        {
                            /* ready to go */
                            bt -> file = ( KFile* ) backing;
                            KRefcountInit ( & bt -> refcount, 1, "KBTree", "make-read", "btree" );
                            bt -> read_only = true;

                            * btp = bt;
                            return 0;
                        }

                        KFileRelease ( backing );
                    }
                }

                free ( bt );
            }
        }

        * btp = NULL;
    }

    return rc;
}


/* MakeUpdate
 *  make a b-tree object backed by supplied KFile
 *
 *  "backing" [ IN ] - open file with read & write permissions.
 *   NB - a reference will be attached to this file.
 *
 *  "climit" [ IN ] - cache limit in bytes. the internal cache will
 *   retain UP TO ( but not exceeding ) the limit specified. a value
 *   of 0 ( zero ) will disable caching.
 *
 *  "write_through" [ IN ] - if true, causes flushing of modified page
 *   after its value is released
 *
 *  "type" [ IN ] - describes the key type ( see above )
 *
 *  "key_chunk_size" [ IN ] - the "chunking" ( alignment ) factor for
 *   storing keys, rounded up to the nearest power of 2.
 *
 *  "value_chunk_size" [ IN ] - chunking factor for values
 *   ( see "key_chunk_size" )
 *
 *  "min_key_size" [ IN ] and "max_key_size" [ IN ] - specifies the allowed
 *   opaque key sizes. min == max implies fixed size. ignored for well
 *   known fixed size key types.
 *
 *  "min_value_size" [ IN ] and "max_value_size" [ IN ] - specifies the allowed
 *   value sizes. min == max implies fixed size.
 *
 *  "cmp" [ IN ] - comparison callback function for opaque keys.
 */
LIB_EXPORT rc_t CC KBTreeMakeUpdate_1 ( KBTree **btp, KFile *backing,
    size_t climit )
{
    rc_t rc;

    if ( btp == NULL )
        rc = RC ( rcDB, rcTree, rcConstructing, rcParam, rcNull );
    else
    {
        {
            KBTree *bt = calloc ( 1,sizeof * bt );
            if ( bt == NULL )
                rc = RC ( rcDB, rcTree, rcConstructing, rcMemory, rcExhausted );
            else
            {
                if ( backing == NULL || ( rc = KBTreeReadHeader ( & bt -> hdr, backing )) == 0 || GetRCState ( rc ) == rcNotFound )
                {
                    /* detect empty file */
                    if ( bt -> hdr . version == 0 )
                    {
                        assert ( bt -> hdr . id_seq == 0 );
                        bt -> hdr . type [ 0 ] = 0;
                        bt -> hdr . type [ 1 ] = 0;
                        bt -> hdr . key_min = 0;
                        bt -> hdr . key_max = 0;
                        bt -> hdr . version = 3;
                        bt -> hdr . endian = eByteOrderTag;
                        rc = 0;
                    }
                    else
                    {
                        /* check for parameter equivalence */
                        if ( bt -> hdr . version < 3 )
                            rc = RC ( rcDB, rcTree, rcConstructing, rcHeader, rcBadVersion );
                    }

                    if ( rc == 0 )
                    {
                        if(backing) rc = KFileAddRef ( backing );
                        if ( rc == 0 )
                        {
                            /* create page file */
                            rc = KPageFileMakeUpdate ( & bt -> pgfile.pager, backing, climit, false );
                            if ( rc == 0 )
                            {
                                /* ready to go */
                                bt -> file = backing;
                                KRefcountInit ( & bt -> refcount, 1, "KBTree", "make-update", "btree" );
                                bt -> read_only = false;

                                * btp = bt;
                                return 0;
                            }

                            if(backing) KFileRelease ( backing );
                        }
                    }
                }

                free ( bt );
            }
        }

        * btp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KBTreeAddRef ( const KBTree *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KBTree" ) )
    {
    case krefOkay:
        break;
    default:
        return RC ( rcDB, rcTree, rcAttaching, rcConstraint, rcViolated );
    }
    return 0;
}

LIB_EXPORT rc_t CC KBTreeRelease ( const KBTree *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KBTree" ) )
    {
    case krefOkay:
        break;
    case krefWhack:
        return KBTreeWhack ( ( KBTree* ) self );
    default:
        return RC ( rcDB, rcTree, rcReleasing, rcConstraint, rcViolated );
    }
    return 0;
}


/* DropBacking
 *  used immediately prior to releasing
 *  prevents modified pages from being flushed to disk
 *  renders object nearly useless
 */
LIB_EXPORT rc_t CC KBTreeDropBacking ( KBTree *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcTree, rcDetaching, rcSelf, rcNull );

    rc = KPageFileDropBacking ( self -> pgfile.pager );
    if ( rc == 0 )
    {
        rc = KFileRelease ( self -> file );
        if ( rc == 0 )
            self -> file = NULL;
    }

    return rc;
}


/* Size
 *  returns size in bytes of file and cache
 *
 *  "lsize" [ OUT, NULL OKAY ] - return parameter for logical size
 *
 *  "fsize" [ OUT, NULL OKAY ] - return parameter for file size
 *
 *  "csize" [ OUT, NULL OKAY ] - return parameter for cache size
 */
LIB_EXPORT rc_t CC KBTreeSize ( const KBTree *self,
    uint64_t *lsize, uint64_t *fsize, size_t *csize )
{
    size_t dummysz;
    uint64_t dummy64;

    if ( self != NULL )
        return KPageFileSize ( self -> pgfile.pager, lsize, fsize, csize );

    if ( lsize == NULL )
        lsize = & dummy64;
    if ( fsize == NULL )
        fsize = & dummy64;
    if ( csize == NULL )
        csize = & dummysz;

    * lsize = 0;
    * fsize = 0;
    * csize = 0;

    return RC ( rcDB, rcTree, rcAccessing, rcSelf, rcNull );
}


/* Find
 *  searches for a match
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
LIB_EXPORT rc_t CC KBTreeFind ( const KBTree *self, uint64_t *id,
    const void *key, size_t key_size )
{
    rc_t rc;

    if ( id == NULL )
        rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTree, rcSelecting, rcSelf, rcNull );
        else if ( key_size == 0 )
            rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcEmpty );
        else if ( key == NULL )
            rc = RC ( rcDB, rcTree, rcSelecting, rcParam, rcNull );
        else if ( self -> hdr . root == 0 )
            rc = RC ( rcDB, rcTree, rcSelecting, rcItem, rcNotFound );
        else
        {
            uint32_t id32 = 0;
            rc = BTreeFind(self->hdr.root, (Pager *)&self->pgfile, &KPageFile_vt, &id32, key, key_size);
            if (self->pgfile.rc)
                rc = self->pgfile.rc;
            *id = id32;
        }
    }

    return rc;
}


/* Entry
 *  searches for a match or creates a new entry
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "was_inserted" [ OUT ] - if true, the returned value was the result of an
 *   insertion and can be guaranteed to be all 0 bits. otherwise, the returned
 *   value will be whatever was there previously.
 *
 *  "alloc_size" [ IN ] - the number of value bytes to allocate upon insertion,
 *   i.e. if the key was not found. this value must agree with the limits
 *   specified in Make ( see above ).
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
LIB_EXPORT rc_t CC KBTreeEntry ( KBTree *self, uint64_t *id,
    bool *was_inserted, const void *key, size_t key_size )
{
    rc_t rc;

    bool dummy;
    if ( was_inserted == NULL )
        was_inserted = & dummy;
    * was_inserted = false;

    if ( id == NULL )
        rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTree, rcUpdating, rcSelf, rcNull );
        else if ( key_size == 0 )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcEmpty );
        else if ( key == NULL )
            rc = RC ( rcDB, rcTree, rcUpdating, rcParam, rcNull );
        else
        {
            uint32_t id32 = *id;
            rc = BTreeEntry(&self->hdr.root, (Pager *)&self->pgfile, &KPageFile_vt, &id32, was_inserted, key, key_size);
            if (self->pgfile.rc)
                rc = self->pgfile.rc;
            *id = id32;
        }
    }

    return rc;
}


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */
LIB_EXPORT rc_t CC KBTreeForEach ( const KBTree *self, bool reverse, void ( CC * f ) ( const void *key, size_t key_size, uint32_t id, void *data ), void *data )
{
    if ( self == NULL )
        return RC ( rcDB, rcTree, rcVisiting, rcSelf, rcNull );
    else if ( f == NULL )
        return RC ( rcDB, rcTree, rcVisiting, rcFunction, rcNull );
    else
        return BTreeForEach(self->hdr.root, (Pager *)&self->pgfile, &KPageFile_vt, reverse, f, data);
}
