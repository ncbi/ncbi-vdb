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
// static rc_t CC KTextColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start );
// static rc_t CC KTextColumnOpenManagerRead ( const KColumn *self, const KDBManager **mgr );
// static rc_t CC KTextColumnOpenParentRead ( const KColumn *self, const KTable **tbl );
// static rc_t CC KTextColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap );
// static rc_t CC KTextColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id );

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
    // KTextColumnFindFirstRowId,
    // KTextColumnOpenManagerRead,
    // KTextColumnOpenParentRead,
    // KTextColumnOpenMetadataRead,
    // KTextColumnOpenBlobRead
};

#define CAST() assert( bself->vt == &KTextColumn_vt ); Column * self = (Column *)bself

Column::Column( const KJsonObject * p_json, const Table * p_parent )
: m_json ( p_json ), m_parent( p_parent )
{
    dad . vt = & KTextColumn_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Column", "ctor", "db" );
}

Column::~Column()
{
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
    }
    else
    {
        string_printf ( p_error, p_error_size, nullptr, "Column name is missing" );
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
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                const KJsonValue * id = KJsonObjectGetMember ( obj, "row" );
                if ( id == nullptr )
                {
                    string_printf ( p_error, p_error_size, nullptr, "%s.data[%i].row is missing", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }
                const KJsonValue * value = KJsonObjectGetMember ( obj, "value" );
                if ( value == nullptr )
                {
                    string_printf ( p_error, p_error_size, nullptr, "%s.data[%i].value is missing", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }

                if ( m_data.find( id ) != m_data . end() )
                {
                    string_printf ( p_error, p_error_size, nullptr, "Duplicate row id: %s", id.c_str() );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }
                // m_data [ id ] = value;
            }
            else
            {   // not an object
                string_printf ( p_error, p_error_size, nullptr, "%s.data[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }
        }

    }

    //TBD

    return rc;
}

pair< int64_t, uint64_t >
Column::idRange() const
{   // { first, last - first + 1 }

    if ( m_data.empty() )
    {
        return make_pair( 0, 0 );
    }
    return make_pair( m_data.begin()->first, m_data.rbegin()->first - m_data.begin()->first + 1 );
}

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
KTextColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count )
{
    *first = 0;
    *count = 0;
    return 0;
}

