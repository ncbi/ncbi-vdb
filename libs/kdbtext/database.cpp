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

#include "table.hpp"
#include "manager.hpp"
#include "metadata.hpp"

#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/database.h>

#include <klib/printf.h>
#include <klib/namelist.h>

#include <algorithm>

using namespace KDBText;
using namespace std;

static rc_t CC KTextDatabaseWhack ( KTextDatabase *self );
static bool CC KTextDatabaseLocked ( const KTextDatabase *self );
static bool CC KTextDatabaseVExists ( const KTextDatabase *self, uint32_t type, const char *name, va_list args );
static bool CC KTextDatabaseIsAlias ( const KTextDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KTextDatabaseVWritable ( const KTextDatabase *self, uint32_t type, const char *name, va_list args );
static rc_t CC KTextDatabaseOpenManagerRead ( const KTextDatabase *self, const KDBManager **mgr );
static rc_t CC KTextDatabaseOpenParentRead ( const KTextDatabase *self, const KDatabase **par );
static rc_t CC KTextDatabaseOpenDirectoryRead ( const KTextDatabase *self, const KDirectory **dir );
static rc_t CC KTextDatabaseVOpenDBRead ( const KTextDatabase *self, const KDatabase **dbp, const char *name, va_list args );
static rc_t CC KTextDatabaseVOpenTableRead ( const KTextDatabase *self, const KTable **tblp, const char *name, va_list args );
static rc_t CC KTextDatabaseOpenMetadataRead ( const KTextDatabase *self, const KMetadata **metap );
static rc_t CC KTextDatabaseVOpenIndexRead ( const KTextDatabase *self, const KIndex **idxp, const char *name, va_list args );
static rc_t CC KTextDatabaseListDB ( const KTextDatabase *self, KNamelist **names );
static rc_t CC KTextDatabaseListTbl ( struct KTextDatabase const *self, KNamelist **names );
static rc_t CC KTextDatabaseListIdx ( struct KTextDatabase const *self, KNamelist **names );
static rc_t CC KTextDatabaseGetPath ( KTextDatabase const *self, const char **path );

static KDatabase_vt KTextDatabase_vt =
{
    KTextDatabaseWhack,
    KDatabaseBaseAddRef,
    KDatabaseBaseRelease,
    KTextDatabaseLocked,
    KTextDatabaseVExists,
    KTextDatabaseIsAlias,
    KTextDatabaseVWritable,
    KTextDatabaseOpenManagerRead,
    KTextDatabaseOpenParentRead,
    KTextDatabaseOpenDirectoryRead,
    KTextDatabaseVOpenDBRead,
    KTextDatabaseVOpenTableRead,
    KTextDatabaseOpenMetadataRead,
    KTextDatabaseVOpenIndexRead,
    KTextDatabaseListDB,
    KTextDatabaseListTbl,
    KTextDatabaseListIdx,
    KTextDatabaseGetPath
};

static char error[1024];

Database::Database( const KJsonObject * p_json, const Manager * p_mgr, const Database * p_parent )
: m_mgr( p_mgr ), m_parent( p_parent ), m_json ( p_json )
{
    dad . vt = & KTextDatabase_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Database", "ctor", "db" );
    Manager::addRef( m_mgr );
}

Database::~Database()
{
    Metadata::release( m_meta );
    Manager::release( m_mgr );
    KRefcountWhack ( & dad . refcount, "KDBText::Database" );
}

void
Database::addRef( const Database * db )
{
    if ( db != nullptr )
    {
        KDatabaseAddRef( (const KDatabase*) db );
    }
}

void
Database::release( const Database * db )
{
    if ( db != nullptr )
    {
        KDatabaseRelease( (const KDatabase*) db );
    }
}

const Database *
Database::openDatabase( Path & p_path ) const
{
    if ( ! p_path.empty() && p_path.front() == m_name )
    {
        p_path.pop();
        if ( p_path.empty() )
        {   // return a new copy of this db
            Database * ret = new Database( m_json, m_mgr, this );
            ret -> inflate( error, sizeof error );
            return ret;
        }
        if ( p_path.front() == "db" )
        {
            p_path.pop();
            if ( ! p_path.empty() )
            {
                auto j = m_subdbs.find( p_path.front() );
                if ( j != m_subdbs.end() )
                {
                    Database * ret = new Database( j -> second, m_mgr, this );
                    ret -> inflate( error, sizeof error );
                    return ret;
                }
            }
        }
    }
    return nullptr;
}

const Database *
Database::openSubDatabase( const std::string & name ) const
{
    auto j = m_subdbs.find( name );
    if ( j != m_subdbs.end() )
    {
        Database * ret = new Database( j -> second, m_mgr, this );
        ret -> inflate( error, sizeof error );
        return ret;
    }
    return nullptr;
}

const Table *
Database::openTable( Path & p_path ) const
{
    if ( ! p_path.empty() && p_path.front() == m_name )
    {   // remove a possible db name from the front
        p_path.pop();
    }
    if ( ! p_path.empty() )
    {
        if ( p_path.front() == "tbl" )
        {
            p_path.pop();
            if ( ! p_path.empty() )
            {
                auto j = m_tables.find( p_path.front() );
                if ( j != m_tables.end() )
                {
                    Table * ret = new Table( j -> second, m_mgr, this );
                    ret -> inflate( error, sizeof error );
                    return ret;
                }
            }
        }
        else if ( p_path.front() == "db" )
        {
            p_path.pop();
            if ( ! p_path.empty() )
            {
                auto j = m_subdbs.find( p_path.front() );
                if ( j != m_subdbs.end() )
                {
                    Database * db = new Database( j -> second, m_mgr, this );
                    if ( db != nullptr )
                    {
                        db -> inflate( error, sizeof error);
                        const Table * ret = db -> openTable( p_path );
                        delete db;
                        return ret;
                    }
                }
            }
        }
    }
    return nullptr;
}

const Table *
Database::openTable( const string & name ) const
{
    auto j = m_tables.find( name );
    if ( j != m_tables.end() )
    {
        Table * ret = new Table( j -> second, m_mgr, this );
        ret -> inflate( error, sizeof error );
        return ret;
    }
    return nullptr;
}

rc_t
Database::inflate( char * p_error, size_t error_size )
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
        string_printf ( p_error, error_size, nullptr, "Database name is missing" );
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
                string_printf ( p_error, error_size, nullptr, "%s.type is not 'database'('%s')", m_name.c_str(), typeStr );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
        }
        else
        {
            string_printf ( p_error, error_size, nullptr, "%s.type is invalid", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
    }
    else
    {
        string_printf ( p_error, error_size, nullptr, "%s.type is missing", m_name.c_str() );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    // nested DBs
    const KJsonValue * dbs = KJsonObjectGetMember ( m_json, "databases" );
    if ( dbs != nullptr )
    {
        const KJsonArray * dbarr = KJsonValueToArray ( dbs );
        if ( dbarr == nullptr )
        {
            string_printf ( p_error, error_size, nullptr, "%s.databases is not an array", m_name.c_str() );
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
                Database subdb ( obj, m_mgr, this ); // temporary, for Json verification
                rc = subdb . inflate ( p_error, error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                if ( m_subdbs.find( subdb . getName() ) != m_subdbs . end() )
                {
                    string_printf ( p_error, error_size, nullptr, "Duplicate nested db: %s", subdb.getName().c_str() );
                    return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
                }
                m_subdbs [ subdb . getName() ] = obj;
            }
            else
            {   // not an object
                string_printf ( p_error, error_size, nullptr, "%s.databases[%i] is not an object", m_name.c_str(), i );
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
            string_printf ( p_error, error_size, nullptr, "%s.tables is not an array", m_name.c_str() );
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
                rc = tbl . inflate ( p_error, error_size );
                if ( rc != 0 )
                {
                    return rc;
                }

                if ( m_tables.find( tbl . getName() ) != m_tables . end() )
                {
                    string_printf ( p_error, error_size, nullptr, "Duplicate table: %s", tbl.getName().c_str() );
                    return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
                }
                m_tables [ tbl . getName() ] = obj;
            }
            else
            {   // not an object
                string_printf ( p_error, error_size, nullptr, "%s.tables[%i] is not an object", m_name.c_str(), i );
                return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
            }
        }
    }

    const KJsonValue * meta = KJsonObjectGetMember ( m_json, "metadata" );
    if ( meta != nullptr )
    {
        const KJsonObject * obj = KJsonValueToObject ( meta );
        if( obj != nullptr )
        {
            Metadata * m  = new Metadata( obj );
            rc = m -> inflate( p_error, error_size );
            if ( rc != 0 )
            {
                delete m;
                return rc;
            }
            m_meta = m;
        }
        else
        {   // not an object
            string_printf ( p_error, error_size, nullptr, "%s.metadata is not an object", m_name.c_str() );
            return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
        }
    }

    return rc;
}

int
Database::pathType( Path & path ) const
{   //TODO: use only Json?
    int ret = kptNotFound;
    if ( path.size() == 1 && m_name == path.front() )
    {
        ret = kptDatabase;
    }
    else if ( ! path.empty() )
    {
        if ( m_name == path.front() )
        {
            path.pop();
            if ( path.size() >= 1 )
            {
                if ( path.front() == "db" )
                {
                    path.pop();
                    const Database * db = openSubDatabase( path.front() );
                    if ( db != nullptr )
                    {
                        ret = db->pathType( path );
                        delete db;
                    }
                }
                else if ( path.front() == "tbl" )
                {
                    const Table * tbl = openTable( path );
                    if ( tbl != nullptr )
                    {
                        ret = tbl -> pathType( path );
                        delete tbl;
                    }
                }
                else if ( path.front() == "md" )
                {
                    if ( m_meta != nullptr )
                    {
                        return kptMetadata;
                    }
                }
            }
        }
    }

    return ret;
}

bool
Database::exists( uint32_t requested, Path & path ) const
{   //TODO: use only Json?
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
            const Database * db = openSubDatabase( path.front() );
            if ( db != nullptr )
            {
                bool ret = db->exists( requested, path );
                delete db;
                return ret;
            }
        }
        else if ( string("tbl") == path.front() )
        {
            const Table * tbl = openTable( path );
            if ( tbl != nullptr )
            {
                bool ret = tbl -> exists( requested, path );
                delete tbl;
                return ret;
            }
        }
        else if ( string("md") == path.front() )
        {
            return m_meta != nullptr;
        }
    }

    return false;
}

