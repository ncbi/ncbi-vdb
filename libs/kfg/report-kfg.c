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

#include <kfg/extern.h>

#include <kfg/config.h>
#include <kfg/repository.h> /* KRepositoryMgr */
#include <kfg/kfg-priv.h>

#include <klib/namelist.h> /* KNamelistRelease */
#include <klib/out.h> /* OUTMSG */
#include <klib/klib-priv.h>
#include <klib/rc.h>

#include <kfs/directory.h>
#include <kfs/dyload.h> /* KDyld */
#include <kfs/file.h> /* KFileRead */
#include <kfs/nullfile.h> /* KFileMakeNullUpdate */
#include <kfs/md5.h> /* KMD5SumFmt */

#include <sysalloc.h>

#include <stdarg.h> /* va_start */
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* memset */
#include <limits.h> /* PATH_MAX */
#include <assert.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/*
 * An unrecoverable error happened.
 * We can help to solve it
 * by reporting information about known application execution environment.
 */

#define report ( * f -> report )
#define reportData ( * f -> reportData )
#define reportData1 ( * f -> reportData1 )
#define reportOpen ( * f -> reportOpen )
#define reportOpen1 ( * f -> reportOpen1 )
#define reportClose ( * f -> reportClose )
#define reportClose1 ( * f -> reportClose1 )
#define reportError ( * f -> reportError )
#define reportErrorStr ( * f -> reportErrorStr )
#define reportErrorStrImpl ( * f -> reportErrorStrImpl )
#define reportErrorStrInt ( * f -> reportErrorStrInt )
#define reportError3Str ( * f -> reportError3Str )

static rc_t ReportKfgFiles(const ReportFuncs *f,
    int indent, const KConfig* cfg)
{
    rc_t rc = 0;

    uint32_t count = 0;
    KNamelist* names = NULL;

    rc = KConfigListIncluded(cfg, &names);

    if (rc != 0) {
        reportOpen(indent, "Files", 0);
        reportError(indent + 1, rc, "KConfigListIncluded");
    }
    else {
        rc = KNamelistCount(names, &count);
        if (rc != 0) {
            reportOpen(indent, "Files", 0);
            reportErrorStr(indent + 1,
                rc, "KNamelistCount", "origin", "KConfigListIncluded");
        }
        else {
            uint32_t i = 0;
            reportOpen(indent, "Files", 1, "count", 'd', count);
            for (i = 0; i < count && rc == 0; ++i) {
                const char* name = NULL;
                rc = KNamelistGet(names, i, &name);
                if (rc != 0) {
                    reportErrorStrInt(indent + 1, rc, "KNamelistGet",
                        "origin", "KConfigListIncluded", "idx", i);
                }
                else
                {   report(indent + 1, "File", 1, "name", 's', name); }
            }
        }
    }
    reportClose(indent, "Files");

    RELEASE(KNamelist, names);

    return rc;
}

static
rc_t sKConfigNode_Read(const KConfigNode* node, char* buffer, size_t buf_sz,
    char** overflow_buf, bool* cant_allocate)
{
    rc_t rc = 0;
    size_t num_read = 0;
    size_t remaining = 0;
    assert(overflow_buf && cant_allocate);
    *cant_allocate = false;
    *overflow_buf = NULL;
    rc = KConfigNodeRead(node, 0, buffer, buf_sz, &num_read, &remaining);
    if (rc != 0)
    {   return rc; }
    if (num_read < buf_sz)
    {   buffer[num_read] = '\0'; }
    else {
        size_t new_buf_sz = num_read + remaining + 1;
        char* overflow = malloc(new_buf_sz);
        if (overflow == NULL) {
            int i = 0;
            for (i = 2; i < 5 && buf_sz - i > 0; ++i)
            {   buffer[buf_sz - i] = '.'; }
            buffer[buf_sz - 1] = '\0';
            *cant_allocate = true;
        }
        else {
            buffer = *overflow_buf = overflow;
            buf_sz = 
            rc = KConfigNodeRead
                (node, 0, buffer, new_buf_sz, &num_read, &remaining);
            if (rc != 0)
            {   return rc; }
            assert(num_read < new_buf_sz && remaining == 0);
            buffer[num_read] = '\0';
        }
    }
    return rc;
}

static rc_t ReportChildNode(const ReportFuncs *f, int indent,
    const KNamelist* names, const KConfigNode* node, const char* root, uint32_t idx);

static
rc_t ReportConfigNodeChildren(const ReportFuncs *f, int indent,
    const KConfigNode* node, const char* nodeName)
{
    KNamelist* names = NULL;
    rc_t rc = KConfigNodeListChild(node, &names);
    if (rc != 0) {
        reportErrorStrImpl(indent,
            rc, "KConfigNodeListChild", "node", nodeName, false);
    }
    else {
        uint32_t count = 0;
        rc = KNamelistCount(names, &count);
        if (rc != 0) {
            reportErrorStrImpl
                (indent, rc, "KNamelistCount", "node", nodeName, false);
        }
        else {
            if (count) {
                uint32_t i = 0;
                int j;
                OUTMSG(("\n"));
                for (i = 0; i < count && rc == 0; ++i) {
                    rc = ReportChildNode
                        (f, indent + 1, names, node, nodeName, i);
                }
                for (j = 0; j < indent; ++j)
                {   OUTMSG((" ")); }
            }
        }
    }
    RELEASE(KNamelist, names);
    return rc;
}

