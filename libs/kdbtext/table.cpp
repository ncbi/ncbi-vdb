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

#include "table.hpp"

#include "metadata.hpp"
#include "manager.hpp"
#include "database.hpp"
#include "column.hpp"

#include <kdb/manager.h>
#include <kdb/table.h>

#include <klib/printf.h>
#include <klib/namelist.h>

using namespace KDBText;
using namespace std;

static rc_t KTextTableWhack ( KTable *self );
static bool CC KTextTableLocked ( const KTable *self );
static bool CC KTextTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args );
static bool CC KTextTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KTextTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args );
static rc_t CC KTextTableOpenManagerRead ( const KTable *self, const KDBManager **mgr );
static rc_t CC KTextTableOpenParentRead ( const KTable *self, const KDatabase **db );
static bool CC KTextTableHasRemoteData ( const KTable *self );
static rc_t CC KTextTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir );
static rc_t CC KTextTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args );
static rc_t CC KTextTableOpenMetadataRead ( const KTable *self, const KMetadata **metap );
static rc_t CC KTextTableVOpenIndexRead ( const KTable *self, const KIndex **idxp, const char *name, va_list args );
static rc_t CC KTextTableGetPath ( const KTable *self, const char **path );
static rc_t CC KTextTableGetName (KTable const *self, char const **rslt);
static rc_t CC KTextTableListCol ( const KTable *self, KNamelist **names );
static rc_t CC KTextTableListIdx ( const KTable *self, KNamelist **names );
static rc_t CC KTextTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal );

static KTable_vt KTextTable_vt =
{
    KTextTableWhack,
    KTableBaseAddRef,
    KTableBaseRelease,
    KTextTableLocked,
    KTextTableVExists,
    KTextTableIsAlias,
    KTextTableVWritable,
    KTextTableOpenManagerRead,
    KTextTableOpenParentRead,
    KTextTableHasRemoteData,
    KTextTableOpenDirectoryRead,
    KTextTableVOpenColumnRead,
    KTextTableOpenMetadataRead,
    KTextTableVOpenIndexRead,
    KTextTableGetPath,
    KTextTableGetName,
    KTextTableListCol,
    KTextTableListIdx,
    KTextTableMetaCompare
};

#define CAST() assert( bself->vt == &KTextTable_vt ); Table * self = (Table *)bself

static char error[1024];

Table::Table( const KJsonObject * p_json, const Manager * p_mgr, const Database * p_parent )
: m_mgr( p_mgr ), m_parent( p_parent ), m_json ( p_json )
{
    dad . vt = & KTextTable_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Table", "ctor", "db" );
    Manager::addRef( m_mgr );
}

Table::~Table()
{
    Metadata::release( m_meta );
    Manager::release( m_mgr );
    KRefcountWhack ( & dad . refcount, "KDBText::Table" );
}

void
Table::addRef( const Table * tbl )
{
    if ( tbl != nullptr )
    {
        KTableAddRef( (const KTable*) tbl );
    }
}

void
Table::release( const Table * tbl )
{
    if ( tbl != nullptr )
    {
        KTableRelease( (const KTable*) tbl );
    }
}

const Column *
Table::openColumn( const string & name ) const
{
    auto j = m_columns.find( name );
    if ( j != m_columns.end() )
    {
        Column * ret = new Column( j -> second, m_mgr, this );
        ret -> inflate( error, sizeof error );
        return ret;
    }
    return nullptr;
}

const Index *
Table::openIndex( const string & name ) const
{
    auto j = m_indexes.find( name );
    if ( j != m_indexes.end() )
    {
        Index * ret = new Index( j -> second, this );
        ret -> inflate( error, sizeof error );
        return ret;
    }
    return nullptr;
}

