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

#pragma once

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <sra/sraschema.h> // VDBManagerMakeSRASchema
#include <vdb/schema.h> /* VSchemaRelease */

#include <map>
#include <string>
#include <stdexcept>

class VDB_Fixture
{
public:
    typedef std :: map< std :: string, const VDatabase * > Databases;

public:
    VDB_Fixture()
    : mgr(0), curs(0)
    {
        if ( VDBManagerMakeRead(&mgr, NULL) != 0 )
            throw std :: logic_error ( "VdbFixture: VDBManagerMakeRead failed" );
        if ( dbs == nullptr )
        {
            dbs = new Databases();
        }
    }

    ~VDB_Fixture()
    {
        VDBManagerRelease ( mgr );
        VCursorRelease ( curs );
    }

    // call once per process
    static void ReleaseCache()
    {
        for (auto d : *dbs )
        {
            VDatabaseRelease ( d.second );
        }
        delete dbs;
        dbs = nullptr;
    }

    rc_t Setup( const char * acc, const char* column[], bool open = true )
    {
        rc_t rc = 0;
        rc_t rc2;
        const VDatabase *db = NULL;

        std :: string ac(acc);
        auto d = dbs->find(ac);
        if ( d != dbs->end() )
        {
            db = d->second;
            rc = VDatabaseAddRef( db );
        }
        else
        {
            rc = VDBManagerOpenDBRead ( mgr, &db, NULL, acc );
            if ( rc == 0 )
            {
                dbs->insert( Databases::value_type( ac, db) );
                rc = VDatabaseAddRef( db );
            }
        }

        const VTable *tbl = NULL;
        if (rc == 0)
        {
            rc = VDatabaseOpenTableRead ( db, &tbl, "SEQUENCE" );
        }
        else
        {
            rc = VDBManagerOpenTableRead ( mgr, &tbl, NULL, acc );
            if (rc != 0)
            {
                VSchema *schema = NULL;
                rc = VDBManagerMakeSRASchema(mgr, &schema);
                if ( rc != 0 )
                {
                    return rc;
                }

                rc = VDBManagerOpenTableRead ( mgr, &tbl, schema, acc );

                rc2 = VSchemaRelease ( schema );
                if ( rc == 0 )
                    rc = rc2;
            }
        }

        if ( rc == 0 && column[0] != NULL )
        {
            rc = VTableCreateCursorRead(tbl, &curs);
            if ( rc == 0 )
            {
                int i;
                int const N = (int)(sizeof(col_idx)/sizeof(col_idx[0]));
                for (i = 0; i < N; ++i)
                    col_idx[i] = (uint32_t)(~0);
                for (i = 0; column[i] != NULL && rc == 0; ++i) {
                    assert(i < N);
                    rc = VCursorAddColumn ( curs, col_idx + i, column[i] );
                }
                if ( rc == 0 && open )
                {
                    rc = VCursorOpen(curs);
                }
            }
        }

        rc2 = VTableRelease ( tbl );
        if ( rc == 0 )
            rc = rc2;

        if ( db != NULL )
        {
            rc2 = VDatabaseRelease ( db );
            if ( rc == 0 )
               rc = rc2;
        }

        return rc;
    }

    const VDBManager * mgr;
    const VCursor * curs;
    uint32_t col_idx[20];
    static Databases * dbs;
};

VDB_Fixture::Databases * VDB_Fixture::dbs = nullptr;
