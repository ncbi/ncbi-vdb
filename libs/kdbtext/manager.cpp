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

#include <kdb/manager.h>

#include "database.hpp"
#include "table.hpp"
#include "path.hpp"

#include <kdb/database.h>
#include <kdb/table.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/data-buffer.h>

#include <vfs/path.h>

#include <string>

using namespace std;

typedef struct KDBManager KDBManager;
#define KDBMGR_IMPL KDBManager
#include "../libs/kdb/manager-base.h"

struct KDBManager
{
    KDBManagerBase dad;
};

const uint32_t LIBKDBTEXT_VERS = 0;

static rc_t CC KTextManagerWhack ( KDBManager *self );
static rc_t CC KTextManagerVersion ( const KDBManager *self, uint32_t *version );
static bool CC KTextManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args );
static rc_t CC KTextManagerVWritable ( const KDBManager *self, const char * path, va_list args );
static rc_t CC KTextManagerRunPeriodicTasks ( const KDBManager *self );
static int CC KTextManagerPathTypeVP( const KDBManager * self, const VPath * path );
static int CC KTextManagerVPathType ( const KDBManager * self, const char *path, va_list args );
static int CC KTextManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args );
static rc_t CC KTextManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, va_list args );
static rc_t CC KTextManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args );
static rc_t CC KTextManagerOpenTableReadVPath(const KDBManager *self, const KTable **tbl, const struct VPath *path);
static rc_t CC KTextManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args );
static rc_t CC KTextManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
static rc_t CC KTextManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );

static KDBManager_vt KTextManager_vt =
{
    KTextManagerWhack,
    KDBManagerBaseAddRef,
    KDBManagerBaseRelease,
    KTextManagerVersion,
    KTextManagerVExists,
    KTextManagerVWritable,
    KTextManagerRunPeriodicTasks,
    KTextManagerPathTypeVP,
    KTextManagerVPathType,
    KTextManagerVPathTypeUnreliable,
    KTextManagerVOpenDBRead,
    KTextManagerVOpenTableRead,
    KTextManagerOpenTableReadVPath,
    KTextManagerVOpenColumnRead,
    KTextManagerVPathOpenLocalDBRead,
    KTextManagerVPathOpenRemoteDBRead
};

namespace KDBText
{
    class Manager : public KDBManager
    {
    public:
        Manager()
        {
            dad . vt = & KTextManager_vt;
            KRefcountInit ( & dad . refcount, 1, "KDBText::Manager", "ctor", "kmgr" );
        }
        ~Manager()
        {
            delete m_tbl;
            delete m_db;
            KJsonValueWhack( m_root );
            KRefcountWhack ( & dad . refcount, "KDBText::Manager" );
        }

        rc_t parse( const char * input, char * error, size_t error_size )
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

        const Database * getRootDatabase() const { return m_db; }
        const Table * getRootTable() const { return m_tbl; }

        int pathType( const string & path ) const
        {
            if ( m_db )
            {   //TODO: implement proper parsing (allow "db/table/col" etc.)
                if ( m_db -> getName () == path )
                {
                    return kptDatabase;
                }
            }
            //TODO: else root table
            return kptNotFound;
        }

    private:
        KJsonValue * m_root = nullptr;
        Database * m_db = nullptr;
        Table * m_tbl = nullptr;
    };

}

using namespace KDBText;
#define CAST() assert( bself -> dad . vt == & KTextManager_vt ); const Manager *self = static_cast<const Manager *>(bself);

static
rc_t CC
KTextManagerWhack ( KDBManager *self )
{
    assert( self -> dad . vt == & KTextManager_vt );
    delete reinterpret_cast<Manager*>( self );
    return 0;
}

static
rc_t CC
KTextManagerVersion ( const KDBManager *self, uint32_t * version )
{
    if ( version == nullptr )
    {
        return SILENT_RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    }

    * version = LIBKDBTEXT_VERS;
    return 0;
}

static
rc_t
PrintToString( const char *fmt, va_list args, string & out )
{
    KDataBuffer buf;
    rc_t rc = KDataBufferMake ( & buf, 8, 0 );
    if ( rc != 0 )
    {
        return false;
    }

    rc = KDataBufferVPrintf ( & buf, fmt, args );
    if ( rc != 0 )
    {
        return rc;
    }

    out = string((const char *) (buf . base)); // will be 0-terminated

    rc = KDataBufferWhack ( & buf );
    if ( rc != 0 )
    {
        return rc;
    }

    return rc;
}



