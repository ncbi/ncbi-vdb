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
 * =============================================================================
 */


#include <vfs/extern.h>

#include <kfg/ngc.h> /* KNgcObjGetTicket */

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfg/repository.h>
#include <kfg/config.h>

#ifdef ERR
#undef ERR
#endif

#include <klib/data-buffer.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/strings.h> /* ENV_MAGIC_REMOTE */
#include <klib/text.h>
#include <klib/time.h> /* KTime */
#include <klib/vector.h>

#include <kns/http.h>
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeReliableHttpFile */
#include <kns/manager.h>
#include <kns/stream.h>

#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>
#include <vfs/resolver-priv.h> /* VResolverQueryWithDir */

#include "services-priv.h"
#include "path-priv.h"
#include "resolver-cgi.h" /* RESOLVER_CGI */
#include "resolver-priv.h"

#include <sysalloc.h>

#include "../kns/mgr-priv.h" /* KNSManager */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/* to turn off CGI name resolution for
   any refseq accessions */
#define NO_REFSEQ_CGI 0

/* to turn off CGI name resolution for
   legacy WGS packages used by refseq */
#define NO_LEGACY_WGS_REFSEQ_CGI NO_REFSEQ_CGI

/* do not allow fragments to be returned
   unless we are expecting them for refseq */
#define DISALLOW_FRAGMENT NO_LEGACY_WGS_REFSEQ_CGI

/* the fail-over mechanism of allowing "aux" repositories */
#define ALLOW_AUX_REPOSITORIES 0

#define NAME_SERVICE_MAJ_VERS_ 1
#define NAME_SERVICE_MIN_VERS_ 2
#define ONE_DOT_ONE 0x01010000

#ifdef TESTING_SERVICES_VS_OLD_RESOLVING
    static uint32_t NAME_SERVICE_MAJ_VERS = NAME_SERVICE_MAJ_VERS_;
    static uint32_t NAME_SERVICE_MIN_VERS = NAME_SERVICE_MIN_VERS_;
    static uint32_t NAME_SERVICE_VERS
        = NAME_SERVICE_MAJ_VERS_ << 24 | NAME_SERVICE_MIN_VERS_ << 16;
#endif


/*--------------------------------------------------------------------------
 * String
 */
static
void CC string_whack ( void *obj, void *ignore )
{
    StringWhack ( ( String* ) obj );
}

/*--------------------------------------------------------------------------
 * VResolverAccToken
 *  breaks up an accession into parts
 *
 *  "acc" is entire accession as given
 *
 *  the remainder is divided like so:
 *
 *    [<prefix>_]<alpha><digits>[.<ext1>[.<ext2>]]
 *
 *  prefix is optional
 *  alpha can be zero length iff prefix is not zero length
 *  digits must be non-zero length
 *  ext1 and ext2 are optional
 */
typedef struct VResolverAccToken VResolverAccToken;
struct VResolverAccToken
{
    String acc;
    String prefix;
    String alpha;
    String digits;
    String ext1;
    String ext2;
    String suffix;
    bool   vdbcache;
    String accOfParentDb;  /* accession of parent DB for refseqs */
    int64_t projectId; /* < 0 : not set; >= 0: set (dbGaP projectId can be 0) */
};

static
void VResolverAccTokenInitFromOID ( VResolverAccToken *t, const String *acc )
{
    CONST_STRING ( & t -> prefix, "" );
    t -> alpha = t -> prefix;
    t -> acc = t -> digits = * acc;
    t -> ext1 = t -> ext2 = t -> prefix;
    t -> suffix = t -> prefix;

    t -> vdbcache = false;
}

/*--------------------------------------------------------------------------
 * VResolverAlg
 *  represents a set of zero or more volumes
 *  each of which is addressed using a particular expansion algorithm
 */
typedef struct VResolverAlg VResolverAlg;
struct VResolverAlg
{
    /* volume paths - stored as String* */
    Vector vols;

    /* root path - borrowed reference */
    const String *root;

    /* download ticket - borrowed reference
       non-NULL means that the root is a
       resolver CGI. also, don't rely on
       presence of any volumes... */
    const String *ticket;

    /* app_id helps to filter out volumes by app */
    VResolverAppID app_id;

    /* how to expand an accession */
    VResolverAlgID alg_id;

    /* a property of the repository */
    bool protected;

    /* whether the volumes are cache-capable
       in particular, enabled if cache forced */
    bool cache_capable;

    /* whether the volumes are cache-enabled */
    bool cache_enabled;

    /* whether the volume is disabled in config */
    bool disabled;
#if 0
    VRemoteProtocols protocols;
#endif

    uint32_t version; /* 3.0 or SDL ... */
};


/* Whack
 */
void CC VResolverAlgWhack ( void *item, void *ignore )
{
    VResolverAlg *self = item;

    /* drop any volumes */
    VectorWhack ( & self -> vols, string_whack, NULL );

    /* everything else is a borrowed reference */

    free ( self );
}

/* Make
 */
rc_t VResolverAlgMake ( VResolverAlg **algp, const String *root,
     VResolverAppID app_id, VResolverAlgID alg_id, bool protected, bool disabled )
{
    rc_t rc;
    VResolverAlg *alg = calloc ( 1, sizeof * alg );
    if ( alg == NULL )
        rc = RC ( rcVFS, rcMgr, rcConstructing, rcMemory, rcExhausted );
    else
    {
        VectorInit ( & alg -> vols, 0, 8 );
        alg -> root = root;
        alg -> app_id = app_id;
        alg -> alg_id = alg_id;
        alg -> protected = protected;
        alg -> disabled = disabled;
        rc = 0;
    }

    assert ( algp != NULL );
    * algp = alg;
    return rc;
}

static rc_t VResolverAlgMakeCgi(VResolverAlg **algp, const String *root,
    bool isProtected, bool disabled,
    const String *ticket, const char *name, VERSNS *versions)
{
    rc_t rc = VResolverAlgMake(algp, root, appAny, algCGI,
        isProtected, disabled);

    if (rc == 0) {
        assert(algp && *algp);

        (*algp)->ticket = ticket;

        if (name != NULL) {
            assert(versions);
            if (strcmp(name, "SDL.2") == 0) {
                (*algp)->version = 0x82000000;
                *versions |= versSDL2;
            }
            else if (strcmp(name, "CGI") == 0)
                (*algp)->version = 0x03000000;
            else if (strcmp(name, "CGI.4") == 0)
                (*algp)->version = 0x04000000;
            else if (strcmp(name, "SDL.1") == 0)
                (*algp)->version = 0x81000000;
        }
    }

    return rc;
}

static
int64_t CC VResolverAlgSort ( const void ** a, const void ** b, void * ignore )
{
    const VResolverAlg * aa = * a;
    const VResolverAlg * ab = * b;

    /* second key is algorithm id */
    return ( int64_t ) aa -> alg_id - ( int64_t ) ab -> alg_id;
}

/* MakeLocalWGSRefseqURI
 *  create a special URI that tells KDB how to open this
 *  obscured table, hidden away within a KAR file
 */
static
rc_t VResolverAlgMakeLocalWGSRefseqURI ( const VResolverAlg *self,
    const String *vol, const String *exp, const String *acc, const VPath ** path )
{
    if ( self -> root == NULL )
        return VPathMakeFmt ( ( VPath** ) path, NCBI_FILE_SCHEME ":%S/%S#tbl/%S", vol, exp, acc );
    return VPathMakeFmt ( ( VPath** ) path, NCBI_FILE_SCHEME ":%S/%S/%S#tbl/%S", self -> root, vol, exp, acc );
}

/* MakeRemoteWGSRefseqURI
 *  create a special URI that tells KDB how to open this
 *  obscured table, hidden away within a KAR file
 */
static
rc_t VResolverAlgMakeRemoteWGSRefseqURI ( const VResolverAlg *self,
    const char *url, const String *acc, const VPath ** path )
{
    return VPathMakeFmt ( ( VPath** ) path, "%s#tbl/%S", url, acc );
}

/* MakeRemotePath
 *  the path is known to exist in the remote file system
 *  turn it into a VPath
 */
static
rc_t VResolverAlgMakeRemotePath ( const VResolverAlg *self,
    const char *url, const VPath ** path )
{
    return VPathMakeFmt ( ( VPath** ) path, "%s", url );
}

/* MakeLocalPath
 *  the path is known to exist in the local file system
 *  turn it into a VPath
 */
static
rc_t VResolverAlgMakeLocalPath ( const VResolverAlg *self,
    const String *vol, const String *exp, const VPath ** path,
    const KDirectory * wd )
{
    if ( self -> root == NULL )
        return VPathMakeFmt ( ( VPath** ) path, "%S/%S", vol, exp );

    if (wd == NULL)
        return VPathMakeFmt ( ( VPath** ) path, "%S/%S/%S",
            self -> root, vol, exp );
    else {
        char resolved [ PATH_MAX ] = "";
        rc_t rc = KDirectoryResolvePath (wd, true, resolved, sizeof resolved,
            "%.*s/%.*s/%.*s", (int)self->root->size, self->root->addr,
            (int)vol->size, vol->addr, (int)exp->size, exp->addr );
        if ( rc != 0 )
            return VPathMakeFmt ( ( VPath** ) path, "%S/%S/%S",
                self -> root, vol, exp );
        else
            return VPathMakeFmt ( ( VPath** ) path, "%s", resolved );
    }
}

static rc_t VResolverMakeAbsPath ( const String * dir, const String * exp,
                                   const VPath ** path )
{
    return VPathMakeFmt ( ( VPath** ) path, "%S/%S", dir, exp );
}

/* MakeLocalFilePath
 *  the path is known to exist in the local file system
 *  turn it into a VPath
 */
static
rc_t VResolverAlgMakeLocalFilePath ( const VResolverAlg *self,
    const String *vol, const String *exp, const char *krypto_ext, const VPath ** path )
{
    if ( self -> root == NULL )
        return VPathMakeFmt ( ( VPath** ) path, "%S/%S%s", vol, exp, krypto_ext );
    return VPathMakeFmt ( ( VPath** ) path, "%S/%S/%S%s", self -> root, vol, exp, krypto_ext );
}

/* expand_accession
 *  expand accession according to algorithm
 */
static
rc_t expand_algorithm ( const VResolverAlg *self, const VResolverAccToken *tok,
    char *expanded, size_t bsize, size_t *size, bool legacy_wgs_refseq )
{
    rc_t rc;
    uint32_t num;

    assert(tok);

    switch ( self -> alg_id )
    {
    case algCGI:
        return RC ( rcVFS, rcResolver, rcResolving, rcType, rcIncorrect );
    case algFlat:
        rc = string_printf ( expanded, bsize, size, "%S", & tok -> acc );
        break;
    case algFlatAD:
        if (tok->projectId < 0)
            rc = string_printf(expanded, bsize, size, "%S", &tok->acc);
        else
            rc = string_printf(expanded, bsize, size, "dbGaP-%d/%S",
                tok->projectId, &tok->acc);
        break;
    case algWithExtFlat:
        if (tok->projectId < 0)
            rc = string_printf ( expanded, bsize, size,
                "%S%S.%S", & tok -> alpha, & tok -> digits,
                & tok -> ext1 );
        else
            rc = string_printf ( expanded, bsize, size,
                "%S%S_dbGaP-%d.%S", & tok -> alpha, & tok -> digits,
                tok -> projectId, & tok -> ext1 );
        break;
    case algAD:
        if (tok->projectId < 0)
            rc = string_printf ( expanded, bsize, size,
                "%S%S/%S%S.%S", & tok -> alpha, & tok -> digits,
                & tok -> alpha, & tok -> digits,
                & tok -> ext1 );
        else
            rc = string_printf ( expanded, bsize, size,
                "%S%S/%S%S_dbGaP-%d.%S", & tok -> alpha, & tok -> digits,
                & tok -> alpha, & tok -> digits, tok -> projectId,
                & tok -> ext1 );
        break;
    case algSRAAD:
        if (tok->projectId < 0)
            rc = string_printf ( expanded, bsize, size,
                "%S%S/%S%S.%s", & tok -> alpha, & tok -> digits,
                & tok -> alpha, & tok -> digits,
                tok -> vdbcache ? "sra.vdbcache" : "sra" );
        else
            rc = string_printf ( expanded, bsize, size,
                "%S%S/%S%S_dbGaP-%d.%s", & tok -> alpha, & tok -> digits,
                & tok -> alpha, & tok -> digits, tok -> projectId,
                tok -> vdbcache ? "sra.vdbcache" : "sra" );
        break;
    case algSRAFlat:
        if (tok->projectId < 0)
            rc = string_printf ( expanded, bsize, size,
                "%S%S.%s", & tok -> alpha, & tok -> digits,
                tok -> vdbcache ? "sra.vdbcache" : "sra" );
        else
            rc = string_printf ( expanded, bsize, size,
                "%S%S_dbGaP-%d.%s", & tok -> alpha, & tok -> digits,
                tok -> projectId,
                tok -> vdbcache ? "sra.vdbcache" : "sra" );
        break;
    case algSRA1024:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "%S/%06u/%S%S.%s", & tok -> alpha, num >> 10, & tok -> alpha, & tok -> digits,
                               tok -> vdbcache ? "sra.vdbcache" : "sra" );
        break;
    case algSRA1000:
        num = ( uint32_t ) ( tok -> alpha . size + tok -> digits . size - 3 );
        rc = string_printf ( expanded, bsize, size,
            "%S/%.*S/%S%S.%s", & tok -> alpha, num, & tok -> acc, & tok -> alpha, & tok -> digits,
                               tok -> vdbcache ? "sra.vdbcache" : "sra" );
        break;
    case algFUSE1000:
        num = ( uint32_t ) ( tok -> alpha . size + tok -> digits . size - 3 );
        rc = string_printf ( expanded, bsize, size,
            "%S/%.*S/%S%S/%S%S.%s", & tok -> alpha, num, & tok -> acc,
            & tok -> alpha, & tok -> digits, & tok -> alpha, & tok -> digits,
            tok -> vdbcache ? "sra.vdbcache" : "sra" );
        break;
    case algREFSEQ:
        if ( ! legacy_wgs_refseq )
            rc = string_printf ( expanded, bsize, size, "%S", & tok -> acc );
        else
            rc = string_printf ( expanded, bsize, size, "%S%.2S", & tok -> alpha, & tok -> digits );
        break;
    case algREFSEQAD:
        if (!legacy_wgs_refseq)
            rc = string_printf(expanded, bsize, size,
                "%S%s%S", &tok->accOfParentDb,
                tok->accOfParentDb.size == 0 ? "" : "/",
                &tok->acc);
        else
            rc = string_printf(expanded, bsize, size,
                "%S/%S%.2S", &tok->accOfParentDb, &tok->alpha, &tok->digits);
        break;
    case algWGSFlat:
        num = ( uint32_t ) ( tok -> alpha . size + 2 );
        if ( tok -> prefix . size != 0 )
            num += (uint32_t) ( tok -> prefix . size + 1 );
        rc = string_printf ( expanded, bsize, size,
            "%.*S", num, & tok -> acc );
        break;
    case algWGS2:
    case algWGS:
        num = ( uint32_t ) ( tok -> alpha . size + 2 );
        if ( tok -> prefix . size != 0 )
            num += (uint32_t) ( tok -> prefix . size + 1 );
        if ( tok -> alpha . size == 6 )
        {
            if ( tok -> ext1 . size == 0 )
            {
                rc = string_printf ( expanded, bsize, size,
                    "WGS/%.2s/%.2s/%.2s/%.*S", tok -> alpha . addr, tok -> alpha . addr + 2, tok -> alpha . addr + 4, num, & tok -> acc );
            }
            else
            {
                rc = string_printf ( expanded, bsize, size,
                    "WGS/%.2s/%.2s/%.2s/%.*S.%S", tok -> alpha . addr, tok -> alpha . addr + 2, tok -> alpha . addr + 4, num, & tok -> acc, & tok -> ext1 );
            }
        }
        else
        {
            if ( tok -> ext1 . size == 0 )
            {
                rc = string_printf ( expanded, bsize, size,
                    "WGS/%.2s/%.2s/%.*S", tok -> alpha . addr, tok -> alpha . addr + 2, num, & tok -> acc );
            }
            else
            {
                rc = string_printf ( expanded, bsize, size,
                    "WGS/%.2s/%.2s/%.*S.%S", tok -> alpha . addr, tok -> alpha . addr + 2, num, & tok -> acc, & tok -> ext1 );
            }
        }
        break;
    case algFuseWGS:
        num = ( uint32_t ) ( tok -> alpha . size + 2 );
        if ( tok -> prefix . size != 0 )
            num += (uint32_t) ( tok -> prefix . size + 1 );
        rc = string_printf ( expanded, bsize, size,
            "%.2s/%.2s/%.*S", tok -> alpha . addr, tok -> alpha . addr + 2, num, & tok -> acc );
        break;
    case algSRA_NCBI:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "%S/%06u/%S%S%s", & tok -> alpha, num >> 10, & tok -> alpha, & tok -> digits,
                              tok -> vdbcache ? ".vdbcache" : "" );
        break;
    case algSRA_EBI:
        num = ( uint32_t ) ( tok -> alpha . size + tok -> digits . size - 3 );
        rc = string_printf ( expanded, bsize, size,
            "%S/%.*S/%S%S%s", & tok -> alpha, num, & tok -> acc, & tok -> alpha, & tok -> digits,
                              tok -> vdbcache ? ".vdbcache" : "" );
        break;

    case algNANNOTFlat:
        rc = string_printf ( expanded, bsize, size, "%S", & tok -> acc );
        break;
    case algNANNOT:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "%03u/%03u/%S", num / 1000000, ( num / 1000 ) % 1000, & tok -> acc );
        break;
    case algFuseNANNOT:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "%03u/%03u/%S", num / 1000000, ( num / 1000 ) % 1000, & tok -> acc );
        break;

    case algNAKMERFlat:
        rc = string_printf ( expanded, bsize, size, "%S", & tok -> acc );
        break;
    case algNAKMER:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "kmer/%03u/%03u/%S", num / 1000000, ( num / 1000 ) % 1000, & tok -> acc );
        break;
    case algFuseNAKMER:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
            "kmer/%03u/%03u/%S", num / 1000000, ( num / 1000 ) % 1000, & tok -> acc );
        break;

    case algPileup_NCBI:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
             "SRZ/%06u/%S%S/%S", num / 1000, & tok -> alpha, & tok -> digits, & tok -> acc );
        break;

    case algPileup_EBI:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
             "ERZ/%06u/%S%S/%S", num / 1000, & tok -> alpha, & tok -> digits, & tok -> acc );
        break;
    case algPileup_DDBJ:
        num = ( uint32_t ) strtoul ( tok -> digits . addr, NULL, 10 );
        rc = string_printf ( expanded, bsize, size,
             "DRZ/%06u/%S%S/%S", num / 1000, & tok -> alpha, & tok -> digits, & tok -> acc );
        break;

    default:
        return RC ( rcVFS, rcResolver, rcResolving, rcType, rcUnrecognized );
    }

   return rc;
}

/* LocalResolve
 *  resolve an accession into a VPath or not found
 *
 *  1. expand accession according to algorithm
 *  2. search all volumes for accession
 *  3. return not found or new VPath
 */
static
rc_t VResolverAlgLocalResolve ( const VResolverAlg *self,
    const KDirectory *wd, const VResolverAccToken *tok, const VPath ** path,
    bool legacy_wgs_refseq, bool for_cache, const char * dir, bool ad )
{
    KPathType kpt;
    uint32_t i, count;

    /* expanded accession */
    String exp;
    size_t size;
    char expanded [ 256 ];

    /* in some cases, "root" is NULL */
    const String *vol, *root = self -> root;

    /* expand the accession */
    rc_t rc = expand_algorithm ( self, tok, expanded, sizeof expanded, & size, legacy_wgs_refseq );

    /* should never have a problem here... */
    if ( rc != 0 )
        return rc;

    assert(tok);

    /* if this is to detect a cache file, append extension */
    if ( for_cache )
    {
        size += string_copy ( & expanded [ size ], sizeof expanded - size, ".cache", sizeof ".cache" - 1 );
        if ( size == sizeof expanded )
            return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    }

    /* turn the expanded portion into a String
       we know that size is also length due to
       accession content rules */
    StringInit ( & exp, expanded, size, ( uint32_t ) size );

    /* remove the cache extension */
    if ( for_cache )
    {
        exp . len -= sizeof ".cache" - 1;
        exp . size -= sizeof ".cache" - 1;
    }

    count = VectorLength ( & self -> vols );

    if ( dir != NULL ) {
        String sDir;
        String sEmpty;
        StringInitCString ( & sDir, dir );
        CONST_STRING ( & sEmpty, "" );
        kpt = KDirectoryPathType ( wd, "%s/%.*s",
                          dir , ( int ) size, expanded );
        switch ( kpt & ~ kptAlias )
        {
        case kptFile:
        case kptDir:
            if ( legacy_wgs_refseq )
                return VResolverAlgMakeLocalWGSRefseqURI (
                    self, & sDir, & sEmpty, & tok -> acc, path );
            return VResolverMakeAbsPath ( & sDir, & exp, path );
        default:
            break;
        }
    }

    /* now search all volumes */
    else if ( root == NULL )
    {
        for ( i = 0; i < count; ++ i )
        {
            vol = VectorGet ( & self -> vols, i );
            kpt = KDirectoryPathType ( wd, "%.*s/%.*s"
                , ( int ) vol -> size, vol -> addr
                , ( int ) size, expanded );
            switch ( kpt & ~ kptAlias )
            {
            case kptFile:
            case kptDir:
                if ( legacy_wgs_refseq )
                    return VResolverAlgMakeLocalWGSRefseqURI ( self, vol, & exp, & tok -> acc, path );
                return VResolverAlgMakeLocalPath ( self, vol, & exp, path,
                    NULL );
            default:
                break;
            }
        }
    }
    else
    {
        for ( i = 0; i < count; ++ i )
        {
            vol = VectorGet ( & self -> vols, i );
            kpt = KDirectoryPathType ( wd, "%.*s/%.*s/%.*s"
                , ( int ) root -> size, root -> addr
                , ( int ) vol -> size, vol -> addr
                , ( int ) size, expanded );
            switch ( kpt & ~ kptAlias )
            {
            case kptFile:
            case kptDir:
                if ( legacy_wgs_refseq )
                    return VResolverAlgMakeLocalWGSRefseqURI ( self, vol, & exp, & tok -> acc, path );
                rc = VResolverAlgMakeLocalPath ( self, vol, & exp, path,
                    ad ? wd : NULL );
                if (rc == 0) {
                    assert(path);
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), (
                        "VResolverAlgLocalResolve: '%S' found in '%S%s'\n",
                        &tok->acc, &(*path)->path, for_cache ? ".cache" : ""));
                }
                return rc;
            default:
                break;
            }
        }
    }

    return SILENT_RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

/* LocalFile
 *  resolve an file name into a VPath or not found
 */
