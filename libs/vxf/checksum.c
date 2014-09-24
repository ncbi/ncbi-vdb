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
#include <vdb/xform.h>
#include <vdb/table.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/checksum.h>
#include <klib/text.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <bitstr.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct CRC32State CRC32State;
struct CRC32State
{
    uint64_t len;
    uint32_t crc;
};

typedef struct checksum checksum;
struct checksum
{
    KMetadata *meta;

    void ( * swap ) ( checksum *self );
    void ( * sum ) ( checksum *self, const void *data, size_t bytes );
    size_t ( * digest ) ( checksum *self, uint8_t *buff, size_t bsize );

    size_t valid;
    union
    {
        CRC32State crc32;
        MD5State md5;
        SHA1State sha1;
        SHA256State sha256;
        SHA384State sha384;
        SHA512State sha512;
        uint8_t buff [ 1 ];
    } u;

    bool byte_swap;
    char node [ 1 ];
};

/*--------------------------------------------------------------------------
 * CRC32
 */
static
void vdb_crc32swap ( checksum *self )
{
    self -> u . crc32 . len = bswap_64 ( self -> u . crc32 . len );
    self -> u . crc32 . crc = bswap_32 ( self -> u . crc32 . crc );
}

static
void vdb_crc32sum ( checksum *self, const void *data, size_t bytes )
{
    self -> u . crc32 . crc = CRC32 ( self -> u . crc32 . crc, data, bytes );
    self -> u . crc32 . len += bytes;
}

static
size_t vdb_crc32digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    uint64_t len = self -> u . crc32 . len;
    uint8_t ch[sizeof(len)];
    unsigned i;

    assert ( bsize >= 4 );

    for (i = 0; len; len >>= 8, ++i)
        ch[i] = (uint8_t)len;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    * ( uint32_t* ) buff = bswap_32 ( ~ CRC32 ( self -> u . crc32 . crc, ch, i ) );
#else
    * ( uint32_t* ) buff =  ~ CRC32 ( self -> u . crc32 . crc, ch, i );
#endif
    return 4;
}

/*--------------------------------------------------------------------------
 * md5
 */
static
void vdb_md5swap ( checksum *self )
{
    self -> u . md5 . count [ 0 ] = bswap_32 ( self -> u . md5 . count [ 0 ] );
    self -> u . md5 . count [ 1 ] = bswap_32 ( self -> u . md5 . count [ 1 ] );
    self -> u . md5 . abcd [ 0 ] = bswap_32 ( self -> u . md5 . abcd [ 0 ] );
    self -> u . md5 . abcd [ 1 ] = bswap_32 ( self -> u . md5 . abcd [ 1 ] );
    self -> u . md5 . abcd [ 2 ] = bswap_32 ( self -> u . md5 . abcd [ 2 ] );
    self -> u . md5 . abcd [ 3 ] = bswap_32 ( self -> u . md5 . abcd [ 3 ] );
}

static
void vdb_md5sum ( checksum *self, const void *data, size_t bytes )
{
    MD5StateAppend ( & self -> u . md5, data, bytes );
}

static
size_t vdb_md5digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    assert ( bsize >= 16 );
    MD5StateFinish ( & self -> u . md5, buff );
    return 16;
}

/*--------------------------------------------------------------------------
 * sha1
 */
static
void vdb_sha32swap ( checksum *self )
{
    unsigned int i;
    self -> u . sha1 . len = bswap_64 ( self -> u . sha1 . len );
    for ( i = 0; i < 8; ++ i )
        self -> u . sha1 . H [ i ] = bswap_32 ( self -> u . sha1 . H [ i ] );
    self -> u . sha1 . cur = bswap_32 ( self -> u . sha1 . cur );
}

static
void vdb_sha1sum ( checksum *self, const void *data, size_t bytes )
{
    SHA1StateAppend ( & self -> u . sha1, data, bytes );
}

static
size_t vdb_sha1digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    assert ( bsize >= 20 );
    SHA1StateFinish ( & self -> u . sha1, buff );
    return 20;
}

/*--------------------------------------------------------------------------
 * sha256
 */
static
void vdb_sha256sum ( checksum *self, const void *data, size_t bytes )
{
    SHA256StateAppend ( & self -> u . sha256, data, bytes );
}

static
size_t vdb_sha256digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    assert ( bsize >= 32 );
    SHA256StateFinish ( & self -> u . sha256, buff );
    return 32;
}

/*--------------------------------------------------------------------------
 * sha384
 */
static
void vdb_sha64swap ( checksum *self )
{
    unsigned int i;
    self -> u . sha384 . len = bswap_64 ( self -> u . sha384 . len );
    for ( i = 0; i < 8; ++ i )
        self -> u . sha384 . H [ i ] = bswap_64 ( self -> u . sha384 . H [ i ] );
    self -> u . sha384 . cur = bswap_32 ( self -> u . sha384 . cur );
}

