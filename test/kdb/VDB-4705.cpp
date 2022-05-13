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

/**
 * Unit tests for Kdb interface
 */

#include <kapp/args.h>

#include <ktst/unit_test.hpp>

#include <sysalloc.h>

#include <string>
#include <vector>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>

#include <vfs/manager.h>

#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/table.h>
#include <kfs/directory.h>
#include <klib/printf.h>


#include <kdb/extern.h>
#include <kdb/kdb-priv.h>
#include <kdb/namelist.h>



//#include <memory>
//#include <string>


using namespace std;

TEST_SUITE ( KdbTestSuite );

const char UsageDefaultName[] = "VDB-4705";

class KDB_ColumnCopyFixture {
public:
    void createTable ( const char *tblname )
    {
        KDirectory *Dir = nullptr;
        THROW_ON_RC ( KDirectoryNativeDir ( &Dir ) );
        KDBManager *mgr = nullptr;
        THROW_ON_RC ( KDBManagerMakeUpdate ( &mgr, Dir ) );

        tables.push_back ( tblname );

        KTable *Tbl = nullptr;
        THROW_ON_RC ( KDBManagerCreateTable ( mgr, &Tbl,
            kcmInit | kcmCreate | kcmParents, "%s/%s", tempPath (), tblname ) );

        KColumn *Col = nullptr;
        THROW_ON_RC (
            KTableCreateColumn ( Tbl, &Col, kcmInit, 0, 0, columnName ) );

        KMetadata *Meta;
        THROW_ON_RC ( KColumnOpenMetadataUpdate ( Col, &Meta ) );

        KMDataNode *Node;
        THROW_ON_RC ( KMetadataOpenNodeUpdate ( Meta, &Node, nodeName ) );
        THROW_ON_RC ( KMDataNodeWrite ( Node, colValue, strlen ( colValue ) ) );
        THROW_ON_RC ( KMDataNodeWriteAttr ( Node, attrName, attrValue ) );

        THROW_ON_RC ( KMDataNodeRelease ( Node ) );
        THROW_ON_RC ( KMetadataRelease ( Meta ) );
        THROW_ON_RC ( KColumnRelease ( Col ) );
        THROW_ON_RC ( KTableRelease ( Tbl ) );
        THROW_ON_RC ( KDBManagerRelease ( mgr ) );
        THROW_ON_RC ( KDirectoryRelease ( Dir ) );
    }

    // Make a column and copy it to another table
    void makeCopy ( const char *dstTable, const char *srcTable )
    {
        KDirectory *Dir = nullptr;
        THROW_ON_RC ( KDirectoryNativeDir ( &Dir ) );
        KDBManager *mgr = nullptr;
        THROW_ON_RC ( KDBManagerMakeUpdate ( &mgr, Dir ) );
        tables.push_back ( dstTable );

        const KTable *Tbl1 = nullptr;
        THROW_ON_RC ( KDBManagerOpenTableRead (
            mgr, &Tbl1, "%s/%s", tempPath (), srcTable ) );

        KTable *Tbl2 = nullptr;
        THROW_ON_RC ( KDBManagerCreateTable ( mgr, &Tbl2,
            kcmInit | kcmCreate | kcmParents, "%s/%s", tempPath (),
            dstTable ) );

        THROW_ON_RC ( KTableCopyColumn ( Tbl2, Tbl1, columnName ) );
        THROW_ON_RC ( KTableRelease ( Tbl1 ) );
        THROW_ON_RC ( KTableRelease ( Tbl2 ) );
        THROW_ON_RC ( KDBManagerRelease ( mgr ) );
        THROW_ON_RC ( KDirectoryRelease ( Dir ) );
    }

    void checkTable ( const char *tblName )
    {
        fprintf ( stderr, "checking %s\n", tblName );

        KDirectory *Dir = nullptr;
        THROW_ON_RC ( KDirectoryNativeDir ( &Dir ) );
        const KDBManager *mgr = nullptr;
        // THROW_ON_RC ( KDBManagerMakeUpdate ( &mgr, Dir ) );
        THROW_ON_RC ( KDBManagerMakeRead ( &mgr, NULL ) );

        const KTable *Tbl = nullptr;
        THROW_ON_RC ( KDBManagerOpenTableRead (
            mgr, &Tbl, "%s/%s", tempPath (), tblName ) );


        const KColumn *Col = nullptr;
        THROW_ON_RC ( KDBManagerOpenColumnRead ( mgr, &Col, columnName ) );

        const KMetadata *Meta;
        THROW_ON_RC ( KColumnOpenMetadataRead ( Col, &Meta ) );

        const KMDataNode *Node;
        THROW_ON_RC ( KMetadataOpenNodeRead ( Meta, &Node, nodeName ) );

        size_t num_read;
        char buf[1024];
        THROW_ON_RC (
            KMDataNodeRead ( Node, 0, buf, sizeof buf, &num_read, 0 ) );

        char attrValue[50];
        THROW_ON_RC ( KMDataNodeReadAttr (
            Node, attrName, attrValue, sizeof ( attrValue ), &num_read ) );


        THROW_ON_RC ( KMDataNodeRelease ( Node ) );
        THROW_ON_RC ( KMetadataRelease ( Meta ) );
        THROW_ON_RC ( KColumnRelease ( Col ) );
        THROW_ON_RC ( KTableRelease ( Tbl ) );
        THROW_ON_RC ( KDBManagerRelease ( mgr ) );
        THROW_ON_RC ( KDirectoryRelease ( Dir ) );
    }

