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
#include <kdb/kdb-priv.h>

#include <kdb/manager.h>

#include "dbmgr.h"
#include "../vfs/path-priv.h" /* VPathSetDirectory */

#include <kfs/directory.h>
#include <kfs/sra.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path-priv.h> /* VPathGetDirectory */

#include <kdb/column.h>
#include <kdb/index.h>
#include "rcolumn.h"
#include "index-cmn.h"

#include <klib/data-buffer.h>
#include <kfs/file.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/rc.h>

#include <ctype.h>
#include <stdio.h>

/* KDBPathType
 *  checks type of path
 */
enum ScanBits
{
    scan_db     = ( 1 <<  0 ),
    scan_tbl    = ( 1 <<  1 ),
    scan_idx    = ( 1 <<  2 ),
    scan_col    = ( 1 <<  3 ),
    scan_idxN   = ( 1 <<  4 ),
    scan_data   = ( 1 <<  5 ),
    scan_dataN  = ( 1 <<  6 ),
    scan_md     = ( 1 <<  7 ),
    scan_cur    = ( 1 <<  8 ),
    scan_rNNN   = ( 1 <<  9 ),
    scan_lock   = ( 1 << 10 ),
    scan_odir   = ( 1 << 11 ),
    scan_ofile  = ( 1 << 12 ),
    scan_meta   = ( 1 << 13 ),
    scan_skey   = ( 1 << 14 ),
    scan_sealed = ( 1 << 15 ),
    scan_zombie = ( 1 << 16 ),
    scan_MD5    = ( 1 << 17 ),
    scan_alias  = ( 1 << 18 )
};

static
rc_t CC scan_dbdir ( const KDirectory *dir, uint32_t type, const char *name, void *data )
{
    uint32_t *bits = data;

    if ((type & kptAlias) != 0) {
        *bits |= scan_alias;
        type ^= kptAlias;
    }

    if ( type == kptDir )
    {
        switch ( name [ 0 ] )
        {
        case 'c':
            if ( strcmp ( name, "col" ) == 0 )
            { * bits |= scan_col; return 0; }
            break;
        case 'm':
            if ( strcmp ( name, "md" ) == 0 )
            { * bits |= scan_md; return 0; }
            break;
        case 't':
            if ( strcmp ( name, "tbl" ) == 0 )
            { * bits |= scan_tbl; return 0; }
            break;
        case 'i':
            if ( strcmp ( name, "idx" ) == 0 )
            { * bits |= scan_idx; return 0; }
            break;
        case 'd':
            if ( strcmp ( name, "db" ) == 0 )
            { * bits |= scan_db; return 0; }
            break;
        }

        * bits |= scan_odir;
    }
    else if ( type == kptFile )
    {
        switch ( name [ 0 ] )
        {
        case 'l':
            if ( strcmp ( name, "lock" ) == 0 )
            { * bits |= scan_lock; return 0; }
            break;
        case 'i':
            if ( memcmp ( name, "idx", 3 ) == 0 )
            {
                if ( isdigit ( name [ 3 ] ) )
                { * bits |= scan_idxN; return 0; }
            }
            break;
        case 'd':
            if ( memcmp ( name, "data", 4 ) == 0 )
            {
                if ( name [ 4 ] == 0 )
                { * bits |= scan_data; return 0; }
                if ( isdigit ( name [ 4 ] ) )
                { * bits |= scan_dataN; return 0; }
            }
        case 'c':
            if ( strcmp ( name, "cur" ) == 0 )
            { * bits |= scan_cur; return 0; }
            break;
        case 'r':
            if ( isdigit ( name [ 1 ] ) && isdigit ( name [ 2 ] ) &&
                 isdigit ( name [ 3 ] ) && name [ 4 ] == 0 )
            { * bits |= scan_rNNN; return 0; }
            break;
        case 'm':
            if ( strcmp ( name, "meta" ) == 0 )
            { * bits |= scan_meta; return 0; }
            if ( strcmp ( name, "md5"  ) == 0 )
            { * bits |= scan_MD5; return 0; }
            break;
        case 's':
            if ( strcmp ( name, "skey" ) == 0 )
            { * bits |= scan_skey; return 0; }
            if ( strcmp ( name, "sealed" ) == 0 )
            { * bits |= scan_sealed; return 0; }
            break;
        }

        * bits |= scan_ofile;
    }
    else if (type == kptZombieFile )
    {
        * bits |= scan_zombie;
    }

    return 0;
}

