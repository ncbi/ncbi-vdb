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

typedef struct CSRA1_ReadCollection CSRA1_ReadCollection;
#define NGS_READCOLLECTION CSRA1_ReadCollection

#include "NGS_ReadCollection.h"

#include "CSRA1_ReadCollection.h"

#include "NGS_Reference.h"
#include "NGS_Alignment.h"
#include "NGS_Read.h"

#include "NGS_Cursor.h"
#include "NGS_String.h"
#include "NGS_Id.h"

#include "SRA_Read.h"
#include "SRA_ReadGroup.h"
#include "SRA_ReadGroupInfo.h"
#include "SRA_Statistics.h"

#include "CSRA1_Alignment.h"
#include "CSRA1_Reference.h"

#include <klib/rc.h>
#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <kdb/meta.h>

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>

#include <strtol.h> /* for strtoi64 */

#include <stddef.h>
#include <assert.h>

#include <sysalloc.h>


/*--------------------------------------------------------------------------
 * CSRA1_ReadCollection
 */

struct CSRA1_ReadCollection
{
    NGS_ReadCollection dad;
    const NGS_String * run_name;
    
    const VDatabase * db;
    
    /* shared cursors (reused for all non-iterating objects) */
    const NGS_Cursor* reference_curs;
    const NGS_Cursor* sequence_curs;
    const NGS_Cursor* primary_al_curs;
    const NGS_Cursor* secondary_al_curs; 
    bool has_secondary;
    
    uint64_t primaryId_count;
    
    const struct SRA_ReadGroupInfo* group_info;
};

/* REFCOUNT */

struct CSRA1_ReadCollection * CSRA1_ReadCollectionDuplicate ( struct CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    assert ( self != NULL );
    return NGS_RefcountDuplicate ( & self -> dad . dad, ctx );
}

void CSRA1_ReadCollectionRelease ( struct CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    assert ( self != NULL );
    NGS_RefcountRelease( & self -> dad . dad, ctx );
}

struct NGS_ReadCollection * CSRA1_ReadCollectionToNGS_ReadCollection ( struct CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    assert ( self != NULL );
    return & self -> dad;
}

static
void CSRA1_ReadCollectionWhack ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    NGS_CursorRelease ( self -> secondary_al_curs, ctx );
    NGS_CursorRelease ( self -> primary_al_curs, ctx );
    NGS_CursorRelease ( self -> sequence_curs, ctx );
    NGS_CursorRelease ( self -> reference_curs, ctx );
    SRA_ReadGroupInfoRelease ( self -> group_info, ctx );
    NGS_StringRelease ( self -> run_name, ctx );
    VDatabaseRelease ( self -> db );
}

static const char * align_col_specs [] =
{
    "(I32)MAPQ",
    "(ascii)CIGAR_LONG",
    "(ascii)CIGAR_SHORT",
    "(ascii)CLIPPED_CIGAR_LONG",
    "(ascii)CLIPPED_CIGAR_SHORT",
    "(INSDC:quality:phred)CLIPPED_QUALITY",
    "(INSDC:dna:text)CLIPPED_READ",
    "(INSDC:coord:len)LEFT_SOFT_CLIP",
    "(INSDC:coord:len)RIGHT_SOFT_CLIP",
    "(INSDC:quality:phred)QUALITY",
    "(INSDC:dna:text)RAW_READ",
    "(INSDC:dna:text)READ",
    "(I64)REF_ID",
    "(INSDC:coord:len)REF_LEN",
    "(ascii)REF_SEQ_ID",	/* was REF_NAME changed March 23 2015 */
    "(bool)REF_ORIENTATION",
    "(INSDC:coord:zero)REF_POS",
    "(INSDC:dna:text)REF_READ",
    "(INSDC:quality:text:phred_33)SAM_QUALITY",
    "(INSDC:coord:one)SEQ_READ_ID",
    "(I64)SEQ_SPOT_ID",
    "(ascii)SPOT_GROUP",
    "(I32)TEMPLATE_LEN",
    "(ascii)RNA_ORIENTATION",
    "(I64)MATE_ALIGN_ID",
    "(ascii)MATE_REF_SEQ_ID",	/* was MATE_REF_NAME changed March 23 2015 */
    "(bool)MATE_REF_ORIENTATION",
};