static
rc_t VResolverAlgLocalFile ( const VResolverAlg *self,
    const KDirectory *wd, const VPath *query,
    const VPath ** path, bool for_cache )
{
    KPathType kpt;
    uint32_t i, count;

    /* in some cases, "root" is NULL */
    const String *vol, *root = self -> root;

    /* the file name */
    String fname = query -> path;

    /* cache extension */
    const char *cache_ext = for_cache ? ".cache" : "";

    /* encryption extension */
    const char *krypto_ext = self -> protected ? ".ncbi_enc" : "";

    /* now search all volumes */
    count = VectorLength ( & self -> vols );
    if ( root == NULL )
    {
        for ( i = 0; i < count; ++ i )
        {
            vol = VectorGet ( & self -> vols, i );
            kpt = KDirectoryPathType ( wd
                , "%.*s/%.*s%s%s"
                , ( int ) vol -> size, vol -> addr
                , ( int ) fname . size, fname . addr
                , krypto_ext
                , cache_ext
            );

            switch ( kpt & ~ kptAlias )
            {
            case kptFile:
            case kptDir:
                return VResolverAlgMakeLocalFilePath ( self, vol, & fname, krypto_ext, path );
            default:
                break;
            }

            if ( krypto_ext [ 0 ] != 0 )
            {
                kpt = KDirectoryPathType ( wd
                    , "%.*s/%.*s%s"
                    , ( int ) vol -> size, vol -> addr
                    , ( int ) fname . size, fname . addr
                    , cache_ext
                );

                switch ( kpt & ~ kptAlias )
                {
                case kptFile:
                case kptDir:
                    return VResolverAlgMakeLocalFilePath ( self, vol, & fname, "", path );
                default:
                    break;
                }
            }
        }
    }
    else
    {
        for ( i = 0; i < count; ++ i )
        {
            vol = VectorGet ( & self -> vols, i );
            kpt = KDirectoryPathType ( wd
                , "%.*s/%.*s/%.*s%s%s"
                , ( int ) root -> size, root -> addr
                , ( int ) vol -> size, vol -> addr
                , ( int ) fname . size, fname . addr
                , krypto_ext
                , cache_ext
            );

            switch ( kpt & ~ kptAlias )
            {
            case kptFile:
            case kptDir:
                return VResolverAlgMakeLocalFilePath ( self, vol, & fname, krypto_ext, path );
            default:
                break;
            }

            if ( krypto_ext [ 0 ] != 0 )
            {
                kpt = KDirectoryPathType ( wd
                    , "%.*s/%.*s/%.*s%s"
                    , ( int ) root -> size, root -> addr
                    , ( int ) vol -> size, vol -> addr
                    , ( int ) fname . size, fname . addr
                    , cache_ext
                );

                switch ( kpt & ~ kptAlias )
                {
                case kptFile:
                case kptDir:
                    return VResolverAlgMakeLocalFilePath ( self, vol, & fname, "", path );
                default:
                    break;
                }
            }
        }
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

rc_t VPathCheckFromNamesCGI ( const VPath * path,
    const String *ticket, int64_t projectId, const VPath ** mapping )
{
    size_t i, size;
    const char * start;

    /* must have an explicit scheme */
    if ( ! path -> from_uri )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );

    /* can only be http, https or fasp */
    switch ( path -> scheme_type )
    {
    case vpuri_http:
    case vpuri_https:
    case vpuri_fasp:
        break;
    default:
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }


    /* must have a host-spec with all ascii-characters */
    switch ( path -> host_type )
    {
    case vhDNSName:
        if ( path -> host . size == 0 || path -> host . size != ( size_t ) path -> host . len )
            return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
        start = path -> host . addr;
        size = path -> host . size;
        for ( i = 0; i < size; ++ i )
        {
            if ( isalnum ( start [ i ] ) )
                continue;
            switch ( start [ i ] )
            {
            case '.':
            case '-':
            case '_':
                continue;
            }
            return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
        }
        break;
    case vhIPv4:
    case vhIPv6:
        break;
    }

    /* must have a full-path */
    if ( path -> path_type != vpFullPath )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    /* only ascii characters */
    assert ( path -> path . size != 0 );
    if ( path -> path . size != ( size_t ) path -> path . len )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    start = path -> path . addr;
    size = path -> path . size;
    for ( i = 0; i < size; ++ i )
    {
        if ( isalnum ( start [ i ] ) )
            continue;
        switch ( start [ i ] )
        {
        case '/':
        case '.':
        case '-':
        case '_':
            continue;
        }
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

#if DO_NOT_USE_TIC_HACK
    /* if the ticket was placed into the mapped path */
    if ( mapping != NULL )
        ticket = NULL;
#endif

    if ( path -> query . size != 0 )
    {
        bool skip = false;

        String name, val, req, host;

        if (!skip) {
            CONST_STRING(&host, "locate.ncbi.nlm.nih.gov");
            /* redirector URLs have query */
            if (StringEqual(&path->host, &host))
                skip = true;
        }
        if (!skip) {
            CONST_STRING(&host, "nih-nhlbi-datacommons.s3.amazonaws.com");
            /* amazonaws URLs can have query */
            if (StringEqual(&path->host, &host))
                skip = true;
        }
        if (!skip) {
            CONST_STRING(&host, "storage.googleapis.com");
            /* googleapis URLs can have query */
            if (StringEqual(&path->host, &host))
                skip = true;
        }
        if (!skip) {
            CONST_STRING(&host, "trace.ncbi.nlm.nih.gov");
            /* redirector URLs have query */
            if (StringEqual(&path->host, &host))
                skip = true;
        }

        if (!skip) {
            /* query must match ticket */
            if (ticket == NULL)
                return RC(rcVFS, rcResolver, rcResolving,
                    rcMessage, rcCorrupt);

            StringSubstr(&path->query, &name, 0, 5);
            StringSubstr(&path->query, &val, 5, 0);

            CONST_STRING(&req, "?tic=");
            if (StringEqual(&name, &req)) {
                if (!StringEqual(&val, ticket))
                    return RC(rcVFS, rcResolver, rcResolving,
                        rcMessage, rcCorrupt);
            }
            else {
                CONST_STRING(&req, "?pId=");
                if (!StringEqual(&name, &req))
                    return RC(rcVFS, rcResolver, rcResolving,
                        rcMessage, rcCorrupt);
                else if (projectId < 0)
                    return RC(rcVFS, rcResolver, rcResolving,
                        rcMessage, rcCorrupt);
                else {
                    String s;
                    char b[256] = "";
                    rc_t rc = string_printf(b, sizeof b, NULL, "%d", projectId);
                    if (rc != 0)
                        return rc;
                    StringInitCString(&s, b);
                    if (!StringEqual(&val, &s))
                        return RC(rcVFS, rcResolver, rcResolving,
                            rcMessage, rcCorrupt);
                }
            }
        }
    }

#if DISALLOW_FRAGMENT
    /* cannot have a fragment */
    if ( path -> fragment . size != 0 )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
#endif

    return 0;
}


/* ParseResolverCGIResponse_1_0
 *  expect single row table, with this structure:
 *
 *  <accession>|<download-ticket>|<url>|<result-code>|<message>
 */
static
rc_t VResolverAlgParseResolverCGIResponse_1_0 ( const char *start, size_t size,
    const VPath ** path, const VPath ** ignore, const String *acc,
    const String *ticket )
{
    rc_t rc;
    KLogLevel lvl;
    char *rslt_end;
    uint32_t result_code;

    String accession, download_ticket, url, rslt_code, msg;

    /* get accession */
    const char *end = start + size;
    const char *sep = string_chr ( start, size, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & accession, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get download-ticket */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & download_ticket, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get url */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & url, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get result-code */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & rslt_code, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get msg */
    start = sep + 1;
    for ( sep = end; sep > start; -- sep )
    {
        switch ( sep [ -1 ] )
        {
        case '\n':
        case '\r':
            continue;
        default:
            break;
        }

        break;
    }
    StringInit ( & msg, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* compare acc to accession */
    assert(acc);
    if ( ! StringEqual ( & accession, acc ) )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );

    /* compare ticket
       currently this makes sense with 1 request from a known workspace */
    if ( download_ticket . size != 0 )
    {
        if ( ticket == NULL || ! StringEqual ( & download_ticket, ticket ) )
            return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

    /* get the result code */
    if ( rslt_code . size == 0 )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    result_code = strtoul ( rslt_code . addr, & rslt_end, 10 );
    if ( ( const char* ) rslt_end - rslt_code . addr != rslt_code . size )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );

    /* still have to test the URL */

    switch ( result_code / 100 )
    {
    case 1:
        /* informational response
           not much we can do here */
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 2:
        /* successful response
           but can only handle 200 */
        if ( result_code == 200 )
        {
            /* normal public response */
            if ( download_ticket . size == 0 )
                rc = VPathMakeFmt ( ( VPath** ) path, "%S", & url );
            else
            {
                /* protected response */
                rc = VPathMakeFmt ( ( VPath** ) path, "%S?tic=%S", & url, & download_ticket );
            }

            if ( rc == 0 )
            {
                rc = VPathCheckFromNamesCGI ( * path, ticket, -1, NULL );
                if ( rc == 0 )
                    return 0;

                VPathRelease ( * path );
                * path = NULL;
            }

            return rc;
        }

        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 3:
        /* redirection
           currently this is being handled by our request object */
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 4:
        /* client error */
        lvl = klogErr;
        switch ( result_code )
        {
        case 400:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcInvalid );
            break;
        case 401:
        case 403:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcQuery, rcUnauthorized );
            break;
        case 404:
            return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
        case 410:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
            break;
        default:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        }
        break;

    case 5:
        /* server error */
        lvl = klogSys;
        switch ( result_code )
        {
        case 503:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcDatabase, rcNotAvailable );
            break;
        case 504:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcTimeout, rcExhausted );
            break;
        default:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        }
        break;

    default:
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
    }

    /* log message to user */
    PLOGERR ( lvl, ( lvl, rc, "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
        "acc=%S,msg=%S,code=%u", acc, & msg, result_code ) );
    return rc;
}

static int getDigit ( char c, rc_t * rc ) {
     assert ( rc );

     if ( * rc != 0 )
         return 0;

     c = tolower ( c );
     if ( ! isdigit ( c ) && c < 'a' && c > 'f' ) {
         * rc = RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
         return 0;
     }

     if ( isdigit ( c ) )
         return c - '0';

     return c - 'a' + 10;
}

/* ParseResolverCGIResponse_1_1
 *  expect single row table, with this structure (SRA-1690) :
 *
 *  <accession>|obj-id|name|size|mod-date|md5|<download-ticket>|<url>|<result-code>|<message>
 */
static
rc_t VResolverAlgParseResolverCGIResponse_1_1 ( const char *astart, size_t size,
    const VPath ** path, const VPath ** mapping, const String *acc,
    const String *ticket )
{
    const char *start = astart;
    rc_t rc = 0;
    KLogLevel lvl;
    char *rslt_end;
    uint32_t result_code;

    String accession, obj_id, name, size_str, mod_date, md5, download_ticket, url, rslt_code, msg;

    /* get accession */
    const char *end = start + size;
    const char *sep = string_chr ( start, size, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & accession, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get obj-id */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & obj_id, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get name */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & name, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get size */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & size_str, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get mod-date */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & mod_date, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get md5 */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & md5, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get download-ticket */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & download_ticket, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get url */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & url, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get result-code */
    start = sep + 1;
    sep = string_chr ( start, end - start, '|' );
    if ( sep == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    StringInit ( & rslt_code, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* get msg */
    start = sep + 1;
    for ( sep = end; sep > start; -- sep )
    {
        switch ( sep [ -1 ] )
        {
        case '\n':
        case '\r':
            continue;
        default:
            break;
        }

        break;
    }
    StringInit ( & msg, start, sep - start, ( uint32_t ) ( sep - start ) );

    /* compare acc to accession or obj_id */
    assert(acc);
    if ( ! StringEqual ( & accession, acc ) && ! StringEqual ( & obj_id, acc ) ) {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
            "@@@@@@@@2 %%s:%s:%d: %s"
                "\n", __FILE__, __func__, __LINE__, astart));
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

    /* compare ticket
       currently this makes sense with 1 request from a known workspace */
    if ( download_ticket . size != 0 )
    {
        if ( ticket == NULL || ! StringEqual ( & download_ticket, ticket ) )
            return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

    /* get the result code */
    if ( rslt_code . size == 0 )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    result_code = strtoul ( rslt_code . addr, & rslt_end, 10 );
    if ( ( const char* ) rslt_end - rslt_code . addr != rslt_code . size )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );

    /* still have to test the URL */

    switch ( result_code / 100 )
    {
    case 1:
        /* informational response
           not much we can do here */
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 2:
        /* successful response
           but can only handle 200 */
        if ( result_code == 200 )
        {
            uint8_t ud5 [ 16 ];
            bool has_md5 = false;
            KTime_t date = 0;
            uint64_t osize = 0;
            if ( size_str . size != 0  && size_str . len != 0 ) {
                rc_t r2 = 0;
                osize = StringToU64 ( & size_str, & r2 );
                if ( r2 != 0 )
                    osize = 0;
            }
            if ( mod_date . addr != NULL && mod_date . size > 0 ) {
                KTime kt;
                const KTime * t = KTimeFromIso8601 ( & kt, mod_date . addr,
                    mod_date . size );
                if ( t != NULL )
                    date = KTimeMakeTime ( & kt );
            }
            if ( md5 . addr != NULL && md5 . size == 32 ) {
                int i = 0;
                for ( i = 0; i < 16 && rc == 0; ++ i ) {
                    ud5 [ i ]  = getDigit ( md5 . addr [ 2 * i ], & rc ) * 16;
                    ud5 [ i ] += getDigit ( md5 . addr [ 2 * i + 1 ], & rc );
                }
                has_md5 = rc == 0;
            }
            /* normal public response *
            if ( download_ticket . size == 0
#if DO_NOT_USE_TIC_HACK
                 || mapping != NULL
#endif
                )*/
            {
                const String * id = & accession;
                if ( id -> size == 0 )
                    id = & obj_id;
                rc = VPathMakeFromUrl ( ( VPath** ) path, & url,
                    & download_ticket, true, id, osize, date,
                    has_md5 ? ud5 : NULL, 0, NULL, NULL, NULL, false, false,
                    NULL, -1, 0 );
            }
            /*else
            {
                * protected response *
                rc = VPath MakeFmtExt ( ( VPath** ) path, true, & accession,
                    size, date, "%S?tic=%S", & url, & download_ticket );
            }*/

            if ( rc == 0 )
            {
                rc = VPathCheckFromNamesCGI ( * path, ticket, -1, mapping );
                if ( rc == 0 )
                {
                    if ( mapping == NULL )
                        return 0;

                    if ( download_ticket . size != 0 )
                    {
                        if ( accession . size != 0 )
                            rc = VPathMakeFmt ( ( VPath** ) mapping, "ncbi-acc:%S?tic=%S", & accession, & download_ticket );
                        else if ( name . size == 0 )
                            return 0;
                        else
                            rc = VPathMakeFmt ( ( VPath** ) mapping, "ncbi-file:%S?tic=%S", & name, & download_ticket );
                    }
                    else if ( accession . size != 0 )
                        rc = VPathMakeFmt ( ( VPath** ) mapping, "ncbi-acc:%S", & accession );
                    else if ( name . size == 0 )
                        return 0;
                    else
                        rc = VPathMakeFmt ( ( VPath** ) mapping, "ncbi-file:%S", & name );

                    if ( rc == 0 )
                        return 0;
                }

                VPathRelease ( * path );
                * path = NULL;
            }

            return rc;
        }

        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 3:
        /* redirection
           currently this is being handled by our request object */
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        break;

    case 4:
        /* client error */
        lvl = klogErr;
        switch ( result_code )
        {
        case 400:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcInvalid );
            break;
        case 401:
        case 403:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcQuery, rcUnauthorized );
            break;
        case 404: /* 404|no data :
                    If it is a real response then this assession is not found.
                    What if it is a DB failure? Will be retried if configured to do so? */
            return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
        case 410:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
            break;
        default:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        }
        break;

    case 5:
        /* server error */
        lvl = klogSys;
        switch ( result_code )
        {
        case 503:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcDatabase, rcNotAvailable );
            break;
        case 504:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcTimeout, rcExhausted );
            break;
        default:
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
        }
        break;

    default:
        lvl = klogInt;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
    }

    /* log message to user */
    PLOGERR ( lvl, ( lvl, rc, "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
        "acc=%S,msg=%S,code=%u", acc, & msg, result_code ) );
    return rc;
}


/* ParseResolverCGIResponse_2_0
 *  expect a table type line followed by the table
 */
static
rc_t VResolverAlgParseResolverCGIResponse_2_0 ( const char *start, size_t size,
    const VPath ** path, const VPath ** mapping, const String *acc,
    const String *ticket )
{
    size_t i;

    if ( string_cmp ( start, size, "name-resolver", sizeof "name-resolver" - 1, sizeof "name-resolver" - 1 ) == 0 )
    {
        do
        {
            /* accept table type line */
            i = sizeof "name-resolver" - 1;

            /* must be followed by eoln */
            if ( start [ i ] == '\r' && start [ i + 1 ] == '\n' )
                i += 2;
            else if ( start [ i ] == '\n' )
                i += 1;
            else
                break;

            /* parse as 1.1 response table */
            return VResolverAlgParseResolverCGIResponse_1_1 ( & start [ i ],
                size - i, path, mapping, acc, ticket );
        }
        while ( false );
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

typedef enum {
    vBad,
    v1_0,
    v1_1,
    v1_2,
    v2,
    v3,
} TVersion;

/* ParseResolverCGIResponse
 *  the response should be NUL terminated
 *  but should also be close to the size of result
 */
rc_t VResolverAlgParseResolverCGIResponse ( const KDataBuffer *result,
    const VPath ** path, const VPath ** mapping, const String *acc,
    const String *ticket )
{
    const char V1_0[] = "#1.0";
    const char V1_1[] = "#1.1";
    const char V1_2[] = "#1.2";
    const char V2  [] = "#2.0";
    const char V3  [] = "#3.0";
    struct {
        const char *c;
        size_t s;
        TVersion v;
        rc_t (*f)( const char *start, size_t size, const VPath **path,
            const VPath **mapping, const String *acc, const String *ticket);
    } version[] = {
        {V1_1, sizeof V1_1 - 1, v1_1, VResolverAlgParseResolverCGIResponse_1_1},
        {V1_2, sizeof V1_2 - 1, v1_2, VResolverAlgParseResolverCGIResponse_1_1},
        {V3  , sizeof V3   - 1, v3  , VResolverAlgParseResolverCGIResponse_3_0},
        {V1_0, sizeof V1_0 - 1, v1_0, VResolverAlgParseResolverCGIResponse_1_0},
        {V2  , sizeof V2   - 1, v2  , VResolverAlgParseResolverCGIResponse_2_0},
    };

    size_t size = 0;
    int iVersion = sizeof version / sizeof version[0];

    /* the textual response */
    size_t i = 0;
    const char *start = NULL;

    assert(result);

    start = ( const void* ) result -> base;
    size = KDataBufferBytes ( result );

    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
        (" Response = %.*s\n", ( int ) size, start));

    /* peel back buffer to significant bytes */
    while ( size > 0 && start [ size - 1 ] == 0 ) -- size;

    /* skip over blanks */
    for ( i = 0; i < size; ++ i ) { if ( ! isspace ( start [ i ] ) ) break; }

    for (iVersion = 0;
        iVersion < sizeof version / sizeof *version; ++iVersion)
    {
        if (string_cmp(&start[i], size - i, version[iVersion].c,
            version[iVersion].s, ( uint32_t ) version[iVersion].s) == 0)
        {
            break;
        }
    }
    switch (iVersion) {
        default:
            if (string_cmp(&start[i], size - i, version[iVersion].c,
                version[iVersion].s, ( uint32_t ) version[iVersion].s) == 0)
            {
                /* accept version line */
                i += version[iVersion].s;

                /* must be followed by eoln */
                if ( start [ i ] == '\r' && start [ i + 1 ] == '\n' )
                    i += 2;
                else if ( start [ i ] == '\n' )
                    i += 1;
                else
                    return
                        RC(rcVFS, rcResolver, rcResolving, rcName, rcNotFound);

                /* parse response table */
                return version[iVersion].f
                    (&start[i], size - i, path, mapping, acc, ticket);
            }
            /* no break */
        case sizeof version / sizeof version[0]:
            return RC(rcVFS, rcResolver, rcResolving, rcName, rcNotFound);
    }
}


#ifdef VDB_3162
#error 1
/*  test-only code to emulate 403 response while calling names.cgi */
static bool TEST_VDB_3162 = false;
void TESTING_VDB_3162 ( void ) {
    TEST_VDB_3162 = true;
}
static uint32_t TESTING_VDB_3162_CODE ( rc_t rc, uint32_t code ) {
    if ( rc == 0 && TEST_VDB_3162 ) {
        TEST_VDB_3162 = false;
        return 403;
    } else {
        return code;
    }
}
#endif

#ifdef TESTING_SERVICES_VS_OLD_RESOLVING
/* RemoteProtectedResolve
 *  use NCBI CGI to resolve accession into URL
 */
static
rc_t oldVResolverAlgRemoteProtectedResolve( const VResolverAlg *self,
    const KNSManager *kns, VRemoteProtocols protocols, const String *acc,
    const VPath ** path, const VPath ** mapping, bool legacy_wgs_refseq )
{
    rc_t rc;
    KHttpRequest *req;

    assert(path);

    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("names.cgi = %S\n", self -> root));
    if(((self)->root)->addr[self->root->size - 1] == 'i')
        rc = KNSManagerMakeReliableClientRequest ( kns, & req, 0x01010000, NULL,
            self -> root -> addr ); 
    else if (((self)->root)->addr[4] == 's')
        rc = KNSManagerMakeReliableClientRequest ( kns, & req, 0x01010000, NULL,
            RESOLVER_CGI);
    else
        rc = KNSManagerMakeReliableClientRequest ( kns, & req, 0x01010000, NULL,
            RESOLVER_CGI_HTTP);

    if ( rc == 0 )
    {
        /* build up POST information: */
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("  version = %u.%u\n",
            NAME_SERVICE_MAJ_VERS, NAME_SERVICE_MIN_VERS));
        rc = KHttpRequestAddPostParam ( req, "version=%u.%u",
            NAME_SERVICE_MAJ_VERS, NAME_SERVICE_MIN_VERS );
        if ( rc == 0 )
        {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("  acc = %S\n", acc));
            rc = KHttpRequestAddPostParam ( req, "acc=%S", acc );
        }
        if ( rc == 0 && legacy_wgs_refseq )
        {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("  ctx = refseq\n"));
            rc = KHttpRequestAddPostParam ( req, "ctx=refseq" );
        }
        if ( rc == 0 && self -> ticket != NULL )
        {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("  tic = %S\n", self -> ticket));
            rc = KHttpRequestAddPostParam ( req, "tic=%S", self -> ticket );
        }

        if ( rc == 0 && NAME_SERVICE_VERS >= ONE_DOT_ONE )
        {
            uint32_t i;
            const char * prefs [ eProtocolMaxPref ];
            const char * seps [ eProtocolMaxPref ];
            VRemoteProtocols protos = protocols;

            prefs [ 0 ] = seps [ 0 ] = NULL;
            prefs [ 1 ] = prefs [ 2 ] = seps [ 1 ] = seps [ 2 ] = "";

            for ( i = 0; protos != 0 && i < sizeof prefs / sizeof prefs [ 0 ]; protos >>= 3 )
            {
                /* 1.1 protocols */
                switch ( protos & eProtocolMask )
                {
                case eProtocolHttp:
                    prefs [ i ] = "http";
                    seps [ i ++ ] = ",";
                    break;
                case eProtocolFasp:
                    prefs [ i ] = "fasp";
                    seps [ i ++ ] = ",";
                    break;
                default:
                    if ( NAME_SERVICE_VERS > ONE_DOT_ONE )
                    {
                        /* 1.2 protocols */
                        switch ( protos & eProtocolMask )
                        {
                        case eProtocolHttps:
                            prefs [ i ] = "https";
                            seps [ i ++ ] = ",";
                            break;
                        }
                    }
                }
            }

            if ( prefs [ 0 ] == NULL )
                rc = RC ( rcVFS, rcResolver, rcResolving, rcParam, rcInvalid );

            else
            {
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS ),
                         ("  accept-proto = %s%s%s%s%s\n", prefs [ 0 ], seps [ 1 ], prefs [ 1 ], seps [ 2 ], prefs [ 2 ] ) );
                rc = KHttpRequestAddPostParam ( req, "accept-proto=%s%s%s%s%s", prefs [ 0 ], seps [ 1 ], prefs [ 1 ], seps [ 2 ], prefs [ 2 ] );
            }
        }

        if ( rc == 0 )
        {
            KHttpResult *rslt;

            rc = KHttpRequestPOST ( req, &rslt ); /* will retry if needed `*/
            if ( rc == 0 )
            {
                uint32_t code;

                rc = KHttpResultStatus ( rslt, &code, NULL, 0, NULL );

#ifdef VDB_3162
                if ( TEST_VDB_3162 ) {
                    code = TESTING_VDB_3162_CODE ( rc, code );
                }
#endif

                DBGMSG
                    ( DBG_VFS, DBG_FLAG ( DBG_VFS ), ( " Code = %d\n", code ) );
                if ( code == 200 )
                {
                    KStream *response;

                    rc = KHttpResultGetInputStream ( rslt, &response );
                    if ( rc == 0 )
                    {
                        size_t num_read;
                        size_t total = 0;

                        KDataBuffer result;
                        memset ( & result, 0, sizeof result );
                        KDataBufferMakeBytes ( & result, 4096 );

                        while ( 1 )
                        {
                            uint8_t *base;
                            uint64_t avail = result . elem_count - total;
                            if ( avail < 256 )
                            {
                                rc = KDataBufferResize ( & result, result . elem_count + 4096 );
                                if ( rc != 0 )
                                    break;
                            }

                            base = result . base;
                            rc = KStreamRead ( response, & base [ total ], ( size_t ) result . elem_count - total, & num_read );
                            if ( rc != 0 )
                            {
                                /* TBD - look more closely at rc */
                                if ( num_read > 0 )
                                    rc = 0;
                                else
                                    break;
                            }

                            if ( num_read == 0 )
                                break;

                            total += num_read;
                        }

                        if ( rc == 0 )
                        {
                            result.elem_count = total;

                            rc = VResolverAlgParseResolverCGIResponse(&result, path, mapping, acc, self->ticket);
                            KDataBufferWhack ( &result );
                        }

                        KStreamRelease ( response );
                    }
                }
                else if ( code == 403 ) { // TODO CHECK AGAINS SERVICES
                    /* HTTP/1.1 403 Forbidden
                     - resolver CGI was called over http insted of https */
                    rc = RC ( rcVFS, rcResolver, rcResolving,
                        rcConnection, rcUnauthorized );
                }
                else if ( code == 404 )
                {
                    /* HTTP/1.1 404 Bad Request - resolver CGI was not found */
                    rc = RC ( rcVFS, rcResolver, rcResolving, rcConnection, rcNotFound );
                }
                else
                {
                    /* Something completely unexpected */
                    rc = RC ( rcVFS, rcResolver, rcResolving, rcConnection, rcUnexpected );
                }
                KHttpResultRelease ( rslt );
            }
        }
        KHttpRequestRelease ( req );
    }

    assert(*path != NULL || rc != 0);

    if (rc == 0 && *path == NULL)
    {
        rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcNull);
    }

    return rc;
}
#endif

