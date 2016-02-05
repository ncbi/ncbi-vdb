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

#include "SRA_ReferenceSequence.h"

typedef struct SRA_ReferenceSequence SRA_ReferenceSequence;
#define NGS_REFERENCESEQUENCE SRA_ReferenceSequence
#include "NGS_ReferenceSequence.h"

#include "NGS_String.h"
#include "NGS_Cursor.h"

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <kns/http.h>
#include <kns/stream.h>
#include <kns/manager.h>

#include <vdb/manager.h>
#include <vdb/table.h>
#include <vdb/database.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>

#include <stddef.h>
#include <assert.h>

#include <strtol.h>
#include <string.h>

#include <sysalloc.h>

/*--------------------------------------------------------------------------
 * SRA_ReferenceSequence
 */

static void                SRA_ReferenceSequenceWhack ( SRA_ReferenceSequence * self, ctx_t ctx );
static NGS_String *        SRA_ReferenceSequenceGetCanonicalName ( SRA_ReferenceSequence * self, ctx_t ctx );
static bool                SRA_ReferenceSequenceGetIsCircular ( SRA_ReferenceSequence const* self, ctx_t ctx );
static uint64_t            SRA_ReferenceSequenceGetLength ( SRA_ReferenceSequence * self, ctx_t ctx );
static struct NGS_String * SRA_ReferenceSequenceGetBases ( SRA_ReferenceSequence * self, ctx_t ctx, uint64_t offset, uint64_t size );
static struct NGS_String * SRA_ReferenceSequenceGetChunk ( SRA_ReferenceSequence * self, ctx_t ctx, uint64_t offset, uint64_t size );

static NGS_ReferenceSequence_vt SRA_ReferenceSequence_vt_inst =
{
    /* NGS_Refcount */
    { SRA_ReferenceSequenceWhack },
    
    /* NGS_ReferenceSequence */
    SRA_ReferenceSequenceGetCanonicalName,
    SRA_ReferenceSequenceGetIsCircular,
    SRA_ReferenceSequenceGetLength,
    SRA_ReferenceSequenceGetBases,
    SRA_ReferenceSequenceGetChunk,
};


struct SRA_ReferenceSequence
{
    NGS_ReferenceSequence dad;

    const VTable * tbl;
    const struct NGS_Cursor * curs;

    uint32_t chunk_size;
    
    int64_t first_row;
    int64_t last_row;  /* inclusive */
    uint64_t cur_length; /* size of current reference in bases (0 = not yet counted) */

    bool is_ebi_reference;  /* the reference has not been found in VDB
                               but it has been found in EBI - it's contained in
                               buf_ref_data
                            */
    char* buf_ref_data;     /* contains reference data if the reference
                               has not been found in VDB
                            */
    NGS_String* ebi_ref_spec;
};

static char const* g_ReferenceTableColumnNames [] =
{
    "(bool)CIRCULAR",
    /*"(utf8)NAME",*/
    "(ascii)SEQ_ID",
    "(INSDC:coord:len)SEQ_LEN",
    /*"(INSDC:coord:one)SEQ_START",*/
    "(U32)MAX_SEQ_LEN",
    "(ascii)READ",
    /*"(I64)PRIMARY_ALIGNMENT_IDS",
    "(I64)SECONDARY_ALIGNMENT_IDS",
    "(INSDC:coord:len)OVERLAP_REF_LEN",
    "(INSDC:coord:zero)OVERLAP_REF_POS"*/
};

enum g_ReferenceTableColumns
{
    reference_CIRCULAR,
    /*reference_NAME,*/
    reference_SEQ_ID,
    reference_SEQ_LEN,
    /*reference_SEQ_START,*/
    reference_MAX_SEQ_LEN,
    reference_READ,
    /*reference_PRIMARY_ALIGNMENT_IDS,
    reference_SECONDARY_ALIGNMENT_IDS,
    reference_OVERLAP_REF_LEN,
    reference_OVERLAP_REF_POS,*/

    reference_NUM_COLS
};


static
void SRA_ReferenceSequenceWhack ( SRA_ReferenceSequence * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcClosing );

    NGS_CursorRelease ( self -> curs, ctx );
    VTableRelease ( self -> tbl );

    self -> curs = NULL;
    self -> tbl = NULL;

    if ( self->buf_ref_data != NULL )
    {
        free ( self->buf_ref_data );
        self->buf_ref_data = NULL;
        self->cur_length = 0;
    }

    self -> is_ebi_reference = false;
    NGS_StringRelease ( self -> ebi_ref_spec, ctx );
}

/* Init
 */
