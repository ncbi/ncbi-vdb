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
#include <sra/extern.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <sra/types.h>
#include <os-native.h>
#include <sysalloc.h>

#include "sra-debug.h"
#include "reader-cmn.h"

#include <stdlib.h>
#include <string.h>

/* mandatory columns used by generic reader */
/* column order is IMPORTANT here: see Init function below!!! */
static
const SRAReaderColumn SRAReader_mandatory_cols[] = {
    {SRAREADER_COL_OPTIONAL, "NAME", "ascii", NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "SPOT_LEN", "INSDC:coord:len", NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "READ_START", "INSDC:coord:zero", NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "READ_LEN", "INSDC:coord:len", NULL, NULL, 0},
    {SRAREADER_COL_MANDATORY, "READ_TYPE", sra_read_type_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "LABEL", vdb_ascii_t, NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "LABEL_START", "INSDC:coord:zero", NULL, NULL, 0},
    {SRAREADER_COL_OPTIONAL, "LABEL_LEN", "INSDC:coord:len", NULL, NULL, 0},
};

static
rc_t SRAReader_ColumnsOpen(SRAReader *self)
{
    rc_t rc = 0;
    int i = 0;

    while(self->cols[i].name != NULL) {
        if( self->cols[i].opt == SRAREADER_COL_MANDATORY || (self->options & self->cols[i].opt) ) {
            rc = SRATableOpenColumnRead(self->table, &self->cols[i].col, self->cols[i].name, self->cols[i].datatype);
            if( rc != 0 ) {
                if( ((self->cols[i].opt & SRAREADER_COL_OPTIONAL) && GetRCState(rc) == rcNotFound) ) {
                    PLOGERR(klogInfo, (klogWarn, rc, "column $(c)", PLOG_S(c), self->cols[i].name));
                    rc = 0;
                } else if( GetRCState(rc) == rcExists ) {
                    rc = 0;
                } else {
                    PLOGERR(klogInfo, (klogErr, rc, "column $(c)", PLOG_S(c), self->cols[i].name));
                    break;
                }
            }
        }
        i++;
    }
    return rc;
}

static rc_t SRAReader_ColumnsRead( SRAReader * self )
{
    rc_t rc = 0;
    int i = 0;

    while ( self->cols[ i ].name != NULL )
    {
        if ( self->cols[ i ].col != NULL )
        {
            bitsz_t bitofs = 0;
            rc = SRAColumnRead( self->cols[ i ].col, self->spot, &self->cols[ i ].base, &bitofs, &self->cols[ i ].size );
            if ( rc != 0 || bitofs != 0 )
            {
                SRADBG (( "%s: read column %s spot %u %R\n", __func__, self->cols[i].name, self->spot, rc ));
                break;
            }
        }
        self->cols[ i ].size /= 8;
        i++;
    }
    return rc;
}

static
void SRAReader_ColumnsRelease(SRAReader *self)
{
    rc_t rc = 0;
    int i = 0;

    while(self->cols[i].name != NULL) {
        if(self->cols[i].col != NULL && (rc = SRAColumnRelease(self->cols[i].col)) != 0) {
            SRADBG (("%s: release column %s %R\n",
                     __func__, self->cols[i].name, rc));
        }
        self->cols[i++].col = NULL;
    }
}