static
void vdb_sha384sum ( checksum *self, const void *data, size_t bytes )
{
    SHA384StateAppend ( & self -> u . sha384, data, bytes );
}

static
size_t vdb_sha384digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    assert ( bsize >= 48 );
    SHA384StateFinish ( & self -> u . sha384, buff );
    return 48;
}

/*--------------------------------------------------------------------------
 * sha512
 */
static
void vdb_sha512sum ( checksum *self, const void *data, size_t bytes )
{
    SHA512StateAppend ( & self -> u . sha512, data, bytes );
}

static
size_t vdb_sha512digest ( checksum *self, uint8_t *buff, size_t bsize )
{
    assert ( bsize >= 64 );
    SHA512StateFinish ( & self -> u . sha512, buff );
    return 64;
}


/*--------------------------------------------------------------------------
 * general checksum function
 */
static
rc_t CC checksum_driver ( void *obj, const VXformInfo *info,
    int64_t row_id, VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    checksum *self = obj;

    /* we ensured the input was an octet-stream within the factory */
    uint32_t elem_bytes = argv [ 0 ] . u . data . elem_bits >> 3;
    const uint8_t *src = argv [ 0 ] . u . data . base;
    uint64_t count = argv [ 0 ] . u . data . elem_count;

    src += argv [ 0 ] . u . data . first_elem * elem_bytes;
    count *= elem_bytes;

    ( * self -> sum ) ( self, src, count );

    return 0;
}

static
void CC checksum_whack ( void *obj )
{
    checksum *self = obj;

    /* store metadata */
    KMDataNode *digest;
    rc_t rc = KMetadataOpenNodeUpdate ( self -> meta, & digest, "%s", self -> node );
    if ( rc == 0 )
    {
        size_t bytes;
        uint8_t buff [ 256 ];

        /* open state node */
        KMDataNode *state;
        rc = KMDataNodeOpenNodeUpdate ( digest, & state, "state" );
        if ( rc == 0 )
        {
            if ( self -> byte_swap )
                ( * self -> swap ) ( self );

            KMDataNodeWrite ( state, & self -> u, self -> valid );
            KMDataNodeRelease ( state );

            if ( self -> byte_swap )
                ( * self -> swap ) ( self );
       }

        /* capture digest */
        bytes = ( * self -> digest ) ( self, buff, sizeof buff );

        /* write digest */
        KMDataNodeWrite ( digest, buff, bytes );
        KMDataNodeRelease ( digest );
    }

    KMetadataRelease ( self -> meta );
    free ( self );
}

static
rc_t checksum_prior_state ( checksum *self, size_t state_size )
{
    const KMDataNode *state;
    rc_t rc = KMetadataOpenNodeRead ( self -> meta, & state, "%s/state", self -> node );
    if ( rc == 0 )
    {
        size_t remaining;
        rc = KMDataNodeRead ( state, 0, self -> u . buff, sizeof self -> u, & self -> valid, & remaining );
        KMDataNodeRelease ( state );
        if ( rc == 0 )
        {
            if ( remaining != 0 || self -> valid > state_size )
                rc = RC ( rcXF, rcFunction, rcConstructing, rcData, rcExcessive );
            else if ( self -> valid != 0 && self -> valid < state_size )
                rc = RC ( rcXF, rcFunction, rcConstructing, rcData, rcInsufficient );
        }
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}

static
rc_t checksum_make ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, size_t state_size )
{
    rc_t rc;
    checksum *self = malloc ( sizeof * self + cp -> argv [ 0 ] . count );
    if ( self == NULL )
        rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = VTableOpenMetadataUpdate ( ( VTable* ) info -> tbl, & self -> meta );
        if ( rc == 0 )
        {
            rc = KMetadataByteOrder ( self -> meta, & self -> byte_swap );
            if ( rc == 0 )
            {
                string_copy ( self -> node, cp -> argv [ 0 ] . count + 1,
                    cp -> argv [ 0 ] . data . ascii, cp -> argv [ 0 ] . count );

                memset ( & self -> u, 0, sizeof self -> u );
                rc = checksum_prior_state ( self, state_size );
                if ( rc == 0 )
                {
                    rslt -> self = self;
                    rslt -> whack = checksum_whack;
                    rslt -> variant = vftNonDetRow;
                    rslt -> u . rf = checksum_driver;

                    return 0;
                }
            }

            KMetadataRelease ( self -> meta );
        }

        free ( self );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * crc32 factory
 */
static
rc_t vdb_crc32_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . crc32 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_crc32swap;
        self -> sum = vdb_crc32sum;
        self -> digest = vdb_crc32digest;
        if ( self -> valid == 0 ) {
            self -> u . crc32 . crc = 0;
            self -> u . crc32 . len = 0;
            self -> valid = sizeof ( self -> u . crc32 );
        }
        else if ( self -> byte_swap )
            vdb_crc32swap ( self );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * md5 factory
 */
static
rc_t vdb_md5sum_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . md5 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_md5swap;
        self -> sum = vdb_md5sum;
        self -> digest = vdb_md5digest;
        if ( self -> valid == 0 ) {
            MD5StateInit ( & self -> u . md5 );
            self -> valid = sizeof ( self -> u . md5 );
        }
        else if ( self -> byte_swap )
            vdb_md5swap ( self );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * sha factories
 */
static
rc_t vdb_sha1_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . sha1 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_sha32swap;
        self -> sum = vdb_sha1sum;
        self -> digest = vdb_sha1digest;
        if ( self -> valid == 0 ) {
            SHA1StateInit ( & self -> u . sha1 );
            self -> valid = sizeof ( self -> u . sha1 );
        }
        else if ( self -> byte_swap )
            vdb_sha32swap ( self );
    }

    return rc;
}