static
void SRA_ReferenceSequenceInit ( ctx_t ctx, 
                           SRA_ReferenceSequence * ref,
                           const char *clsname, 
                           const char *instname )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcOpening );

    if ( ref == NULL )
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    else
    {
        TRY ( NGS_ReferenceSequenceInit ( ctx, & ref -> dad, & SRA_ReferenceSequence_vt_inst, clsname, instname ) )
        {
            /* TODO: maybe initialize more*/
        }
    }
}

static bool is_md5 ( const char * spec )
{
    size_t char_count = 32;
    const char allowed_chars[] = "0123456789abcdefABCDEF";

    size_t i;
    for ( i = 0; spec [i] != '\0' && i < char_count; ++i )
    {
        if ( strchr ( allowed_chars, spec[i] ) == NULL )
        {
            return false;
        }
    }

    return i == char_count;
}

static rc_t NGS_ReferenceSequenceComposeEBIUrl ( ctx_t ctx, const char * spec, bool ismd5, char* url, size_t url_size )
{
    char const url_templ_md5[] = "http://www.ebi.ac.uk/ena/cram/md5/%s";
    char const url_templ_acc[] = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nucleotide&rettype=fasta&id=%s";

    size_t num_written = 0;
    rc_t rc = string_printf ( url, url_size, & num_written, ismd5 ? url_templ_md5 : url_templ_acc, spec );

    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcStorageExhausted, "insufficient url buffer for NGS_ReferenceSequenceComposeEBIUrl" );
    }

    return rc;
}

static rc_t NGS_ReferenceSequenceEBIInitReference (
    ctx_t ctx, bool ismd5, SRA_ReferenceSequence * ref,
    const char* ebi_data, size_t ebi_data_size)
{
    rc_t rc = 0;
    ref -> buf_ref_data = malloc ( ebi_data_size );
    if ( ref -> buf_ref_data == NULL )
        return RC ( rcRuntime, rcBuffer, rcAllocating, rcMemory, rcExhausted );

    ref -> is_ebi_reference = true;

    if ( ismd5 )
    {
        memcpy ( ref->buf_ref_data, ebi_data, ebi_data_size );
        ref -> cur_length = ebi_data_size;
    }
    else
    {
        size_t i, i_dst;

        /* this is FASTA file - skip first line and parse out all other '\n' */

        /* 1. skip 1st line */
        for ( i = 0; i < ebi_data_size && ebi_data [i] != '\0'; ++i )
        {
            if ( ebi_data [i] == '\n' )
            {
                ++i;
                break;
            }
        }

        if ( i == ebi_data_size || ebi_data [i] == '\0' )
            return RC ( rcText, rcDoc, rcParsing, rcFormat, rcInvalid );

        /* copy everything except '\n' to the reference buffer */

        i_dst = 0;
        for (; i < ebi_data_size && ebi_data [i] != '\0'; ++i)
        {
            if ( ebi_data [i] != '\n' )
                ref->buf_ref_data [i_dst++] = ebi_data [i];
        }
        ref -> cur_length = i_dst;
    }

    return rc;
}


#define URL_SIZE 512

static rc_t NGS_ReferenceSequenceOpenEBI ( ctx_t ctx, const char * spec, SRA_ReferenceSequence * ref )
{
    rc_t rc = 0;
    KDataBuffer result;
    KHttpRequest *req = NULL;
    KHttpResult *rslt = NULL;
    bool ismd5 = is_md5 ( spec );
    KNSManager * mgr;

    size_t const url_size = URL_SIZE;
    char url_request [ URL_SIZE ];

    rc = KNSManagerMake ( & mgr );
    if ( rc != 0 )
        return rc;

    memset(&result, 0, sizeof result);
    rc = NGS_ReferenceSequenceComposeEBIUrl ( ctx, spec, ismd5, url_request, url_size );

    if ( rc == 0 )
        rc = KNSManagerMakeRequest (mgr, &req, 0x01010000, NULL,url_request);

    if ( rc == 0 )
        rc = KHttpRequestGET(req, &rslt);

    if ( rc == 0 )
    {
        uint32_t code = 0;
        rc = KHttpResultStatus(rslt, &code, NULL, 0, NULL);
        if (rc == 0 && code != 200)
            rc = RC(rcNS, rcFile, rcReading, rcFile, rcInvalid);
    }

    if ( rc == 0 )
    {
        size_t total = 0;
        KStream *response = NULL;
        rc = KHttpResultGetInputStream(rslt, &response);
        if (rc == 0)
            rc = KDataBufferMakeBytes(&result, 1024);

        while (rc == 0)
        {
            size_t num_read = 0;
            uint8_t *base = NULL;
            uint64_t avail = result.elem_count - total;
            if (avail < 256)
            {
                rc = KDataBufferResize(&result, result.elem_count + 1024);
                if (rc != 0)
                    break;
            }
            base = result.base;
            rc = KStreamRead(response, &base[total], result.elem_count - total, &num_read);
            if (rc != 0)
            {
                /* TBD - look more closely at rc */
                if (num_read > 0)
                    rc = 0;
                else
                    break;
            }
            if (num_read == 0)
                break;

            total += num_read;
        }
        KStreamRelease ( response );
        if (rc == 0)
        {
            result.elem_count = total;
        }
    }

    if ( rc == 0 )
    {
        const char* start = (const char*) result.base;
        size_t size = KDataBufferBytes ( & result );

        rc = NGS_ReferenceSequenceEBIInitReference ( ctx, ismd5, ref, start, size );
        if (rc == 0)
            ref->ebi_ref_spec = NGS_StringMakeCopy ( ctx, spec, strlen(spec) );
    }

    /* TODO: release only if they were allocated */
    KDataBufferWhack ( &result );
    KHttpResultRelease ( rslt );
    KHttpRequestRelease ( req );

    KNSManagerRelease ( mgr );

    return rc;
}