static const char * reference_col_specs [] =
{
    "(bool)CIRCULAR",
    "(utf8)NAME",
    "(ascii)SEQ_ID",
    "(INSDC:coord:len)SEQ_LEN",
    "(INSDC:coord:one)SEQ_START",
    "(U32)MAX_SEQ_LEN",
    "(ascii)READ",
    "(I64)PRIMARY_ALIGNMENT_IDS",
    "(I64)SECONDARY_ALIGNMENT_IDS",
    "(INSDC:coord:len)OVERLAP_REF_LEN",
    "(INSDC:coord:zero)OVERLAP_REF_POS"
};

const NGS_Cursor* CSRA1_ReadCollectionMakeAlignmentCursor ( CSRA1_ReadCollection * self, 
                                                            ctx_t ctx, 
                                                            bool primary, 
                                                            bool exclusive )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    assert ( self );
    
    if ( exclusive )
    {   /* detach and return cached cursor if its refcount == 1 */
        if ( primary )
        {
            NGS_Refcount * refc = ( NGS_Refcount * ) ( self -> primary_al_curs ) ;
            if ( self -> primary_al_curs && atomic32_read ( & refc -> refcount ) == 1 ) 
            {
                const NGS_Cursor* ret = self -> primary_al_curs;
                self -> primary_al_curs = NULL;
                return ret;
            }
        }
        else
        {
            NGS_Refcount * refc = ( NGS_Refcount * ) ( self -> secondary_al_curs ) ;
            if ( self -> secondary_al_curs && atomic32_read ( & refc -> refcount ) == 1 ) 
            {
                const NGS_Cursor* ret = self -> secondary_al_curs;
                self -> secondary_al_curs = NULL;
                return ret;
            }
        }
        return NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, primary ? "PRIMARY_ALIGNMENT" : "SECONDARY_ALIGNMENT", align_col_specs, align_NUM_COLS );
    }
    if ( primary )
    {
        if ( self -> primary_al_curs == NULL )
        {
            self -> primary_al_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "PRIMARY_ALIGNMENT", align_col_specs, align_NUM_COLS );
        }
        return NGS_CursorDuplicate ( self -> primary_al_curs, ctx );
    }
    else if ( self -> has_secondary )
    {
        if ( self -> secondary_al_curs == NULL )
        {
            self -> secondary_al_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SECONDARY_ALIGNMENT", align_col_specs, align_NUM_COLS );
        }
        return NGS_CursorDuplicate ( self -> secondary_al_curs, ctx );
    }
    else
    {
        INTERNAL_ERROR ( xcCursorAccessFailed, "Secondary alignments table is missing");
        return NULL;
    }
}

/* READCOLLECTION */

static
NGS_String * CSRA1_ReadCollectionGetName ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    return NGS_StringDuplicate ( self -> run_name, ctx );
}

static
const struct SRA_ReadGroupInfo* GetGroupInfo ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    if ( self -> group_info == NULL )
    {
        const VTable * table;
        rc_t rc = VDatabaseOpenTableRead ( self -> db, & table, "SEQUENCE" );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcUnexpected, "VDatabaseOpenTableRead(SEQUENCE) rc = %R", rc );
        }
        ON_FAIL ( self -> group_info = SRA_ReadGroupInfoMake ( ctx, table ) )
        {
            VTableRelease ( table );
            return NULL;
        }
        VTableRelease ( table );
    }
    return self -> group_info;
}

static
NGS_ReadGroup * CSRA1_ReadCollectionGetReadGroups ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );

    TRY ( const NGS_Cursor * curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS ) )
    {
        TRY ( GetGroupInfo ( self, ctx ) )
        {
            NGS_ReadGroup * ret = SRA_ReadGroupIteratorMake ( ctx, curs, self -> group_info, self -> run_name );
            NGS_CursorRelease ( curs, ctx );
            return ret;
        }
    }
    return NULL;

}

static
NGS_ReadGroup * CSRA1_ReadCollectionGetReadGroup ( CSRA1_ReadCollection * self, ctx_t ctx, const char * spec )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );

    if ( self -> sequence_curs == NULL )
    {
        self -> sequence_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS );
    }

    TRY ( GetGroupInfo ( self, ctx ) )
    {
        TRY ( NGS_String * name = NGS_StringMakeCopy ( ctx, spec, string_size ( spec ) ) )
        {
            NGS_ReadGroup * ret = SRA_ReadGroupMake ( ctx, self -> sequence_curs, self -> group_info, self -> run_name, name);
            NGS_StringRelease ( name, ctx );
            return ret;
        }
    }
    
    return NULL;
}

