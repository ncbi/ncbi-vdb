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

#include "path.hpp"

using namespace KDBText;
using namespace std;

static rc_t CC KTextMetanodeWhack ( KMDataNode *cself );
static rc_t CC KTextMetanodeByteOrder ( const KMDataNode *self, bool *reversed );
static rc_t CC KTextMetanodeRead ( const KMDataNode *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KTextMetanodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args );
static rc_t CC KTextMetanodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size );
static rc_t CC KTextMetanodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal );
static rc_t CC KTextMetanodeAddr ( const KMDataNode *self, const void **addr, size_t *size );
static rc_t CC KTextMetanodeListAttr ( const KMDataNode *self, KNamelist **names );
static rc_t CC KTextMetanodeListChildren ( const KMDataNode *self, KNamelist **names );

static KMDataNode_vt KTextMetanode_vt =
{
    KTextMetanodeWhack,
    KMDataNodeBaseAddRef,
    KMDataNodeBaseRelease,
    KTextMetanodeByteOrder,
    KTextMetanodeRead,
    KTextMetanodeVOpenNodeRead,
    KTextMetanodeReadAttr,
    KTextMetanodeCompare,
    KTextMetanodeAddr,
    KTextMetanodeListAttr,
    KTextMetanodeListChildren
};;

#define CAST() assert( bself->vt == &KTextMetanode_vt ); Metanode * self = (Metanode *)bself

Metanode::Metanode( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextMetanode_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Metanode", "ctor", "db" );
}

Metanode::~Metanode()
{
    for (auto & ch : m_children )
    {
        Metanode::release( ch );
    }
    KRefcountWhack ( & dad . refcount, "KDBText::Metanode" );
}

void
Metanode::addRef( const Metanode * node )
{
    if ( node != nullptr )
    {
        KMDataNodeAddRef( (const KMDataNode*) node );
    }
}

void
Metanode::release( const Metanode * node )
{
    if ( node != nullptr )
    {
        KMDataNodeRelease( (const KMDataNode*) node );
    }
}

bool
Metanode::operator == ( const Metanode& other ) const
{
    if ( m_name == other.m_name &&
         m_value == other.m_value &&
         m_attrs == other.m_attrs &&
         m_children.size() == other.m_children.size() )
    {   // deep compare of children
        for ( size_t i = 0; i < m_children.size(); ++i )
        {
            if ( ! (*m_children[0] == *other.m_children[0]) )
            {
                return false;
            }
        }
        return true;
    }
    return false;
}


