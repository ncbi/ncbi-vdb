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

#include <kdb/extern.h>
#include "windex-priv.h"
#include "kdbfmt-priv.h"
#include <klib/ptrie.h>
#include <klib/text.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
#include <assert.h>

#define DISABLE_PROJ 0
#define LIMIT_INSERTS 0

#if LIMIT_INSERTS
#define INSERT_LIMIT 100000000
#endif

/*--------------------------------------------------------------------------
 * KPTrieIndex_v1
 *  persisted keymap
 */

/* KPTrieIndexInit
 *  opens and initializes persisted keymap structure
 */
rc_t KPTrieIndexInit_v1 ( KPTrieIndex_v1 *self, const KMMap *mm, bool byteswap )
{
    size_t size;
    rc_t rc = KMMapSize ( mm, & size );
    if ( rc == 0 )
    {
        const KDBHdr *hdr;
        rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
        if ( rc == 0 )
        {
            /* try to create the pttree */
            rc = PTrieMakeOrig ( & self -> key2id,
                hdr + 1, size -= sizeof * hdr, byteswap );
            if ( rc == 0 )
            {
                size_t ptsize = PTrieSize ( self -> key2id );
                if ( ptsize <= size )
                {
                    /* just in case */
                    self -> mm = NULL;

                    /* record for projection */
                    self -> byteswap = byteswap;

                    /* it could be stored without projection */
                    if ( ptsize == size )
                    {
                        self -> id2node = NULL;
                        self -> first = self -> last = 0;
                        return 0;
                    }
                            
                    /* assume this is projection index */
                    self -> id2node = ( void* )
                        ( ( char* ) ( hdr + 1 ) + ptsize );
                    size -= ptsize;
                            
                    /* it must have at least 4 bytes
                       and be 4 byte aligned */
                    if ( size >= sizeof ( uint32_t ) && ( size & 3 ) == 0 )
                    {
                        /* first entry is starting key
                           remaining entries are node ids */
                        self -> first = * self -> id2node ++;
                        size -= sizeof self -> id2node [ 0 ];
                        if ( size == 0 )
                        {
                            /* forget if empty */
                            self -> id2node = NULL;
                            self -> first = self -> last = 0;
                            return 0;
                        }
                        /* remaining entries */
                        self -> last = self -> first + ( size >> 2 ) - 1;
                        return 0;
                    }
                }

                PTrieWhack ( self -> key2id );
                self -> key2id = NULL;

                rc = RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );
            }
        }
    }

    return rc;
}

/* KPTrieIndexWhack_v1
 *  closes down keymap
 */
void KPTrieIndexWhack_v1 ( KPTrieIndex_v1 *self )
{
    PTrieWhack ( self -> key2id );
    KMMapRelease ( self -> mm );
    memset ( self, 0, sizeof * self );
}


/*--------------------------------------------------------------------------
 * KTrieIdxNode_v1
 */

static
void CC KTrieIdxNodeWhack_v1 ( TNode *n, void *data )
{
    TNodeWhack ( n );
}

static
void CC KTrieIdxNodeUnlink_v1 ( TNode *n, void *data )
{
    if ( TrieUnlink ( data, n ) )
        TNodeWhack ( n );
}

static
void CC KTrieIdxNodeCaptureID_v1 ( TNode *n, void *data )
{
    KTrieIndex_v1 *self = data;
    KTrieIdxNode_v1 *node = ( KTrieIdxNode_v1* ) n;
    self -> id2node [ node -> id - self -> first ] = node;
}


/*--------------------------------------------------------------------------
 * KTrieIndex_v1
 */

/* KTrieIndexWrite_v1
 */
typedef struct PersistTrieData PersistTrieData;
struct PersistTrieData
{
    uint64_t pos;
    KFile *f;
    uint8_t *buffer;
    size_t bsize;
    size_t marker;

    size_t ptt_size;
    uint32_t first;
    uint32_t last;
    rc_t rc;
};

