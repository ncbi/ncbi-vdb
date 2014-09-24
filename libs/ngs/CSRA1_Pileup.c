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

#include "CSRA1_Pileup.h"

typedef struct CSRA1_Pileup CSRA1_Pileup;
#define NGS_PILEUP CSRA1_Pileup
#include "NGS_Pileup.h"

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <klib/printf.h>

#include "NGS_String.h"

#include <sysalloc.h>

struct CSRA1_Pileup
{
    NGS_Pileup dad;   
    const NGS_String * ref_spec;
    
    bool primary;
    bool secondary;
};

static void                     CSRA1_PileupWhack                   ( CSRA1_Pileup * self, ctx_t ctx );
static struct NGS_String *      CSRA1_PileupGetReferenceSpec        ( const CSRA1_Pileup * self, ctx_t ctx );
static int64_t                  CSRA1_PileupGetReferencePosition    ( const CSRA1_Pileup * self, ctx_t ctx );
static struct NGS_PileupEvent * CSRA1_PileupGetEvents               ( const CSRA1_Pileup * self, ctx_t ctx );
static unsigned int             CSRA1_PileupGetDepth                ( const CSRA1_Pileup * self, ctx_t ctx );
static bool                     CSRA1_PileupIteratorGetNext         ( const CSRA1_Pileup * self, ctx_t ctx );

static NGS_Pileup_vt CSRA1_Pileup_vt_inst =
{
    {
        /* NGS_Refcount */
        CSRA1_PileupWhack
    },

    CSRA1_PileupGetReferenceSpec,    
    CSRA1_PileupGetReferencePosition,
    CSRA1_PileupGetEvents,           
    CSRA1_PileupGetDepth,            
    CSRA1_PileupIteratorGetNext,     
};

void CSRA1_PileupInit ( CSRA1_Pileup * self, 
                        ctx_t ctx, 
                        const char *clsname, 
                        const char *instname, 
                        const NGS_String* ref_spec, 
                        bool wants_primary, 
                        bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    assert ( self );
    assert ( ref_spec );
    
    TRY ( NGS_PileupInit ( ctx, & self -> dad, & CSRA1_Pileup_vt_inst, clsname, instname ) ) 
    {
        self -> ref_spec = NGS_StringDuplicate ( ref_spec, ctx );
        self -> primary = wants_primary;
        self -> secondary = wants_secondary;
    }
}

void CSRA1_PileupWhack ( CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    NGS_StringRelease ( self -> ref_spec, ctx );
}

struct NGS_String * CSRA1_PileupGetReferenceSpec ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED ();
    return NULL;
}

int64_t CSRA1_PileupGetReferencePosition ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED ();
    return 0;
}

struct NGS_PileupEvent * CSRA1_PileupGetEvents ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED ();
    return NULL;
}

unsigned int CSRA1_PileupGetDepth ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED ();
    return 0;
}

bool CSRA1_PileupIteratorGetNext ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED ();
    return false;
}

struct NGS_Pileup* CSRA1_PileupIteratorMake ( ctx_t ctx, const NGS_String* ref_spec, bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    CSRA1_Pileup * ref;
    
    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_Pileup on '%.*s'", 
                       NGS_StringSize ( ref_spec, ctx ), 
                       NGS_StringData ( ref_spec, ctx ) );
    else
    {
#if _DEBUGGING
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( ref_spec, ctx ), 
                        NGS_StringData ( ref_spec, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_PileupInit ( ref, ctx, "CSRA1_Pileup", instname, ref_spec, wants_primary, wants_secondary ) )
        {
            return ( NGS_Pileup* ) ref;
        }
        free ( ref );
    }

    return NULL;
}

