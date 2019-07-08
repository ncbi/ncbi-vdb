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

#ifndef _h_align_iterator_
#define _h_align_iterator_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_insdc_insdc_
#include <insdc/insdc.h>
#endif

#ifndef _h_vdb_database_
#include <vdb/database.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VCursor;
struct AlignMgr;
struct ReferenceObj;


/*--------------------------------------------------------------------------
 * AlignmentIterator
 *  walk across a single alignment in reference space
 */
typedef struct AlignmentIterator AlignmentIterator;


/* Make
 *  create an encapsulation of alignment
 *
 *  iter [ OUT ] - return parameter for new iterator
 *
 *  copy [ IN ] - when "true" the data will be copied.
 *  otherwise, pointers will be expected to refer to data
 *  with a lifetime >= that of the iterator being created.
 *
 *  "ref_pos" [ IN ] and "ref_len" [ IN ] - projection onto reference
 *
 *  "read" [ IN ] and "read_len" [ IN ] - full sequence of alignment
 *
 *  "has_mismatch" [ IN ] - describes comparison result of each base
 *  in "read" against the alignment.
 *
 *  "has_ref_offset" [ IN ] - describes positions of reference offsets
 *
 *  "ref_offset" [ IN ] and "ref_offset_len" [ IN ] - packed array of
 *  offsets of position against reference.
 */
ALIGN_EXTERN rc_t CC AlignMgrMakeAlignmentIterator ( struct AlignMgr const *self,
    AlignmentIterator **iter,
    bool copy,
    INSDC_coord_zero ref_pos,
    INSDC_coord_len ref_len,
    const INSDC_4na_bin *read,
    INSDC_coord_len read_len,
    const bool *has_mismatch,
    const bool *has_ref_offset,
    const int32_t *ref_offset,
    uint32_t ref_offset_len,
    INSDC_coord_zero ref_window_start,
    INSDC_coord_len ref_window_len );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignmentIteratorAddRef ( const AlignmentIterator *self );
ALIGN_EXTERN rc_t CC AlignmentIteratorRelease ( const AlignmentIterator *self );


/* Next
 *  advance position by 1 in reference space
 *  must be called initially to advance to first element
 *
 *  returns RCState = rcDone when done
 */
ALIGN_EXTERN rc_t CC AlignmentIteratorNext ( AlignmentIterator *self );


/* State
 *  returns bitmap of state bits and event code at the current position
 *  will return invalid before initial Next message or after final
 *
 *  bits [ 0..7 ] :
 *    { 0..15 }             = 4na mismatch (NACMGRSVTWYHKDBN)
 *    64                    = match
 *    128                   = skip
 *  bit [ 8 ]               = have insert if ! 0
 *  bit [ 9 ]               = have delete if ! 0
 *  bit [ 10 ]              = first base if ! 0
 *  bit [ 11 ]              = last base if ! 0
 *  bit [ 31 ]              = iterator position is invalid if ! 0
 *                            NB - converts state word to negative
 */
enum
{
    align_iter_match      = ( 1 <<  8 ),
    align_iter_skip       = ( 1 <<  9 ),

    align_iter_insert     = ( 1 << 10 ),
    align_iter_delete     = ( 1 << 11 ),
    align_iter_first      = ( 1 << 12 ),
    align_iter_last       = ( 1 << 13 ),

    align_iter_invalid    = ( int ) ( 1U << 31 )
};

ALIGN_EXTERN int32_t CC AlignmentIteratorState ( const AlignmentIterator *self,
                                                 INSDC_coord_zero *seq_pos );


/* Position
 *  return current position of iterator relative to reference
 */
ALIGN_EXTERN rc_t CC AlignmentIteratorPosition ( const AlignmentIterator *self,
                                                 INSDC_coord_zero *pos );


/* BasesInserted
 *  return the number of inserted bases and a pointer to their values
 *
 *  "bases" [ OUT, NULL OKAY ] - optional output parameter to inserted bases
 *
 *  returns count of bases inserted at current position
 */
