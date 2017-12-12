// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#ifndef _hpp_wvdb_fixture_
#define _hpp_wvdb_fixture_

#include <string>

#include <vdb/database.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/table.h>

#include <ktst/unit_test.hpp> // THROW_ON_RC

class WVDB_Fixture
{
public:
    WVDB_Fixture()
    :   m_mgr ( 0 ),
        m_schema ( 0 ),
        m_db ( 0 ),
        m_keepDb ( false )
    {
    }
    ~WVDB_Fixture()
    {
        if ( m_db != 0 )
        {
            VDatabaseRelease ( m_db );
        }
        if ( m_schema != 0 )
        {
            VSchemaRelease ( m_schema );
        }
        if ( m_mgr != 0 )
        {
            VDBManagerRelease ( m_mgr );
        }

        if ( ! m_keepDb )
        {
            RemoveDatabase();
        }
    }

    void RemoveDatabase ()
    {
        if ( ! m_databaseName . empty () )
        {
            KDirectory* wd;
            KDirectoryNativeDir ( & wd );
            KDirectoryRemove ( wd, true, m_databaseName . c_str () );
            KDirectoryRelease ( wd );
        }
    }

    void MakeDatabase ( const std :: string & p_schemaText, const std :: string & p_schemaSpec )
    {
        RemoveDatabase();

        THROW_ON_RC ( VDBManagerMakeUpdate ( & m_mgr, NULL ) );
        THROW_ON_RC ( VDBManagerMakeSchema ( m_mgr, & m_schema ) );
        ParseSchema ( m_schema, p_schemaText );
        THROW_ON_RC ( VDBManagerCreateDB ( m_mgr,
                                          & m_db,
                                          m_schema,
                                          p_schemaSpec . c_str (),
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );
    }

    virtual void ParseSchema ( VSchema * p_schema, const std :: string & p_schemaText )
    {
        THROW_ON_RC ( VSchemaParseText ( p_schema, NULL, p_schemaText . c_str(), p_schemaText . size () ) );
    }

    VCursor * CreateTable ( const char * p_tableName ) // returns write cursor
    {
        VTable* table;
        THROW_ON_RC ( VDatabaseCreateTable ( m_db, & table, p_tableName, kcmCreate | kcmMD5, "%s", p_tableName ) );
        VCursor * ret;
        THROW_ON_RC ( VTableCreateCursorWrite ( table, & ret, kcmInsert ) );
        THROW_ON_RC ( VTableRelease ( table ) );
        return ret;
    }

    const VCursor * OpenTable ( const char * p_tableName ) // returns read cursor
    {
        const VTable* table;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & table, "%s", p_tableName ) );
        const VCursor * ret;
        THROW_ON_RC ( VTableCreateCursorRead ( table, & ret ) );
        THROW_ON_RC ( VTableRelease ( table ) );
        return ret;
    }

    void WriteRow ( VCursor * p_cursor, uint32_t p_colIdx, const std :: string & p_value )
    {
        THROW_ON_RC ( VCursorOpenRow ( p_cursor ) );
        THROW_ON_RC ( VCursorWrite ( p_cursor, p_colIdx, 8, p_value . c_str (), 0, p_value . length () ) );
        THROW_ON_RC ( VCursorCommitRow ( p_cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( p_cursor ) );
    }

    std :: string   m_databaseName;
    VDBManager *    m_mgr;
    VSchema *       m_schema;
    VDatabase *     m_db;
    bool            m_keepDb;
};

#endif