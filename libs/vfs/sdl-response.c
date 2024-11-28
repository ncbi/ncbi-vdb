/*==============================================================================
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
* =========================================================================== */

#include <klib/debug.h> /* DBGMSG */
#include <klib/json.h> /* KJsonValue */
#include <klib/log.h> /* PLOGERR */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* String */

#include "json-response.h" /* Response4MakeSdl */
#include "path-priv.h" /* VPathMakeFmt */

#include <ctype.h> /* isdigit */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (0)

static void DataInit(Data * self) {
    assert(self);

    memset(self, 0, sizeof * self);

    self->qual = eUnknown;

    self->id = -1;
    self->exp = -1;
    self->encryptedForProjectId = -1;
    self->quality = eQualLast;
}

static void DataClone(const Data * self, Data * clone) {
    DataInit(clone);

    if (self == NULL)
        return;

    clone->acc = self->acc;
    clone->accession = self->accession;
    clone->bundle = self->bundle;
    clone->ceRequired = self->ceRequired;
    clone->cls = self->cls; /* itemClass */
    clone->code = self->code;
    clone->encryptedForProjectId = self->encryptedForProjectId;
    clone->exp = self->exp; /* expDate */
    clone->expirationDate = self->expirationDate;
    clone->fmt = self->fmt; /* format */
    clone->id = self->id; /* oldCartObjId */
    clone->link = self->link; /* ???????????????????????????????????????? */
    clone->md5 = self->md5;
    clone->mod = self->mod; /* modDate */
    clone->modificationDate = self->modificationDate;
    clone->name = self->name;
    clone->noqual = self->noqual;
    clone->object = self->object;
    clone->payRequired = self->payRequired;
    clone->qual = self->qual; /* hasOrigQuality */
    clone->reg = self->reg; /* region */
    clone->sha = self->sha; /* sha256 */
    clone->srv = self->srv; /* service */
    clone->sz = self->sz; /* size */
    clone->tic = self->tic;
    clone->type = self->type;
    clone->vsblt = self->vsblt;
    clone->quality = self->quality;
}

static rc_t DataUpdate(const Data * self,
    Data * next, const KJsonObject * node, JsonStack * path)
{
    const char * name = NULL;

    assert(next);

    DataClone(self, next);

    if (node == NULL)
        return 0;

    name = "accession";
    StrSet(&next->accession, KJsonObjectGetMember(node, name), name, path);

    name = "bundle";
    StrSet(&next->acc, KJsonObjectGetMember(node, name), name, path);
    StrSet(&next->bundle, KJsonObjectGetMember(node, name), name, path);

    name = "ceRequired";
    BulSet(&next->ceRequired, KJsonObjectGetMember(node, name), name, path);

    name = "encryptedForProjectId";
    StrSet(&next->sEncryptedForProjectId, KJsonObjectGetMember(node, name),
        name, path);
    if (next->sEncryptedForProjectId != NULL)
        next->encryptedForProjectId = atoi(next->sEncryptedForProjectId);

    name = "expirationDate";
    StrSet(&next->expirationDate,
        KJsonObjectGetMember(node, name), name, path);

    name = "link";
    StrSet(&next->link, KJsonObjectGetMember(node, name), name, path);

    name = "md5";
    StrSet(&next->md5, KJsonObjectGetMember(node, name), name, path);

    name = "modificationDate";
    StrSet(&next->modificationDate,
        KJsonObjectGetMember(node, name), name, path);

    name = "msg";
    StrSet(&next->msg, KJsonObjectGetMember(node, name), name, path);

    name = "name";
    StrSet(&next->name, KJsonObjectGetMember(node, name), name, path);

    name = "noqual";
    BulSet(&next->noqual, KJsonObjectGetMember(node, name), name, path);

    switch (next->noqual) {
    case eTrue:
        next->quality = eQualNo; /* eQualFull; */
    default:
        break;
    }

    name = "object";
    StrSet(&next->object, KJsonObjectGetMember(node, name), name, path);

    name = "region";
    StrSet(&next->reg, KJsonObjectGetMember(node, name), name, path);

    name = "payRequired";
    BulSet(&next->payRequired, KJsonObjectGetMember(node, name), name, path);

    if ( ! next -> payRequired ) {
        name = "paymentRequired";
        BulSet(&next->payRequired, KJsonObjectGetMember(node, name), name,
            path);
    }
    name = "service";
    StrSet(&next->srv, KJsonObjectGetMember(node, name), name, path);

    name = "size";
    IntSet(&next->sz, KJsonObjectGetMember(node, name), name, path);

    name = "status";
    IntSet(&next->code, KJsonObjectGetMember(node, name), name, path);

    name = "type";
    StrSet(&next->type, KJsonObjectGetMember(node, name), name, path);

    return 0;
}