rc_t VResolverAlgRemoteProtectedResolve( const VResolverAlg *self,
    const KNSManager *kns, VRemoteProtocols protocols, const String *acc,
    const VPath ** path, const VPath ** mapping, bool legacy_wgs_refseq,
    const char * version )
{
    rc_t rc = 0;
    const char * ticket = NULL;

    char vers[512] = "";

    assert ( self && self -> root && acc );
    if ( self -> ticket != NULL ) {
        ticket = self -> ticket -> addr;
    }

    if (self->version != 0) { /* Use alg's version after we start using SDL.. */
        bool version1 = false;
        if (version != NULL) {
            if (version[0] == '1' && version[1] == '.')
                version1 = true;
            else if (version[0] == '#' &&
                version[1] == '1' && version[2] == '.')
            {
                version1 = true;
            }
        }
        /* ... but use provided version when asked for 1.x
           (used to query dbGaP OIDs) */
        if (!version1) {
            rc = string_printf(vers, sizeof vers, NULL, "%V", self->version);
            if (rc == 0)
                version = vers;
        }
    }

    if (rc == 0) {
        rc = KService1NameWithVersion(kns, self->root->addr,
            acc->addr, acc->len, ticket, protocols, path, mapping,
            legacy_wgs_refseq, version, self -> protected);

        assert(*path != NULL || rc != 0);

        if (rc == 0) {
            if (*path == NULL)
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcNull);
            else
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), (
                    "VResolverAlgRemoteProtectedResolve: "
                    "'%S' resolved to '%s'\n", acc, (*path)->scheme.addr));
        }
    }

    return rc;
}

/* If resolver-cgi is on government site and is called over http:
   fix it to https */
static
rc_t VResolverAlgFixHTTPSOnlyStandard ( VResolverAlg * self, bool * fixed )
{
    rc_t rc = 0;

    const String * root = NULL;

    assert ( self && fixed );

    * fixed = false;

    root = self -> root;

    if ( root != NULL ) {
        size_t size = 0;
        String http;
        CONST_STRING ( & http, "http://" );
        size = http . size;

        /* resolver-cgi is called over http */
        if ( root -> size > size && strcase_cmp ( root -> addr, size,
                                   http . addr, size, ( uint32_t ) size ) == 0 )
        {
            VPath * path = NULL;
            rc = VPathMakeFmt ( & path, "%S", root );
            if ( rc == 0 ) {
                String host;
                rc = VPathGetHost ( path, & host );
                if ( rc == 0 ) {
                    String gov;
                    CONST_STRING ( & gov, ".gov" );
                    size = gov . size;

                    /* If resolver-cgi is on government site */
                    if ( host . size > size &&
                        strcase_cmp ( host . addr + host . size - size,
                            size, gov . addr, size, ( uint32_t ) size ) == 0 )
                    {
                        size_t newLen = root -> len + 2;
                        String * tmp = malloc ( sizeof * tmp + newLen );
                        if ( tmp == NULL ) {
                            rc = RC ( rcVFS, rcResolver,
                                rcResolving, rcMemory, rcExhausted );
                        }
                        else
                        {
                            /* capture all of root past "http" */
                            String remainder;
                            StringSubstr ( root, & remainder, 4, 0 );

                            /* prepare "tmp" to point to buffer space */
                            tmp -> addr = ( char * ) ( tmp + 1 );

                            /* print into buffer */
                            rc = string_printf ( ( char * ) tmp -> addr,
                                newLen, & tmp -> size, "https%S", & remainder );
                            if ( rc != 0 )
                                free ( tmp );
                            else
                            {
                                tmp -> len = root -> len + 1;
                                rc = VectorAppend ( & self -> vols, NULL, tmp );
                                if ( rc == 0 )
                                {
                                    self -> root = tmp;
                                    * fixed = true;
                                }
                            }
                        }
                    }
                }
            }
            VPathRelease ( path );
        }
    }

    return rc;
}

/* RemoteResolve
 *  resolve an accession into a VPath or not found
 *
 *  1. expand accession according to algorithm
 *  2. search all volumes for accession
 *  3. return not found or new VPath
 */
static
rc_t VResolverAlgRemoteResolve ( const VResolverAlg *self,
    const KNSManager *kns, VRemoteProtocols protocols, const VResolverAccToken *tok,
    const VPath ** path, const VPath ** mapping, const KFile ** opt_file_rtn, bool legacy_wgs_refseq,
    const char * version )
{
    rc_t rc = 0;
    uint32_t i, count;

    /* expanded accession */
    String exp;
    size_t size;
    char expanded [ 256 ];

    const String *root;

    assert(self);

    /* check for download ticket */
    if ( self -> alg_id == algCGI
#if NO_LEGACY_WGS_REFSEQ_CGI
         && ! legacy_wgs_refseq
#endif
        )
    {
        bool done = false;
        int i = 0;
        for ( i = 0; i < 2 && ! done; ++i ) {
            if ( version == NULL )
#ifdef TESTING_SERVICES_VS_OLD_RESOLVING
              rc = oldVResolverAlgRemoteProtectedResolve ( self, kns,
                protocols, & tok -> acc, path, mapping, legacy_wgs_refseq );
#else
              assert(0);
#endif
            else
              rc = VResolverAlgRemoteProtectedResolve ( self, kns,
                protocols, & tok -> acc, path, mapping, legacy_wgs_refseq, version );
            if ( rc == SILENT_RC (
                rcVFS, rcResolver, rcResolving, rcConnection, rcUnauthorized )
                ||  rc == SILENT_RC (
                rcVFS, rcQuery, rcExecuting, rcConnection, rcUnauthorized )
               )
            { /* resolver-cgi is called over http instead of https:
                 fix it */
                bool fixed = false;
                rc_t r2 = VResolverAlgFixHTTPSOnlyStandard
                    ( ( VResolverAlg * ) self, & fixed );
                if ( ! fixed || r2 != 0 ) {
                    if ( r2 != 0 ) {
                        rc = r2;
                    }
                    done = true;
                }
            }
            else {
                done = true;
            }
        }

        if (rc == 0 && path != NULL && *path != NULL &&
            opt_file_rtn != NULL && *opt_file_rtn == NULL &&
            VPathGetUri_t ( * path ) != vpuri_fasp )
        {
            const String *s = NULL;
            rc_t rc = VPathMakeString(*path, &s);
            if (rc != 0)
            {
                LOGERR(klogInt, rc,
                    "failed to make string from remote protected path");
            }
            else
            {
                rc = KNSManagerMakeReliableHttpFile ( kns, opt_file_rtn, NULL, 0x01010000, true, false, false, "%S", s );
                if (rc != 0)
                {
                    PLOGERR(klogInt, (klogInt, rc,
                        "failed to open file for $(path)", "path=%s", s->addr));
                }
                StringWhack ( s );
            }
        }
        return rc;
    }

    /* now we know just how to resolve HTTP locations */
    if ( protocols == eProtocolFasp ) {
        return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
    }

    /* for remote, root can never be NULL */
    root = self -> root;

    /* expand the accession */
    rc = expand_algorithm ( self, tok, expanded, sizeof expanded, & size, legacy_wgs_refseq );

    /* should never have a problem here... */
    if ( rc != 0 )
        return rc;

    /* turn the expanded portion into a String
       we know that size is also length due to
       accession content rules */
    StringInit ( & exp, expanded, size, ( uint32_t ) size );

    /* now search all remote volumes */
    count = VectorLength ( & self -> vols );
    for ( i = 0; i < count; ++ i )
    {
        char url [ 8192 ];
        const String *vol = VectorGet ( & self -> vols, i );
        rc = string_printf ( url, sizeof url, NULL, "%S/%S/%S", root, vol, & exp );
        if ( rc == 0 )
        {
            const KFile *f;
            rc = KNSManagerMakeHttpFile ( kns, & f, NULL, 0x01010000, url );
            if ( rc == 0 )
            {
                if ( opt_file_rtn != NULL )
                    * opt_file_rtn = f;
                else
                    KFileRelease ( f );

                if ( legacy_wgs_refseq )
                    return VResolverAlgMakeRemoteWGSRefseqURI ( self, url, & tok -> acc, path );
                return VResolverAlgMakeRemotePath ( self, url, path );
            }
        }
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

/* CacheResolve
 *  resolve accession to the current directory
 */
static rc_t VResolverAlgCacheResolveDir ( const VResolverAlg *self,
    const KDirectory *wd, const VResolverAccToken *tok, const VPath ** path,
    bool legacy_wgs_refseq, const char * dir, bool * resolvedToDir )
{
    /* expanded accession */
    String exp;
    size_t size = 0;
    char expanded [ 256 ] = "";
    char resolved [ PATH_MAX ] = "";

    /* expand the accession */
    rc_t rc = expand_algorithm ( self, tok, expanded, sizeof expanded,
                                 & size, legacy_wgs_refseq );

    /* should never have a problem here... */
    if ( rc != 0 )
        return rc;

    /* turn the expanded portion into a String
       we know that size is also length due to
       accession content rules */
    StringInit ( & exp, expanded, size, ( uint32_t ) size );

    if ( dir == NULL )
        dir = ".";

    rc = KDirectoryResolvePath ( wd, true, resolved, sizeof resolved,
                                 "%s/%s", dir, expanded );

    if ( rc == 0 )
        rc = VPathMakeFmt ( ( VPath ** ) path, "%s", resolved );

    if ( rc == 0 && resolvedToDir )
        * resolvedToDir = true;

    return rc;
}

/* CacheResolve
 *  try to resolve accession for currently cached file
 */
static
rc_t VResolverAlgCacheResolve ( const VResolverAlg *self,
    const KDirectory *wd, const VResolverAccToken *tok,
    const VPath ** path, bool legacy_wgs_refseq )
{
    /* see if the cache file already exists */
    const bool for_cache = true;
    rc_t rc = VResolverAlgLocalResolve ( self, wd, tok, path, legacy_wgs_refseq, for_cache, NULL, false );
    if ( rc == 0 )
        return 0;

    /* TBD - see if any of these volumes is a good candidate for creating a file */

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}


/* CacheFile
 *  try to resolve name for currently cached file
 */
static
rc_t VResolverAlgCacheFile ( const VResolverAlg *self,
    const KDirectory *wd, const VPath *query, const VPath ** path )
{
    /* see if the cache file already exists */
    const bool for_cache = true;
    rc_t rc = VResolverAlgLocalFile ( self, wd, query, path, for_cache );
    if ( rc == 0 )
        return 0;

    /* TBD - see if any of these volumes is a good candidate for creating a file */

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}



/* MakeCachePath
 *  we have an accession that matches this volume
 *  create a path for it
 */
static
rc_t VResolverAlgMakeCachePath ( const VResolverAlg *self,
    const VResolverAccToken *tok, const VPath ** path, bool legacy_wgs_refseq,
    const KDirectory * wd )
{
    uint32_t i, count;

    /* expanded accession */
    String exp;
    size_t size;
    char expanded [ 256 ];

    const String *vol;

    /* expand the accession */
    rc_t rc = expand_algorithm ( self, tok, expanded, sizeof expanded, & size, legacy_wgs_refseq );

    /* should never have a problem here... */
    if ( rc != 0 )
        return rc;

    /* turn the expanded portion into a String
       we know that size is also length due to
       accession content rules */
    StringInit ( & exp, expanded, size, ( uint32_t ) size );

    /* now search all volumes */
    count = VectorLength ( & self -> vols );
    for ( i = 0; i < count; ++ i )
    {
        vol = VectorGet ( & self -> vols, i );
        return VResolverAlgMakeLocalPath ( self, vol, & exp, path, wd );
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
}


/* MakeCacheFilePath
 *  we have a filename that matches this volume
 *  create a path for it
 */
static
rc_t VResolverAlgMakeCacheFilePath ( const VResolverAlg *self,
    const VPath *query, const VPath ** path )
{
    String fname = query -> path;

    /* needs proper extension for krypto */
    const char * krypto_ext = self -> protected ? ".ncbi_enc" : "";

    /* now search all volumes */
    uint32_t i, count = VectorLength ( & self -> vols );
    for ( i = 0; i < count; ++ i )
    {
        const String *vol = VectorGet ( & self -> vols, i );
        return VResolverAlgMakeLocalFilePath ( self, vol, & fname, krypto_ext, path );
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
}



/*--------------------------------------------------------------------------
 * VResolver
 */
struct VResolver
{
    /* root paths - stored as String* */
    Vector roots;

    /* volume algorithms - stored as VResolverAlg* */
    Vector local;
    Vector remote;
    Vector ad; /* vector having 1 dummy alg to implement AccessionAsDirectory */

    /* working directory for testing local paths */
    const KDirectory *wd;

    /* if there is a protected remote resolver,
       we will need a KNS manager */
    const KNSManager *kns;

    /* if there is a working protected repository,
       store the download ticket here */
    const String *ticket;

    KRefcount refcount;

    /* counters for various app volumes */
    uint32_t num_app_vols [ appCount ];

    /* preferred protocols preferences. Default: HTTP */
    VRemoteProtocols protocols, dflt_protocols;

    /** projectId of protected user repository;
        0 when repository is not user protected */
    uint32_t projectId;

    char *version;
    VERSNS versions;
    bool resoveOidName;
};


/* "process" global settings
 *  actually, they are library-closure global
 */
static atomic32_t enable_local, enable_remote, enable_cache;


/* Whack
 */
static
rc_t VResolverWhack ( VResolver *self )
{
    assert ( self );

    free ( self -> version );

    KRefcountWhack ( & self -> refcount, "VResolver" );

    /* drop all remote volume sets */
    VectorWhack ( & self -> remote, VResolverAlgWhack, NULL );

    /* drop local volume sets */
    VectorWhack ( & self -> local, VResolverAlgWhack, NULL );

    VectorWhack ( & self -> ad, VResolverAlgWhack, NULL );

    /* drop download ticket */
    if ( self -> ticket != NULL )
        StringWhack ( ( String* ) self -> ticket );

    /* drop root paths */
    VectorWhack ( & self -> roots, string_whack, NULL );

    /* release kns */
    if ( self -> kns != NULL )
        KNSManagerRelease ( self -> kns );

    /* release directory onto local file system */
    KDirectoryRelease ( self -> wd );

    memset ( self, 0, sizeof * self );
    free ( self );
    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT
rc_t CC VResolverAddRef ( const VResolver * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VResolver" ) )
        {
        case krefOkay:
            break;
        case krefZero:
            return RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcIncorrect );
        case krefLimit:
            return RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcExhausted );
        case krefNegative:
            return RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcInvalid );
        default:
            return RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcUnknown );
        }
    }
    return 0;
}

LIB_EXPORT
rc_t CC VResolverRelease ( const VResolver * self )
{
    rc_t rc = 0;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VResolver" ) )
        {
        case krefOkay:
        case krefZero:
            break;
        case krefWhack:
            VResolverWhack ( ( VResolver* ) self );
            break;
        case krefNegative:
            return RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcInvalid );
        default:
            rc = RC ( rcVFS, rcResolver, rcAttaching, rcRefcount, rcUnknown );
            break;
        }
    }
    return rc;
}


/* get_accession_code
 */
static
uint32_t get_accession_code ( const String * accession, VResolverAccToken *tok )
{
#if USE_VPATH_OPTIONS_STRINGS
#error this thing is wrong
#else

#define MAX_ACCESSION_LEN 20

    uint32_t code;

    const char *acc = NULL;
    size_t i, size = 0;

    assert(accession);

    acc = accession -> addr;
    size = accession -> size;


    tok -> vdbcache = false;

    /* capture the whole accession */
    tok -> acc = * accession;

    /* scan prefix or alpha */
    for ( i = 0; i < size; ++ i )
    {
        if ( ! isalpha ( acc [ i ] ) )
            break;
    }

    /* terrible situation - unrecognizable */
    if ( i == size || i == 0 || i >= MAX_ACCESSION_LEN )
    {
        StringInit ( & tok -> prefix, acc, 0, 0 );
        StringInit ( & tok -> alpha, acc, i, (uint32_t)i );
        StringInit ( & tok -> digits, & acc [ i ], 0, 0 );
        tok -> ext1 = tok -> ext2 = tok -> suffix = tok -> digits;
        return 0;
    }

    /* if stopped on '_', we have a prefix */
    if ( acc [ i ] == '_' )
    {
        /* prefix
           store only its presence, not length */
        code = 1 << 4 * 4;
        StringInit ( & tok -> prefix, acc, i, (uint32_t)i );

        /* remove prefix */
        acc += ++ i;
        size -= i;

        /* scan for alpha */
        for ( i = 0; i < size; ++ i )
        {
            if ( ! isalpha ( acc [ i ] ) )
                break;
        }

        if ( i == size || i >= MAX_ACCESSION_LEN )
        {
            StringInit ( & tok -> alpha, acc, i, (uint32_t)i );
            StringInit ( & tok -> digits, & acc [ i ], 0, 0 );
            tok -> ext1 = tok -> ext2 = tok -> suffix = tok -> digits;
            return 0;
        }

        code |= i << 4 * 3;
        StringInit ( & tok -> alpha, acc, i, (uint32_t)i );
    }
    else if ( ! isdigit ( acc [ i ] ) )
    {
        StringInit ( & tok -> prefix, acc, 0, 0 );
        StringInit ( & tok -> alpha, acc, i, (uint32_t)i );
        StringInit ( & tok -> digits, & acc [ i ], 0, 0 );
        tok -> ext1 = tok -> ext2 = tok -> suffix = tok -> digits;
        return 0;
    }
    else
    {
        /* alpha */
        code = (uint32_t) ( i << 4 * 3 );
        StringInit ( & tok -> prefix, acc, 0, 0 );
        StringInit ( & tok -> alpha, acc, i, (uint32_t)i );
    }

    /* remove alpha */
    acc += i;
    size -= i;

    /* scan digits */
    for ( i = 0; i < size; ++ i )
    {
        if ( ! isdigit ( acc [ i ] ) )
            break;
    }

    /* record digits */
    StringInit ( & tok -> digits, acc, i, (uint32_t)i );
    StringInit ( & tok -> ext1, & acc [ i ], 0, 0 );
    tok -> ext2 = tok -> suffix = tok -> ext1;

    if ( i == 0 || i >= MAX_ACCESSION_LEN )
        return 0;

    code |= i << 4 * 2;

    /* normal return point for SRA */
    if ( i == size )
        return code;

    /* check for extension */
    if ( acc [ i ] != '.' )
        return 0;

    /* remove digit */
    acc += ++ i;
    size -= i;

    /* check pileup extension */
    if ( string_cmp( acc, size, "pileup", 6, size + 6 ) == 0 )
    {
        i = 6;
    }
    /* check realign extension */
    else if (string_cmp(acc, size, "realign", 7, size + 7) == 0)
    {
        i = 7;
    }
    /* check vdbcache extension */
    else if (string_cmp(acc, size, "vdbcache", 8, size + 8) == 0)
    {   /* vdbcache uses the same code as its accession */
        tok -> vdbcache = true;
        return code;
    }
    else if (string_cmp(acc, size, "sra.vdbcache", 12, size + 8) == 0)
    {   /* vdbcache uses the same code as its accession */
        tok->vdbcache = true;
        return code;
    }
    else
    {
        /* scan numeric extension */
        for ( i = 0; i < size; ++ i )
        {
            if ( ! isdigit ( acc [ i ] ) )
                break;
        }
    }

    if ( i == 0 || i >= MAX_ACCESSION_LEN )
        return 0;

    /* record the actual extension */
    StringInit ( & tok -> ext1, acc, i, (uint32_t)i );
    /* codify the extension simply as present, not by its length */
    code |= 1 << 4 * 1;

    if ( i == size )
        return code;

    /* scan for suffix */
    if ( acc [ i ] == '_' )
    {
        acc += ++ i;
        size -= i;
        for ( i = 0; i < size; ++ i )
        {
            if ( ! isalpha ( acc [ i ] ) )
                break;
        }

        /* this has to end the whole thing */
        if ( i == 0 || i != size )
            return 0;

        StringInit ( & tok -> suffix, acc, i, (uint32_t)i );
        /* NB - not incorporating suffix into code right now */
        return code;
    }

    if ( acc [ i ] != '.' )
        return 0;


    /* remove first extension */
    acc += ++ i;
    size -= i;

    /* scan 2nd numeric extension */
    for ( i = 0; i < size; ++ i )
    {
        if ( ! isdigit ( acc [ i ] ) )
            break;
    }

    if ( i == 0 || i >= MAX_ACCESSION_LEN )
        return 0;

    StringInit ( & tok -> ext2, acc, i, (uint32_t)i );
    code |= 1 << 4 * 0;

    if ( i == size )
        return code;

    /* no other patterns are accepted */
    return 0;
#endif
}