static
rc_t CC KTrieIndexWrite_v1 ( void *param,
    const void *buffer, size_t size, size_t *num_writ )
{
    PersistTrieData *pb = param;
    size_t total, to_write;

    for ( total = 0; total < size; total += to_write )
    {
        to_write = size - total;
        if ( pb -> marker + to_write > pb -> bsize )
            to_write = pb -> bsize - pb -> marker;

        if ( to_write > 0 )
        {
            memmove ( pb -> buffer + pb -> marker,
                ( const uint8_t* ) buffer + total, to_write );
            pb -> marker += to_write;
        }

        if ( pb -> marker == pb -> bsize )
        {
            size_t num_flushed;
            pb -> rc = KFileWrite ( pb -> f, pb -> pos,
                pb -> buffer, pb -> bsize, & num_flushed );
            if ( pb -> rc != 0 )
            {
                * num_writ = 0;
                return pb -> rc;
            }

            if ( num_flushed == 0 )
            {
                * num_writ = total + to_write;
                return pb -> rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
            }

            pb -> marker -= num_flushed;
            pb -> pos += num_flushed;

            if ( pb -> marker != 0 )
                memmove ( pb -> buffer, pb -> buffer + num_flushed, pb -> marker );
        }
    }

    * num_writ = total;
    return 0;
}

/* KTrieIndexAux_v1
 */
static
rc_t CC KTrieIndexAux_v1 ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    const KTrieIdxNode_v1 *n = node;

    if ( write != NULL )
    {
        PersistTrieData *pb = param;
        if ( n -> id < pb -> first )
            pb -> first = n -> id;
        if ( n -> id > pb -> last )
            pb -> last = n -> id;

        return ( * write ) ( write_param, & n -> id, sizeof n -> id, num_writ );
    }

    * num_writ = sizeof n -> id;
    return 0;
}

/* KTrieIndexPersist_v1
 *  write keymap to indicated location
 */
static
rc_t KTrieIndexPersistTrie_v1 ( const KTrieIndex_v1 *self, PersistTrieData *pb )
{
    rc_t rc;
    KDBHdr *hdr;

    pb -> pos = 0;

    hdr = ( KDBHdr* ) pb -> buffer;
    KDBHdrInit ( hdr, 1 );
    pb -> marker = sizeof * hdr;

    /* persist the trie to file,
       using tree-internal key storage,
       capture the image size in pb */
    rc = TriePersist ( & self -> key2id, & pb -> ptt_size,
        false, KTrieIndexWrite_v1, pb, KTrieIndexAux_v1, pb );
    if ( rc == 0 && pb -> marker != 0 )
    {
        size_t num_writ;
        rc = KFileWrite ( pb -> f, pb -> pos,
            pb -> buffer, pb -> marker, & num_writ );
        if ( rc == 0 && num_writ != pb -> marker )
            rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
    }

    return rc;
}


typedef struct PersistReverseData PersistReverseData;
struct PersistReverseData
{
    PTrie *tt;
    uint32_t *a;
    uint32_t first;
    uint32_t count;
    uint32_t notfound;
};

static
void CC KTrieIndexRecordNodeId_v1 ( TNode *node, void *data )
{
    PTNode pn;
    PersistReverseData *pb = data;
    const KTrieIdxNode_v1 *n = ( const KTrieIdxNode_v1* ) node;

    /* lookup up the persisted node within image */
    uint32_t id = PTrieFind ( pb -> tt, & n -> n . key, & pn, NULL, NULL );

    /* write it into array */
    pb -> a [ n -> id - pb -> first ] = ( uint32_t ) id;

    if ( id == 0 )
        ++ pb -> notfound;
    else
        ++ pb -> count;
}