ALIGN_EXTERN uint32_t CC AlignmentIteratorBasesInserted
    ( const AlignmentIterator *self, const INSDC_4na_bin **bases );


/* BasesDeleted
 *  return the number of bases deleted at the current position
 *  also returns the location on the reference where the delete starts
 *
 *  "pos" [ OUT ] - return parameter for location on the reference
 *  where delete starts, and continues for the number of bases given by function return
 *
 *  returns count of bases deleted at current position
 */
ALIGN_EXTERN uint32_t CC AlignmentIteratorBasesDeleted
    ( const AlignmentIterator *self, INSDC_coord_zero *pos );


/*--------------------------------------------------------------------------
 * PlacementRecord
 *  record describing a placement
 */
typedef struct PlacementRecord PlacementRecord;
struct PlacementRecord
{
    DLNode n;

    /* row id of alignment record */
    int64_t id;

    /* object representing reference sequence */
    struct ReferenceObj const *ref;

    /* placement position and length on reference */
    INSDC_coord_zero pos;
    INSDC_coord_len len;

    /* mapping quality of alignment */
    int32_t mapq;

    /* spotgroup is now in here too */
    uint32_t spot_group_len;
    char * spot_group;
};


/* Cast
 *  cast to an extended object
 *
 *  "ext" [ IN ] - selects the extended object level
 *  can be placementRecordExtension0 or placementRecordExtension1
 */

enum { placementRecordExtension0, placementRecordExtension1 };

ALIGN_EXTERN void* CC PlacementRecordCast ( const PlacementRecord *self, uint32_t ext );


ALIGN_EXTERN void* CC PlacementRecord_get_ext_data_ptr ( const PlacementRecord *self, uint32_t ext );


/* Whack
 *  destroys PlacementRecord and any associated extensions
 */
ALIGN_EXTERN void CC PlacementRecordWhack ( const PlacementRecord *self );


/* structure of function pointers for creating extensions
   all function pointers are optional ( NULL OKAY ) */
typedef struct PlacementRecordExtendFuncs PlacementRecordExtendFuncs;
struct PlacementRecordExtendFuncs
{
    /* opaque pointer to data passed to each function */
    void *data;

    /* destructor */
    void ( CC * destroy ) ( void *obj, void *data );

    /* constructor */
    rc_t ( CC * populate ) ( void *obj, const PlacementRecord *placement,
        struct VCursor const *curs, INSDC_coord_zero ref_window_start,
        INSDC_coord_len ref_window_len, void *data, void * placement_ctx );

    /* variable allocation size calculation
       when non-NULL, takes precedence over "fixed_size" */
    bool ( CC * filter ) ( struct VCursor const *curs, int64_t row_id,
        const PlacementRecord *placement, INSDC_coord_zero ref_window_start,
        INSDC_coord_len ref_window_len, void *data, void * placement_ctx );

    /* variable allocation size calculation
       when non-NULL, takes precedence over "fixed_size" */
    rc_t ( CC * alloc_size ) ( struct VCursor const *curs, int64_t row_id, size_t * size, void *data, void * placement_ctx );
    
    /* fixed allocation size
       ignored if "alloc_size" is non-NULL,
       must be non-zero otherwise */
    size_t fixed_size;
};


/* external functions for extension of a placement record
   to include ( construct ) an AlignmentIterator */
ALIGN_EXTERN void CC AlignIteratorRecordDestroy ( void *obj, void *data );
ALIGN_EXTERN rc_t CC AlignIteratorRecordPopulate ( void *obj,
    const PlacementRecord *placement, struct VCursor const *curs,
    INSDC_coord_zero ref_window_start, INSDC_coord_len ref_window_len, void *data );
ALIGN_EXTERN rc_t CC AlignIteratorRecordSize ( struct VCursor const *curs, int64_t row_id, size_t * size, void *data );


/*--------------------------------------------------------------------------
 * PlacementIterator
 *  walk across placements from an alignment db within a reference window
 */