int KDBPathTypeDir (const KDirectory * dir, int type, bool * pHasZombies, const char * path)
{
    const char * leaf, * parent;
    uint32_t bits;
    rc_t rc;

    bits = 0;

    assert ((type == kptDir) || (type == (kptDir|kptAlias)));

    rc = KDirectoryVisit ( dir, false, scan_dbdir, & bits, "%s", path );
    if ( rc == 0 ) do
    {
        if ( ( bits & scan_zombie ) != 0 ) {
            bits &= ~scan_zombie;
            if (pHasZombies)
                *pHasZombies = true;
        }
        /* look for a column */
        if ( ( bits & scan_idxN ) != 0 &&
             ( bits & ( scan_data | scan_dataN ) ) != 0 )
        {
            if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                type += kptColumn - kptDir;
            break;
        }

        /* look for a table */
        if ( ( bits & scan_col ) != 0 )
        {
            /* can't have sub-tables or a db */
            if ( ( bits & ( scan_db | scan_tbl ) ) == 0 )
            {
                /* look for an old-structure table */
                if ( ( bits & ( scan_meta | scan_md ) ) == scan_meta ||
                     ( bits & ( scan_skey | scan_idx ) ) == scan_skey )
                    type += kptPrereleaseTbl - kptDir;
                else
                    type += kptTable - kptDir;
            }
            break;
        }

        /* look for metadata */
        if ( ( bits & ( scan_cur | scan_rNNN ) ) != 0 )
        {
            if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                type += kptMetadata - kptDir;
            break;
        }

        /* look for a database */
        if ( ( bits & scan_tbl ) != 0 )
        {
            if ( ( bits & scan_col ) == 0 )
                type += kptDatabase - kptDir;
            break;
        }

        /* look for a structured column */
        if ( ( bits & scan_odir ) != 0 )
        {
            leaf = strrchr ( path, '/' );
            if ( leaf != NULL )
            {
                parent = string_rchr ( path, leaf - path, '/' );
                if ( parent ++ == NULL )
                    parent = path;
                if ( memcmp ( parent, "col/", 4 ) != 0 )
                    break;

                bits = 0;
                if ( KDirectoryVisit ( dir, 1, scan_dbdir, & bits, "%s", path ) == 0 )
                {
                    if ( ( bits & scan_idxN ) != 0 &&
                         ( bits & ( scan_data | scan_dataN ) ) != 0 )
                    {
                        if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                            type += kptColumn - kptDir;
                        break;
                    }
                }
            }
        }
    } while (0);

    return type;
}

int KDBPathType ( const KDirectory *dir, bool *pHasZombies, const char *path )
{
    const char *leaf, *parent;


    rc_t rc;
    int type = KDirectoryPathType ( dir, "%s", path );

    if (pHasZombies)
        *pHasZombies = false;

    switch ( type )
    {
    case kptDir:
    case kptDir | kptAlias:
        type = KDBPathTypeDir (dir, type, pHasZombies, path);
        break;

    case kptFile:
    case kptFile | kptAlias:
    {
        /* if we hit a file first try it as an archive */
        const KDirectory * ldir;

        rc = KDirectoryOpenSraArchiveRead_silent ( dir, &ldir, false, "%s", path );
#if SUPPORT_KDB_TAR
        if ( rc != 0 )
            rc = KDirectoryOpenTarArchiveRead_silent ( dir, &ldir, false, "%s", path );
#endif
        /* it was an archive so recur */
        if ( rc == 0 )
        {
            /* recheck this newly opened directory for KDB/KFS type */
            int type2;

            type2 = KDBPathType ( ldir, NULL, "." );
            if ((type2 != kptDir) || (type != (kptDir|kptAlias)))
                type = type2;

            KDirectoryRelease (ldir);
        }
        /* it was not an archive so see if it it's an idx file */
        else
        {
            leaf = strrchr ( path, '/' );
            if ( leaf != NULL )
            {
                parent = string_rchr ( path, leaf - path, '/' );
                if ( parent ++ == NULL )
                    parent = path;
                if ( memcmp ( parent, "idx/", 4 ) == 0 )
                    type += kptIndex - kptFile;
            }
        }
        break;
    }
    }
    return type;
}