const Metadata *
Database::openMetadata() const
{
    return m_meta;
}


// API functions

#define CAST() assert( bself -> dad . vt == & KTextDatabase_vt ); const Database *self = static_cast<const Database *>(bself);

static
rc_t CC
KTextDatabaseWhack ( KTextDatabase *bself )
{
    CAST();
    delete self;
    return 0;
}

static
bool CC
KTextDatabaseLocked ( const KTextDatabase *self )
{
    return false;
}

static
bool CC
KTextDatabaseVExists ( const KTextDatabase *bself, uint32_t type, const char *name, va_list args )
{
    CAST();
    Path p( name, args );
    return self -> exists( type, p );
}

static
bool CC
KTextDatabaseIsAlias ( const KTextDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    return false;
}

static
rc_t CC
KTextDatabaseVWritable ( const KTextDatabase *self, uint32_t type, const char *name, va_list args )
{
    return false;
}

static
rc_t CC
KTextDatabaseOpenManagerRead ( const KTextDatabase *bself, const KDBManager **mgr )
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
KTextDatabaseOpenParentRead ( const KTextDatabase *bself, const KDatabase **par )
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
rc_t CC
KTextDatabaseOpenDirectoryRead ( const KTextDatabase *self, const KDirectory **dir )
{
    return SILENT_RC ( rcDB, rcDatabase, rcAccessing, rcDirectory, rcUnsupported );
}

