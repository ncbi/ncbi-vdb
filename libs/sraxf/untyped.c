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
#include <vdb/vdb-priv.h>
#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static
bool KMetadataStrEqual ( const KMetadata *meta, const char *path, const char *str )
{
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "%s", path );
    if ( rc == 0 )
    {
        size_t num_read;
        char buff [ 4096 ];
        rc = KMDataNodeReadCString ( node, buff, sizeof buff, & num_read );
        KMDataNodeRelease ( node );
        if ( rc == 0 )
        {
            if ( strcmp ( buff, str ) == 0 )
                return true;
        }
    }

    return false;
}

static
bool KMetadataNumEqual ( const KMetadata *meta, const char *path, int64_t num )
{
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "%s", path );
    if ( rc == 0 )
    {
        int64_t i;
        rc = KMDataNodeReadAsI64 ( node, & i );
        KMDataNodeRelease ( node );
        if ( rc == 0 )
        {
            if ( i == num )
                return true;
        }
    }

    return false;
}

static
bool KMetadataVersEqual ( const KMetadata *meta, const char *name, const char *vers )
{
    bool equal = false;
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "SOFTWARE/loader" );
    if ( rc == 0 )
    {
        size_t num_read;
        char attr [ 256 ];
        rc = KMDataNodeReadAttr ( node, "name", attr, sizeof attr, & num_read );
        if ( rc == 0 )
        {
            if ( memcmp ( attr, name, strlen ( name ) ) == 0 )
            {
                if ( vers == NULL || vers [ 0 ] == 0 )
                    equal = true;
                else
                {
                    rc = KMDataNodeReadAttr ( node, "vers", attr, sizeof attr, & num_read );
                    if ( rc == 0 )
                    {
                        if ( memcmp ( attr, vers, strlen ( vers ) ) == 0 )
                            equal = true;
                    }
                }
            }
        }

        KMDataNodeRelease ( node );
    }

    return equal;
}

static
bool KMetadataExists ( const KMetadata *meta, const char *path )
{
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "%s", path );
    if ( rc != 0 )
        return false;
    KMDataNodeRelease ( node );
    return true;
}

static
bool KMetadataEmpty ( const KMetadata *meta, const char *path )
{
    size_t num_read, remaining;
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( meta, & node, "%s", path );
    if ( rc != 0 )
        return true;
    rc = KMDataNodeRead ( node, 0, & num_read, 0, & num_read, & remaining );
    KMDataNodeRelease ( node );
    if ( rc == 0 && remaining != 0 )
        return false;
    return true;
}

static
bool KColumnTypeEqual ( const KTable *tbl, const char *col, const char *type )
{
    /* TBD - this operation is expensive
       should be addressed either by caching opened columns on table
       or by introducing a path to open column metadata from table */
    const KColumn *kcol;
    rc_t rc = KTableOpenColumnRead ( tbl, & kcol, "%s", col );
    if ( rc == 0 )
    {
        const KMetadata *meta;
        rc = KColumnOpenMetadataRead ( kcol, & meta );
        KColumnRelease ( kcol );
        if ( rc == 0 )
        {
            /* this is a expected to be a v1 column
               open its decoding node */
            const KMDataNode *node;
            rc = KMetadataOpenNodeRead ( meta, & node, "decoding" );
            KMetadataRelease ( meta );
            if ( rc == 0 )
            {
                /* read its type */
                size_t size;
                char type_expr [ 256 ];
                rc = KMDataNodeReadAttr ( node, "type",
                    type_expr, sizeof type_expr, & size );
                KMDataNodeRelease ( node );
                if ( rc == 0 )
                {
                    if ( memcmp ( type_expr, type, strlen ( type ) ) == 0 )
                        return true;
                }
            }
        }
    }
    return false;
}


/* accept_untyped
 *  recognizes any table at all
 *  used by admin to correct corrupt tables
 */
MOD_EXPORT
bool CC NCBI_SRA_accept_untyped ( const KTable *tbl, const KMetadata *meta )
{
    return true;
}


/* 454_untyped_0
 *  recognizes all runs produced with v0 loader
 */
MOD_EXPORT
bool CC NCBI_SRA__454__untyped_0 ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "454" ) )
    {
        if ( ! KMetadataExists ( meta, "SOFTWARE" ) )
        {
            if ( KMetadataExists ( meta, "MSC454_FLOW_CHARS" ) &&
                 KMetadataExists ( meta, "MSC454_KEY_SEQUENCE" ) )
            {
                /* don't bother looking at columns */
                return true;
            }
        }
    }
    return false;
}


/* 454_untyped_1_2a
 *  recognizes runs produced with v1.2 loader
 *  where the linker is present
 *  and a physical READ_SEG is present
 */