NGS_ReferenceSequence * NGS_ReferenceSequenceMakeSRA ( ctx_t ctx, const char * spec )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcOpening );

    SRA_ReferenceSequence * ref;

    assert ( spec != NULL );
    assert ( spec [0] != '\0' );

    ref = calloc ( 1, sizeof *ref );
    if ( ref == NULL )
    {
        SYSTEM_ERROR ( xcNoMemory, "allocating SRA_ReferenceSequence ( '%s' )", spec );
    }
    else
    {
        TRY ( SRA_ReferenceSequenceInit ( ctx, ref, "NGS_ReferenceSequence", spec ) )
        {
            rc_t rc;

            const VDBManager * mgr = ctx -> rsrc -> vdb;
            assert ( mgr != NULL );

            rc = VDBManagerOpenTableRead ( mgr, & ref -> tbl, NULL, spec );
            if ( GetRCState ( rc ) == rcNotFound )
            {
                rc_t rc_vdb = rc;
                rc = NGS_ReferenceSequenceOpenEBI ( ctx, spec, ref );
                if ( rc != 0 )
                {
                    /*CLEAR();*/
                    rc = rc_vdb;
                    SET_RC_FILE_FUNC_LINE ( rc );
                    INTERNAL_ERROR ( xcUnexpected, "failed to open table '%s': rc = %R", spec, rc );
                }
                else
                    return (NGS_ReferenceSequence*) ref;
            }
            else if ( rc != 0 )
            {
                INTERNAL_ERROR ( xcUnexpected, "failed to open table '%s': rc = %R", spec, rc );
            }
            else
            {   /* VDB-2641: examine the schema name to make sure this is an SRA table */
                char ts_buff[1024];
                rc = VTableTypespec ( ref -> tbl, ts_buff, sizeof ( ts_buff ) );
                if ( rc != 0 )
                {
                    INTERNAL_ERROR ( xcUnexpected, "VTableTypespec failed: rc = %R", rc );
                }
                else
                {
                    const char REF_PREFIX[] = "NCBI:refseq:";
                    size_t pref_size = sizeof ( REF_PREFIX ) - 1;
                    if ( string_match ( REF_PREFIX, pref_size, ts_buff, string_size ( ts_buff ), (uint32_t)pref_size, NULL ) != pref_size )
                    {
                        INTERNAL_ERROR ( xcUnimplemented, "Cannot open accession '%s' as a reference table.", spec );
                    }
                    else
                    {
                        ref -> curs = NGS_CursorMake ( ctx, ref -> tbl, g_ReferenceTableColumnNames, reference_NUM_COLS );
                        if ( ref -> curs != NULL )
                        {
                            uint64_t row_count = 0;
                            TRY ( NGS_CursorGetRowRange ( ref->curs, ctx, & ref -> first_row, & row_count ) )
                            {
                                ref -> last_row = ref -> first_row + (int64_t) row_count - 1; /* TODO: it might be incorrect in general case */
                                TRY ( ref -> chunk_size = NGS_CursorGetUInt32 ( ref -> curs, ctx, ref -> first_row, reference_MAX_SEQ_LEN ) )
                                {
                                    return (NGS_ReferenceSequence*) ref;
                                }
                            }
                        }
                    }
                }
            }
            SRA_ReferenceSequenceWhack ( ref , ctx );
        }
        free ( ref );
    }
    return NULL;
}


int64_t SRA_ReferenceSequence_GetFirstRowId ( const struct NGS_ReferenceSequence * self, ctx_t ctx )
{
    assert ( ( void * ) self -> dad . vt == ( void * ) & SRA_ReferenceSequence_vt_inst );
    return ( ( SRA_ReferenceSequence const * ) self ) -> first_row;
}

