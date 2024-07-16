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

#include "manager.hpp"

#include <kdb/manager.h>

#include <kdb/database.h>
#include <kdb/table.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/data-buffer.h>

#include <vfs/path.h>

using namespace std;
using namespace KDBText;

Manager::Manager( const KDBManager_vt & vt)
: m_isDb( false )
{
    dad . vt = & vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Manager", "ctor", "kmgr" );
}

Manager::~Manager()
{
    KJsonValueWhack( m_root );
    KRefcountWhack ( & dad . refcount, "KDBText::Manager" );
}

void
Manager::addRef( const Manager * mgr )
{
    if ( mgr != nullptr )
    {
        KDBManagerAddRef( (const KDBManager*) mgr );
    }
}

void
Manager::release( const Manager * mgr )
{
    if ( mgr != nullptr )
    {
        KDBManagerRelease( (const KDBManager*) mgr );
    }
}

rc_t
Manager::parse( const char * input, char * error, size_t error_size )
{
    rc_t rc = KJsonValueMake ( & m_root, input, error, error_size );
    if ( rc != 0 )
    {
        return rc;
    }

    if ( KJsonGetValueType ( m_root ) != jsObject )
    {
        string_printf ( error, error_size, nullptr, "Json root is invalid" );
        return SILENT_RC( rcDB, rcMgr, rcCreating, rcParam, rcInvalid );
    }

    const KJsonObject * rootObj = KJsonValueToObject ( m_root );
    assert( rootObj != nullptr );

    const KJsonValue * type = KJsonObjectGetMember ( rootObj, "type" );
    if ( type != nullptr )
    {
        const char * typeStr = nullptr;
        rc = KJsonGetString ( type, & typeStr );
        if ( rc == 0 )
        {   // determine the type of the root object (which is then discarded)
            if ( strcmp( typeStr, "database") == 0 )
            {
                rc = Database( rootObj, this ) . inflate( error, error_size );
                if ( rc == 0 )
                {
                    m_isDb = true;
                }
            }
            else if ( strcmp( typeStr, "table") == 0 )
            {
                rc = Table( rootObj ) . inflate( error, error_size );
                if ( rc == 0 )
                {
                    m_isDb = false;
                }
            }
        }
    }

    return rc;
}

const Database *
Manager::getRootDatabase() const
{
    Database * db = new Database( KJsonValueToObject ( m_root ), this );
    char error[1024];
    if ( db -> inflate( error, sizeof error ) == 0 )
    {
        return db;
    }
    delete db;
    return nullptr;
}

const Table *
Manager::getRootTable() const
{
    Table * tbl = new Table( KJsonValueToObject ( m_root ), this, nullptr );
    char error[1024];
    if ( tbl -> inflate( error, sizeof error ) == 0 )
    {
        return tbl;
    }
    delete tbl;
    return nullptr;
}


int
Manager::pathType( const Path & p_path ) const
{
    Path path( p_path );
    int ret = kptNotFound;

    //TODO: do it without creating a DB/Tbl, by walking the Json

    if ( m_isDb )
    {
        const Database * db = getRootDatabase();
        if ( db != nullptr )
        {
            ret = db -> pathType( path );
        }
        delete db;
    }
    else
    {
        const Table * tbl = getRootTable();
        if ( tbl != nullptr )
        {
            ret = tbl -> pathType( path );
        }
        delete tbl;
    }
    return ret;
}

bool
Manager::exists( uint32_t requested, const Path & p_path ) const
{
    Path path( p_path );
    bool ret = false;

    //TODO: do it without creating a DB/Tbl, by walking the Json
    if ( m_isDb )
    {
        const Database * db = getRootDatabase();
        if ( db != nullptr )
        {
            ret = db -> exists( requested, path );
        }
        delete db;
    }
    else
    {
        const Table * tbl = getRootTable();
        if ( tbl )
        {
            ret = tbl -> exists( requested, path );
        }
        delete tbl;
    }
    return ret;
}

rc_t
Manager::writable( const Path & p_path ) const
{
    //if the object is valid return rcReadOnly, otherwise rcNotFound
    return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, pathType( p_path ) != kptNotFound ? rcReadonly : rcNotFound );
}

rc_t
Manager::openDatabase( const Path & p_path, const Database *& p_db ) const
{
    if ( ! p_path.empty() )
    {
        if ( m_isDb )
        {
            Path path ( p_path );
            const Database * root = getRootDatabase();
            if ( root != nullptr )
            {
                const Database * db = root -> openDatabase( path ); // could be same as root
                if ( db != nullptr )
                {
                    if ( db != root )
                    {
                        KDatabaseRelease( (const KDatabase *)root );
                        p_db = db;
                        return 0;
                    }
                    else
                    {
                        p_db = root;
                        return 0;
                    }
                }
                else
                {
                    delete root;
                    return SILENT_RC (rcDB, rcMgr, rcOpening, rcDirectory, rcNotFound );
                }
            }
        }
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

rc_t
Manager::openTable( const Path & p_path, const Table *& p_tbl ) const
{
    if ( ! p_path.empty() )
    {
        if ( m_isDb )
        {
            Path path ( p_path );
            const Database * root = getRootDatabase();
            if ( root != nullptr )
            {
                const Table * tbl = root -> openTable( path );
                KDatabaseRelease( (const KDatabase *)root );
                if ( tbl != nullptr )
                {
                    p_tbl = tbl;
                    return 0;
                }
            }
        }
        else
        {
            Path path ( p_path );
            const Table * tbl = getRootTable();
            if ( tbl != nullptr )
            {
                if ( tbl -> getName() == p_path.front() )
                {
                    p_tbl = tbl;
                    return 0;
                }
            }
            KTableRelease( (const KTable*)tbl );
        }
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}
