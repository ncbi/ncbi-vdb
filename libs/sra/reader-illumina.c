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
#include <sra/illumina.h>
#include <os-native.h>
#include <sysalloc.h>
#include <va_copy.h>

#include "reader-cmn.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum IlluminaReaderOptions_enum {
    eRead = 0x02,
    eQual1 = 0x04,
    eQual4 = 0x08,
    eIntensity = 0x10,
    eNoise = 0x20,
    eSignal = 0x40,
    eQSeq  = 0x80
} IlluminaReaderOptions;

/* column order is important here: see Init function below!!! */
static
const SRAReaderColumn IlluminaReader_master_columns_desc[] = {
    {SRAREADER_COL_OPTIONAL, "LANE", vdb_int32_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "TILE", vdb_int32_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "X", vdb_int32_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "Y", vdb_int32_t, NULL, NULL, 0},
    {eRead | eQSeq, "READ", insdc_fasta_t, NULL, NULL, 0},
    {eQual1 | eQSeq, "QUALITY", insdc_phred_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eQual4, "QUALITY", ncbi_qual4_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eSignal, "SIGNAL", ncbi_fsamp4_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eNoise, "NOISE", ncbi_fsamp4_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eIntensity, "INTENSITY", ncbi_fsamp4_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eQSeq, "READ_FILTER", sra_read_filter_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL | eQSeq , "SPOT_GROUP", vdb_ascii_t, NULL, NULL, 0},
    {0, NULL, NULL, NULL, NULL, 0} /* terminator */
};

struct IlluminaReader {
    /* SRAReader always must be a first member! */
    SRAReader dad;
    unsigned char phred2logodds_printable[256];
    /* current spot data shortcuts */
    const int32_t** lane;
    const int32_t** tile;
    const int32_t** x;
    const int32_t** y;
    const SRAReaderColumn* read;
    const SRAReaderColumn* qual1;
    const signed char** qual4;
    const float** signal;
    const float** noise;
    const float** intensity;
    const uint8_t** rfilter;
    const SRAReaderColumn* spot_group;
};