static rc_t ReportConfigNode(const ReportFuncs *f, int indent, const char* root,
    const char* name, const KConfigNode* node, const char* node_name)
{
    rc_t rc = 0;

    char local[PATH_MAX + 1] = "";
    const char* nodeName = node_name ? node_name : name;
    char* overflow = NULL;
    bool cantallocate = false;

    reportOpen1(indent, nodeName);
    rc = sKConfigNode_Read(node, local, sizeof local, &overflow, &cantallocate);
    if (rc == 0) {
        char* buf = overflow ? overflow : local;
        rc = reportData1(buf);
        free(overflow);
        overflow = NULL;
        if (rc == 0)
        {   rc = ReportConfigNodeChildren(f, indent, node, nodeName); }
    }
    else {
        reportError3Str
            (indent + 1, rc, "KConfigNodeRead", "node", root, "/", name, false);
    }
    reportClose1(nodeName);

    return rc;
}

static rc_t ReportConfigNode_File(const ReportFuncs *f, int indent,
    const char* root,
    const char* name, const KConfigNode* node, const char* node_name)
{
    rc_t rc = 0;
    char local[PATH_MAX + 1] = "";
    const char* nodeName = node_name ? node_name : name;
    char* overflow = NULL;
    bool cantallocate = false;
    rc = sKConfigNode_Read(node, local, sizeof local, &overflow, &cantallocate);
    if (rc) {
        reportError3Str
            (indent, rc, "KConfigNodeRead", "node", root, "/", name, false);
    }
    else {
        const char* buf = overflow ? overflow : local;
        KPathType file_type = kptNotFound;
        const char* type = NULL;
        if (!cantallocate) {
            KDirectory* dir = NULL;
            rc = KDirectoryNativeDir(&dir);
            if (rc == 0) {
                file_type = KDirectoryPathType(dir, "%s", buf);
                file_type &= ~kptAlias;
                type = file_type == kptFile ? "exists" :
                          file_type == kptNotFound ? "not found" : "unexpected";
            }
            RELEASE(KDirectory, dir);
        }
        else
        {   type = "noo long"; }
        if (rc == 0)
        {   reportData(indent, nodeName, buf, 1, "file", 's', type); }
    }
    free(overflow);
    overflow = NULL;
    return rc;
}

static rc_t ReportChildNode(const ReportFuncs *f, int indent,
    const KNamelist* names,
    const KConfigNode* node, const char* root, uint32_t idx)
{
    rc_t rc = 0;

    const char* name = NULL;
    rc = KNamelistGet(names, idx, &name);
    if (rc != 0) {
        reportErrorStrInt
            (indent + 1, rc, "KNamelistGet", "node", root, "idx", idx);
    }
    else {
        const KConfigNode* child = NULL;

        rc = KConfigNodeOpenNodeRead(node, &child, "%s", name);
        if (rc != 0) {
            reportOpen(indent, name, 0);
            reportError3Str(indent + 1, rc, "KConfigNodeOpenNodeRead",
                "node", root, "/", name, true);
            reportClose(indent, name);
        }
        else {
            rc = ReportConfigNode(f, indent, root, name, child, NULL);
        }

        RELEASE(KConfigNode, child);
    }

    return rc;
}

static
rc_t ReportRefseq(const ReportFuncs *f, int indent, const KConfig* cfg)
{
    rc_t rc = 0;

    const char root[] = "refseq";

    const KConfigNode* node = NULL;
    rc = KConfigOpenNodeRead(cfg, &node, "%s", root);
    if (rc != 0) {
        if (GetRCState(rc) == rcNotFound) {
            report(indent, root, 1, "state", 's', "not found");
            rc = 0;
        }
        else
        {   reportErrorStr(indent, rc, "KConfigOpenNodeRead", "node", root); }
    }
    else {
        KNamelist* names = NULL;
        rc = KConfigNodeListChild(node, &names);
        if (rc != 0)
        {   reportErrorStr(indent, rc, "KConfigNodeListChild", "node", root); }
        else {
            uint32_t count = 0;
            rc = KNamelistCount(names, &count);
            if (rc != 0)
            {   reportErrorStr(indent, rc, "KNamelistCount", "node", root); }
            else {
                uint32_t i = 0;
                reportOpen(indent, root, 0);
                for (i = 0; i < count && rc == 0; ++i)
                {   rc = ReportChildNode(f, indent + 1, names, node, root, i); }
                reportClose(indent, root);
            }
        }
        RELEASE(KNamelist, names);
    }

    RELEASE(KConfigNode, node);

    return rc;
}