void KDBContentsWhack(KDBContents const *const self)
{
    if (self) {
        KDBContentsWhack(self->firstChild);
        KDBContentsWhack(self->nextSibling);
        free((void *)self);
    }
}

static KDBContents *KDBContentsAlloc(size_t const namelen)
{
    size_t const n = namelen + 1;
    struct Layout {
        KDBContents result;
        char name[1];
    } *result = malloc(offsetof(struct Layout, name[n]));
    if (result) {
        result->result.name = &result->name[0];
        return &result->result;
    }
    return NULL;
}

static KDBContents *KDBContentsInit(KDBContents *const result
                                    , char const *const name
                                    , size_t const namelen
                                    , KPathType const fstype
                                    , KDBContents const *const parent
                                    , KDBContents const *const previous)
{
    if (result) {
        char *rname = (char *)result->name;
        char const *const endp = name + namelen;
        char const *cp = name;

        while (cp != endp)
            *rname++ = *cp++;
        *rname = '\0';

        result->parent = parent;
        result->firstChild = result->nextSibling = NULL;
        result->prevSibling = previous;
        result->dbtype = (result->fstype = fstype) ^ (fstype & kptAlias);
        result->attributes = 0;
        result->levelOfDetail = parent ? parent->levelOfDetail : lod_Full;
    }
    return result;
}

static KDBContents *KDBContentsMake(char const *const name
                                    , KPathType const fstype
                                    , KDBContents const *const parent
                                    , KDBContents const *const previous)
{
    size_t const n = strlen(name);
    return KDBContentsInit(KDBContentsAlloc(n), name, n, fstype, parent, previous);
}

static KDBContents *KDBContentsVMake(KPathType const fstype
                                    , KDBContents const *const parent
                                    , KDBContents const *const previous
                                    , char const *const name
                                    , va_list args)
{
    rc_t rc = 0;
    KDBContents *result = NULL;
    KDataBuffer buffer;

    KDataBufferMakeBytes(&buffer, 0);
    rc = KDataBufferVPrintf(&buffer, name, args);
    if (rc == 0) {
        result = KDBContentsInit(KDBContentsAlloc(buffer.elem_count), buffer.base, buffer.elem_count, fstype, parent, previous);
        KDataBufferWhack(&buffer);
    }
    return result;
}

struct KDBGetPathContents_Gather_context {
    KDBContents const *parent;
    KDBContents const *previous;
    KDBContents **target;
};

static
struct KDBGetPathContents_Gather_context makeContext(KDBContents *const contents)
{
    struct KDBGetPathContents_Gather_context result = {
        contents,
        NULL,
        (KDBContents **)&contents->firstChild
    };
    assert(contents->firstChild == NULL);
    return result;
}

static rc_t CC KDBGetPathContents_Gather_cb(const KDirectory *const dir
                                            , uint32_t const type
                                            , char const *const name
                                            , void *const vp)
{
    struct KDBGetPathContents_Gather_context *const ctx = vp;
    KDBContents *const content = KDBContentsMake(name, type, ctx->parent, ctx->previous);
    if (content == NULL)
        return RC(rcDB, rcDirectory, rcVisiting, rcMemory, rcExhausted);
    ctx->previous = *ctx->target = content;
    ctx->target = (KDBContents **)&content->nextSibling;

    return 0;
}

static rc_t KDBGetPathContents_GatherChildren(KDBContents *result, const KDirectory *const dir, char const *sub)
{
    struct KDBGetPathContents_Gather_context context = makeContext(result);
    return KDirectoryVisit(dir, false, KDBGetPathContents_Gather_cb, &context, sub);
}