static
rc_t KTrieIndexPersistProj_v1 ( const KTrieIndex_v1 *self, PersistTrieData *pb )
{
#if 0
    rc_t rc;
    KMMap *mmr;
    size_t map_size;

    /* there must be something to do */
    if ( pb -> first > pb -> last )
        return 0;


    /* open a read-write map onto file
       start at offset 0, which is the header,
       followed by pb -> ptt_size bytes which is the tree,
       and add on a slot for first id,
       followed by last - first + 1 slots for id to node id map */
    map_size = pb -> ptt_size + ( ( pb -> last - pb -> first + 2 ) << 2 );

    rc = KMMapMakeRgnUpdate ( & mmr, pb -> f, sizeof ( KDBHdr ), map_size );
    if ( rc == 0 )
    {
        void *addr;
        PersistReverseData pb2;

        rc = KMMapAddrUpdate ( mmr, & addr );
        if ( rc == 0 )
        {
            rc = PTrieMakeOrig ( & pb2 . tt, addr, pb -> ptt_size );
            if ( rc == 0 )
            {
                assert ( pb -> ptt_size == PTrieSize ( pb2 . tt ) );
                pb2 . a = ( void* ) ( ( char* ) addr + pb -> ptt_size );
                assert ( ( ( size_t ) pb2 . a & 3 ) == 0 );

                /* record first id */
                * pb2 . a ++ = pb -> first;
                pb2 . first = pb -> first;
                pb2 . count = pb2 . notfound = 0;

                /* record all id to node mappings */
                TrieForEach ( & self -> key2id, KTrieIndexRecordNodeId_v1, & pb2 );

                /* check for having written ids */
                assert ( pb2 . count == PTrieCount ( pb2 . tt ) );

                /* done with pttree */
                PTrieWhack ( pb2 . tt );

                /* if there were nodes not found,
                   the initial persist was bad */
                if ( pb2 . notfound != 0 )
                    rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
            }
        }

        /* done with map - commits changes to disk */
        KMMapRelease ( mmr );

        /* truncate file to desired size */
        KFileSetSize ( pb -> f, map_size + sizeof ( KDBHdr ) );
    }

    return rc;
#else
    rc_t rc;
    void * addr;
    uint64_t file_size;
    size_t num_to_read;
    size_t map_size;

    /* there must be something to do */
    if ( pb -> first > pb -> last )
        return 0;

    /* open a read-write map onto file
       start at offset 0, which is the header,
       followed by pb -> ptt_size bytes which is the tree,
       and add on a slot for first id,
       followed by last - first + 1 slots for id to node id map */
    map_size = pb -> ptt_size + ( ( pb -> last - pb -> first + 2 ) << 2 );

    rc = KFileSize ( pb -> f, & file_size );
    if ( rc == 0 )
    {
        addr = malloc ( map_size );
        if ( addr == NULL )
            rc = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
        else
        {
            size_t num_read;
            num_to_read = file_size - sizeof ( KDBHdr );
            rc = KFileReadAll ( pb -> f, sizeof ( KDBHdr ),
                addr, num_to_read, & num_read );
            if ( rc != 0 )
                free ( addr );
            else if ( num_read != num_to_read )
            {
                free ( addr );
                rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
            }
        }
    }

    if ( rc == 0 )
    {
        size_t num_writ;
        PersistReverseData pb2;

        rc = PTrieMakeOrig ( & pb2 . tt, addr, pb -> ptt_size, false );
        if ( rc == 0 )
        {
            assert ( pb -> ptt_size == PTrieSize ( pb2 . tt ) );
            pb2 . a = ( void* ) ( ( char* ) addr + pb -> ptt_size );
            assert ( ( ( size_t ) pb2 . a & 3 ) == 0 );

            /* record first id */
            * pb2 . a ++ = pb -> first;
            pb2 . first = pb -> first;
            pb2 . count = pb2 . notfound = 0;

            /* record all id to node mappings */
            TrieForEach ( & self -> key2id, KTrieIndexRecordNodeId_v1, & pb2 );

            /* check for having written ids */
            assert ( pb2 . count == PTrieCount ( pb2 . tt ) );

            /* done with pttree */
            PTrieWhack ( pb2 . tt );

            /* if there were nodes not found,
               the initial persist was bad */
            if ( pb2 . notfound != 0 )
                rc = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
            else
            {
                rc = KFileWrite ( pb -> f, file_size, ( uint8_t* ) addr + num_to_read,
                    map_size - num_to_read, & num_writ );
                if ( rc == 0  && num_writ != map_size - num_to_read )
                    rc = RC ( rcDB, rcIndex, rcPersisting, rcHeader, rcInsufficient );
            }
        }

        /* done with map - commits changes to disk */
        free ( addr );

        /* truncate file to desired size */
        KFileSetSize ( pb -> f, map_size + sizeof ( KDBHdr ) );
    }

    return rc;
#endif
}

