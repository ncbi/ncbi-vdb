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
#include <kdb/meta.h>

#include <klib/printf.h>

using namespace KDBText;

static rc_t KTextMetadataWhack ( KMetadata *self );
static rc_t CC KTextMetadataVersion ( const KMetadata *self, uint32_t *version );
// static rc_t CC KTextMetadataByteOrder ( const KMetadata *self, bool *reversed );
// static rc_t CC KTextMetadataRevision ( const KMetadata *self, uint32_t *revision );
// static rc_t CC KTextMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );
// static rc_t CC KTextMetadataOpenRevision ( const KMetadata *self, const KMetadata **metap, uint32_t revision );
// static rc_t CC KTextMetadataGetSequence ( const KMetadata *self, const char *seq, int64_t *val );
// static rc_t CC KTextMetadataVOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, va_list args );

static KMetadata_vt KTextMetadata_vt =
{
    KTextMetadataWhack,
    KMetadataBaseAddRef,
    KMetadataBaseRelease,
    KTextMetadataVersion,
    // KTextMetadataByteOrder,
    // KTextMetadataRevision,
    // KTextMetadataMaxRevision,
    // KTextMetadataOpenRevision,
    // KTextMetadataGetSequence,
    // KTextMetadataVOpenNodeRead
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

void
Metadata::addRef( const Metadata * meta )
{
    if ( meta != nullptr )
    {
        KMetadataAddRef( (const KMetadata*) meta );
    }
}

void
Metadata::release( const Metadata * meta )
{
    if ( meta != nullptr )
    {
        KMetadataRelease( (const KMetadata*) meta );
    }
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

static
rc_t CC
KTextMetadataVersion ( const KMetadata *self, uint32_t *version )
{
    if ( version == nullptr )
    {
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    *version = 0;
    return 0;
}
