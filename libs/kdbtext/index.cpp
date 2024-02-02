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

#include "index.hpp"

#include <kdb/manager.h>

#include <klib/printf.h>
#include <klib/text.h>

using namespace KDBText;
using namespace std;

static rc_t KTextIndexWhack ( KIndex *self );
static bool CC KTextIndexLocked ( const KIndex *self );
static rc_t CC KTextIndexVersion ( const KIndex *self, uint32_t *version );
static rc_t CC KTextIndexType ( const KIndex *self, KIdxType *type );
static rc_t CC KTextIndexConsistencyCheck ( const KIndex *self, uint32_t level,
     int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
     uint64_t *num_rows, uint64_t *num_holes );
static rc_t CC KTextIndexFindText ( const KIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data );
static rc_t CC KTextIndexFindAllText ( const KIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data );
static rc_t CC KTextIndexProjectText ( const KIndex *self,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize );
static rc_t CC KTextIndexProjectAllText ( const KIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data );
static rc_t CC KTextIndexFindU64( const KIndex* self, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty );
static rc_t CC KTextIndexFindAllU64( const KIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data);
static void CC KTextIndexSetMaxRowId ( const KIndex *cself, int64_t max_row_id );

static KIndex_vt KTextIndex_vt =
{
    KTextIndexWhack,
    KIndexBaseAddRef,
    KIndexBaseRelease,
    KTextIndexLocked,
    KTextIndexVersion,
    KTextIndexType,
    KTextIndexConsistencyCheck,
    KTextIndexFindText,
    KTextIndexFindAllText,
    KTextIndexProjectText,
    KTextIndexProjectAllText,
    KTextIndexFindU64,
    KTextIndexFindAllU64,
    KTextIndexSetMaxRowId
};

#define CAST() assert( bself->vt == &KTextIndex_vt ); Index * self = (Index *)bself

Index::Index( const KJsonObject * p_json, const Table * p_parent )
: m_json ( p_json ), m_parent( p_parent )
{
    dad . vt = & KTextIndex_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Index", "ctor", "db" );
}

Index::~Index()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Index" );
}

void
Index::addRef( const Index * idx )
{
    if ( idx != nullptr )
    {
        KIndexAddRef( (const KIndex*) idx );
    }
}

void
Index::release( const Index * idx )
{
    if ( idx != nullptr )
    {
        KIndexRelease( (const KIndex*) idx );
    }
}

