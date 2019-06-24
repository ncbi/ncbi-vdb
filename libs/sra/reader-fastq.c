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
#include <sra/fastq.h>
#include <os-native.h>
#include <sysalloc.h>

#include "reader-cmn.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum FastqReaderOptions_enum {
    eBaseSpace   = 0x02,
    eColorSpace  = 0x04,
    eOrigFormat  = 0x08,
    ePrintLabel  = 0x10,
    ePrintReadId = 0x20,
    eClipQual    = 0x40,
    eUseQual     = 0x80,
    eSuppressQualForCSKey   = 0x100     /* added Jan 15th 2014 ( a new fastq-variation! ) */
} FastqReaderOptions;

/* column order is important here: see Init function below!!! */
static
const SRAReaderColumn FastqReader_master_columns_desc[] = {
    {eBaseSpace, "READ", insdc_fasta_t, NULL, NULL, 0},
    {eColorSpace, "CSREAD", insdc_csfasta_t, NULL, NULL, 0},
    {eColorSpace, "CS_KEY", insdc_fasta_t, NULL, NULL, 0},
    {eUseQual, "QUALITY", insdc_phred_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "SPOT_GROUP", vdb_ascii_t, NULL, NULL, 0},
    {eClipQual, "TRIM_START", "INSDC:coord:zero", NULL, NULL, 0},
    {eClipQual, "TRIM_LEN", "INSDC:coord:len", NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, NULL, NULL, NULL, NULL, 0} /* terminator */
};

struct FastqReader {
    /* SRAReader always must be a first member! */
    SRAReader dad;
    uint32_t minReadLen;
    int offset;
    char csKey;
    /* current spot data shortcuts */
    const SRAReaderColumn* read;
    const SRAReaderColumn* csread;
    const char** cs_key;
    const uint8_t** qual1;
    const SRAReaderColumn* spot_group;
    const INSDC_coord_zero** trim_start;
    const INSDC_coord_len** trim_len;
    /* quality conversion table */
    char q2ascii[256];
};