rc_t
Table::inflate( char * p_error, size_t p_error_size )
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
        string_printf ( p_error, p_error_size, nullptr, "Table name is missing" );
        return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
    }

    const KJsonValue * type = KJsonObjectGetMember ( m_json, "type" );
    if ( type != nullptr )
    {
        const char * typeStr = nullptr;
        rc = KJsonGetString ( type, & typeStr );
        if ( rc == 0 )
        {
            if ( strcmp( "table", typeStr ) != 0 )
            {
                string_printf ( p_error, p_error_size, nullptr, "%s.type is not 'table'('%s')", m_name.c_str(), typeStr );
                return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
            }
        }
        else
        {
            string_printf ( p_error, p_error_size, nullptr, "%s.type is invalid", m_name.c_str() );
            return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
        }
    }
    else
    {
        string_printf ( p_error, p_error_size, nullptr, "%s.type is missing", m_name.c_str() );
        return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
    }

    // Columns
    const KJsonValue * columns = KJsonObjectGetMember ( m_json, "columns" );
    if ( columns != nullptr )
    {
        const KJsonArray * colarr = KJsonValueToArray ( columns );
        if ( colarr == nullptr )
        {
            string_printf ( p_error, p_error_size, nullptr, "%s.columns is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
        }

        uint32_t len = KJsonArrayGetLength ( colarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( colarr, i );
            assert( v != nullptr );
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                Column col( obj ); // a temporary for Json verification
                rc = col . inflate ( p_error, p_error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                if ( m_columns.find( col . getName() ) != m_columns . end() )
                {
                    string_printf ( p_error, p_error_size, nullptr, "Duplicate column: %s", col . getName().c_str() );
                    return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
                }
                m_columns [ col . getName() ] = obj;
            }
            else
            {   // not an object
                string_printf ( p_error, p_error_size, nullptr, "%s.columns[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    // Indexes
    const KJsonValue * indexes = KJsonObjectGetMember ( m_json, "indexes" );
    if ( indexes != nullptr )
    {
        const KJsonArray * idxarr = KJsonValueToArray ( indexes );
        if ( idxarr == nullptr )
        {
            string_printf ( p_error, p_error_size, nullptr, "%s.indexes is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
        }

        uint32_t len = KJsonArrayGetLength ( idxarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( idxarr, i );
            assert( v != nullptr );
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                Index idx( obj ); // a temporary for Json verification
                rc = idx . inflate ( p_error, p_error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                if ( m_indexes.find( idx . getName() ) != m_indexes . end() )
                {
                    string_printf ( p_error, p_error_size, nullptr, "Duplicate index: %s", idx.getName().c_str() );
                    return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
                }
                m_indexes [ idx . getName() ] = obj;
            }
            else
            {   // not an object
                string_printf ( p_error, p_error_size, nullptr, "%s.indexes[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcTable, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    // metadata
    const KJsonValue * meta = KJsonObjectGetMember ( m_json, "metadata" );
    if ( meta != nullptr )
    {
        const KJsonObject * obj = KJsonValueToObject ( meta );
        if( obj != nullptr )
        {
            Metadata * m  = new Metadata( obj );
            rc = m -> inflate( p_error, p_error_size );
            if ( rc != 0 )
            {
                delete m;
                return rc;
            }
            m_meta = m;
        }
        else
        {   // not an object
            string_printf ( p_error, p_error_size, nullptr, "%s.metadata is not an object", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
    }

    return rc;
}

int
Table::pathType( Path & path ) const
{
    if ( ! path.empty() )
    {
        if ( path.front() == m_name )
        {
            path.pop();
            if ( path.empty() )
            {
                return kptTable;
            }
            if ( path.front() == "col" )
            {
                path.pop();
                if ( path.size() == 1 && hasColumn( path.front() ) )
                {
                    return kptColumn;
                }
            }
            else if ( path.front() == "md" )
            {
                if ( path.size() == 1 && m_meta != nullptr )
                {
                    return kptMetadata;
                }
            }
            else if ( path.front() == "idx" )
            {
                path.pop();
                if ( path.size() == 1 && hasIndex( path.front() ) )
                {
                    return kptIndex;
                }
            }
        }
    }
    return kptNotFound;
}

bool
Table::exists( uint32_t requested, Path & path ) const
{
    if ( ! path.empty() && m_name == path.front() )
    {
        path.pop();
        if ( path.empty() )
        {
            return requested == kptTable;
        }
    }

    if ( ! path.empty() )
    {
        if ( path.front() == "idx" )
        {
            path.pop();
            if ( path.size() == 1 )
            {
                return hasIndex( path.front() );
            }
        }
        else if ( path.front() == "col" )
        {
            path.pop();
            if ( path.size() == 1 )
            {
                return hasColumn( path.front() );
            }
        }
        else if ( path.front() == "md" )
        {
            return path.size() == 1 && m_meta != nullptr;
        }
    }

    return false;
}

static
rc_t CC
KTextTableWhack ( KTable *bself )
{
    CAST();

    delete reinterpret_cast<Table*>( self );
    return 0;
}

static
bool CC
KTextTableLocked ( const KTable *self )
{
    return false;
}

static
bool CC
KTextTableVExists ( const KTable *bself, uint32_t type, const char *name, va_list args )
{
    CAST();
    Path p ( name, args );
    return self -> exists( type, p );
}

static
bool CC
KTextTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    return false;
}

static
rc_t CC
KTextTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    return false;
}

static
rc_t CC
KTextTableOpenManagerRead ( const KTable *bself, const KDBManager **mgr )
{
    CAST();

    const Manager * m = self -> getManager();
    if ( m != nullptr )
    {
        Manager::addRef( m );
    }

    *mgr = (const KDBManager*)m;

    return 0;
}

static
rc_t CC
KTextTableOpenParentRead ( const KTable *bself, const KDatabase **par )
{
    CAST();

    const Database * p = self -> getParent();
    if ( p != nullptr )
    {
        Database::addRef( p );
    }

    *par = (const KDatabase*) p;

    return 0;
}

static
bool CC
KTextTableHasRemoteData ( const KTable *self )
{
    return false;
}

static
rc_t CC
KTextTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir )
{
    return SILENT_RC ( rcDB, rcTable, rcAccessing, rcDirectory, rcUnsupported );
}

static
rc_t CC
KTextTableVOpenColumnRead ( const KTable * bself, const KColumn **colp, const char *fmt, va_list args )
{
    CAST();

    string name;
    rc_t rc = Path::PrintToString( fmt, args, name );
    if ( rc == 0 )
    {
        const Column * col = self -> openColumn( name );
        if ( col != nullptr )
        {
            *colp = (const KColumn*)col;
        }
        else
        {
            rc =  SILENT_RC( rcDB, rcTable, rcOpening, rcColumn, rcNotFound );
        }
    }
    return rc;
}

static
rc_t CC
KTextTableOpenMetadataRead ( const KTable * bself, const KMetadata **metap )
{
    CAST();
    const Metadata * m = self->openMetadata();
    if ( m != nullptr )
    {
        Metadata::addRef( m );
        *metap = (const KMetadata*)m;
        return 0;
    }
    return SILENT_RC( rcDB, rcTable, rcOpening, rcMetadata, rcInvalid );;
}

static
rc_t CC
KTextTableVOpenIndexRead ( const KTable * bself, const KIndex **idxp, const char *fmt, va_list args )
{
    CAST();

    string name;
    rc_t rc = Path::PrintToString( fmt, args, name );
    if ( rc == 0 )
    {
        const Index * idx = self->openIndex( name );
        if ( idx != nullptr )
        {
            *idxp = (const KIndex*)idx;
            return 0;
        }
    }
    return SILENT_RC( rcDB, rcTable, rcOpening, rcIndex, rcInvalid );;
}

static
rc_t CC
KTextTableGetPath ( const KTable *self, const char **path )
{
    return SILENT_RC ( rcDB, rcTable, rcAccessing, rcPath, rcUnsupported );
}

static
rc_t CC
KTextTableGetName (KTable const * bself, char const **rslt)
{
    CAST();

    *rslt = self->getName().c_str();
    return 0;
}

static
rc_t CC
KTextTableListCol ( const KTable * bself, KNamelist **names )
{
    CAST();

    VNamelist * ret;
    const Table::Subobjects & cols = self -> getColumns();
    rc_t rc = VNamelistMake( & ret, cols.size() );
    if (rc == 0 )
    {
        for (auto & key_val : cols )
        {
            VNamelistAppend ( ret, key_val . first . c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return rc;
}

static
rc_t CC
KTextTableListIdx ( const KTable * bself, KNamelist **names )
{
    CAST();

    VNamelist * ret;
    const Table::Subobjects & cols = self -> getIndexes();
    rc_t rc = VNamelistMake( & ret, cols.size() );
    if (rc == 0 )
    {
        for (auto & key_val : cols )
        {
            VNamelistAppend ( ret, key_val . first . c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return rc;
}

static
rc_t CC
KTextTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal )
{
    return SILENT_RC ( rcDB, rcTable, rcComparing, rcMetadata, rcUnsupported );
}