rc_t
Index::inflate( char * p_error, size_t p_error_size )
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
        else
        {
            string_printf ( p_error, p_error_size, nullptr, "Index name is invalid" );
            return SILENT_RC( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
        }
    }
    else
    {
        string_printf ( p_error, p_error_size, nullptr, "Index name is missing" );
        return SILENT_RC( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
    }

    const KJsonValue * text = KJsonObjectGetMember ( m_json, "text" );
    if ( text != nullptr )
    {
        const KJsonArray * textarr = KJsonValueToArray ( text );
        if ( textarr == nullptr )
        {
            string_printf ( p_error, p_error_size, nullptr, "%s.text is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
        }

        uint32_t len = KJsonArrayGetLength ( textarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( textarr, i );
            assert( v != nullptr );
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                // key
                const KJsonValue * key = KJsonObjectGetMember ( obj, "key" );
                if ( key == nullptr )
                {
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].key is missing", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }

                const char * keyStr = nullptr;
                rc = KJsonGetString ( key, & keyStr );
                if ( rc != 0 )
                {   // invalid value (nullptr?)
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].key is invalid", m_name.c_str(), i );
                    return rc;
                }

                // start
                const KJsonValue * start = KJsonObjectGetMember ( obj, "startId" );
                if ( start == nullptr )
                {
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].startId is missing", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }

                int64_t startId;
                rc = KJsonGetNumber ( start, &startId );
                if ( rc != 0 )
                {   // invalid value
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].startId is invalid", m_name.c_str(), i );
                    return rc;
                }

                // count
                const KJsonValue * count = KJsonObjectGetMember ( obj, "count" );
                if ( count == nullptr )
                {
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].count is missing", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }

                int64_t countValue;
                rc = KJsonGetNumber ( count, &countValue );
                if ( rc != 0 )
                {   // invalid value
                    string_printf ( p_error, p_error_size, nullptr, "%s.text[%i].count is invalid", m_name.c_str(), i );
                    return rc;
                }

                if ( m_textData.find( keyStr ) != m_textData . end() )
                {
                    string_printf ( p_error, p_error_size, nullptr, "Duplicate key: %s", keyStr );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }

                m_textData[ keyStr ] = make_pair( startId, (uint32_t)countValue );

            }
            else
            {   // not an object
                string_printf ( p_error, p_error_size, nullptr, "%s.text[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }
        }
    }
    else
    {   // int
        const KJsonValue * int_ = KJsonObjectGetMember ( m_json, "int" );
        if ( int_ != nullptr )
        {
            const KJsonArray * intarr = KJsonValueToArray ( int_ );
            if ( intarr == nullptr )
            {
                string_printf ( p_error, p_error_size, nullptr, "%s.int is not an array", m_name.c_str() );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }

            uint32_t len = KJsonArrayGetLength ( intarr );
            for ( uint32_t i = 0; i < len; ++i )
            {
                const KJsonValue * v = KJsonArrayGetElement ( intarr, i );
                assert( v != nullptr );
                const KJsonObject * obj = KJsonValueToObject ( v );
                if( obj != nullptr )
                {
                    // key
                    const KJsonValue * key = KJsonObjectGetMember ( obj, "key" );
                    if ( key == nullptr )
                    {
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].key is missing", m_name.c_str(), i );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }

                    int64_t keyVal = 0;
                    rc = KJsonGetNumber ( key, & keyVal );
                    if ( rc != 0 )
                    {   // invalid value (not a number)
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].key is invalid", m_name.c_str(), i );
                        return rc;
                    }

                    // size
                    const KJsonValue * size = KJsonObjectGetMember ( obj, "size" );
                    if ( size == nullptr )
                    {
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].size is missing", m_name.c_str(), i );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }

                    int64_t sizeVal = 0;
                    rc = KJsonGetNumber ( size, & sizeVal );
                    if ( rc != 0 )
                    {   // invalid value (not a number)
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].size is invalid", m_name.c_str(), i );
                        return rc;
                    }

                    // start
                    const KJsonValue * start = KJsonObjectGetMember ( obj, "startId" );
                    if ( start == nullptr )
                    {
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].startId is missing", m_name.c_str(), i );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }

                    int64_t startVal;
                    rc = KJsonGetNumber ( start, &startVal );
                    if ( rc != 0 )
                    {   // invalid value
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].startId is invalid", m_name.c_str(), i );
                        return rc;
                    }

                    // count
                    const KJsonValue * count = KJsonObjectGetMember ( obj, "count" );
                    if ( count == nullptr )
                    {
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].count is missing", m_name.c_str(), i );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }

                    int64_t countVal;
                    rc = KJsonGetNumber ( count, &countVal );
                    if ( rc != 0 )
                    {   // invalid value
                        string_printf ( p_error, p_error_size, nullptr, "%s.int[%i].count is invalid", m_name.c_str(), i );
                        return rc;
                    }

                    IntDataNode n;
                    n.key = keyVal;
                    n.key_size = sizeVal;
                    n.id_start = startVal;
                    n.id_count = countVal;
                    if ( m_intData.find( n ) != m_intData . end() )
                    {
                        string_printf ( p_error, p_error_size, nullptr, "Duplicate key: %lu,%lu", keyVal, sizeVal );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }

                    m_intData.insert( n );

                }
                else
                {   // not an object
                    string_printf ( p_error, p_error_size, nullptr, "%s.int[%i] is not an object", m_name.c_str(), i );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }
            }
        }
    }

    return rc;
}

static
rc_t CC
KTextIndexWhack ( KIndex *bself )
{
    CAST();

    delete reinterpret_cast<Index*>( self );
    return 0;
}

static
bool CC
KTextIndexLocked ( const KIndex *self )
{
    return false;
}

static
rc_t CC
KTextIndexVersion ( const KIndex *self, uint32_t *version )
{
    if ( version == nullptr )
    {
        return SILENT_RC ( rcVDB, rcIndex, rcReading, rcParam, rcNull );
    }

    *version = 0;

    return 0;
}

