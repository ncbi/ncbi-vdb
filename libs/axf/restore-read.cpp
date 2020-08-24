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
#include <klib/log.h>
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

#define DEFAULT_WGS_OPEN_LIMIT 8

#include "restore-read.h"
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include "refseq.hpp"
#include "wgs.hpp"

static VFSManager *getVFSManager(VDBManager const *mgr)
{
    VFSManager *result = NULL;
#if 0
    KDBManager const *kmgr = NULL;
    rc_t rc = 0;

    rc = VDBManagerGetKDBManager(mgr, &kmgr);
    assert(rc == 0);

    rc = KDBManagerGetVFSManager(kmgr, &result);
    KDBManagerRelease(kmgr);
    assert(rc == 0);
#endif
    return result;
}

static VPath *makePath(VDBManager const *mgr, std::string const &accession)
{
    VPath *result = NULL;
    auto const vfs = getVFSManager(mgr);
    VFSManagerMakePath(vfs, &result, "%s", accession.c_str());
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

static std::string getSchemaName(KMDataNode const *node)
{
    char buffer[1024];
    size_t sz = 0;
    auto const rc = KMDataNodeReadAttr(node, "name", buffer, 1024, &sz);
    KMDataNodeRelease(node);
    if (rc) return "";
    return std::string(buffer, sz);
}

static std::string getSchemaName(VDatabase const *db)
{
    KMetadata const *meta = NULL;
    rc_t rc = VDatabaseOpenMetadataRead(db, &meta);
    assert(rc == 0);

    KMDataNode const *node = NULL;
    rc = KMetadataOpenNodeRead(meta, &node, "schema");
    KMetadataRelease(meta);
    if (rc) return "";

    return getSchemaName(node);
}

static std::string getSchemaName(VTable const *tbl)
{
    KMetadata const *meta = NULL;
    rc_t rc = VTableOpenMetadataRead(tbl, &meta);
    assert(rc == 0);

    KMDataNode const *node = NULL;
    rc = KMetadataOpenNodeRead(meta, &node, "schema");
    KMetadataRelease(meta);
    if (rc) return "";

    return getSchemaName(node);
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
    unsigned wgsOpenCount;
    unsigned const wgsOpenCountLimit;

    RestoreRead(VDBManager const *const mgr, rc_t *rcp, unsigned wgsOpenCountLimit = DEFAULT_WGS_OPEN_LIMIT)
    : mgr(mgr)
    , wgsOpenCount(0)
    , wgsOpenCountLimit(wgsOpenCountLimit)
    {
        *rcp = VDBManagerAddRef(mgr);
    }
    ~RestoreRead()
    {
        VDBManagerRelease(mgr);
    }

    using WGS_Iter = decltype(wgs)::iterator;
    std::vector<WGS_Iter> wgsOpenOrderByLastAccess() {
        auto list = std::vector<WGS_Iter>();
        list.reserve(wgsOpenCount);
        for (auto i = wgs.begin(); i != wgs.end(); ++i) {
            if (i->second.is_open())
                list.push_back(i);
        }
        assert(list.size() == wgsOpenCount);
        std::sort(list.begin(), list.end(), [](WGS_Iter a, WGS_Iter b) {
            return a->second < b->second;
        });
        return list;
    }

    void limitOpenWGS() {
        if (wgsOpenCount < wgsOpenCountLimit)
            return;

        for (auto && i : wgsOpenOrderByLastAccess()) {
            i->second.close();
            --wgsOpenCount;
            if (wgsOpenCount < wgsOpenCountLimit)
                return;
        }
    }

    void openSeqID(  std::string const &seq_id
                   , VTable const *const vtbl)
    {
        rc_t rc = 0;
        if (!seq_id.empty()) {
            VDatabase const *db = NULL;
            VTable const *tbl = NULL;
            auto const url = getURL(mgr, seq_id, vtbl);

            if (url) {
                // open the new way with the URL
                VDBManagerOpenTableReadVPath(mgr, &tbl, NULL, url);
                if (tbl == NULL)
                    rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, url);
            }
            else {
                // open the old way
                VDBManagerOpenTableRead(mgr, &tbl, NULL, "ncbi-acc:%s?vdb-ctx=refseq", seq_id.c_str());
                if (tbl == NULL)
                    rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%s", seq_id.c_str());
            }
            if (tbl == NULL && db == NULL) {
                VPathRelease(url);
                // last chance, try to open as a bare WGS accession (name without row)
                // WGS::splitName(seq_id).first will be "" if seq_id doesn't fit WGS pattern
                return openSeqID(WGS::splitName(seq_id).first, vtbl);
            }

            auto const scheme = tbl ? getSchemaName(tbl) : getSchemaName(db);

            if (RefSeq::isScheme(scheme)) {
                if (tbl) {
                    refSeqs.emplace(seq_id, RefSeq::load(url, tbl));
                    return;
                }
            }
            else if (WGS::isScheme(scheme)) {
                if (db) {
                    limitOpenWGS();
                    wgs.emplace(WGS::splitName(seq_id).first, WGS(url, db));
                    ++wgsOpenCount;
                    return;
                }
            }
            VTableRelease(tbl);
            VDatabaseRelease(db);
            VPathRelease(url);
        }
        throw rc ? rc : RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
    }
    unsigned getBases_WGS(  uint8_t *const dst
                          , unsigned const start
                          , unsigned const length
                          , WGS::SplitName const &nr
                          , WGS const &cwgs)
    {
        auto &wgs = const_cast<WGS &>(cwgs);
        if (!wgs.is_open()) {
            limitOpenWGS();
            wgs.reopen(mgr, nr.first);
            ++wgsOpenCount;
        }
        return wgs.getBases(dst, start, length, nr.second);
    }
    unsigned getBases(  uint8_t *const dst
                      , unsigned const start
                      , unsigned const length
                      , std::string const &seq_id
                      , VTable const *const vtbl)
    {
        switch (last.type) {
        case Last::refSeq_type:
            if (last.name == seq_id)
                return last.u.ri->second.getBases(dst, start, length);
            break;
        case Last::wgs_type:
            {
                auto const nr = WGS::splitName(seq_id);
                if (last.name == nr.first)
                    return getBases_WGS(dst, start, length, nr, last.u.wi->second);
            }
            break;
        default:
            break;
        }
        {
            auto const i = errors.find(seq_id);
            if (i != errors.end())
                throw i->second;
        }
        {
            auto const i = refSeqs.find(seq_id);
            if (i != refSeqs.end()) {
                last.name = seq_id;
                last.type = Last::refSeq_type;
                last.u.ri = i;
                return i->second.getBases(dst, start, length);
            }
        }
        {
            auto const nr = WGS::splitName(seq_id);
            auto const i = wgs.find(nr.first);
            if (i != wgs.end() && nr.second != 0) {
                last.name = nr.first;
                last.type = Last::wgs_type;
                last.u.wi = i;
                return getBases_WGS(dst, start, length, nr, i->second);
            }
        }
        last.type = Last::none;

        try {
            openSeqID(seq_id, vtbl);
            return getBases(dst, start, length, seq_id, vtbl);
        }
        catch (rc_t rc) {
            errors[seq_id] = rc;
            throw rc;
        }
        catch (...) {
            errors[seq_id] = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            throw;
        }
    }
};

extern "C" {
    void RestoreReadFree(void *const self)
    {
        delete reinterpret_cast<RestoreRead *>(self);
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
        try {
            *actual = self->getBases(dst, start, length, std::string(seq_id, id_len), forTable);
            return 0;
        }
        catch (rc_t rc) { return rc; }
        catch (...) {
            rc_t const rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            PLOGERR(klogErr, (klogErr, rc, "Unexpected exception type while reading $(seqId)@$(start)[$(length)]", "seqId=%.*s,start=%u,length=%zu", (int)id_len, seq_id, start, length));
            return rc;
        }
    }
}
