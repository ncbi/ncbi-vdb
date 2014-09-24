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

#include <kfs/extern.h>
#include <kfs/impl.h>
#include <klib/rc.h>
#include <sysalloc.h>


/*--------------------------------------------------------------------------
 * KArrayFile
 */


/* Destroy
 *  destroy array-file
 */
LIB_EXPORT rc_t CC KArrayFileDestroy ( KArrayFile *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcDestroying, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . destroy ) ( self );
    }

    return RC ( rcFS, rcFile, rcDestroying, rcInterface, rcBadVersion );
}


/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KArrayFileAddRef ( const KArrayFile *self )
{
    if ( self != NULL )
        atomic32_inc ( & ( ( KArrayFile* ) self ) -> refcount );
    return 0;
}

/* Release
 *  discard reference to file
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KArrayFileRelease ( const KArrayFile *cself )
{
    KArrayFile *self = ( KArrayFile* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
        {
            rc_t rc = KArrayFileDestroy ( self );
            if ( rc != 0 )
                atomic32_set ( & self -> refcount, 1 );
            return rc;
        }
    }
    return 0;
}

/* Dimensionality
 *  returns the number of dimensions in the ArrayFile
 *
 *  "dim" [ OUT ] - return parameter for number of dimensions
 */
LIB_EXPORT rc_t CC KArrayFileDimensionality ( const KArrayFile *self, uint8_t *dim )
{
    if ( dim == NULL )
        return RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );

    * dim = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . dimensionality ) ( self, dim );
    }

    return RC ( rcFS, rcFile, rcAccessing, rcInterface, rcBadVersion );
}


/* SetDimensionality
 *  sets the number of dimensions in the ArrayFile
 *
 *  "dim" [ IN ] - new number of dimensions; must be > 0
 */
LIB_EXPORT rc_t CC KArrayFileSetDimensionality ( KArrayFile *self, uint8_t dim )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcResizing, rcSelf, rcNull );

    if ( dim == 0 )
        return RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcFS, rcFile, rcResizing, rcFile, rcNoPerm );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . set_dimensionality ) ( self, dim );
    }

    return RC ( rcFS, rcFile, rcResizing, rcInterface, rcBadVersion );
}


/* DimExtents
 *  returns the extent of every dimension
 *
 *  "dim" [ IN ] - the dimensionality of "extents"
 *
 *  "extents" [ OUT ] - returns the extent for every dimension
 */
LIB_EXPORT rc_t CC KArrayFileDimExtents ( const KArrayFile *self, uint8_t dim, uint64_t *extents )
{
    uint8_t i;
    if ( extents == NULL || dim == 0 )
        return RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );

    for ( i = 0; i < dim; ++i )
        extents[ i ] = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . dim_extents ) ( self, dim, extents );
    }

    return RC ( rcFS, rcFile, rcAccessing, rcInterface, rcBadVersion );
}


/* SetDimExtents
 *  sets the new extents for every dimension
 *
 *  "dim" [ IN ] - the dimensionality of "extents"
 *
 *  "extents" [ IN ] - new extents for every dimension
 */
LIB_EXPORT rc_t CC KArrayFileSetDimExtents ( KArrayFile *self, uint8_t dim, uint64_t *extents )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcResizing, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcFS, rcFile, rcResizing, rcFile, rcNoPerm );

    if ( extents == NULL || dim == 0 )
        return RC ( rcFS, rcFile, rcResizing, rcParam, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . set_dim_extents ) ( self, dim, extents );
    }

    return RC ( rcFS, rcFile, rcResizing, rcInterface, rcBadVersion );
}


/* ElementSize
 *  returns the element size in bits
 *
 *  "elem_bits" [ OUT ] - size of each element in bits
 */
LIB_EXPORT rc_t CC KArrayFileElementSize ( const KArrayFile *self, uint64_t *elem_bits )
{
    if ( elem_bits == NULL )
        return RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );

    *elem_bits = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . element_size ) ( self, elem_bits );
    }

    return RC ( rcFS, rcFile, rcAccessing, rcInterface, rcBadVersion );
}


/* Read
 *  read from n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional starting position in elements
 *
 *  "buffer" [ OUT ] and "elem_count" [ IN ] - return buffer for read
 *  where "elem_count" is n-dimensional in elements
 *
 *  "num_read" [ OUT ] - n-dimensional return parameter giving back
 *      the number of read elements in every dimension
 */