static KDBContents *KDBContents_UnlinkDeadNode(KDBContents *node)
{
    while (node && node->dbtype == 0) {
        KDBContents *prev = (KDBContents *)node->prevSibling;
        KDBContents *next = (KDBContents *)node->nextSibling;
        if (next)
            next->prevSibling = node->prevSibling;
        if (prev)
            prev->nextSibling = node->nextSibling;
        node->nextSibling = NULL;
        KDBContentsWhack(node);
        node = next;
    }
    return node;
}

// NB: Ownership is transferred to `result`
static void KDBContents_appendChildren(KDBContents *result, KDBContents *node)
{
    node = KDBContents_UnlinkDeadNode(node);
    if (result->firstChild) {
        KDBContents *dst = (KDBContents *)result->firstChild;
        while (dst->nextSibling)
            dst = (KDBContents *)dst->nextSibling;
        dst->nextSibling = node;
        node->prevSibling = dst;
    }
    else
        result->firstChild = node;

    while (node) {
        node->parent = result;
        node = KDBContents_UnlinkDeadNode((KDBContents *)node->nextSibling);
    }
}

static uint32_t KDBGetPathContents_ScanBitsAndSubtype(KDBContents *node, const KDirectory *const dir, rc_t *prc)
{
    rc_t dummy = 0;
    uint32_t bits = 0;

    if (prc == NULL)
        prc = &dummy;

    *prc = KDirectoryVisit(dir, false, scan_dbdir, &bits, ".");
    if (*prc == 0) {
        if ((bits & scan_lock) != 0)
            node->attributes |= cca_HasLock;
        if ((bits & scan_sealed) != 0)
            node->attributes |= cca_HasSealed;
        if ((bits & scan_MD5) != 0)
            node->attributes |= cca_HasMD5_File;
        if ((bits & scan_md) != 0) {
            rc_t rc = KDirectoryVisit(dir, false, scan_dbdir, &bits, "md");
            if (rc == 0 && (bits & scan_cur) != 0)
                node->attributes |= cca_HasMetadata;
        }
    }
    return bits;
}

static bool scanBitsIsColumn(uint32_t const bits)
{
    bool const has_idx = (bits & scan_idxN) != 0;
    bool const has_data = (bits & (scan_data | scan_dataN)) != 0;
    bool const has_no_db = (bits & scan_db) == 0;
    bool const has_no_tbl = (bits & scan_tbl) == 0;
    bool const has_no_idx = (bits & scan_idx) == 0;
    bool const has_no_col = (bits & scan_col) == 0;

    return (has_idx && has_data && has_no_db && has_no_tbl && has_no_idx && has_no_col);
}

static void KDBGetPathContents_Column(KDBContents *node, const struct KDirectory *const dir)
{
    uint32_t const bits = KDBGetPathContents_ScanBitsAndSubtype(node, dir, NULL);

    if (scanBitsIsColumn(bits)) {
        if (node->levelOfDetail == lod_Full) {
            struct KRColumn const *colp = NULL;
            rc_t rc = KRColumnMakeRead((struct KRColumn **)&colp, dir, node->name);
            if (rc == 0) {
                bool reversed = false;

                KColumnByteOrder((KColumn const *)colp, &reversed);
                if (reversed)
                    node->attributes |= cca_ReversedByteOrder;

                switch (colp->checksum) {
                case kcsCRC32:
                    node->attributes |= cca_HasChecksum_CRC;
                    break;
                case kcsMD5:
                    node->attributes |= cca_HasChecksum_MD5;
                    break;
                case kcsNone:
                    break;
                default:
                    node->attributes |= cca_HasChecksum_CRC;
                    node->attributes |= cca_HasChecksum_MD5;
                    break;
                }
                KDirectoryAddRef(dir); ///< KColumnRelease is unbalanced!!!!
                KColumnRelease((KColumn const *)colp);
            }
            else {
                node->attributes |= cca_HasErrors;
            }
        }
        node->dbtype = kptColumn;
    }
}

