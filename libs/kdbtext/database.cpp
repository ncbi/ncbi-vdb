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

#include "database.hpp"

#include <kdb/manager.h>
#include <klib/printf.h>

#include <algorithm>

using namespace KDBText;
using namespace std;

static rc_t CC KTextDatabaseWhack ( KTextDatabase *self );
// static bool CC KTextDatabaseLocked ( const KTextDatabase *self );
// static bool CC KTextDatabaseVExists ( const KTextDatabase *self, uint32_t type, const char *name, va_list args );
// static bool CC KTextDatabaseIsAlias ( const KTextDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name );
// static rc_t CC KTextDatabaseVWritable ( const KTextDatabase *self, uint32_t type, const char *name, va_list args );
// static rc_t CC KTextDatabaseOpenManagerRead ( const KTextDatabase *self, const KDBManager **mgr );
// static rc_t CC KTextDatabaseOpenParentRead ( const KTextDatabase *self, const KTextDatabase **par );
// static rc_t CC KTextDatabaseOpenDirectoryRead ( const KTextDatabase *self, const KDirectory **dir );
// static rc_t CC KTextDatabaseVOpenDBRead ( const KTextDatabase *self, const KTextDatabase **dbp, const char *name, va_list args );
// static rc_t CC KTextDatabaseVOpenTableRead ( const KTextDatabase *self, const KTable **tblp, const char *name, va_list args );
// static rc_t CC KTextDatabaseOpenMetadataRead ( const KTextDatabase *self, const KMetadata **metap );
// static rc_t CC KTextDatabaseVOpenIndexRead ( const KTextDatabase *self, const KIndex **idxp, const char *name, va_list args );
// static rc_t CC KTextDatabaseListDB ( const KTextDatabase *self, KNamelist **names );
// static rc_t CC KTextDatabaseListTbl ( struct KTextDatabase const *self, KNamelist **names );
// static rc_t CC KTextDatabaseListIdx ( struct KTextDatabase const *self, KNamelist **names );
// static rc_t CC KTextDatabaseGetPath ( KTextDatabase const *self, const char **path );

static KDatabase_vt KTextDatabase_vt =
{
    KTextDatabaseWhack,
    KDatabaseBaseAddRef,
    KDatabaseBaseRelease,
    // KTextDatabaseLocked,
    // KTextDatabaseVExists,
    // KTextDatabaseIsAlias,
    // KTextDatabaseVWritable,
    // KTextDatabaseOpenManagerRead,
    // KTextDatabaseOpenParentRead,
    // KTextDatabaseOpenDirectoryRead,
    // KTextDatabaseVOpenDBRead,
    // KTextDatabaseVOpenTableRead,
    // KTextDatabaseOpenMetadataRead,
    // KTextDatabaseVOpenIndexRead,
    // KTextDatabaseListDB,
    // KTextDatabaseListTbl,
    // KTextDatabaseListIdx,
    // KTextDatabaseGetPath
};

Database::Database( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextDatabase_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Database", "ctor", "db" );
}

Database::~Database()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Database" );
}

const Database *
Database::getDatabase( const std::string & name ) const
{
    for( auto & d : m_subdbs )
    {
        if ( name == d.getName() )
        {
            return & d;
        }
    }
    return nullptr;
}

const Table *
Database::getTable( const std::string & name ) const
{
    for( auto & d : m_tables )
    {
        if ( name == d.getName() )
        {
            return & d;
        }
    }
    return nullptr;
}

