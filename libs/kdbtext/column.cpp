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

#include "column.hpp"

#include <klib/printf.h>

using namespace KDBText;

static rc_t KTextColumnWhack ( KColumn *self );

static KColumn_vt KTextColumn_vt =
{
    KTextColumnWhack,
    KColumnBaseAddRef,
    KColumnBaseRelease,
};

#define CAST() assert( bself->vt == &KTextColumn_vt ); Column * self = (Column *)bself

Column::Column( const KJsonObject * p_json ) : m_json ( p_json )
{
    dad . vt = & KTextColumn_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::Column", "ctor", "db" );
}

Column::~Column()
{
    KRefcountWhack ( & dad . refcount, "KDBText::Column" );
}

rc_t
Column::inflate( char * error, size_t error_size )
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
        string_printf ( error, error_size, nullptr, "Column name is missing" );
        return SILENT_RC( rcDB, rcDatabase, rcCreating, rcParam, rcInvalid );
    }

    //TBD

    return rc;
}

static
rc_t CC
KTextColumnWhack ( KColumn *bself )
{
    CAST();

    delete reinterpret_cast<Column*>( self );
    return 0;
}