int64_t SRA_ReferenceSequence_GetLastRowId ( const struct NGS_ReferenceSequence * self, ctx_t ctx )
{
    assert ( ( void * ) self -> dad . vt == ( void * ) & SRA_ReferenceSequence_vt_inst );
    return ( ( SRA_ReferenceSequence const * ) self ) -> last_row;
}

NGS_String * SRA_ReferenceSequenceGetCanonicalName ( SRA_ReferenceSequence * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    assert ( self != NULL );
    
    if ( self -> is_ebi_reference )
        return NGS_StringDuplicate ( self -> ebi_ref_spec, ctx );
    else
        return NGS_CursorGetString ( self -> curs, ctx, self -> first_row, reference_SEQ_ID);
}

bool SRA_ReferenceSequenceGetIsCircular ( const SRA_ReferenceSequence * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
   
    if ( self -> curs == NULL && !self -> is_ebi_reference )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return false;
    }

    /* if current row is valid, read data */
    if ( self -> first_row <= self -> last_row && !self -> is_ebi_reference )
    {
        return NGS_CursorGetBool ( self -> curs, ctx, self -> first_row, reference_CIRCULAR );
    }

    return false;
}

uint64_t SRA_ReferenceSequenceGetLength ( SRA_ReferenceSequence * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL && !self -> is_ebi_reference)
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return 0;
    }
    
    if ( self -> cur_length == 0 ) /* not yet calculated */
    {   
        assert ( ! self -> is_ebi_reference );
        self -> cur_length =  self -> chunk_size * ( self -> last_row - self -> first_row ) + 
                              NGS_CursorGetUInt32 ( self -> curs, 
                                                    ctx, 
                                                    self -> last_row, 
                                                    reference_SEQ_LEN );
    }
    
    return self -> cur_length;
}

struct NGS_String * SRA_ReferenceSequenceGetBases ( SRA_ReferenceSequence * self, ctx_t ctx, uint64_t offset, uint64_t size )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL && !self -> is_ebi_reference )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    
    {
        uint64_t totalBases = SRA_ReferenceSequenceGetLength ( self, ctx );
        if ( offset >= totalBases )
        {
            return NGS_StringMake ( ctx, "", 0 );
        }
        else
        {   
            uint64_t basesToReturn = totalBases - offset;
            char* data;
            
            if (size != (size_t)-1 && basesToReturn > size)
                basesToReturn = size;
                
            data = (char*) malloc ( basesToReturn );
            if ( data == NULL )
            {
                SYSTEM_ERROR ( xcNoMemory, "allocating %lu bases", basesToReturn );
                return NGS_StringMake ( ctx, "", 0 );
            }
            else
            {
                if ( self -> is_ebi_reference )
                {
                    return NGS_StringMakeCopy ( ctx, (const char*) self -> buf_ref_data + offset, basesToReturn );
                }
                else
                {
                    size_t cur_offset = 0;
                    while ( cur_offset < basesToReturn )
                    {
                        /* we will potentially ask for more than available in the current chunk; 
                            SRA_ReferenceSequenceGetChunkSize will return only as much as is available in the chunk */
                        NGS_String* chunk = SRA_ReferenceSequenceGetChunk ( self, ctx, offset + cur_offset, basesToReturn - cur_offset );
                        cur_offset += string_copy(data + cur_offset, basesToReturn - cur_offset, 
                                                  NGS_StringData ( chunk, ctx ), NGS_StringSize ( chunk, ctx ) );
                        NGS_StringRelease ( chunk, ctx );
                    }
                    return NGS_StringMakeOwned ( ctx, data, basesToReturn );
                }
            }
        }
    }
}

struct NGS_String * SRA_ReferenceSequenceGetChunk ( SRA_ReferenceSequence * self, ctx_t ctx, uint64_t offset, uint64_t size )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL && !self -> is_ebi_reference)
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }

    if ( self -> is_ebi_reference )
    {
        return SRA_ReferenceSequenceGetBases ( self, ctx, offset, size );
    }
    
    if ( offset >= SRA_ReferenceSequenceGetLength ( self, ctx ) )
    {
        return NGS_StringMake ( ctx, "", 0 );
    }
    else
    {
        const NGS_String* read = NGS_CursorGetString ( self -> curs, ctx, self -> first_row + offset / self -> chunk_size, reference_READ);
        NGS_String* ret;
        if ( size == (size_t)-1 )
            ret = NGS_StringSubstrOffset ( read, ctx, offset % self -> chunk_size );
        else
            ret = NGS_StringSubstrOffsetSize ( read, ctx, offset % self -> chunk_size, size );
        NGS_StringRelease ( read, ctx );
        return ret;
    }
}
