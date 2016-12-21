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
#include <sra/rd-extern.h>

#include <klib/rc.h>
#include <sra/types.h>
#include <sra/sff.h>
#include <sra/sff-file.h>
#include <sysalloc.h>

#include "reader-cmn.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#   include <byteswap.h>
#else
#   define bswap_16(x) (x)
#   define bswap_32(x) (x)
#endif
#include <string.h>

/* column order is important here: see Init function below!!! */
static
const SRAReaderColumn SFFReader_master_columns_desc[] = {
    {SRAREADER_COL_MANDATORY, "FLOW_CHARS", insdc_fasta_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "KEY_SEQUENCE", insdc_fasta_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "READ", insdc_fasta_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "QUALITY", insdc_phred_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "SIGNAL", ncbi_isamp1_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "POSITION", "INSDC:position:one", NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "CLIP_QUALITY_LEFT", "INSDC:coord:one", NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "CLIP_QUALITY_RIGHT", "INSDC:coord:one", NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "CLIP_ADAPTER_LEFT", "INSDC:coord:one", NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "CLIP_ADAPTER_RIGHT", "INSDC:coord:one", NULL, NULL, 0},
    {0, NULL, NULL, NULL, NULL, 0} /* terminator */
};

struct SFFReader {
    /* SRAReader always must be a first member! */
    SRAReader dad;
    /* current spot data shortcuts */
    const SRAReaderColumn* flow_chars;
    const SRAReaderColumn* key_seq;
    const SRAReaderColumn* read;
    const SRAReaderColumn* qual1;
    const SRAReaderColumn* signal;
    const SRAReaderColumn* position;
    const INSDC_coord_one** clip_q_left;
    const INSDC_coord_one** clip_q_right;
    const INSDC_coord_one** clip_adapter_left;
    const INSDC_coord_one** clip_adapter_right;
};