/* get_accession_app
 */
VResolverAppID get_accession_app ( const String * accession, bool refseq_ctx,
    VResolverAccToken *tok, bool *legacy_wgs_refseq,
    bool resolveAllAccToCache, bool * forDirAdjusted, const String * parentAcc,
    int64_t projectId )
{
    VResolverAppID app;
    uint32_t code = 0;

    VResolverAccToken tummy;

    bool dummy;
    if ( forDirAdjusted == NULL)
        forDirAdjusted = & dummy;
    *forDirAdjusted = false;

    if (tok == NULL)
        tok = &tummy;

    memset(tok, 0, sizeof *tok);
    tok->projectId = projectId;

    code = get_accession_code(accession, tok);

    if (accession != NULL &&
        accession->addr != NULL && isdigit(accession->addr[0]))
    {
        if ( ! resolveAllAccToCache ) {
            * forDirAdjusted = true;
            return appFILE;
        }
        else /* TODO: KART */
            return appAny;
    }

    /* disregard extensions at this point */
    switch ( code >> 4 * 2 )
    {
    case 0x015: /* e.g. "J01415" or "J01415.2"     */
    case 0x026: /* e.g. "CM000071" or "CM000039.2" */
    case 0x126: /* e.g. "NZ_DS995509.1"            */
        app = appREFSEQ;
        break;

    case 0x036: /* e.g. "SRR012345"    */
    case 0x037: /* e.g. "SRR0123456"   */
    case 0x038: /* e.g. "SRR01234567"  */
    case 0x039: /* e.g. "SRR012345678" */

        /* detect accession with extension */
        if ( ( code & 0xFF ) != 0 )
        {
            /* check pileup suffix, e.g. "SRR012345.pileup" */
            String suffix = tok -> ext1;
            if ( suffix . size == 6 &&
                 suffix . addr [ 0 ] == 'p' &&
                 suffix . addr [ 1 ] == 'i' &&
                 suffix . addr [ 2 ] == 'l' &&
                 suffix . addr [ 3 ] == 'e' &&
                 suffix . addr [ 4 ] == 'u' &&
                 suffix . addr [ 5 ] == 'p' )
            {
                app = appSRAPileup;
            }
            /* check realign suffix, e.g. "SRR012345.realign" */
            else if (suffix.size == 7 &&
                suffix.addr[0] == 'r' &&
                suffix.addr[1] == 'e' &&
                suffix.addr[2] == 'a' &&
                suffix.addr[3] == 'l' &&
                suffix.addr[4] == 'i' &&
                suffix.addr[5] == 'g' &&
                suffix.addr[6] == 'n')
            {
                app = appSRARealign;
            }
            else
            {
                app = appAny;
            }
        }
        else
        {
            app = appSRA;
        }
        break;

    case 0x106: /* e.g. "NC_000012.10"                      */
    case 0x109: /* e.g. "NW_003315935.1", "GPC_000000393.1" */
        if ( tok -> prefix . size == 3 &&
             tok -> prefix . addr [ 0 ] == 'G' &&
             tok -> prefix . addr [ 1 ] == 'C' &&
             ( tok -> prefix . addr [ 2 ] == 'A' || tok -> prefix . addr [ 2 ] == 'F' ) )
        {
            /* e.g. "GCA_000392025.1_L" */
            app = appNAKMER;
            break;
        }

        app = appREFSEQ;
        break;

    case 0x042: /* e.g. "AAAB01" is WGS package name    */
    case 0x048: /* e.g. "AAAA01000001"                  */
    case 0x049: /* contig can be between 6 and 8 digits */
    case 0x04A:

    case 0x062: /* e.g. "ABCDEF01" is WGS package name  */
    case 0x068: /* e.g. "ABCDEF01000001"                */
    case 0x069: /* contig can be between 6 and 8 digits */
    case 0x06A:

    case 0x142: /* e.g. "NZ_AAEW01"                     */
    case 0x148: /* e.g. "NZ_AAEW01000001"               */
    case 0x149:
    case 0x14A:

    case 0x162:
    case 0x168:
    case 0x169:
    case 0x16A:
        app = appWGS;
        break;

    case 0x029: /* e.g. NA000008777.1 */
        if ( code == 0x02910 )
        {
            if ( tok -> alpha . addr [ 0 ] == 'N' && tok -> alpha . addr [ 1 ] == 'A' )
            {
                app = appNANNOT;
                break;
            }
        }

        /* no break */

    default:
    {
        String hs37d5;
        CONST_STRING(&hs37d5, "hs37d5");
        if (accession != NULL && StringEqual(accession, &hs37d5))
            app = appREFSEQ;
        /* TBD - people appear to be able to throw anything into refseq,
           so anything unrecognized we may as well test out there...
           but this should not stay the case */
        else if ( ! resolveAllAccToCache ) {
            * forDirAdjusted = true;
            app = appFILE;
        }
        else
            app = appREFSEQ;
    }
    }

    if ( app == appWGS )
    {
        /* if we know this is for refseq, clobber it here */
        if ( refseq_ctx )
        {
            app = appREFSEQ;
            * legacy_wgs_refseq = true;
        }
    }

    if (app == appREFSEQ && parentAcc != NULL)
        StringInit(
            &tok->accOfParentDb, parentAcc->addr, parentAcc->size, parentAcc->len);

    return app;
}

/* Temporary patch for FUSE mounted accessions
 *  1. determined if accession is mounter locally
 *  2. return not found or new VPath
 */
static
rc_t VResolverFuseMountedResolve ( const VResolver * self,
    const String * accession, const VPath ** path )
{
    rc_t rc;
    struct KDirectory * NativeDir;
    uint32_t PathType;

    rc = 0;
    NativeDir = NULL;
    PathType = kptNotFound;

    rc = KDirectoryNativeDir ( & NativeDir );
    if ( rc == 0 ) {
        PathType = KDirectoryPathType ( NativeDir, ".#dbgap-mount-tool#" );
        if ( PathType == kptFile ) {
            PathType = KDirectoryPathType (
                                        NativeDir,
                                        "%.*s",
                                        ( int ) accession -> size,
                                        accession -> addr
                                        );
            if ( PathType == kptFile ) {
                rc = VPathMakeFmt (
                                ( VPath ** ) path,
                                "%.*s",
                                ( int ) accession -> size,
                                accession -> addr
                                );
            }
            else {
                rc = RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
            }
        }
        else {
            rc = SILENT_RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
        }

        KDirectoryRelease ( NativeDir );
    }

    return rc;
}

typedef enum {
    eCheckExistFalse,
    eCheckExistTrue,
} ECheckExist;

typedef enum {
    eCheckFilePathFalse,
    eCheckFilePathTrue,
} ECheckFilePath;

typedef enum {
    eCheckUrlFalse,
    eCheckUrlTrue,
} ECheckUrl;

static rc_t VResolverMagicResolve(const VResolver * self, const VPath ** path,
    const String * accession, VResolverAppID app, const char * name,
    ECheckExist checkExist,
    ECheckFilePath checkPath,
    ECheckUrl checkUrl)
{
    rc_t rc = 0;

    KPathType kpt = kptNotFound;
    const char * magic;

    /* resolver is not confused by shell variables
        when retrieving reference objects */
    if (app == appREFSEQ) {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH),
            ("'%s' magic ignored for refseq\n", name));
        return 0;
    }

    magic = getenv(name);
    if (magic == NULL) {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), ("'%s' magic not set\n", name));
        return 0;
    }

    /* variable set to empty: VResilverQuery returns not found */
    if (magic[0] == '\0') {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), ("'%s' magic empty\n", name));
        return RC(rcVFS, rcResolver, rcResolving, rcName, rcNotFound);
    }

    if (checkExist == eCheckExistTrue) {
        assert(self->wd);
        kpt = KDirectoryPathType(self->wd, magic) & ~kptAlias;
        if (kpt != kptFile && kpt != kptDir) {
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                "'%s' magic '%s' not found\n", name, magic));
            return RC(rcVFS, rcResolver, rcResolving, rcName, rcNotFound);
        }
    }

    rc = LegacyVPathMakeFmt((VPath**)path, "%s", magic);

    if (rc == 0)
        assert(*path);

    if (rc == 0)
        VPathSetMagic((VPath*)*path, true);

    if (rc == 0 && accession != NULL)
        rc = VPathSetId((VPath*)*path, accession);

    if (rc == 0) {
        assert(path);

        assert(checkPath == eCheckFilePathTrue || checkUrl == eCheckUrlTrue);

        if (checkPath == eCheckFilePathTrue) {
            if ((*path)->from_uri)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is URL\n", name, magic));
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcInvalid);
            }
            else if ((*path)->path_type != vpFullPath
                  && (*path)->path_type != vpRelPath
                  && (*path)->path_type != vpUNCPath)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is not path\n", name, magic));
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcInvalid);
            }
        }

        if (checkUrl == eCheckUrlTrue) {
            if (!(*path)->from_uri)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is not URL\n", name, magic));
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcInvalid);
            }
            else if ((*path)->scheme_type != vpuri_http
                  && (*path)->scheme_type != vpuri_https)
            {
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                    "'%s' magic '%s' is not HTTP[S] URL\n", name, magic));
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcInvalid);
            }
        }

        if (rc != 0) {
            VPathRelease(*path);
            *path = NULL;
            return rc;
        }
        else
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
                "'%s' magic '%s' found\n", name, magic));
    }

    else
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH), (
            "'%s' magic '%s' cannot be converted to VPath: %R\n",
            name, magic, rc));

    return rc;
}

static rc_t VResolverCacheMagicResolve(
    const VResolver * self, const VPath ** path, VResolverAppID app)
{
    return VResolverMagicResolve(self, path, NULL, app,
        ENV_MAGIC_CACHE, eCheckExistFalse, eCheckFilePathTrue, eCheckUrlFalse);
}

static rc_t VResolverLocalMagicResolve(
    const VResolver * self, const VPath ** path, VResolverAppID app)
{
    return VResolverMagicResolve(self, path, NULL, app,
        ENV_MAGIC_LOCAL, eCheckExistTrue, eCheckFilePathTrue, eCheckUrlFalse);
}

static rc_t VResolverRemoteMagicResolve(const VResolver * self,
    const VPath ** path, const String * accession, VResolverAppID app)
{
    return VResolverMagicResolve(self, path, accession, app,
        ENV_MAGIC_REMOTE, eCheckExistFalse, eCheckFilePathFalse, eCheckUrlTrue);
}

/* LocalResolve
 *  resolve an accession into a VPath or not found
 *
 *  1. determine the type of accession we have, i.e. its "app"
 *  2. search all local algorithms of app type for accession
 *  3. return not found or new VPath
 */
static
rc_t VResolverLocalResolve ( const VResolver *self, const String * accession,
    const VPath ** path, bool refseq_ctx, bool resolveAllAccToCache,
    const char * dir, const String * parentAcc, int64_t projectId )
{
    rc_t rc = 0;

    uint32_t i, count;

    VResolverAccToken tok;
    bool legacy_wgs_refseq = false;

    VResolverAppID app;

/*
    bool resolveAllAccToCache = true;
    if ( dir != NULL )
        resolveAllAccToCache = false;
*/

    assert(path);

    if ( VResolverFuseMountedResolve ( self, accession, path ) == 0 ) {
        return 0;
    }

    app = get_accession_app ( accession, refseq_ctx, & tok,
        & legacy_wgs_refseq, resolveAllAccToCache, NULL, parentAcc, projectId );

    rc = VResolverLocalMagicResolve(self, path, app);
    if (rc != 0 || *path != NULL)
        return rc;

    /* check AD */
    count = VectorLength ( & self -> ad );
    for ( i = 0; i < count; ++ i )
    {
        const VResolverAlg *alg = VectorGet ( & self ->ad, i );
        if ( alg -> app_id == app )
        {
            const bool for_cache = false;
            rc_t rc = VResolverAlgLocalResolve ( alg, self -> wd,
                & tok, path, legacy_wgs_refseq, for_cache, dir, true );
            if ( rc == 0 )
                return 0;
        }
    }

    /* search all local volumes by app and accession algorithm expansion */

    count = VectorLength ( & self -> local );
    for ( i = 0; i < count; ++ i )
    {
        const VResolverAlg *alg = VectorGet ( & self -> local, i );
        if ( alg -> app_id == app )
        {
            const bool for_cache = false;
            rc_t rc = VResolverAlgLocalResolve ( alg, self -> wd,
                & tok, path, legacy_wgs_refseq, for_cache, dir, false );
            if ( rc == 0 )
                return 0;
        }
    }

    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), (
        "VResolverLocalResolve: local location of '%S' not found\n",
        accession));
    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

/* LocalFile
 *  locate a locally stored file
 */
static
rc_t VResolverLocalFile ( const VResolver *self, const VPath * query, const VPath ** path )
{
    uint32_t i, count;

    count = VectorLength ( & self -> local );
    for ( i = 0; i < count; ++ i )
    {
        const VResolverAlg *alg = VectorGet ( & self -> local, i );
        if ( alg -> app_id == appFILE )
        {
            const bool for_cache = false;
            rc_t rc = VResolverAlgLocalFile ( alg, self -> wd, query, path, for_cache );
            if ( rc == 0 )
                return 0;
        }
    }

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}

LIB_EXPORT
bool CC VPathHasRefseqContext ( const VPath * accession )
{
    size_t num_read;
    char option [ 64 ];
    rc_t rc = VPathOption ( accession, vpopt_vdb_ctx, option, sizeof option, & num_read );
    if ( rc != 0 )
        return false;

    return ( num_read == sizeof "refseq" - 1 &&
             strcase_cmp ( "refseq", sizeof "refseq" - 1,
                           option, num_read, (uint32_t)num_read ) == 0 );
}


/* Local - DEPRECATED
 *  Find an existing local file/directory that is named by the accession.
 *  rcState of rcNotFound means it does not exist.
 *
 *  other rc code for failure are possible.
 *
 *  Accession must be an ncbi-acc scheme or a simple name with no
 *  directory paths.
 */
LIB_EXPORT
rc_t CC VResolverLocal ( const VResolver * self,
    const VPath * accession, const VPath ** path )
{
    rc_t rc =  VResolverQuery ( self, self -> protocols, accession, path, NULL, NULL );
    if ( rc == 0 )
    {
        switch ( accession -> path_type )
        {
        case vpOID:
        case vpAccession:
        case vpNameOrOID:
        case vpNameOrAccession:
            if ( * path != accession )
                return 0;
            break;

        case vpName:
            if ( accession -> query . size != 0 && VPathHasRefseqContext ( accession ) )
                return 0;
            break;
        }

        VPathRelease ( * path );
        * path = NULL;
        rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
    }
    return rc;
}


/* LocalEnable
 *  modify settings for using local repositories,
 *  meaning site, user-public and user-protected.
 *
 *  "enable" [ IN ] - enable or disable local access,
 *  or follow settings in KConfig
 *
 *  returns the previous state of "local-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
LIB_EXPORT
VResolverEnableState CC VResolverLocalEnable ( const VResolver * self, VResolverEnableState enable )
{
    int32_t val, cur, prior;

    if ( self == NULL )
        return vrUseConfig;

    /* convert "VResolverEnableState" to 32-bit signed integer for atomic operation */
    val = ( int32_t ) enable;

    /* before performing atomic swap, get the current setting,
       and return right away if it is already set correctly */
    prior = atomic32_read ( & enable_local );
    if ( prior != val ) do
    {
        cur = prior;
        prior = atomic32_test_and_set ( & enable_local, val, prior );
    }
    while ( prior != cur );

    return prior;
}


/* RemoteEnable
 *  apply or remove a process-wide enabling of remote access
 *  regardless of configuration settings
 *
 *  "enable" [ IN ] - if "true", enable all remote access
 *  if false, use settings from configuration.
 *
 *  returns the previous state of "remote-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
LIB_EXPORT
VResolverEnableState CC VResolverRemoteEnable ( const VResolver * self, VResolverEnableState enable )
{
    int32_t val, cur, prior;

    if ( self == NULL )
        return vrUseConfig;

    /* convert "VResolverEnableState" to 32-bit signed integer for atomic operation */
    val = ( int32_t ) enable;

    /* before performing atomic swap, get the current setting,
       and return right away if it is already set correctly */
    prior = atomic32_read ( & enable_remote );
    if ( prior != val ) do
    {
        cur = prior;
        prior = atomic32_test_and_set ( & enable_remote, val, prior );
    }
    while ( prior != cur );

    return prior;
}


/* CacheEnable
 *  modify settings for caching files in user repositories
 *
 *  "enable" [ IN ] - enable or disable user repository cache,
 *  or follow settings in KConfig
 *
 *  returns the previous state of "cache-enabled" property
 *
 * NB - in VDB-2, the state is associated with library code
 *  shared libraries in separate closures will have separate
 *  state. this can only occur if dynamic ( manual ) loading of
 *  shared libraries is used, and will not occur with normal
 *  usage. in VDB-3 the state will be stored with the process,
 *  not the library.
 */
LIB_EXPORT
VResolverEnableState CC VResolverCacheEnable ( const VResolver * self, VResolverEnableState enable )
{
    int32_t val, cur, prior;

    if ( self == NULL )
        return vrUseConfig;

    /* convert "VResolverEnableState" to 32-bit signed integer for atomic operation */
    val = ( int32_t ) enable;

    /* before performing atomic swap, get the current setting,
       and return right away if it is already set correctly */
    prior = atomic32_read ( & enable_cache );
    if ( prior != val ) do
    {
        cur = prior;
        prior = atomic32_test_and_set ( & enable_cache, val, prior );
    }
    while ( prior != cur );

    return prior;
}

rc_t VResolverResolveName ( VResolver * self, int resolve ) {
    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcUpdating, rcSelf, rcNull );

    switch ( resolve ) {
        case 0 : self -> resoveOidName = DEFAULT_RESOVE_OID_NAME; break;
        case 1 : self -> resoveOidName = true                   ; break;
        default: self -> resoveOidName = false                  ; break;
    }
    return 0;
}

/* RemoteResolve
 *  resolve an accession into a remote VPath or not found
 *  may optionally open a KFile to the object in the process
 *  of finding it
 *
 *  2. determine the type of accession we have, i.e. its "app"
 *  3. search all local algorithms of app type for accession
 *  4. return not found or new VPath
 */
rc_t VResolverRemoteResolve ( const VResolver *self,
    VRemoteProtocols protocols, const String * accession,
    const VPath ** path, const VPath **mapping,
    const KFile ** opt_file_rtn, bool refseq_ctx, bool is_oid, const char * version )
{
    rc_t rc, try_rc;
    uint32_t i, count;

    VResolverAccToken tok;
    VResolverAppID app, wildCard;
    bool legacy_wgs_refseq = false;

    VResolverEnableState remote_state = atomic32_read ( & enable_remote );

    /* subject the accession to pattern recognition */
    if ( ! is_oid )
        app = get_accession_app ( accession, refseq_ctx, & tok,
                                  & legacy_wgs_refseq, true, NULL, NULL, -1 );
    else
    {
        app = appAny;
        VResolverAccTokenInitFromOID ( & tok, accession );
    }

    rc = VResolverRemoteMagicResolve(self, path, accession, app);
    if (rc != 0 || *path != NULL)
        return rc;

    assert(self);

    /* search all remote volumes by app and accession algorithm expansion */
    count = VectorLength ( & self -> remote );

    /* allow matching wildcard app */
    wildCard = appAny;
#if NO_REFSEQ_CGI
    if ( app == appREFSEQ )
        wildCard = -1;
#endif

    /* no error recorded yet */
    rc = 0;

    /* TBD - determine whether these settings interfere with
       case of resolving oid to cache location */

    /* test for forced enable, which applies only to main guys
       TBD - limit to main sub-category */
    if ( remote_state == vrAlwaysEnable )
    {
        for ( i = 0; i < count; ++ i )
        {
            const VResolverAlg *alg = VectorGet ( & self -> remote, i );
            if ( alg -> app_id == app || alg -> app_id == wildCard )
            {
                try_rc = VResolverAlgRemoteResolve ( alg, self -> kns, protocols,
                    & tok, path, mapping, opt_file_rtn, legacy_wgs_refseq, version );
                if ( try_rc == 0 )
                    return 0;
                if ( rc == 0 )
                    rc = try_rc;
            }
        }
    }
    else
    {
        const VResolverAlg * alg4 = NULL;
        ver_t v = InitVersion(version, self->ticket);
        for ( i = 0; i < count; ++ i )
        {
            const VResolverAlg *alg = VectorGet ( & self -> remote, i );
            assert(alg);
            if ( ( alg -> app_id == app || alg -> app_id == wildCard ) && ! alg -> disabled )
            {
                bool ok = false;
                if (v == 0);
                else if (v <= VERSION_4_0) {
                    if (alg->version == VERSION_3_0 ||
                        alg->version == VERSION_4_0)
                    {
                        ok = true;
                    }
                }
                else if (v == alg->version)
                    ok = true;
                if (ok) {
                    try_rc = VResolverAlgRemoteResolve(alg, self->kns,
                        protocols, &tok, path, mapping, opt_file_rtn,
                        legacy_wgs_refseq, version);
                    if (try_rc == 0)
                        return 0;
                    if (rc == 0)
                        rc = try_rc;
                }
                else if (alg->version == VERSION_3_0 ||
                    alg->version == VERSION_4_0)
                {
                    alg4 = alg;
                }
            }
        }
        if (rc == 0 && count > 0) {
            if (v > VERSION_4_0 && alg4 != NULL) {
                /* fallback to old names service */
                try_rc = VResolverAlgRemoteResolve(alg4, self->kns,
                    protocols, &tok, path, mapping, opt_file_rtn,
                    legacy_wgs_refseq, "4");
                if (try_rc == 0)
                    return 0;
                if (rc == 0)
                    rc = try_rc;
            }
            else {
                rc = RC(rcVFS, rcResolver, rcResolving, rcName, rcNotFound);
                PLOGERR(klogErr, (klogErr, rc,
                    "cannot find names service version $(vers). "
                    "Hint: run \"vdb-config --restore-defaults\"",
                    "vers=%V", v));
            }
        }
    }

    if ( rc != 0 )
        return rc;

    return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
}