static
rc_t SRAReaderAlloc(SRAReader** reader, size_t size, const char* accession)
{
    if( reader == NULL || size < sizeof(**reader) || accession == NULL || accession[0] == '\0' ) {
        return RC(rcSRA, rcFormatter, rcConstructing, rcParam, rcNull);
    }
    *reader = calloc(1, size);
    if( *reader == NULL ) {
        return RC(rcSRA, rcFormatter, rcConstructing, rcMemory, rcExhausted);
    } else {
		(*reader)->accession = strdup(accession);
        if( (*reader)->accession == NULL ) {
            free(*reader);
            *reader = NULL;
            return RC(rcSRA, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        }
    }
    return 0;
}

rc_t SRAReaderInit(const SRAReader* self, int options, const SRAReaderColumn* const cols)
{
    rc_t rc = 0;
    int i = 0, j = 0, m = sizeof(SRAReader_mandatory_cols) / sizeof(SRAReaderColumn);
    const SRAReaderColumn* col_read_len;

    CHECK_SELF(SRAReader);
    if( cols == NULL ) {
        return RC(rcSRA, rcFormatter, rcConstructing, rcParam, rcNull);
    }
    me->options = SRAREADER_COL_OPTIONAL | options; /* always try to open optional cols */
    /* copy mandatory columns */
    memmove(me->cols, SRAReader_mandatory_cols, sizeof(SRAReader_mandatory_cols));
    /* copy reader specific columns */
    for(i = 0; cols[i].name != NULL && m < 64; i++, m++) {
        for(j = 0; j < m; j++) {
            if( strcmp(cols[i].name, self->cols[j].name) == 0 &&
                strcmp(cols[i].datatype, self->cols[j].datatype) == 0 ) {
                return RC(rcSRA, rcFormatter, rcConstructing, rcData, rcDuplicate);
            }
        }
        memmove(&me->cols[m], &cols[i], sizeof(SRAReaderColumn));
    }
    if( (rc = SRAReader_ColumnsOpen(me)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[0], &me->spot_name, NULL)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[1], NULL, (const void***)&self->spot_len)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[2], NULL, (const void***)&self->read_start)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[3], NULL, (const void***)&self->read_len)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[3], &col_read_len, NULL)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[4], NULL, (const void***)&self->read_type)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[5], NULL, (const void***)&self->label)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[6], NULL, (const void***)&self->label_start)) == 0 &&
        (rc = SRAReader_FindColData(me, &SRAReader_mandatory_cols[7], NULL, (const void***)&self->label_len)) == 0 ) {
        ((SRAReader*)self)->read_len_size = &col_read_len->size;
    }
    return rc;
}

rc_t SRAReaderMake(const SRAReader** self, size_t size, const SRATable* table,
                   const char* accession, spotid_t minSpotId, spotid_t maxSpotId)
{
    rc_t rc = 0;
    SRAReader* me = NULL;

    if( table == NULL ) {
        rc = RC(rcSRA, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        if( (rc = SRAReaderAlloc(&me, size, accession)) == 0 ) {
            me->table = table;
            if( (rc = SRATableMinSpotId(me->table, &me->minSpotId)) == 0 &&
                (rc = SRATableMaxSpotId(me->table, &me->maxSpotId)) == 0 ) {
                if( rc == 0 && minSpotId > 0 && (rc = SRAReaderSeekSpot(me, minSpotId)) == 0 ) {
                    me->minSpotId = minSpotId;
                }
                if( rc == 0 && maxSpotId > 0 && (rc = SRAReaderSeekSpot(me, maxSpotId)) == 0 ) {
                    me->maxSpotId = maxSpotId;
                }
            }
            /* reset after seekspot to initial state */
            me->spot = 0;
        }
    }
    if( rc != 0 ) {
        SRAReaderWhack(me);
    } else {
        *self = me;
    }
    return rc;
}

rc_t SRAReaderWhack ( const SRAReader* self )
{
    if( self != NULL) {
        SRAReader* me = (SRAReader*)self;
        SRAReader_ColumnsRelease(me);
        free(me->accession);
        free(me);
    }
    return 0;
}

rc_t SRAReader_FindColData ( const SRAReader* self,
        const SRAReaderColumn* definition, const SRAReaderColumn** col, const void*** base )
{
    int i;

    CHECK_SELF(SRAReader);
    if( definition == NULL || (col == NULL && base == NULL) ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcParam, rcNull);
    }
    if( definition->opt != 0 && !(self->options & definition->opt) ) {
        return 0;
    }
    for(i = 0; me->cols[i].name != NULL; i++) {
        if( self->cols[i].col != NULL &&
            strcmp(self->cols[i].name, definition->name) == 0 &&
            strcmp(self->cols[i].datatype, definition->datatype) == 0 ) {
            if( col != NULL ) {
                *col = &self->cols[i];
            } else {
                *base = (const void**)&self->cols[i].base;
            }
            return 0;
        }
    }
    return (definition->opt & SRAREADER_COL_OPTIONAL) ? 0 : RC(rcSRA, rcFormatter, rcAccessing, rcColumn, rcNotFound);
}