rc_t KTrieIndexPersist_v1 ( const KTrieIndex_v1 *self,
    bool proj, KDirectory *dir, const char *path, bool use_md5 )
{
    rc_t rc;
    PersistTrieData pb;

    assert ( self != NULL );

    pb . buffer = malloc ( pb . bsize = 32 * 1024 );
    if ( pb . buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
    else
    {
        char tmpname [ 256 ];
        char tmpmd5name [ 256 ];
        char md5path [ 256 ];
        rc = KDirectoryResolvePath ( dir, false,
            tmpname, sizeof tmpname, "%s.tmp", path );
        if ( rc == 0 )
        {
            rc = KDirectoryCreateFile ( dir, & pb . f,
                                         true, 0664, kcmInit, "%s", tmpname );

	    if (use_md5 && rc == 0 )
	    {
		size_t tmplen = snprintf ( tmpmd5name, sizeof tmpmd5name, "%s.md5", tmpname );
		KFile * kf;
		KMD5SumFmt *km;
		if ( tmplen >= sizeof ( tmpmd5name ) ) /* can't be == or no NUL */
		{
		    rc = RC ( rcDB, rcIndex, rcPersisting, rcName, rcExcessive );
		}
		else
		{
		    tmplen = snprintf ( md5path, sizeof md5path, "%s.md5", path );
		    
		    if ( tmplen >= sizeof ( md5path ) ) /* can't be == or no NUL */
		    {
			rc = RC ( rcDB, rcIndex, rcPersisting, rcName, rcExcessive );
		    }
		    else
		    {
			rc = KDirectoryCreateFile ( dir, &kf, true, 0664, 
                                         kcmInit, "%s", tmpmd5name );
			if ( rc == 0 )
			{
			    rc = KMD5SumFmtMakeUpdate ( &km, kf );
			    if ( rc == 0 )
			    {
				KMD5File * k5f;
				kf = NULL;
				rc = KMD5FileMakeWrite ( &k5f, pb . f, km, path );
				if ( rc == 0 )
				{
				    pb . f = KMD5FileToKFile ( k5f );
				}
				/* release pass or fail */
				KMD5SumFmtRelease ( km );
			    }
			    else
				KFileRelease ( kf );
			}
			else
			    KFileRelease ( kf );
		    }
		}
		if ( rc != 0 )
		    KFileRelease ( pb . f );
	    }

            if ( rc == 0 )
            {
                pb . ptt_size = 0;
                pb . first = ~ 0;
                pb . last = 0;

                rc = KTrieIndexPersistTrie_v1 ( self, & pb );
                if ( rc == 0 )
                {
                    if ( proj )
                        rc = KTrieIndexPersistProj_v1 ( self, & pb );
                }
                
                KFileRelease ( pb . f );
                pb . f = NULL;
            }
        }
        
        free ( pb . buffer );
        pb . buffer = NULL;
        
        if ( rc == 0 )
        {
            rc = KDirectoryRename ( dir, false, tmpname, path );                        
            if ( rc == 0 )
            {
		if ( use_md5 )
		    rc = KDirectoryRename ( dir, false, tmpmd5name, md5path );                        
		if ( rc == 0 )
		{
		    /* done */
		    return 0;
		}
            }
        }
        
        /* whack temporary file */
        KDirectoryRemove ( dir, false, "%s", tmpname );
	if ( use_md5 )
	    KDirectoryRemove ( dir, false, "%s", tmpmd5name );
    }
    
    return rc;
}


/* whack whack */
void KTrieIndexWhack_v1 ( KTrieIndex_v1 *self )
{
    KPTrieIndexWhack_v1 ( & self -> pt );
    TrieWhack ( & self -> key2id, KTrieIdxNodeWhack_v1, NULL );
}

/* initialize an index from file - can be NULL */
rc_t KTrieIndexOpen_v1 ( KTrieIndex_v1 *self, const KMMap *mm, bool byteswap )
{
    rc_t rc;

    memset ( self, 0, sizeof * self );
    rc = TrieInit ( & self -> key2id, "0-9", 512, true );
    if ( rc != 0 )
        return rc;

    self -> first = ~ 0;

    if ( mm == NULL )
        return 0;

    rc = KPTrieIndexInit_v1 ( & self -> pt, mm, byteswap );
    if ( rc == 0 )
    {
        rc = KMMapAddRef ( mm );
        if ( rc == 0 )
        {
            self -> pt . mm = mm;
            return 0;
        }
    }

    KTrieIndexWhack_v1 ( self );
    return rc;
}

/* KTrieIndexPopulate_v1
 */
typedef struct KTrieIndexPopulateData_v1 KTrieIndexPopulateData_v1;
struct KTrieIndexPopulateData_v1
{
    KTrieIndex_v1 *idx;
    uint32_t id;
    rc_t rc;
};

static
bool CC KTrieIndexPopulate_v1 ( PTNode *n, void *data )
{
    const String *key;
    KTrieIndexPopulateData_v1 *pb = data;

    /* capture node id */
    uint32_t id;
    assert ( n -> data . size == sizeof id );
    memmove ( & id, n -> data . addr, sizeof id );

    /* reject already mapped */
    if ( id == pb -> id )
    {
        pb -> rc = RC ( rcDB, rcIndex, rcConstructing, rcNode, rcExists );
        return true;
    }

    pb -> rc = PTNodeMakeKey ( n, & key );
    if ( pb -> rc == 0 )
    {
        KTrieIdxNode_v1 *node;
        pb -> rc = TNodeMake ( ( TNode** ) & node,
            sizeof * node + key -> size );
        if ( pb -> rc == 0 )
        {
            KTrieIndex_v1 *self = pb -> idx;

            StringInit ( & node -> n . key, node -> key, key -> size, key -> len );
            node -> id = id;
            string_copy ( node -> key, key -> size + 1,
                key -> addr, key -> size );

            pb -> rc = TrieInsert ( & self -> key2id, & node -> n );
            if ( pb -> rc == 0 )
            {
                free ( ( void* ) key );
    
                /* if copying projection index */
                if ( self -> id2node != NULL )
                {
                    if ( self -> pt . id2node != NULL )
                        self -> id2node [ node -> id - self -> pt . first ] = node;
                    else
                    {
                        if ( node -> id < self -> first )
                            self -> first = node -> id;
                        if ( node -> id > self -> last )
                            self -> last = node -> id;
                    }
                }
                return 0;
            }

            TNodeWhack ( & node -> n );
        }

        StringWhack ( ( String* ) key );
    }

    return true;
}

/* KTrieIndexAttach_v1
 *  attach a keymap to an existing table
 *
 *  "pkm" [ IN ] - a persisted keymap
 */
static
rc_t KTrieIndexAttach_v1 ( KTrieIndex_v1 *self, bool proj, uint32_t id )
{
    uint32_t proj_len;
    KTrieIndexPopulateData_v1 pb;

#if LIMIT_INSERTS
    proj_len = self -> pt . last - self -> pt . first + 1;
#endif

    /* see if we can use existing projection index */
    if ( proj && self -> pt . id2node != NULL )
    {
        /* reject if already mapped */
        if ( id != 0 &&
             id >= self -> pt . first &&
             id <= self -> pt . last &&
             self -> pt . id2node [ id - self -> pt . first ] != 0 )
        {
            return RC ( rcDB, rcIndex, rcUpdating, rcId, rcExists );
        }

        /* allocate index array */
#if LIMIT_INSERTS
        if ( proj_len > INSERT_LIMIT )
            return RC ( rcDB, rcIndex, rcUpdating, rcRange, rcExcessive );
#else
        proj_len = self -> pt . last - self -> pt . first + 1;
#endif
        proj_len = ( proj_len + 4095 ) & - 4096;
        self -> id2node = calloc ( proj_len, sizeof self -> id2node [ 0 ] );
        if ( self -> id2node == NULL )
            return RC ( rcDB, rcIndex, rcUpdating, rcMemory, rcExhausted );

        /* record known dimensions */
        self -> first = self -> pt . first;
        self -> last = self -> pt . last;
        self -> len = proj_len;
    }

#if LIMIT_INSERTS
    else if ( proj_len > INSERT_LIMIT )
        return RC ( rcDB, rcIndex, rcUpdating, rcRange, rcExcessive );
#endif

    /* inflate persisted trie */
    pb . idx = self;
    pb . id = id;
    pb . rc = 0;
    PTrieDoUntil ( self -> pt . key2id, KTrieIndexPopulate_v1, & pb );

    /* if successful but needing to add projection index */
    if ( pb . rc == 0 && proj && self -> id2node == NULL )
    {
        proj_len = self -> last - self -> first + 1;
        proj_len = ( proj_len + 4095 ) & - 4096;
        self -> id2node = calloc ( proj_len, sizeof self -> id2node [ 0 ] );
        if ( self -> id2node == NULL )
            pb . rc = RC ( rcDB, rcIndex, rcUpdating, rcMemory, rcExhausted );
        else
        {
            self -> len = proj_len;
            TrieForEach ( & self -> key2id, KTrieIdxNodeCaptureID_v1, self );
        }
    }

    if ( pb . rc == 0 )
        KPTrieIndexWhack_v1 ( & self -> pt );
    else if ( self -> id2node != NULL )
    {
        TrieForEach ( & self -> key2id,
            KTrieIdxNodeUnlink_v1, & self -> key2id );
        free ( self -> id2node );
        self -> id2node = NULL;
        self -> first = ~0;
        self -> last = 0;
        self -> len = 0;
    }
    return pb . rc;
}

/* insert string into trie, mapping to 32 bit id */
static
rc_t KTrieIndexExpandId2Node_v1 ( KTrieIndex_v1 *self, uint32_t range )
{
    KTrieIdxNode_v1 **id2node;
    range = ( range + 4095 ) & - 4096;
    id2node = realloc ( self -> id2node, range * sizeof id2node [ 0 ] );
    if ( id2node == NULL )
        return RC ( rcDB, rcIndex, rcInserting, rcMemory, rcExhausted );

    self -> id2node = id2node;

#if ZERO_ID2NODE
    /* why zero this when it is known to be invalid? */
    memset ( id2node + self -> len, 0,
        ( range - self -> len ) * sizeof id2node [ 0 ] );
#endif

    self -> len = range;

    return 0;
}

rc_t KTrieIndexInsert_v1 ( KTrieIndex_v1 *self,
    bool proj, const char *str, uint32_t id )
{
    rc_t rc;

    String key;
    KTrieIdxNode_v1 *node;

#if DISABLE_PROJ
    proj = false;
#endif

    /* detect first modification */
    if ( self -> last < self -> first )
    {
        /* detect persisted data */
        if ( self -> pt . key2id != NULL )
        {
            rc = KTrieIndexAttach_v1 ( self, proj, id );
            if ( rc != 0 )
                return rc;
        }

        /* create empty projection array */
        else if ( proj )
        {
            self -> id2node = malloc ( 4096 * sizeof self -> id2node [ 0 ] );
            if ( self -> id2node == NULL )
                return RC ( rcDB, rcIndex, rcInserting, rcMemory, rcExhausted );
            self -> first = self -> last = id;
            self -> len = 4096;
        }
    }

    /* reject if already mapped */
    else if ( self -> id2node != NULL &&
              id >= self -> first &&
              id <= self -> last &&
              self -> id2node [ id - self -> first ] != NULL )
    {
        return RC ( rcDB, rcIndex, rcInserting, rcId, rcExists );
    }
#if LIMIT_INSERTS && INSERT_LIMIT > 0
    else if ( ( self -> last - self -> first ) >= ( INSERT_LIMIT - 1 ) )
    {
        return RC ( rcDB, rcIndex, rcUpdating, rcRange, rcExcessive );
    }
#endif

    StringInitCString ( & key, str );
    rc = TNodeMake ( ( TNode** ) & node, sizeof * node + key . size );
    if ( rc != 0 )
        rc = RC ( rcDB, rcIndex, rcInserting, rcMemory, rcExhausted );
    else
    {
        StringInit ( & node -> n . key, node -> key, key . size, key . len );
        node -> id = id;
        strcpy ( node -> key, str );

        rc = TrieInsertUnique ( & self -> key2id, & node -> n, NULL );
        if ( rc != 0 )
            TNodeWhack ( & node -> n );
        else if ( proj )
        {
            uint32_t range, offset;

            if ( id < self -> first )
            {
                range = self -> last - id + 1;
                if ( range > self -> len )
                {
                    rc = KTrieIndexExpandId2Node_v1 ( self, range );
                    if ( rc != 0 )
                    {
                        TrieUnlink ( & self -> key2id, & node -> n );
                        KTrieIdxNodeWhack_v1 ( & node -> n, NULL );
                        return rc;
                    }
                }

                offset = self -> first - id;
                memmove ( & self -> id2node [ offset ], self -> id2node,
                    ( self -> last - self -> first + 1 ) * sizeof self -> id2node [ 0 ] );
                memset ( & self -> id2node [ 1 ], 0,
                    ( offset - 1 ) * sizeof self -> id2node [ 0 ] );

                self -> first = id;
            }
            else if ( id > self -> last )
            {
                range = id - self -> first + 1;
                if ( range > self -> len )
                {
                    rc = KTrieIndexExpandId2Node_v1 ( self, range );
                    if ( rc != 0 )
                    {
                        TrieUnlink ( & self -> key2id, & node -> n );
                        KTrieIdxNodeWhack_v1 ( & node -> n, NULL );
                        return rc;
                    }
                }

                offset = id - 1 - self -> last;
                if ( offset > 0 )
                {
                    memset ( & self -> id2node [ self -> last - self -> first + 1 ],
                        0, offset * sizeof self -> id2node [ 0 ] );
                }

                self -> last = id;
            }

            assert ( self -> id2node != NULL );
            self -> id2node [ id - self -> first ] = node;
        }
    }

    return rc;
}

/* drop string from trie and all mappings */
rc_t KTrieIndexDelete_v1 ( KTrieIndex_v1 *self, bool proj, const char *str )
{
    rc_t rc;

    String key;
    KTrieIdxNode_v1 *node;

#if DISABLE_PROJ
    proj = 0;
#endif

    /* detect first modification */
    if ( self -> last < self -> first )
    {
        /* detect persisted data */
        if ( self -> pt . key2id != NULL )
        {
            rc = KTrieIndexAttach_v1 ( self, proj, 0 );
            if ( rc != 0 )
                return rc;
        }

        /* create empty projection array */
        else if ( proj )
        {
            self -> id2node = malloc ( 4096 * sizeof self -> id2node [ 0 ] );
            if ( self -> id2node == NULL )
                return RC ( rcDB, rcIndex, rcRemoving, rcMemory, rcExhausted );
            self -> first = self -> last = 0;
            self -> len = 4096;
        }
    }

    /* interface states that all objects are dropped.
       however, this implementation only allows unique
       mappings, so a simple find is sufficient */
    StringInitCString ( & key, str );
    node = ( KTrieIdxNode_v1* ) TrieFind ( & self -> key2id, & key );
    if ( node == NULL )
        return RC ( rcDB, rcIndex, rcRemoving, rcString, rcNotFound );

    /* drop from projection index */
    if ( self -> id2node != NULL &&
         node -> id >= self -> first &&
         node -> id <= self -> last )
    {
        assert ( self -> id2node [ node -> id - self -> first ] == node );
        if ( node -> id == self -> last )
        {
            if ( -- self -> last < self -> first )
            {
                free ( self -> id2node );
                self -> id2node = NULL;
                self -> len = 0;
            }
        }
        else if ( node -> id == self -> first )
        {
            memmove ( self -> id2node, self -> id2node + 1,
                ( self -> last - self -> first ) * sizeof self -> id2node [ 0 ] );
            ++ self -> first;
        }
        else
        {
            self -> id2node [ node -> id - self -> first ] = NULL;
        }
    }

    TrieUnlink ( & self -> key2id, & node -> n );
    KTrieIdxNodeWhack_v1 ( & node -> n, NULL );

    return 0;
}

/* map key to id ( was Key2Id ) */
rc_t KTrieIndexFind_v1 ( const KTrieIndex_v1 *self, const char *str, uint32_t *id,
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void * data )
{
    String key;

    if ( self -> last < self -> first )
    {
        if ( self -> pt . key2id != NULL )
        {
            PTNode n;
            uint32_t nid;
            StringInitCString ( & key, str );
            nid = PTrieFind ( self -> pt . key2id, & key, & n, custom_cmp ,data);
            if ( nid != 0 )
            {
                assert ( n . data . size == sizeof * id );
                memmove ( id, n . data . addr, sizeof * id );
                return 0;
            }
        }
    }
    else
    {
        const KTrieIdxNode_v1 *n;

        StringInitCString ( & key, str );
        n = ( const KTrieIdxNode_v1* ) TrieFind ( & self -> key2id, & key );
        if ( n != NULL )
        {
            * id = n -> id;
            return 0;
        }
    }

    return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

/* projection index id to key-string ( was Id2Key ) */
rc_t KTrieIndexProject_v1 ( const KTrieIndex_v1 *self,
    uint32_t id, char *key_buff, size_t buff_size, size_t *actsize )
{
    if ( self -> last < self -> first )
    {
        if ( self -> pt . id2node != NULL &&
             id >= self -> pt . first &&
             id <= self -> pt . last )
        {
            PTNode n;
            uint32_t node = self -> pt . id2node [ id - self -> pt . first ];
            rc_t rc = PTrieGetNode ( self -> pt . key2id,
                & n, self -> pt . byteswap ? bswap_32 ( node ) : node );
            if ( rc == 0 )
            {
                const String *key;
                rc = PTNodeMakeKey ( & n, & key );
                if ( rc == 0 )
                {
                    if (actsize)
                        *actsize = key -> size;
                    
                    if ( key -> size >= buff_size )
                        rc = RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                    else
                        string_copy ( key_buff, buff_size, key -> addr, key -> size );

                    StringWhack ( ( String* ) key );
                    return rc;
                }
            }
        }
    }
    else
    {
        if ( self -> id2node != NULL &&
             id >= self -> first &&
             id <= self -> last )
        {
            const KTrieIdxNode_v1 *n = self -> id2node [ id - self -> first ];
            if ( n != NULL )
            {
                if ( n -> n . key . size >= buff_size )
                    return RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                string_copy ( key_buff, buff_size,
                    n -> n . key . addr, n -> n . key . size );
                return 0;
            }
        }
    }

    return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}
