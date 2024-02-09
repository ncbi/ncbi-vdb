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

#include "metanode.hpp"

#include <kdb/meta.h>

#include <klib/printf.h>
#include <klib/namelist.h>

using namespace KDBText;
using namespace std;

static rc_t CC KTextMetanodeWhack ( KMDataNode *cself );
// static rc_t CC MetanodeByteOrder ( const KMDataNode *self, bool *reversed );
// static rc_t CC MetanodeRead ( const KMDataNode *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
// static rc_t CC MetanodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args );
// static rc_t CC MetanodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size );
// static rc_t CC MetanodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal );
// static rc_t CC MetanodeAddr ( const KMDataNode *self, const void **addr, size_t *size );
// static rc_t CC MetanodeListAttr ( const KMDataNode *self, KNamelist **names );
// static rc_t CC MetanodeListChildren ( const KMDataNode *self, KNamelist **names );

static KMDataNode_vt KTextMetanode_vt =
{
    KTextMetanodeWhack,
    KMDataNodeBaseAddRef,
    KMDataNodeBaseRelease,
    // MetanodeByteOrder,
    // MetanodeRead,
    // MetanodeVOpenNodeRead,
    // MetanodeReadAttr,
    // MetanodeCompare,
    // MetanodeAddr,
    // MetanodeListAttr,
    // MetanodeListChildren
};;

#define CAST() assert( bself->vt == &KTextMetanode_vt ); Metanode * self = (Metanode *)bself

Metanode::Metanode( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextMetanode_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Metanode", "ctor", "db" );
}

Metanode::~Metanode()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Metanode" );
}

void
Metanode::addRef( const Metanode * meta )
{
    if ( meta != nullptr )
    {
        KMDataNodeAddRef( (const KMDataNode*) meta );
    }
}

void
Metanode::release( const Metanode * meta )
{
    if ( meta != nullptr )
    {
        KMDataNodeRelease( (const KMDataNode*) meta );
    }
}

rc_t
Metanode::inflate( char * p_error, size_t p_error_size )
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
        string_printf ( p_error, p_error_size, nullptr, "Metanode name is missing" );
        return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
    }
    if ( rc == 0 )
    {
        const KJsonValue * value = KJsonObjectGetMember ( m_json, "value" );
        if ( value != nullptr && KJsonGetValueType ( value ) != jsNull )
        {
            const char * valStr = nullptr;
            rc = KJsonGetString ( value, & valStr );
            if ( rc == 0 )
            {
                m_value = valStr;
            }
            else
            {
                string_printf ( p_error, p_error_size, nullptr, "Metanode value is invalid" );
                return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
            }
        }

        const KJsonValue * attrs = KJsonObjectGetMember ( m_json, "attributes" );
        if ( attrs != nullptr && KJsonGetValueType ( attrs ) != jsNull )
        {
            const KJsonObject * attrsObj = KJsonValueToObject ( attrs );
            if ( attrsObj != nullptr )
            {
                struct VNamelist * names;
                rc = VNamelistMake ( &names, 0 );
                if ( rc == 0 )
                {
                    rc = KJsonObjectGetNames ( attrsObj, names );
                    if ( rc == 0 )
                    {
                        uint32_t count;
                        rc = VNameListCount ( names, &count );
                        for ( uint32_t i = 0; i < count; ++i )
                        {
                            const char * name;
                            rc = VNameListGet ( names, i, & name );
                            if ( rc == 0 )
                            {
                                const KJsonValue * vObj = KJsonObjectGetMember ( attrsObj, name );
                                const char * v;
                                rc = KJsonGetString ( vObj, &v );
                                if ( rc == 0 )
                                {   // json parser guarantees that names are all distinct
                                    m_attrs[ name ] = v;
                                }
                                else
                                {
                                    string_printf ( p_error, p_error_size, nullptr, "Metanode attribute %s is invalid", name );
                                    rc = SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
                                    break;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    rc_t rc2 = VNamelistRelease( names );
                    if ( rc == 0 )
                    {
                        rc = rc2;
                    }
                }
            }
            else
            {
                string_printf ( p_error, p_error_size, nullptr, "Metanode attributes object is invalid" );
                return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
            }
        }

        const KJsonValue * children = KJsonObjectGetMember ( m_json, "children" );
        if ( children != nullptr && KJsonGetValueType ( children ) != jsNull )
        {
            const KJsonArray * chArr = KJsonValueToArray ( children );
            if ( chArr != nullptr )
            {
                uint32_t len = KJsonArrayGetLength ( chArr );
                for ( uint32_t i = 0; i < len; ++i )
                {
                    const KJsonValue * v = KJsonArrayGetElement ( chArr, i );
                    assert( v != nullptr );
                    const KJsonObject * obj = KJsonValueToObject ( v );
                    if( obj != nullptr )
                    {
                        Metanode ch( obj );
                        rc = ch.inflate( p_error, p_error_size );
                        if ( rc == 0 )
                        {
                            m_children.push_back( ch );
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {   // not an object
                        string_printf ( p_error, p_error_size, nullptr, "Metanode.child[%i] is not an object", i );
                        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                    }
                }
            }
            else
            {
                string_printf ( p_error, p_error_size, nullptr, "Metanode.children is not an array" );
                return SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    return rc;
}

static
rc_t CC
KTextMetanodeWhack ( KMDataNode *bself )
{
    CAST();

    delete reinterpret_cast<Metanode*>( self );
    return 0;
}

static
rc_t CC
KTextMetanodeByteOrder ( const KMDataNode *self, bool *reversed )
{
    if ( reversed == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    }

    *reversed = false;
    return 0;
}

