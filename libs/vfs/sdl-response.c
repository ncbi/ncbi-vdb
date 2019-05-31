#include <klib/json.h> /* KJsonValue */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* String */

#include "json-response.h" /* Response4MakeSdl */
#include "path-priv.h" /* VPathMakeFmt */

typedef struct Data {
    const char * acc;
    const char * bundle;
    const char * type;
    const char * link;
} Data;

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

static rc_t DataUpdate(Data * self, const KJsonObject * node)
{
    const char * name = NULL;

    assert(self);

    memset(self, 0, sizeof *self);

    name = "bundle";
    StrSet(&self->acc, KJsonObjectGetMember(node, name), name);

    name = "bundle";
    StrSet(&self->bundle, KJsonObjectGetMember(node, name), name);

    name = "link";
    StrSet(&self->link, KJsonObjectGetMember(node, name), name);

    name = "type";
    StrSet(&self->type, KJsonObjectGetMember(node, name), name);

    return 0;
}

struct Locations;
rc_t ItemAddFormat(Item * self, const char * cType, const Data * dad,
    struct Locations ** added);
rc_t LocationsAddVPath(struct Locations * self, const VPath * path,
    const VPath * mapping, bool setHttp, uint64_t osize);

/* We are scanning Item(Run) to find all its Elm-s(Files) -sra, vdbcache, ??? */
static rc_t ItemAddElms(Item * self, const KJsonObject * node)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;

    struct Locations * elm = NULL;

    Data data;
    DataUpdate(&data, node);

    value = KJsonObjectGetMember(node, "link");
    if (data.type != NULL || value != NULL) {
        rc = ItemAddFormat(self, data.type, NULL, &elm);
        if (elm == NULL || rc != 0) {
            return rc;
        }
    }

    value = KJsonObjectGetMember ( node, "link" );
    if (value != NULL)
    {
        Data data;
        DataUpdate(&data, node);

        if (data.link != NULL) {
            VPath * path = NULL;

            String url;
            StringInitCString(&url, data.link);

            rc = VPathMakeFmt(&path, "%s", data.link);

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
    DataUpdate(&data, node);

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
                r2 = ItemAddElms(item, object);
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
