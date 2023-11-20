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

using namespace KDBText;


static rc_t KTextTableWhack ( KTable *self );
// static bool CC KTextTableLocked ( const KTable *self );
// static bool CC KTextTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args );
// static bool CC KTextTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name );
// static rc_t CC KTextTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args );
// static rc_t CC KTextTableOpenManagerRead ( const KTable *self, const KDBManager **mgr );
// static rc_t CC KTextTableOpenParentRead ( const KTable *self, const KDatabase **db );
// static bool CC KTextTableHasRemoteData ( const KTable *self );
// static rc_t CC KTextTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir );
// static rc_t CC KTextTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args );
// static rc_t CC KTextTableOpenMetadataRead ( const KTable *self, const KMetadata **metap );
// static rc_t CC KTextTableVOpenIndexRead ( const KTable *self, const KIndex **idxp, const char *name, va_list args );
// static rc_t CC KTextTableGetPath ( const KTable *self, const char **path );
// static rc_t CC KTextTableGetName (KTable const *self, char const **rslt);
// static rc_t CC KTextTableListCol ( const KTable *self, KNamelist **names );
// static rc_t CC KTextTableListIdx ( const KTable *self, KNamelist **names );
// static rc_t CC KTextTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal );

static KTable_vt KTextTable_vt =
{
    KTextTableWhack,
    KTableBaseAddRef,
    KTableBaseRelease,
    // KTextTableLocked,
    // KTextTableVExists,
    // KTextTableIsAlias,
    // KTextTableVWritable,
    // KTextTableOpenManagerRead,
    // KTextTableOpenParentRead,
    // KTextTableHasRemoteData,
    // KTextTableOpenDirectoryRead,
    // KTextTableVOpenColumnRead,
    // KTextTableOpenMetadataRead,
    // KTextTableVOpenIndexRead,
    // KTextTableGetPath,
    // KTextTableGetName,
    // KTextTableListCol,
    // KTextTableListIdx,
    // KTextTableMetaCompare
};

#define CAST() assert( bself->vt == &KTextTable_vt ); Table * self = (Table *)bself

Table::Table( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextTable_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Table", "ctor", "db" );
}

Table::~Table()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Table" );
}

rc_t
Table::inflate( char * error, size_t error_size )
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
KTextTableWhack ( KTable *bself )
{
    CAST();

    delete reinterpret_cast<Table*>( self );
    return 0;
}