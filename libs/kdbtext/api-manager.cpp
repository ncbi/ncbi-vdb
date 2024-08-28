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

#include "api-manager.hpp"

#include "manager.hpp"
#include "database.hpp"
#include "table.hpp"
#include "path.hpp"

#include <kdb/database.h>
#include <kdb/table.h>
#include <kdb/manager.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/data-buffer.h>

#include <vfs/path.h>
#include <vfs/manager.h>

#include <string>

using namespace std;

const uint32_t LIBKDBTEXT_VERS = 0;

static rc_t CC KTextManagerWhack ( KDBManager *self );
static rc_t CC KTextManagerVersion ( const KDBManager *self, uint32_t *version );
static bool CC KTextManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args );
static rc_t CC KTextManagerVWritable ( const KDBManager *self, const char * path, va_list args );
static rc_t CC KTextManagerRunPeriodicTasks ( const KDBManager *self );
static int CC KTextManagerPathTypeVP( const KDBManager * self, const VPath * path );
static int CC KTextManagerVPathType ( const KDBManager * self, const char *path, va_list args );
static int CC KTextManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args );
static rc_t CC KTextManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, va_list args, const VPath * );
static rc_t CC KTextManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args );
static rc_t CC KTextManagerOpenTableReadVPath(const KDBManager *self, const KTable **tbl, const struct VPath *path);
static rc_t CC KTextManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args );
static rc_t CC KTextManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
static rc_t CC KTextManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );
static rc_t CC KTextManagerGetVFSManager ( const KDBManager *self, const struct VFSManager **vmanager );

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
    KTextManagerVPathOpenRemoteDBRead,
    KTextManagerGetVFSManager
};

using namespace KDBText;

#define CAST() assert( bself -> dad . vt == & KTextManager_vt ); const Manager *self = static_cast<const Manager *>(bself);

static
rc_t CC
KTextManagerWhack ( KDBManager *bself )
{
    CAST();
    assert( self -> dad . vt == & KTextManager_vt );
    delete self;
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
    return self -> exists( requested, Path( fmt, args ) );
}

static
rc_t CC
KTextManagerVWritable ( const KDBManager *bself, const char * fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );

    return self -> writable( Path(path) );
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
KTextManagerVOpenDBRead ( const KDBManager *bself, const KDatabase **p_db, const char *fmt, va_list args, const VPath *vpath )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );
    const Database * db = nullptr;
    rc_t rc = self -> openDatabase( Path( path ), db );
    if ( rc == 0 )
    {
        *p_db = (const KDatabase *)db;
    }

    return rc;
}

static
rc_t CC
KTextManagerVOpenTableRead ( const KDBManager *bself, const KTable **p_tbl, const char *fmt, va_list args )
{
    CAST();

    string path;
    PrintToString( fmt, args, path );
    const Table * tbl = nullptr;
    rc_t rc = self -> openTable( Path( path ), tbl );
    if ( rc == 0 )
    {
        *p_tbl = (const KTable *)tbl;
    }

    return rc;
}

static
rc_t CC
KTextManagerOpenTableReadVPath(const KDBManager *bself, const KTable **p_tbl, const struct VPath *p_path)
{
    CAST();

    const Table * tbl = nullptr;
    rc_t rc = self -> openTable( Path( p_path ), tbl );
    if ( rc == 0 )
    {
        *p_tbl = (const KTable *)tbl;
    }

    return rc;
}

static
rc_t CC
KTextManagerVOpenColumnRead ( const KDBManager *bself, const KColumn **p_col, const char *fmt, va_list args )
{   // not supported here
    return SILENT_RC ( rcDB, rcMgr, rcAccessing, rcColumn, rcUnsupported );
}

static
rc_t CC
KTextManagerVPathOpenLocalDBRead ( const KDBManager * bself, struct KDatabase const ** p_db, struct VPath const * vpath )
{
    CAST();

    const Database * db = nullptr;
    rc_t rc = self -> openDatabase( Path( vpath ), db );
    if ( rc == 0 )
    {
        *p_db = (const KDatabase *)db;
    }

    return rc;
}

static
rc_t CC
KTextManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
{
    return SILENT_RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
}

rc_t CC
KDBManagerMakeText ( const KDBManager ** p_mgr, const char * input, char * error, size_t error_size )
{
    if ( p_mgr == nullptr )
    {
        return SILENT_RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );
    }

    Manager * mgr = new Manager( KTextManager_vt );

    rc_t rc = mgr -> parse( input, error, error_size );
    if ( rc == 0 )
    {
        // if (vmanager == NULL)
        // {
        //     rc = VFSManagerMake ( & mgr -> vfsmgr );
        // }

        *p_mgr = mgr;
    }
    else
    {
        delete mgr;
    }

    return rc;
}

static
rc_t CC
KTextManagerGetVFSManager ( const KDBManager *self, const struct VFSManager **vmanager )
{
    if (vmanager == nullptr)
    {
        return SILENT_RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    }
    else
    {
        VFSManager * ret;
        rc_t rc = VFSManagerMake( & ret );
        if (rc == 0)
        {
            * vmanager = ret;
        }
        else
        {
            * vmanager = nullptr;
        }
        return rc;
    }
}
