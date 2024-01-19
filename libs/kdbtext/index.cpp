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

using namespace KDBText;

static rc_t KTextIndexWhack ( KIndex *self );
static bool CC KTextIndexLocked ( const KIndex *self );
static rc_t CC KTextIndexVersion ( const KIndex *self, uint32_t *version );
static rc_t CC KTextIndexType ( const KIndex *self, KIdxType *type );
static rc_t CC KTextIndexConsistencyCheck ( const KIndex *self, uint32_t level,
     int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
     uint64_t *num_rows, uint64_t *num_holes );
// static rc_t CC KTextIndexFindText ( const KIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
//     int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
//     void *data );
// static rc_t CC KTextIndexFindAllText ( const KIndex *self, const char *key,
//     rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data );
// static rc_t CC KTextIndexProjectText ( const KIndex *self,
//     int64_t id, int64_t *start_id, uint64_t *id_count,
//     char *key, size_t kmax, size_t *actsize );
// static rc_t CC KTextIndexProjectAllText ( const KIndex *self, int64_t id,
//     rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
//     void *data );
// static rc_t CC KTextIndexFindU64( const KIndex* self, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty );
// static rc_t CC KTextIndexFindAllU64( const KIndex* self, uint64_t offset,
//     rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data);
// static void CC KTextIndexSetMaxRowId ( const KIndex *cself, int64_t max_row_id );

static KIndex_vt KTextIndex_vt =
{
    KTextIndexWhack,
    KIndexBaseAddRef,
    KIndexBaseRelease,
    KTextIndexLocked,
    KTextIndexVersion,
    KTextIndexType,
    KTextIndexConsistencyCheck,
    // KTextIndexFindText,
    // KTextIndexFindAllText,
    // KTextIndexProjectText,
    // KTextIndexProjectAllText,
    // KTextIndexFindU64,
    // KTextIndexFindAllU64,
    // KTextIndexSetMaxRowId
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
Index::inflate( char * error, size_t error_size )
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
        string_printf ( error, error_size, nullptr, "Index name is missing" );
        return SILENT_RC( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
    }

    // const KJsonValue * type = KJsonObjectGetMember ( m_json, "type" );
    // if ( type != nullptr )
    // {
    //     const char * typeStr = nullptr;
    //     rc = KJsonGetString ( type, & typeStr );
    //     //TBD
    // }
    // else
    // {
    //     string_printf ( error, error_size, nullptr, "%s.type is missing", m_name.c_str() );
    //     return SILENT_RC( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
    // }

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
    if ( version == NULL )
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
    if ( type == NULL )
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