typedef struct PlacementIterator PlacementIterator;


/* Make
 *  create a placement iterator
 *
 *  "iter" [ OUT ] - return parameter for iterator
 *
 *  "ref_obj" [ IN, NULL OKAY ] - optional parameter giving an object
 *  representing the reference sequence for this iterator. it will be
 *  inserted into each PlacementRecord (see above) and made available to
 *  outer code.
 *
 *  "ref_pos" [ IN ] and "ref_len" [ IN ] - window onto reference
 *
 *  "min_mapq" [ IN ] - minimum map quality value
 *
 *  "ref_cur" [ IN ] - read-only cursor on REFERENCE table
 *  will be modified as necessary to contain requisite columns
 *  will be opened by iterator.
 *
 *  "align_cur" [ IN ] - read-only cursor on PRIMARY_ALIGNMENT or SECONDARY_ALIGNMENT
 *  table ( see "secondary" ). will be modified as necessary to contain
 *  requisite columns. will be opened by iterator.
 *
 *  "ids" [ IN ] - an enum describing which column of alignment ids should
 *  be used when reading "ref"
 *
 *  "ext_0" [ IN, NULL OKAY ] and "ext_1" [ IN, NULL OKAY ] - optional pointers
 *  to blocks describing how to extend the basic placement record
 *
 *  rd_group [ IN, NULL OKAY ]
 *      != NULL, non empty string ... produce all alignments with this string as
 *                  spot-group ( no matter what the "real" spot-group of the
 *                  alignment is )
 *
 *      != NULL, empty string ... produce all alignments with the "real" spot-group
 *                  read from the column "SPOT_GROUP"
 *
 *      == NULL, ... produce all alignments with no spot-group assigned ( the user
 *                  does not wish the data to be read, the alignment to be bined )
 */

typedef uint8_t align_id_src;
enum { primary_align_ids, secondary_align_ids, evidence_align_ids };

ALIGN_EXTERN rc_t CC AlignMgrMakePlacementIterator ( struct AlignMgr const *self,
    PlacementIterator **iter, struct ReferenceObj const *ref_obj,
    INSDC_coord_zero ref_pos, INSDC_coord_len ref_len, int32_t min_mapq,
    struct VCursor const *ref_cur, struct VCursor const *align_cur, align_id_src ids,
    const PlacementRecordExtendFuncs *ext_0, const PlacementRecordExtendFuncs *ext_1,
    const char * spot_group );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC PlacementIteratorAddRef ( const PlacementIterator *self );
ALIGN_EXTERN rc_t CC PlacementIteratorRelease ( const PlacementIterator *self );


/* RefWindow
 *  returns the reference identification string and iteration window
 */
ALIGN_EXTERN rc_t CC PlacementIteratorRefWindow ( const PlacementIterator *self,
    const char **idstr, INSDC_coord_zero *pos, INSDC_coord_len *len );


/* RefObj
 *  returns the Ref-obj, that was used to create this placement-iterator
 */
ALIGN_EXTERN rc_t CC PlacementIteratorRefObj( const PlacementIterator * self,
    struct ReferenceObj const ** refobj );


/* NextAvailPos
 *  check the next available position having one or more placements
 *
 *  "pos" [ OUT ] - next position on reference having one or more placements
 *  may return negative position, indicating an alignment that wraps around
 *  a circular reference, and starts in negative space after linearization.
 *
 *  "len" [ OUT, NULL OKAY ] - optional return parameter for length of
 *  placement at that position
 *
 *  returns non-zero rc when no window is done
 *  (rcRange, rcDone)
 */
ALIGN_EXTERN rc_t CC PlacementIteratorNextAvailPos ( const PlacementIterator *self,
    INSDC_coord_zero *pos, INSDC_coord_len *len );