static rc_t VPath_SetQuality(VPath * self, const Data * data) {
    VQuality q = eQualFull;

    assert(data);
    if (data->quality < eQualLast)
        q = data->quality;

    return VPathSetQuality(self, q);
}

/* We are adding a location to file */
static
rc_t FileAddSdlLocation(struct File * file, const KJsonObject * node,
    const Data * dad, JsonStack * path, int64_t aProjectId)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;

    const char * name = "locations";

    Data data;
    DataUpdate(dad, &data, node, path);

    value = KJsonObjectGetMember(node, name);
    assert(!value);

    value = KJsonObjectGetMember(node, "link");
    if (value != NULL) {
        Data ldata;
        DataUpdate(&data, &ldata, node, path);

        if (ldata.link != NULL) {
            bool ceRequired = false;
            bool payRequired = false;

            int64_t exp = 0;  /* expirationDate */
            int64_t mod = 0;  /* modDate */

            int64_t projectId = -1;

            uint8_t md5[16];
            bool    hasMd5 = false;

            VPath * path = NULL;

            String acc;
            String id;
            String objectType;
            String type;

            String url;
            StringInitCString(&url, ldata.link);

            StringInitCString(&id, ldata.acc);

            memset(&acc, 0, sizeof acc);
            memset(&objectType, 0, sizeof objectType);
            memset(&type, 0, sizeof type);

            if (ldata.expirationDate != NULL) {
                KTime        kt;
                const KTime* t = KTimeFromIso8601(&kt, ldata.expirationDate,
                    string_measure(ldata.expirationDate, NULL));
                if (t == NULL)
                    return RC(rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect);
                else
                    exp = KTimeMakeTime(&kt);
            }

            if (ldata.modificationDate != NULL) {
                KTime        modT; /* modificationDate */
                const KTime* t = KTimeFromIso8601(&modT, ldata.modificationDate,
                    string_measure(ldata.modificationDate, NULL));
                if (t == NULL)
                    return RC(rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect);
                else
                    mod = KTimeMakeTime(&modT);
            }

            if (ldata.object != NULL) {
                size_t size = 0;
                uint32_t len = 0;
                const char * c = strchr(ldata.object, '|');
                if (c != NULL)
                    size = len = c - data.object;
                else
                    len = string_measure(data.object, &size);;
                StringInit(&objectType, ldata.object, size, len);

                if (c != NULL) {
                    size = len = string_measure(data.object, NULL) - len;
                    if (len > 0)
                        StringInit(&acc, c + 1, size - 1, len - 1);
                }
            }

            if (ldata.accession != NULL)
                StringInitCString(&acc, ldata.accession);

            if (ldata.type != NULL) {
                size_t size = 0;
                uint32_t len = string_measure(data.type, &size);;
                StringInit(&type, ldata.type, size, len);
            }

            if (ldata.ceRequired == eTrue)
                ceRequired = true;
            if (ldata.payRequired == eTrue)
                payRequired = true;

            projectId = ldata.encryptedForProjectId;
            if (aProjectId >= 0 && projectId >= 0 &&
                aProjectId != projectId)
            {
                rc = RC(rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect);
                PLOGERR(klogInt, (klogInt, rc,
                    "'$(name)' was encrypted for project 'dbGaP-$(id)'. "
                    "Please contact sra-tools@ncbi.nlm.nih.gov for details.",
                    "name=%s,id=%lu", data.name, projectId));
                return rc;
            }

            if (ldata.md5 != NULL) {
                int i = 0;
                for (i = 0; i < 16; ++i) {
                    if (ldata.md5[2 * i] == '\0')
                        break;
                    if (isdigit(ldata.md5[2 * i]))
                        md5[i] = (ldata.md5[2 * i] - '0') * 16;
                    else
                        md5[i] = (ldata.md5[2 * i] - 'a' + 10) * 16;
                    if (ldata.md5[2 * i + 1] == '\0')
                        break;
                    if (isdigit(ldata.md5[2 * i + 1]))
                        md5[i] += ldata.md5[2 * i + 1] - '0';
                    else
                        md5[i] += ldata.md5[2 * i + 1] - 'a' + 10;
                }
                if (i == 16)
                    hasMd5 = true;
            }

            rc = VPathMakeFromUrl(&path, &url, NULL, true, &id, ldata.sz,
                mod, hasMd5 ? md5 : NULL, exp, ldata.srv, &objectType, &type,
                ceRequired, payRequired, ldata.name, projectId, 128, &acc);

            if (rc == 0)
                rc = VPath_SetQuality(path, &data);

            if (rc == 0)
                VPathMarkHighReliability(path, true);

            if (rc != 0)
                return rc;

            rc = FileAddVPath(file, path, NULL, false, 0);

            RELEASE(VPath, path);

            if (rc == 0)
                FileLogAddedLink(file, ldata.link);
        }
    }

    return rc;
}

