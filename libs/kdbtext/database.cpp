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

// static rc_t CC KRDatabaseWhack ( KDatabase *self );
// static bool CC KRDatabaseLocked ( const KDatabase *self );
// static bool CC KRDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args );
// static bool CC KRDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name );
// static rc_t CC KRDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args );
// static rc_t CC KRDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr );
// static rc_t CC KRDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par );
// static rc_t CC KRDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir );
// static rc_t CC KRDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args );
// static rc_t CC KRDatabaseVOpenTableRead ( const KDatabase *self, const KTable **tblp, const char *name, va_list args );
// static rc_t CC KRDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap );
// static rc_t CC KRDatabaseVOpenIndexRead ( const KDatabase *self, const KIndex **idxp, const char *name, va_list args );
// static rc_t CC KRDatabaseListDB ( const KDatabase *self, KNamelist **names );
// static rc_t CC KRDatabaseListTbl ( struct KDatabase const *self, KNamelist **names );
// static rc_t CC KRDatabaseListIdx ( struct KDatabase const *self, KNamelist **names );
// static rc_t CC KRDatabaseGetPath ( KDatabase const *self, const char **path );

static KDatabase_vt KRTextDatabase_vt =
{
    // KRDatabaseWhack,
    // KDatabaseBaseAddRef,
    // KDatabaseBaseRelease,
    // KRDatabaseLocked,
    // KRDatabaseVExists,
    // KRDatabaseIsAlias,
    // KRDatabaseVWritable,
    // KRDatabaseOpenManagerRead,
    // KRDatabaseOpenParentRead,
    // KRDatabaseOpenDirectoryRead,
    // KRDatabaseVOpenDBRead,
    // KRDatabaseVOpenTableRead,
    // KRDatabaseOpenMetadataRead,
    // KRDatabaseVOpenIndexRead,
    // KRDatabaseListDB,
    // KRDatabaseListTbl,
    // KRDatabaseListIdx,
    // KRDatabaseGetPath
};

Database::Database( const KJsonObject * p_json ) : m_json ( p_json )
{
}

Database::~Database()
{
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

