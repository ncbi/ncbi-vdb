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
* =========================================================================== */

#include "path-priv.h" /* VPath */
#include "SraDesc.h" /* SraDescConvert */

#include <kfs/directory.h> /* KDirectoryNativeDir */
#include <kfs/file.h> /* KFileRelease */

#include <klib/data-buffer.h> /* KDataBuffer */
#include <klib/log.h> /* PLOGERR */
#include <klib/out.h> /* OUTMSG */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */
#include <klib/status.h> /* STSMSG */
#include <klib/text.h> /* string_cmp */

#include <vdb/quality.h> /* VQuality */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (0)

#define STS_DBG  2

typedef enum {
    eBad,
    eBin,
    eTextual,
} EType;

typedef struct {
    VQuality _quality;
    size_t   _size;
    EType    _sdType;
} SraDesc;

void SraDescSet(SraDesc * self, VQuality q, size_t s, EType t) {
    assert(self);
    memset(self, 0, sizeof *self);

    self->_quality = q;
    self->_size = s;
    self->_sdType = t;
}

rc_t SraDescCmp(const SraDesc * self, const SraDesc * self2) {
    rc_t rc = 0;

    assert(self && self2);

    if (rc == 0 && self->_quality != self2->_quality)
        rc = 1;

    if (rc == 0 && self->_size != self2->_size)
        rc = 2;

    if (rc == 0 && self->_sdType != self2->_sdType)
        rc = 3;

    return rc;
}

#define MAGIC "NCBIRrDs"
static rc_t StringRelease(const String * self) {
    free((String*)self);

    return 0;
}

static rc_t SraDescLoadBin(SraDesc * self,
    const char * in, uint64_t size)
{
    uint64_t from = 8;
    uint8_t q = 0;
    union {
        uint64_t u;
        uint8_t b[8];
    } n;

    assert(self);
    memset(self, 0, sizeof *self);
    self->_quality = eQualLast;

    if (size < sizeof MAGIC - 1)
        return RC(rcExe, rcFile, rcReading, rcFile, rcInsufficient);

    if (string_cmp((char*)in, sizeof MAGIC - 1, MAGIC, sizeof MAGIC - 1,
        sizeof MAGIC - 1) != 0)
    {
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
    }

    q = in[8];
    q &= 3;
    if (q < 1 || q > 3)
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
    switch (q) {
    case 1:
        self->_quality = eQualNo;
        break;
    case 2:
        self->_quality = eQualFull;
        break;
    case 3:
        self->_quality = eQualDefault;
        break;
    default:
        assert(0);
    }
    for (from = 9; from < size;) {
        int i = 0;

        for (i = 0, n.u = 0; i < 8 && from < size; ++i, ++from)
            n.b[i] = in[from];
    }

    self->_size = n.u;

    return 0;
}

static rc_t SraDescLoadText(SraDesc * self,
    const char * in, uint64_t size)
{
    uint64_t from = 0;
    char q = 0;
    uint64_t u = 0;

    assert(self);
    memset(self, 0, sizeof *self);
    self->_quality = eQualLast;

    if (size < sizeof MAGIC - 1)
        return RC(rcExe, rcFile, rcReading, rcFile, rcInsufficient);

    if (string_cmp(in, sizeof MAGIC - 1, MAGIC, sizeof MAGIC - 1,
        sizeof MAGIC - 1) != 0)
    {
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
    }
    from += sizeof MAGIC - 1;

    if (in[from++] != '\n')
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);

    for (; from < size; ++from) {
        q = in[from];
        if (q == '\n')
            break;
        if (q < '0' || q > '9')
            return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
        u = u * 10 + q - '0';
    }
    switch (u & 3) {
    case 1:
        self->_quality = eQualNo;
        break;
    case 2:
        self->_quality = eQualFull;
        break;
    case 3:
        self->_quality = eQualDefault;
        break;
    default:
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
    }

    if (in[from++] != '\n')
        return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);

    for (; from < size; ++from) {
        q = in[from];
        if (q == '\n')
            break;
        if (q < '0' || q > '9')
            return RC(rcExe, rcFile, rcReading, rcData, rcInvalid);
        self->_size = self->_size * 10 + q - '0';
    }

    return 0;
}