/* We are scanning files(Item(Run)) to find all its locations */
static
rc_t ItemAddSdlFile(Item * self, const KJsonObject * node,
    const Data * dad, JsonStack * path, int64_t projectId)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;

    struct File * file = NULL;

    const char * name = "locations";

    Data data;
    DataUpdate(dad, &data, node, path);

    rc = ItemAddFormat(self, data.type, &data, &file, false);
    if (file == NULL || rc != 0)
        return rc;
    else
        if (THRESHOLD > THRESHOLD_ERROR)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("Adding links to a file...\n"));

    value = KJsonObjectGetMember ( node, name );

    if ( value != NULL ) {
        uint32_t i = 0;

        const KJsonArray * array = KJsonValueToArray ( value );
        uint32_t n = KJsonArrayGetLength ( array );
        rc = JsonStackPushArr(path, name);
        if (rc != 0)
            return rc;
        for ( i = 0; i < n; ++ i ) {
            rc_t r2 = 0;

            const KJsonObject * object = NULL;

            value = KJsonArrayGetElement ( array, i );
            object = KJsonValueToObject ( value );
            r2 = FileAddSdlLocation ( file, object, & data, path, projectId );
            if ( r2 != 0 && rc == 0 )
                rc = r2;

            if ( i + 1 < n )
                JsonStackArrNext ( path );
        }

        JsonStackPop(path);
    }

    value = KJsonObjectGetMember(node, "link");
    if (value != NULL) {
        rc = FileAddSdlLocation(file, node, &data, path, projectId);
        /*rc = ItemAddFormat(self, data.type, &data, &file, false);
        if (file == NULL || rc != 0)
            return rc;
        else
            if (THRESHOLD > THRESHOLD_ERROR)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                ("Adding links to a file...\n"));*/
    }

#if 0
    value = KJsonObjectGetMember ( node, "link" );
    if (value != NULL)
    {
        Data ldata;
        DataUpdate(&data, &ldata, node, path);

        assert(0);

        if (ldata.link != NULL) {
            bool ceRequired = false;
            bool payRequired = false;

            int64_t mod = 0;  /* modDate */

            uint8_t md5[16];
            bool    hasMd5 = false;

            VPath * path = NULL;

            String id;
            String objectType;
            String type;

            String url;
            StringInitCString(&url, ldata.link);

            StringInitCString(&id, ldata.acc);

            memset(&objectType, 0, sizeof objectType);
            memset(&type, 0, sizeof type);

            if (ldata.modificationDate != NULL) {
                KTime        modT; /* modificationDate */
                const KTime* t = KTimeFromIso8601(&modT, ldata.modificationDate,
                    string_measure(ldata.modificationDate, NULL));
                if (t == NULL)
                    return RC(rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect);
                else
                    mod = KTimeMakeTime(&modT);
            }

            if (ldata.object != NULL) {
                size_t size = 0;
                uint32_t len = 0;
                const char * c = strchr(ldata.object, '|');
                if (c != NULL)
                    size = len = c - data.object;
                else
                    len = string_measure(data.object, &size);;
                StringInit(&objectType, ldata.object, size, len);
            }

            if (ldata.type != NULL) {
                size_t size = 0;
                uint32_t len = string_measure(data.type, &size);;
                StringInit(&type, ldata.type, size, len);
            }

            if (ldata.ceRequired == eTrue)
                ceRequired = true;
            if (ldata.payRequired == eTrue)
                payRequired = true;

            if (ldata.md5 != NULL) {
                int i = 0;
                for (i = 0; i < 16; ++i) {
                    if (ldata.md5[2 * i] == '\0')
                        break;
                    if (isdigit(ldata.md5[2 * i]))
                        md5[i] = (ldata.md5[2 * i] - '0') * 16;
                    else
                        md5[i] = (ldata.md5[2 * i] - 'a' + 10) * 16;
                    if (ldata.md5[2 * i + 1] == '\0')
                        break;
                    if (isdigit(ldata.md5[2 * i + 1]))
                        md5[i] += ldata.md5[2 * i + 1] - '0';
                    else
                        md5[i] += ldata.md5[2 * i + 1] - 'a' + 10;
                }
                if (i == 16)
                    hasMd5 = true;
            }

            rc = VPathMakeFromUrl(&path, &url, NULL, true, &id, ldata.sz,
                mod, hasMd5 ? md5 : NULL, 0, ldata.srv, &objectType, &type,
                ceRequired, payRequired, ldata.name);

            if (rc == 0)
                VPathMarkHighReliability(path, true);

            if (rc != 0) {
                return rc;
            }

            rc = FileAddVPath(file, path, NULL, false, 0);

            RELEASE(VPath, path);

            if (rc == 0)
                FileLogAddedLink(file, ldata.link);
        }
    }
