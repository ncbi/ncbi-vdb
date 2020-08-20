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
#include <vdb/vdb-priv.h>
#include <klib/text.h>
#include <kdb/kdb-priv.h> /* KDBManagerGetVFSManager */
#include <kdb/manager.h>
#include <kdb/meta.h>
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path-priv.h> /* VPathSetAccOfParentDb */
}

struct WGS {
private:
    VPath const *url;
    VTable const *tbl;
public:
    WGS(VPath const *url, VTable const *tbl)
    : url(url)
    , tbl(tbl)
    {}

    ~WGS() {
        VPathRelease(url);
        VTableRelease(tbl);
    }

    bool is_open() const { return tbl != NULL; }

    void close() {
        VTableRelease(tbl);
        tbl = NULL;
    }

    rc_t reopen(VDBManager const *mgr, std::string const &seq_id)
    {
        VDatabase const *db = NULL;
        rc_t rc = 0;

        if (url)
            rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, url);
        else
            rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%s", seq_id.c_str());

        if (rc) return rc;

        rc = VDatabaseOpenTableRead(db, &tbl, "SEQUENCE");
        VDatabaseRelease(db);

        return rc;
    }

    unsigned getBases(uint8_t *const dst, unsigned const start, unsigned const len, int64_t const row) const
    {
        return 0;
    }
    static std::pair<std::string, int64_t> splitName(std::string const &name) {
        auto accession = std::string();
        int64_t row = 0;
        unsigned digits = 0;

        for (auto && ch : name) {
            if (ch >= '0' && ch <= '9')
                ++digits;
            else
                digits = 0;
        }
        if (digits > 2) {
            auto const row_len = digits - 2;
            auto const acc_len = name.size() - row_len;
            auto const &row_str = name.substr(acc_len);

            accession = name.substr(0, acc_len);
            for (auto && ch : row_str) {
                assert(ch >= '0' && ch <= '9');
                row = row * 10 + (ch - '0');
            }
        }
        return std::make_pair(accession, row);
    }
};