static
NGS_Reference * CSRA1_ReadCollectionGetReferences ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );

    /* Iterators have their own cursors */
    TRY ( const NGS_Cursor* curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "REFERENCE", reference_col_specs, reference_NUM_COLS ) )
    {
        NGS_Reference * ret = CSRA1_ReferenceIteratorMake ( ctx, & self -> dad, self -> db, curs, self -> primaryId_count );
        NGS_CursorRelease ( curs, ctx );
        return ret;
    }
    
    return NULL;
}

static
NGS_Reference * CSRA1_ReadCollectionGetReference ( CSRA1_ReadCollection * self, ctx_t ctx, const char * spec )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    
    if ( self -> reference_curs == NULL )
    {
        ON_FAIL ( self -> reference_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "REFERENCE", reference_col_specs, reference_NUM_COLS ) ) 
            return NULL;
    }
    
    return CSRA1_ReferenceMake ( ctx, & self -> dad, self -> db, self -> reference_curs, spec, self -> primaryId_count );
}

static
NGS_Alignment * CSRA1_ReadCollectionGetAlignments ( CSRA1_ReadCollection * self, ctx_t ctx,
    bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    
    return CSRA1_AlignmentIteratorMake ( ctx, self, wants_primary, wants_secondary, self -> run_name, self -> primaryId_count );
}

static
NGS_Alignment * CSRA1_ReadCollectionGetAlignment ( CSRA1_ReadCollection * self, ctx_t ctx, const char * alignmentIdStr )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );

    TRY ( const NGS_String* id_str = NGS_StringMakeCopy ( ctx, alignmentIdStr, string_size ( alignmentIdStr ) ) )
    {
        TRY ( struct NGS_Id id = NGS_IdParse ( id_str, ctx ) )
        {
            if ( string_cmp ( NGS_StringData ( self -> run_name, ctx ), 
                              NGS_StringSize ( self -> run_name, ctx ),
                              id . run . addr, 
                              id . run . size, 
                              id . run . len ) != 0 ) 
            {
                INTERNAL_ERROR ( xcArcIncorrect, 
                                 " expected '%.*s', actual '%.*s'", 
                                 NGS_StringSize ( self -> run_name, ctx ),
                                 NGS_StringData ( self -> run_name, ctx ), 
                                 id . run . size, 
                                 id . run . addr );
            }
            else if ( id . object == NGSObject_PrimaryAlignment )
            {
                NGS_StringRelease ( id_str, ctx );
                return CSRA1_AlignmentMake ( ctx, self, id . rowId, self -> run_name, true, 0 );
            }
            else if ( id . object == NGSObject_SecondaryAlignment )
            {
                NGS_StringRelease ( id_str, ctx );
                return CSRA1_AlignmentMake ( ctx, self, id . rowId, self -> run_name, false, self -> primaryId_count );
            }
            else
            {
                INTERNAL_ERROR ( xcTypeIncorrect, 
                                 " expected alignment (%i or %i), actual %i", 
                                 NGSObject_PrimaryAlignment, 
                                 NGSObject_SecondaryAlignment,
                                 id . object );
            }
        }
        NGS_StringRelease ( id_str, ctx );
    }
    return NULL;
}

static
uint64_t CSRA1_ReadCollectionGetAlignmentCount ( CSRA1_ReadCollection * self, ctx_t ctx,
    bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );
    uint64_t ret = 0;
    if ( wants_primary )
    {
        ret += ( self -> primaryId_count );
    }
    if ( wants_secondary && self -> has_secondary )
    {
        if ( self -> secondary_al_curs == NULL )
        {
            ON_FAIL ( self -> secondary_al_curs = NGS_CursorMakeDb ( ctx, 
                                                                     self -> db, 
                                                                     self -> run_name, 
                                                                     "SECONDARY_ALIGNMENT", 
                                                                     align_col_specs, 
                                                                     align_NUM_COLS ) )
                return 0;
        }
        ret += NGS_CursorGetRowCount ( self -> secondary_al_curs, ctx );
    }
    return ret;
}