    static char const *tempPath ()
    {
        static char const *const env_vars[] = { "TMPDIR", "TEMPDIR", "TEMP",
            "TMP", "TMP_DIR", "TEMP_DIR", nullptr };
        for ( auto var = env_vars; *var; ++var ) {
            auto const val = getenv ( *var );
            if ( val ) return val;
        }
        return "/tmp";
    }

    /*
    void checkValue ( Metadata const &src, Metadata const &other )
    {
        if ( src.nodeValue () == other.nodeValue () ) return;
        throw std::logic_error ( "value differs" );
    }

    void checkAttributes ( Metadata const &src, Metadata const &other )
    {
        auto const &names = src.attributes ();
        {
            auto const others = other.attributes ();

            if ( names.count () == others.count () )
                ;
            else
                throw std::logic_error ( "attribute count differs" );
        }
        names.foreach ( [&] ( char const *name ) {
            auto const &value = src.attribute ( name );
            try {
                auto const &others = other.attribute ( name );
                if ( value == others )
                    ;
                else
                    throw std::logic_error (
                        std::string ( "attribute " ) + name + " differs" );
            } catch ( Klib::ResultCodeException const &rce ) {
                throw std::logic_error (
                    std::string ( "attribute " ) + name + " does not exist" );
            }
        } );
    }

    void checkChildren ( Metadata const &src, Metadata const &other )
    {
        auto const &names = src.children ();
        {
            auto const &others = other.children ();
            if ( names.count () == others.count () )
                ;
            else
                throw std::logic_error ( "child count differs" );
        }
        names.foreach ( [&] ( char const *name ) {
            try {
                checkEqual ( src[name], other[name] );
            } catch ( RC_Exception const &e ) {
                throw std::logic_error (
                    "child " + std::string ( name ) + " does not exist" );
            } catch ( std::logic_error const &e ) {
                throw std::logic_error ( "child " + std::string ( name )
                    + " differs; " + e.what () );
            }
        } );
    }
*/

    ~KDB_ColumnCopyFixture ()
    {
        for ( auto tblname : tables ) {
            KDirectory *Dir = nullptr;
            KDirectoryNativeDir ( &Dir );
            KDirectoryRemove (
                Dir, true, "%s/%s", tempPath (), tblname.c_str () );
            KDirectoryRelease ( Dir );
        }
    }

    /*
        void checkEqual ( Metadata const &src, Metadata const &other )
        {
            checkValue ( src, other );
            checkAttributes ( src, other );
            checkChildren ( src, other );
        }

        // create column with some metadata
        // this will become the source metadata
        void makeNode ( MutatingTable &tbl )
        {
            auto md = tbl[columnName ()][nodeName ()];
            auto child = md[childName ()];

            md.setValue ( nodeValue () );
            child.setValue ( childValue () );
            child.setAttribute ( attrName (), attrValue () );
        }

        // verify that node has expected values
        void checkNode ( Metadata const &source_md )
        {
            if ( source_md.value () != nodeValue () )
                throw std::logic_error ( "node value is unexpected" );

            if ( source_md[childName ()].value () != childValue () )
                throw std::logic_error ( "child node value is unexpected" );

            if ( source_md[childName ()].attribute ( attrName () ) != attrValue
       () ) throw std::logic_error ( "child node attribute value is unexpected"
       );
        }
        */
private:
    static constexpr char const *columnName = "COL_1";
    static constexpr char const *nodeName = "MDN_1";
    static constexpr char const *childName = "MDN_C";
    static constexpr char const *nodeValue = "Metadata 1";
    static constexpr char const *childValue = "Metadata child";
    static constexpr char const *attrName = "foo";
    static constexpr char const *attrValue = "bar";
    static constexpr char const *colValue = "colValue";

    vector<string> tables;
};

FIXTURE_TEST_CASE ( CopyColumn, KDB_ColumnCopyFixture )
{
    static constexpr char const *tableName = "VDB-4705.ktable";
    static constexpr char const *tableName2 = "VDB-4705_2.ktable";
    createTable ( tableName );
    checkTable ( tableName );
    makeCopy ( tableName2, tableName );
    checkTable ( tableName2 );
}

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char *progname ) { return 0; }

rc_t CC Usage ( const Args *args ) { return 0; }

rc_t CC KMain ( int argc, char *argv[] )
{
    KConfigDisableUserSettings ();
    return KdbTestSuite ( argc, argv );
}
}