static
rc_t vdb_sha256_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . sha256 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_sha32swap;
        self -> sum = vdb_sha256sum;
        self -> digest = vdb_sha256digest;
        if ( self -> valid == 0 ) {
            SHA256StateInit ( & self -> u . sha256 );
            self -> valid = sizeof ( self -> u . sha256 );
        }
        else if ( self -> byte_swap )
            vdb_sha32swap ( self );
    }

    return rc;
}

static
rc_t vdb_sha384_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . sha384 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_sha64swap;
        self -> sum = vdb_sha384sum;
        self -> digest = vdb_sha384digest;
        if ( self -> valid == 0 ) {
            SHA384StateInit ( & self -> u . sha384 );
            self -> valid = sizeof ( self -> u . sha384 );
        }
        else if ( self -> byte_swap )
            vdb_sha64swap ( self );
    }

    return rc;
}

static
rc_t vdb_sha512_fact ( const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp )
{
    checksum *self;
    rc_t rc = checksum_make ( info, rslt, cp, sizeof self -> u . sha512 );
    if ( rc == 0 )
    {
        self = rslt -> self;
        self -> swap = vdb_sha64swap;
        self -> sum = vdb_sha512sum;
        self -> digest = vdb_sha512digest;
        if ( self -> valid == 0 ) {
            SHA512StateInit ( & self -> u . sha512 );
            self -> valid = sizeof ( self -> u . sha512 );
        }
        else if ( self -> byte_swap )
            vdb_sha64swap ( self );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * checksum factory
 */
VTRANSFACT_IMPL ( vdb_checksum, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* version 1.0 has 2 factory params... */
    assert ( cp -> argc == 2 );
    /* ...and 1 function param */
    assert ( dp -> argc == 1 );

    /* both factory params are of type ascii */
    assert ( cp -> argv [ 0 ] . desc . domain == vtdAscii );
    assert ( cp -> argv [ 1 ] . desc . domain == vtdAscii );
    assert ( cp -> argv [ 0 ] . desc . intrinsic_bits == 8 );
    assert ( cp -> argv [ 1 ] . desc . intrinsic_bits == 8 );
    assert ( cp -> argv [ 0 ] . desc . intrinsic_dim == 1 );
    assert ( cp -> argv [ 1 ] . desc . intrinsic_dim == 1 );

    /* the input must be limited to bytes */
    assert ( dp -> argc == 1 );
    assert ( dp -> argv [ 0 ] . desc . intrinsic_bits == 8 );

    /* look up algorithm */
    switch ( cp -> argv [ 1 ] . count )
    {
    case 3:
        if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 3, "md5", 3, 3 ) == 0 )
            return vdb_md5sum_fact ( info, rslt, cp );
        break;
    case 4:
        if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 4, "sha1", 4, 4 ) == 0 )
            return vdb_sha1_fact ( info, rslt, cp );
        break;
    case 5:
        if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 5, "crc32", 5, 5 ) == 0 )
            return vdb_crc32_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 5, "sha-1", 5, 5 ) == 0 )
            return vdb_sha1_fact ( info, rslt, cp );
        break;
    case 6:
        if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 6, "crc-32", 6, 6 ) == 0 )
            return vdb_crc32_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 6, "sha256", 6, 6 ) == 0 )
            return vdb_sha256_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 6, "sha384", 6, 6 ) == 0 )
            return vdb_sha384_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 6, "sha512", 6, 6 ) == 0 )
            return vdb_sha512_fact ( info, rslt, cp );
        break;
    case 7:
        if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 7, "sha-256", 7, 7 ) == 0 )
            return vdb_sha256_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 7, "sha-384", 7, 7 ) == 0 )
            return vdb_sha384_fact ( info, rslt, cp );
        else if ( strcase_cmp ( cp -> argv [ 1 ] . data . ascii, 7, "sha-512", 7, 7 ) == 0 )
            return vdb_sha512_fact ( info, rslt, cp );
        break;
    }

    /* failed to resolve algorithm */
    return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
}