/* Remote
 *  Find an existing remote file that is named by the accession.
 *
 *  rcState of rcNotFound means it did not exist and can not be
 *  downloaded. Probably a bad accession name.
 *
 *  Need a specific rc for no network configured.
 *  Need a specific rc for network access permitted.
 *
 *  Other rc code for failure are possible.
 *
 *  Accession must be an ncbi-acc scheme or a simple name with no
 *  directory paths.
 */
LIB_EXPORT
rc_t CC VResolverRemote ( const VResolver * self,
    VRemoteProtocols protocols, const VPath * accession,
    const VPath ** path )
{
    return VResolverQuery ( self, protocols, accession, NULL, path, NULL );
}

/* ExtractAccessionApp
 *  examine a path for accession portion,
 *  and try to recognize what app it belongs to
 */
static
VResolverAppID VResolverExtractAccessionApp ( const VResolver *self,
    const VPath * query, bool has_fragment,
    String * accession, VResolverAccToken * tok,
    bool *legacy_wgs_refseq, bool resolveAllAccToCache, bool * forDirAdjusted )
{
    bool refseq_ctx = has_fragment;

    assert(query);

    * accession = query -> path;

    if ( query -> fragment . size > 1 )
        refseq_ctx = true;

    /* should have something looking like an accession.
       determine its app to see if we were successful */
    return get_accession_app ( accession, refseq_ctx, tok, legacy_wgs_refseq,
        resolveAllAccToCache, forDirAdjusted,
        query->accOfParentDb, query->projectId );
}

static
bool VPathHasDownloadTicket ( const VPath * url )
{
    size_t num_read;
    char option [ 64 ];

    rc_t rc = VPathOption ( url, vpopt_gap_ticket, option, sizeof option, & num_read );

    if (rc != 0)
        rc = VPathOption(url, vpopt_gap_prjId, option, sizeof option,
            &num_read);

    return rc == 0;
}

static
rc_t VPathExtractAcc ( const VPath * url, VPath ** acc )
{
    rc_t rc;
    String accession;

    /* locate last path or accession guy */
    const char * start, * sep, * end;

    String empty;
    memset(&empty, 0, sizeof empty);

    assert(url);

    start = string_rchr ( url -> path . addr, url -> path . size, '/' );
    end = url -> path . addr + url -> path . size;
    if ( start ++ == NULL )
        start = url -> path . addr;

    /* strip off known extensions */
    sep = string_rchr ( start, end - start, '.' );
    while ( sep != NULL )
    {
        switch ( end - sep )
        {
        case 4:
            if ( strcase_cmp ( ".sra", 4, sep, 4, 4 ) == 0 )
                end = sep;
            else if ( strcase_cmp ( ".wgs", 4, sep, 4, 4 ) == 0 )
                end = sep;
            break;
        case 9:
            if ( strcase_cmp ( ".vdbcache", 9, sep, 9, 9 ) == 0 ||
                 strcase_cmp ( ".ncbi_enc", 9, sep, 9, 9 ) == 0 )
            {
                end = sep;
                sep = string_rchr ( start, end - start, '.' );
                continue;
            }
            break;
        }
        break;
    }

    if (url -> id . addr != NULL && url -> id . size != 0)
        StringInit(&accession, url->id.addr, url->id.size, url->id.len);
    else
        /* this is the string */
        StringInit ( & accession, start, end - start,
            string_len ( start, end - start ) );

    /* now extract the mapping */
    rc = LegacyVPathMakeFmt ( acc, "ncbi-acc:%S%S%S", & accession,
        url -> magic ? &empty : & url -> query, & url -> fragment );
    if ( rc == 0 )
    {
        VPath * ap = * acc;

        /* fix up case where we said accession but it was really a name */
        if ( ap -> acc_code == 0 || ap -> path_type != vpAccession )
            CONST_STRING ( & ap -> scheme, "ncbi-file" );

        ap->projectId = url->projectId;
        rc = VPathSetAccOfParentDb(ap, url->accOfParentDb);
    }

    return rc;
}

bool VResolverResolveToAd(const VResolver *self) {
    if (self != NULL && self->kns != NULL)
        return self->kns->enabledResolveToAd;
    else
        return false;
}

static
rc_t VResolverCacheResolve ( const VResolver *self, const VPath * query,
    bool has_fragment, const VPath ** cache, bool refseq_ctx,
    bool resolveAllAccToCache, const char * dir, bool * resolvedToDir )
{
    rc_t rc = 0;

    String accession;
    VResolverAccToken tok;
    bool legacy_wgs_refseq = false;

 /* Requested "to resolve ACC to cache" (in this case we resolving URL query)
    but query is not accession so we should resolve to the [current] directory
  */
    bool forDirAdjusted = false;

    VResolverAppID app = VResolverExtractAccessionApp ( self,
        query, has_fragment, & accession, & tok, & legacy_wgs_refseq,
        resolveAllAccToCache, & forDirAdjusted );

    /* going to walk the local volumes, and remember
       which one was best. actually, we have no algorithm
       for determining it, so it's just the comment for TBD */
    const VResolverAlg *alg, *better = NULL, *best = NULL;

    /* search the volumes for a cache-enabled place */
    uint32_t i, count = VectorLength ( & self -> local );

    /* check for protected status by presence of a download ticket */
    bool protected = VPathHasDownloadTicket ( query );

    VResolverEnableState cache_state = atomic32_read ( & enable_cache );

 /* bool ad = false; */

    rc = VResolverCacheMagicResolve(self, cache, app);
    if (rc != 0 || *cache != NULL)
        return rc;

    if ( dir != NULL )
        forDirAdjusted = true;

    /* check for cache-enable override */
    if ( cache_state == vrAlwaysEnable )
    {
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
               ("VResolverCacheResolve: app = %d\n", app));
        assert(self->kns);
        if (self->kns->resolveToCache) {
            for (i = 0; i < count; ++i)
            {
                alg = VectorGet(&self->local, i);

                if (alg->cache_capable && alg -> protected == protected &&
                    (alg->app_id == app || alg->app_id == appAny))
                {
                    /* try to find an existing cache file
                       NB - race condition exists unless
                       we do something with lock files */
                    if (forDirAdjusted)
                        rc = VResolverAlgCacheResolveDir(alg, self->wd, &tok,
                            cache, legacy_wgs_refseq, dir, resolvedToDir);
                    else
                        rc = VResolverAlgCacheResolve(alg, self->wd, &tok,
                            cache, legacy_wgs_refseq);
                    if (rc == 0)
                        return 0;

                    /* just remember the first as best for now */
                    if (alg->app_id == app) {
                        if (best == NULL)
                            best = alg;
                    }
                    else {
                        assert(alg->app_id == appAny);
                        if (better == NULL)
                            better = alg;
                    }
                }
            }
        }

        /* check AD */
        count = VResolverResolveToAd(self) ? VectorLength(&self->ad) : 0;
        for (i = 0; i < count; ++i)
        {
            const VResolverAlg *alg = VectorGet(&self->ad, i);
            if (alg->cache_capable && alg -> protected == protected &&
                (alg->app_id == app || alg->app_id == appAny))
            {
                /* try to find an existing cache file
                   NB - race condition exists unless
                   we do something with lock files */
                if (forDirAdjusted)
                    rc = VResolverAlgCacheResolveDir(alg, self->wd, &tok,
                        cache, legacy_wgs_refseq, dir, resolvedToDir);
                else
                    rc = VResolverAlgCacheResolve(alg, self->wd, &tok,
                        cache, legacy_wgs_refseq);
                if (rc == 0) {
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
                        ("VResolverCacheResolve: "
                        "cache location of '%S' resolved to '%S' with %R\n",
                        &query->path, &(*cache)->path, rc));
                    return 0;
                }

                /* just remember the first as best for now */
                if (alg->app_id == app) {
                    if (best == NULL) {
                        best = alg;
                     /* ad = true; */
                    }
                }
                else {
                    assert(alg->app_id == appAny);
                    if (better == NULL)
                        better = alg;
                }
            }
        }
    }
    else
    {
        /* we use user cache when it is allowed by kns manager */
        bool useCache = self->kns->resolveToCache;

        /* check AD */
        bool useAd = false;
        if (VResolverResolveToAd(self)) /* resolving to AD is enabled */
            useAd = true;                  /*     in KnsMgr [by prefetch]*/
        if (dir != NULL)    /* out-dir is provided */
            useAd = true;   /* [when prefetch downloads to out-dir]
                                - use AD, too */
#if 0
        if (!protected && VPathGetProjectId(query, NULL)) {
            useAd = true;     /* resolving protected URL returned by SDL */
            useCache = false; /* here resolve only to AD, not to cache */
        }
#endif

        /* we don't use user cache but AD
           when we can use AD and: */
        if (useAd && dir != NULL && /* downloading to dir */
            (app == appSRA ||      /* and downloading SRA */
                app == appFILE))   /*                 or FILE */
            useCache = false;

        assert(self->kns);

        if (useCache) {
            for (i = 0; i < count; ++i)
            {
                alg = VectorGet(&self->local, i);

                if (alg->cache_enabled && alg -> protected == protected &&
                    (alg->app_id == app || alg->app_id == appAny))
                {
                    /* try to find an existing cache file
                       NB - race condition exists unless
                       we do something with lock files */
                    if (forDirAdjusted)
                        rc = VResolverAlgCacheResolveDir(alg, self->wd, &tok,
                            cache, legacy_wgs_refseq, dir, resolvedToDir);
                    else
                        rc = VResolverAlgCacheResolve(alg, self->wd, &tok,
                            cache, legacy_wgs_refseq);
                    if (rc == 0)
                        return 0;

                    /* just remember the first as best for now */
                    if (best == NULL)
                        best = alg;
                }
            }
        }

        /* use AD when allowed */
        count = useAd ? VectorLength(&self->ad) : 0;
        for (i = 0; i < count; ++i)
        {
            const VResolverAlg *alg = VectorGet(&self->ad, i);
            if (alg->cache_enabled && alg -> protected == protected &&
                (alg->app_id == app || alg->app_id == appAny))
            {
                /* try to find an existing cache file
                   NB - race condition exists unless
                   we do something with lock files */
                if (forDirAdjusted)
                    rc = VResolverAlgCacheResolveDir(alg, self->wd, &tok,
                        cache, legacy_wgs_refseq, dir, resolvedToDir);
                else
                    rc = VResolverAlgCacheResolve(alg, self->wd, &tok,
                        cache, legacy_wgs_refseq);
                if (rc == 0)
                    return 0;

                /* just remember the first as best for now */
                if (best == NULL) {
                    best = alg;
                 /* ad = true; */
                }
            }
        }
    }

    /* no existing cache file was found,
       so create a new one using the best
       TBD - this should remember a volume path */
    if ( best == NULL && better == NULL ) {
        rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
        assert(query);
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), (
            "VResolverCacheResolve: cache location of '%S' not found\n",
            &query->path));
    }
    else {
        alg = best == NULL ? better : best;
        assert ( alg );
        rc = VResolverAlgMakeCachePath ( alg, & tok, cache, legacy_wgs_refseq,
            self -> wd /* ad ? self -> wd : NULL*/ );
        assert(cache);
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("VResolverCacheResolve: "
            "cache location of '%S' resolved to '%S' with %R\n",
            &query->path, &(*cache)->path, rc));
    }

    return rc;
}

static
rc_t VResolverCacheFile ( const VResolver *self, const VPath * query, const VPath ** cache )
{
    rc_t rc = 0;

    /* going to walk the local volumes, and remember
       which one was best. actually, we have no algorithm
       for determining it, so it's just the comment for TBD */
    const VResolverAlg *alg, *best = NULL;

    /* search the volumes for a cache-enabled place */
    uint32_t i, count = VectorLength ( & self -> local );

    /* check for protected status by presence of a download ticket */
    bool protected = VPathHasDownloadTicket ( query );

    VResolverEnableState cache_state = atomic32_read ( & enable_cache );

    /* check for cache-enable override */
    if ( cache_state == vrAlwaysEnable )
    {
        for ( i = 0; i < count; ++ i )
        {
            alg = VectorGet ( & self -> local, i );
            if ( alg -> cache_capable && alg -> protected == protected && alg -> app_id == appFILE )
            {
                /* try to find an existing cache file
                   NB - race condition exists unless
                   we do something with lock files */
                rc = VResolverAlgCacheFile ( alg, self -> wd, query, cache );
                if ( rc == 0 )
                    return 0;

                /* just remember the first as best for now */
                if ( best == NULL )
                    best = alg;
            }
        }
        /* check AD */
        count = VResolverResolveToAd(self) ? VectorLength(&self->ad) : 0;
        for (i = 0; i < count; ++i) {
            const VResolverAlg *alg = VectorGet(&self->ad, i);
            if (alg->cache_capable && alg -> protected == protected
                && alg->app_id == appFILE)
            {
                {
                    /* try to find an existing cache file
                       NB - race condition exists unless
                       we do something with lock files */
                    rc = VResolverAlgCacheFile(alg, self->wd, query, cache);
                    if (rc == 0)
                        return 0;

                    /* just remember the first as best for now */
                    if (best == NULL)
                        best = alg;
                }
            }
        }
    }
    else
    {
        for ( i = 0; i < count; ++ i )
        {
            alg = VectorGet ( & self -> local, i );
            if ( alg -> cache_enabled && alg -> protected == protected && alg -> app_id == appFILE )
            {
                /* try to find an existing cache file
                   NB - race condition exists unless
                   we do something with lock files */
                rc = VResolverAlgCacheFile ( alg, self -> wd, query, cache );
                if ( rc == 0 )
                    return 0;

                /* just remember the first as best for now */
                if ( best == NULL )
                    best = alg;
            }
        }
    }

    /* no existing cache file was found,
       so create a new one using the best
       TBD - this should remember a volume path */
    if ( best == NULL )
        rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
    else
        rc = VResolverAlgMakeCacheFilePath ( best, query, cache );

    return rc;
}


/* Cache
 *  Find a cache directory that might or might not contain a partially
 *  downloaded file.
 *
 *  Accession must be an ncbi-acc scheme, an http url or a simple name with no
 *  directory paths. All three should return the same directory URL as a VPath. (?)
 *  Or should it be a directory or a file url depending upon finding a partial
 *  download? This would require co-ordination with all download mechanisms that
 *  we permit.
 *
 *  With refseq holding wgs objects we have a case were the downloaded file is not
 *  named the same as the original accession as the file archive you want is a
 *  container for other files.
 *
 *  Find local will give a path that has a special scheme in these cases.
 *  Find remote will give the url for the container that contains the accession
 *  so using the returned VPath from resolve remote is better than the original
 *  accession in this one case.  I think...
 */
LIB_EXPORT
rc_t CC VResolverCache ( const VResolver * self,
    const VPath * url, const VPath ** path, uint64_t file_size )
{
    return VResolverQuery ( self, self -> protocols, url, NULL, NULL, path );
}

/* QueryOID
 */

static
rc_t get_query_accession ( const VPath * query, String * accession, char * oid_str, size_t bsize )
{
    rc_t rc;

    /* going to treat oid as accession */
    * accession = query -> path;

    /* if the VPath already gives us a numeral, great */
    if ( query -> path . size != 0 && query -> path . addr [ 0 ] != '0' )
        return 0;

    /* otherwise, generate one on stack */
    rc = string_printf ( oid_str, bsize, & accession -> size, "%u", query -> obj_id );
    if ( rc == 0 )
    {
        accession -> addr = oid_str;
        accession -> len = ( uint32_t ) accession -> size;
    }

    return rc;
}

static
rc_t VResolverQueryOID ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local, const VPath ** remote, const VPath ** cache, const char * version )
{
    rc_t rc;

    assert(query);

    /* require non-zero oid */
    if ( query -> obj_id == 0 )
        rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcCorrupt );
    else
    {
        /* temporary - no access to vfs
           NB - this manager will either use a singleton
           or create a new one with its existing config */
        VFSManager * vfs;
        rc = VFSManagerMake ( & vfs );
        if ( rc == 0 )
        {
            char oid_str [ 32 ];
            String accession;
            VPath * mapped_query = NULL;

            /* not expected to ever be true */
            bool refseq_ctx = VPathHasRefseqContext ( query );

/* OLD =====================================================
   PREFACE - having an oid, we will need to map it to either
   an accession or simple filename before resolving to a
   local or cache path. there are two ways of getting this
   mapping: either through the VFS manager, or by asking the
   remote resolver CGI.

   ASSUMPTION - if the file exists locally or is cached,
   there should be a mapping available to VFS manager. this
   assumption can fail if the mapping database has been lost
   or damaged.
   ==========================================================
   NEW ======================================================
   Remote resolver CGI does not return file name since version 3.
   File name, aling with oid comes from kart file.
   Mapping database is updated while kart is read.
 */

            /* MAP OID TO ACCESSION */
            if ( local != NULL || cache != NULL )
            {
                /* we want a mapping. ask VFS manager for one */
                rc = VFSManagerGetObject ( vfs, query -> obj_id, & mapped_query );
                if ( GetRCState ( rc ) == rcNotFound && self -> resoveOidName )
                {
/* NEW: should never got here, mapping should be registered when reading kart
   file in the same application.
   'bool resoveOidName' is used for testing
   or not to fail when sometihing unexpected happens
 */
                    /* no mapping could be found. another possibility is to resolve remotely */
                    if ( remote != NULL || atomic32_read ( & enable_remote ) != vrAlwaysDisable )
                    {
                        rc = get_query_accession ( query, & accession, oid_str, sizeof oid_str );
                        if ( rc == 0 )
                        {
                            const VPath * remote2 = NULL;
                            const VPath * remote_mapping = NULL;
/* call CGI with version 1.2 */
                            rc = VResolverRemoteResolve ( self, protocols,
                                & accession, & remote2, & remote_mapping, NULL,
                                refseq_ctx, true, "#1.2" );
                            if ( rc == 0 )
                            {
                                /* got it. now enter into VFS manager's table */
                                rc = VFSManagerRegisterObject ( vfs, query -> obj_id, remote_mapping );
                                if ( rc == 0 )
                                {
                                    mapped_query = ( VPath* ) remote_mapping;
                                    remote_mapping = NULL;
                                    if ( remote != NULL )
                                    {
                                        * remote = remote2;
                                        remote2 = NULL;
                                    }
                                }

                                VPathRelease ( remote2 );
                                VPathRelease ( remote_mapping );
                            }
                        }
                    }
                }

                if ( rc == 0 )
                {
                    assert ( mapped_query != NULL );

                    /* the returned VPath should be of a usable type */
                    assert ( mapped_query -> path_type == vpAccession       ||
                             mapped_query -> path_type == vpNameOrAccession ||
                             mapped_query -> path_type == vpName );
                    assert ( mapped_query -> path . size != 0 );
                }
            }

            /* RESOLVE FOR LOCAL PATH */
            if ( local != NULL && mapped_query != NULL )
            {
                if ( mapped_query -> path_type == vpName )
                {
                    /* see if this is a file stored locally */
                    rc = VResolverLocalFile ( self, mapped_query, local );
                }
                else
                {
                    /* grab the path as accession */
                    accession = mapped_query -> path;

                    /* resolve from accession to local path
                       will NOT find partial cache files */
                    rc = VResolverLocalResolve ( self, & accession, local,
                        refseq_ctx, false, NULL,
                        query -> accOfParentDb, query -> projectId );
                }

                if ( rc == 0 && remote != NULL && * remote != NULL )
                {
                    /* dump remote path used to map oid */
                    VPathRelease ( * remote );
                    * remote = NULL;
                }
            }

/* NEW: not sure why this code is here, so do not touch it,
        call resolver-1.2 when 'resoveOidName' is set */
            if ( local == NULL || * local == NULL )
            {
                bool has_fragment = false;

                /* RESOLVE FOR REMOTE */
                if ( remote != NULL && * remote == NULL
                                    && self -> resoveOidName )
                {
                    rc = get_query_accession ( query, & accession, oid_str, sizeof oid_str );
                    if ( rc == 0 )
                    {
                        const VPath * remote_mapping = NULL;
/* call CGI with version 1.2 */
                        rc = VResolverRemoteResolve ( self, protocols,
            & accession, remote,
            ( mapped_query == NULL && cache != NULL ) ? & remote_mapping : NULL,
            NULL, refseq_ctx, true, "#1.2" );

                        if ( rc == 0 && mapped_query == NULL && cache != NULL && remote_mapping == NULL )
                        {
                            /* THIS IS LIKELY AN INTERNAL ERROR
                               EITHER THE CGI DID NOT RETURN A MAPPING
                               OR WE DID NOT PROPERLY PARSE IT */
                            VPathRelease ( * remote );
                            rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNull );
                        }

                        /* register new mapping */
                        if ( rc == 0 )
                        {
                            assert ( * remote != NULL );
                            if ( ( * remote ) -> fragment . size != 0 )
                                has_fragment = true;
                            if ( remote_mapping != NULL )
                            {
                                rc = VFSManagerRegisterObject ( vfs, query -> obj_id, remote_mapping );
                                if ( rc == 0 )
                                {
                                    mapped_query = ( VPath* ) remote_mapping;
                                    remote_mapping = NULL;
                                }
                                VPathRelease ( remote_mapping );
                            }
                        }
                    }
                }

                /* RESOLVE FOR CACHE */
                if ( ( remote == NULL || * remote != NULL ) && cache != NULL && mapped_query != NULL )
                {
                    /* test for file or accession */
                    if ( mapped_query -> path_type == vpName )
                    {
                        /* see if this is a file stored locally */
                        rc = VResolverCacheFile ( self, mapped_query, cache );
                    }
                    else
                    {
                        /* resolve from accession to cache path */
                        rc = VResolverCacheResolve ( self, mapped_query,
                            has_fragment, cache, refseq_ctx, true, NULL, NULL );
                    }
                    if ( rc != 0 && remote != NULL )
                    {
                        assert ( * cache == NULL );
                        VPathRelease ( * remote );
                        * remote = NULL;
                    }
                }
            }

            VPathRelease ( mapped_query );

            VFSManagerRelease ( vfs );
        }
    }

    return rc;
}

/* QueryAcc
 */