static void KDBGetPathContents_Index(KDBContents *result, KDBContents *node, const struct KDirectory *const dir, char const *fmt)
{
    if (result->levelOfDetail == lod_Full) {
        struct KFile const *fh = NULL;
        rc_t rc = KDirectoryOpenFileRead(dir, &fh, fmt, node->name);
        if (rc == 0) {
            char buffer[sizeof(KIndexFileHeader_v3_v4)];
            size_t nread = 0;
            bool reversed = false;
            uint32_t type = 0;

            rc = KFileRead(fh, 0, buffer, sizeof(buffer), &nread);
            KFileRelease(fh);

            rc = KIndexValidateHeader(&reversed, &type, buffer, nread);
            if (rc)
                goto HAS_ERRORS;

            if (type == kitText)
                node->attributes |= cia_IsTextIndex;
            else
                node->attributes |= cia_IsIdIndex;

            if (reversed)
                node->attributes |= cia_ReversedByteOrder;
        }
        else {
HAS_ERRORS:
            result->attributes |= cca_HasErrors;
        }
    }
    node->dbtype = kptIndex;
    result->attributes |= cta_HasIndices;
}

/// @brief Traverse a table hierarchy and gather the columns found.
static void KDBGetPathContents_GatherColumns(KDBContents *result, const struct KDirectory *const dir)
{
    rc_t rc = 0;
    KDBContents *added = NULL, *save = (KDBContents *)result->firstChild;

    result->firstChild = NULL;
    rc = KDBGetPathContents_GatherChildren(result, dir, "col");
    added = (KDBContents *)result->firstChild;
    result->firstChild = save;
    if (rc == 0) {
        KDBContents *node = added;
        while (node) {
            if (node->fstype == kptDir) {
                struct KDirectory const *node_dir = NULL;
                rc = KDirectoryOpenDirRead(dir, &node_dir, false, "col/%s", node->name);
                assert(rc == 0);

                KDBGetPathContents_Column(node, node_dir);
                KDirectoryRelease(node_dir);

                if ((node->attributes & cca_HasErrors) != 0)
                    result->attributes |= cca_HasErrors;
                if (node->dbtype == kptColumn)
                    result->attributes |= cta_HasColumns;
            }
            node = (KDBContents *)node->nextSibling;
        }
        KDBContents_appendChildren(result, added);
    }
}

/// @brief Traverse a table hierarchy and gather the indices found.
static void KDBGetPathContents_GatherIndices(KDBContents *result, const struct KDirectory *const dir)
{
    rc_t rc = 0;
    KDBContents *added = NULL, *save = (KDBContents *)result->firstChild;

    result->firstChild = NULL;
    rc = KDBGetPathContents_GatherChildren(result, dir, "idx");
    added = (KDBContents *)result->firstChild;
    result->firstChild = save;
    if (rc == 0) {
        KDBContents *node;

        // find checksum files
        for (node = added; node; node = (KDBContents *)node->nextSibling) {
            if (node->fstype == kptFile) {
                char const *dot = NULL;
                char const *cp;
                for (cp = node->name; *cp; ++cp) {
                    if (*cp == '.')
                        dot = cp;
                }
                if (dot && strcmp(dot, ".md5") == 0) {
                    KDBContents *idx;
                    for (idx = added; idx; idx = (KDBContents *)node->nextSibling) {
                        if (idx->fstype == kptFile && strncmp(idx->name, node->name, dot - node->name) == 0) {
                            idx->attributes |= cia_HasChecksum_MD5;
                            node->dbtype = 0; // mark node as dead
                            break;
                        }
                    }
                }
            }
        }
        for (node = added; node; node = (KDBContents *)node->nextSibling) {
            if (node->fstype == kptFile && node->dbtype != 0)
                KDBGetPathContents_Index(result, node, dir, "idx/%s");
        }
        KDBContents_appendChildren(result, added);
    }
}

/// @brief Traverse a table hierarchy and gather the columns found.
static void KDBGetPathContents_Table(KDBContents *result, const struct KDirectory *const dir)
{
    KDBGetPathContents_GatherColumns(result, dir);
    KDBGetPathContents_GatherIndices(result, dir);
}

