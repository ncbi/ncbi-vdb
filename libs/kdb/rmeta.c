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

#include "rmeta.h"
#include "rmetadatanode.h"

#include "database-cmn.h"
#include "rtable.h"
#include "rcolumn.h"
#include "rdbmgr.h"

#include <kfs/file.h>
#include <kfs/mmap.h>

#include <klib/debug.h>
#include <klib/rc.h>

#include <strtol.h>
#include <byteswap.h>

#define KMETADATAVERS 2

/*--------------------------------------------------------------------------
 * KRMetadata
 *  a versioned, hierarchical structure
 */

static rc_t CC KRMetadataWhack ( KMetadata *self );
static rc_t CC KRMetadataVersion ( const KMetadata *self, uint32_t *version );
static rc_t CC KRMetadataByteOrder ( const KMetadata *self, bool *reversed );
static rc_t CC KRMetadataRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KRMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KRMetadataOpenRevision ( const KMetadata *self, const KMetadata **metap, uint32_t revision );
static rc_t CC KRMetadataGetSequence ( const KMetadata *self, const char *seq, int64_t *val );
static rc_t CC KRMetadataVOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, va_list args );

static KMetadata_vt KRMetadata_vt =
{
    KRMetadataWhack,
    KMetadataBaseAddRef,
    KMetadataBaseRelease,
    KRMetadataVersion,
    KRMetadataByteOrder,
    KRMetadataRevision,
    KRMetadataMaxRevision,
    KRMetadataOpenRevision,
    KRMetadataGetSequence,
    KRMetadataVOpenNodeRead
};

#define CAST() assert( bself->vt == &KRMetadata_vt ); KRMetadata * self = (KRMetadata *)bself

/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */

/* Whack
 */
static
rc_t
KRMetadataWhack ( KMetadata *bself )
{
    CAST();

    rc_t rc = 0;

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc == 0 )
            self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc = KTableSever ( self -> tbl );
        if ( rc == 0 )
            self -> tbl = NULL;
    }
    else if ( self -> col != NULL )
    {
        rc = KColumnSever ( self -> col );
        if ( rc == 0 )
            self -> col = NULL;
    }

    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        KMDataNodeRelease ( & self -> root -> dad );
        return KMetadataBaseWhack( bself );
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KRMetadata", "whack", "kmeta" );
    return rc;
}

/* Make
 */

