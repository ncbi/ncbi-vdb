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

#define NOMINMAX 

#include "metadata.hpp"

#include "metanode.hpp"
#include "path.hpp"

#include <kdb/manager.h>
#include <kdb/meta.h>

#include <klib/printf.h>

#include <limits>

using namespace KDBText;
using namespace std;

static rc_t KTextMetadataWhack ( KMetadata *self );
static rc_t CC KTextMetadataVersion ( const KMetadata *self, uint32_t *version );
static rc_t CC KTextMetadataByteOrder ( const KMetadata *self, bool *reversed );
static rc_t CC KTextMetadataRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KTextMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KTextMetadataOpenRevision ( const KMetadata *self, const KMetadata **metap, uint32_t revision );
static rc_t CC KTextMetadataGetSequence ( const KMetadata *self, const char *seq, int64_t *val );
static rc_t CC KTextMetadataVOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, va_list args );

static KMetadata_vt KTextMetadata_vt =
{
    KTextMetadataWhack,
    KMetadataBaseAddRef,
    KMetadataBaseRelease,
    KTextMetadataVersion,
    KTextMetadataByteOrder,
    KTextMetadataRevision,
    KTextMetadataMaxRevision,
    KTextMetadataOpenRevision,
    KTextMetadataGetSequence,
    KTextMetadataVOpenNodeRead
};

#define CAST() assert( bself->vt == &KTextMetadata_vt ); Metadata * self = (Metadata *)bself

Metadata::Metadata( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextMetadata_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Metadata", "ctor", "db" );
}

Metadata::~Metadata()
{
    Metanode::release( m_root );
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
Metadata::inflate( char * p_error, size_t p_error_size )
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
        string_printf ( p_error, p_error_size, nullptr, "Metadata name is missing" );
        return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
    }

    const KJsonValue * rev = KJsonObjectGetMember ( m_json, "revision" );
    if ( rev != nullptr )
    {
        int64_t r = 0;
        rc = KJsonGetNumber ( rev, & r );
        if ( rc != 0 || r < 0 || r > numeric_limits<int64_t>::max())
        {
            string_printf ( p_error, p_error_size, nullptr, "metadata.revision is invalid" );
            return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
        }
        m_revision = (uint32_t)r;
    }

    const KJsonValue * rootVal = KJsonObjectGetMember ( m_json, "root" );
    if ( rootVal != nullptr )
    {
        const KJsonObject * rootObj = KJsonValueToObject ( rootVal );
        if ( rootObj != nullptr )
        {
            m_root = new Metanode( rootObj );
            rc = m_root -> inflate( p_error, p_error_size );
        }
        else
        {
            string_printf ( p_error, p_error_size, nullptr, "metadata.root is invalid" );
            return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
        }
    }

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
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    *version = 0;
    return 0;
}

static
rc_t CC
KTextMetadataByteOrder ( const KMetadata *self, bool *reversed )
{
    if ( reversed == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    *reversed = false;
    return 0;
}

static
rc_t CC
KTextMetadataRevision ( const KMetadata * bself, uint32_t *revision )
{
    CAST();

    if ( revision == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    *revision = self->getRevision();
    return 0;
}

static
rc_t CC
KTextMetadataMaxRevision ( const KMetadata * bself, uint32_t *revision )
{
    CAST();

    if ( revision == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    // we only expect one revision in Json; report as the max
    *revision = self->getRevision();
    return 0;
}

static
rc_t CC
KTextMetadataOpenRevision ( const KMetadata *bself, const KMetadata **metap, uint32_t revision )
{
    CAST();

    if ( metap == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    }

    if ( revision != 0 && revision != self->getRevision() )
    {
        return SILENT_RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcNotFound  );
    }

    *metap = bself;
    return KMetadataAddRef( bself );
}

static
rc_t CC
KTextMetadataGetSequence ( const KMetadata * bself, const char *seq, int64_t *val )
{
    CAST();

    if ( val == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }
    if ( seq == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcString, rcNull );
    }
    if ( seq [ 0 ] == 0 )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcString, rcInvalid );
    }

    const KMDataNode *found;
    rc_t rc = KMDataNodeOpenNodeRead( & self -> getRoot() -> dad, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KMDataNodeRead ( found, 0, val, sizeof * val, & num_read, & remaining );
        assert ( rc != 0 || ( num_read == sizeof * val && remaining == 0 ) );
        KMDataNodeRelease ( found );
    }

    return rc;
}

static
rc_t CC
KTextMetadataVOpenNodeRead ( const KMetadata *bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    if ( node == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    }

    const Metanode * r = self->getRoot();
    if ( r == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcNotFound );
    }

    Path p(path, args);
    const Metanode * ret = r->getNode( p );
    if ( ret == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcNotFound );
    }

    *node = & ret -> dad;
    Metanode::addRef( ret );

    return 0;
}