/// @brief Descend a database hierarchy and gather the tables found.
static void KDBGetPathContents_Tables(KDBContents *result, const struct KDirectory *const dir)
{
    rc_t rc = 0;
    KDBContents *added = NULL, *save = (KDBContents *)result->firstChild;

    result->firstChild = NULL;
    rc = KDBGetPathContents_GatherChildren(result, dir, "tbl");

    added = (KDBContents *)result->firstChild;
    result->firstChild = save;

    if (rc == 0) {
        KDBContents *node = added;
        while (node) {
            if (node->fstype == kptDir) {
                struct KDirectory const *node_dir = NULL;
                rc = KDirectoryOpenDirRead(dir, &node_dir, false, "tbl/%s", node->name);
                assert(rc == 0);

                KDBGetPathContents_ScanBitsAndSubtype(node, node_dir, NULL);
                KDBGetPathContents_Table(node, node_dir);
                KDirectoryRelease(node_dir);

                if ((node->attributes & cca_HasErrors) != 0)
                    result->attributes |= cca_HasErrors;

                node->dbtype = kptTable;
                result->attributes |= cda_HasTables;
            }
            node = (KDBContents *)node->nextSibling;
        }
        KDBContents_appendChildren(result, added);
    }
}

/// @brief Descend a database hierarchy and gather the databases found.
static void KDBGetPathContents_Databases(KDBContents *result, const struct KDirectory *const dir)
{
    rc_t rc = 0;
    KDBContents *added = NULL, *save = (KDBContents *)result->firstChild;

    result->firstChild = NULL;
    rc = KDBGetPathContents_GatherChildren(result, dir, "db");

    added = (KDBContents *)result->firstChild;
    result->firstChild = save;

    if (rc == 0) {
        KDBContents *node = added;
        while (node) {
            if (node->fstype == kptDir) {
                struct KDirectory const *node_dir = NULL;
                rc = KDirectoryOpenDirRead(dir, &node_dir, false, "db/%s", node->name);
                assert(rc == 0);

                KDBGetPathContents_ScanBitsAndSubtype(node, node_dir, NULL);
                KDBGetPathContents_Tables(node, node_dir);
                KDBGetPathContents_Databases(node, node_dir);
                KDirectoryRelease(node_dir);

                if ((node->attributes & cca_HasErrors) != 0)
                    result->attributes |= cca_HasErrors;

                node->dbtype = kptDatabase;
                result->attributes |= cda_HasDatabases;
            }
            node = (KDBContents *)node->nextSibling;
        }
        KDBContents_appendChildren(result, added);
    }
}

static rc_t KDBVGetPathContents_1(KDBContents const **presult, int levelOfDetail, const struct KDirectory *const dir, KPathType fstype, char const *const path, va_list args)
{
    KDBContents *result = NULL;
    KPathType type = KDirectoryPathType(dir, ".");

    *presult = (result = KDBContentsVMake(fstype, NULL, NULL, path, args));
    if (result == NULL)
        return RC(rcDB, rcDirectory, rcVisiting, rcMemory, rcExhausted);

    result->levelOfDetail = levelOfDetail;
    if ((type | kptAlias) == (kptDir | kptAlias)) {
        rc_t rc = 0;
        uint32_t const bits = KDBGetPathContents_ScanBitsAndSubtype(result, dir, &rc);
        if (rc) return rc;

        if ((bits & scan_col) != 0) {
            // tables only contain columns
            if ((bits & (scan_db | scan_tbl)) == 0) {
                KDBGetPathContents_Table(result, dir);
                result->dbtype = kptTable;
            }
        }
        else {
            // Maybe it is a database.
            // Descend `tbl` trees first
            if ((bits & scan_tbl) != 0) {
                KDBGetPathContents_Tables(result, dir);
                result->dbtype = kptDatabase;
            }

            // unlikely to happen in RL
            if ((bits & scan_db) != 0) {
                KDBGetPathContents_Databases(result, dir);
                result->dbtype = kptDatabase;
            }
        }
    }
    return 0;
}