static
bool CC
KTextManagerVExists ( const KDBManager *bself, uint32_t requested, const char *fmt, va_list args )
{
    CAST();

    Path path( fmt, args );

    // TODO: non-root dbs and other objects (incl root tables)
    switch ( requested )
    {
    case kptDatabase:
        {
            const Database * db = self -> getRootDatabase();
            do
            {
                if ( db && path.size() > 0 && db -> getName() == path.front() )
                {
                    path.pop();
                    if ( path.empty() )
                    {
                        return true;
                    }

                    if ( string("db") == path.front() )
                    {
                        path.pop();
                        if (  path.size() > 0 )
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

static
rc_t CC
KTextManagerVWritable ( const KDBManager *bself, const char * fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );

    //TODO: parse and resolve the path, if the object is valid return rcReadOnly, otherwise rcNotFound

    const Database * db = self -> getRootDatabase();
    if (db && db -> getName() == path )
    {
        return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
    }
    const Table * tbl= self -> getRootTable();
    if (tbl && tbl -> getName() == path )
    {
        return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
    }

    return SILENT_RC ( rcDB, rcPath, rcAccessing, rcPath, rcNotFound );
}

static
rc_t CC
KTextManagerRunPeriodicTasks ( const KDBManager *self )
{
    return 0;
}

static
int CC
KTextManagerPathTypeVP( const KDBManager * bself, const VPath * path )
{
    CAST();

    // parse and resolve the path
    const String * p;
    rc_t rc = VPathMakeString ( path, &p );
    if ( rc == 0 )
    {
        int ret = self -> pathType( string ( p -> addr, p -> size ) );
        StringWhack ( p );
        return ret;
    }

    return kptNotFound;
}

static
int CC
KTextManagerVPathType ( const KDBManager * bself, const char *fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );

    return self -> pathType( path );
}

static
int CC
KTextManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args )
{
    return KTextManagerVPathType( self, path, args );
}

static
rc_t CC
KTextManagerVOpenDBRead ( const KDBManager *bself, const KDatabase **p_db, const char *fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );

    // TODO: non-root dbs
    const Database * db = self -> getRootDatabase();
    if (db && db -> getName() == path )
    {
        rc_t rc = KDatabaseAddRef( (const KDatabase *)db );
        if ( rc != 0 )
        {
            return rc;
        }
        *p_db = (const KDatabase *)db;
        return 0;
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

static
rc_t CC
KTextManagerVOpenTableRead ( const KDBManager *bself, const KTable **p_tbl, const char *fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );

    // TODO: non-root tables?
    const Table * tbl= self -> getRootTable();
    if (tbl && tbl -> getName() == path )
    {
        rc_t rc = KTableAddRef( (const KTable *)tbl );
        if ( rc != 0 )
        {
            return rc;
        }
        *p_tbl = (const KTable *)tbl;
        return 0;
    }
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

static
rc_t CC
KTextManagerOpenTableReadVPath(const KDBManager *bself, const KTable **p_tbl, const struct VPath *path)
{
    CAST();

    // parse and resolve the path
    const String * p;
    rc_t rc = VPathMakeString ( path, &p );
    if ( rc == 0 )
    {
        // TODO: non-root tables?
        const Table * tbl= self -> getRootTable();
        if (tbl && tbl -> getName() == string ( p -> addr, p -> size ) )
        {
            StringWhack ( p );
            rc_t rc = KTableAddRef( (const KTable *)tbl );
            if ( rc != 0 )
            {
                return rc;
            }
            *p_tbl = (const KTable *)tbl;
            return 0;
        }
        StringWhack ( p );
        return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
    }

    return rc;
}

static
rc_t CC
KTextManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args )
{
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

static
rc_t CC
KTextManagerVPathOpenLocalDBRead ( const struct KDBManager * bself, struct KDatabase const ** p_db, struct VPath const * path )
{
    CAST();

    // parse and resolve the path
    const String * p;
    rc_t rc = VPathMakeString ( path, &p );
    if ( rc == 0 )
    {
        // TODO: non-root dbs
        const Database * db = self -> getRootDatabase();
        if ( db && db -> getName() == string( p -> addr, p -> size ) )
        {
            rc_t rc = KDatabaseAddRef( (const KDatabase *)db );
            StringWhack ( p );
            if ( rc != 0 )
            {
                return rc;
            }
            *p_db = (const KDatabase *)db;
            return 0;
        }
        StringWhack ( p );
        return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
    }
    return rc;
}

static
rc_t CC
KTextManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
{
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

LIB_EXPORT
rc_t CC
KDBManagerMakeText ( const KDBManager ** p_mgr, const char * input, char * error, size_t error_size )
{
    if ( p_mgr == nullptr )
    {
        return SILENT_RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );
    }

    Manager * mgr = new Manager();

    rc_t rc = mgr -> parse( input, error, error_size );
    if ( rc == 0 )
    {
        *p_mgr = mgr;
    }
    else
    {
        delete mgr;
    }
    return rc;
}

