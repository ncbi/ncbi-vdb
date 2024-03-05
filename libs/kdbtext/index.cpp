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
// static bool CC KTextIndexLocked ( const KIndex *self );
// static bool CC KTextIndexVExists ( const KIndex *self, uint32_t type, const char *name, va_list args );
// static bool CC KTextIndexIsAlias ( const KIndex *self, uint32_t type, char *resolved, size_t rsize, const char *name );
// static rc_t CC KTextIndexVWritable ( const KIndex *self, uint32_t type, const char *name, va_list args );
// static rc_t CC KTextIndexOpenManagerRead ( const KIndex *self, const KDBManager **mgr );
// static rc_t CC KTextIndexOpenParentRead ( const KIndex *self, const KDatabase **db );
// static bool CC KTextIndexHasRemoteData ( const KIndex *self );
// static rc_t CC KTextIndexOpenDirectoryRead ( const KIndex *self, const KDirectory **dir );
// static rc_t CC KTextIndexVOpenColumnRead ( const KIndex *self, const KColumn **colp, const char *name, va_list args );
// static rc_t CC KTextIndexOpenMetadataRead ( const KIndex *self, const KMetadata **metap );
// static rc_t CC KTextIndexVOpenIndexRead ( const KIndex *self, const KIndex **idxp, const char *name, va_list args );
// static rc_t CC KTextIndexGetPath ( const KIndex *self, const char **path );
// static rc_t CC KTextIndexGetName (KIndex const *self, char const **rslt);
// static rc_t CC KTextIndexListCol ( const KIndex *self, KNamelist **names );
// static rc_t CC KTextIndexListIdx ( const KIndex *self, KNamelist **names );
// static rc_t CC KTextIndexMetaCompare( const KIndex *self, const KIndex *other, const char * path, bool * equal );

static KIndex_vt KTextIndex_vt =
{
    KTextIndexWhack,
    KIndexBaseAddRef,
    KIndexBaseRelease,
    // KTextIndexLocked,
    // KTextIndexVExists,
    // KTextIndexIsAlias,
    // KTextIndexVWritable,
    // KTextIndexOpenManagerRead,
    // KTextIndexOpenParentRead,
    // KTextIndexHasRemoteData,
    // KTextIndexOpenDirectoryRead,
    // KTextIndexVOpenColumnRead,
    // KTextIndexOpenMetadataRead,
    // KTextIndexVOpenIndexRead,
    // KTextIndexGetPath,
    // KTextIndexGetName,
    // KTextIndexListCol,
    // KTextIndexListIdx,
    // KTextIndexMetaCompare
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