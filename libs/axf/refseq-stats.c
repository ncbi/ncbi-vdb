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
#include <vdb/table.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <klib/checksum.h>
#include <kdb/meta.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef struct refseq_meta_stats_data_struct {
    uint64_t total_seq_len;
    uint32_t crc32;
    MD5State md5;
    KMDataNode* stats;
    uint64_t buf_sz;
    char* buf;
} refseq_meta_stats_data;


static
void CC refseq_meta_stats_whack( void* self )
{
    if( self != NULL ) {
        refseq_meta_stats_data* data = self;
        KMDataNodeRelease(data->stats);
        free(data->buf);
        free(data);
    }
}

static
rc_t CC refseq_meta_stats( void *self, const VXformInfo *info, int64_t row_id,
                           VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    KMDataNode* node;
    refseq_meta_stats_data* data = self;
    uint64_t i, seq_len = argv[ 0 ].u.data.elem_count;
    const INSDC_4na_bin * seq = argv[ 0 ].u.data.base;

    uint64_t read_len = seq_len;
   
    seq +=  argv[0].u.data.first_elem;

    if ( argc > 1 )
    {
        const INSDC_coord_len * read_len_ptr = argv [ 1 ] . u . data . base;
        read_len_ptr += argv [ 1 ] . u . data . first_elem;
        read_len = * read_len_ptr;
    }

    assert( data != NULL );

    assert ( seq_len <= read_len );

    if ( data->buf_sz < read_len )
    {
        char * x = realloc( data->buf, ( size_t )read_len );
        if ( x == NULL )
        {
            rc = RC( rcVDB, rcFunction, rcUpdating, rcMemory, rcExhausted );
        }
        else
        {
            data->buf = x;
            data->buf_sz = read_len;
        }
    }

    for ( i = 0; rc == 0 && i < seq_len; ++ i )
    {
        data->buf[ i ] = INSDC_4na_map_CHARSET[ seq[ i ] ];
    }

    for ( ; rc == 0 && i < read_len; ++ i )
    {
        data->buf[ i ] = 'N';
    }

    if ( rc == 0 )
    {
        rc = KMDataNodeOpenNodeUpdate( data->stats, &node, "TOTAL_SEQ_LEN" );
        if ( rc == 0 )
        {
            if ( data->total_seq_len + read_len < data->total_seq_len )
            {
                rc = RC( rcVDB, rcFunction, rcUpdating, rcMetadata, rcOutofrange );
            }
            else
            {
                data->total_seq_len += read_len;
                rc = KMDataNodeWriteB64( node, &data->total_seq_len );
            }
            KMDataNodeRelease( node );
        }
    }

    if ( rc == 0 )
    {
        rc = KMDataNodeOpenNodeUpdate( data->stats, &node, "CRC32" );
        if ( rc == 0 )
        {
            data->crc32 = CRC32( data->crc32, data->buf, ( size_t )read_len );
            rc = KMDataNodeWriteB32( node, &data->crc32 );
            KMDataNodeRelease( node );
        }
    }

    if ( rc == 0 )
    {
        rc = KMDataNodeOpenNodeUpdate( data->stats, &node, "MD5" );
        if ( rc == 0 )
        {
            uint8_t digest[ 16 ];
            MD5State md5;
            MD5StateAppend( &data->md5, data->buf, ( size_t )read_len );
            memmove( &md5, &data->md5, sizeof( md5 ) );
            MD5StateFinish( &md5, digest );
            rc = KMDataNodeWrite( node, digest, sizeof( digest ) );
            KMDataNodeRelease( node );
        }
    }
    return rc;
}


VTRANSFACT_IMPL ( NCBI_refSeq_stats, 2, 0, 0 ) ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
                                                 const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;

    if ( dp->argc < 1 )
    {
        rc = RC( rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid );
    }
    else
    {
        refseq_meta_stats_data * data;
        data = calloc( 1, sizeof( refseq_meta_stats_data ) );
        if ( data == NULL )
        {
            rc = RC( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            KMetadata *meta;
            rc = VTableOpenMetadataUpdate( ( VTable* )info->tbl, &meta );
            if ( rc == 0 )
            {
                rc = KMetadataOpenNodeUpdate( meta, &data->stats, "STATS" );
                KMetadataRelease( meta );
            }

            if ( rc == 0 )
            {
                rslt->self = data;
                rslt->whack = refseq_meta_stats_whack;
                rslt->variant = vftNonDetRow;
                rslt->u.rf = refseq_meta_stats;
                data->crc32 = 0;
                CRC32Init();
                MD5StateInit( &data->md5 );
            }
            else
            {
                refseq_meta_stats_whack( data );
            }
        }
    }
    return rc;
}