static
rc_t
KRMetadataPopulate ( KRMetadata *self, const KDirectory *dir, const char *path )
{
    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );
    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            size_t size;
            const void *addr = NULL;
            rc = KMMapSize ( mm, & size );
            if ( rc == 0 )
                rc = KMMapAddrRead ( mm, & addr );

            if ( rc == 0 )
            {
                union
                {
                    KDBHdr v1;
                    KDBHdr v2;
                } hdrs;

                const KDBHdr *hdr = ( const KDBHdr* ) addr;
                const void *pbstree_src = hdr + 1;

                rc = KDBHdrValidate ( hdr, size, 1, KMETADATAVERS );
                if ( GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
                {
                    hdrs . v1 . endian = bswap_32 ( hdr -> endian );
                    hdrs . v1 . version = bswap_32 ( hdr -> version );
                    rc = KDBHdrValidate ( & hdrs . v1, size, 1, KMETADATAVERS );
                    if ( rc == 0 )
                    {
                        self -> byteswap = true;
                        switch ( hdrs . v1 . version )
                        {
                        case 1:
                            hdr = & hdrs . v1;
                            break;
                        case 2:
                            hdr = & hdrs . v2;
                            break;
                        }
                    }
                }
                if ( rc == 0 )
                {
                    PBSTree *bst;
                    rc = PBSTreeMake ( & bst, pbstree_src, size - sizeof * hdr, self -> byteswap );
                    if ( rc != 0 )
                        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
                    else
                    {
                        KRMDataNodeInflateData pb;

                        pb . meta = self;
                        pb . par = self -> root;
                        pb . bst = & self -> root -> child;
                        pb . node_size_limit = NODE_SIZE_LIMIT;
                        pb . node_child_limit = NODE_CHILD_LIMIT;
                        pb . rc = 0;
                        pb . byteswap = self -> byteswap;

                        if ( hdr -> version == 1 )
                            PBSTreeDoUntil ( bst, 0, KRMDataNodeInflate_v1, & pb );
                        else
                            PBSTreeDoUntil ( bst, 0, KRMDataNodeInflate, & pb );
                        rc = pb . rc;

                        self -> vers = hdr -> version;

                        PBSTreeWhack ( bst );
                    }
                }
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}

rc_t
KRMetadataMakeRead ( KRMetadata **metap, const KDirectory *dir, const char *path, uint32_t rev )
{
    rc_t rc = 0;
    KRMetadata *meta = malloc ( sizeof * meta + strlen ( path ) );
    if ( meta == NULL )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    else
    {
        memset ( meta, 0, sizeof * meta );
        meta -> dad . vt = & KRMetadata_vt;
        if ( KRMDataNodeMakeRoot( & meta -> root, meta ) == 0 )
        {
            meta -> dir = dir;
            KRefcountInit ( & meta -> dad . refcount, 1, "KRMetadata", "make-read", path );
            meta -> rev = rev;
            meta -> byteswap = false;
            strcpy ( meta -> path, path );

            rc = KRMetadataPopulate ( meta, dir, path );
            if ( rc == 0 )
            {
                KDirectoryAddRef ( dir );
                * metap = meta;
                return 0;
            }

            free ( meta -> root );
        }

        free ( meta );
    }
    * metap = NULL;
    return rc;
}

/* Version
 *  returns the metadata format version
 */
static
rc_t CC
KRMetadataVersion ( const KMetadata *bself, uint32_t *version )
{
    CAST();

    if ( version == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * version = self -> vers;
    return 0;
}


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
static
rc_t CC
KRMetadataByteOrder ( const KMetadata *bself, bool *reversed )
{
    CAST();

    if ( reversed == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * reversed = self -> byteswap;
    return 0;
}


/* Revision
 *  returns current revision number
 *  where 0 ( zero ) means tip
 */
static
rc_t CC
KRMetadataRevision ( const KMetadata *bself, uint32_t *revision )
{
    CAST();

    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = self -> rev;
    return 0;
}


/* MaxRevision
 *  returns the maximum revision available
 */
static
rc_t CC
KRMetadataMaxRevision ( const KMetadata *bself, uint32_t *revision )
{
    CAST();

    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = 0;

    KNamelist *listing;
    rc_t rc = KDirectoryList ( self -> dir,
        & listing, NULL, NULL, "md" );
    if ( rc == 0 )
    {
        uint32_t rev_max, count;
        rc = KNamelistCount ( listing, & count );
        if ( rc == 0 )
        {
            uint32_t idx;
            for ( rev_max = idx = 0; idx < count; ++ idx )
            {
                const char *name;

                rc = KNamelistGet ( listing, idx, & name );
                if ( rc != 0 )
                    break;

                if ( name [ 0 ] == 'r' )
                {
                    char *end;
                    uint32_t rev = strtou32 ( name + 1, & end, 10 );
                    if ( end [ 0 ] == 0 && rev > rev_max )
                        rev_max = rev;
                }
            }

            * revision = rev_max;
        }

        KNamelistRelease ( listing );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}


/* OpenRevision
 *  opens a read-only indexed revision of metadata
 */
static
rc_t CC
KRMetadataOpenRevision ( const KMetadata *bself, const KMetadata **metap, uint32_t revision )
{
    CAST();

    rc_t rc;
    KRMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBRManagerOpenMetadataReadInt ( self -> mgr,
        & meta, self -> dir, revision, false );
    if ( rc == 0 )
    {
        if ( self -> db != NULL )
            meta -> db = KDatabaseAttach ( self -> db );
        else if ( self -> tbl != NULL )
            meta -> tbl = KTableAttach ( self -> tbl );
        else if ( self -> col != NULL )
            meta -> col = KColumnAttach ( self -> col );

        * metap = & meta -> dad;
    }

    return rc;
}


/* GetSequence
 *  access a named sequence
 *
 *  "seq" [ IN ] - NUL terminated sequence name
 *
 *  "val" [ OUT ] - return parameter for sequence value
 */
static
rc_t CC
KRMetadataGetSequence ( const KMetadata *bself, const char *seq, int64_t *val )
{
    CAST();

    rc_t rc;
    const KMDataNode *found;

    if ( val == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * val = 0;

    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcString, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcAccessing, rcString, rcInvalid );

    rc = KMDataNodeOpenNodeRead
        ( & self -> root -> dad, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KMDataNodeRead ( found, 0, val, sizeof * val, & num_read, & remaining );
        assert ( rc != 0 || ( num_read == sizeof * val && remaining == 0 ) );
        KMDataNodeRelease ( found );
    }

    return rc;
}

static
rc_t CC
KRMetadataVOpenNodeRead ( const KMetadata *bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc = 0;

    if ( node == NULL )
        rc = RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    else
        rc = KMDataNodeVOpenNodeRead ( & self -> root -> dad, node, path, args );

    return rc;
}
