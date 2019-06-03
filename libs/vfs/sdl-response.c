#include <klib/json.h> /* KJsonValue */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* String */

#include "json-response.h" /* Response4MakeSdl */
#include "path-priv.h" /* VPathMakeFmt */

#include <ctype.h> /* isdigit */

static rc_t IntSet(int64_t * self, const KJsonValue * node,
    const char * name)
{
    rc_t rc = 0;

    assert(self);

    if (node == NULL)
        return 0;

    rc = KJsonGetNumber(node, self);
    if (rc != 0)
        return rc;

    /*if (THRESHOLD > THRESHOLD_INFO)
        StackPrintInt(path, name, *self);*/

    return rc;
}

static rc_t BulSet(EState * self, const KJsonValue * node,
    const char * name)
{
    rc_t rc = 0;
    bool value = false;

    assert(self);

    if (node == NULL)
        return 0;

    rc = KJsonGetBool(node, &value);
    if (rc != 0)
        return rc;

    /*if (THRESHOLD > THRESHOLD_INFO)
        StackPrintBul(path, name, value);*/

    *self = value ? eTrue : eFalse;
    return 0;
}

static rc_t StrSet(const char ** self, const KJsonValue * node,
    const char * name)
{
    rc_t rc = 0;
    const char * value = NULL;

    assert(self);

    if (node == NULL)
        return 0;

    rc = KJsonGetString(node, &value);
    if (rc != 0)
        return rc;

    if (value == NULL)
        return 0;

    //if (THRESHOLD > THRESHOLD_INFO && path != NULL)        StackPrintStr(path, name, value);

    if (value[0] == '\0')
        return 0;

    *self = value;
    return 0;
}

static void DataInit(Data * self) {
    assert(self);

    memset(self, 0, sizeof * self);

    self->qual = eUnknown;

    self->id = -1;
    self->exp = -1;
}

static void DataClone(const Data * self, Data * clone) {
    DataInit(clone);

    if (self == NULL)
        return;

    clone->acc = self->acc;
    clone->bundle = self->bundle;
    clone->id = self->id; /* oldCartObjId */
    clone->cls = self->cls; /* itemClass */
    clone->vsblt = self->vsblt;
    clone->name = self->name;
    clone->fmt = self->fmt; /* format */
    clone->qual = self->qual; /* hasOrigQuality */
    clone->sz = self->sz; /* size */
    clone->md5 = self->md5;
    clone->sha = self->sha; /* sha256 */
    clone->mod = self->mod; /* modDate */
    clone->exp = self->exp; /* expDate */
    clone->srv = self->srv; /* service */
    clone->reg = self->reg; /* region */
    clone->link = self->link; /* ???????????????????????????????????????? */
    clone->tic = self->tic;

    clone->code = self->code;
}

static rc_t DataUpdate(const Data * self,
    Data * next, const KJsonObject * node)
{
    rc_t rc = 0;

    const char * name = NULL;

    assert(next);

    DataClone(self, next);

    if (node == NULL)
        return 0;

    name = "bundle";
    StrSet(&next->acc, KJsonObjectGetMember(node, name), name);

    name = "ceRequired";
    BulSet(&next->ceRequired, KJsonObjectGetMember(node, name), name);

    name = "link";
    StrSet(&next->link, KJsonObjectGetMember(node, name), name);

    name = "bundle";
    StrSet(&next->name, KJsonObjectGetMember(node, name), name);

    name = "region";
    StrSet(&next->reg, KJsonObjectGetMember(node, name), name);

    name = "size";
    IntSet(&next->sz, KJsonObjectGetMember(node, name), name);

    name = "service";
    StrSet(&next->srv, KJsonObjectGetMember(node, name), name);

    name = "type";
    StrSet(&next->type, KJsonObjectGetMember(node, name), name);

    name = "md5";
    StrSet(&next->md5, KJsonObjectGetMember(node, name), name);

    if (next->md5 != NULL) {
        int i = 0;
        for (i = 0; i < 16; ++i) {
            if (next->md5[2 * i] == '\0')
                break;
            if (isdigit(next->md5[2 * i]))
                next->md5i[i] = (next->md5[2 * i] - '0') * 16;
            else
                next->md5i[i] = (next->md5[2 * i] - 'a' + 10) * 16;
            if (next->md5[2 * i + 1] == '\0')
                break;
            if (isdigit(next->md5[2 * i + 1]))
                next->md5i[i] += next->md5[2 * i + 1] - '0';
            else
                next->md5i[i] += next->md5[2 * i + 1] - 'a' + 10;
        }
        if (i == 16)
            next->hasMd5 = true;
    }

    name = "modificationDate";
    StrSet(&next->modificationDate, KJsonObjectGetMember(node, name), name);

    if (next->modificationDate != NULL) {
        const KTime* t = KTimeFromIso8601(&next->modT, next->modificationDate,
            string_measure(next->modificationDate, NULL));
        if (t == NULL)
            rc = RC(rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect);
        else
            next->mod = KTimeMakeTime(&next->modT);
    }

    return rc;
}