static
rc_t ReportSra(const ReportFuncs *f, int indent, const KConfig* cfg)
{
    rc_t rc = 0;
    const char root[] = "sra";
    const KConfigNode* node = NULL;
    assert(cfg);
    rc = KConfigOpenNodeRead(cfg, &node, "%s", root);
    if (rc != 0) {
        if (GetRCState(rc) == rcNotFound) {
            report(indent, root, 1, "state", 's', "not found");
            rc = 0;
        }
        else
        {   reportErrorStr(indent, rc, "KConfigOpenNodeRead", "node", root); }
    }
    else {
        KNamelist* names = NULL;
        rc = KConfigNodeListChild(node, &names);
        if (rc != 0)
        {   reportErrorStr(indent, rc, "KConfigNodeListChild", "node", root); }
        else {
            uint32_t count = 0;
            rc = KNamelistCount(names, &count);
            if (rc != 0)
            {   reportErrorStr(indent, rc, "KNamelistCount", "node", root); }
            else {
                uint32_t i = 0;
                reportOpen(indent, root, 0);
                for (i = 0; i < count && rc == 0; ++i)
                {   rc = ReportChildNode(f, indent + 1, names, node, root, i); }
                reportClose(indent, root);
            }
        }
        RELEASE(KNamelist, names);
    }
    RELEASE(KConfigNode, node);
    return rc;
}

static
rc_t ReportKrypto(const ReportFuncs *f, int indent, const KConfig* cfg)
{
    rc_t rc = 0;

    const char root[] = KFG_KRYPTO_PWFILE;
    const char name[] = "krypto";

    const KConfigNode* node = NULL;
    rc = KConfigOpenNodeRead(cfg, &node, "%s", root);
    if (rc != 0) {
        if (GetRCState(rc) == rcNotFound) {
            report(indent, name, 1, "state", 's', "pwfile: not found");
            rc = 0;
        }
        else
        {   reportErrorStr(indent, rc, "KConfigOpenNodeRead", "node", root); }
    }
    else {
        rc = ReportConfigNode_File(f, indent, "krypto", "pwfile", node, "krypto");
    }

    RELEASE(KConfigNode, node);

    return rc;
}

static rc_t ReportRemoteAccess(const ReportFuncs *f,
    int indent, const KRepositoryMgr *mgr)
{
    const char root[] = "RemoteAccess";

    bool available = KRepositoryMgrHasRemoteAccess(mgr);
    report(indent, root, 1, "available", 's', available ? "true" : "false");

    return 0;
}

static rc_t ReportCrntRepository(const ReportFuncs *f,
    int indent, const KRepositoryMgr *mgr)
{
    rc_t rc = 0;

    const KRepository *protectd = NULL;

    const char root[] = "CurrentProtectedRepository";
    bool open = false;

    if (rc == 0) {
        rc = KRepositoryMgrCurrentProtectedRepository(mgr, &protectd);
        if (rc != 0) {
            if (rc == SILENT_RC(
                rcKFG, rcMgr, rcAccessing, rcNode, rcNotFound))
            {
                report(indent, root, 1, "found", 's', "false");
            }
            else {
                reportOpen(indent, root, 0);
                open = true;
                reportErrorStr(indent + 1, rc,
                    "KRepositoryMgrCurrentProtectedRepository", NULL, NULL);
            }
        }
    }
    if (rc == 0) {
        char buffer[256];
        size_t name_size = 0;
        reportOpen(indent, root, 1, "found", 's', "true");
        open = true;
        rc = KRepositoryName(protectd, buffer, sizeof buffer, &name_size);
        if (rc != 0) {
            reportErrorStr(indent + 1, rc, "KRepositoryName",
                "origin", "KRepositoryMgrCurrentProtectedRepository");
        }
        else {
            reportData(indent + 1, "name", buffer, 0);
        }
    }
    if (open) {
        reportClose(indent, root);
    }

    RELEASE(KRepository, protectd);

    return rc;
}

rc_t ReportKfg
    ( const ReportFuncs *f, uint32_t indent, uint32_t skipCount, va_list args )
{
    rc_t rc = 0;

    KConfig *cfg = NULL;
    const KRepositoryMgr *mgr = NULL;

    const char tag[] = "Configuration";

    reportOpen(indent, tag, 0);

    rc = KConfigMake(&cfg, NULL);
    if (rc != 0) {
        reportError(indent + 1, rc, "KConfigMake");
    }
    else if ((rc = KConfigMakeRepositoryMgrRead(cfg, &mgr)) != 0) {
        reportError(indent + 1, rc, "KConfigMakeRepositoryMgrRead");
    }
    else {
        {
            rc_t rc2 = ReportKfgFiles(f, indent + 1, cfg);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = ReportRefseq(f, indent + 1, cfg);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = ReportKrypto(f, indent + 1, cfg);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = ReportSra(f, indent + 1, cfg);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = KConfigPrintPartial(cfg, indent, skipCount, args);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = ReportRemoteAccess(f, indent + 1, mgr);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
        {
            rc_t rc2 = ReportCrntRepository(f, indent + 1, mgr);
            if (rc == 0 && rc2 != 0)
            {   rc = rc2; }
        }
    }

    reportClose(indent, tag);

    RELEASE(KRepositoryMgr, mgr);
    RELEASE(KConfig, cfg);

    return rc;
}
