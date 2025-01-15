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

#include "column.hpp"

#include "manager.hpp"
#include "metadata.hpp"
#include "columnblob.hpp"

#include <kdb/column.h>

#include <klib/printf.h>

using namespace KDBText;
using namespace std;

/*--------------------------------------------------------------------------
 * KTextColumn (formerly KColumn)
 *  a read-only collection of blobs indexed by oid; file system-based
 */

static rc_t CC KTextColumnWhack ( KColumn *self );
static bool CC KTextColumnLocked ( const KColumn *self );
static rc_t CC KTextColumnVersion ( const KColumn *self, uint32_t *version );
static rc_t CC KTextColumnByteOrder ( const KColumn *self, bool *reversed );
static rc_t CC KTextColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count );
static rc_t CC KTextColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start );
static rc_t CC KTextColumnOpenManagerRead ( const KColumn *self, const KDBManager **mgr );
static rc_t CC KTextColumnOpenParentRead ( const KColumn *self, const KTable **tbl );
static rc_t CC KTextColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap );
static rc_t CC KTextColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id );

static KColumn_vt KTextColumn_vt =
{
    /* Public API */
    KTextColumnWhack,
    KColumnBaseAddRef,
    KColumnBaseRelease,
    KTextColumnLocked,
    KTextColumnVersion,
    KTextColumnByteOrder,
    KTextColumnIdRange,
    KTextColumnFindFirstRowId,
    KTextColumnOpenManagerRead,
    KTextColumnOpenParentRead,
    KTextColumnOpenMetadataRead,
    KTextColumnOpenBlobRead
};

#define CAST() assert( bself->vt == &KTextColumn_vt ); Column * self = (Column *)bself

Column::Column( const KJsonObject * p_json, const Manager * p_mgr, const Table * p_parent )
: m_mgr( p_mgr ), m_parent( p_parent ), m_json ( p_json )
{
    dad . vt = & KTextColumn_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Column", "ctor", "db" );
    Manager::addRef( m_mgr );
}

Column::~Column()
{
    Metadata::release( m_meta );
    Manager::release( m_mgr );

    KRefcountWhack ( & dad . refcount, "KDBText::Column" );
}

void
Column::addRef( const Column * col )
{
    if ( col != nullptr )
    {
        KColumnAddRef( (const KColumn*) col );
    }
}

void
Column::release( const Column * col )
{
    if ( col != nullptr )
    {
        KColumnRelease( (const KColumn*) col );
    }
}

//
// map of type sizes. Size 0 will only accept string values
// any other size wil only accept unsigned integer value, encoded in 'size' bits
// (will error out if the value does not fit)
//
// based on dt[] in vdb/schema-int.c:179
//
static const map< string, size_t > TypeSizes = {
    { "any", 0 },
    { "void", 0 },
    { "opaque", 0 },

    /* bundles of bits in machine native order */
    { "B1", 1 },
    { "B8", 8 },
    { "B16", 16 },
    { "B32", 32 },
    { "B64", 64 },

    /* the basic unsigned integer types */
    { "U1", 1 },
    { "U8", 8 },
    { "U16", 16 },
    { "U32", 32 },
    { "U64", 64 },

    /* the basic signed integer types */
    /*TODO: support negative */
    { "I8", 8 },
    { "I16", 16 },
    { "I32", 32 },
    { "I64", 64 },

    // /*TODO: floating point */
    // { "F32", 32 },
    // { "F64", 64 },

    /* bool is typed to reflect C/C++ */
    { "bool", 8 }, // for now, 0 or 1 only; TODO: support true/false

    { "utf8", 8 },
    { "utf16", 16 },
    { "utf32", 32 },

    { "ascii", 0 },
    { "text", 0 },
};