static
rc_t VResolverQueryAcc ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local,
    const VPath ** remote, const VPath ** cache, const char * version,
    bool resolveAllAccToCache, const char * dir, bool * resolvedToDir,
    const VPath * oldRemote, const VPath * oldMapping )
{
    rc_t rc = 0;

    /* the accession should be directly usable */
    const String * accession = & query -> path;

    /* check if it is intended to locate a legacy refseq object */
    bool refseq_ctx = VPathHasRefseqContext ( query );

    /* will be needed to consult CGI */
    const VPath * remote2 = NULL, * mapped_query = NULL;

    /* LOCAL RESOLUTION */
    if ( local != NULL )
        rc = VResolverLocalResolve ( self, accession, local, refseq_ctx,
            resolveAllAccToCache, dir,
            query -> accOfParentDb, query -> projectId );

    if ( local == NULL || * local == NULL )
    {
        bool has_fragment = false;

        /* REMOTE RESOLUTION */
        if ( remote != NULL || ( self -> ticket != NULL && cache != NULL ) )
        {
            /* will need to map if protected */
            const VPath ** mapped_ptr = ( self -> ticket != NULL && cache != NULL ) ?
                & mapped_query : NULL;

            if (oldRemote != NULL) {
                remote2 = oldRemote;
                if (mapped_ptr != NULL) {
                    rc = VPathAddRef(oldMapping);
                    if (rc != 0)
                        return rc;
                    mapped_query = oldMapping;
                }
            }
            else {
                /* request remote resolution
                   this does not need to map the query to an accession */
                rc = VResolverRemoteResolve ( self, protocols, accession,
                    & remote2, mapped_ptr, NULL, refseq_ctx, false, version );

                if ( rc == 0 ) {
                    if ( remote2 -> fragment . size != 0 )
                        has_fragment = true;

                    if ( remote != NULL )
                        * remote = remote2;
                    else
                        VPathRelease ( remote2 );

                    remote2 = NULL;
                }
            }
        }

        if ( ( remote == NULL || * remote != NULL ) && cache != NULL )
        {
            if ( mapped_query != NULL )
                rc = VResolverCacheResolve ( self, mapped_query, has_fragment,
                    cache, refseq_ctx,
                    resolveAllAccToCache, dir, resolvedToDir );
#if 0
            /* the bad assumption that every remotely retrieved accession MUST be mapped */
            else if ( self -> ticket != NULL )
                rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
#endif
            else
                rc = VResolverCacheResolve ( self, query, has_fragment,
                    cache, refseq_ctx,
                    resolveAllAccToCache, dir, resolvedToDir );

            if ( rc != 0 && remote != NULL )
            {
                assert ( * cache == NULL );
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else
                {
                    VPathRelease ( * remote );
                    * remote = NULL;
                }
            }
        }

        if ( mapped_query != NULL )
            VPathRelease ( mapped_query );
    }

    return rc;
}

/* QueryPath
 *  this behavior may not be correct
 *  perhaps we should reject paths upon input,
 *  and only resolve things that need resolving
 *  but there is a thought that we can also transform paths
 */
static
rc_t VResolverQueryPath ( const VResolver * self, const VPath * query, const VPath ** local )
{
    rc_t rc;

    if ( local == NULL )
        return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );

    switch ( KDirectoryPathType ( self -> wd, "%.*s", ( int ) query -> path . size, query -> path . addr ) )
    {
    case kptFile:
    case kptDir:
    case kptCharDev:
    case kptBlockDev:
    case kptFIFO:
    case kptFile | kptAlias:
    case kptDir | kptAlias:
    case kptCharDev | kptAlias:
    case kptBlockDev | kptAlias:
    case kptFIFO | kptAlias:
        break;
    default:
        return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
    }

    rc = VPathAddRef ( query );
    if ( rc == 0 )
        * local = query;

    return rc;
}


/* QueryName
 *  may eventually look for the name in local cache,
 *  but for now just return it as a path
 */
static
rc_t VResolverQueryName ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local, const VPath ** remote, const VPath ** cache )
{
    return VResolverQueryPath ( self, query, local );
}


/* QueryURL
 *  URL resolves to itself for remote and potentially to a path for cache
 */
static
rc_t VResolverQueryURL ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** remote, const VPath ** cache )
{
    rc_t rc = 0;

    /* if neither remote nor cache, then must have requested local,
       and a URL cannot be resolved to local in our world... */
    if ( ( ( size_t ) remote | ( size_t ) cache ) == 0 )
        return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );

    /* the URL always resolves to itself for remote */
    if ( remote != NULL )
    {
        rc = VPathAddRef ( query );
        if ( rc != 0 )
            return rc;
        * remote = query;
    }

    /* if we want a cache location, then try to resolve it */
    if ( cache != NULL )
    {
        VPath *mapping;

        /* check for refseq context */
        bool refseq_ctx = VPathHasRefseqContext ( query );

        /* first, extract accession or name from URL */
        rc = VPathExtractAcc ( query, & mapping );
        if ( rc == 0 )
        {
            /* now map to cache location */
            rc = VResolverCacheResolve ( self, mapping, false,
                                         cache, refseq_ctx, true, NULL, NULL );
            VPathRelease ( mapping );
            if ( GetRCState ( rc ) == rcNotFound && remote != NULL )
                rc = 0;
        }

        /* any error must invalidate "remote" */
        if ( rc != 0 && remote != NULL )
        {
            VPathRelease ( * remote );
            * remote = NULL;
        }
    }

    return rc;
}


static rc_t VPaths_SetParentAcc(const VPath * query,
    const VPath ** local, const VPath ** remote, const VPath ** cache)
{
    rc_t rc = 0;

    if (query != NULL) {
        const String * parentAcc = query->accOfParentDb;

        if (parentAcc != NULL) {
            if (rc == 0 && local != NULL && * local != NULL)
                rc = VPathSetAccOfParentDb((VPath *)* local, parentAcc);

            if (rc == 0 && remote != NULL && * remote != NULL)
                rc = VPathSetAccOfParentDb((VPath *)* remote, parentAcc);

            if (rc == 0 && cache != NULL && * cache != NULL)
                rc = VPathSetAccOfParentDb((VPath *)* cache, parentAcc);
        }
    }

    return rc;
}

/* Query
 *  resolve object location to either an existing local path,
 *  or a pair of remote URL + local cache location.
 *
 *  "protocols" [ IN ] - the desired protocols for remote resolution
 *
 *  "query" [ IN ] - a path that can represent:
 *     accession : a recognizable accession from NCBI or known organization
 *     obj-id    : a dbGaP object id
 *     path      : a filesystem path
 *     url       : a remote location
 *
 *  "local" [ OUT, NULL OKAY ] - optional return parameter for local path:
 *     accession : resolve to local user or site path
 *     obj-id    : resolve to local user protected path
 *     path      : return duplicate of input
 *     url       : set to NULL
 *
 *  "remote" [ OUT, NULL OKAY ] - optional return parameter for remote path:
 *     accession : resolve to URL
 *     obj-id    : resolve to URL
 *     path      : set to NULL
 *     url       : set to duplicate
 *
 *  "cache" [ OUT, NULL OKAY ] - optional return parameter for cache path:
 *     accession : resolve to user cache path
 *     obj-id    : resolve to user cache path
 *     path      : set to NULL
 *     url       : resolve to user cache path
 *
 *  any of the output parameters may be NULL, but not all, i.e. there
 *  must be at least one non-NULL return parameter.
 *
 *  if you DON'T want local resolution, pass NULL for "local" and
 *  the query will be resolved remotely. if you don't want remote
 *  resolution, pass NULL for "remote".
 *
 *  a query that is resolved locally will always return NULL for
 *  "remote" and "cache", if the parameters are provided.
 */
static
rc_t VResolverQueryInt ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local,
    const VPath ** remote, const VPath ** cache, const char * version,
    bool resolveAllAccToCache, const char * dir, bool * resolvedToDir,
    const VPath * oldRemote, const VPath * oldMapping )
{
    rc_t rc;

    if ( ( ( size_t ) local | ( size_t ) remote | ( size_t ) cache ) == 0 )
        rc = RC ( rcVFS, rcResolver, rcResolving, rcParam, rcNull );
    else
    {
        if ( protocols == eProtocolDefault )
            protocols = self -> protocols;

        if ( local != NULL )
        {
            * local = NULL;
            if ( atomic32_read ( & enable_local ) == vrAlwaysDisable )
                local = NULL;
        }

        if ( remote != NULL )
        {
            VRemoteProtocols remote_protos;

            * remote = NULL;

            if ( protocols > VRemoteProtocolsMake3 ( eProtocolMax, eProtocolMax, eProtocolMax ) )
                return RC ( rcVFS, rcResolver, rcUpdating, rcParam, rcInvalid );

            for ( remote_protos = protocols; remote_protos != 0; remote_protos >>= 3 )
            {
                VRemoteProtocols proto = remote_protos & eProtocolMask;
                if ( proto == eProtocolNone || proto > eProtocolMax )
                    return RC ( rcVFS, rcResolver, rcUpdating, rcParam, rcInvalid );
            }

            if ( atomic32_read ( & enable_remote ) == vrAlwaysDisable )
                remote = NULL;
        }

        if ( cache != NULL )
        {
            * cache = NULL;
            if ( atomic32_read ( & enable_cache ) == vrAlwaysDisable )
                cache = NULL;
        }

        if ( self == NULL )
            rc = RC ( rcVFS, rcResolver, rcResolving, rcSelf, rcNull );
        else if ( query == NULL )
            rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNull );
        else if ( ( ( size_t ) local | ( size_t ) remote | ( size_t ) cache ) == 0 )
            rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcNotFound );
        else
        {
            uint32_t i;

            String sQuery;

            /* record requested protocols */
            bool has_proto [ eProtocolMask + 1 ];
            memset ( has_proto, 0, sizeof has_proto );

            for ( i = 0; i < eProtocolMaxPref; ++ i )
                has_proto [ ( ( protocols >> ( i * 3 ) ) & eProtocolMask ) ] = true;

            memset(&sQuery, 0, sizeof sQuery);
            VPathGetPath(query, &sQuery);

            switch ( query -> scheme_type )
            {
            case vpuri_none:
            case vpuri_ncbi_file:
            case vpuri_file:
            case vpuri_ncbi_acc:
            case vpuri_ncbi_obj:
                break;

            case vpuri_http:
                /* check for all that allow http */
                if ( has_proto [ eProtocolHttp ] )
                    return VResolverQueryURL ( self, protocols, query, remote, cache );
                return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );

            case vpuri_https:
                /* check for all that allow https */
                if ( has_proto [ eProtocolHttps ] )
                    return VResolverQueryURL ( self, protocols, query, remote, cache );
                return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );

            case vpuri_fasp:
                /* check for all that allow fasp */
                if ( has_proto [ eProtocolFasp ] )
                    return VResolverQueryURL ( self, protocols, query, remote, cache );
                return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );

            default:
                return RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );
            }

            switch ( query -> path_type )
            {
            case vpInvalid:
                rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcInvalid );
                break;

            case vpOID:
                rc = VResolverQueryOID ( self, protocols, query, local, remote, cache, version );
                break;

            case vpAccession:
                rc = VResolverQueryAcc ( self, protocols, query, local, remote,
                    cache, version, resolveAllAccToCache, dir, resolvedToDir,
                    oldRemote, oldMapping );
                break;

            case vpNameOrOID:
                rc = VResolverQueryOID ( self, protocols, query, local, remote, cache, version );
                if (rc != 0) {
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH),
                        ("Resolver-%s: VResolverQueryOID('%S') failed, try_name\n",
                            self->version, &sQuery));
                    goto try_name;
                }
                break;

            case vpNameOrAccession:
                rc = VResolverQueryAcc ( self, protocols, query, local, remote,
                    cache, version, resolveAllAccToCache, dir, resolvedToDir,
                    oldRemote, oldMapping );
                if (rc != 0) {
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH),
                        ("Resolver-%s: VResolverQueryAcc('%S') failed, try_name\n",
                            self->version, &sQuery));
                    goto try_name;
                }
                break;

            case vpName:
                /* try to handle a weird case with non-accessioned names in refseq area */
                if ( query -> query . size != 0 )
                {
                    if ( VPathHasRefseqContext ( query ) )
                    {
                        rc = VResolverQueryAcc ( self, protocols, query, local,
                            remote, cache, version, resolveAllAccToCache,
                            dir, resolvedToDir, oldRemote, oldMapping );
                        if ( rc == 0 )
                            break;
                    }
                }
                else if ( ! resolveAllAccToCache ) {
                    rc = VResolverQueryAcc ( self, protocols, query, local,
                        remote, cache, version, resolveAllAccToCache,
                        dir, resolvedToDir, oldRemote, oldMapping );
                    if ( rc == 0 )
                        break;
                }
            try_name:
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_PATH),
                       ( "Resolver-%s: checking '%S' as file name\n",
                         self -> version, &sQuery) );
                rc = VResolverQueryName(self, protocols, query,
                                        local, remote, cache);
                break;

            case vpRelPath:
            case vpFullPath:
            case vpUNCPath:
                rc = VResolverQueryPath ( self, query, local );
                break;

            default:
                rc = RC ( rcVFS, rcResolver, rcResolving, rcPath, rcIncorrect );
            }
        }
    }

    if (rc == 0)
        rc = VPaths_SetParentAcc(query, local, remote, cache);

    return rc;
}

#ifdef TESTING_SERVICES_VS_OLD_RESOLVING
LIB_EXPORT
rc_t CC oldVResolverQuery ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local, const VPath ** remote, const VPath ** cache )
{
    rc_t rc = VResolverQueryInt ( self, protocols, query, local, remote, cache,
        NULL, true, NULL, NULL, NULL, NULL);
    if ( rc == 0 )
    {
        /* the paths returned from resolver are highly reliable */
        if ( local != NULL && * local != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) local, true );
        if ( remote != NULL && * remote != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) remote, true );
        if ( cache != NULL && * cache != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) cache, true );
    }
    return rc;
}
#endif

static
rc_t CC VResolverQueryImpl ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local, const VPath ** remote,
    const VPath ** cache, bool resolveAllAccToCache, const char * dir,
    bool * inOutDir, bool queryIsUrl,
    const VPath * oldRemote, const VPath * oldMapping )
{
    rc_t rcs = -1;
    rc_t rc = rcs = VResolverQueryInt ( self, protocols, query, local,
        remote, cache, self -> version, resolveAllAccToCache, dir, inOutDir,
        oldRemote, oldMapping );
    if ( rc == 0 )
    {
        /* the paths returned from resolver are highly reliable */
        if ( local != NULL && * local != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) local, true );
        if ( remote != NULL && * remote != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) remote, true );
        if ( cache != NULL && * cache != NULL )
            VPathMarkHighReliability ( * ( VPath ** ) cache, true );
    }