/* NextRecordAt
 *  retrieve a placement at the requested position
 *
 *  "pos" [ IN ] - required position of the placement
 *  obtained from "NextAvailPos"
 *
 *  "rec" [ OUT ] - returned record
 *  must be freed via PlacementRecordWhack
 *
 *  returns non-zero rc when no more placements are available
 *  (rcOffset, rcDone)
 */
ALIGN_EXTERN rc_t CC PlacementIteratorNextRecordAt ( PlacementIterator *self,
    INSDC_coord_zero pos, const PlacementRecord **rec );


/* NextIdAt
 *  retrieve a row id at the requested position
 *
 *  "pos" [ IN ] - required position of the placement
 *  obtained from "NextAvailPos"
 *
 *  "id" [ OUT ] - returned row-id, within domain of align cursor
 *
 *  "len" [ OUT, NULL OKAY ] - optional return parameter for length of
 *  placement on reference
 *
 *  returns non-zero rc when no more placements are available
 *  (rcOffset, rcDone)
 */
ALIGN_EXTERN rc_t CC PlacementIteratorNextIdAt ( PlacementIterator *self,
    INSDC_coord_zero pos, int64_t *row_id, INSDC_coord_len *len );


/*--------------------------------------------------------------------------
 * PlacementSetIterator
 *  walk across placements from an alignment db within a reference window
 */
typedef struct PlacementSetIterator PlacementSetIterator;


/* Make
 *  create a placement set iterator
 *
 *  "iter" [ OUT ] - return parameter for iterator
 *
 *  "ref_pos" [ IN ] and "ref_len" [ IN ] - window onto reference
 */
ALIGN_EXTERN rc_t CC AlignMgrMakePlacementSetIterator ( struct AlignMgr const *self,
    PlacementSetIterator **iter );


/* AddPlacementIterator
 *  adds a placement iterator
 *  used to provide ordered placements within window
 */
ALIGN_EXTERN rc_t CC PlacementSetIteratorAddPlacementIterator
    ( PlacementSetIterator *self, PlacementIterator *pi );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC PlacementSetIteratorAddRef ( const PlacementSetIterator *self );
ALIGN_EXTERN rc_t CC PlacementSetIteratorRelease ( const PlacementSetIterator *self );


ALIGN_EXTERN rc_t CC PlacementSetIteratorNextReference ( PlacementSetIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len, struct ReferenceObj const ** refobj );

ALIGN_EXTERN rc_t CC PlacementSetIteratorNextWindow ( PlacementSetIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len );

/* NextAvailPos
 *  check the next available position having one or more placements
 *
 *  "pos" [ OUT ] - next position on reference having one or more placements
 *  may return negative position, indicating an alignment that wraps around
 *  a circular reference, and starts in negative space after linearization.
 *
 *  "len" [ OUT, NULL OKAY ] - optional return parameter for length of
 *  placement at that position
 *
 *  returns non-zero rc when no more placements are available
 *  TBD - define a proper value
 */
ALIGN_EXTERN rc_t CC PlacementSetIteratorNextAvailPos ( const PlacementSetIterator *self,
    INSDC_coord_zero *pos, INSDC_coord_len *len );


/* NextRecordAt
 *  retrieve a placement at the requested position
 *
 *  "pos" [ IN ] - required position of the placement
 *  obtained from "NextAvailPos"
 *
 *  "rec" [ OUT ] - returned record
 *  must be freed via PlacementRecordWhack
 */
ALIGN_EXTERN rc_t CC PlacementSetIteratorNextRecordAt ( PlacementSetIterator *self,
    INSDC_coord_zero pos, const PlacementRecord **rec );


/* NextIdAt
 *  retrieve a row id at the requested position
 *
 *  "pos" [ IN ] - required position of the placement
 *  obtained from "NextAvailPos"
 *
 *  "id" [ OUT ] - returned row-id, within domain of align cursor
 *
 *  "len" [ OUT, NULL OKAY ] - optional return parameter for length of
 *  placement on reference
 */
ALIGN_EXTERN rc_t CC PlacementSetIteratorNextIdAt ( PlacementSetIterator *self,
    INSDC_coord_zero pos, int64_t *row_id, INSDC_coord_len *len );


