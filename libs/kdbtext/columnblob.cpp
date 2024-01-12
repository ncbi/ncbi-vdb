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

#include "columnblob.hpp"

#include <klib/printf.h>

using namespace KDBText;
using namespace std;

/*--------------------------------------------------------------------------
 * KTextColumnBlob
 *  one or more rows of column data
 */

static rc_t KTextColumnBlobWhack ( KColumnBlob *self );
// static rc_t CC KTextColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
// static rc_t CC KTextColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
// static rc_t CC KTextColumnBlobValidate ( const KColumnBlob *self );
// static rc_t CC KTextColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
// static rc_t CC KTextColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KTextColumnBlob_vt =
{
    /* Public API */
    KTextColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    // KTextColumnBlobRead,
    // KTextColumnBlobReadAll,
    // KTextColumnBlobValidate,
    // KTextColumnBlobValidateBuffer,
    // KTextColumnBlobIdRange
};

#define CAST() assert( bself->vt == &KTextColumnBlob_vt ); ColumnBlob * self = (ColumnBlob *)bself

ColumnBlob::ColumnBlob( const KJsonObject * p_json )
: m_json ( p_json )
{
    dad . vt = & KTextColumnBlob_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );
}

ColumnBlob::~ColumnBlob()
{
    KRefcountWhack ( & dad . refcount, "KDBText::ColumnBlob" );
}

void
ColumnBlob::addRef( const ColumnBlob * col )
{
    if ( col != nullptr )
    {
        KColumnBlobAddRef( (const KColumnBlob*) col );
    }
}

void
ColumnBlob::release( const ColumnBlob * col )
{
    if ( col != nullptr )
    {
        KColumnBlobRelease( (const KColumnBlob*) col );
    }
}

rc_t
ColumnBlob::inflate( char * p_error, size_t p_error_size )
{
    rc_t rc = 0;

    // const KJsonValue * name = KJsonObjectGetMember ( m_json, "name" );
    // if ( name != nullptr )
    // {
    //     const char * nameStr = nullptr;
    //     rc = KJsonGetString ( name, & nameStr );
    //     if ( rc == 0 )
    //     {
    //         m_name = nameStr;
    //     }
    // }
    // else
    // {
    //     string_printf ( p_error, p_error_size, nullptr, "ColumnBlob name is missing" );
    //     return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    // }

    // const KJsonValue * type = KJsonObjectGetMember ( m_json, "type" );
    // if ( type != nullptr )
    // {
    //     const char * typeStr = nullptr;
    //     rc = KJsonGetString ( type, & typeStr );
    //     if ( rc == 0 )
    //     {
    //         m_type = typeStr;
    //     }
    // }
    // else
    // {
    //     string_printf ( p_error, p_error_size, nullptr, "ColumnBlob type is missing" );
    //     return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    // }

    // // data
    // const KJsonValue * data = KJsonObjectGetMember ( m_json, "data" );
    // if ( data != nullptr )
    // {
    //     const KJsonArray * dataarr = KJsonValueToArray ( data );
    //     if ( dataarr == nullptr )
    //     {
    //         string_printf ( p_error, p_error_size, nullptr, "%s.data is not an array", m_name.c_str() );
    //         return SILENT_RC( rcDB, rcBlob, rcCreating, rcParam, rcInvalid );
    //     }

    //     uint32_t len = KJsonArrayGetLength ( dataarr );
    //     for ( uint32_t i = 0; i < len; ++i )
    //     {
    //         const KJsonValue * v = KJsonArrayGetElement ( dataarr, i );
    //         assert( v != nullptr );
    //         const KJsonObject * obj = KJsonValueToObject ( v );
    //         if( obj != nullptr )
    //         {
    //             const KJsonValue * id = KJsonObjectGetMember ( obj, "row" );
    //             if ( id == nullptr )
    //             {
    //                 string_printf ( p_error, p_error_size, nullptr, "%s.data[%i].row is missing", m_name.c_str(), i );
    //                 return SILENT_RC( rcDB, rcBlob, rcCreating, rcParam, rcInvalid );
    //             }
    //             const KJsonValue * value = KJsonObjectGetMember ( obj, "value" );
    //             if ( value == nullptr )
    //             {
    //                 string_printf ( p_error, p_error_size, nullptr, "%s.data[%i].value is missing", m_name.c_str(), i );
    //                 return SILENT_RC( rcDB, rcBlob, rcCreating, rcParam, rcInvalid );
    //             }

    //             int64_t rowId;
    //             rc = KJsonGetNumber ( id, &rowId );
    //             if ( rc == 0 )
    //             {
    //                 if ( m_data.find( rowId ) != m_data . end() )
    //                 {
    //                     string_printf ( p_error, p_error_size, nullptr, "Duplicate row id: %s", rowId );
    //                     return SILENT_RC( rcDB, rcBlob, rcCreating, rcParam, rcInvalid );
    //                 }

    //                 const char * valueStr = nullptr;
    //                 rc = KJsonGetString ( value, & valueStr );
    //                 if ( rc == 0 )
    //                 {
    //                     KDataBuffer b;
    //                     KDataBufferMakeBytes( & b, strlen( valueStr ) + 1 );
    //                     strcpy( (char*)b.base, valueStr );
    //                     m_data [ rowId ] = b;
    //                 }
    //             }
    //             else
    //             {   // not an object
    //                 string_printf ( p_error, p_error_size, nullptr, "%s.data[%i].row is not an integer", m_name.c_str(), i );
    //                 return rc;
    //             }
    //         }
    //         else
    //         {   // not an object
    //             string_printf ( p_error, p_error_size, nullptr, "%s.data[%i] is not an object", m_name.c_str(), i );
    //             return SILENT_RC( rcDB, rcBlob, rcCreating, rcParam, rcInvalid );
    //         }
    //     }
    // }

    // // metadata
    // const KJsonValue * meta = KJsonObjectGetMember ( m_json, "metadata" );
    // if ( meta != nullptr )
    // {
    //     const KJsonObject * obj = KJsonValueToObject ( meta );
    //     if( obj != nullptr )
    //     {
    //         Metadata * m  = new Metadata( obj );
    //         rc = m -> inflate( p_error, p_error_size );
    //         if ( rc != 0 )
    //         {
    //             delete m;
    //             return rc;
    //         }
    //         m_meta = m;
    //     }
    //     else
    //     {   // not an object
    //         string_printf ( p_error, p_error_size, nullptr, "%s.metadata is not an object", m_name.c_str() );
    //         return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    //     }
    // }

    return rc;
}

// API

static
rc_t
KTextColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    return KColumnBlobBaseWhack ( bself );
}