static
rc_t FastqReaderInit(const FastqReader* self,
                     bool colorSpace, bool origFormat, bool fasta, bool printLabel, bool printReadId,
                     bool noClip, bool SuppressQualForCSKey, uint32_t minReadLen, int offset, char csKey)
{
    rc_t rc = 0;
    int options = colorSpace ? eColorSpace : eBaseSpace;

    CHECK_SELF(FastqReader);

    options |= origFormat ? eOrigFormat : 0;
    options |= printLabel ? ePrintLabel : 0;
    options |= printReadId ? ePrintReadId : 0;
    options |= noClip ? 0 : eClipQual;
    options |= fasta ? 0 : eUseQual;
    options |= SuppressQualForCSKey ? eSuppressQualForCSKey : 0; /* added Jan 15th 2014 ( a new fastq-variation! ) */

    me->minReadLen = minReadLen;
    me->offset = offset > 0 ? offset : 33;
    me->csKey = csKey;

    if( (rc = SRAReaderInit(&self->dad, options, FastqReader_master_columns_desc)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[0], &me->read, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[1], &me->csread, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[2], NULL, (const void***)&self->cs_key)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[3], NULL, (const void***)&self->qual1)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[4], &me->spot_group, NULL)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[5], NULL, (const void***)&self->trim_start)) == 0 &&
        (rc = SRAReader_FindColData(&self->dad, &FastqReader_master_columns_desc[6], NULL, (const void***)&self->trim_len)) == 0 ) {
        memset(me->q2ascii, '~', sizeof(self->q2ascii));
        for(options = 0; options < 256; options++) {
            me->q2ascii[options] = options + self->offset;
            if( self->q2ascii[options] == '~' ) {
                break;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReaderMake(const FastqReader** self, const SRATable* table, const char* accession,
                                   bool colorSpace, bool origFormat, bool fasta, bool printLabel, bool printReadId,
                                   bool noClip, bool SuppressQualForCSKey, uint32_t minReadLen, char offset, char csKey,
                                   spotid_t minSpotId, spotid_t maxSpotId)
{
    rc_t rc = SRAReaderMake((const SRAReader**)self, sizeof **self, table, accession, minSpotId, maxSpotId);

    if( rc == 0 ) {
        rc = FastqReaderInit(*self, colorSpace, origFormat, fasta,
                             printLabel, printReadId, noClip, SuppressQualForCSKey, minReadLen, offset, csKey);
    }
    if( rc != 0 ) {
        FastqReaderWhack(*self);
        *self = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReaderWhack(const FastqReader* self)
{
    return SRAReaderWhack(&self->dad);
}

LIB_EXPORT rc_t CC FastqReaderFirstSpot(const FastqReader* self)
{
    return SRAReaderFirstSpot(&self->dad);
}

LIB_EXPORT rc_t CC FastqReaderSeekSpot(const FastqReader* self, spotid_t spot)
{
    return SRAReaderSeekSpot(&self->dad, spot);
}

LIB_EXPORT rc_t CC FastqReaderNextSpot(const FastqReader* self)
{
    return SRAReaderNextSpot(&self->dad);
}

LIB_EXPORT rc_t CC FastqReaderCurrentSpot(const FastqReader* self, spotid_t* spot)
{
    return SRAReaderCurrentSpot(&self->dad, spot);
}

LIB_EXPORT rc_t CC FastqReader_SpotInfo(const FastqReader* self,
                                        const char** spotname, size_t* spotname_sz,
                                        const char** spotgroup, size_t* spotgroup_sz,
                                        uint32_t* spot_len, uint32_t* num_reads)
{
    rc_t rc = SRAReader_SpotInfo(&self->dad, spotname, spotname_sz, spot_len, num_reads);
    if( rc == 0 ) {
        if( spotgroup && !spotgroup_sz ) {
            return RC(rcSRA, rcFormatter, rcAccessing, rcParam, rcInvalid);
        }
        if( spot_len ) {
            if( self->dad.options & eClipQual ) {
                *spot_len = **self->trim_len;
            }
            if( *spot_len < self->minReadLen ) {
                *spot_len = 0;
            }
        }
        if( spotgroup ) {
            if( self->spot_group != NULL ) {
                *spotgroup = self->spot_group->base;
                *spotgroup_sz = self->spot_group->size;
                if(*spotgroup && *spotgroup_sz && (*spotgroup)[*spotgroup_sz - 1] == '\0' ) {
                    /* Consider zero-padded spotgroup */
                    *spotgroup_sz = strlen(*spotgroup);
                }
            } else {
                *spotgroup = NULL;
                *spotgroup_sz = 0;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReader_SpotReadInfo(const FastqReader* self, uint32_t readId, SRAReadTypes* read_type, 
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

static
rc_t FastqReader_Header(const FastqReader* self, bool* label, 
                        char* buf, size_t maxlen, size_t* written,
                        char prefix, int readId)
{
    rc_t rc = 0;
    int ret = 0;

    const char* spotname;
    size_t x;
    int spotname_sz;
    uint32_t spot_len;
    uint32_t num_reads;
    INSDC_coord_len read_len, read_label_sz = 0;
    const char* read_label;
    bool addLabel = label ? *label : (self->dad.options & ePrintLabel);

    if( (rc = FastqReader_SpotInfo(self, &spotname, &x, NULL, NULL, &spot_len, &num_reads)) != 0 ) {
        return rc;
    }
    spotname_sz = (int)x;
    if( readId > 0 ) {
        if( (rc = FastqReader_SpotReadInfo(self, readId, NULL, &read_label, &read_label_sz, NULL, &read_len)) != 0 ) {
            return rc;
        }
    }
    /* do not add empty labels */
    addLabel = read_label_sz > 0 ? addLabel : false;

    if( self->dad.options & eOrigFormat ) {
        char tmp[1024];
        if( spotname_sz == 0 ) {
            ret = snprintf(tmp, sizeof(tmp) - 1, "%s.%lld", self->dad.accession, ( long long int ) self->dad.spot);
            if ( ret < 0 )
                return RC ( rcSRA, rcString, rcConstructing, rcData, rcCorrupt );
            spotname = tmp;
            spotname_sz = ret;
        }
        if( addLabel && readId > 0 ) {
            ret = snprintf(buf, maxlen, "%c%.*s_%.*s", prefix, spotname_sz, spotname, read_label_sz, read_label);
        } else {
            ret = snprintf(buf, maxlen, "%c%.*s", prefix, spotname_sz, spotname);
        }
    } else {
        const char* sep = spotname_sz ? " " : "";
        if( addLabel && readId > 0 ) {
            sep = spotname_sz ? "_" : "";
            if( self->dad.options & ePrintReadId ) {
                ret = snprintf(buf, maxlen, "%c%s.%lld.%d %.*s%s%.*s length=%d",
                    prefix, self->dad.accession, ( long long int ) self->dad.spot, readId, spotname_sz, spotname, sep, read_label_sz, read_label, read_len);
            } else {
                ret = snprintf(buf, maxlen, "%c%s.%lld %.*s%s%.*s length=%d",
                    prefix, self->dad.accession, ( long long int ) self->dad.spot, spotname_sz, spotname, sep, read_label_sz, read_label, read_len);
            }
        } else if( readId > 0 ) {
            if( self->dad.options & ePrintReadId ) {
                ret = snprintf(buf, maxlen, "%c%s.%lld.%d %.*s%slength=%d",
                    prefix, self->dad.accession, ( long long int ) self->dad.spot, readId, spotname_sz, spotname, sep, read_len);
            } else {
                ret = snprintf(buf, maxlen, "%c%s.%lld %.*s%slength=%d",
                    prefix, self->dad.accession, ( long long int ) self->dad.spot, spotname_sz, spotname, sep, read_len);
            }
        } else {
            ret = snprintf(buf, maxlen, "%c%s.%lld %.*s%slength=%u",
                prefix, self->dad.accession, ( long long int ) self->dad.spot, spotname_sz, spotname, sep, spot_len);
        }
    }

#if SNPRINTF_ACTUALLY_WORKED_THE_WAY_YOU_THINK
    if( ret < 0 ) {
        ret = 0;
        rc = RC(rcSRA, rcString, rcConstructing, rcMessage, rcUnknown);
    } else if( ret >= (int)maxlen ) {
#else
    if( ret < 0 || ret >= (int)maxlen ) {
#endif
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    if( written != NULL ) {
        *written = ret;
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReaderBaseName(const FastqReader* self, uint32_t readId,
                                       bool* label, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;

    CHECK_SELF(FastqReader);
    CHECK_SPOT(self->dad);
    rc = FastqReader_Header(me, label, data, dsize, written, '@', readId);
    return rc;
}

LIB_EXPORT rc_t CC FastqReaderQualityName(const FastqReader* self, uint32_t readId,
                                          bool* label, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;

    CHECK_SELF(FastqReader);
    CHECK_SPOT(self->dad);
    if( self->qual1 == NULL ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMode, rcNotAvailable);
    } else {
        rc = FastqReader_Header(me, label, data, dsize, written, '+', readId);
    }
    return rc;
}

/*
#if _DEBUGGING && defined __GNUC__
#pragma message "TBD determine if we need conversions between color spaces at all"
#endif
*/

static
rc_t FastqReader_CSconvert(char from, char to, char* first)
{
#define CSKEY2IDX(k) (k == 'A' ? 0 : (k == 'C' ? 1 : (k == 'G' ? 2 : (k == 'T' ? 3 : -1))))

    const char* x[4][4] = {
        /*   TO:      A       C       G       T   */
        /* F: A */ {"0123", "1032", "2301", "3210"},
        /* R: C */ {"1032", "0123", "3210", "2301"},
        /* O: G */ {"2301", "3210", "0123", "1032"},
        /* M: T */ {"3210", "2301", "1032", "0123"}};

    if( to != '\0' && *first != '.' && toupper(*first) != 'N' ) {
        int ifrom = CSKEY2IDX(toupper(from));
        int ito = CSKEY2IDX(toupper(to));
        int ifirst = *first - '0';

        if( ifrom < 0 || ito < 0 || ifirst < 0 || ifirst > 3 ) {
            return RC(rcSRA, rcString, rcConverting, rcParam, rcInvalid);
        }
        *first = x[ifrom][ito][ifirst];
    }
    return 0;
}

LIB_EXPORT rc_t CC FastqReaderBase(const FastqReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    uint32_t num_reads;
    INSDC_coord_zero read_start = 0;
    INSDC_coord_len read_len = 0;

    CHECK_SELF(FastqReader);
    CHECK_SPOT(self->dad);

    if( readId > 0 ) {
        if( (rc = FastqReader_SpotReadInfo(self, readId--, NULL, NULL, NULL, &read_start, &read_len)) != 0 ) {
            return rc;
        }
    } else if( (rc = FastqReader_SpotInfo(self, NULL, NULL, NULL, NULL, &read_len, &num_reads)) != 0 ) {
        return rc;
    } else if( (me->dad.options & eColorSpace) && num_reads > 2 ) {
        return RC(rcSRA, rcFormatter, rcConstructing, rcFormat, rcUnsupported);
    }
    if( me->dad.options & eColorSpace ) {
        read_len++;
    }
    if( written != NULL ) {
        *written = read_len;
    }
    if( read_len >= dsize ) {
        return RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    }
    if( me->dad.options & eColorSpace ) {
        const char* b = self->csread->base;
        data[0] = me->csKey == '\0' ? (*self->cs_key)[readId] : me->csKey;
        memmove(&data[1], &b[read_start], read_len - 1);
        if( read_len > 1 ) {
            rc = FastqReader_CSconvert((*self->cs_key)[readId], data[0], &data[1]);
        }
    } else {
        const char* b = self->read->base;
        memmove(data, &b[read_start], read_len);
    }
    data[read_len] = '\0';
    return rc;
}

LIB_EXPORT rc_t CC FastqReaderQuality(const FastqReader* self, uint32_t readId, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    INSDC_coord_zero read_start = 0;
    INSDC_coord_len read_len = 0;
    bool print_quality_for_cskey = false; /* added Jan 15th 2014 ( a new fastq-variation! )*/

    CHECK_SELF(FastqReader);
    CHECK_SPOT(self->dad);

    if( self->qual1 == NULL ) {
        return RC(rcSRA, rcString, rcConstructing, rcMode, rcNotAvailable);
    }
    if( readId > 0 ) {
        if( (rc = FastqReader_SpotReadInfo(self, readId, NULL, NULL, NULL, &read_start, &read_len)) != 0 ) {
            return rc;
        }
    } else if( (rc = FastqReader_SpotInfo(self, NULL, NULL, NULL, NULL, &read_len, NULL)) != 0 ) {
        return rc;
    }

    /* added Jan 15th 2014 ( a new fastq-variation! )*/
    print_quality_for_cskey = ( ( me->dad.options & eColorSpace )&&( ( me->dad.options & eSuppressQualForCSKey ) == 0 ) );

    if( read_len < self->minReadLen ) {
        read_len = 0;
    }
    if( print_quality_for_cskey ) { /* changed Jan 15th 2014 ( a new fastq-variation! ) */
        read_len++;
    }
    if( written != NULL ) {
        *written = read_len;
    }
    if( read_len >= dsize ) {
        rc = RC(rcSRA, rcString, rcConstructing, rcMemory, rcInsufficient);
    } else {
        const uint8_t* q = *self->qual1;
        char* d = data;
        INSDC_coord_len i, j;

        if ( print_quality_for_cskey ) /* changed Jan 15th 2014 ( a new fastq-variation! ) */
        {
            *d++ = me->offset;
            --read_len;
        }
        /* read end */
        read_len += read_start;
        for(j = 0, i = read_start; i < read_len; i++, j++) {
            d[j] = self->q2ascii[q[i]];
        }
        d[j] = '\0';
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReader_GetCurrentSpotData(const FastqReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t len = 0, left = dsize, w = 0;
    char* b = data;

    CHECK_SELF(FastqReader);
    CHECK_SPOT(me->dad);

    rc = FastqReaderBaseName(self, 0, NULL, b, left, &w);
    len += ++w;
    if( rc != 0 ) {
        if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
            return rc;
        }
        b = data, left = dsize;
    } else {
        b[w - 1] = '\n'; b += w; left -= w;
    }

    rc = FastqReaderBase(self, 0, b, left, &w);
    len += ++w;
    if( rc != 0 ) {
        if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
            return rc;
        }
        b = data, left = dsize;
    } else {
        b[w - 1] = '\n'; b += w; left -= w;
    }
    if( self->qual1 != NULL ) {
        rc = FastqReaderQualityName(self, 0, NULL, b, left, &w);
        len += ++w;
        if( rc != 0 ) {
            if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
                return rc;
            }
            b = data, left = dsize;
        } else {
            b[w - 1] = '\n'; b += w; left -= w;
        }
        rc = FastqReaderQuality(self, 0, b, left, &w);
        len += ++w; b[w - 1] = '\n';
        if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
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

LIB_EXPORT rc_t CC FastqReader_GetCurrentSpotSplitData(const FastqReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = 0;
    size_t len = 0, left = dsize, w = 0;
    char* b = data;
    uint32_t r, num_reads;

    CHECK_SELF(FastqReader);
    CHECK_SPOT(me->dad);

    if( (rc = FastqReader_SpotInfo(self, NULL, NULL, NULL, NULL, NULL, &num_reads)) != 0 ) {
        return rc;
    }
    for(r = 1; r <= num_reads; r++) {
        rc = FastqReaderBaseName(self, r, NULL, b, left, &w);
        len += ++w;
        if( rc != 0 ) {
            if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
                return rc;
            }
            b = data, left = dsize;
        } else {
            b[w - 1] = '\n'; b += w; left -= w;
        }
        rc = FastqReaderBase(self, r, b, left, &w);
        len += ++w;
        if( rc != 0 ) {
            if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
                return rc;
            }
            b = data, left = dsize;
        } else {
            b[w - 1] = '\n'; b += w; left -= w;
        }
        if( self->qual1 != NULL ) {
            rc = FastqReaderQualityName(self, r, NULL, b, left, &w);
            len += ++w;
            if( rc != 0 ) {
                if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
                    return rc;
                }
                b = data, left = dsize;
            } else {
                b[w - 1] = '\n'; b += w; left -= w;
            }
            rc = FastqReaderQuality(self, r, b, left, &w);
            len += ++w;
            if( rc != 0 ) {
                if( rc != 0 && !(GetRCObject(rc) == rcMemory && GetRCState(rc) == rcInsufficient) ) {
                    return rc;
                }
                b = data, left = dsize;
            } else {
                b[w - 1] = '\n'; b += w; left -= w;
            }
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

LIB_EXPORT rc_t CC FastqReader_GetNextSpotData(const FastqReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = FastqReaderNextSpot(self);
    if( GetRCObject(rc) == rcRow && GetRCState(rc) == rcUnknown ) {
        rc = FastqReaderFirstSpot(self);
    }
    if( rc == 0 ) {
        rc = FastqReader_GetCurrentSpotData(self, data, dsize, written);
    }
    return rc;
}

LIB_EXPORT rc_t CC FastqReader_GetNextSpotSplitData(const FastqReader* self, char* data, size_t dsize, size_t* written)
{
    rc_t rc = FastqReaderNextSpot(self);
    if( GetRCObject(rc) == rcRow && GetRCState(rc) == rcUnknown ) {
        rc = FastqReaderFirstSpot(self);
    }
    if( rc == 0 ) {
        rc = FastqReader_GetCurrentSpotSplitData(self, data, dsize, written);
    }
    return rc;
}