static
rc_t CC
KTextIndexType ( const KIndex *self, KIdxType *type )
{
    if ( type == nullptr )
    {
        return SILENT_RC ( rcVDB, rcIndex, rcReading, rcParam, rcNull );
    }

    *type = kitText;

    return 0;
}

static
rc_t CC
KTextIndexConsistencyCheck ( const KIndex *self, uint32_t level,
     int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
     uint64_t *num_rows, uint64_t *num_holes )
{   // no op
    return 0;
}

static
rc_t CC
KTextIndexFindText ( const KIndex *bself,
    const char *key,
    int64_t *start_id,
    uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data )
{
    CAST();

    if ( start_id == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcParam, rcNull );
    }
    if ( key == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNull );
    }
    if ( key [ 0 ] == 0 )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcEmpty );
    }

    if ( custom_cmp != nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcFunction, rcUnsupported );
    }

    auto it = self -> getTextData() . find( key );
    if ( it == self->getTextData().end() )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    }

    *start_id = it->second.first;
    *id_count = it->second.second;

    return 0;
}

static
rc_t CC
KTextIndexFindAllText ( const KIndex * bself, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data )
{
    CAST();

    if ( key == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNull );
    }
    if ( key [ 0 ] == 0 )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcEmpty );
    }
    if ( f == nullptr )
    {
        return RC ( rcDB, rcIndex, rcSelecting, rcFunction, rcNull );
    }

    auto it = self -> getTextData() . find( key );
    if ( it == self->getTextData().end() )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    }

    return f( it->second.first, it->second.second, data );
}

static
rc_t CC
KTextIndexProjectText ( const KIndex * bself,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize )
{
    CAST();

    if ( key == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcNull );
    }

    // linear search
    for ( auto i: self->getTextData() )
    {
        if ( id >= i.second.first && id < i.second.first + (int64_t)i.second.second )
        {
            if ( start_id != nullptr )
            {
                *start_id = i.second.first;
            }
            if ( id_count != nullptr )
            {
                *id_count = i.second.second;
            }

            if ( actsize != nullptr )
            {
                *actsize = i.first.size();
            }
            if ( kmax < i.first.size() )
            {
                return SILENT_RC( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
            }
            string_copy( key, kmax, i.first.c_str(), i.first.size() );
            return 0;
        }
    }

    return SILENT_RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}

static rc_t CC KTextIndexProjectAllText ( const KIndex * bself, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data )
{
    CAST();

    if ( f == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcProjecting, rcFunction, rcNull );
    }

    // linear search
    for ( auto i: self->getTextData() )
    {
        if ( id >= i.second.first && id < i.second.first + (int64_t)i.second.second )
        {
            return f( i.second.first, i.second.second, i.first.c_str(), data );
        }
    }

    return SILENT_RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}

static
rc_t CC
KTextIndexFindU64( const KIndex* bself, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    CAST();

    // linear search, find first
    for ( auto i: self->getIntData() )
    {
        if ( offset >= i.key && offset < i.key + i.key_size )
        {
            if ( key != nullptr )
            {
                *key = i.key;
            }
            if ( key_size != nullptr )
            {
                *key_size = i.key_size;
            }
            if ( id != nullptr )
            {
                *id = i.id_start;
            }
            if ( id_qty != nullptr )
            {
                *id_qty = i.id_count;
            }
            return 0;
        }
    }

    return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

static
rc_t CC
KTextIndexFindAllU64( const KIndex* bself, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data)
{
    CAST();

    if ( f == nullptr )
    {
        return SILENT_RC ( rcDB, rcIndex, rcSelecting, rcFunction, rcNull );
    }

    rc_t rc = SILENT_RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    // linear search, find all
    for ( auto i: self->getIntData() )
    {
        if ( offset >= i.key && offset < i.key + i.key_size )
        {
            rc = f( i.key, i.key_size, i.id_start, i.id_count, data );
            if (rc != 0 )
            {
                return rc;
            }
        }
    }
    return rc;
}

static
void CC
KTextIndexSetMaxRowId ( const KIndex *cself, int64_t max_row_id )
{ // nothing to do here
}