rc_t
Column::inflate( char * p_error, size_t p_error_size )
{
    rc_t rc = 0;

    const KJsonValue * name = KJsonObjectGetMember ( m_json, "name" );
    if ( name != nullptr )
    {
        const char * nameStr = nullptr;
        rc = KJsonGetString ( name, & nameStr );
        if ( rc == 0 )
        {
            m_name = nameStr;
        }
        //TODO: error
    }
    else
    {
        string_printf ( p_error, p_error_size, nullptr, "Column name is missing" );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    const KJsonValue * type = KJsonObjectGetMember ( m_json, "type" );
    size_t typeSize = 0;
    if ( type != nullptr )
    {
        const char * typeStr = nullptr;
        rc = KJsonGetString ( type, & typeStr );
        if ( rc == 0 )
        {   // look up intrinsic types, extract value size
            auto i = TypeSizes.find( typeStr );
            if ( i == TypeSizes.end() )
            {
                string_printf ( p_error, p_error_size, nullptr, "Unknown column type" );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
            else
            {
                typeSize = i -> second;
                m_type = typeStr;
            }
        }
        //TODO: error
    }
    else
    {
        string_printf ( p_error, p_error_size, nullptr, "Column type is missing" );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    // data
    const KJsonValue * data = KJsonObjectGetMember ( m_json, "data" );
    if ( data != nullptr )
    {
        const KJsonArray * dataarr = KJsonValueToArray ( data );
        if ( dataarr == nullptr )
        {
            string_printf ( p_error, p_error_size, nullptr, "%s.data is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
        }

        uint32_t len = KJsonArrayGetLength ( dataarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( dataarr, i );
            assert( v != nullptr );

            shared_ptr<ColumnBlob> b ( new ColumnBlob( v ) );
            rc = b->inflate( p_error, p_error_size, typeSize );
            if ( rc == 0 )
            {
                m_data [ b->getIdRange() ] = b;
            }
        }
    }

    // metadata
    const KJsonValue * meta = KJsonObjectGetMember ( m_json, "metadata" );
    if ( meta != nullptr )
    {
        const KJsonObject * obj = KJsonValueToObject ( meta );
        if( obj != nullptr )
        {
            Metadata * m  = new Metadata( obj );
            rc = m -> inflate( p_error, p_error_size );
            if ( rc != 0 )
            {
                delete m;
                return rc;
            }
            m_meta = m;
        }
        else
        {   // not an object
            string_printf ( p_error, p_error_size, nullptr, "%s.metadata is not an object", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
    }

    return rc;
}

pair< int64_t, uint64_t >
Column::idRange() const
{   // { first, last - first + 1 }

    if ( m_data.empty() )
    {
        return make_pair( 0, 0 );
    }
    int64_t startRow = m_data.begin()->first.first;
    int64_t endRow = m_data.rbegin()->first.first + m_data.rbegin()->first.second;
    return make_pair( startRow, (uint64_t)endRow - (uint64_t)startRow );
}

Column::BlobMap::const_iterator
Column::findBlob( int64_t row ) const
{   // linear search, until proven too slow
    for( Column::BlobMap::const_iterator d = m_data.begin(); d != m_data.end(); ++d )
    {
        if ( row >= d->first.first && row < d->first.first + (int64_t)d->first.second )
        {
            return d;
        }
    }

    return m_data.end();
}

int64_t
Column::findFirst( int64_t rowId ) const // the first valid row-id starting from the given id.
{   // linear search, until proven too slow
    for( Column::BlobMap::const_iterator d = m_data.begin(); d != m_data.end(); ++d )
    {
        if ( rowId >= d->first.first && rowId < d->first.first + (int64_t)d->first.second )
        {
            return rowId;
        }
        if ( rowId < d->first.first )
        {
            return d->first.first;
        }
    }
    return 0;
}

const ColumnBlob *
Column::openBlob( int64_t id ) const
{
    auto it = findBlob( id );
    if ( it != m_data.end() )
    {
        it->second.get()->addRef();
        return it->second.get();
    }
    return nullptr;
}

// API

static
rc_t CC
KTextColumnWhack ( KColumn *bself )
{
    CAST();

    delete reinterpret_cast<Column*>( self );
    return 0;
}

static
bool CC
KTextColumnLocked ( const KColumn *self )
{
    return false;
}

static
rc_t CC
KTextColumnVersion ( const KColumn *self, uint32_t *version )
{
    *version = 0;
    return 0;
}

static
rc_t CC
KTextColumnByteOrder ( const KColumn *self, bool *reversed )
{
    *reversed = false;
    return 0;
}

static
rc_t CC
KTextColumnIdRange ( const KColumn * bself, int64_t *first, uint64_t *count )
{
    CAST();

    auto p = self->idRange();

    *first = p.first;
    *count = p.second;
    return 0;
}

static
rc_t CC
KTextColumnFindFirstRowId ( const KColumn * bself, int64_t * found, int64_t start )
{
    CAST();

    *found = self -> findFirst( start );
    if ( *found == 0 )
    {
        return SILENT_RC ( rcDB, rcColumn, rcSelecting, rcRow, rcNotFound );
    }
    return 0;
}

static
rc_t CC
KTextColumnOpenManagerRead ( const KColumn *bself, const KDBManager **mgr )
{
    CAST();

    const Manager * m = self -> getManager();
    if ( m != nullptr )
    {
        Manager::addRef( m );
    }

    *mgr = (const KDBManager*)m;

    return 0;
}

static
rc_t CC
KTextColumnOpenParentRead ( const KColumn *bself, const KTable **tbl )
{
    CAST();

    const Table * p = self -> getParent();
    if ( p != nullptr )
    {
        Table::addRef( p );
    }

    *tbl = (const KTable*) p;

    return 0;
}

static
rc_t CC
KTextColumnOpenMetadataRead ( const KColumn *bself, const KMetadata **metap )
{
    CAST();
    const Metadata * m = self->openMetadata();
    if ( m != nullptr )
    {
        Metadata::addRef( m );
        *metap = (const KMetadata*)m;
        return 0;
    }
    return SILENT_RC( rcDB, rcColumn, rcOpening, rcMetadata, rcNotFound );
}

static
rc_t CC
KTextColumnOpenBlobRead ( const KColumn *bself, const KColumnBlob **blobp, int64_t id )
{
    CAST();
    const ColumnBlob * b = self->openBlob( id );
    if ( b != nullptr )
    {
        *blobp = (const KColumnBlob*)b;
        return 0;
    }
    return SILENT_RC( rcDB, rcColumn, rcOpening, rcBlob, rcNotFound );
}
