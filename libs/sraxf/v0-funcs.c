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
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include <endian.h>
#include <byteswap.h>

#ifdef HTONL
#undef HTONL
#endif

#ifdef HTONS
#undef HTONS
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define HTONL( x ) bswap_32 ( x )
#define HTONS( x ) bswap_16 ( x )
#else
#define HTONL( x ) ( x )
#define HTONS( x ) ( x )
#endif

#include "v0-decompress.h"

static
rc_t CC sra_read_decompress_454 ( void *self, const VXformInfo *info,
    VLegacyBlobResult *rslt, const KDataBuffer *src )
{
    rc_t rc;
    KDataBuffer *dst = rslt -> dst;
    const uint8_t *p = src -> base;

    uint32_t num_bases = HTONS ( * ( const uint16_t* ) & p [ 1 ] );
    uint32_t hdr_size = 3;

    /* make sub-buffer from input */
    KDataBuffer tmp;
    rc = KDataBufferCast ( src, & tmp, 8, false );
    if ( rc == 0 )
    {
        KDataBufferWhack ( dst );
        rc = KDataBufferSub ( & tmp, dst, hdr_size, UINT64_MAX );
        if ( rc == 0 )
        {
            /* cast to 2na */
            rc = KDataBufferCast ( dst, dst, 2, true );
            if ( rc == 0 )
            {
                /* resize to actual number of bases */
                assert ( num_bases <= dst -> elem_count );
                rc = KDataBufferResize ( dst, num_bases );
            }
        }

        KDataBufferWhack ( & tmp );
    }

    return rc;
}

static
rc_t CC sra_read_decompress_slx ( void *self, const VXformInfo *info,
    VLegacyBlobResult *rslt, const KDataBuffer *src )
{
    rc_t rc;
    KDataBuffer tmp;
    uint32_t hdr_size;
    KDataBuffer *dst = rslt -> dst;
    const uint8_t *p = src -> base;

    uint32_t num_bases = HTONL ( * ( const uint32_t* ) & p [ 1 ] );
    uint32_t row_len = HTONS ( * ( const uint16_t* ) & p [ 5 ] );
    assert ( row_len != 0 );
    assert ( num_bases % row_len == 0 );
    hdr_size = 7;

    /* make sub-buffer from input */
    rc = KDataBufferCast ( src, & tmp, 8, false );
    if ( rc == 0 )
    {
        KDataBufferWhack ( dst );
        rc = KDataBufferSub ( & tmp, dst, hdr_size, UINT64_MAX );
        if ( rc == 0 )
        {
            /* cast to 2na */
            rc = KDataBufferCast ( dst, dst, 2, true );
            if ( rc == 0 )
            {
                /* resize to actual number of bases */
                assert ( num_bases <= dst -> elem_count );
                rc = KDataBufferResize ( dst, num_bases );
            }
        }

        KDataBufferWhack ( & tmp );
    }

    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_READ, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VNoHdrBlobFunc f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    switch ( cp -> argv [ 0 ] . data . u8 [ 0 ] )
    {
    case SRA_PLATFORM_454:
        f = sra_read_decompress_454;
        break;
    case SRA_PLATFORM_ILLUMINA:
        f = sra_read_decompress_slx;
        break;
    default:
        return RC ( rcSRA, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    /* DO NOT CHANGE ASSIGNMENT FROM "f"
       we want the compiler to catch type mismatches */
    rslt -> u . bf = ( VBlobFunc ) f;

    return 0;
}

/* all of the functions have this prototype */
typedef int ( * legacy_decompress_func ) ( KDataBuffer*, KDataBuffer*, const void*, int );

/* function ids */
enum
{
    sra_prb_454,
    sra_prb_slx,
    sra_sig_454,
    sra_sig_slx,
    sra_int,
    sra_nse,
    sra_clip,
    sra_pos
};

static
legacy_decompress_func funcs [] =
{
    sra_decompress_prb_454,
    sra_decompress_prb_slx,
    sra_decompress_sig_454,
    sra_decompress_sig_slx,
    sra_decompress_int,
    sra_decompress_nse,
    sra_decompress_clp,
    sra_decompress_pos
};

static
rc_t CC sra_legacy_decompress ( void *self, const VXformInfo *info,
    VLegacyBlobResult *rslt, const KDataBuffer *src )
{
    int failed;
    KDataBuffer *dst = rslt -> dst;

    /* an initially empty buffer */
    KDataBuffer alt;
    KDataBufferMakeBytes ( & alt, 0 );

    assert(KDataBufferBytes ( src ) < INT_MAX);
    /* invoke the decompression function */
    failed = funcs [ ( size_t ) self ] ( dst, & alt, src -> base, (int)KDataBufferBytes ( src ) );

    /* whack any double buffer */
    KDataBufferWhack ( & alt );

    /* return non-descript status */
    if ( failed )
        return RC ( rcSRA, rcBlob, rcUnpacking, rcNoObj, rcUnknown );
    return 0;
}

static
rc_t sra_legacy_init ( void )
{
    static bool done;
    if ( ! done )
    {
        sra_decompress_init ();
        done = true;
    }

    return 0;
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_QUALITY, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    switch ( cp -> argv [ 0 ] . data . u8 [ 0 ] )
    {
    case SRA_PLATFORM_454:
        rslt -> self = ( void* ) sra_prb_454;
        break;
    case SRA_PLATFORM_ILLUMINA:
        rslt -> self = ( void* ) sra_prb_slx;
        break;
    default:
        return RC ( rcSRA, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    return sra_legacy_init ();
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_SIGNAL, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    switch ( cp -> argv [ 0 ] . data . u8 [ 0 ] )
    {
    case SRA_PLATFORM_454:
        rslt -> self = ( void* ) sra_sig_454;
        break;
    case SRA_PLATFORM_ILLUMINA:
        rslt -> self = ( void* ) sra_sig_slx;
        break;
    default:
        return RC ( rcSRA, rcFunction, rcConstructing, rcParam, rcInvalid );
    }

    return sra_legacy_init ();
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_INTENSITY, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    rslt -> self = ( void* ) sra_int;

    return sra_legacy_init ();
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_NOISE, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    rslt -> self = ( void* ) sra_nse;

    return sra_legacy_init ();
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_CLIP, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    rslt -> self = ( void* ) sra_clip;

    return sra_legacy_init ();
}

VTRANSFACT_IMPL ( NCBI_SRA_decode_POSITION, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* DO NOT REMOVE THIS ASSIGNMENT
       we want the compiler to catch type mismatches */
    VNoHdrBlobFunc f = sra_legacy_decompress;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    /* the discriminator */
    rslt -> self = ( void* ) sra_pos;

    return sra_legacy_init ();
}