/*--------------------------------------------------------------------------
 * ReferenceIterator
 *  walk across placements from an alignment db within a reference window
 */
typedef struct ReferenceIterator ReferenceIterator;


/* Make
 *  create a reference iterator
 *
 *  "iter" [ OUT ] - return parameter for iterator
 *
 *  "ext_1" [ IN, NULL OKAY ] - optional pointer to a block describing how
 *  to extend the align-iterator record
 *
 *  "min_mapq" [ IN ] - minimum map quality value
 */
ALIGN_EXTERN rc_t CC AlignMgrMakeReferenceIterator ( struct AlignMgr const *self,
    ReferenceIterator **iter, const PlacementRecordExtendFuncs *ext_1, int32_t min_mapq );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorAddRef ( const ReferenceIterator *self );
ALIGN_EXTERN rc_t CC ReferenceIteratorRelease ( const ReferenceIterator *self );


/* AddPlacementIterator
 *  adds a placement iterator
 *  used to provide ordered placements within window
 */
#if 0
ALIGN_EXTERN rc_t CC ReferenceIteratorAddPlacementIterator
    ( ReferenceIterator *self, PlacementIterator *pi );
#endif

/* AddPlacements
 *  adds a source for placements (file/table)
 *  used to provide ordered placements within window
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorAddPlacements ( ReferenceIterator *self,
    struct ReferenceObj const *ref_obj, INSDC_coord_zero ref_pos, INSDC_coord_len ref_len,
    struct VCursor const *ref, struct VCursor const *align, align_id_src ids,
    const char * spot_group, void * placement_ctx );


/* NextReference
 *  advance to the next reference
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorNextReference ( ReferenceIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len, struct ReferenceObj const ** refobj );

/* NextWindow
 *  advance to the next window on the reference
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorNextWindow ( ReferenceIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len );

/* NextSpotGroup
 *  advance to the next spot_group on the reference
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorNextSpotGroup ( ReferenceIterator *self,
    const char ** name, size_t * len );


/* NextPos
 *  advance to the next position on current reference
 *  resets internal iterator on placements at that position
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorNextPos ( ReferenceIterator *self, bool skip_empty );


/* Position
 *  return current position on the reference
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorPosition ( const ReferenceIterator *self,
    INSDC_coord_zero *pos, uint32_t * depth, INSDC_4na_bin * base );


/* NextPlacement
 *  advance internal iterator to next placement in list
 *  returns a pointer to the next placement object at current position
 */
ALIGN_EXTERN rc_t CC ReferenceIteratorNextPlacement ( ReferenceIterator *self,
    const PlacementRecord **rec );


/* State
 *  return state of current placement at current position
 */
ALIGN_EXTERN int32_t CC ReferenceIteratorState ( const ReferenceIterator *self, INSDC_coord_zero *seq_pos );


/* BasesInserted
 *  return the number of inserted bases and a pointer to their values
 *
 *  "bases" [ OUT, NULL OKAY ] - optional output parameter to inserted bases
 *
 *  returns count of bases inserted at current position
 */
ALIGN_EXTERN uint32_t CC ReferenceIteratorBasesInserted ( const ReferenceIterator *self,
    const INSDC_4na_bin **bases );


/* BasesDeleted
 *  return the number of bases deleted at the current position
 *  also returns the location on the reference where the delete starts
 *
 *  "pos" [ OUT ] - return parameter for location on the reference
 *  where delete starts, and continues for the number of bases given by function return
 *
 *  "bases" [ OUT, NULL OKAY ] - optional output parameter to deleted bases
 *
 *  returns count of bases deleted at current position
 */
ALIGN_EXTERN uint32_t CC ReferenceIteratorBasesDeleted ( const ReferenceIterator *self,
    INSDC_coord_zero *pos, const INSDC_4na_bin **bases );


#ifdef __cplusplus
}
#endif

#endif /* _h_align_iterator_ */
