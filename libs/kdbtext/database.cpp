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

using namespace KDBText;

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

    return rc;
}

static
rc_t CC
KTextDatabaseWhack ( KTextDatabase *self )
{
    assert( self -> dad . vt == & KTextDatabase_vt );
    delete reinterpret_cast<Database*>( self );
    return 0;
}