rc_t SRAReaderFirstSpot ( const SRAReader* self )
{
    CHECK_SELF(SRAReader);
    return SRAReaderSeekSpot(me, self->minSpotId);
}

rc_t SRAReaderSeekSpot ( const SRAReader* self, spotid_t spot )
{
    rc_t rc = 0;

    CHECK_SELF(SRAReader);

    if( spot < self->minSpotId ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcRow, rcTooShort);
    }
    if( spot > self->maxSpotId ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcRow, rcTooBig);
    }
    if( me->spot != spot ) {
        me->spot = spot;
        if( (rc = SRAReader_ColumnsRead(me)) != 0 ) {
            me->spot = 0;
        }
    }
    return rc;
}

rc_t SRAReaderNextSpot( const SRAReader* self )
{
    rc_t rc = 0;

    CHECK_SELF(SRAReader);
    CHECK_SPOT(*self);

    me->spot++;
    while( me->spot <= self->maxSpotId ) {
        if( (rc = SRAReader_ColumnsRead(me)) == 0 ) {
            return 0;
        }
        if( GetRCState(rc) != rcNotFound ) {
            me->spot = 0;
            return rc;
        }
        me->spot++;
    }
    CHECK_SPOT(*self);
    return 0;
}

rc_t SRAReaderCurrentSpot( const SRAReader* self, spotid_t* spot)
{
    CHECK_SELF(SRAReader);
    if( spot == NULL ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcParam, rcNull);
    }
    CHECK_SPOT(*self);
    *spot = me->spot;
    return 0;
}

rc_t SRAReader_SpotInfo( const SRAReader* self, const char** spotname, size_t* spotname_sz,
                                                uint32_t* spot_len, uint32_t* num_reads )
{
    CHECK_SELF(SRAReader);
    CHECK_SPOT(*self);

    if( spotname && !spotname_sz ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcParam, rcInvalid);
    }
    if( spotname ) {
        if( me->spot_name ) {
            *spotname = me->spot_name->base;
            *spotname_sz = me->spot_name->size;
	        if( *spotname && *spotname_sz && (*spotname)[*spotname_sz - 1] == '\0' ) {
	            /* Consider zero-padded spotname */
	            *spotname_sz = strlen(*spotname);
	        }
        } else {
            *spotname = "";
            *spotname_sz = 0;
        }
    }
    if( spot_len ) {
        *spot_len = **self->spot_len;
    }
    if( num_reads ) {
        *num_reads = (*self->read_len_size) / sizeof(**self->read_len);
    }
    return 0;
}

rc_t SRAReader_SpotReadInfo( const SRAReader* self, uint32_t readId, SRAReadTypes* read_type,
                             const char** read_label, INSDC_coord_len* read_label_sz,
                             INSDC_coord_zero* read_start, INSDC_coord_len* read_len )
{
    CHECK_SELF(SRAReader);
    CHECK_SPOT(*self);

    if( read_label && !read_label_sz ) {
        return RC(rcSRA, rcFormatter, rcAccessing, rcParam, rcInvalid);
    }
    if( readId < 1 || readId > ((*(me->read_len_size)) / sizeof(**self->read_len)) ) {
        return RC(rcSRA, rcTable, rcReading, rcId, rcInvalid);
    }
    --readId;
    if( read_start ) {
        *read_start = (*self->read_start)[readId];
    }
    if( read_len ) {
        *read_len = (*self->read_len)[readId];
    }
    if( read_type ) {
        *read_type = (*self->read_type)[readId];
    }
    if( read_label ) {
        if( self->label && self->label_start && self->label_len ) {
            *read_label = &((*self->label)[(*self->label_start)[readId]]);
            *read_label_sz = (*self->label_len)[readId];
        } else {
            *read_label = "";
            *read_label_sz = 0;
        }
    }
    return 0;
}

rc_t SRAReader_GetCurrentSpotData( const SRAReader* self, char *data, size_t dsize, size_t* written )
{
    return RC(rcSRA, rcFormatter, rcReading, rcInterface, rcIncomplete);
}

rc_t SRAReader_GetNextSpotData( const SRAReader* self, char *data, size_t dsize, size_t* written )
{
    return RC(rcSRA, rcFormatter, rcReading, rcInterface, rcIncomplete);
}