#ifdef TESTING_SERVICES_VS_OLD_RESOLVING
    if ( ! queryIsUrl ) {
        const VPath * oath = NULL;
        const VPath ** p = remote ? & oath : NULL;
        const VPath * cath = NULL;
        const VPath ** c = cache ? & cath : NULL;
        const VPath * lath = NULL;
        const VPath ** l = local ? & lath : NULL;
#if _DEBUGGING
        rc_t ro =
#endif
            VResolverQueryInt ( self, protocols, query, l, p, c,
                                "3", true, NULL, NULL, NULL, NULL);
        assert ( rcs == ro );
        if ( remote == NULL )
            assert ( p == NULL );
        else if ( * remote == NULL )
            assert ( p && * p == NULL && oath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) oath, true );
            assert ( ! VPathEqual ( * remote, oath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( cache == NULL )
            assert ( c == NULL );
        else if ( * cache == NULL )
            assert ( c && * c == NULL && cath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) cath, true );
            assert ( ! VPathEqual ( * cache, cath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( local == NULL )
            assert ( l == NULL );
        else if ( * local == NULL )
            assert ( l && * l == NULL && lath == NULL );
        else {
            VPathMarkHighReliability ( ( VPath * ) lath, true );
#if _DEBUGGING
            {
                int notequal = ~ 0;
                assert ( ! VPathEqual ( * local, lath, & notequal ) );
                assert ( ! notequal );
            }
#endif
        }
        RELEASE ( VPath, lath );
        RELEASE ( VPath, oath );
        RELEASE ( VPath, cath );
    }
    if ( true || ! queryIsUrl ) {
        const VPath * oath = NULL;
        const VPath ** p = remote ? & oath : NULL;
        const VPath * cath = NULL;
        const VPath ** c = cache ? & cath : NULL;
        const VPath * lath = NULL;
        const VPath ** l = local ? & lath : NULL;
#if _DEBUGGING
        rc_t ro =
#endif
            VResolverQueryInt ( self, protocols, query, l, p, c, "3",
                            resolveAllAccToCache, dir, inOutDir, NULL, NULL);
        assert ( rcs == ro );
        if ( remote == NULL )
            assert ( p == NULL );
        else if ( * remote == NULL )
            assert ( p && * p == NULL && oath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) oath, true );
            assert ( ! VPathEqual ( * remote, oath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( cache == NULL )
            assert ( c == NULL );
        else if ( * cache == NULL )
            assert ( c && * c == NULL && cath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) cath, true );
            assert ( ! VPathEqual ( * cache, cath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( local == NULL )
            assert ( l == NULL );
        else if ( * local == NULL )
            assert ( l && * l == NULL && lath == NULL );
        else {
            VPathMarkHighReliability ( ( VPath * ) lath, true );
#if _DEBUGGING
            {
                int notequal = ~ 0;
                assert ( ! VPathEqual ( * local, lath, & notequal ) );
                assert ( ! notequal );
            }
#endif
        }
        RELEASE ( VPath, lath );
        RELEASE ( VPath, oath );
        RELEASE ( VPath, cath );
    }
    if ( ! queryIsUrl ) {
        const VPath * oath = NULL;
        const VPath ** p = remote ? & oath : NULL;
        const VPath * cath = NULL;
        const VPath ** c = cache ? & cath : NULL;
        const VPath * lath = NULL;
        const VPath ** l = local ? & lath : NULL;
#if _DEBUGGING
        rc_t ro =
#endif
            VResolverQueryInt ( self, protocols, query, l, p, c,
                                "1.2", true, NULL, NULL, NULL, NULL );

        assert ( rcs == ro );
        if ( remote == NULL )
            assert ( p == NULL );
        else if ( * remote == NULL )
            assert ( p && * p == NULL && oath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) oath, true );
            assert ( ! VPathEqual ( * remote, oath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( cache == NULL )
            assert ( c == NULL );
        else if ( * cache == NULL )
            assert ( c && * c == NULL && cath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) cath, true );
            assert ( ! VPathEqual ( * cache, cath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( local == NULL )
            assert ( l == NULL );
        else if ( * local == NULL )
            assert ( l && * l == NULL && lath == NULL );
        else {
            VPathMarkHighReliability ( ( VPath * ) lath, true );
#if _DEBUGGING
            {
                int notequal = ~ 0;
                assert ( ! VPathEqual ( * local, lath, & notequal ) );
                assert ( ! notequal );
            }
#endif
        }
        RELEASE ( VPath, lath );
        RELEASE ( VPath, oath );
        RELEASE ( VPath, cath );
    }
    if ( true ) { // ! queryIsUrl ) {
        const VPath * oath = NULL;
        const VPath ** p = remote ? & oath : NULL;
        const VPath * cath = NULL;
        const VPath ** c = cache ? & cath : NULL;
        const VPath * lath = NULL;
        const VPath ** l = local ? & lath : NULL;
#if _DEBUGGING
        rc_t ro =
#endif
            VResolverQueryInt ( self, protocols, query, l, p, c, "1.2",
                            resolveAllAccToCache, dir, inOutDir, NULL, NULL );

        assert ( rcs == ro );
        if ( remote == NULL )
            assert ( p == NULL );
        else if ( * remote == NULL )
            assert ( p && * p == NULL && oath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) oath, true );
            assert ( ! VPathEqual ( * remote, oath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( cache == NULL )
            assert ( c == NULL );
        else if ( * cache == NULL )
            assert ( c && * c == NULL && cath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) cath, true );
            assert ( ! VPathEqual ( * cache, cath, & notequal ) );
            if ( notequal )
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
        }
        if ( local == NULL )
            assert ( l == NULL );
        else if ( * local == NULL )
            assert ( l && * l == NULL && lath == NULL );
        else {
            VPathMarkHighReliability ( ( VPath * ) lath, true );
#if _DEBUGGING
            {
                int notequal = ~ 0;
                assert ( ! VPathEqual ( * local, lath, & notequal ) );
                assert ( ! notequal );
            }
#endif
        }
        RELEASE ( VPath, lath );
        RELEASE ( VPath, oath );
        RELEASE ( VPath, cath );
    }
    if ( ! queryIsUrl ) {
        const VPath * oath = NULL;
        const VPath ** p = remote ? & oath : NULL;
        const VPath * cath = NULL;
        const VPath ** c = cache ? & cath : NULL;
        const VPath * lath = NULL;
        const VPath ** l = local ? & lath : NULL;
        rc_t ro = oldVResolverQuery ( self, protocols, query, l, p, c );
        /*ros = ro;*/
        if ( rc != ro ) {
            enum RCModule  mod = GetRCModule  ( rc );
            enum RCTarget  targ = GetRCTarget  ( rc );
            enum RCContext ctx = GetRCContext ( rc );
            if ( targ == rcQuery && GetRCTarget ( ro ) == rcTree ) {
                rc = ResetRCContext ( rc, mod, rcTree, ctx );
                /*fixed = true;*/
            }
        }
        assert ( rc == ro );
        if ( remote == NULL )
            assert ( p == NULL );
        else if ( * remote == NULL )
            assert ( p && * p == NULL && oath == NULL );
        else {
            int notequal = ~ 0;
            assert ( ! VPathEqual ( * remote, oath, & notequal ) );
            if ( notequal ) {
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
            }
        }
        if ( cache == NULL )
            assert ( c == NULL );
        else if ( * cache == NULL )
            assert ( c && * c == NULL && cath == NULL );
        else {
            int notequal = ~ 0;
            VPathMarkHighReliability ( ( VPath * ) cath, true );
            assert ( ! VPathEqual ( * cache, cath, & notequal ) );
            if ( notequal ) {
                assert ( VPathHasRefseqContext ( query ) && notequal == 2 );
            }
        }
        if ( local == NULL )
            assert ( l == NULL );
        else if ( * local == NULL )
            assert ( l && * l == NULL && lath == NULL );
        else {
            VPathMarkHighReliability ( ( VPath * ) lath, true );
#if _DEBUGGING
            {
                int notequal = ~ 0;
                assert ( ! VPathEqual ( * local, lath, & notequal ) );
                assert ( ! notequal );
            }
#endif
        }
        RELEASE ( VPath, lath );
        RELEASE ( VPath, oath );
        RELEASE ( VPath, cath );
    }
#endif

    return rc;
}

LIB_EXPORT
rc_t CC VResolverQuery ( const VResolver * self, VRemoteProtocols protocols,
    const VPath * query, const VPath ** local, const VPath ** remote,
    const VPath ** cache )
{
    return VResolverQueryImpl ( self, protocols, query, local, remote, cache,
                               false, NULL, NULL, false, NULL, NULL );
}

LIB_EXPORT
rc_t CC VResolverQueryWithDir ( const VResolver * self,
    VRemoteProtocols protocols, const VPath * query, const VPath ** local,
    const VPath ** remote, const VPath ** cache, bool resolveAccToCache,
    const char * outDir, bool * inOutDir, bool queryIsUrl,
    const VPath * oldRemote, const VPath * oldMapping )
{
    return VResolverQueryImpl ( self, protocols, query, local, remote, cache,
        resolveAccToCache, outDir, inOutDir, queryIsUrl,
        oldRemote, oldMapping );
}

/* LoadVolume
 *  capture volume path and other information
 */
static
rc_t VResolverAlgLoadVolume ( VResolverAlg *self, uint32_t *num_vols, const char *start, size_t size )
{
    rc_t rc = 0;

    uint32_t dummy = 0;
    if (num_vols == NULL)
        num_vols = &dummy;

#if 0
    /* trim volume whitespace */
    while ( size != 0 && isspace ( start [ 0 ] ) )
    {
        ++ start;
        -- size;
    }
    while ( size != 0 && isspace ( start [ size - 1 ] ) )
        -- size;
#endif

    /* trim trailing slashes */
    while ( size != 0 && start [ size - 1 ] == '/' )
        -- size;

    /* now see if the string survives */
    if ( size != 0 )
    {
        String loc_vol_str;
        const String *vol_str;
        StringInit ( & loc_vol_str, start, size, string_len ( start, size ) );
        rc = StringCopy ( & vol_str, & loc_vol_str );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> vols, NULL, vol_str );
            if ( rc == 0 )
            {
                * num_vols += 1;
                return 0;
            }

            StringWhack ( vol_str );
        }
    }

    return rc;
}

/* LoadVolumes
 *
 *    path-list
 *        = PATH
 *        | <path-list> ':' PATH ;
 */
static
rc_t VResolverAlgLoadVolumes ( VResolverAlg *self, uint32_t *num_vols, const String *vol_list )
{
    const char *start = vol_list -> addr;
    const char *end = & vol_list -> addr [ vol_list -> size ];
    const char *sep = string_chr ( start, end - start, ':' );
    while ( sep != NULL )
    {
        rc_t rc = VResolverAlgLoadVolume ( self, num_vols, start, sep - start );
        if ( rc != 0 )
            return rc;
        start = sep + 1;
        sep = string_chr ( start, end - start, ':' );
    }
    return VResolverAlgLoadVolume ( self, num_vols, start, end - start );
}

/* LoadAlgVolumes
 *
 *    volumes
 *        = <path-list> ;
 */
static
rc_t VResolverLoadAlgVolumes ( Vector *algs, const String *root,
    const String *ticket, bool cache_capable, VResolverAppID app_id,
    VResolverAlgID alg_id, uint32_t *num_vols, const String *vol_list,
    bool protected, bool disabled, bool cacheEnabled )
{
    VResolverAlg *alg;
    rc_t rc = VResolverAlgMake ( & alg, root, app_id, alg_id, protected, disabled );
    if ( rc == 0 )
    {
        alg -> ticket = ticket;
        alg -> cache_capable = cache_capable;
        alg -> cache_enabled = cacheEnabled;

        if ( ticket != NULL )
            alg -> alg_id = algCGI;

        rc = VResolverAlgLoadVolumes ( alg, num_vols, vol_list );
        if ( rc == 0 && VectorLength ( & alg -> vols ) != 0 )
        {
            rc = VectorAppend ( algs, NULL, alg );
            if ( rc == 0 )
                return 0;
        }

        VResolverAlgWhack ( alg, NULL );
    }

    return rc;
}

/* LoadApp
 *
 *    alg-block
 *        = <alg-type> <volumes> ;
 *
 *    alg-type
 *        = "flat" | "sraFlat" | "sra1024" | "sra1000" | "fuse1000"
 *        | "refseq" | "wgs" | "wgsFlag" | "fuseWGS"
 *        | "ncbi" | "ddbj" | "ebi"
 *        | "nannot" | "nannotFlat" | "fuseNANNOT" | "pileupNCBI" | "pileupEBI" | "pileupDDBJ" ;
 */
static
rc_t VResolverLoadVolumes ( Vector *algs, const String *root,
    const String *ticket, bool cache_capable, VResolverAppID app_id,
    uint32_t *num_vols, const KConfigNode *vols, bool resolver_cgi,
     bool protected, bool disabled, bool cacheEnabled )
{
    KNamelist *algnames;
    rc_t rc = KConfigNodeListChildren ( vols, & algnames );
    if ( rc == 0 )
    {
        uint32_t i, count;
        rc = KNamelistCount ( algnames, & count );
        for ( i = 0; i < count && rc == 0; ++ i )
        {
            const char *algname;
            rc = KNamelistGet ( algnames, i, & algname );
            if ( rc == 0 )
            {
                const KConfigNode *alg;
                rc = KConfigNodeOpenNodeRead ( vols, & alg, "%s", algname );
                if ( rc == 0 )
                {
                    VResolverAlgID alg_id = algUnknown;

                    /* if using CGI for resolution */
                    if ( resolver_cgi || strcmp ( algname, "cgi" ) == 0 )
                        alg_id = algCGI;
                    /* SRA "Accession Directory": acc/acc.sra */
                    else if ( strcmp ( algname, "sraAd" ) == 0 )
                        alg_id = algSRAAD;
                    /* stored in a flat directory as-is */
                    else if ( strcmp ( algname, "flat" ) == 0 )
                        alg_id = algFlat;
                    /* stored in a in Accesion as Directory as-is */
                    else if (strcmp(algname, "flatAd") == 0)
                        alg_id = algFlatAD;
                    /* file with extensions stored in a flat directory:
                    as-is for public files,
                    with project-id injected before extension for protected ones
                     */
                    else if (strcmp(algname, "withExtFlat") == 0)
                        alg_id = algWithExtFlat;
                    /* stored in Accesion as Directory
                       with ".sra" or ".sra.vdbcache" extension */
                    else if ( strcmp ( algname, "sraAd" ) == 0 )
                        alg_id = algSRAAD;
                    /* stored in a flat directory with ".sra" extension */
                    else if ( strcmp ( algname, "sraFlat" ) == 0 )
                        alg_id = algSRAFlat;
                    /* stored in a three-level directory with 1K banks and ".sra" extension */
                    else if ( strcmp ( algname, "sra1024" ) == 0 )
                        alg_id = algSRA1024;
                    /* stored in a three-level directory with 1000 banks and ".sra" extension */
                    else if ( strcmp ( algname, "sra1000" ) == 0 )
                        alg_id = algSRA1000;
                    /* stored in a four-level directory with 1000 banks and ".sra" extension */
                    else if ( strcmp ( algname, "fuse1000" ) == 0 )
                        alg_id = algFUSE1000;
                    /* stored in a flat directory with no extension */
                    else if ( strcmp ( algname, "refseq" ) == 0 )
                        alg_id = algREFSEQ;
                    /* Refseq "Accession Directory": acc/refseq */
                    else if (strcmp(algname, "refseqAd") == 0)
                        alg_id = algREFSEQAD;
                    /* stored in a flat directory with no extension */
                    else if ( strcmp ( algname, "wgsFlat" ) == 0 )
                        alg_id = algWGSFlat;
                    /* stored in a multi-level directory with no extension */
                    else if ( strcmp ( algname, "wgs" ) == 0 )
                        alg_id = algWGS;
                    else if ( strcmp ( algname, "wgs2" ) == 0 )
                        alg_id = algWGS2;
                    else if ( strcmp ( algname, "fuseWGS" ) == 0 )
                        alg_id = algFuseWGS;
                    /* stored in a three-level directory with 1K banks and no extension */
                    else if ( strcmp ( algname, "ncbi" ) == 0 ||
                              strcmp ( algname, "ddbj" ) == 0 )
                        alg_id = algSRA_NCBI;
                    /* stored in a three-level directory with 1000 banks and no extension */
                    else if ( strcmp ( algname, "ebi" ) == 0 )
                        alg_id = algSRA_EBI;

                    /* store files under their names in Accesion as Directory */
                    else if ( strcmp ( algname, "ad" ) == 0 )
                        alg_id = algAD;

                    /* new named annotation */
                    else if ( strcmp ( algname, "nannotFlat" ) == 0 )
                        alg_id = algNANNOTFlat;
                    else if ( strcmp ( algname, "nannot" ) == 0 )
                        alg_id = algNANNOT;
                    else if ( strcmp ( algname, "fuseNANNOT" ) == 0 )
                        alg_id = algFuseNANNOT;

                    /* new named annotation */
                    else if ( strcmp ( algname, "nakmerFlat" ) == 0 )
                        alg_id = algNAKMERFlat;
                    else if ( strcmp ( algname, "nakmer" ) == 0 )
                        alg_id = algNAKMER;
                    else if ( strcmp ( algname, "fuseNAKMER" ) == 0 )
                        alg_id = algFuseNAKMER;

                    /* pileup files */
                    else if ( strcmp ( algname, "pileupNCBI" ) == 0 )
                        alg_id = algPileup_NCBI;
                    else if ( strcmp ( algname, "pileupEBI" ) == 0 )
                        alg_id = algPileup_EBI;
                    else if ( strcmp ( algname, "pileupDDBJ" ) == 0 )
                        alg_id = algPileup_DDBJ;

                    if ( alg_id != algUnknown )
                    {
                        String *vol_list;
                        rc = KConfigNodeReadString ( alg, & vol_list );
                        if ( rc == 0 )
                        {
                            if ( StringLength ( vol_list ) != 0 )
                            {
                                rc = VResolverLoadAlgVolumes ( algs,
                                    root, ticket, cache_capable,
                                    app_id, alg_id, num_vols, vol_list,
                                    protected, disabled, cacheEnabled );
                            }
                            StringWhack ( vol_list );
                        }
                    }

                    KConfigNodeRelease ( alg );
                }
            }
        }

        KNamelistRelease ( algnames );
    }
    return rc;
}

/* LoadApp
 *
 *    app
 *        = [ <disabled> ] [ <cache-enabled> ] <vol-group> ;
 *
 *    disabled
 *        = "disabled" '=' ( "true" | "false" ) ;
 *
 *    cache-enabled
 *        = "cache-enabled" '=' ( "true" | "false" ) ;
 *
 *    vol-group
 *        = "volumes" <alg-block>* ;
 */
static
rc_t VResolverLoadApp ( VResolver *self, Vector *algs, const String *root,
    const String *ticket, bool cache_capable, VResolverAppID app_id,
    uint32_t *num_vols, const KConfigNode *app, bool resolver_cgi,
    bool protected, bool disabled, bool cacheEnabled )
{
    const KConfigNode *node;

    /* test for disabled app - it is entirely possible */
    rc_t rc = KConfigNodeOpenNodeRead ( app, & node, "disabled" );
    if ( rc == 0 )
    {
        bool app_disabled = false;
        rc = KConfigNodeReadBool ( node, & app_disabled );
        KConfigNodeRelease ( node );
        if ( rc == 0 && app_disabled && (
            algs == & self -> local || /* allow to disable user app */
            algs == & self -> ad ) )   /* or AD app */
            return 0;
        disabled |= app_disabled;
    }

    /* test again for cache enabled */
    if ( cache_capable && cacheEnabled )
    {
        rc = KConfigNodeOpenNodeRead ( app, & node, "cache-enabled" );
        if ( rc == 0 )
        {
            /* allow this node to override current value */
            bool cache;
            rc = KConfigNodeReadBool ( node, & cache );
            KConfigNodeRelease ( node );
            if ( rc == 0 )
                cacheEnabled = cache;
        }
    }

    /* get volumes */
    rc = KConfigNodeOpenNodeRead ( app, & node, "volumes" );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        rc = VResolverLoadVolumes ( algs, root, ticket, cache_capable, app_id,
            num_vols, node, resolver_cgi, protected, disabled, cacheEnabled );
        KConfigNodeRelease ( node );
    }

    return rc;
}

/* LoadApps
 *
 *    app-block
 *        = <app-name> <app> ;
 *
 *    app-name
 *        = "refseq" | "sra" | "wgs" | "nannot" | "nakmer" | "sraPileup"
 *                                                         | "sraRealign";
 */
static
rc_t VResolverLoadApps ( VResolver *self, Vector *algs, const String *root,
    const String *ticket, bool cache_capable, const KConfigNode *apps,
    bool resolver_cgi, bool protected, bool disabled, bool cacheEnabled,
    bool noRegister )
{
    KNamelist *appnames;
    rc_t rc = KConfigNodeListChildren ( apps, & appnames );
    if ( rc == 0 )
    {
        uint32_t i, count;
        rc = KNamelistCount ( appnames, & count );
        if ( resolver_cgi && rc == 0 && count == 0 )
        {
            VResolverAlg *cgi;
            rc = VResolverAlgMake ( & cgi, root, appAny, algCGI, protected, disabled );
            if ( rc == 0 )
            {
                rc = VectorAppend ( algs, NULL, cgi );
                if ( rc == 0 )
                {
                    ++ self -> num_app_vols [ appAny ];
                    return 0;
                }
            }
        }
        else for ( i = 0; i < count && rc == 0; ++ i )
        {
            const char *appname;
            rc = KNamelistGet ( appnames, i, & appname );
            if ( rc == 0 )
            {
                const KConfigNode *app;
                rc = KConfigNodeOpenNodeRead ( apps, & app, "%s", appname );
                if ( rc == 0 )
                {
                    VResolverAppID app_id = appUnknown;
                    if ( strcmp ( appname, "file" ) == 0 )
                        app_id = appFILE;
                    else if ( strcmp ( appname, "nakmer" ) == 0 )
                        app_id = appNAKMER;
                    else if ( strcmp ( appname, "nannot" ) == 0 )
                        app_id = appNANNOT;
                    else if ( strcmp ( appname, "refseq" ) == 0 )
                        app_id = appREFSEQ;
                    else if ( strcmp ( appname, "sra" ) == 0 )
                        app_id = appSRA;
                    else if ( strcmp ( appname, "wgs" ) == 0 )
                        app_id = appWGS;
                    else if ( strcmp ( appname, "sraPileup" ) == 0 )
                        app_id = appSRAPileup;
                    else if (strcmp(appname, "sraRealign") == 0)
                        app_id = appSRARealign;

                    rc = VResolverLoadApp ( self, algs, root, ticket,
                        cache_capable, app_id,
                        noRegister ? NULL : & self -> num_app_vols [ app_id ],
                        app, resolver_cgi,
                        protected, disabled, cacheEnabled );

                    KConfigNodeRelease ( app );
                }
            }
        }
        KNamelistRelease ( appnames );
    }
    return rc;
}

typedef uint32_t EDisabled;
enum {
    eDisabledSet,
    eEnabledSet,
    eDisabledNotSet,
};

/* LoadRepo
 *
 *    repository
 *        = [ <disabled> ] [ <cache-enabled> ] <root> <app-group> ;
 *
 *    disabled
 *        = "disabled" '=' ( "true" | "false" ) ;
 *
 *    cache-enabled
 *        = "cache-enabled" '=' ( "true" | "false" ) ;
 *
 *    root
 *        = "root" '=' PATH ;
 *
 *    app-group
 *        = "apps" <app-block>* ;
 */
static
rc_t VResolverLoadRepo ( VResolver *self, Vector *algs, const KConfigNode *repo,
    const String *ticket, const char *name, bool cache_capable, bool protected,
    EDisabled isDisabled, bool cacheEnabled, bool noRegister )
{
    rc_t rc = 0;
    const KConfigNode *node;
    bool resolver_cgi;

    /* test for disabled repository */
    bool disabled = false;
    switch (isDisabled) {
        case eDisabledSet:
            disabled = true;
            break;
        case eEnabledSet:
            disabled = false;
            break;
        case eDisabledNotSet:
            rc = KConfigNodeOpenNodeRead ( repo, & node, "disabled" );
            if ( rc == 0 )
            {
                rc = KConfigNodeReadBool ( node, & disabled );
                KConfigNodeRelease ( node );
            }
            break;
    }

    assert(self);

    /* don't bother recording local, disabled repositories */
    if ( rc == 0 && disabled && algs == & self -> local )
        return 0;

    /* Check for cache-enabled.
       Cache-capable repositories cannot be remote.
       we do not check "cache-enabled" for not cache_capable repositories */
    if ( cacheEnabled ) {
        cacheEnabled = cache_capable;
    }
    if ( cacheEnabled )
    {
        rc = KConfigNodeOpenNodeRead ( repo, & node, "cache-enabled" );
        if ( rc == 0 )
        {
            rc = KConfigNodeReadBool ( node, & cacheEnabled );
            KConfigNodeRelease ( node );
            if ( rc != 0 )
                cacheEnabled = false;
        }
    }

    resolver_cgi = false;
    if ( cache_capable )
        rc = KConfigNodeOpenNodeRead ( repo, & node, "root" );
    else
    {
        /* check for specific resolver CGI */
        rc = KConfigNodeOpenNodeRead ( repo, & node, "resolver-cgi" );
        if ( rc == 0 )
            resolver_cgi = true;
        /* or get the repository root */
        else if ( GetRCState ( rc ) == rcNotFound )
        {
            rc = KConfigNodeOpenNodeRead ( repo, & node, "root" );
        }
    }
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        /* read root as String */
        String *root = NULL;
        rc = KConfigNodeReadString ( node, & root );
        KConfigNodeRelease ( node );
        if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;
        else if (root->size == 0); /* ignore repository with empty root node */
        else if ( rc == 0 )
        {
            /* perform a bit of cleanup on root */
            while ( root -> size != 0 && root -> addr [ root -> size - 1 ] == '/' )
            {
                /* this is terribly nasty, but known to be safe */
                -- root -> len;
                -- root -> size;
            }

            /* store string on VResolver for management purposes,
               pass the loaned reference to sub-structures */
            rc = VectorAppend ( & self -> roots, NULL, root );
            if ( rc != 0 )
                StringWhack ( root );
            else
            {
                /* open the "apps" sub-node */
                rc = KConfigNodeOpenNodeRead ( repo, & node, "apps" );
                if ( rc == 0 )
                {
                    rc = VResolverLoadApps ( self, algs, root, ticket,
                        cache_capable, node, resolver_cgi,
                        protected, disabled, cacheEnabled, noRegister );
                    KConfigNodeRelease ( node );
                }
                else if ( GetRCState ( rc ) == rcNotFound )
                {
                    rc = 0;
                    if ( resolver_cgi )
                    {
                        VResolverAlg *cgi;
                        rc = VResolverAlgMakeCgi( & cgi, root, protected,
                            disabled, ticket, name, &self->versions );
                        if ( rc == 0 )
                        {
                            assert(cgi);

                            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_KFG),
                                ("VResolverAlg(%s.%V, %S)\n",
                                    cgi->protected
                                    ? " PROTECTED" : "!protected",
                                    cgi->version, cgi->root));

                            rc = VectorAppend ( algs, NULL, cgi );
                            if ( rc == 0 )
                            {
                                ++ self -> num_app_vols [ appAny ];
                                return 0;
                            }
                        }

                        VResolverAlgWhack ( cgi, NULL );
                    }
                }
            }
        }
    }

    return rc;
}

/* LoadNamedRepo
 *
 *    repository-block
 *        = ID <repository> ;
 */
static
rc_t VResolverLoadNamedRepo ( VResolver *self, Vector *algs,
    const KConfigNode *sub, const String *ticket, const char *name,
    bool cache_capable, bool protected, EDisabled disabled, bool cacheEnabled,
    bool noRegister )
{
    const KConfigNode *repo;
    rc_t rc = KConfigNodeOpenNodeRead ( sub, & repo, "%s", name );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        rc = VResolverLoadRepo ( self, algs, repo, ticket, name,
            cache_capable, protected, disabled, cacheEnabled, noRegister );
        KConfigNodeRelease ( repo );
    }
    return rc;
}

/* LoadSubCategory
 *
 *    sub-category-block
 *        = <sub-category> <repository-block>* ;
 *
 *    sub-category
 *        = "main" | "aux" | "protected"
 *
 *    repository-block
 *        = ID <repository> ;
 */
static
rc_t VResolverLoadSubCategory ( VResolver *self, Vector *algs,
    const KConfigNode *kfg, const String *ticket, const char *sub_path,
    bool cache_capable, bool protected, EDisabled disabled, bool cacheEnabled,
    bool noRegister )
{
    const KConfigNode *sub;
    rc_t rc = KConfigNodeOpenNodeRead ( kfg, & sub, "%s", sub_path );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        KNamelist *children;
        rc = KConfigNodeListChildren ( sub, & children );
        if ( rc == 0 )
        {
            uint32_t i, count;
            rc = KNamelistCount ( children, & count );
            for ( i = 0; i < count && rc == 0; ++ i )
            {
                const char *name;
                rc = KNamelistGet ( children, i, & name );
                if ( rc == 0 )
                    rc = VResolverLoadNamedRepo ( self, algs, sub, ticket, name,
                        cache_capable, protected, disabled, cacheEnabled,
                        noRegister );
            }

            KNamelistRelease ( children );
        }
        KConfigNodeRelease ( sub );
    }
    return rc;
}

/* LoadProtected
 *  special function to handle single, active protected workspace
 */
static
rc_t VResolverLoadProtected ( VResolver *self, const KConfigNode *kfg,
    const char *rep_name,
    bool cache_capable, EDisabled disabled, bool cacheEnabled )
{
    const KConfigNode *repo;
    rc_t rc = KConfigNodeOpenNodeRead ( kfg, & repo, "user/protected/%s", rep_name );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        rc = VResolverLoadRepo ( self, & self -> local, repo,
            NULL, NULL, cache_capable, true, disabled, cacheEnabled, false );
        KConfigNodeRelease ( repo );
    }
    return rc;
}

/* LoadLegacyRefseq
 *  load refseq information from KConfig
 *
 *  there are two legacy versions being supported
 *
 *    legacy-refseq
 *        = "refseq" <legacy-vol-or-repo> ;
 *
 *    legacy-vol-or-repo
 *        = "volumes" '=' <path-list>
 *        | <legacy-refseq-repo> <legacy-refseq-vols>
 *        ;
 */
static
rc_t VResolverLoadLegacyRefseq
    ( VResolver *self, const KConfig *cfg, bool cache_capable )
{
    const KConfigNode *vols;
    rc_t rc = KConfigOpenNodeRead ( cfg, & vols, "/refseq/paths" );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        String *vol_list;
        rc = KConfigNodeReadString ( vols, & vol_list );
        if ( rc == 0 )
        {
            const bool protected = false;
            const bool disabled = false;
            const bool caching = true;
            rc = VResolverLoadAlgVolumes ( & self -> local, NULL, NULL,
                cache_capable,
                appREFSEQ, algREFSEQ,  & self -> num_app_vols [ appREFSEQ ],
                vol_list, protected, disabled, caching );
            StringWhack ( vol_list );
        }
        KConfigNodeRelease ( vols );
    }

    return rc;
}