static rc_t SraDescLoadPath(SraDesc * self,
    const KDirectory * dir, const char * path)
{
    rc_t rc = 0;
    const KFile * in = NULL;
    uint64_t fsize = 0;

    KDataBuffer buf;
    memset(&buf, 0, sizeof buf);
    buf.elem_bits = 8;

    if (rc == 0)
        rc = KDirectoryOpenFileRead(dir, &in, "%s", path);

    if (rc == 0) {
#ifdef DEBUGGING
        OUTMSG(("%s: %s found: loading...\n", __FUNCTION__, path));
#endif
        STSMSG(STS_DBG, ("loading %s", path));
        rc = KFileSize(in, &fsize);
        if (rc != 0)
            PLOGERR(klogInt, (klogInt, rc, "Cannot Size($(path))",
                "path=%s", path));
    }

    if (rc == 0)
        rc = KDataBufferResize(&buf, fsize);

    if (rc == 0) {
        rc = KFileReadExactly(in, 0, buf.base, fsize);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc, "Cannot Read($(path))",
                "path=%s", path));
            return rc;
        }
    }

    RELEASE(KFile, in);
    if (rc != 0)
        return rc;

    assert(self);

    switch (self->_sdType) {
    case eBin:
        rc = SraDescLoadBin(self, (char*)buf.base, fsize);
        self->_sdType = eBin;
        break;
    case eTextual:
        rc = SraDescLoadText(self, (char*)buf.base, fsize);
        self->_sdType = eTextual;
        break;
    default:assert(0); return 1;
    }

    {
        rc_t r2 = KDataBufferWhack(&buf);
        return rc == 0 ? r2 : rc;
    }
}

static rc_t SraDescSaveText(const SraDesc * self,
    char * buffer, size_t size, size_t * num_writ)
{
    rc_t rc = 0;

    size_t nw = 0;
    uint64_t to = 0;

    if (rc == 0)
        rc = string_printf(buffer + to, size, &nw, MAGIC, sizeof MAGIC - 1);
    to += nw;

    if (rc == 0 && to < size)
        buffer[to++] = '\n';

    assert(self && num_writ);

    if (rc == 0 && to < size) switch (self->_quality) {
    case eQualNo: buffer[to] = '1'; break;
    case eQualFull: buffer[to] = '2'; break;
    case eQualDefault: buffer[to] = '3'; break;
    default: assert(0); return RC(rcExe, rcFile, rcWriting, rcData, rcInvalid);
    }
    ++to;

    if (rc == 0 && to < size)
        buffer[to++] = '\n';

    if (rc == 0 && to < size)
        rc = string_printf(buffer + to, size - to, &nw, "%lu\n", self->_size);
    to += nw;

    *num_writ = to;
    return rc;
}

static rc_t SraDescSaveBin(const SraDesc * self,
    char * buffer, size_t size, size_t * num_writ)
{
    rc_t rc = 0;

    size_t nw = 0;
    uint64_t to = 0;

    if (rc == 0)
        rc = string_printf(buffer + to, size, &nw, MAGIC, sizeof MAGIC - 1);
    to += nw;

    assert(self && num_writ);

    if (rc == 0 && to < size) switch (self->_quality) {
    case eQualNo: buffer[to] = 1; break;
    case eQualFull: buffer[to] = 2; break;
    case eQualDefault: buffer[to] = 3; break;
    default: assert(0); return RC(rcExe, rcFile, rcWriting, rcData, rcInvalid);
    }
    ++to;

    if (rc == 0 && to < size)
        memmove(buffer + to, &self->_size, sizeof self->_size);
    to += sizeof self->_size;

    *num_writ = to;
    return rc;
}

static rc_t SraDescSave(const SraDesc * self, KFile * out) {
    rc_t rc = 0;

    char buffer[512] = "";
    size_t num_writ = 0;

    assert(self);

    switch (self->_sdType) {
    case eBin:
        rc = SraDescSaveBin(self, buffer, sizeof buffer, &num_writ);
        break;
    case eTextual:
        rc = SraDescSaveText(self, buffer, sizeof buffer, &num_writ);
        break;
    default:
        assert(0);
        break;
    }

    if (rc == 0)
        rc = KFileWrite(out, 0, buffer, num_writ, NULL);

    return rc;
}

static bool KDirectory_Exist(const KDirectory * self,
    const String * name, const char * sfx)
{
    assert(name && sfx);

    KPathType type = KDirectoryPathType(
        self, "%.*s%s", name->size, name->addr, sfx);
    if (type == kptNotFound)
        return false;
    else
        return true;
}

static rc_t Convert(KDirectory * dir,
    const String * from, bool fromBin)
{
    rc_t rc = 0;

    KFile * out = NULL;
    const String * to = NULL;

    SraDesc sd;
    KDataBuffer buf;
    memset(&sd, 0, sizeof sd);
    memset(&buf, 0, sizeof buf);
    buf.elem_bits = 8;

    rc = StringCopy(&to, from);

    if (rc == 0) {
        assert(to);
        if (fromBin)
            ((char*)to->addr)[to->size - 1] = 't';
        else
            ((char*)to->addr)[to->size - 1] = 'c';
        OUTMSG(("%S -> %S\n", from, to));
    }

    sd._sdType = fromBin ? eBin : eTextual;

    if (rc == 0)
        rc = SraDescLoadPath(&sd, dir, from->addr);

    if (rc == 0 && KDirectory_Exist(dir, to, ""))
        rc = KDirectoryRemove(dir, false, "%s", to->addr);

    if (rc == 0)
        rc = KDirectoryCreateFile(dir, &out, false,
            0664, kcmInit | kcmParents, "%s", to->addr);

    sd._sdType = fromBin ? eTextual : eBin;
    if (rc == 0)
        rc = SraDescSave(&sd, out);

    RELEASE(KFile, out);
    RELEASE(String, to);

    {
        rc_t r2 = KDataBufferWhack(&buf);
        return rc == 0 ? r2 : rc;
    }

    return rc;
}

