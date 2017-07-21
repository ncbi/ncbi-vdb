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

#include <ktst/unit_test.hpp> // THROW_ON_RC

class WVDB_Fixture
{
public:
    WVDB_Fixture()
    : m_db ( 0 )
    {
    }
    ~WVDB_Fixture()
    {
        if ( m_db )
        {
            VDatabaseRelease ( m_db );
        }
        RemoveDatabase();
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

        VDBManager* mgr;
        THROW_ON_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VSchema* schema;
        THROW_ON_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        THROW_ON_RC ( VSchemaParseText(schema, NULL, p_schemaText . c_str(), p_schemaText . size () ) );

        THROW_ON_RC ( VDBManagerCreateDB ( mgr,
                                          & m_db,
                                          schema,
                                          p_schemaSpec . c_str (),
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );
        THROW_ON_RC ( VSchemaRelease ( schema ) );
        THROW_ON_RC ( VDBManagerRelease ( mgr ) );
    }

    std :: string m_databaseName;
    VDatabase* m_db;
};

#endif