/* ForceRemoteRefseq
 *  makes sure there is a remote source of refseq
 *  or else adds a hard-coded URL to NCBI
 */
static
rc_t VResolverForceRemoteRefseq ( VResolver *self )
{
    rc_t rc;
    bool found;
    String local_root;
    const String *root;

    uint32_t i, count = VectorLength ( & self -> remote );
    for ( found = false, i = 0; i < count; ++ i )
    {
        VResolverAlg *alg = ( VResolverAlg* ) VectorGet ( & self -> remote, i );
        if ( alg -> app_id == appREFSEQ )
        {
            found = true;
            if ( alg -> disabled )
                alg -> disabled = false;
        }
    }

    if ( found )
        return 0;

    if ( self -> num_app_vols [ appAny ] != 0 )
    {
        for ( i = 0; i < count; ++ i )
        {
            VResolverAlg *alg = ( VResolverAlg* ) VectorGet ( & self -> remote, i );
            if ( alg -> app_id == appAny )
            {
                found = true;
                if ( alg -> disabled )
                    alg -> disabled = false;
            }
        }
    }

    if ( found )
        return 0;

    /* create one from hard-coded constants */
    StringInitCString ( & local_root, "https://ftp-trace.ncbi.nlm.nih.gov/sra" );
    rc = StringCopy ( & root, & local_root );
    if ( rc == 0 )
    {
        rc = VectorAppend ( & self -> roots, NULL, root );
        if ( rc != 0 )
            StringWhack ( root );
        else
        {
            String vol_list;
            const bool protected = false;
            const bool disabled = false;
            const bool caching = false;
            StringInitCString ( & vol_list, "refseq" );
            rc = VResolverLoadAlgVolumes ( & self -> remote, root, NULL, false,
                appREFSEQ, algREFSEQ, & self -> num_app_vols [ appREFSEQ ],
                & vol_list, protected, disabled, caching );
        }
    }

    return rc;
}


/* GetDownloadTicket
 *  if we are within a working environment that has a download ticket,
 *  capture it here and add that local repository into the mix
 */
static
const String *VResolverGetDownloadTicket ( const VResolver *self,
    const KRepository *protected, char *buffer, size_t bsize )
{
    const String *ticket = NULL;
    if ( protected != NULL )
    {
        rc_t rc = KRepositoryName ( protected, buffer, bsize, NULL );
        if ( rc == 0 )
        {
            size_t ticsz;
            char ticbuf [ 256 ];
            rc = KRepositoryDownloadTicket ( protected, ticbuf, sizeof ticbuf, & ticsz );
            if ( rc == 0 )
            {
                String tic;
                StringInit ( & tic, ticbuf, ticsz, ( uint32_t ) ticsz );
                rc = StringCopy ( & ticket, & tic );
            }
        }
    }
    return ticket;
}


/* ForceRemoteProtected
 *  makes sure there is a remote CGI
 */
static
rc_t VResolverForceRemoteProtected ( VResolver *self )
{
    rc_t rc;
    const String *root;

    /* create one from hard-coded constants */
    String cgi_root;
    StringInitCString ( & cgi_root, SDL_CGI );

    assert(self);

    rc = StringCopy ( & root, & cgi_root );
    if ( rc == 0 )
    {
        rc = VectorAppend ( & self -> roots, NULL, root );
        if ( rc != 0 )
            StringWhack ( root );
        else
        {
            const bool protected = true;
            const bool disabled = false;

            VResolverAlg *cgi = NULL;
            rc = VResolverAlgMakeCgi( & cgi, root, protected, disabled,
                self->ticket, "SDL.2", &self->versions );
            if ( rc == 0 )
            {
                /* Remote Protected algorythm should come first: see VDB-2679 */
                if ( VectorLength ( & self -> remote ) > 0 ) {
                    void *prior = NULL;
                    rc = VectorSwap ( &self -> remote, 0, cgi, & prior );
                    if ( rc == 0 ) {
                        rc = VectorAppend ( &self -> remote, NULL, prior );
                    }
                }
                else {
                    rc = VectorAppend ( & self -> remote, NULL, cgi );
                }

                if ( rc == 0 )
                {
                    ++ self -> num_app_vols [ appAny ];
                    return 0;
                }
            }

            VResolverAlgWhack ( cgi, NULL );
        }
    }

    return rc;
}


/* Load
 *  load the respository from ( current ) KConfig
 *
 *  using pseudo BNF, it looks like this:
 *
 *    repositories
 *        = "repository" <category-block>* ;
 *
 *    category-block
 *        = <category> <sub-category-block>* ;
 *
 *    category
 *        = "remote" | "site" | "user" ;
 *
 *    sub-category-block
 *        = <sub-category> <repository-block>* ;
 *
 *    sub-category
 *        = "main" | "aux" | "protected"
 */
static
rc_t VResolverDetectSRALeafPath ( VResolver *self )
{
    /* capture working directory as "root" path */
    const KDirectory *wd = self -> wd;
    char cwd [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true, cwd, sizeof cwd, "." );
    if ( rc == 0 )
    {
        const String *root;

        /* convert C-string to real string */
        String cwd_str;
        StringInitCString ( & cwd_str, cwd );

        /* create a copy on heap */
        rc = StringCopy ( & root, & cwd_str );
        if ( rc == 0 )
        {
            /* insert into "roots" */
            rc = VectorAppend ( & self -> roots, NULL, root );
            if ( rc == 0 )
            {
                /* create an algorithm for any application where the
                   spec is to be treated as a leaf path */
                VResolverAlg *alg;
                rc = VResolverAlgMake ( & alg, root, appAny, algFlat, self -> ticket != NULL, false );
                if ( rc == 0 )
                {
                    const String *vol;

                    /* create a single volume - "." */
                    CONST_STRING ( & cwd_str, "." );
                    rc = StringCopy ( & vol, & cwd_str );
                    if ( rc == 0 )
                    {
                        rc = VectorAppend ( & alg -> vols, NULL, vol );
                        if ( rc != 0 )
                            free ( ( void* ) vol );
                        else
                        {
                            /* insert into local resolution path */
                            rc = VectorAppend ( & self -> local, NULL, alg );
                            if ( rc == 0 )
                                return 0;
                        }
                    }

                    VResolverAlgWhack ( alg, NULL );
                }
            }

            free ( ( void* ) root );
        }
    }
    return rc;
}

static
rc_t VResolverForceUserFilesVol ( VResolver *self, const VResolverAlg *sraAlg )
{
   /* create an algorithm for "file" application where the
       spec is to be treated as a leaf path */
    VResolverAlg *alg;
    rc_t rc = VResolverAlgMake ( & alg, sraAlg -> root, appFILE, algFlat, sraAlg -> protected, sraAlg -> disabled );
    if ( rc == 0 )
    {
        String vol_str;
        const String *vol;

        /* create a single volume - "files" */
        CONST_STRING ( & vol_str, "files" );
        rc = StringCopy ( & vol, & vol_str );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & alg -> vols, NULL, vol );
            if ( rc != 0 )
                free ( ( void* ) vol );
            else
            {
                /* copy sra settings */
                alg -> ticket = sraAlg -> ticket;
                alg -> cache_capable = sraAlg -> cache_capable;
                alg -> cache_enabled = sraAlg -> cache_enabled;

                /* insert into local resolution path */
                rc = VectorAppend ( & self -> local, NULL, alg );
                if ( rc == 0 )
                {
                    ++ self -> num_app_vols [ appFILE ];
                    return 0;
                }
            }
        }

        VResolverAlgWhack ( alg, NULL );
    }

    return rc;
}

static
rc_t VResolverForceUserFiles ( VResolver *self )
{
    rc_t rc;
    uint32_t i, count = VectorLength ( & self -> local );

    for ( rc = 0, i = 0; i < count; ++ i )
    {
        const VResolverAlg *alg = VectorGet ( & self -> local, i );
        if ( alg -> app_id == appSRA && alg -> cache_capable )
        {
            rc = VResolverForceUserFilesVol ( self, alg );
            if ( rc != 0 )
                break;
        }
    }

    return rc;
}

static EDisabled _KConfigNodeRepoDisabled(
    const KConfigNode *self, const char *name)
{
    EDisabled isDisabled = eDisabledNotSet;
    const KConfigNode *node = NULL;
    rc_t rc = KConfigNodeOpenNodeRead(self, &node, "%s/disabled", name);
    bool disabled = false;
    if (rc == 0) {
        rc = KConfigNodeReadBool(node, &disabled);
    }
    if (rc == 0) {
        isDisabled = disabled ? eDisabledSet : eEnabledSet;
    }
    KConfigNodeRelease(node);
    return isDisabled;
}

static rc_t VResolverLoad(VResolver *self, const KRepository *protectedRepo,
    const KConfig *cfg, const KNSManager *kns, const KNgcObj * ngc)
{
    bool have_remote_protected = false;

    const KConfigNode *kfg;
    rc_t rc = KConfigOpenNodeRead ( cfg, & kfg, "repository" );
    if ( GetRCState ( rc ) == rcNotFound )
        rc = 0;
    else if ( rc == 0 )
    {
        bool userCacheEnabled = true;

        EDisabled remoteDisabled = _KConfigNodeRepoDisabled(kfg, "remote");
        EDisabled siteDisabled = _KConfigNodeRepoDisabled(kfg, "site");
        EDisabled userDisabled = _KConfigNodeRepoDisabled(kfg, "user");

        /* check to see what the current directory is */
        char buffer [ 256 ] = "";
        self->ticket = NULL;
        if (protectedRepo != NULL)
            self -> ticket = VResolverGetDownloadTicket ( self, protectedRepo,
                buffer, sizeof buffer );
        else if (ngc != NULL) {
            char b[512] = "";
            rc = KNgcObjGetTicket(ngc, b, sizeof b, NULL);
            if (rc == 0) {
                String s;
                StringInitCString(&s, b);
                rc = StringCopy(&self->ticket, &s);
            }
            if (rc == 0) {
                uint32_t projectId = 0;
                rc = KNgcObjGetProjectId(ngc, &projectId);
                rc = string_printf(buffer, sizeof buffer, NULL,
                    "dbGaP-%d", projectId);
            }
        }

        if (rc == 0) {
            const KConfigNode * node = NULL;
            rc_t rc =
                KConfigNodeOpenNodeRead ( kfg, & node, "user/cache-disabled" );
            if ( rc == 0 ) {
                bool disabled = false;
                rc = KConfigNodeReadBool ( node, & disabled );
                KConfigNodeRelease ( node );
                if (rc == 0 && disabled)
                    userCacheEnabled = false;
            }
            rc = 0;
        }

        /* allow user to specify leaf paths in current directory */
        if (rc == 0)
            rc = VResolverDetectSRALeafPath ( self );

        /* if the user is inside of a protected workspace, load it now */
        if ( rc == 0 && self -> ticket != NULL )
        {
            rc = VResolverLoadProtected
                ( self, kfg, buffer, true, userDisabled, userCacheEnabled );
            if ( rc == 0 && self -> num_app_vols [ appFILE ] == 0 )
                rc = VResolverForceUserFiles ( self );
        }

        /* now load user public repositories */
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> local, kfg, NULL,
                "user/main", true, false, userDisabled, userCacheEnabled,
                false );
#if ALLOW_AUX_REPOSITORIES
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> local, kfg, NULL,
                "user/aux", true, false, userDisabled, userCacheEnabled,
                false );
#endif

        /* load Accession as Directory repository */
        if (rc == 0) {
            rc = VResolverLoadSubCategory(self, &self->ad, kfg, NULL,
                "user/ad", true, false, eDisabledNotSet, true, true);
        }
        /* TODO:
        Add ad to embedded configuration.
        Add ad to default.kfg */

        /* load any site repositories */
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> local, kfg, NULL,
                "site/main", false, false, siteDisabled, false, false );
#if ALLOW_AUX_REPOSITORIES
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> local, kfg, NULL,
                "site/aux", false, false, siteDisabled, false, false);
#endif

        /* if within a protected workspace, load protected remote repositories */
        if ( rc == 0 && self -> ticket != NULL )
        {
            if (kns == NULL) {
                rc = KNSManagerMake ( ( KNSManager** ) & self -> kns );
            }
            else {
                rc = KNSManagerAddRef(kns);
                if (rc == 0) {
                    self -> kns = kns;
                }
            }
            if ( rc == 0 )
            {
                uint32_t entry_vols = VectorLength ( & self -> remote );
                rc = VResolverLoadSubCategory ( self, & self -> remote, kfg,
                    self -> ticket, "remote/protected", false, true,
                    remoteDisabled, false, false );
                have_remote_protected = VectorLength ( & self -> remote ) > entry_vols;
            }
        }

        /* load any remote repositories */
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> remote, kfg, NULL,
                "remote/main", false, false, remoteDisabled, false, false );
#if ALLOW_AUX_REPOSITORIES
        if ( rc == 0 )
            rc = VResolverLoadSubCategory ( self, & self -> remote, kfg, NULL,
                "remote/aux", false, false, remoteDisabled, false, false );
#endif

        KConfigNodeRelease ( kfg );

        /* recover from public remote repositories using resolver CGI */
        if ( self -> kns == NULL )
        {
            if (kns == NULL) {
                rc = KNSManagerMake ( ( KNSManager** ) & self -> kns );
            }
            else {
                rc = KNSManagerAddRef(kns);
                if (rc == 0) {
                    self -> kns = kns;
                }
            }
        }
    }

    if ( rc == 0 && self -> num_app_vols [ appAny ] == 0 )
    {
        bool has_current_refseq = true;

        /* AT THIS POINT, a current configuration will have something.
           But, older out-of-date configurations may exist and need special handling. */
        if ( self -> num_app_vols [ appREFSEQ ] == 0 )
        {
            has_current_refseq = false;
            rc = VResolverLoadLegacyRefseq ( self, cfg, true );
        }

        /* now, one more special case - for external users
           who had legacy refseq configuration but nothing for SRA,
           force existence of a remote refseq access */
        if ( rc == 0
             && ! has_current_refseq
             && self -> num_app_vols [ appREFSEQ ] != 0
             && self -> num_app_vols [ appSRA ] == 0 )
        {
            rc = VResolverForceRemoteRefseq ( self );
        }
    }

    if ( rc == 0 && self -> num_app_vols [ appFILE ] == 0 )
        rc = VResolverForceUserFiles ( self );

    if ( rc == 0 && self -> ticket != NULL && ! have_remote_protected )
        rc = VResolverForceRemoteProtected ( self );

    if ( rc == 0 )
    {
        VectorReorder ( & self -> local, VResolverAlgSort, NULL );
        VectorReorder ( & self -> remote, VResolverAlgSort, NULL );
    }

    return rc;
}

LIB_EXPORT
rc_t CC VResolverProtocols ( VResolver * self, VRemoteProtocols protocols )
{
    VRemoteProtocols remote_protos;

    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcUpdating, rcSelf, rcNull );

    if ( protocols == eProtocolDefault )
        self -> protocols = self -> dflt_protocols;
    else
    {
        if ( protocols > VRemoteProtocolsMake3 ( eProtocolMax, eProtocolMax, eProtocolMax ) )
            return RC ( rcVFS, rcResolver, rcUpdating, rcParam, rcInvalid );

        for ( remote_protos = protocols; remote_protos != 0; remote_protos >>= 3 )
        {
            VRemoteProtocols proto = remote_protos & eProtocolMask;
            if ( proto == eProtocolNone || proto > eProtocolMax )
                return RC ( rcVFS, rcResolver, rcUpdating, rcParam, rcInvalid );
        }

        self -> protocols = protocols;
    }

    return 0;
}


rc_t VResolverGetProjectId ( const VResolver * self, uint32_t * projectId )
{
    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcAccessing, rcSelf, rcNull );
    else if ( projectId == NULL )
        return RC ( rcVFS, rcResolver, rcAccessing, rcParam, rcNull );
    else {
        bool has_project_id = self -> projectId != 0;

        * projectId = 0;

        if ( has_project_id ) {
            * projectId = self -> projectId;
        }

        return 0;
    }
}

LIB_EXPORT rc_t CC VResolverGetProject ( const VResolver * self,
                                         uint32_t * project )
{
    if ( project == NULL )
        return RC ( rcVFS, rcResolver, rcAccessing, rcParam, rcNull );

    * project = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcAccessing, rcSelf, rcNull );

    * project = self -> projectId;

    return 0;
}

static rc_t VResolverInitVersion(VResolver * self, const KConfig *kfg) {
    rc_t rc = 0;

    String * result = NULL;

    assert(self);

    rc = KConfigReadString(kfg, "/repository/remote/version", &result);

    if (rc == 0) {
        assert(result);

        self->version = string_dup_measure(result->addr, NULL);

        free(result);

        if (self->version == NULL)
            return RC(rcVFS, rcMgr, rcCreating, rcMemory, rcExhausted);
        else
            return 0;
    }

    else {
        if (self->ticket == NULL)
             /* default version for public data is SDL-2 ( 128(SDL) | 2 ) */
            self->version = string_dup_measure("130", NULL);
        else if (self->versions == 0)
            /* default version for protected data is 3.0 when calling names.cgi */
            self->version = string_dup_measure("3", NULL);
        else
            /* default version for protected data is SDL-2 when calling sdl */
            self->version = string_dup_measure("130", NULL);

        if (self->version == NULL)
            return RC(rcVFS, rcMgr, rcCreating, rcMemory, rcExhausted);
        else
            return 0;
    }
}

/* Make
 *  internal factory function
 */
static
rc_t VResolverMake ( VResolver ** objp, const KDirectory *wd,
    const KRepository *protected, const KConfig *kfg, const VFSManager *mgr,
    const KNgcObj * ngc )
{
    rc_t rc = 0;

    VResolver *obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL )
        rc = RC ( rcVFS, rcMgr, rcCreating, rcMemory, rcExhausted );
    else
    {
        KNSManager *kns = NULL;
        VectorInit ( & obj -> roots, 0, 8 );
        VectorInit ( & obj -> local, 0, 8 );
        VectorInit ( & obj -> remote, 0, 8 );
        obj -> wd = wd;

        KRefcountInit ( & obj -> refcount, 1, "VResolver", "make", "resolver" );

        if ( mgr != NULL )
        {
            rc_t rc = VFSManagerGetKNSMgr ( mgr, & kns );
            if ( rc != 0 )
            {
                rc = 0;
                kns = NULL;
            }
        }
        else {
            rc_t rc = KNSManagerMake ( & kns );
            if ( rc != 0 )
            {
                rc = 0;
                kns = NULL;
            }
        }

        /* set up protocols */
        obj -> dflt_protocols = DEFAULT_PROTOCOLS;
        if ( kfg != NULL )
            KConfigReadRemoteProtocols ( kfg, & obj -> dflt_protocols );

        obj -> protocols = obj -> dflt_protocols;

        rc = VResolverLoad ( obj, protected, kfg, kns, ngc );

        if (obj->kns == NULL)
            obj->kns = kns;
        else
            RELEASE(KNSManager, kns);

        KRepositoryProjectId ( protected, & obj -> projectId );

        if (rc == 0)
            rc = VResolverInitVersion(obj, kfg);

        obj -> resoveOidName = DEFAULT_RESOVE_OID_NAME; /* just in case */

        if ( rc == 0 )
        {
            * objp = obj;
            return 0;
        }

        VResolverWhack ( obj );
    }

    return rc;
}

rc_t VResolverSetVersion ( VResolver *self, const char * version ) {
    if ( self == NULL )
        return RC ( rcVFS, rcResolver, rcUpdating, rcSelf, rcNull );
    if ( self == NULL || version == NULL )
        return RC ( rcVFS, rcResolver, rcUpdating, rcParam, rcNull );

    free ( self -> version );
    self -> version = string_dup_measure ( version, NULL );
    return 0;
}

/* Make
 *  ask the VFS manager or repository to make a resolver
 */
LIB_EXPORT
rc_t CC VFSManagerMakeResolver ( const VFSManager * self,
    VResolver ** new_resolver, const KConfig * cfg )
{
    rc_t rc;

    if ( new_resolver == NULL )
        rc = RC ( rcVFS, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcCreating, rcSelf, rcNull );
        else if ( cfg == NULL )
            rc = RC ( rcVFS, rcMgr, rcCreating, rcParam, rcNull );
        else
        {
            KDirectory *wd;
            rc = VFSManagerGetCWD ( self, & wd );
            if ( rc == 0 )
            {
                const KRepositoryMgr *rmgr;
                rc = KConfigMakeRepositoryMgrRead ( cfg, & rmgr );
                if ( rc == 0 )
                {
                    const KRepository *protected = NULL;
                    rc = KRepositoryMgrCurrentProtectedRepository ( rmgr, & protected );
                    if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
                    {
                        rc = VResolverMake(new_resolver,
                            wd, protected, cfg, self, NULL);
                        KRepositoryRelease ( protected );

                        if ( rc == 0 )
                        {
                            KRepositoryMgrRelease ( rmgr );
                            return 0;
                        }
                    }

                    KRepositoryMgrRelease ( rmgr );
                }

                KDirectoryRelease ( wd );
            }
        }

        *new_resolver = NULL;
    }

    return rc;
}

/*rc_t VFSManagerMakeDbgapResolver(const VFSManager * self,
    VResolver ** new_resolver,
    const KConfig * cfg, const struct KNgcObj * ngc)
{
    rc_t rc = 0;

    if (new_resolver == NULL)
        rc = RC(rcVFS, rcMgr, rcCreating, rcParam, rcNull);
    else {
        if (self == NULL)
            rc = RC(rcVFS, rcMgr, rcCreating, rcSelf, rcNull);
        else if (cfg == NULL)
            rc = RC(rcVFS, rcMgr, rcCreating, rcParam, rcNull);
        else {
            KDirectory *wd = NULL;
            rc = VFSManagerGetCWD(self, &wd);

            if (rc == 0) {
                if (rc == 0)
                    rc = VResolverMake(new_resolver, wd, NULL, cfg, NULL, ngc);

                if (rc == 0) {
                    uint32_t projectId = 0;
                    rc = KNgcObjGetProjectId(ngc, &projectId);

                    assert(*new_resolver);
                    (*new_resolver)->projectId = projectId;

                    return rc;
                }

                KDirectoryRelease(wd);
            }
        }

        *new_resolver = NULL;
    }

    return rc;
}*/

LIB_EXPORT
rc_t CC KRepositoryMakeResolver ( const KRepository *self,
    VResolver ** new_resolver, const KConfig * cfg )
{
    rc_t rc;

    if ( new_resolver == NULL )
        rc = RC ( rcVFS, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVFS, rcMgr, rcCreating, rcSelf, rcNull );
        else if ( cfg == NULL )
            rc = RC ( rcVFS, rcMgr, rcCreating, rcParam, rcNull );
        else
        {
            KDirectory *wd;
            rc = KDirectoryNativeDir ( & wd );
            if ( rc == 0 )
            {
                rc = VResolverMake ( new_resolver, wd, self, cfg, NULL, NULL );
                if ( rc == 0 )
                    return 0;

                KDirectoryRelease ( wd );
            }
        }

        *new_resolver = NULL;
    }

    return rc;
}
