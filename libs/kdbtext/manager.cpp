
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

#include <klib/rc.h>

typedef struct KDBManager KDBManager;
#define KDBMGR_IMPL KDBManager
#include "../libs/kdb/manager-base.h"

struct KDBManager
{
    KDBManagerBase dad;
};

const uint32_t LIBKDBTEXT_VERS = 0;

static rc_t CC KDBTextManagerWhack ( KDBManager *self );
static rc_t CC KDBTextManagerVersion ( const KDBManager *self, uint32_t *version );
static bool CC KDBTextManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args );

// static rc_t CC KDBTextManagerVWritable ( const KDBManager *self, const char * path, va_list args );
// static rc_t CC KDBTextManagerRunPeriodicTasks ( const KDBManager *self );
// static int CC KDBTextManagerPathTypeVP( const KDBManager * self, const VPath * path );
// static int CC KDBTextManagerVPathType ( const KDBManager * self, const char *path, va_list args );
// static int CC KDBTextManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args );
// static rc_t CC KDBTextManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, va_list args );
// static rc_t CC KDBTextManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args );
// static rc_t CC KDBTextManagerOpenTableReadVPath(const KDBManager *self, const KTable **tbl, const struct VPath *path);
// static rc_t CC KDBTextManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args );
// static rc_t CC KDBTextManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
// static rc_t CC KDBTextManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );

static KDBManager_vt KDBTextManager_vt =
{
    KDBTextManagerWhack,
    KDBManagerBaseAddRef,
    KDBManagerBaseRelease,
    KDBTextManagerVersion,
    KDBTextManagerVExists,
    // KDBTextManagerVWritable,
    // KDBTextManagerRunPeriodicTasks,
    // KDBTextManagerPathTypeVP,
    // KDBTextManagerVPathType,
    // KDBTextManagerVPathTypeUnreliable,
    // KDBTextManagerVOpenDBRead,
    // KDBTextManagerVOpenTableRead,
    // KDBTextManagerOpenTableReadVPath,
    // KDBTextManagerVOpenColumnRead,
    // KDBTextManagerVPathOpenLocalDBRead,
    // KDBTextManagerVPathOpenRemoteDBRead
};

class Manager : public KDBManager
{
public:
    Manager()
    {
        dad . vt = & KDBTextManager_vt;
        KRefcountInit ( & dad . refcount, 1, "KDBManager", "make-read-text", "kmgr" );
    }
    ~Manager()
    {
        KRefcountWhack ( & dad . refcount, "KDBManager" );
    }
};

static
rc_t CC
KDBTextManagerWhack ( KDBManager *self )
{
    assert( self -> dad . vt == & KDBTextManager_vt );
    delete reinterpret_cast<Manager*>( self );
    return 0;
}

static
rc_t CC
KDBTextManagerVersion ( const KDBManager *self, uint32_t * version )
{
    if ( version == nullptr )
    {
        return SILENT_RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    }

    * version = LIBKDBTEXT_VERS;
    return 0;
}

static
bool CC
KDBTextManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args )
{
    return false;
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
    //     type = kptTable;
    //     break;
    // default:
    //     return false;
    // }
    // return requested == ( uint32_t ) type;
}

LIB_EXPORT
rc_t CC
KDBManagerMakeText ( const KDBManager ** p_mgr, const char * data )
{
    if ( p_mgr == nullptr )
    {
        return SILENT_RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );
    }

    *p_mgr = new Manager();
    return 0;
}

