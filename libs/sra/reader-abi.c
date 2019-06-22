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
#include <sra/abi.h>
#include <sysalloc.h>
#include <klib/text.h>

#include "reader-cmn.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>

typedef enum AbsolidReaderOptions_enum {
    eOrigFormat  = 0x02,
    eSignal      = 0x04,
    eClipQual    = 0x08
} AbsolidReaderOptions;

/* column order is important here: see Init function below!!! */
static
const SRAReaderColumn AbsolidReader_master_columns_desc[] = {
    {SRAREADER_COL_MANDATORY, "CSREAD", insdc_csfasta_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "CS_KEY", insdc_fasta_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "QUALITY", insdc_phred_t, NULL, NULL, 0},
    {eClipQual, "TRIM_START", "INSDC:coord:zero", NULL, NULL, 0},
    {eClipQual, "TRIM_LEN", "INSDC:coord:len", NULL, NULL, 0},
    {eSignal | SRAREADER_COL_OPTIONAL, "SIGNAL", ncbi_fsamp4_t, NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, NULL, NULL, NULL, NULL, 0} /* terminator */
};

struct AbsolidReader {
    /* SRAReader always must be a first member! */
    SRAReader dad;
    uint32_t minReadLen;
    /* current spot data shortcuts */
    const SRAReaderColumn* csread;
    const char** cs_key;
    const SRAReaderColumn* qual1;
    const INSDC_coord_zero** trim_start;
    const INSDC_coord_len** trim_len;
    const float** signal;
    char prefix_buf[1024];
    size_t prefix_sz;
};