static
rc_t CC
KTextDatabaseVOpenDBRead ( const KTextDatabase *bself, const KDatabase **dbp, const char *name, va_list args )
{
    CAST();

    Path p( name, args );
    if ( p.size() > 0 )
    {
        const Database * db = self -> openSubDatabase( p.front() );
        if ( p.size() > 1 )
        {
            p.pop();
            const Database * topdb = db;
            db = self -> openDatabase( p );
            Database::release( topdb );
        }

        if ( db != nullptr )
        {
            * dbp = (const KDatabase *)db;
            return 0;
        }
    }

    return SILENT_RC( rcDB, rcDatabase, rcOpening, rcParam, rcInvalid );
}

static
rc_t CC
KTextDatabaseVOpenTableRead ( const KTextDatabase *bself, const KTable **tblp, const char * fmt, va_list args )
{
    CAST();

    string name;
    rc_t rc = Path::PrintToString( fmt, args, name );
    if ( rc == 0 )
    {
        const Table * t = self -> openTable( name );
        if ( t != nullptr )
        {
            *tblp = (const KTable *) t;
        }
        else
        {
            rc =  SILENT_RC( rcDB, rcDatabase, rcOpening, rcTable, rcNotFound );
        }
    }

    return rc;
}

static
rc_t CC
KTextDatabaseOpenMetadataRead ( const KTextDatabase *bself, const KMetadata **metap )
{
    CAST();
    const Metadata * m = self->openMetadata();
    if ( m != nullptr )
    {
        Metadata::addRef( m );
        *metap = (const KMetadata*)m;
        return 0;
    }
    return SILENT_RC( rcDB, rcMetadata, rcOpening, rcParam, rcInvalid );;
}

