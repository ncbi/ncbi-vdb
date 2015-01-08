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

typedef struct CSRA1_PileupEvent CSRA1_PileupEvent;
#define NGS_PILEUPEVENT CSRA1_PileupEvent
#include "NGS_PileupEvent.h"

#include "CSRA1_PileupEvent.h"
#include "CSRA1_Pileup.h"
#include "NGS_Pileup.h"

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <klib/printf.h>

#include "NGS_String.h"
#include "NGS_Pileup.h"

#include <sysalloc.h>

struct CSRA1_PileupEvent
{
    NGS_PileupEvent dad;
};

static void                    CSRA1_PileupEventWhack                      ( CSRA1_PileupEvent * self, ctx_t ctx );
static struct NGS_String *     CSRA1_PileupEventGetReferenceSpec           ( const NGS_PileupEvent * self, ctx_t ctx );
static int64_t                 CSRA1_PileupEventGetReferencePosition       ( const NGS_PileupEvent * self, ctx_t ctx );
static int                     CSRA1_PileupEventGetMappingQuality          ( const NGS_PileupEvent * self, ctx_t ctx );
static struct NGS_String *     CSRA1_PileupEventGetAlignmentId             ( const NGS_PileupEvent * self, ctx_t ctx );
static struct NGS_Alignment *  CSRA1_PileupEventGetAlignment               ( const NGS_PileupEvent * self, ctx_t ctx );
static int64_t                 CSRA1_PileupEventGetAlignmentPosition       ( const NGS_PileupEvent * self, ctx_t ctx );
static int64_t                 CSRA1_PileupEventGetFirstAlignmentPosition  ( const NGS_PileupEvent * self, ctx_t ctx );
static int64_t                 CSRA1_PileupEventGetLastAlignmentPosition   ( const NGS_PileupEvent * self, ctx_t ctx );
static int                     CSRA1_PileupEventGetEventType               ( const NGS_PileupEvent * self, ctx_t ctx );
static char                    CSRA1_PileupEventGetAlignmentBase           ( const NGS_PileupEvent * self, ctx_t ctx );
static char                    CSRA1_PileupEventGetAlignmentQuality        ( const NGS_PileupEvent * self, ctx_t ctx );
static struct NGS_String *     CSRA1_PileupEventGetInsertionBases          ( const NGS_PileupEvent * self, ctx_t ctx );
static struct NGS_String *     CSRA1_PileupEventGetInsertionQualities      ( const NGS_PileupEvent * self, ctx_t ctx );
static unsigned int            CSRA1_PileupEventGetRepeatCount             ( const NGS_PileupEvent * self, ctx_t ctx );
static int                     CSRA1_PileupEventGetIndelType               ( const NGS_PileupEvent * self, ctx_t ctx );
static bool                    CSRA1_PileupEventIteratorNext               ( NGS_PileupEvent * self, ctx_t ctx );    

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
    CSRA1_PileupEventGetRepeatCount,
    CSRA1_PileupEventGetIndelType,
    
    CSRA1_PileupEventIteratorNext,
};

static
void CSRA1_PileupEventInit ( CSRA1_PileupEvent * self, 
                             ctx_t ctx, 
                             const char *clsname, 
                             const char *instname, 
                             struct NGS_Pileup* pileup )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    assert ( self != NULL );
    
    NGS_PileupEventInit ( ctx, & self -> dad, & CSRA1_PileupEvent_vt_inst, clsname, instname, pileup );
}

static
void CSRA1_PileupEventWhack ( CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcDestroying );
    NGS_PileupEventWhack ( & self -> dad, ctx );
}

static
struct NGS_String * CSRA1_PileupEventGetReferenceSpec ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    /*
      reach into Pileup,
      reach into Reference
     */
    UNIMPLEMENTED();
    return NULL;
}

static
int64_t CSRA1_PileupEventGetReferencePosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
int CSRA1_PileupEventGetMappingQuality ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
struct NGS_String * CSRA1_PileupEventGetAlignmentId ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

static
struct NGS_Alignment * CSRA1_PileupEventGetAlignment ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

static
int64_t CSRA1_PileupEventGetAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
int64_t CSRA1_PileupEventGetFirstAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
int64_t CSRA1_PileupEventGetLastAlignmentPosition ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
int CSRA1_PileupEventGetEventType ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
char CSRA1_PileupEventGetAlignmentBase ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
char CSRA1_PileupEventGetAlignmentQuality ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionBases ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionQualities ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return NULL;
}

static
unsigned int CSRA1_PileupEventGetRepeatCount ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
int CSRA1_PileupEventGetIndelType ( const NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return 0;
}

static
bool CSRA1_PileupEventIteratorNext ( NGS_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    UNIMPLEMENTED();
    return false;
}

struct NGS_PileupEvent* CSRA1_PileupEventIteratorMake ( ctx_t ctx, struct NGS_Pileup * pileup )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    CSRA1_PileupEvent * ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
    {
        NGS_String* ref_spec = NGS_PileupGetReferenceSpec ( pileup, ctx );
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_PileupEventIterator on '%.*s'", 
                       NGS_StringSize ( ref_spec, ctx ), 
                       NGS_StringData ( ref_spec, ctx ) );
        NGS_StringRelease ( ref_spec, ctx );
    }
    else
    {
#if _DEBUGGING
        NGS_String * ref_spec = NGS_PileupGetReferenceSpec ( pileup, ctx );
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( ref_spec, ctx ), 
                        NGS_StringData ( ref_spec, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
        NGS_StringRelease ( ref_spec, ctx );
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_PileupEventInit ( ref, ctx, "CSRA1_PileupEvent", instname, pileup ) )
        {
            return & ref -> dad;
        }
        free ( ref );
    }

    return NULL;
}