rc_t ItemAddFormat(Item * self, const char * cType, const Data * dad,
    struct Locations ** added);

/* We are scanning Item(Run) to find all its Elm-s(Files) -sra, vdbcache, ??? */
static
rc_t ItemAddElmsSdl(Item * self, const KJsonObject * node, const Data * dad)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;

    struct Locations * elm = NULL;

    Data data;
    DataUpdate(dad, &data, node);

    value = KJsonObjectGetMember(node, "link");
    if (data.type != NULL || value != NULL) {
        rc = ItemAddFormat(self, data.type, &data, &elm);
        if (elm == NULL || rc != 0) {
            return rc;
        }
    }

    value = KJsonObjectGetMember ( node, "link" );
    if (value != NULL)
    {
        Data ldata;
        DataUpdate(&data, &ldata, node);

        if (ldata.link != NULL) {
            VPath * path = NULL;

            String id;

            String url;
            StringInitCString(&url, ldata.link);

            StringInitCString(&id, ldata.acc);

            rc = VPathMakeFromUrl(&path, &url, NULL, true, &id, ldata.sz,
                ldata.mod, ldata.hasMd5 ? ldata.md5i : NULL, 0, ldata.srv);

            if (rc == 0)
                VPathMarkHighReliability(path, true);

            if (rc != 0) {
                return rc;
            }

            rc = LocationsAddVPath(elm, path, NULL, false, 0);
        }
    }

    return rc;
}

/* We are inside or above of a Container
   and are looking for Items(runs, gdGaP files) to add */
static rc_t Response4AddItems(Response4 * self, const KJsonObject * node)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;

    Container * box = NULL;
    Item * item = NULL;

    Data data;
    DataUpdate(NULL, &data, node);

    {
        const char * bundle = data.bundle;
        int64_t id = 0;
        rc = Response4AddAccOrId(self, bundle, id, &box);
        if (box == NULL || rc != 0) {
            return rc;
        }

        rc = ContainerAdd(box, bundle, id, &item, NULL);
        if (item == NULL || rc != 0) {
            return rc;
        }
    }

    {
        const char * name = "files";
        value = KJsonObjectGetMember(node, name);
        if (value != NULL) {
            uint32_t i = 0;
            const KJsonArray * array = KJsonValueToArray(value);
            uint32_t n = KJsonArrayGetLength(array);
            for (i = 0; i < n; ++i) {
                rc_t r2 = 0;

                const KJsonObject * object = NULL;

                value = KJsonArrayGetElement(array, i);
                object = KJsonValueToObject(value);
                r2 = ItemAddElmsSdl(item, object, &data);
                if (r2 != 0 && rc == 0)
                    rc = r2;
            }
        }
    }

    return rc;
}

/* Add response document */
static rc_t ResponseSdlInit(Response4 * self, const char * input) {
    rc_t rc = 0;

    KJsonValue * root = NULL;
    const KJsonObject * object = NULL;
    const KJsonValue * value = NULL;
    char error[99] = "";

    const char name[] = "result";

    rc = KJsonValueMake(&root, input, error, sizeof error);
    if (rc != 0) {
        return rc;
    }

    object = KJsonValueToObject(root);
    value = KJsonObjectGetMember(object, name);
    if (value == NULL) {
        rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
        KJsonValueWhack(root);
    }
    else {
        const KJsonArray * array = KJsonValueToArray(value);
        if (array == NULL)
            rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcInvalid);
        else {
            uint32_t n = KJsonArrayGetLength(array);
            if (n == 0) {
                rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
            }
            else {
                uint32_t i = 0;
                for (i = 0; i < n; ++i) {
                    rc_t r2 = 0;
                    value = KJsonArrayGetElement(array, i);
                    object = KJsonValueToObject(value);
                    r2 = Response4AddItems(self, object);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
            }
        }
    }

    return rc;
}

rc_t Response4MakeSdl(Response4 ** self, const char * input) {
    rc_t rc = 0;

    Response4 * r = NULL;

    assert(self);

    rc = Response4MakeEmpty(&r);
    if (rc != 0)
        return rc;

    rc = ResponseSdlInit(r, input);
    if (rc != 0)
        free(r);
    else
        * self = r;

    return rc;
}
