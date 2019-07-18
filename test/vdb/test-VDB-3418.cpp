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

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <ktst/unit_test.hpp> // TEST_CASE
#include <vfs/path.h>
#include <vfs/manager.h>
#include <klib/text.h> 
#include <klib/out.h> 
#include <klib/printf.h> 
#include <kfs/directory.h> 
#include <kfg/config.h> 

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <utility>

using namespace std;

TEST_SUITE( VDB_3418 )

struct Cursor {
    Cursor(char const db[], char const table[], char const *expr[]) {
        VDBManager const *mgr = 0;
        if (VDBManagerMakeRead(&mgr, 0)) throw std::runtime_error("VDBManagerMakeRead");

        VDatabase const *vdb = 0;
        if (VDBManagerOpenDBRead(mgr, &vdb, 0, "%s", db)) throw std::runtime_error("VDBManagerOpenDBRead");

        VTable const *tbl = 0;
        if (VDatabaseOpenTableRead(vdb, &tbl, "%s", table)) throw std::runtime_error("VDatabaseOpenTableRead");

        if (VTableCreateCursorRead(tbl, &curs)) throw std::runtime_error("VTableCreateCursorRead");

        for (unsigned i = 0; expr[i]; ++i) {
            uint32_t cid = 0;
            if (VCursorAddColumn(curs, &cid, "%s", expr[i])) throw std::runtime_error(std::string("VCursorAdd ") + expr[i]);
            column.push_back(std::make_pair(cid, std::string(expr[i])));
        }

        if (VCursorOpen(curs)) throw std::runtime_error("VCursorOpen");

        VTableRelease(tbl);
        VDatabaseRelease(vdb);
        VDBManagerRelease(mgr);
    }
    ~Cursor() {
        VCursorRelease(curs);
    }

    int columnNumber(std::string const &expr) const
    {
        unsigned i;
        unsigned const N = column.size();

        for (i = 0; i < N; ++i) {
            if (column[i].second == expr) return i;
        }
        return -1;
    }

    template <typename T>
    std::vector<T> read(int64_t row, unsigned col) const
    {
        void const *base = 0;
        uint32_t count = 0;
        uint32_t boff = 0;
        uint32_t elem_bits = 0;
            
        if (VCursorCellDataDirect(curs, row, column[col].first, &elem_bits, &base, &boff, &count)) throw std::runtime_error("VCursorCellDataDirect");
        if (sizeof(T) * 8 != elem_bits) throw std::logic_error("element size");
        if (boff) throw std::logic_error("bit aligned data");

        T const *const beg = reinterpret_cast<T const *>(base);
        return std::vector<T>(beg, beg + count);
    }

    std::pair<int64_t, int64_t> rowRange() const
    {
        uint64_t count = 0;
        int64_t first = 0;
        if (VCursorIdRange(curs, 0, &first, &count)) throw std::runtime_error("VCursorIdRange");
        return std::make_pair(first, first + count);
    }

private:
    VCursor const *curs;
    std::vector<std::pair<uint32_t, std::string>> column;
};

#define COL_READ "(INSDC:dna:text)READ"
#define COL_ALIGN_COUNT "(U8)ALIGNMENT_COUNT"
#define COL_SPOT_ID "(INSDC:SRA:spotid_t)SPOT_ID"
#define COL_PRIM_AL_ID "(I64)PRIMARY_ALIGNMENT_ID"
#define COL_READ_TYPE "(INSDC:SRA:xread_type)READ_TYPE"
#define COL_READ_FILTER "(INSDC:SRA:read_filter)READ_FILTER"
#define COL_READ_LEN "(INSDC:coord:len)READ_LEN"
#define COL_READ_START "(INSDC:coord:zero)READ_START"
#define COL_QUALITY "(INSDC:quality:text:phred_33)QUALITY"
// #define COL_QUALITY "(INSDC:quality:phred)QUALITY"
#define COL_SPOT_GROUP "(ascii)SPOT_GROUP"
#define COL_NAME "(ascii)NAME"
#define COL_LNK_GROUP "(ascii)LINKAGE_GROUP"

TEST_CASE( CheckSEQUENCE )
{
    char const *columns[] = {
        COL_ALIGN_COUNT,          /*  0 */
        COL_PRIM_AL_ID,           /*  1 */
        COL_NAME,                 /*  2 */
        COL_LNK_GROUP,            /*  3 */
        COL_READ_TYPE,            /*  4 */
        COL_READ_FILTER,          /*  5 */
        COL_READ_LEN,             /*  6 */
        COL_READ_START,           /*  7 */
        COL_READ,                 /*  8 */
        COL_QUALITY,              /*  9 */
        COL_SPOT_GROUP,           /* 10 */
        0
    };
    auto const curs = Cursor("./db/VDB-3418.sra", "SEQUENCE", columns);
    auto const range = curs.rowRange();
    auto const readCol = curs.columnNumber(COL_READ);
    auto const qualCol = curs.columnNumber(COL_QUALITY);

    for (auto row = range.first; row < range.second; ++row) {
        auto const counts = curs.read<uint8_t>(row, 0);
        REQUIRE(counts.size() == 1);
        if (counts[0] == 0) {
            auto const read = curs.read<char>(row, readCol);
            auto const qual = curs.read<uint8_t>(row, qualCol);

            REQUIRE(read.size() == qual.size());
        }
    }
}

//////////////////////////////////////////// Main
extern "C"
{
#include <kapp/args.h>

    ver_t CC KAppVersion ( void ) { return 0x1000000; }
    rc_t CC UsageSummary ( const char * progname ) { return 0; }
    rc_t CC Usage ( const Args * args ) { return 0; }
    const char UsageDefaultName[] = "test-VDB-3418";

    rc_t CC KMain(int argc, char *argv[])
    {
        return VDB_3418(argc, argv);
    }
}