static
rc_t
ParseValue( const KJsonValue * value, string & out_value, char * p_error, size_t p_error_size )
{
    rc_t rc = 0;

    switch( KJsonGetValueType ( value ) )
    {
    case jsString:
    case jsNumber:
    {
        // first, see if it is a number
        int64_t i;
        rc = KJsonGetNumber ( value, &i );
        if ( rc == 0 )
        {
            out_value = string( (const char*)&i, sizeof(i) );
        }
        else
        {   // a double?
            double d;
            rc = KJsonGetDouble ( value, &d );
            if ( rc == 0 )
            {
                out_value = string( (const char*)&d, sizeof(d) );
            }
            else
            {
                const char * valStr = nullptr;
                rc = KJsonGetString ( value, & valStr );
                if ( rc == 0 )
                {
                    out_value = valStr;
                }
            }
        }
        break;
    }

    case jsBool:
    {
        bool b;
        rc = KJsonGetBool ( value, & b );
        if ( rc == 0 )
        {   // represent as 1 byte binary
            out_value = string( b ? "\x01" : "\x00", 1 );
        }
        break;
    }
    case jsNull:
        break;
    case jsObject: // expect a sized integer
    {
        const KJsonObject * obj = KJsonValueToObject ( value );
        const KJsonValue * sizeVal = KJsonObjectGetMember ( obj, "size" );
        if ( sizeVal != nullptr )
        {
            int64_t size;
            rc = KJsonGetNumber ( sizeVal, &size );
            if ( rc == 0 )
            {
                const KJsonValue * intVal = KJsonObjectGetMember ( obj, "int" );
                if ( intVal != nullptr )
                {
                    int64_t num;
                    rc = KJsonGetNumber ( intVal, &num );
                    if ( rc == 0 )
                    {
                        switch (size)
                        {
                        case 1: { uint8_t  v = (uint8_t)num;  out_value = string( (const char*)&v, size ); break; }
                        case 2: { uint16_t v = (uint16_t)num; out_value = string( (const char*)&v, size ); break; }
                        case 4: { uint32_t v = (uint32_t)num; out_value = string( (const char*)&v, size ); break; }
                        case 8: { uint64_t v = (uint64_t)num; out_value = string( (const char*)&v, size ); break; }
                        default:
                            string_printf ( p_error, p_error_size, nullptr, "Invalid integer size(only 1,2,4,8 are supported)" );
                            rc = SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );;
                        }
                    }
                }
                else
                {
                    string_printf ( p_error, p_error_size, nullptr, "Sized integer is missing \"int\":" );
                    rc = SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );;
                }
            }
        }
        else
        {
            string_printf ( p_error, p_error_size, nullptr, "Sized integer is missing \"size\":" );
            rc = SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );;
        }

        break;
    }
    default:
        rc = SILENT_RC( rcDB, rcMetadata, rcCreating, rcParam, rcInvalid );
        break;
    }

    return rc;
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
        if ( value != nullptr )
        {
            rc = ParseValue( value, m_value, p_error, p_error_size );
            if ( rc != 0 )
            {
                return rc;
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
                        Metanode * ch = new Metanode( obj );
                        rc = ch -> inflate( p_error, p_error_size );
                        if ( rc == 0 )
                        {
                            m_children.push_back( ch );
                        }
                        else
                        {
                            delete ch;
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

const Metanode *
Metanode::getNode( Path & p ) const
{
    if ( p.empty() ||
         ( p.size() == 1 && p.front() == "." ) )
    {
        return this;
    }
    if ( p.front() == "." )
    {
        p.pop();
    }

    for( auto const & c : m_children )
    {
        if ( c -> getName() == p.front() )
        {
            p.pop();
            return c -> getNode( p );
        }
    }

    return nullptr;
}

// API

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

static
rc_t CC
KTextMetanodeRead ( const KMDataNode * bself, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    CAST();

    if ( num_read == nullptr )
    {
        return SILENT_RC ( rcDB, rcNode, rcReading, rcParam, rcNull );
    }
    if ( buffer == nullptr && bsize != 0 )
    {
        return SILENT_RC ( rcDB, rcNode, rcReading, rcBuffer, rcNull );
    }

    if ( offset >= self->getValue().size() )
    {
        * num_read = 0;
        if ( remaining != nullptr )
        {
            *remaining = 0;
        }
    }
    else
    {
        size_t to_read = self->getValue().size() - offset;
        if ( to_read > bsize )
        {
            to_read = bsize;
        }

        if ( to_read > 0 )
        {
            memmove ( buffer, self->getValue().c_str() + offset, to_read );
        }

        * num_read = to_read;
        if ( remaining != nullptr )
        {
            *remaining = self->getValue().size() - offset - to_read;
        }
    }

    return 0;
}

static
rc_t CC
KTextMetanodeVOpenNodeRead ( const KMDataNode * bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    if ( node == nullptr )
    {
        return SILENT_RC ( rcDB, rcNode, rcOpening, rcParam, rcNull );
    }

    Path p ( path, args );
    const Metanode * n = self -> getNode( p );
    if ( n == nullptr )
    {
        *node = nullptr;
        return SILENT_RC ( rcDB, rcMetadata, rcSelecting, rcPath, rcNotFound );
    }

    Metanode::addRef( n );
    *node = (const KMDataNode *)n;
    return 0;
}

static
rc_t CC
KTextMetanodeReadAttr ( const KMDataNode * bself, const char *name, char *buffer, size_t bsize, size_t *size )
{
    CAST();

    if ( size == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    }
    if ( name == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcString, rcNull );
    }
    if ( name [ 0 ] == 0 )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcString, rcEmpty );
    }
    if ( buffer == nullptr && bsize != 0 )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcNull );
    }

    auto attr = self->getAttributes().find(name);
    if ( attr == self->getAttributes().end() )
    {
        * size = 0;
        if ( bsize != 0 )
        {
            buffer [ 0 ] = 0;
        }
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcAttr, rcNotFound );
    }

    * size = attr->second.size();
    if ( * size >= bsize )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcBuffer, rcInsufficient );
    }
    memmove ( buffer, attr->second.data(), *size);
    buffer [ *size ] = 0;

    return 0;
}

static
rc_t CC
KTextMetanodeCompare( const KMDataNode *bself, KMDataNode const *bother, bool *equal )
{
    CAST();
    assert( bself->vt == &KTextMetanode_vt );
    Metanode * other = (Metanode *)bother;

    if ( other == nullptr || equal == nullptr )
    {
        return SILENT_RC( rcDB, rcNode, rcComparing, rcParam, rcNull );
    }

    *equal = *self == *other;
    return 0;
}

static
rc_t CC
KTextMetanodeAddr ( const KMDataNode *bself, const void **addr, size_t *size )
{
    CAST();

    if ( addr == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcReading, rcParam, rcNull );
    }

    * addr = self->getValue().data();
    if ( size != nullptr )
    {
        * size = self->getValue().size();
    }
    return 0;
}

static
rc_t CC
KTextMetanodeListAttr ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    if ( names == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );
    }

    VNamelist * ret;
    const Metanode::Attributes & attrs = self -> getAttributes();
    rc_t rc = VNamelistMake( & ret, attrs.size() );
    if (rc == 0 )
    {
        for (auto & key_val : attrs )
        {
            VNamelistAppend ( ret, key_val . first . c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return 0;
}

static
rc_t CC
KTextMetanodeListChildren ( const KMDataNode *bself, KNamelist **names )
{
    CAST();

    if ( names == nullptr )
    {
        return SILENT_RC ( rcDB, rcMetadata, rcListing, rcParam, rcNull );
    }

    VNamelist * ret;
    const Metanode::Children & children = self -> getChildren();
    rc_t rc = VNamelistMake( & ret, children.size() );
    if (rc == 0 )
    {
        for (auto & ch : children )
        {
            VNamelistAppend ( ret, ch -> getName().c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return 0;
}