static
rc_t CC
KTextDatabaseVOpenIndexRead ( const KTextDatabase *bself, const KIndex **idxp, const char *name, va_list args )
{   // no database-level indices here
    return SILENT_RC( rcDB, rcDatabase, rcAccessing, rcIndex, rcUnsupported );
}

static
rc_t CC
KTextDatabaseListDB ( const KTextDatabase *bself, KNamelist **names )
{
    CAST();

    VNamelist * ret;
    const Database::Subobjects & dbs = self -> getDatabases();
    rc_t rc = VNamelistMake( & ret, dbs.size() );
    if (rc == 0 )
    {
        for (auto & key_val : dbs )
        {
            VNamelistAppend ( ret, key_val . first . c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return rc;
}

static
rc_t CC
KTextDatabaseListTbl ( struct KTextDatabase const *bself, KNamelist **names )
{
    CAST();

    VNamelist * ret;
    const Database::Subobjects & dbs = self -> getTables();
    rc_t rc = VNamelistMake( & ret, dbs.size() );
    if (rc == 0 )
    {
        for (auto & key_val : dbs )
        {
            VNamelistAppend ( ret, key_val . first . c_str() );
        }
        *names = (KNamelist*) ret;
    }

    return rc;
}

static
rc_t CC
KTextDatabaseListIdx ( struct KTextDatabase const *self, KNamelist **names )
{   // an empty list
    VNamelist * ret;
    rc_t rc = VNamelistMake( & ret, 0 );
    if (rc == 0 )
    {
        *names = (KNamelist*) ret;
    }
    return rc;
}

static
rc_t CC
KTextDatabaseGetPath ( KTextDatabase const *self, const char **path )
{
    return SILENT_RC( rcDB, rcDatabase, rcAccessing, rcPath, rcUnsupported );
}
