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

#include "restore-read.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include "range-list.hpp"
#include "refseq.hpp"
#include "wgs.hpp"

static VFSManager *getVFSManager(VDBManager const *mgr)
{
    VFSManager *result = NULL;
    KDBManager const *kmgr = NULL;
    rc_t rc = 0;

    rc = VDBManagerGetKDBManager(mgr, &kmgr);
    assert(rc == 0);

    rc = KDBManagerGetVFSManager(kmgr, &result);
    KDBManagerRelease(kmgr);
    assert(rc == 0);

    return result;
}

static VPath *makePath(VDBManager const *mgr, std::string const &accession)
{
    VPath *result = NULL;
    auto const vfs = getVFSManager(mgr);
    auto const rc = VFSManagerMakePath(vfs, &result, "%s", accession.c_str());
    VFSManagerRelease(vfs);
    return result;
}

static String const *getContainer(VTable const *const forTable)
{
    VDatabase const *db = NULL;
    rc_t rc = VTableOpenParentRead(forTable, &db);
    if (rc == 0) {
        String const *container = NULL;
        rc = VDatabaseGetAccession(db, &container);
        VDatabaseRelease(db);
        if (rc == 0)
            return container;
    }
    return NULL;
}

static VPath const *getURL(VDBManager const *mgr, std::string const &refSeqID, VTable const *const forTable)
{
    auto const result = makePath(mgr, refSeqID);
    if (result) {
        auto const container = getContainer(forTable);
        auto const rc = VPathSetAccOfParentDb(result, container);
        StringWhack(container);
        if (rc == 0)
            return result;
        VPathRelease(result);
    }
    return NULL;
}

struct RestoreRead {
    VDBManager const *mgr;
    std::map<std::string, RefSeq> refSeqs;
    std::map<std::string, WGS> wgs;
    std::map<std::string, rc_t> errors;
    struct Last {
        std::string name;
        enum { none, refSeq_type, wgs_type } type;
        union U {
            decltype(refSeqs)::const_iterator ri;
            decltype(wgs)::const_iterator wi;
            U() {}
        } u;
        Last() : type(none) {}
    };
    Last last;

    RestoreRead(VDBManager const *const mgr, rc_t *rcp)
    : mgr(mgr)
    {
        *rcp = VDBManagerAddRef(mgr);
    }
    ~RestoreRead()
    {
        VDBManagerRelease(mgr);
    }
    rc_t openSeqID(  std::string const &seq_id
                   , VTable const *const vtbl)
    {
        rc_t rc = 0;
        KMetadata const *meta = NULL;
        VDatabase const *db = NULL;
        VTable const *tbl = NULL;
        auto const url = getURL(mgr, seq_id, vtbl);

        if (url) {
            VDBManagerOpenTableReadVPath(mgr, &tbl, NULL, url);
            if (tbl == NULL)
                rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, url);
        }
        else {
            VDBManagerOpenTableRead(mgr, &tbl, NULL, "ncbi-acc:%s?vdb-ctx=refseq", seq_id.c_str());
            if (tbl == NULL)
                rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%s", seq_id.c_str());
        }
        if (tbl == NULL && db == NULL) return rc;
    }
    rc_t getBases(  uint8_t *const dst
                  , unsigned const start
                  , unsigned const length
                  , unsigned *actual
                  , std::string const &seq_id
                  , VTable const *const vtbl)
    {
        switch (last.type) {
        case refSeq_type:
            if (last.name == seq_id) {
                *actual = last.u.ri->second.getBases(dst, start, length);
                return 0;
            }
            break;
        case wgs_type:
            {
                auto const nr = WGS::splitName(seq_id);
                if (last.name == nr.first) {
                    auto &wgs = last.u.wi->second;
                    if (!wgs.is_open()) {
                        auto const rc = wgs.reopen(mgr, nr.first);
                        if (rc) return rc;
                    }
                    *actual = wgs.getBases(dst, start, length, nr.second);
                    return 0;
                }
            }
            break;
        default:
            break;
        }
        {
            auto const i = errors.find(seq_id);
            if (i != errors.end())
                return i->second;
        }
        {
            auto const i = refSeqs.find(seq_id);
            if (i != refSeqs.end()) {
                *actual = i->second.getBases(dst, start, length);
                last.name = seq_id;
                last.type = refSeq_type;
                last.u.ri = i;
                return 0;
            }
        }
        {
            auto const nr = WGS::splitName(seq_id);
            auto const i = wgs.find(nr.first);
            if (i != wgs.end() && nr.second != 0) {
                auto &wgs = last.u.wi->second;
                if (!wgs.is_open()) {
                    auto const rc = wgs.reopen(mgr, nr.first);
                    if (rc) return rc;
                }
                *actual = i->second.getBases(dst, start, length, nr.second);
                last.name = nr.first;
                last.type = wgs_type;
                last.u.wi = i;
                return 0;
            }
        }
        last.type = none;

        rc_t rc = openSeqID(seq_id, vtbl);
        if (rc == 0)
            return getBases(dst, start, length, actual, seq_id, vtbl);

        errors[seq_id] = rc;
        return rc;
    }
};

extern "C" {
    void RestoreReadFree(RestoreRead *const self)
    {
        delete self;
    }

    RestoreRead *RestoreReadMake(VDBManager const *vmgr, rc_t *rcp)
    {
        return new RestoreRead(vmgr, rcp);
    }

    rc_t RestoreReadGetSequence(  RestoreRead *const self
                                , unsigned const start
                                , size_t const length, uint8_t *const dst
                                , size_t const id_len, char const *const seq_id
                                , unsigned *actual
                                , VTable const *const forTable)
    {
        return self->getBases(dst, start, length, actual, std::string(seq_id, id_len), forTable);
    }
}