static
NGS_Alignment * CSRA1_ReadCollectionGetAlignmentRange ( CSRA1_ReadCollection * self, 
                                                        ctx_t ctx, 
                                                        uint64_t first, 
                                                        uint64_t count,
                                                        bool wants_primary, 
                                                        bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    
    return CSRA1_AlignmentRangeMake ( ctx, self, wants_primary, wants_secondary, self -> run_name, self -> primaryId_count, first, count );
}

struct NGS_Read * CSRA1_ReadCollectionGetReads ( CSRA1_ReadCollection * self, ctx_t ctx,
    bool wants_full, bool wants_partial, bool wants_unaligned )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );

    TRY ( const NGS_Cursor * curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS ) )
    {
        TRY ( NGS_Read * ref = SRA_ReadIteratorMake ( ctx, curs, self -> run_name, wants_full, wants_partial, wants_unaligned ) )
        {
            NGS_CursorRelease ( curs, ctx );
            return ref;
        }

        NGS_CursorRelease ( curs, ctx );
    }

    return NULL;
}

struct NGS_Read * CSRA1_ReadCollectionGetRead ( CSRA1_ReadCollection * self, ctx_t ctx, const char * readIdStr )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );

    TRY ( const NGS_String* id_str = NGS_StringMakeCopy ( ctx, readIdStr, string_size ( readIdStr ) ) )
    {
        TRY ( struct NGS_Id id = NGS_IdParse ( id_str, ctx ) )
        {
            if ( string_cmp ( NGS_StringData ( self -> run_name, ctx ), 
                              NGS_StringSize ( self -> run_name, ctx ),
                              id . run . addr, 
                              id . run . size, 
                              id . run . len ) != 0 ) 
            {
                INTERNAL_ERROR ( xcArcIncorrect, 
                                 " expected '%.*s', actual '%.*s'", 
                                 NGS_StringSize ( self -> run_name, ctx ),
                                 NGS_StringData ( self -> run_name, ctx ), 
                                 id . run . size, 
                                 id . run . addr );
            }    
            else
            {   
                NGS_StringRelease ( id_str, ctx );
                /* individual reads share one iterator attached to ReadCollection */
                if ( self -> sequence_curs == NULL )
                {
                    ON_FAIL ( self -> sequence_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS ) )
                        return NULL;
                }
                return SRA_ReadMake ( ctx, self -> sequence_curs, id . rowId, self -> run_name );
            }
        }
        NGS_StringRelease ( id_str, ctx );
    }
    return NULL;
}

static
uint64_t CSRA1_ReadCollectionGetReadCount ( CSRA1_ReadCollection * self, ctx_t ctx,
    bool wants_full, bool wants_partial, bool wants_unaligned )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcAccessing );
    
    if ( self -> sequence_curs == NULL )
    {
        ON_FAIL ( self -> sequence_curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS ) )
        {
            return 0;
        }
    }

    if ( wants_full && wants_partial && wants_unaligned )    
        return NGS_CursorGetRowCount ( self -> sequence_curs, ctx );

    /* have a problem here */
    UNIMPLEMENTED ();

    return 0;
}

struct NGS_Read * CSRA1_ReadCollectionGetReadRange ( CSRA1_ReadCollection * self, 
                                                     ctx_t ctx, 
                                                     uint64_t first, 
                                                     uint64_t count,
                                                     bool wants_full, 
                                                     bool wants_partial, 
                                                     bool wants_unaligned )
{
    FUNC_ENTRY ( ctx, rcSRA, rcTable, rcAccessing );

    TRY ( const NGS_Cursor* curs = NGS_CursorMakeDb ( ctx, self -> db, self -> run_name, "SEQUENCE", sequence_col_specs, seq_NUM_COLS ) )
    {
        NGS_Read * ret = SRA_ReadIteratorMakeRange ( ctx, curs, self -> run_name, first, count, wants_full, wants_partial, wants_unaligned );
        NGS_CursorRelease ( curs, ctx );
        return ret;
    }
    return NULL;
}

static void LoadTableStats ( CSRA1_ReadCollection * self, ctx_t ctx, const char* table_name, NGS_Statistics * stats )
{
    const VTable * table;
    rc_t rc = VDatabaseOpenTableRead ( self -> db, & table, table_name ); 
    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcUnexpected, "VDatabaseOpenTableRead(%s) rc = %R", table_name, rc );
    }
    else
    {
        SRA_StatisticsLoadTableStats ( stats, ctx, table, table_name );
        VTableRelease ( table );
    }
} 