static
rc_t AbsolidReaderInit(const AbsolidReader* self,
                       bool origFormat, bool noClip, uint32_t minReadLen, bool signal)
{
    rc_t rc = 0;
    int options = origFormat ? eOrigFormat : 0;

    CHECK_SELF(AbsolidReader);

    options |= signal ? eSignal : 0;
    options |= noClip ? 0 : eClipQual;
    me->minReadLen = minReadLen;

    if( (rc = SRAReaderInit(&self->dad, options, AbsolidReader_master_columns_desc)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[0], &me->csread, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[1], NULL, (const void***)&self->cs_key)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[2], &me->qual1, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[3], NULL, (const void***)&self->trim_start)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[4], NULL, (const void***)&self->trim_len)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &AbsolidReader_master_columns_desc[5], NULL, (const void***)&self->signal)) == 0 ) {
    }
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderMake(const AbsolidReader** self, const SRATable* table,
                                     const char* accession, bool origFormat,
                                     bool noClip, uint32_t minReadLen,
                                     spotid_t minSpotId, spotid_t maxSpotId, bool signal)
{
    rc_t rc = SRAReaderMake((const SRAReader**)self, sizeof **self, table, accession, minSpotId, maxSpotId);

    if( rc == 0 ) {
        rc = AbsolidReaderInit(*self, origFormat, noClip, minReadLen, signal);
    }
    if( rc != 0 ) {
        AbsolidReaderWhack(*self);
        *self = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderWhack(const AbsolidReader* self)
{
    return SRAReaderWhack(&self->dad);
}

LIB_EXPORT rc_t CC AbsolidReaderFirstSpot(const AbsolidReader* self)
{
    return SRAReaderFirstSpot(&self->dad);
}

LIB_EXPORT rc_t CC AbsolidReaderSeekSpot(const AbsolidReader* self, spotid_t spot)
{
    return SRAReaderSeekSpot(&self->dad, spot);
}

LIB_EXPORT rc_t CC AbsolidReaderNextSpot(const AbsolidReader* self)
{
    return SRAReaderNextSpot(&self->dad);
}

LIB_EXPORT rc_t CC AbsolidReaderCurrentSpot(const AbsolidReader* self, spotid_t* spot)
{
    return SRAReaderCurrentSpot(&self->dad, spot);
}

LIB_EXPORT rc_t CC AbsolidReader_SpotInfo(const AbsolidReader* self,
                                          const char** spotname, size_t* spotname_sz,
                                          uint32_t* spot_len, uint32_t* num_reads)
{
    rc_t rc = SRAReader_SpotInfo(&self->dad, spotname, spotname_sz, spot_len, num_reads);
    if( rc == 0 ) {
        if( spot_len ) {
            if( self->dad.options & eClipQual ) {
                *spot_len = **self->trim_len;
            }
            if( *spot_len < self->minReadLen ) {
                *spot_len = 0;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderSpotName(const AbsolidReader* self,
                                         const char** prefix, size_t* prefix_sz,
                                         const char** suffix, size_t* suffix_sz)
{
    rc_t rc = 0;
    const char* spotname;
    size_t spotname_sz;

    CHECK_SELF(AbsolidReader);

    rc = SRAReader_SpotInfo(&self->dad, &spotname, &spotname_sz, NULL, NULL);
    if( rc == 0 ) {
        if( !self->prefix_sz || self->prefix_sz > spotname_sz || strncmp(spotname, self->prefix_buf, self->prefix_sz) != 0 ) {
            if( spotname_sz == 0 ) {
                me->prefix_sz = 0;
            } else {
                int k = 0;
                size_t psz = spotname_sz;
                while( psz > 0 && k < 3 ) {
                    /* take out PLATE_X_Y and optional label _(F|R)3 */
                    while( psz > 0 && isdigit( *(spotname + psz - 1)) ) {
                        psz--;
                    }

                    if( *(spotname + psz - 1) == 'F' || *(spotname + psz - 1) == 'R') {
                        /* Discard F|R and preceding underscore */
                        if( --psz > 0 && !isdigit(*(spotname + psz - 1)) ) {
                            psz--;
                        }
                        continue;
                    } else if( psz > 0 ) {
                        /* Discard underscore */
                        psz--;
                        k++;
                    }
                }
                if( psz > 0 ) {
                    /* Add one to restore underscore at end of prefix */
                    me->prefix_sz = psz + 1;
                    string_copy(me->prefix_buf, sizeof(me->prefix_buf), spotname, me->prefix_sz);
                } else {
                    me->prefix_sz = 0;
                }
            }
            me->prefix_buf[me->prefix_sz] = '\0';
        }
        if( suffix ) {
            *suffix = &spotname[self->prefix_sz];
        }
        if( suffix_sz ) {
            *suffix_sz = spotname_sz - self->prefix_sz;
        }
        if( prefix ) {
            *prefix = self->prefix_buf;
        }
        if( prefix_sz ) {
            *prefix_sz = self->prefix_sz;
        }
    }
    return rc;

}

LIB_EXPORT rc_t CC AbsolidReader_SpotReadInfo(const AbsolidReader* self, uint32_t readId, SRAReadTypes* read_type, 
                                              const char** read_label, INSDC_coord_len* read_label_sz,
                                              INSDC_coord_zero* read_start, INSDC_coord_len* read_len)
{
    INSDC_coord_zero rs;
    INSDC_coord_len rl;

    rc_t rc = SRAReader_SpotReadInfo(&self->dad, readId, read_type, read_label, read_label_sz, &rs, &rl);
    if( rc == 0 ) {
        if( read_start || read_len ) {
            if( self->dad.options & eClipQual ) {
                INSDC_coord_zero end = rs + rl - 1;
                INSDC_coord_zero trim_end = ((**self->trim_start) + (**self->trim_len)) - 1;
                if( end < (**self->trim_start) || rs > trim_end ) {
                    rl = 0;
                } else {
                    if( (**self->trim_start) > rs && (**self->trim_start) <= end ) {
                        rl -= (**self->trim_start) - rs;
                        rs = (**self->trim_start);
                    }
                    if( end > trim_end ) {
                        rl = trim_end - rs + 1;
                    }
                }
            }
            if( rl < self->minReadLen ) {
                rl = 0;
            }
            if( read_start ) {
                *read_start = rl ? rs : 0;
            }
            if( read_len ) {
                *read_len = rl;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderHeader(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    int ret = 0;

    const char* spotname;
    size_t x;
    int spotname_sz;
    INSDC_coord_len read_label_sz = 0;
    const char* read_label;

    CHECK_SELF(AbsolidReader);
    CHECK_SPOT(self->dad);

    if( (rc = AbsolidReaderSpotName(me, NULL, NULL, &spotname, &x)) != 0 ) {
        return rc;
    }
    spotname_sz = (int)x;
    if( readId > 0 ) {
        if( (rc = AbsolidReader_SpotReadInfo(self, readId, NULL, &read_label, &read_label_sz, NULL, NULL)) != 0 ) {
            return rc;
        }
    }
    if( self->dad.options & eOrigFormat ) {
        char tmp[1024];
        if( spotname_sz == 0 ) {
            spotname_sz = snprintf(tmp, sizeof(tmp) - 1, "%s.%lld", self->dad.accession, ( long long int ) self->dad.spot);
            if ( spotname_sz < 0 )
                return RC ( rcSRA, rcString, rcConstructing, rcData, rcCorrupt );
            spotname = tmp;
        }
        if( readId > 0 && read_label_sz > 0 ) {
            ret = snprintf(data, dsize, ">%.*s%s%.*s", spotname_sz, spotname, spotname_sz ? "_" : "", read_label_sz, read_label);
        } else {
            ret = snprintf(data, dsize, ">%.*s", spotname_sz, spotname);
        }
    } else {
        if( readId > 0 && read_label_sz > 0 ) {
            ret = snprintf(data, dsize, ">%s.%lld %.*s%s%.*s",
                self->dad.accession, ( long long int ) self->dad.spot, spotname_sz, spotname, spotname_sz ? "_" : "", read_label_sz, read_label);
        } else {
            ret = snprintf(data, dsize, ">%s.%lld %.*s",
                           self->dad.accession, ( long long int ) self->dad.spot, spotname_sz, spotname);
        }
    }
#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
    if( ret < 0 ) {
        ret = 0;
        rc = RC(rcSRA, rcString, rcConstructing, rcMessage, rcUnknown);
    } else if( ret >= (int)dsize ) {
#else
    if( ret < 0 || ret >= (int)dsize ) {
#endif
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    if( written != NULL ) {
        *written = ret;
    }
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderBase(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    INSDC_coord_zero read_start = 0;
    INSDC_coord_len read_len = 0;

    CHECK_SELF(AbsolidReader);
    CHECK_SPOT(self->dad);

    if( readId > 0 ) {
        if( (rc = AbsolidReader_SpotReadInfo(self, readId--, NULL, NULL, NULL, &read_start, &read_len)) != 0 ) {
            return rc;
        }
    } else {
        return RC(rcSRA, rcFormatter, rcConstructing, rcFormat, rcUnsupported);
    }
    if( read_len < self->minReadLen ) {
        read_len = 0;
    }
    /* for cs_key */
    read_len++;
    if( written != NULL ) {
        *written = read_len;
    }
    if( read_len >= dsize ) {
        return RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    } else {
        const char* b = self->csread->base;
        data[0] = (*me->cs_key)[readId];
        memmove(&data[1], &b[read_start], read_len - 1);
    }
    data[read_len] = '\0';
    return rc;
}

LIB_EXPORT rc_t CC AbsolidReaderQuality(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    INSDC_coord_zero read_start = 0;
    INSDC_coord_len j = 0, read_len = 0;

    CHECK_SELF(AbsolidReader);
    CHECK_SPOT(self->dad);

    if( readId > 0 ) {
        if( (rc = AbsolidReader_SpotReadInfo(self, readId, NULL, NULL, NULL, &read_start, &read_len)) != 0 ) {
            return rc;
        }
    } else {
        return RC(rcSRA, rcFormatter, rcConstructing, rcFormat, rcUnsupported);
    }
    if( read_len >= self->minReadLen && me->qual1->size ) {
        const int8_t* q = me->qual1->base;
        char* d = data;
        INSDC_coord_len i;

        /* read end */
        read_len += read_start;
        for(i = read_start; i < read_len; i++) {
            int x;
            if( j + 2 > dsize ) {
                /* do not overflow buffer in case it's too small */
                d = data;
            }
            x = snprintf(d, dsize - j, "%i ", (int)(q[i]));
            if ( x < 0 )
                return RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
            d += x;
            j += x;
        }
        j--;
        d[j] = '\0';
    }
    if( written != NULL ) {
        *written = j;
    }
    if( j >= dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    return rc;
}

static
rc_t AbsolidReaderSignal(const AbsolidReader* self, uint32_t readId, int idx, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    INSDC_coord_len j = 0;

    CHECK_SELF(AbsolidReader);
    CHECK_SPOT(self->dad);

    if( me->signal != NULL ) {
        INSDC_coord_zero read_start = 0;
        INSDC_coord_len read_len = 0;
        if( readId > 0 ) {
            if( (rc = AbsolidReader_SpotReadInfo(self, readId, NULL, NULL, NULL, &read_start, &read_len)) != 0 ) {
                return rc;
            }
        } else {
            return RC(rcSRA, rcFormatter, rcConstructing, rcFormat, rcUnsupported);
        }
        if( read_len >= self->minReadLen ) {
            const float* s = *me->signal;
            char* d = data;
            INSDC_coord_len i;

            /* read end */
            read_len += read_start;
            for(i = read_start; i < read_len; i++) {
                int x;
                if( j + 9 > dsize ) {
                    /* do not overflow buffer in case it's too small */
                    d = data;
                }
                x = snprintf(d, dsize - j, "%.6g ", s[i * 4 + idx]);
                if ( x < 0 )
                    return RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
                d += x;
                j += x;
            }
            *d = '\0';
            --j;
        }
    }
    if( written != NULL ) {
        *written = j;
    }
    if( j >= dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    return rc;

}

LIB_EXPORT rc_t CC AbsolidReaderSignalFTC(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    return AbsolidReaderSignal(self, readId, 0, data, dsize, written);
}

LIB_EXPORT rc_t CC AbsolidReaderSignalCY3(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    return AbsolidReaderSignal(self, readId, 1, data, dsize, written);
}

LIB_EXPORT rc_t CC AbsolidReaderSignalTXR(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    return AbsolidReaderSignal(self, readId, 2, data, dsize, written);
}

LIB_EXPORT rc_t CC AbsolidReaderSignalCY5(const AbsolidReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    return AbsolidReaderSignal(self, readId, 3, data, dsize, written);
}