MOD_EXPORT
bool CC NCBI_SRA__454__untyped_1_2a ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataVersEqual ( meta, "sff-load", "1" ) ||
         KMetadataVersEqual ( meta, "sff-load", "0" ) )
    {
        if ( KMetadataExists ( meta, "col/NREADS" ) )
        {
            if ( ! KMetadataEmpty ( meta, "col/LINKER_SEQUENCE/row" ) )
                return KTableExists ( tbl, kptColumn, "READ_SEG" );
        }
    }
    return false;
}


/* 454_untyped_1_2b
 *  recognizes runs produced with v1.2 loader
 *  where the linker is missing or empty
 *  and a physical READ_SEG is present
 */
MOD_EXPORT
bool CC NCBI_SRA__454__untyped_1_2b ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataVersEqual ( meta, "sff-load", "1" ) ||
         KMetadataVersEqual ( meta, "sff-load", "0" ) )
    {
        if ( KMetadataExists ( meta, "col/NREADS" ) )
        {
            if ( KMetadataEmpty ( meta, "col/LINKER_SEQUENCE/row" ) )
                return KTableExists ( tbl, kptColumn, "READ_SEG" );
        }
    }
    return false;
}


/* Illumina_untyped_0a
 *  recognizes runs produced with v0 loader with 4-channel QUALITY
 */
MOD_EXPORT
bool CC NCBI_SRA_Illumina_untyped_0a ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "SOLEXA" ) )
    {
        if ( ! KMetadataExists ( meta, "SOFTWARE" ) )
        {
            if ( KMetadataNumEqual ( meta, "NUMBER_PRB_CHANNELS", 4 ) ||
                 KMetadataNumEqual ( meta, "NUMBER_PRB_CHANNELS_1", 4 ) )
            {
                return true;
            }
        }
    }
    return false;
}


/* Illumina_untyped_0b
 *  recognizes runs produced with v0 loader with single-channel QUALITY or QUALITY2
 */
MOD_EXPORT
bool CC NCBI_SRA_Illumina_untyped_0b ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "SOLEXA" ) )
    {
        if ( ! KMetadataExists ( meta, "SOFTWARE" ) )
        {
            if ( KMetadataNumEqual ( meta, "NUMBER_PRB_CHANNELS", 1 ) ||
                 KMetadataNumEqual ( meta, "NUMBER_PRB_CHANNELS_1", 1 ) ||
                 KMetadataNumEqual ( meta, "NUMBER_PRB_CHANNELS_2", 1 ) )
            {
                return true;
            }
        }
    }
    return false;
}


/* Illumina_untyped_1a
 *  recognizes runs produced with v1 srf and native loaders
 *  having 4-channel qualities
 */
MOD_EXPORT
bool CC NCBI_SRA_Illumina_untyped_1a ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "ILLUMINA" ) )
    {
        if ( KMetadataVersEqual ( meta, "illumina-load", "1" ) ||
             KMetadataVersEqual ( meta, "srf-load", "1" ) )
        {
            return KColumnTypeEqual ( tbl, "QUALITY", "NCBI:SRA:swapped_qual4" );
        }
    }
    return false;
}


/* Illumina_untyped_1b
 *  recognizes runs produced with all v1 loaders
 *  having single-channel qualities
 */
MOD_EXPORT
bool CC NCBI_SRA_Illumina_untyped_1b ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "ILLUMINA" ) )
    {
        if ( KMetadataVersEqual ( meta, "fastq-load", "1" ) ||
             KMetadataVersEqual ( meta, "fastq-load", "0" ) )
            return true;
        if ( KMetadataVersEqual ( meta, "illumina-load", "1" ) ||
             KMetadataVersEqual ( meta, "srf-load", "1" ) )
        {
            return ! KColumnTypeEqual ( tbl, "QUALITY", "NCBI:SRA:swapped_qual4" );
        }
    }
    return false;
}


/* ABI_untyped_1
 *  recognizes runs produced with all v1 loaders
 */
MOD_EXPORT
bool CC NCBI_SRA_ABI_untyped_1 ( const KTable *tbl, const KMetadata *meta )
{
    if ( KMetadataStrEqual ( meta, "PLATFORM", "ABSOLID" ) )
    {
        if ( ! KMetadataExists ( meta, "SOFTWARE" ) ||
             KMetadataVersEqual ( meta, "abi-load", "1" ) ||
             KMetadataVersEqual ( meta, "srf-load", "1" ) )
        {
            return true;
        }
    }
    return false;
}


/* this is here simply to force the compiler to check
   the function signatures listed above against a
   typedef for the function used by vdb. */
#if _DEBUGGING
VUntypedFunc test_signature [] =
{
    NCBI_SRA__454__untyped_0,
    NCBI_SRA__454__untyped_1_2a,
    NCBI_SRA__454__untyped_1_2b,
    NCBI_SRA_Illumina_untyped_0a,
    NCBI_SRA_Illumina_untyped_0b,
    NCBI_SRA_Illumina_untyped_1a,
    NCBI_SRA_Illumina_untyped_1b,
    NCBI_SRA_ABI_untyped_1
};
#endif
