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

#define REFCOUNTING_MESSAGES 0

#include <ktst/unit_test.hpp>

#include "kdb.hpp"

using namespace std;
using namespace KDB;
using namespace KFS;

TEST_SUITE(KdbTestSuite);

class WKDB_MetaCopyFixture
{
    void checkValue(Metadata const &src, Metadata const &other) {
        if (src.nodeValue() == other.nodeValue())
            return;
        throw std::logic_error("value differs");
    }

    void checkAttributes(Metadata const &src, Metadata const &other) {
        auto const &names = src.attributes();
        {
            auto const others = other.attributes();

            if (names.count() == others.count())
                ;
            else
                throw std::logic_error("attribute count differs");
        }
        names.foreach([&](char const *name) {
            auto const &value = src.attribute(name);
            try {
                auto const &others = other.attribute(name);
                if (value == others)
                    ;
                else
                    throw std::logic_error(std::string("attribute ") + name + " differs");
            }
            catch (Klib::ResultCodeException const &rce) {
                throw std::logic_error(std::string("attribute ") + name + " does not exist");
            }
        });
    }

    void checkChildren(Metadata const &src, Metadata const &other) {
        auto const &names = src.children();
        {
            auto const &others = other.children();
            if (names.count() == others.count())
                ;
            else
                throw std::logic_error("child count differs");
        }
        names.foreach([&](char const *name) {
            try {
                checkEqual(src[name], other[name]);
            }
            catch (RC_Exception const &e) {
                throw std::logic_error("child " + std::string(name) + " does not exist");
            }
            catch (std::logic_error const &e) {
                throw std::logic_error("child " + std::string(name) + " differs; " + e.what());
            }
        });
    }

    static char const *tempPath() {
        static char const *const env_vars[] = {
            "TMPDIR", "TEMPDIR", "TEMP", "TMP", "TMP_DIR", "TEMP_DIR", nullptr
        };
        for (auto var = env_vars; *var; ++var) {
            auto const val = getenv(*var);
            if (val)
                return val;
        }
        return "/tmp";
    }
public:

    static constexpr char const *tableName() { return "VDB-4706.ktable"; }
    static constexpr char const *tableName2() { return "VDB-4706_2.ktable"; }
    static constexpr char const *columnName1() { return "COL_1"; }
    static constexpr char const *columnName2() { return "COL_2"; }
    static constexpr char const *nodeName() { return "MDN_1"; }
    static constexpr char const *childName() { return "MDN_C"; }
    static constexpr char const *nodeValue() { return "Metadata 1"; }
    static constexpr char const *childValue() { return "Metadata child"; }
    static constexpr char const *attrName() { return "foo"; }
    static constexpr char const *attrValue() { return "bar"; }

    MutatingDirectory dir = MutatingDirectory(tempPath());

    WKDB_MetaCopyFixture()
    {
        try {
            dir.remove(tableName(), true);
            dir.remove(tableName2(), true);
        }
        catch (...) {}
    }

    ~WKDB_MetaCopyFixture()
    {
        dir.remove(tableName(), true);
    }

    void checkEqual(Metadata const &src, Metadata const &other) {
        checkValue(src, other);
        checkAttributes(src, other);
        checkChildren(src, other);
    }

    // create column with some metadata
    // this will become the source metadata
    void makeNode(MutatingTable &tbl) {
        auto md = tbl[columnName1()][nodeName()];
        auto child = md[childName()];

        md.setValue(nodeValue());
        child.setValue(childValue());
        child.setAttribute(attrName(), attrValue());
    }

    // verify that node has expected values
    void checkNode(Metadata const &source_md) {
        if (source_md.value() != nodeValue())
            throw std::logic_error("node value is unexpected");

        if (source_md[childName()].value() != childValue())
            throw std::logic_error("child node value is unexpected");

        if (source_md[childName()].attribute(attrName()) != attrValue())
            throw std::logic_error("child node attribute value is unexpected");
    }

    // Copy a metadata node from one column to another
    void makeCopy() {
        MutatingManager mgr(dir);
        auto tbl = mgr.createTable(tableName());

        makeNode(tbl);

        auto const &src = tbl.readOnly()[columnName1()][nodeName()];
        checkNode(src);

        auto dst = tbl[columnName2()][nodeName()];
        dst.copy(src);
    }

    // Copy a metadata node from one column to another table
    void makeCopy2() {
        MutatingManager mgr(dir);
        auto tbl = mgr.createTable(tableName());

        makeNode(tbl);

        auto const &src = tbl.readOnly()[columnName1()][nodeName()];
        checkNode(src);

        auto tbl2 = mgr.createTable(tableName2());
        auto dst = tbl2[columnName1()][nodeName()];
        dst.copy(src);
    }
};

FIXTURE_TEST_CASE ( CopyMetadata, WKDB_MetaCopyFixture )
{
    makeCopy();

    auto const &mgr = MutatingManager(dir).readOnly();
    auto tbl = mgr.openTable(tableName());
    checkEqual(tbl[columnName1()][nodeName()], tbl[columnName2()][nodeName()]);
}

FIXTURE_TEST_CASE ( CopyMetadata2, WKDB_MetaCopyFixture )
{
    makeCopy2();

    auto const &mgr = MutatingManager(dir).readOnly();
    auto tbl = mgr.openTable(tableName());
    auto tbl2 = mgr.openTable(tableName2());
    checkEqual(tbl[columnName1()][nodeName()], tbl2[columnName1()][nodeName()]);
}

extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "VDB-4706";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    return KdbTestSuite(argc, argv);
}

}