static
rc_t IlluminaReaderInit(const IlluminaReader* self, bool read, bool qual1, bool qual4, bool intensity, bool noise, bool signal, bool qseq)
{
    rc_t rc = 0;
    int options = read ? eRead : 0;

    CHECK_SELF(IlluminaReader);

    options |= qual1 ? eQual1 : 0;
    options |= qual4 ? eQual4 : 0;
    options |= intensity ? eIntensity : 0;
    options |= noise ? eNoise : 0;
    options |= signal ? eSignal : 0;
    options |= qseq ? eQSeq : 0;

    if( (rc = SRAReaderInit(&self->dad, options, IlluminaReader_master_columns_desc)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[0], NULL, (const void***)&self->lane)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[1], NULL, (const void***)&self->tile)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[2], NULL, (const void***)&self->x)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[3], NULL, (const void***)&self->y)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[4], &me->read, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[5], &me->qual1, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[6], NULL, (const void***)&self->qual4)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[7], NULL, (const void***)&self->signal)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[8], NULL, (const void***)&self->noise)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[9], NULL, (const void***)&self->intensity)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[10], NULL, (const void***)&self->rfilter)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &IlluminaReader_master_columns_desc[11], &me->spot_group, NULL)) == 0) {
    }
    if( self->qual1 != NULL ) {
        int i;
        const float offset = (float)64.499;
        me->phred2logodds_printable[0] = (unsigned char)offset;
        for(i = 1; i < 256; i++) {
            me->phred2logodds_printable[i] = (unsigned char)(10 * log(pow(10, i / 10.0) - 1) / log(10) + offset);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReaderMake(const IlluminaReader** self, const SRATable* table, const char* accession,
                          bool read, bool qual1, bool qual4, bool intensity, bool noise, bool signal, bool qseq,
                          spotid_t minSpotId, spotid_t maxSpotId)
{
    rc_t rc = SRAReaderMake((const SRAReader**)self, sizeof **self, table, accession, minSpotId, maxSpotId);
    if( rc == 0 ) {
        rc = IlluminaReaderInit(*self, read, qual1, qual4, intensity, noise, signal, qseq);
    }
    if( rc != 0 ) {
        IlluminaReaderWhack(*self);
        *self = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReaderWhack(const IlluminaReader* self)
{
    return SRAReaderWhack( &self->dad );
}

LIB_EXPORT rc_t CC IlluminaReaderFirstSpot(const IlluminaReader* self)
{
    return SRAReaderFirstSpot( &self->dad );
}

LIB_EXPORT rc_t CC IlluminaReaderSeekSpot(const IlluminaReader* self, spotid_t spot)
{
    return SRAReaderSeekSpot( &self->dad, spot );
}

LIB_EXPORT rc_t CC IlluminaReaderNextSpot(const IlluminaReader* self)
{
    return SRAReaderNextSpot( &self->dad );
}

LIB_EXPORT rc_t CC IlluminaReaderCurrentSpot(const IlluminaReader* self, spotid_t* spot)
{
    return SRAReaderCurrentSpot( &self->dad, spot );
}

LIB_EXPORT rc_t CC IlluminaReader_SpotInfo(const IlluminaReader* self,
                                           const char** spotname, size_t* spotname_sz,
                                           INSDC_coord_val* lane, INSDC_coord_val* tile, 
                                           INSDC_coord_val* x, INSDC_coord_val* y,
                                           uint32_t* spot_len, uint32_t* num_reads)
{
    rc_t rc = SRAReader_SpotInfo(&self->dad, spotname, spotname_sz, spot_len, num_reads);
    if( rc == 0 ) {
        if( lane ) {
            *lane = (self->lane && *self->lane) ? **self->lane : 0;
        }
        if( tile ) {
            *tile = (self->tile && *self->tile) ? **self->tile : ((self->dad.spot / 10001) + 1);
        }
        if( x ) {
            *x = (self->x && *self->x) ? **self->x : 0;
        }
        if( y ) {
            *y = (self->y && *self->y) ? **self->y : self->dad.spot;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReader_SpotReadInfo(const IlluminaReader* self, uint32_t readId, SRAReadTypes* read_type,
                                               const char** read_label, INSDC_coord_len* read_label_sz,
                                               INSDC_coord_zero* read_start, INSDC_coord_len* read_len)
{
    return SRAReader_SpotReadInfo(&self->dad, readId, read_type,
                                  read_label, read_label_sz, read_start, read_len);
}

#if 0
static
int kludge_snprintf ( char *buff, size_t bsize, const char *fmt, ... )
{
    int status;
    va_list args, args2;

    va_start ( args, fmt );
    va_copy ( args2, args );

    status = vsnprintf ( buff, bsize, fmt, args );
    if ( status < 0 )
    {
        size_t kludge_size = 64 * 1024;
        void *kludge = malloc ( kludge_size );
        if ( kludge != NULL )
        {
            status = vsnprintf ( kludge, kludge_size, fmt, args2 );
            if ( status >= 0 )
            {
                memmove ( buff, kludge, ( size_t ) status > bsize ? bsize : ( size_t ) status );
                if ( ( size_t ) status < bsize )
                    buff [ status ] = 0;
            }
            free ( kludge );
        }
    }

    va_end ( args2 );
    va_end ( args );

    return status;
}
#endif

LIB_EXPORT rc_t CC IlluminaReaderBase(const IlluminaReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    int sz = 0;
    uint32_t spot_len;
    INSDC_coord_val lane, tile, x, y;

    CHECK_SELF(IlluminaReader);
    CHECK_SPOT(me->dad);

    if( data == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcNull);
    } else if( !(self->dad.options & eRead) ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMessage, rcUnexpected);
    } else if( (rc = IlluminaReader_SpotInfo(self, NULL, NULL, &lane, &tile, &x, &y, &spot_len, NULL)) == 0 ) {
        sz = snprintf(data, dsize, "%d\t%d\t%d\t%d\t", lane, tile, x, y);
        if( sz < 0 ) {
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
            rc = RC(rcSRA, rcString, rcConstructing, rcNoObj, rcUnknown);
#else
            /* both approaches are wrong - the error code is rcBuffer, rcInsufficient */
            rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
#endif
        } else {
            sz += spot_len;
            if( sz >= (int)dsize ) {
                rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
            } else {
                size_t len = 0;
                sz -= spot_len;
                while(len < spot_len) {
                    const char* b = self->read->base;
                    data[sz + len] = b[len];
                    if( data[sz + len] == 'N' ) {
                        data[sz + len] = '.';
                    }
                    ++len;
                }
                sz += spot_len;
                data[sz] = '\0';
            }
        }
    }
    if( written != NULL ) {
        *written = sz;
    }
    return rc;
}

static
rc_t IlluminaReader_4floats(const IlluminaReader* self, const float** col_data, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    int sz = 0;
    const float* f = NULL;
    uint32_t i, spot_len;
    INSDC_coord_val lane, tile, x, y;

    CHECK_SELF(IlluminaReader);
    CHECK_SPOT(me->dad);

    if( data == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcNull);
    } else if( (rc = IlluminaReader_SpotInfo(self, NULL, NULL, &lane, &tile, &x, &y, &spot_len, NULL)) == 0 ) {
        if( col_data == NULL || *col_data == NULL ) {
            /* optional column is missing */
            if( written != NULL ) {
                *written = 0;
            }
            data[0] = '\0';
        } else {
            f = *col_data;
            sz = snprintf(data, dsize, "%d\t%d\t%d\t%d", lane, tile, x, y);
            if( sz < 0 ) {
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
                rc = RC(rcSRA, rcString, rcConstructing, rcNoObj, rcUnknown);
#else
                rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
#endif
            } else {
                for(i = 0; rc == 0 && i < spot_len; i++, f += 4) {
                    int x = sz >= (int)dsize ? 0 : sz;
                    x = snprintf(&data[x], dsize - x, "\t%.*f %.*f %.*f %.*f", f[0] ? 1 : 0, f[0], f[1] ? 1 : 0, f[1],
                                 f[2] ? 1 : 0,  f[2], f[3] ? 1 : 0,  f[3]);
                    if( x < 0 ) {
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
                        rc = RC(rcSRA, rcString, rcConstructing, rcNoObj, rcUnknown);
#else
                        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
#endif
                    }
                    sz += x;
                }
                if( rc == 0 && written != NULL ) {
                    *written = sz;
                }
            }
        }
    }
    if( rc == 0 && sz >= (int)dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReaderNoise(const IlluminaReader* self, char* data, size_t dsize, size_t* written)
{
    return IlluminaReader_4floats(self, self->noise, data, dsize, written);
}

LIB_EXPORT rc_t CC IlluminaReaderIntensity(const IlluminaReader* self, char* data, size_t dsize, size_t* written)
{
    return IlluminaReader_4floats(self, self->intensity, data, dsize, written);
}

LIB_EXPORT rc_t CC IlluminaReaderSignal(const IlluminaReader* self, char* data, size_t dsize, size_t* written)
{
    return IlluminaReader_4floats(self, self->signal, data, dsize, written);
}

LIB_EXPORT rc_t CC IlluminaReaderQuality1(const IlluminaReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t sz = 0;
    const uint8_t* p = NULL;

    CHECK_SELF(IlluminaReader);
    CHECK_SPOT(me->dad);

    if( data == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcNull);
    } else if( !(self->dad.options & eQual1) ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMessage, rcUnexpected);
    } else if( self->qual1 == NULL || self->qual1->size == 0 ) {
        /* empty value */
    } else if( readId > 0 ) {
        INSDC_coord_zero read_start;
        INSDC_coord_len read_len;
        if( (rc = IlluminaReader_SpotReadInfo(self, readId, NULL, NULL, NULL, &read_start, &read_len)) == 0 ) {
            sz = read_len;
            p = self->qual1->base;
            p += read_start;
        }
    } else {
        uint32_t spot_len;
        if( (rc = IlluminaReader_SpotInfo(self, NULL, NULL, NULL, NULL, NULL, NULL, &spot_len, NULL)) == 0 ) {
            sz = spot_len;
            p = self->qual1->base;
        }
    }
    if( rc == 0 ) {
        if( sz >= dsize ) {
            rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
        } else {
            size_t i;
            for(i = 0; i < sz; i++) {
                data[i] = self->phred2logodds_printable[p[i]];
            }
            data[sz] = '\0';
        }
    }
    if( written != NULL ) {
        *written = sz;
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReaderQuality4(const IlluminaReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    uint32_t spot_len;

    CHECK_SELF(IlluminaReader);
    CHECK_SPOT(me->dad);

    if( data == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcNull);
    } else if( self->qual4 == NULL ) {
        /* optional column is missing */
        if( written != NULL ) {
            *written = 0;
        }
        data[0] = '\0';
    } else if( (rc = IlluminaReader_SpotInfo(self, NULL, NULL, NULL, NULL, NULL, NULL, &spot_len, NULL)) == 0 ) {
        int sz = spot_len * 20 - 1; /* no \t at line start */
        if( written != NULL ) {
            *written = sz;
        }
        if( sz >= (int)dsize ) {
            rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
        } else {
            const signed char* q4 = *self->qual4;
            int x;
            uint32_t len = 0;
            for(sz = 0; rc == 0 && len < spot_len; len++, q4 += 4) {
                x = snprintf(&data[sz], dsize - sz, "%s%4d %4d %4d %4d", len > 0 ? "\t" : "", q4[0], q4[1], q4[2], q4[3]);
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
                if( x < 0 ) {
                    rc = RC(rcSRA, rcString, rcConstructing, rcNoObj, rcUnknown);
#else
                if( x < 0 || ( size_t ) sz + x >= dsize ) {
                    rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
#endif
                }
                sz += x;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC IlluminaReaderQSeq(const IlluminaReader* self, uint32_t readId, bool spot_group,
                                      char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    const char* spotname;
    size_t spotname_sz;
    uint32_t nreads, spotlen;
    INSDC_coord_val lane, tile, x, y;
    int sz = 0;

    CHECK_SELF(IlluminaReader);
    CHECK_SPOT(me->dad);

    if( data == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcNull);
    } else if( !(self->dad.options & eQSeq) ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMessage, rcUnexpected);
    } else if( (rc = IlluminaReader_SpotInfo(self, &spotname, &spotname_sz, &lane, &tile, &x, &y, &spotlen, &nreads)) == 0 ) {
        INSDC_coord_zero read_start = 0;
        INSDC_coord_len read_len = spotlen;
        if( readId > 0 ) {
            rc = IlluminaReader_SpotReadInfo(self, readId, NULL, NULL, NULL, &read_start, &read_len);
        }
        if( rc == 0 ) {
            const char* sg = "0";
            int sg_sz = 1;
            const char* runid = "0";
            size_t runid_sz = 1;
            int i = 0;

            if( spotname_sz > 0 ) {
                const char* c = spotname + spotname_sz;
                while( i < 4 && c != NULL ) {
                    c = memrchr(spotname, ':', c - spotname);
                    if( c != NULL ) {
                        i++;
                    }
                }
                if( i == 4 ) {
                    spotname_sz = c - spotname;
                    c = memrchr(spotname, '_', spotname_sz);
                    if( c != NULL ) {
                        runid = c + 1;
                        runid_sz = spotname_sz - (c - spotname) - 1;
                        spotname_sz = c - spotname;
                    }
                } else {
                    spotname_sz = 0;
                }
            }
            if( spotname_sz == 0 ) {
                spotname = self->dad.accession;
                spotname_sz = strlen(spotname);
            }
            if( spot_group && self->spot_group && self->spot_group->size > 0 ) {
                sg = self->spot_group->base;
                sg_sz = self->spot_group->size;
            }
            i = snprintf(data, dsize, "%.*s\t%.*s\t%d\t%d\t%d\t%d\t%.*s\t%d\t",
                (int)spotname_sz, spotname, (int)runid_sz, runid, lane, tile, x, y, sg_sz, sg, readId > 0 ? readId : 1);
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
            if( i < 0 ) {
                rc = RC(rcSRA, rcString, rcConstructing, rcNoObj, rcUnknown);
#else
            if( i < 0 || ( size_t ) i >= dsize ) {
                rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
#endif
            } else {
                sz += i;
                i = sz; /* save length of initial part */

                /* precalc full line length: seq\tqual\tflt */
                sz += 2 * (read_len + 1) + 1;
                if( sz >= (int)dsize ) {
                    rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
                } else {
                    char flt = '1';
                    const char* b = self->read->base;

                    /* reverse back to initial part length */
                    sz = i;
                    if( *self->rfilter != NULL ) {
                        if( readId > 0 && (*self->rfilter)[readId - 1] != SRA_READ_FILTER_PASS ) {
                            flt = '0';
                        } else {
                            for(i = 0; i < nreads; i++) {
                                if( (*self->rfilter)[i] != SRA_READ_FILTER_PASS ) {
                                    flt = '0';
                                    break;
                                }
                            }
                        }
                    }
                    for(i = 0; i < read_len; i++) {
                        data[sz + i] = b[read_start + i];
                        if( data[sz + i] == 'N' ) {
                            data[sz + i] = '.';
                        }
                    }
                    sz += read_len;
                    data[sz++] = '\t';
                    if( self->qual1 && self->qual1->size ) {
                        const uint8_t* p = self->qual1->base;
                        for(i = 0; i < read_len; i++) {
                            data[sz + i] = p[read_start + i] + 64;
                        }
                        sz += read_len;
                    }
                    data[sz++] = '\t';
                    data[sz++] = flt;
                    data[sz] = '\0';
                }
            }
        }
    }
    if( written != NULL ) {
        *written = sz;
    }
    return rc;
}