#define EXT_1   ".ds"
#define EXT_BIN ".dsc"
#define EXT_TXT ".dst"

rc_t SraDescConvert(KDirectory * dir, const char * path,
    bool * recognized)
{
    assert(recognized);
    *recognized = false;

    if (path == NULL)
        return 0;

    else {
        String s;
        StringInitCString(&s, path);
        if (s.size < sizeof EXT_BIN)
            return 0;

        if (string_cmp(s.addr + s.size - sizeof EXT_1, sizeof EXT_1 - 1,
            EXT_1, sizeof EXT_1 - 1, sizeof EXT_1 - 1) != 0)
        {
            return 0;
        }

        switch (s.addr[s.size - 1]) {
        case 'c':
            *recognized = true;
            return Convert(dir, &s, true);
        case 't':
            *recognized = true;
            return Convert(dir, &s, false);
        default:
            return 0;
        }
        return 0;
    }
}

static const char * SDExt(const SraDesc * self) {
    switch (self->_sdType) {
    case eTextual:
        return EXT_TXT;
    case eBin:
        return EXT_BIN;
    default:
        assert(0);
        return "";
    }
}

rc_t SraDescSaveQuality(const String * sra, VQuality quality) {
    rc_t rc = 0;

    KDirectory * dir = NULL;
    const KFile * in = NULL;
    KFile * out = NULL;
    uint64_t size = 0;
    SraDesc sd;

    assert(sra);

    if (rc == 0)
        rc = KDirectoryNativeDir(&dir);

    if (rc == 0)
        rc = KDirectoryOpenFileRead(dir, &in, "%.*s", sra->size, sra->addr);

    if (rc == 0)
        rc = KFileSize(in, &size);

    SraDescSet(&sd, quality, size, eBin);

    if (rc == 0)
        rc = KDirectoryCreateFile(dir, &out, false, 0664,
            kcmInit | kcmParents, "%.*s%s", sra->size, sra->addr, SDExt(&sd));

    if (rc == 0)
        SraDescSave(&sd, out);

    RELEASE(KFile, in);
    RELEASE(KFile, out);
    RELEASE(KDirectory, dir);

    return rc;
}

rc_t SraDescLoadQuality(const String * sra, VQuality * quality) {
    rc_t rc = 0;

    char path[PATH_MAX] = "";
    KDirectory * dir = NULL;
    const KFile * in = NULL;
    uint64_t fsize = 0;

    SraDesc sd;
    memset(&sd, 0, sizeof sd);

    assert(sra && quality);

    *quality = eQualLast;

    if (rc == 0)
        rc = KDirectoryNativeDir(&dir);

    if (rc == 0)
        rc = KDirectoryOpenFileRead(dir, &in, "%.*s", sra->size, sra->addr);

    if (rc == 0)
        rc = KFileSize(in, &fsize);

    RELEASE(KFile, in);

    if (rc == 0) {
        sd._sdType = eBin;
        if (!KDirectory_Exist(dir, sra, SDExt(&sd))) {
            sd._sdType = eTextual;
            if (!KDirectory_Exist(dir, sra, SDExt(&sd)))
                sd._sdType = eBad;
        }

        if (sd._sdType == eBad)
            *quality = eQualLast;
        else {
            rc = string_printf(path, sizeof path, NULL, "%S%s",
                sra, SDExt(&sd));
            STSMSG(STS_DBG, ("opening %s", path));
            if (rc == 0)
                rc = SraDescLoadPath(&sd, dir, path);
            if (rc == 0 && sd._size == fsize)
                *quality = sd._quality;
        }
    }

    RELEASE(KDirectory, dir);

    return rc;
}

rc_t VPathSaveQuality(const VPath * self) {
    String sraPath;
    rc_t rc = VPathGetPath(self, &sraPath);

    VQuality q = VPathGetQuality(self);

    if (rc == 0)
        rc = SraDescSaveQuality(&sraPath, q);

    return rc;
}

rc_t VPathLoadQuality(VPath * self) {
    rc_t rc = 0;

    String sraPath;
    VQuality q = eQualLast;

    assert(self);

    if (rc == 0)
        rc = VPathSetQuality(self, eQualLast);

    if (rc == 0)
        rc = VPathGetPath(self, &sraPath);

    if (rc == 0)
        rc = SraDescLoadQuality(&sraPath, &q);

    if (rc == 0)
        rc = VPathSetQuality(self, q);

    return rc;
}
