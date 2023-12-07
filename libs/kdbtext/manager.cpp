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
{
    dad . vt = & vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Manager", "ctor", "kmgr" );
}

Manager::~Manager()
{
    delete m_tbl;
    delete m_db;
    KJsonValueWhack( m_root );
    KRefcountWhack ( & dad . refcount, "KDBText::Manager" );
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
        {
            if ( strcmp( typeStr, "database") == 0 )
            {
                m_db = new Database( rootObj );
                rc = m_db -> inflate( error, error_size );
            }
            else if ( strcmp( typeStr, "table") == 0 )
            {
                m_tbl = new Table( rootObj );
                rc = m_tbl -> inflate( error, error_size );
            }
        }
    }

    return rc;
}

int
Manager::pathType( const string & p_path ) const
{
//TODO: allow db/table/col etc.
    Path path( p_path );
    if ( m_db != nullptr )
    {
        return m_db -> pathType( path );
    }
    if ( m_tbl != nullptr )
    {
        return m_tbl -> pathType( path );
    }
    return kptNotFound;
}

bool
Manager::exists( uint32_t requested, const Path & p_path ) const
{
    // TODO: non-root dbs and other objects (incl root tables)
    Path path = p_path;
    switch ( requested )
    {
    case kptDatabase:
        {
            const Database * db = m_db;
            do
            {
                if ( db && ! path.empty() && db -> getName() == path.front() )
                {
                    path.pop();
                    if ( path.empty() )
                    {
                        return true;
                    }

                    if ( string("db") == path.front() )
                    {
                        path.pop();
                        if (  ! path.empty() )
                        {
                            db = db -> getDatabase( path.front() );
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            while ( path . size () > 0 );
            return true;
        }

    case kptTable:
        return m_tbl != nullptr && path.size() == 1 && m_tbl -> getName() == path.front();

    default:
        return false;
    }
    // int type;

    // type = KDBManagerVPathType (self, name, args);
    // switch ( type )
    // {
    // case kptDatabase:
    // case kptTable:
    // case kptIndex:
    // case kptColumn:
    // case kptMetadata:
    //     break;
    // case kptPrereleaseTbl:
    // default:
    //     return false;
    // }
    // return requested == ( uint32_t ) type;
}

rc_t
Manager::writable( const Path & p_path ) const
{
    if ( ! p_path.empty() )
    {
        //TODO: parse and resolve the path, if the object is valid return rcReadOnly, otherwise rcNotFound
        if ( m_db && m_db -> getName() == p_path.front() )
        {
            return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
        }
        if ( m_tbl && m_tbl -> getName() == p_path.front() )
        {
            return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
        }
    }

    return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcNotFound );
}

rc_t
Manager::openDatabase( const Path & p_path, const Database *& p_db ) const
{
    if ( ! p_path.empty() )
    {
        // TODO: non-root dbs
        if ( m_db && m_db -> getName() == p_path.front() )
        {
            rc_t rc = KDatabaseAddRef( (const KDatabase *)m_db );
            if ( rc != 0 )
            {
                return rc;
            }
            p_db = m_db;
            return 0;
        }
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

rc_t
Manager::openTable( const Path & p_path, const Table *& p_tbl ) const
{
    if ( ! p_path.empty() )
    {
        // TODO: non-root tables
        if ( m_tbl && m_tbl -> getName() == p_path.front() )
        {
            rc_t rc = KTableAddRef( (const KTable *)m_tbl );
            if ( rc != 0 )
            {
                return rc;
            }
            p_tbl = m_tbl;
            return 0;
        }
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

rc_t
Manager::openColumn( const Path & p_path, const Column *& p_col) const
{
    if ( ! p_path.empty() )
    {
        // TODO: non-root tables
        if ( ! p_path.empty() && m_tbl && m_tbl -> getName() == p_path.front() )
        {
            Path path = p_path;
            path.pop();
            if ( ! path.empty() && path.front() == "col" )
            {
                path.pop();
                if ( ! path.empty() )
                {
                    const Column* col = m_tbl -> getColumn( path.front() );
                    if ( col != nullptr )
                    {
                        rc_t rc = KColumnAddRef( (const KColumn *)col );
                        if ( rc != 0 )
                        {
                            return rc;
                        }
                        p_col = col;
                        return 0;
                    }
                }
            }
        }
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}
