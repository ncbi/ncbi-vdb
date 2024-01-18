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

#include "metadata.hpp"

#include <kdb/manager.h>

#include <klib/printf.h>

using namespace KDBText;

static rc_t KTextMetadataWhack ( KMetadata *self );
// static bool CC KTextMetadataLocked ( const KMetadata *self );
// static bool CC KTextMetadataVExists ( const KMetadata *self, uint32_t type, const char *name, va_list args );
// static bool CC KTextMetadataIsAlias ( const KMetadata *self, uint32_t type, char *resolved, size_t rsize, const char *name );
// static rc_t CC KTextMetadataVWritable ( const KMetadata *self, uint32_t type, const char *name, va_list args );
// static rc_t CC KTextMetadataOpenManagerRead ( const KMetadata *self, const KDBManager **mgr );
// static rc_t CC KTextMetadataOpenParentRead ( const KMetadata *self, const KDatabase **db );
// static bool CC KTextMetadataHasRemoteData ( const KMetadata *self );
// static rc_t CC KTextMetadataOpenDirectoryRead ( const KMetadata *self, const KDirectory **dir );
// static rc_t CC KTextMetadataVOpenColumnRead ( const KMetadata *self, const KColumn **colp, const char *name, va_list args );
// static rc_t CC KTextMetadataOpenMetadataRead ( const KMetadata *self, const KMetadata **metap );
// static rc_t CC KTextMetadataVOpenMetadataRead ( const KMetadata *self, const KMetadata **idxp, const char *name, va_list args );
// static rc_t CC KTextMetadataGetPath ( const KMetadata *self, const char **path );
// static rc_t CC KTextMetadataGetName (KMetadata const *self, char const **rslt);
// static rc_t CC KTextMetadataListCol ( const KMetadata *self, KNamelist **names );
// static rc_t CC KTextMetadataListIdx ( const KMetadata *self, KNamelist **names );
// static rc_t CC KTextMetadataMetaCompare( const KMetadata *self, const KMetadata *other, const char * path, bool * equal );

static KMetadata_vt KTextMetadata_vt =
{
    KTextMetadataWhack,
    KMetadataBaseAddRef,
    KMetadataBaseRelease,
    // KTextMetadataLocked,
    // KTextMetadataVExists,
    // KTextMetadataIsAlias,
    // KTextMetadataVWritable,
    // KTextMetadataOpenManagerRead,
    // KTextMetadataOpenParentRead,
    // KTextMetadataHasRemoteData,
    // KTextMetadataOpenDirectoryRead,
    // KTextMetadataVOpenColumnRead,
    // KTextMetadataOpenMetadataRead,
    // KTextMetadataVOpenMetadataRead,
    // KTextMetadataGetPath,
    // KTextMetadataGetName,
    // KTextMetadataListCol,
    // KTextMetadataListIdx,
    // KTextMetadataMetaCompare
};

#define CAST() assert( bself->vt == &KTextMetadata_vt ); Metadata * self = (Metadata *)bself

Metadata::Metadata( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextMetadata_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Metadata", "ctor", "db" );
}

Metadata::~Metadata()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Metadata" );
}

rc_t
Metadata::inflate( char * error, size_t error_size )
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
        string_printf ( error, error_size, nullptr, "Metadata name is missing" );
        return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
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
    //     return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
    // }

    return rc;
}

static
rc_t CC
KTextMetadataWhack ( KMetadata *bself )
{
    CAST();

    delete reinterpret_cast<Metadata*>( self );
    return 0;
}