static struct NGS_Statistics* CSRA1_ReadCollectionGetStatistics ( CSRA1_ReadCollection * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

    TRY ( NGS_Statistics * ret = SRA_StatisticsMake ( ctx ) )
    {
        TRY ( LoadTableStats ( self, ctx, "SEQUENCE", ret ) )
        {
            TRY ( LoadTableStats ( self, ctx, "REFERENCE", ret ) )
            {
                TRY ( LoadTableStats ( self, ctx, "PRIMARY_ALIGNMENT", ret ) )
                {
                    /* SECONDARY_ALIGNMENT may be missing */
                    ON_FAIL ( LoadTableStats ( self, ctx, "SECONDARY_ALIGNMENT", ret ) ) CLEAR ();
                    return ret;
                }
            }
        }
        NGS_StatisticsRelease ( ret, ctx );
    }
    
    return NULL;
}

static NGS_ReadCollection_vt CSRA1_ReadCollection_vt =
{
    /* NGS_Refcount */
    { CSRA1_ReadCollectionWhack },

    /* NGS_ReadCollection */
    CSRA1_ReadCollectionGetName,
    CSRA1_ReadCollectionGetReadGroups,
    CSRA1_ReadCollectionGetReadGroup,
    CSRA1_ReadCollectionGetReferences,
    CSRA1_ReadCollectionGetReference,
    CSRA1_ReadCollectionGetAlignments,
    CSRA1_ReadCollectionGetAlignment,
    CSRA1_ReadCollectionGetAlignmentCount,
    CSRA1_ReadCollectionGetAlignmentRange,
    CSRA1_ReadCollectionGetReads,
    CSRA1_ReadCollectionGetRead,
    CSRA1_ReadCollectionGetReadCount,
    CSRA1_ReadCollectionGetReadRange,
    CSRA1_ReadCollectionGetStatistics
};

NGS_ReadCollection * NGS_ReadCollectionMakeCSRA ( ctx_t ctx, const VDatabase *db, const char * spec )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcConstructing );

    size_t spec_size;
    CSRA1_ReadCollection * ref;

    assert ( db != NULL );

    assert ( spec != NULL );
    spec_size = string_size ( spec );
    assert ( spec_size != 0 );

    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, "allocating CSRA1_ReadCollection ( '%s' )", spec );
    else
    {
        TRY ( NGS_ReadCollectionInit ( ctx, & ref -> dad, & CSRA1_ReadCollection_vt, "CSRA1_ReadCollection", spec ) )
        {
            const char * name, * dot, * end;
            
            ref -> db = db;

            end = & spec [ spec_size ];

            /* TBD - this is a hack */
            name = string_rchr ( spec, spec_size, '/' );
            if ( name == NULL )
                name = spec;

            dot = string_rchr ( name, end - name, '.' );
            if ( dot != NULL )
            {
                if ( strcase_cmp ( dot, end - dot, ".ncbi_enc", sizeof ".ncbi_enc" - 1, -1 ) == 0 )
                {
                    end = dot;
                    dot = string_rchr ( name, end - name, '.' );
                }
                if ( dot != NULL && strcase_cmp ( dot, end - dot, ".sra", sizeof ".sra" - 1, -1 ) == 0 )
                    end = dot;
            }

            /* initialize "run_name" */
            TRY ( ref -> run_name = NGS_StringMakeCopy ( ctx, name, end - name ) )
            {
                TRY ( ref -> primary_al_curs = NGS_CursorMakeDb ( ctx, ref -> db, ref -> run_name, "PRIMARY_ALIGNMENT", align_col_specs, align_NUM_COLS ) )
                {
                    TRY ( ref -> primaryId_count = NGS_CursorGetRowCount ( ref -> primary_al_curs, ctx ) )
                    {   
                        /* check for existence of SECONDARY_ALIGNMENT table */
                        const VTable * table;
                        if ( VDatabaseOpenTableRead ( db, & table, "SECONDARY_ALIGNMENT" ) == 0 )
                        {
                            ref -> has_secondary = true;
                            VTableRelease ( table );
                        }
                        
                        return & ref -> dad;
                    }
                }
            }
            
            CSRA1_ReadCollectionWhack ( ref, ctx );
            return NULL;
        }

        free ( ref );
    }

    VDatabaseRelease ( db );

    return NULL;
}