#endif

    if (rc == 0)
        rc = ItemInitMapping(self);

    return rc;
}

/* We are inside or above of a Container
   and are looking for Items(runs, gdGaP files) to add */
static rc_t Response4AddItemsSdl(Response4 * self,
    const KJsonObject * node, JsonStack * path, const char * phid)
{
    rc_t rc = 0;

    Container * box = NULL;
    Item * item = NULL;

    const KJsonValue * value = NULL;

    Data data;
    DataUpdate(NULL, &data, node, path);

    {
        const char * bundle = data.bundle;
        int64_t id = 0;
        rc = Response4AddAccOrId(self, bundle, id, &box);
        if (box == NULL || rc != 0) {
            if (THRESHOLD > THRESHOLD_NO_DEBUG)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                ("... error: cannot find any 'acc' or 'id'\n"));
            return rc;
        }

        rc = ContainerStatusInit(box, data.code, data.msg);

        data.phid = phid;

        if (rc == 0)
            ContainerProcessStatus(box, &data);

        rc = ContainerAdd(box, bundle, id, &item, NULL);
        if (item == NULL || rc != 0)
            return rc;
        else
            ItemLogAdd(item);
    }

    {
        const char * name = "files";
        value = KJsonObjectGetMember(node, name);
        if (value != NULL) {
            uint32_t i = 0;

            int64_t projectId = Response4GetProjectId(self);

            const KJsonArray * array = KJsonValueToArray(value);
            uint32_t n = KJsonArrayGetLength(array);
            rc = JsonStackPushArr(path, name);
            if (rc != 0)
                return rc;
            for (i = 0; i < n; ++i) {
                rc_t r2 = 0;

                const KJsonObject * object = NULL;

                value = KJsonArrayGetElement(array, i);
                object = KJsonValueToObject(value);
                r2 = ItemAddSdlFile(item, object, &data, path, projectId);
                if (r2 != 0 && rc == 0)
                    rc = r2;

                if (i + 1 < n)
                    JsonStackArrNext(path);
            }

            JsonStackPop(path);
        }
    }

    if (ContainerIs200AndEmpty(box)) {
        rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("... error: cannot find any container\n"));
    }

    return rc;
}