static
rc_t SFFReaderInit(const SFFReader* self)
{
    rc_t rc = 0;

    CHECK_SELF(SFFReader);
    rc = SRAReaderInit(&self->dad, 0, SFFReader_master_columns_desc);
    if( GetRCState(rc) == rcNotFound && GetRCObject(rc) == (enum RCObject) rcColumn ) {
        rc = RC(rcSRA, rcFormatter, rcConstructing, rcData, rcUnsupported);
    } else if( rc == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[0], &me->flow_chars, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[1], &me->key_seq, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[2], &me->read, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[3], &me->qual1, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[4], &me->signal, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[5], &me->position, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[6], NULL, (const void***)&self->clip_q_left)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[7], NULL, (const void***)&self->clip_q_right)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[8], NULL, (const void***)&self->clip_adapter_left)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &SFFReader_master_columns_desc[9], NULL, (const void***)&self->clip_adapter_right)) == 0 ) {
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReaderMake(const SFFReader** self, const SRATable* table,
                   const char* accession, spotid_t minSpotId, spotid_t maxSpotId )
{
    rc_t rc = SRAReaderMake((const SRAReader**)self, sizeof **self, table, accession, minSpotId, maxSpotId);

    if( rc == 0 ) {
        rc = SFFReaderInit(*self);
    }
    if( rc != 0 ) {
        SFFReaderWhack(*self);
        *self = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReaderWhack(const SFFReader *self)
{
    return SRAReaderWhack( &self->dad );
}

LIB_EXPORT rc_t CC SFFReaderFirstSpot(const SFFReader* self)
{
    return SRAReaderFirstSpot( &self->dad );
}

LIB_EXPORT rc_t CC SFFReaderSeekSpot(const SFFReader* self, spotid_t spot)
{
    return SRAReaderSeekSpot( &self->dad, spot );
}

LIB_EXPORT rc_t CC SFFReaderNextSpot(const SFFReader* self)
{
    return SRAReaderNextSpot( &self->dad );
}

LIB_EXPORT rc_t CC SFFReaderCurrentSpot(const SFFReader* self, spotid_t* spot)
{
    return SRAReaderCurrentSpot( &self->dad, spot );
}

LIB_EXPORT rc_t CC SFFReader_SpotInfo(const SFFReader* self, const char** spotname, size_t* spotname_sz,
                                      uint32_t* spot_len, uint32_t* num_reads)
{
    return SRAReader_SpotInfo( &self->dad, spotname, spotname_sz, spot_len, num_reads );
}

LIB_EXPORT rc_t CC SFFReader_SpotReadInfo(const SFFReader* self, uint32_t readId, SRAReadTypes* read_type,
                                          const char** read_label, INSDC_coord_len* read_label_sz,
                                          INSDC_coord_zero* read_start, INSDC_coord_len* read_len)
{
    return SRAReader_SpotReadInfo(&self->dad, readId, read_type,
                                  read_label, read_label_sz, read_start, read_len);
}

LIB_EXPORT rc_t CC SFFReaderHeader(const SFFReader* self, spotid_t spots, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    SFFCommonHeader h;

    CHECK_SELF(SFFReader);

    /* we need to read 1st spot in order to get to flow and key seq col data */
    if( me->dad.spot == 0 && (rc = SFFReaderSeekSpot(self, me->dad.minSpotId)) != 0 ) {
        return rc;
    }
    memset(&h, 0, sizeof(SFFCommonHeader));
    memmove(&h, ".sff\0\0\0\1", 8);
    h.number_of_reads = spots != 0 ? spots : (self->dad.maxSpotId - me->dad.minSpotId + 1);
    h.key_length = (uint16_t)self->key_seq->size;
    h.num_flows_per_read = (uint16_t)self->flow_chars->size;
    h.flowgram_format_code = SFFFormatCodeUI16Hundreths;

    h.header_length = SFFCommonHeader_size + h.key_length + h.num_flows_per_read;
    /* 8-byte pad */
    h.header_length += (h.header_length % 8) ? (8 - (h.header_length % 8)) : 0;
    if( written != NULL ) {
        *written = h.header_length;
    }
    if( h.header_length > dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    } else {
        uint16_t len = h.header_length;

        h.number_of_reads = bswap_32( h.number_of_reads );
        h.header_length = bswap_16( h.header_length );
        h.key_length = bswap_16( h.key_length );
        h.num_flows_per_read = bswap_16( h.num_flows_per_read );

        memset(data, 0, len);
        memmove(data, &h, SFFCommonHeader_size);
        memmove(&data[SFFCommonHeader_size], self->flow_chars->base, self->flow_chars->size);
        memmove(&data[SFFCommonHeader_size + self->flow_chars->size], self->key_seq->base, self->key_seq->size);
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReaderReadHeader(const SFFReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t spotname_sz;
    const char* spotname;
    uint32_t spot_len;
    SFFReadHeader h;

    CHECK_SELF(SFFReader);
    CHECK_SPOT(me->dad);

    memset(&h, 0, sizeof(SFFReadHeader));

    if( (rc = SFFReader_SpotInfo(self, &spotname, &spotname_sz, &spot_len, NULL)) != 0 ) {
        return rc;
    }
    if( self->clip_q_left != NULL && *self->clip_q_left != NULL ) {
        h.clip_quality_left = **self->clip_q_left;
    }
    if( self->clip_q_right != NULL && *self->clip_q_right != NULL ) {
        h.clip_quality_right = **self->clip_q_right;
    }
    if( self->clip_adapter_left != NULL && *self->clip_adapter_left != NULL ) {
        h.clip_adapter_left = **self->clip_adapter_left;
    }
    if( self->clip_adapter_right != NULL && *self->clip_adapter_right != NULL ) {
        h.clip_adapter_right = **self->clip_adapter_right;
    }
    if( spotname_sz == 0 ) {
        spotname = self->dad.accession;
        spotname_sz = strlen(spotname);
    }
    h.name_length = (uint16_t)spotname_sz;
    h.number_of_bases = spot_len;
    h.header_length = (uint16_t)( SFFReadHeader_size + spotname_sz );
    /* 8-byte pad */
    h.header_length += (h.header_length % 8) ? (8 - (h.header_length % 8)) : 0;
    if( written != NULL ) {
        *written = h.header_length;
    }
    if( h.header_length > dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    } else {
        uint16_t len = h.header_length;

        h.header_length = bswap_16(h.header_length);
        h.name_length = bswap_16(h.name_length);
        h.number_of_bases = bswap_32(h.number_of_bases);
        h.clip_quality_right = bswap_16(h.clip_quality_right);
        h.clip_quality_left = bswap_16(h.clip_quality_left);
        h.clip_adapter_left = bswap_16(h.clip_adapter_left);
        h.clip_adapter_right = bswap_16(h.clip_adapter_right);

        memset(data, 0, len);
        memmove(data, &h, SFFReadHeader_size);
        memmove(&data[SFFReadHeader_size], spotname, spotname_sz);
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReaderReadData(const SFFReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t flows, len, padding;
    uint32_t spot_len;
    /* SFF type defining vars, not used */
    const uint16_t sff_signal;
    const uint8_t sff_position;
    const char sff_bases;
    const uint8_t sff_quality;

    CHECK_SELF(SFFReader);
    CHECK_SPOT(me->dad);

    /* if for some reason READ, QUAL and POSITION rows are uneqaul in size,
       base their length on actual READ size */
    spot_len = self->read->size / sizeof(INSDC_dna_text);
    /* this size if fixed across the file */
    flows = self->flow_chars->size / sizeof(INSDC_dna_text);

    /* read data section size: per SFF spec types sizes + 8-byte padding */
    len = flows * sizeof(sff_signal) + spot_len * (sizeof(sff_position) + sizeof(sff_bases) + sizeof(sff_quality));
    padding = (len % 8) ? (8 - (len % 8)) : 0;
    len += padding;

    if( written != NULL ) {
        *written = len;
    }
    if( len > dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    } else {
        size_t qty;
        INSDC_position_one ppos;
        const uint16_t* signal = self->signal->base;
        uint16_t* dst_sig = (uint16_t*)data;
        const INSDC_position_one* position = self->position->base;

        qty = self->signal->size / sizeof(*signal);
        if( qty > flows ) {
            qty = flows;
        }
        for(len = 0; len < qty; len++) {
            dst_sig[len] = bswap_16(signal[len]);
        }
        if( qty < flows ) {
            memset(&data[qty * sizeof(sff_signal)], 0, (flows - qty) * sizeof(sff_signal));
        }
        data += flows * sizeof(sff_signal);

        qty = self->position->size / sizeof(*position);
        if( qty > spot_len ) {
            qty = spot_len;
        }
        for(ppos = 0, len = 0; len < qty; len++) {
            data[len] = (uint8_t)(position[len] - ppos);
            ppos = position[len];
        }
        if( qty < spot_len ) {
            memset(&data[qty], 0, (spot_len - qty) * sizeof(sff_position));
        }
        data += spot_len;

        /* this are guaranteed above to be same length */
        memmove(data, self->read->base, spot_len);
        data += spot_len;

        qty = self->qual1->size / sizeof(INSDC_quality_phred);
        if( qty > spot_len ) {
            qty = spot_len;
        }
        memmove(data, self->qual1->base, qty);
        if( qty < spot_len ) {
            memset(&data[qty], 0, (spot_len - qty) * sizeof(sff_quality));
        }
        if( padding > 0 ) {
            memset(&data[spot_len], 0, padding);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReader_GetCurrentSpotData(const SFFReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t len = 0, left = dsize, w = 0;
    char* b = data;

    CHECK_SELF(SFFReader);
    CHECK_SPOT(me->dad);

    rc = SFFReaderReadHeader(me, b, left, &w);
    len += w, b += w, left -= w;
    if( rc != 0 ) {
        if( !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
            return rc;
        }
        b = data, left = dsize;
    }
    rc = SFFReaderReadData(self, b, left, &w);
    len += w, b += w, left -= w;
    if( rc != 0 ) {
        if( !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
            return rc;
        }
    }
    if( written != NULL ) {
        *written = len;
    }
    if( len > dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    return rc;
}

LIB_EXPORT rc_t CC SFFReader_GetNextSpotData(const SFFReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = SFFReaderNextSpot(self);
    if( GetRCObject(rc) == rcRow && GetRCState(rc) == rcUnknown ) {
        rc = SFFReaderFirstSpot(self);
    }
    if( rc == 0 ) {
        rc = SFFReader_GetCurrentSpotData(self, data, dsize, written);
    }
    return rc;
}