LIB_EXPORT rc_t CC KArrayFileRead ( const KArrayFile *self, uint8_t dim,
    const uint64_t *pos, void *buffer, const uint64_t *elem_count,
    uint64_t *num_read )
{
    uint8_t i;
    if ( num_read == NULL || pos == NULL || elem_count == NULL || dim == 0 )
        return RC ( rcFS, rcFile, rcReading, rcParam, rcNull );

    for ( i = 0; i < dim; ++i )
        num_read[ i ] = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcFS, rcFile, rcReading, rcFile, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcReading, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . read ) ( self, dim, pos, buffer, elem_count, num_read );
    }

    return RC ( rcFS, rcFile, rcReading, rcInterface, rcBadVersion );
}


/* Read_v
 *  read one element of variable length from n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional starting position in elements
 *
 *  "buffer" [ OUT ] and "elem_count" [ IN ] - return buffer for read
 *  where "elem_count" is length of buffer in elements
 *
 *  "num_read" [ OUT ] - return parameter giving back
 *      the number of read elements in every dimension
 */
LIB_EXPORT rc_t CC KArrayFileRead_v ( const KArrayFile *self, uint8_t dim,
    const uint64_t *pos, void *buffer, const uint64_t elem_count,
    uint64_t *num_read )
{
    if ( num_read == NULL || pos == NULL || elem_count == 0 || dim == 0 )
        return RC ( rcFS, rcFile, rcReading, rcParam, rcNull );

    *num_read = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcReading, rcSelf, rcNull );

    if ( ! self -> read_enabled )
        return RC ( rcFS, rcFile, rcReading, rcFile, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcReading, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . read_v ) ( self, dim, pos, buffer, elem_count, num_read );
    }

    return RC ( rcFS, rcFile, rcReading, rcInterface, rcBadVersion );
}


/* Write
 *  write into n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional offset where to write to
 *                   in elements
 *
 *  "buffer" [ IN ] and "elem_count" [ IN ] - data to be written
 *  where "elem_count" is n-dimensional in elements
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of elements actually written per dimension
 */
LIB_EXPORT rc_t CC KArrayFileWrite ( KArrayFile *self, uint8_t dim,
    const uint64_t *pos, const void *buffer, const uint64_t *elem_count,
    uint64_t *num_writ )
{
    uint8_t i;
    if ( num_writ == NULL || pos == NULL || elem_count == NULL || dim == 0 )
        return RC ( rcFS, rcFile, rcWriting, rcParam, rcNull );

    for ( i = 0; i < dim; ++i )
        num_writ[ i ] = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcWriting, rcSelf, rcNull );

    if ( ! self -> write_enabled )
        return RC ( rcFS, rcFile, rcWriting, rcFile, rcNoPerm );

    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcWriting, rcBuffer, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . write ) ( self, dim, pos, buffer, elem_count, num_writ );
    }

    return RC ( rcFS, rcFile, rcWriting, rcInterface, rcBadVersion );
}


/* GetMeta
 *  extracts metadata into a string-vector
 *
 *  "key"   [ IN ]  - the key which part of the metadata to retrieve
 *
 *  "list"  [ OUT ] - pointer to a KNamelist-pointer
 *
 */
LIB_EXPORT rc_t CC KArrayFileGetMeta ( struct KArrayFile const *self, const char *key,
    const struct KNamelist **list )
{
    if ( key == NULL || list == NULL )
        return RC ( rcFS, rcFile, rcReading, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcReading, rcSelf, rcNull );

    *list = NULL;
    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . get_meta ) ( self, key, list );
    }

    return RC ( rcFS, rcFile, rcWriting, rcInterface, rcBadVersion );
}


/* Init
 *  initialize a newly allocated file object
 */
LIB_EXPORT rc_t CC KArrayFileInit ( KArrayFile *self, const KArrayFile_vt *vt,
    bool read_enabled, bool write_enabled )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    if ( vt == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
    switch ( vt -> v1 . maj )
    {
    case 0:
        return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcInvalid );

    case 1:
        switch ( vt -> v1 . min )
        {
            /* ADD NEW MINOR VERSION CASES HERE */
        case 0:
#if _DEBUGGING
        if ( vt -> v1 . write == NULL              ||
             vt -> v1 . read == NULL               ||
             vt -> v1 . element_size == NULL       ||
             vt -> v1 . set_dim_extents == NULL    ||
             vt -> v1 . dim_extents == NULL        ||
             vt -> v1 . set_dimensionality == NULL ||
             vt -> v1 . dimensionality == NULL     ||
             vt -> v1 . get_meta == NULL           ||
             vt -> v1 . destroy == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcBadVersion );
        }
        break;

    default:
        return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcBadVersion );
    }

    self -> vt = vt;
    KRefcountInit ( & self -> refcount, 1, "KArrayFile", "Creating", "-" );
    self -> read_enabled = ( uint8_t ) ( read_enabled != 0 );
    self -> write_enabled = ( uint8_t ) ( write_enabled != 0 );

    return 0;
}