rc_t
Database::inflate( char * error, size_t error_size )
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
        string_printf ( error, error_size, nullptr, "Database name is missing" );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    const KJsonValue * type = KJsonObjectGetMember ( m_json, "type" );
    if ( type != nullptr )
    {
        const char * typeStr = nullptr;
        rc = KJsonGetString ( type, & typeStr );
        if ( rc == 0 )
        {
            if ( strcmp( "database", typeStr ) != 0 )
            {
                string_printf ( error, error_size, nullptr, "%s.type is not 'database'('%s')", m_name.c_str(), typeStr );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
        }
        else
        {
            string_printf ( error, error_size, nullptr, "%s.type is invalid", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
    }
    else
    {
        string_printf ( error, error_size, nullptr, "%s.type is missing", m_name.c_str() );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    // nested DBs
    const KJsonValue * dbs = KJsonObjectGetMember ( m_json, "databases" );
    if ( dbs != nullptr )
    {
        const KJsonArray * dbarr = KJsonValueToArray ( dbs );
        if ( dbarr == nullptr )
        {
            string_printf ( error, error_size, nullptr, "%s.databases is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }

        uint32_t len = KJsonArrayGetLength ( dbarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( dbarr, i );
            assert( v != nullptr );
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                Database subdb( obj );
                rc = subdb . inflate ( error, error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                for( auto & d : m_subdbs )
                {
                    if ( subdb.getName() == d.getName() )
                    {
                        string_printf ( error, error_size, nullptr, "Duplicate nested db: %s", subdb.getName().c_str() );
                        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
                    }
                }
                m_subdbs .push_back( subdb );
            }
            else
            {   // not an object
                string_printf ( error, error_size, nullptr, "%s.databases[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    // tables
    const KJsonValue * tables = KJsonObjectGetMember ( m_json, "tables" );
    if ( tables != nullptr )
    {
        const KJsonArray * tblarr = KJsonValueToArray ( tables );
        if ( tblarr == nullptr )
        {
            string_printf ( error, error_size, nullptr, "%s.tables is not an array", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
        uint32_t len = KJsonArrayGetLength ( tblarr );
        for ( uint32_t i = 0; i < len; ++i )
        {
            const KJsonValue * v = KJsonArrayGetElement ( tblarr, i );
            assert( v != nullptr );
            const KJsonObject * obj = KJsonValueToObject ( v );
            if( obj != nullptr )
            {
                Table tbl ( obj );
                rc = tbl . inflate ( error, error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                for( auto & t : m_tables )
                {
                    if ( tbl.getName() == t.getName() )
                    {
                        string_printf ( error, error_size, nullptr, "Duplicate table: %s", tbl.getName().c_str() );
                        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
                    }
                }
                m_tables .push_back( tbl );
            }
            else
            {   // not an object
                string_printf ( error, error_size, nullptr, "%s.tables[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    return rc;
}

int
Database::pathType( Path & p ) const
{
    if ( ! p.empty() && p.front() == m_name )
    {
        p.pop();
        if ( p.empty() )
        {
            return kptDatabase;
        }
        if ( p.front() == "db" )
        {
            p.pop();
            if ( ! p.empty() )
            {
                const Database * db = getDatabase( p.front() );
                if ( db != nullptr )
                {
                    return db->pathType( p );
                }
            }
        }
        else if ( p.front() == "tbl" )
        {
            p.pop();
            if ( ! p.empty() )
            {
                const Table * t = getTable( p.front() );
                if ( t != nullptr  )
                {
                    return t -> pathType( p );
                }
            }
        }
    }
    return kptNotFound;
}

bool
Database::exists( uint32_t requested, Path & path ) const
{
    if ( ! path.empty() && m_name == path.front() )
    {
        path.pop();
        if ( path.empty() )
        {
            return requested == kptDatabase;
        }

        if ( string("db") == path.front() )
        {
            path.pop();
            if (  ! path.empty() )
            {
                const Database * db = getDatabase( path.front() );
                if ( db != nullptr )
                {
                    return db->exists( requested, path );
                }
            }
        }
        else if ( string("tbl") == path.front() )
        {
            path.pop();
            if ( ! path.empty() )
            {
                const Table * tbl= getTable( path.front() );
                if ( tbl != nullptr )
                {
                    return tbl -> exists( requested, path );
                }
            }
        }
    // case kptIndex:
    // case kptColumn:
    // case kptMetadata:
    }

    return false;
}

static
rc_t CC
KTextDatabaseWhack ( KTextDatabase *self )
{
    assert( self -> dad . vt == & KTextDatabase_vt );
    delete reinterpret_cast<Database*>( self );
    return 0;
}

