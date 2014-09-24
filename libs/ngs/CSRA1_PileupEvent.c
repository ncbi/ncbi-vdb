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

#include "CSRA1_PileupEvent.h"

typedef struct CSRA1_PileupEvent CSRA1_PileupEvent;
#define NGS_PILEUPEVENT CSRA1_PileupEvent
#include "NGS_PileupEvent.h"

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <klib/printf.h>

#include "NGS_String.h"

#include <sysalloc.h>


struct CSRA1_PileupEvent
{
    NGS_PileupEvent  dad;   
    const NGS_String * ref_spec;
    
    bool primary;
    bool secondary;
};

static void             CSRA1_PileupEventWhack                      ( CSRA1_PileupEvent * self, ctx_t ctx );
struct NGS_String *     CSRA1_PileupEventGetReferenceSpec           ( const NGS_PileupEvent * self, ctx_t ctx );
int64_t                 CSRA1_PileupEventGetReferencePosition       ( const NGS_PileupEvent * self, ctx_t ctx );
int                     CSRA1_PileupEventGetMappingQuality          ( const NGS_PileupEvent * self, ctx_t ctx );
struct NGS_String *     CSRA1_PileupEventGetAlignmentId             ( const NGS_PileupEvent * self, ctx_t ctx );
struct NGS_Alignment *  CSRA1_PileupEventGetAlignment               ( const NGS_PileupEvent * self, ctx_t ctx );
int64_t                 CSRA1_PileupEventGetAlignmentPosition       ( const NGS_PileupEvent * self, ctx_t ctx );
int64_t                 CSRA1_PileupEventGetFirstAlignmentPosition  ( const NGS_PileupEvent * self, ctx_t ctx );
int64_t                 CSRA1_PileupEventGetLastAlignmentPosition   ( const NGS_PileupEvent * self, ctx_t ctx );
int                     CSRA1_PileupEventGetEventType               ( const NGS_PileupEvent * self, ctx_t ctx );
char                    CSRA1_PileupEventGetAlignmentBase           ( const NGS_PileupEvent * self, ctx_t ctx );
char                    CSRA1_PileupEventGetAlignmentQuality        ( const NGS_PileupEvent * self, ctx_t ctx );
struct NGS_String *     CSRA1_PileupEventGetInsertionBases          ( const NGS_PileupEvent * self, ctx_t ctx );
struct NGS_String *     CSRA1_PileupEventGetInsertionQualities      ( const NGS_PileupEvent * self, ctx_t ctx );
unsigned int            CSRA1_PileupEventGetDeletionCount           ( const NGS_PileupEvent * self, ctx_t ctx );
bool                    CSRA1_PileupEventIteratorNext               ( const NGS_PileupEvent * self, ctx_t ctx );    

static NGS_PileupEvent_vt CSRA1_PileupEvent_vt_inst =
{
    {
        /* NGS_Refcount */
        CSRA1_PileupEventWhack
    },

    CSRA1_PileupEventGetReferenceSpec,
    CSRA1_PileupEventGetReferencePosition,
    CSRA1_PileupEventGetMappingQuality,
    CSRA1_PileupEventGetAlignmentId,
    CSRA1_PileupEventGetAlignment,
    CSRA1_PileupEventGetAlignmentPosition,
    CSRA1_PileupEventGetFirstAlignmentPosition,
    CSRA1_PileupEventGetLastAlignmentPosition,
    CSRA1_PileupEventGetEventType,
    CSRA1_PileupEventGetAlignmentBase,
    CSRA1_PileupEventGetAlignmentQuality,
    CSRA1_PileupEventGetInsertionBases,
    CSRA1_PileupEventGetInsertionQualities,
    CSRA1_PileupEventGetDeletionCount,
    
    CSRA1_PileupEventIteratorNext,
};

void CSRA1_PileupEventInit ( CSRA1_PileupEvent * self, 
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
    
    TRY ( NGS_PileupEventInit ( ctx, & self -> dad, & CSRA1_PileupEvent_vt_inst, clsname, instname ) ) 
    {
        self -> ref_spec = NGS_StringDuplicate ( ref_spec, ctx );
        self -> primary = wants_primary;
        self -> secondary = wants_secondary;
    }
}

static void CSRA1_PileupEventWhack ( CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcDestroying );
    UNIMPLEMENTED();
}

struct NGS_String * CSRA1_PileupEventGetReferenceSpec ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

int64_t CSRA1_PileupEventGetReferencePosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

int CSRA1_PileupEventGetMappingQuality ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

struct NGS_String * CSRA1_PileupEventGetAlignmentId ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

struct NGS_Alignment * CSRA1_PileupEventGetAlignment ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

int64_t CSRA1_PileupEventGetAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

int64_t CSRA1_PileupEventGetFirstAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

int64_t CSRA1_PileupEventGetLastAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

int CSRA1_PileupEventGetEventType ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

char CSRA1_PileupEventGetAlignmentBase ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

char CSRA1_PileupEventGetAlignmentQuality ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

struct NGS_String * CSRA1_PileupEventGetInsertionBases ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

struct NGS_String * CSRA1_PileupEventGetInsertionQualities ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

unsigned int CSRA1_PileupEventGetDeletionCount ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

bool CSRA1_PileupEventIteratorNext ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return false;
}

struct NGS_PileupEvent* CSRA1_PileupEventIteratorMake ( ctx_t ctx, const NGS_String* ref_spec, bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    CSRA1_PileupEvent * ref;
    
    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_PileupEventIterator on '%.*s'", 
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
        TRY ( CSRA1_PileupEventInit ( ref, ctx, "CSRA1_PileupEvent", instname, ref_spec, wants_primary, wants_secondary ) )
        {
            return ( NGS_PileupEvent* ) ref;
        }
        free ( ref );
    }

    return NULL;
}

