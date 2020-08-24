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


extern "C" {
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vfs/path.h>
}

#define USE_ATOMIC (0)
#if USE_ATOMIC
#include <atomic>
#endif

#include "wgs.hpp"

void WGS::stamp() {
#if USE_ATOMIC
    static std::atomic<uint64_t> generation = 0;
    lastAccessStamp = atomic_add(&generation, 1);
#else
    static uint64_t generation = 0;
    lastAccessStamp = generation++;
#endif
}

/// Exceptions from this function really are exceptional.
/// Everything was supposed to have been checked before this function is called.
void WGS::openCursor(VDatabase const *db)
{
    VTable const *tbl = NULL;
    rc_t rc = VDatabaseOpenTableRead(db, &tbl, "SEQUENCE");
    VDatabaseRelease(db);
    if (rc) throw rc;

    rc = VTableCreateCachedCursorRead(tbl, &curs, 0);
    VTableRelease(tbl);
    if (rc) throw rc;

    try {
        rc = VCursorAddColumn(curs, &colID, "(INSDC:4na:bin)READ");
        if (rc) throw rc;

        rc = VCursorOpen(curs);
        if (rc) throw rc;
    }
    catch (...) {
        VCursorRelease(curs);
        curs = NULL;
        throw;
    }
}

/// Exceptions from this function really are exceptional. We had the database open before.
void WGS::reopen(VDBManager const *mgr, std::string const &seq_id)
{
    VDatabase const *db = NULL;
    rc_t rc = 0;

    if (url)
        rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, url);
    else
        rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%s", seq_id.c_str());

    if (rc) throw rc;

    openCursor(db);
}

void WGS::close() {
    VCursorRelease(curs);
    curs = NULL;
}

WGS::~WGS() {
    VCursorRelease(curs);
    VPathRelease(url);
}

/// There is some expectation that this function can throw, e.g. if the row is outside the valid row range.
WGS::ReadResult WGS::read(int64_t row) const
{
    void const *value = NULL;
    uint32_t length = 0;
    rc_t const rc = VCursorCellDataDirect(curs, row, colID, NULL, &value, NULL, &length);
    if (rc) throw rc;
    return { reinterpret_cast<uint8_t const *>(value), unsigned(length) };
}