rc_t KDBVGetPathContents(KDBContents const **presult, int levelOfDetail, const struct KDirectory *const dir, KPathType type, char const *const path, va_list args)
{
    if (presult == NULL || dir == NULL || path == NULL)
        return RC(rcDB, rcDirectory, rcVisiting, rcParam, rcNull);

    return KDBVGetPathContents_1(presult, levelOfDetail, dir, type, path, args);
}

rc_t KDBGetPathContents(KDBContents const **presult, int levelOfDetail, const struct KDirectory *dir, KPathType type, char const *path, ...)
{
    rc_t rc = 0;
    va_list ap;
    va_start(ap, path);
    rc = KDBVGetPathContents(presult, levelOfDetail, dir, type, path, ap);
    va_end(ap);
    return rc;
}

/* GetObjModDate
 *  extract mod date from a path
 */
rc_t KDBGetObjModDate ( const KDirectory *dir, KTime_t *mtime )
{
    /* HACK ALERT - there needs to be a proper way to record modification times */

    /* this only tells the last time the table was locked,
       which may be close to the last time it was modified */
    rc_t rc = KDirectoryDate ( dir, mtime, "lock" );
    if ( rc == 0 )
        return 0;

    if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = KDirectoryDate ( dir, mtime, "sealed" );
        if ( rc == 0 )
            return 0;
    }

    /* get directory timestamp */
    rc = KDirectoryDate ( dir, mtime, "." );
    if ( rc == 0 )
        return 0;

    * mtime = 0;
    return rc;
}


/* GetPathModDate
 *  extract mod date from a path
 */
rc_t KDBVGetPathModDate ( const KDirectory *dir,
    KTime_t *mtime, const char *path, va_list args )
{
    rc_t rc;
    uint32_t ptype;
    const KDirectory *obj_dir;

    va_list cpy;
    va_copy ( cpy, args );
    ptype = KDirectoryVPathType ( dir, path, cpy );
    va_end ( cpy );

    switch ( ptype )
    {
    case kptDir:
    case kptDir | kptAlias:
        break;

    default:
        return KDirectoryVDate ( dir, mtime, path, args );
    }

    * mtime = 0;
    rc = KDirectoryVOpenDirRead ( dir, & obj_dir, true, path, args );
    if ( rc == 0 )
    {
        rc = KDBGetObjModDate ( obj_dir, mtime );
        KDirectoryRelease ( obj_dir );
    }

    return rc;
}


/* KDBVMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBVMakeSubPath ( struct KDirectory const *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, va_list args )
{
    rc_t rc;

    if ( ns_size > 0 )
    {
        subpath += ns_size + 1;
        subpath_max -= ns_size + 1;
    }

#if CRUFTY_USE_OF_RESOLVE_PATH
    /* because this call only builds a path instead of resolving anything
     * is is okay that we are using the wrong directory */
    rc = KDirectoryVResolvePath ( dir, false,
        subpath, subpath_max, path, args );
#else
    {
        int sz = vsnprintf ( subpath, subpath_max, path, args );
        if ( sz < 0 || ( size_t ) sz >= subpath_max )
            rc = RC ( rcDB, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        else if ( sz == 0 )
            rc = RC ( rcDB, rcDirectory, rcResolving, rcPath, rcEmpty );
        else
        {
            rc = 0;
        }
    }
#endif
    switch ( GetRCState ( rc ) )
    {
    case 0:
        assert ( subpath [ 0 ] != 0 );
        if ( subpath [ 0 ] == '.' || subpath [ 1 ] == '/' )
            return RC ( rcDB, rcDirectory, rcResolving, rcPath, rcInvalid );
        break;
    case rcInsufficient:
        return RC ( rcDB, rcDirectory, rcResolving, rcPath, rcExcessive );
    default:
        return rc;
    }

    if ( ns_size != 0 )
    {
        subpath -= ns_size + 1;
        memmove ( subpath, ns, ns_size );
        subpath [ ns_size ] = '/';
    }
    return rc;
}

/* KDBMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBMakeSubPath ( struct KDirectory const *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, ... )
{
    rc_t rc = 0;
    va_list args;
    va_start(args, path);
    rc = KDBVMakeSubPath(dir, subpath, subpath_max, ns, ns_size, path, args);
    va_end(args);
    return rc;
}

static rc_t KDBOpenPathTypeReadInt ( const KDBManager * mgr, const KDirectory * dir, const char * path,
                                     const KDirectory ** pdir, int * type,
                                     int pathtype, uint32_t rcobj, bool try_srapath,
                                     const VPath * aVpath )
{
    VFSManager * vmgr = mgr->vfsmgr;
    const KDirectory * ldir = NULL;
    rc_t rc = 0;

    /* object relative opens can be done using KFS - we hacked in VFS after all */
    if (! try_srapath)
    {
        rc = KDirectoryOpenDirUpdate ((KDirectory*)dir, (KDirectory**)pdir, false, "%s", path);
        if ((rc) && (GetRCState(rc) != rcNotFound))
            rc = KDirectoryOpenDirRead (dir, pdir, false, "%s", path);
    }
    else
    {
        VPath * vpath = ( VPath * ) aVpath;

        rc = VPathGetDirectory(vpath, &ldir);
        if (rc != 0 || ldir == NULL)
          /*
           * We've got to decide if the path coming in is a full or relative
           * path and if relative make it relative to dir or possibly its
           * a srapath accession
           *
           */
          rc = VFSManagerMakeDirectoryRelativeVPath (vmgr,
            &vpath, dir, path, vpath );
        if ( rc == 0 )
        {
            if (ldir == NULL)
            {
                rc = VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( vmgr,
                    dir, &ldir, vpath );
                if (rc == 0)
                    VPathSetDirectory((VPath*)aVpath, ldir);
            }

            if ( rc == 0 )
            {
                *type = (~kptAlias) & KDBPathType ( ldir, NULL, "." );

                /* just a directory, not a kdb type */
                if ( *type == kptDir )
                    rc = RC (rcDB, rcMgr, rcOpening, rcPath, rcIncorrect);

                else if ( *type != pathtype )
                {
                    KDirectoryRelease( ldir );
                    rc = RC ( rcDB, rcMgr, rcOpening, rcobj, rcIncorrect );
                }
                else
                {
                    if ( pdir != NULL )
                        *pdir = ldir;
                    else
                        KDirectoryRelease( ldir );
                }
            }
            if ( aVpath != vpath )
                VPathRelease ( vpath );
        }
    }

    return rc;
}

LIB_EXPORT rc_t KDBManagerOpenPathTypeRead ( const KDBManager * mgr, const KDirectory * dir, const char * path,
    const KDirectory ** pdir, int pathtype, int * ppathtype, bool try_srapath,
    const VPath * vpath )
{
    const KDirectory *ldir;
    rc_t rc = 0;
    uint32_t rcobj;
    int type = kptNotFound; /* bogus? */

/*     KOutMsg ("%s: %s\n", __func__, path); */

    if ( pdir != NULL )
        *pdir = NULL;
    if ( ppathtype != NULL )
        *ppathtype = type;

    switch (pathtype & ~ kptAlias) /* tune the error message based on path type */
    {
        /* we'll hit this if we don't track defines in kdb/manager.h */
    default:
        rc = RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
        return rc;

    case kptTable:
    case kptPrereleaseTbl:
        rcobj = rcTable;
        break;

    case kptColumn:
        rcobj = rcColumn;
        break;

    case kptDatabase:
    case kptDatabase | kptAlias:
        rcobj = rcDatabase;
        break;
    }

    rc = KDBOpenPathTypeReadInt( mgr, dir, path, &ldir, &type, pathtype, rcobj,
        try_srapath, vpath );

    if (rc == 0)
    {
        if ( ppathtype != NULL )
            *ppathtype = type;

        if (pdir != NULL)
            *pdir = ldir;
        else
            KDirectoryRelease (ldir);
    }

    return rc;
}