/* Add response document */
static
rc_t Response4InitSdl(Response4 * self, const char * input, const char * phid)
{
    rc_t rc = 0;

    JsonStack path;

    KJsonValue * root = NULL;
    const KJsonObject * object = NULL;
    const KJsonValue * value = NULL;
    char error[99] = "";

    const char * name = "status";

    JsonStackPrintInput(input);

    rc = KJsonValueMake(&root, input, error, sizeof error);
    if (rc != 0) {
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("... error: invalid JSON\n"));
        return rc;
    }

    rc = JsonStackInit(&path);
    if (rc != 0)
    {
        KJsonValueWhack(root);
        return rc;
    }

    object = KJsonValueToObject(root);

    name = "status";
    value = KJsonObjectGetMember(object, name);
    if (value != NULL) {
        const char * message = NULL;
        bool ok = true;

        int64_t status = 0;
        rc = KJsonGetNumber(value, &status);
        if (rc != 0) {
            if (THRESHOLD > THRESHOLD_NO_DEBUG)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), (
                    "... error: cannot get '%s'\n", name));
            JsonStackRelease(&path, rc != 0);
            KJsonValueWhack(root);
            return rc;
        }
        if (THRESHOLD > THRESHOLD_ERROR)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), (
                "\"/%s\" = %ld\n", name, status));

        name = "msg";
        value = KJsonObjectGetMember(object, name);

        if (value == NULL) {
            name = "message";
            value = KJsonObjectGetMember(object, name);
        }

        if (value != NULL) {
            rc = KJsonGetString(value, &message);
            if (rc != 0) {
                if (THRESHOLD > THRESHOLD_NO_DEBUG)
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), (
                        "... error: cannot get '%s'\n", name));
                JsonStackRelease(&path, rc != 0);
                KJsonValueWhack(root);
                return rc;
            }
            if (THRESHOLD > THRESHOLD_ERROR)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), (
                    "\"/%s\" = \"%s\"\n", name, message));
        }

        if (status != 200)
            ok = false;
        else if (message != NULL
            && (message[0] != 'o' || message[1] != 'k' || message[2] != '\0'))
        {
            ok = false;
        }

        if (!ok) {
            rc_t r = rc;
            if (message == NULL)
                message = "External service returned an error";
            rc = Response4StatusInit(self, status, message, true);
            if (rc == 0) {
                rc = Response4GetRc(self, &r);
                if (rc != 0)
                    rc = r;
            }
            PLOGERR(klogErr, (klogErr, r, "$(msg) ( $(code) )",
                "msg=%s,code=%lu", message, status));
            JsonStackRelease(&path, 1);
            KJsonValueWhack(root);
            return rc;
        }
    }

    name = "NCBI-PHID";
    value = KJsonObjectGetMember(object, name);
    if (value != NULL && phid == NULL)
        rc = StrSet(&phid, value, name, &path);

    name = "result";
    value = KJsonObjectGetMember(object, name);
    if (value == NULL) {
        rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("... error: cannot find '%s'\n", name));
    }
    else {
        const KJsonArray * array = KJsonValueToArray(value);
        if (array == NULL)
            rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcInvalid);
        else {
            uint32_t n = KJsonArrayGetLength(array);

            rc = JsonStackPushArr(&path, name);
            if (rc != 0)
            {
                JsonStackRelease(&path, 1);
                KJsonValueWhack(root);
                return rc;
            }

            if (n == 0) {
                rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
                if (THRESHOLD > THRESHOLD_NO_DEBUG)
                    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), (
                        "... error: '%s' is empty\n", name));
            }
            else {
                uint32_t i = 0;
                for (i = 0; i < n; ++i) {
                    rc_t r2 = 0;
                    value = KJsonArrayGetElement(array, i);
                    const KJsonObject * object = KJsonValueToObject(value);
                    r2 = Response4AddItemsSdl(self, object, &path, phid);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                    if (i + 1 < n)
                        JsonStackArrNext(&path);
                }
            }
            JsonStackPop(&path);
        }
    }

    {
        const char name[] = "nextToken";
        value = KJsonObjectGetMember(object, name);
        if (value != NULL) {
            const char * nextToken = NULL;
            rc = StrSet(&nextToken, value, name, &path);
            if (rc == 0)
                rc = Response4SetNextToken(self, nextToken);
        }
    }

    KJsonValueWhack(root);

    if (rc != 0)
        Response4Fini(self);

    {
        rc_t r2 = JsonStackRelease(&path, rc != 0);
        if (r2 != 0 && rc == 0)
            rc = r2;
    }

    return rc;
}

rc_t Response4MakeSdlExt(Response4 ** self, const struct VFSManager * vfs,
    const struct KNSManager * kns, const struct KConfig * kfg,
    const char * input, bool logNamesServiceErrors,
    int64_t projectId, const char * quality, const char * phid)
{
    rc_t rc = 0;

    Response4 * r = NULL;

    assert(self);

    if (*self == NULL) {
        rc = Response4MakeEmpty(&r, vfs, kns, kfg,
            logNamesServiceErrors, projectId, quality);
        if (rc != 0)
            return rc;
    }
    else
        r = *self;

    rc = Response4InitSdl(r, input, phid);

/*  if (rc == SILENT_RC(rcCont, rcNode, rcParsing, rcFormat, rcUnrecognized))
        PLOGERR(klogInt, (klogInt, rc,
            "Received '$(json)'.", "json=%s", input)); */

    if (*self == NULL) {
        if (rc != 0)
            free(r);
        else
            * self = r;
    }

    return rc;
}

rc_t Response4MakeSdl(Response4 ** self, const char * input) {
    return Response4MakeSdlExt(self, NULL, NULL, NULL,
        input, false, -1, 0, NULL);
}
