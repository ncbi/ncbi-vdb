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

#include <klib/debug.h> /* DBGMSG */
#include <klib/json.h> /* KJsonObject */
#include <klib/log.h> /* PLOGERR */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */

#include <vfs/manager.h> /* VFSManagerMake */
#include <vfs/path.h> /* VPath */
#include <vfs/services.h> /* KSrvRespObjIterator */

#include "json-response.h" /* Response4 */

#include "path-priv.h" /* VPathMake */
#include "resolver-priv.h" /* DEFAULT_PROTOCOLS */

#include <ctype.h> /* isdigit */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (0)

typedef struct Node {
    const char * name;
    int32_t level;
} Node;

#define THRESHOLD_INFO     2

int THRESHOLD = THRESHOLD_NO_DEBUG;

#define MAX_PATHS 6 /* Locations for File (sra, vdbcache, ???) */
typedef struct File {
    ESrvFileFormat type;
    char * cType;
    char * name;

    int64_t size;

    const VPath * http; /* http path from path[]
when all path[] are alternative ways to get the same acc by different protocols:
data received by names protocol-3.0 */

    const VPath * path [ MAX_PATHS ];

    const VPath * local;
    rc_t localRc;

    const VPath * cache;
    rc_t cacheRc;

    VPath * mapping;
} File;

struct Item { /* Run ob dbGaP file */
    char * acc;
    int64_t id;
    char * name;
    char * itemClass;
    char * tic;
    File * elm;
    uint32_t nElm;
};

typedef struct Status {
    int64_t code;
    char * msg;
} Status;

/* Corresponds to a request item, resolved to 1 or more Items (runs) */
struct Container {
    Status status;
    char * acc;
    uint32_t id;
    Item * files;
    uint32_t nFiles;
    rc_t rc;
};

struct Response4 { /* Response object */
    atomic32_t refcount; 
    Status status;
    Container * items;
    uint32_t nItems;
    char * nextToken;
    rc_t rc;
};

struct KSrvRespObj {
    atomic32_t refcount;
    const Response4 * dad;
    const Container * obj;
};

struct KSrvRespObjIterator {
    atomic32_t refcount;
    const Response4 * dad;
    const Container * obj;
    uint32_t iFile;
    uint32_t iElm;
};

struct KSrvRespFile {
    atomic32_t refcount;
    const Response4 * dad;
    const Item * item;
    File * file;
};

struct KSrvRespFileIterator {
    atomic32_t refcount;
    const Response4 * dad;
    const VPath * const * path;
    uint32_t i;
};

/********************************** Node *********************************/

/*static rc_t NodeMake ( Node ** self, const char * name, bool list ) {
    assert ( self && ! * self );

    * self = ( Node * ) calloc ( 1, sizeof ** self );
    if ( * self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    ( * self ) -> name = name;
    ( * self ) -> level = list ? 0 : -1;

    return 0;
}*/

/********************************** JsonStack *********************************/

void JsonStackPrintInput ( const char * input ) {
    if (THRESHOLD > THRESHOLD_ERROR)
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "Parsing \"%s\"\n", input ) );
}

static
void JsonStackPrint ( const JsonStack * self, const char * msg, bool eol )
{
    size_t i = 0;

    assert ( self );
    assert ( msg || ! eol );

    if (THRESHOLD <= THRESHOLD_ERROR)
        return;

    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ), ( "\"/" ) );

    for ( i = 0; i < self -> i; ++ i ) {
        assert ( self -> nodes );
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "%s", self -> nodes [ i ] . name ) );

        if ( self -> nodes [ i ] . level >= 0 )
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "[%d]", self -> nodes [ i ] . level ) );

        if ( i + 1 < self -> i )
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ), ( "/" ) );
    }

    if ( eol )
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ), ( "\": %s\n", msg ) );
}

static void JsonStackPrintBegin ( const JsonStack * self )
{   JsonStackPrint ( self, "entering", true ); }

static void JsonStackPrintEnd ( const JsonStack * self )
{   JsonStackPrint ( self, "exiting", true ); }

static void JsonStackPrintInt
    ( const JsonStack * self, const char * name, int64_t val )
{
    JsonStackPrint ( self, NULL, false );
    if (THRESHOLD > THRESHOLD_ERROR)
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "/%s\" = %ld\n", name, val ) );
}

static void JsonStackPrintBul
    ( const JsonStack * self, const char * name, bool val )
{
    JsonStackPrint ( self, NULL, false );
    if (THRESHOLD > THRESHOLD_ERROR)
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "/%s\" = %s\n", name, val ? "true" : "false" ) );
}

static void JsonStackPrintStr
    (const JsonStack * self, const char * name, const char * val)
{
    bool first = self->i == 0;
    if (!first)
        JsonStackPrint(self, NULL, false);
    if (THRESHOLD > THRESHOLD_ERROR) {
        if (first)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), ("\""));
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("/%s\" = \"%s\"\n", name, val));
    }
}

rc_t JsonStackRelease ( JsonStack * self, bool failed ) {
    assert ( self );

    assert ( self -> i == 0 );

    JsonStackPrint ( self, failed ? "exiting (failure)\n"
                              : "exiting (success)\n", true );

    free ( self -> nodes );

    memset ( self, 0, sizeof * self );

    return 0;
}

rc_t JsonStackInit ( JsonStack * self ) {
    size_t nmemb = 1;

    assert ( self );

    memset ( self, 0, sizeof * self );

    self -> nodes = ( Node * ) calloc ( nmemb, sizeof * self -> nodes );
    if ( self -> nodes == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    self -> n = nmemb;

    JsonStackPrintBegin ( self );

    return 0;
}

void JsonStackPop ( JsonStack * self ) {
    assert ( self );

    if ( self -> i == 0 )
        return;

    JsonStackPrintEnd ( self );

    -- self -> i;
}

static
rc_t JsonStackPush ( JsonStack * self, const char * name, int32_t level )
{
    assert ( self );

    assert ( self -> i <= self -> n );

    if ( self -> i == self -> n ) {
        size_t nmemb = self -> n + 1;
        void * tmp = realloc ( self -> nodes, nmemb * sizeof * self -> nodes );
        if ( tmp == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

        self -> nodes = ( Node * ) tmp;
        self -> n = nmemb;
    }

    assert ( self -> i < self -> n );

    self -> nodes [ self -> i ] . name  = name;
    self -> nodes [ self -> i ] . level = level;

    ++ self -> i;

    JsonStackPrintBegin ( self );

    return 0;
}

static rc_t JsonStackPushObj ( JsonStack * self, const char * name )
{   return JsonStackPush ( self, name, -1 ); }

 rc_t JsonStackPushArr ( JsonStack * self, const char * name ) {
    return JsonStackPush ( self, name, 0 );
}

rc_t JsonStackArrNext ( JsonStack * self ) {
    assert ( self && self -> i > 0 );
    assert ( self -> nodes [ self -> i - 1 ] . level >= 0 );

    JsonStackPrintEnd(self);
    ++ self -> nodes [ self -> i - 1 ] . level;
    JsonStackPrintBegin(self);

    return 0;
}

/********************************** File *********************************/

static rc_t FileRelease ( File * self ) {

#define TYPES_OF_SCHEMAS 1

    rc_t rc = 0;

    int i = 0;

    if ( self == NULL )
        return 0;

    for ( i = 0; i < MAX_PATHS; ++ i ) {
        if ( self -> path [ i ] == NULL )
            break;
        RELEASE ( VPath, self->path[ i ] );
    }

    RELEASE ( VPath, self -> local );
    RELEASE ( VPath, self -> cache );
    RELEASE ( VPath, self -> mapping );
    RELEASE(VPath, self->http);
    free(self->cType);
    free ( self -> name );

    memset ( self, 0, sizeof * self );

    return rc;
}

static bool FileEmpty ( const File * self ) {
    int i = 0;

    if ( self == NULL )
        return true;

    for ( i = 0; i < MAX_PATHS; ++ i ) {
        if (self->path [ i ] == NULL )
            break;
        return false;
    }

    return true;
}

static rc_t FileSetHttp(File * self, const VPath * path) {
    rc_t rc = 0;

    char scheme[6] = "";

    assert( self );

    if (self->http != NULL)
        return 0;

    rc = VPathReadScheme(path, scheme, sizeof scheme, NULL);
    if (rc != 0)
        return rc;

    if (scheme[0] != 'h' ||
        scheme[1] != 't' ||
        scheme[2] != 't' ||
        scheme[3] != 'p')
    {
        return 0;
    }

    rc = VPathAddRef(path);
    if (rc != 0)
        return rc;

    self->http = path;
    return rc;
}

rc_t FileAddVPath ( File * self, const VPath * path,
                const VPath * mapping, bool setHttp, uint64_t osize)
{
    int i = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( path == NULL )
        return 0;

    for ( i = 0; i < MAX_PATHS; ++ i ) {
        if ( self -> path [ i ] == NULL ) {
            rc_t rc = VPathAddRef ( path );
            if ( rc != 0 )
                return rc;

            self -> path [ i ] = path;

            if (mapping != NULL && self->mapping == NULL) {
                rc_t rc = VPathAddRef(mapping);
                if (rc != 0)
                    return rc;
                self->mapping = (VPath *) mapping;
            }

            if (setHttp) {
                self->size = osize;
                rc = FileSetHttp(self, path);
            }
            else {
                uint64_t size = VPathGetSize(path);
                if (size > 0) {
                    if (self->size < 0)
                        self->size = size;
                    else if (self->size != size)
                        PLOGERR(klogFatal, (klogFatal,
                            RC(rcVFS, rcQuery, rcExecuting, rcString, rcUnexpected),
                            "different sizes for the same file '$name$type': $s1:$s2"
                            "name=%s,type=%s,s1=%lu,s2=%lu",
                            self->name, self->cType, self->size, size));
                }
                rc = FileSetHttp(self, path);
            }

            return rc;
        }
    }

    return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcInsufficient );
}

static rc_t FileAddCache
    ( File * self, const VPath * path, rc_t aRc )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    self -> cacheRc = aRc;

    rc = VPathRelease ( self -> cache );
    rc = VPathAddRef ( path );

    self -> cache = path;

    return rc;
}

static rc_t FileAddLocal
    ( File * self, const VPath * path, rc_t aRc )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    self -> localRc = aRc;

    rc = VPathRelease ( self -> local );
    rc = VPathAddRef ( path );

    self -> local = path;

    return rc;
}

/********************************** Item **********************************/

static rc_t ItemRelease ( Item * self ) {
    rc_t rc = 0;

    uint32_t i = 0;

    if ( self == NULL )
        return 0;

    for ( i = 0; i < self -> nElm; ++ i ) {
        rc_t r2 = FileRelease ( & self -> elm [ i ] );
        if ( r2 != 0 && rc == 0 )
            rc = r2;
    }

    free ( self -> acc );
    free ( self -> elm );
    free ( self -> name );
    free ( self -> tic );
    free(self->itemClass);

    memset ( self, 0, sizeof * self );

    return rc;
}

static bool ItemHasLinks ( const Item * self ) {
    uint32_t i = 0;

    if ( self == NULL )
        return false;

    for ( i = 0; i < self -> nElm; ++ i )
        if ( ! FileEmpty ( & self -> elm [ i ] ) )
            return true;

    return false;
}

rc_t ItemAddFormat ( Item * self, const char * cType, const Data * dad,
                     File ** added, bool checkSameType )
{
    rc_t rc = 0;
    ESrvFileFormat type = eSFFInvalid;
    int idx = -1;
    File * elm = NULL;
    const char * name = dad == NULL ? NULL : dad -> name;
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    assert ( added );
    if ( type >= eSFFMax )
        type = eSFFInvalid;
    if ( cType == NULL ) {
        if ( type == eSFFInvalid )
            type = eSFFSkipped;
    }
    else if ( type == eSFFInvalid ) {
        if      ( strcmp ( cType, "sra"      ) == 0 )
            type = eSFFSra;
        else if ( strcmp ( cType, "vdbcache" ) == 0 )
            type = eSFFVdbcache;
        else
            type = eSFFMax;
    }
    if ( self -> elm == NULL )  {
        size_t n = 1;
        switch ( type ) {
            case eSFFSkipped : idx = 0; n = 1; break;
            case eSFFSra     : idx = 0; n = 1; break;
            case eSFFVdbcache: idx = 0; n = 1; break;
            case eSFFMax     : idx = 0; n = 1; break;
            default         :  assert ( 0 );
        }
        self -> elm = ( File * ) calloc ( n, sizeof * self -> elm );
        if ( self -> elm == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        self->elm->size = -1; /* unknown */
        self -> nElm = n;
    }
    else {
/*      switch ( type ) {
            case eSFFSra     : idx = 0; break;
            case eSFFVdbcache: idx = 0; break;
            case eSFFMax     : {*/
        uint32_t i =0 ;
        for ( i = 0; checkSameType && i < self -> nElm; ++ i ) {
            assert ( ( cType && self -> elm [ i ] . cType )
                    || type == eSFFSkipped );
            if ( ( cType != NULL && self -> elm [ i ] . cType != NULL &&
                   strcmp ( self -> elm [ i ] . cType, cType ) == 0 )
                 ||
                 ( cType == NULL && self -> elm [ i ] . cType == NULL &&
                   self -> elm [ i ] . type == type && type == eSFFSkipped )
               )
            {
                idx = i;
                break;
            }
        }
        if ( idx == -1 ) {
            void * tmp = realloc ( self -> elm,
                                  ( self -> nElm + 1 ) * sizeof * self -> elm );
            if ( tmp == NULL )
                return RC ( rcVFS, rcQuery, rcExecuting,
                            rcMemory, rcExhausted );
            self -> elm = ( File * ) tmp;
            idx = self -> nElm ++;
            elm = & self -> elm [ idx ];
            memset ( elm, 0, sizeof * elm );
            elm->size = -1; /* unknown */
        }
/*              break;
            }
            default       :  assert ( 0 );
        }*/
    }
    assert ( idx >= 0 );

    elm = & self -> elm [ idx ];

    if ( elm -> cType == NULL ) {
        if ( cType != NULL ) {
            elm -> cType = strdup ( cType );
            if ( elm -> cType == NULL )
                return RC ( rcVFS, rcQuery, rcExecuting,
                                   rcMemory, rcExhausted );
        }
        elm -> type = type;
    }

    if (elm->name == NULL ) {
        if ( name != NULL) {
            elm->name = strdup(name);
            if (elm->name == NULL)
                return RC(rcVFS, rcQuery, rcExecuting,
                                 rcMemory, rcExhausted);
	}
	else if ( type == eSFFVdbcache && dad != NULL && dad -> acc != NULL ) {
	    uint32_t s = string_measure ( dad -> acc, NULL ) + 1 + 4 + 8 + 1;
            elm->name = calloc ( 1, s );
            if ( elm->name == NULL )
                return RC ( rcVFS, rcQuery, rcExecuting,
                            rcMemory, rcExhausted );
            rc = string_printf ( elm->name, s, NULL, "%s.sra.vdbcache", dad -> acc );
	}
    }

    * added = elm;

    if (THRESHOLD > THRESHOLD_ERROR) {
        if ( self -> acc != NULL )
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Item '%s': added file '%s'\n", self -> acc,
                  ( * added ) -> cType ) );
        else
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Item %u: added file '%s'\n", self -> id,
                  ( * added ) -> cType ) );
    }

    return rc;
}

rc_t ItemAddVPath ( Item * self, const char * type,
    const VPath * path, const VPath * mapping, bool setHttp, uint64_t osize )
{
    rc_t rc = 0;
    File * l = NULL;
    rc = ItemAddFormat ( self, type, NULL, & l, true );
    if ( rc == 0 )
        rc = FileAddVPath ( l, path, mapping, setHttp, osize);
    return rc;
}

rc_t ItemSetTicket ( Item * self, const String * tic ) {
    if ( self == NULL
      || tic == NULL || tic -> addr == NULL || tic -> size == 0 )
    {   return 0; }

    free ( self -> tic );

    self -> tic = string_dup ( tic -> addr, tic -> size );
    if ( self -> tic == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    return 0;
}

/*rc_t ItemGetId ( const Item * self, const char ** id ) {
    assert ( self && id );

    * id = self -> acc;

    return 0;
}

static rc_t ItemAdd ( Item * self, const VPath * path,
               EType type, const char * cType)
{
    int idx = -1;
    File * elm = NULL;
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    if ( path == NULL )
        return 0;
    if ( type >= eMax )
        type = eInvalid;
    if ( cType == NULL ) {
        if ( type == eInvalid )
            return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    }
    else if ( type == eInvalid ) {
        if      ( strcmp ( cType, "sra"      ) == 0 )
            type = eSra;
        else if ( strcmp ( cType, "vdbcache" ) == 0 )
            type = eVdbcache;
        else
            type = eMax;
    }
    if ( self -> elm == NULL )  {
        size_t n = 2;
        switch ( type ) {
            case eSra     : idx = 0; n = 2; break;
            case eVdbcache: idx = 1; n = 2; break;
            case eMax     : idx = 2; n = 3; break;
            default       :  assert ( 0 );
        }
        self -> elm = ( File * ) calloc ( n, sizeof * self -> elm );
        if ( self -> elm == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        self -> nElm = n;
    }
    else {
        switch ( type ) {
            case eSra     : idx = 0; break;
            case eVdbcache: idx = 1; break;
            case eMax     : {
                uint32_t i =0 ;
                for ( i = 2; i < self -> nElm; ++ i ) {
                    assert ( cType && self -> elm [ i ] . cType );
                    if ( strcmp ( self -> elm [ i ] . cType, cType ) == 0 ) {
                        idx = i;
                        break;
                    }
                }
                if ( idx == -1 ) {
                    void * tmp = realloc ( self -> elm,
                        ( self -> nElm + 1 ) * sizeof * self -> elm );
                    if ( tmp == NULL )
                        return RC ( rcVFS, rcQuery, rcExecuting,
                                    rcMemory, rcExhausted );
                    self -> elm = ( File * ) tmp;
                    idx = self -> nElm ++;
                    elm = & self -> elm [ idx ];
                    memset ( elm, 0, sizeof * elm );
                }
                break;
            }
            default       :  assert ( 0 );
        }
    }
    assert ( idx >= 0 );
    elm = & self -> elm [ idx ];
    if ( elm -> cType == NULL ) {
        elm -> cType = strdup ( cType );
        if ( elm -> cType == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        elm -> type = type;
    }
    return FileAddVCache ( & self -> elm [ idx ], path );
}

static rc_t ItemAddVPath ( Item * self, const VPath * path ) {
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    return ItemAdd ( self, path, eSra, "sra" );
}

static rc_t ItemAddVbdcache ( Item * self, const VPath * path ) {
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    return ItemAdd ( self, path, eVdbcache, "vdbcache" );
}*/

/******************************** Status **************************************/

rc_t StatusInit(Status * self, int64_t code, const char * msg) {
    assert(self);

    self->code = code;

    self->msg = string_dup_measure(msg, NULL);
    if (self->msg == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted);
    return 0;
}

rc_t ContainerStatusInit(Container * self, int64_t code, const char * msg) {
    assert(self);
    return StatusInit(&self->status, code, msg);
}

static rc_t StatusFini(Status * self) {
    assert(self);
    free(self->msg);
    memset(self, 0, sizeof * self);
    return 0;
}

static rc_t StatusSet(
    Status * self, const KJsonObject * node, JsonStack * path)
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;
    const KJsonObject * object = NULL;
    int64_t code = -1;
    const char * msg = NULL;
    const char * name = "status";
    assert(self);
    StatusInit(self, -1, NULL);
    if (node == NULL)
        return rc;
    value = KJsonObjectGetMember(node, name);
    if (value == NULL) {
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
            ("... error: cannot find '%s'\n", name));
        return RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
    }
    rc = JsonStackPushObj(path, name);
    if (rc != 0)
        return rc;
    object = KJsonValueToObject(value);
    if (object == NULL)
        rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);

    if (rc == 0) {
        name = "code";
        value = KJsonObjectGetMember(object, name);
        if (value == NULL) {
            rc = RC(rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete);
            if (THRESHOLD > THRESHOLD_NO_DEBUG)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                ("... error: cannot find 'status/code'\n"));
        }

        if (rc == 0)
            rc = KJsonGetNumber(value, &code);
        if (rc == 0)
            JsonStackPrintInt(path, name, code);
    }

    if (rc == 0) {
        name = "msg";
        value = KJsonObjectGetMember(object, name);
        if (value == NULL) {
            if (THRESHOLD > THRESHOLD_NO_DEBUG)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                ("... error: cannot find 'status/msg'\n"));
        }
        else {
            rc = KJsonGetString(value, &msg);
            if (rc == 0)
                JsonStackPrintStr(path, name, msg);
        }
    }

    if (rc == 0)
        StatusInit(self, code, msg);

    JsonStackPop(path);

    return rc;
}

/********************************** Container *********************************/

static rc_t ContainerRelease ( Container * self ) {
    rc_t rc = 0;

    uint32_t i = 0;

    if ( self == NULL )
        return 0;

    for ( i = 0; i < self -> nFiles; ++ i ) {
        Item * item = & self -> files [ i ];
        assert ( item );
        RELEASE ( Item, item );
    }

    StatusFini( & self -> status );

    free ( self -> files );
    free ( self -> acc );

    memset ( self, 0, sizeof * self );

    return rc;
}

rc_t ContainerAdd ( Container * self, const char * acc, int64_t id,
                    Item ** newItem, const Data * data )
{
    Item * item = NULL;
    void * tmp = NULL;
    const char * name = NULL;
    const char * tic  = NULL;
    uint32_t i = 0;

    if ( newItem == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * newItem = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( data != NULL ) {
        name = data -> name;
        tic  = data -> tic;
    }

    for ( i = 0; i < self -> nFiles; ++ i ) {
        Item * item = & self -> files [ i ];
        assert ( item );
        if ( acc != NULL ) {
            if ( item -> acc != NULL )
                if ( strcmp ( item -> acc, acc ) == 0 ) {
                    * newItem = item;
                    if (THRESHOLD > THRESHOLD_ERROR) {
                        if (self->acc != NULL)
                            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                                ("Container '%s': found '%s' item '%s'\n",
                                    self->acc, item->itemClass, item->acc));
                        else
                            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                                ("Container %u: found '%s' item '%s'\n",
                                    self->id, item->itemClass, item->acc));
                    }
                    return 0;
                }
        }
        else {
            if ( item -> id != 0 )
                if (item->id == id) {
                    *newItem = item;
                    if (self->acc != NULL)
                        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                            ("Container '%s': added '%s' item %u\n",
                                self->acc, item->itemClass, item->id));
                    else
                        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                            ("Container %u: added '%s' item %u\n",
                                self->id, item->itemClass, item->id));
                    return 0;
                }
        }
    }

    ++ self -> nFiles;
    if ( self -> files == NULL ) {
        self -> files = ( Item * ) malloc ( sizeof * self -> files );
        self -> nFiles = 1;
    }
    else {
        tmp = realloc ( self -> files,
                        self -> nFiles * sizeof * self -> files );
        if ( tmp == NULL ) {
            -- self -> nFiles;
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        }
        self -> files = ( Item * ) tmp;
    }

    item = & self -> files [ self -> nFiles - 1 ];
    memset ( item, 0, sizeof * item );

    if ( acc != NULL ) {
        item -> acc = string_dup_measure( acc, NULL);
        if ( item -> acc == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }
    else
        item -> id = id;

    if ( name != NULL ) {
        item -> name = string_dup_measure( name, NULL);
        if ( item -> name == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }

    if ( tic != NULL ) {
        item -> tic = string_dup_measure( tic, NULL);
        if ( item -> tic == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }

    if (data != NULL && data->cls != NULL) {
        item->itemClass = string_dup_measure(data->cls, NULL);
        if (item->itemClass == NULL)
            return RC(rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted);
    }

    * newItem = item;

    if (THRESHOLD > THRESHOLD_ERROR) {
        if (self->acc != NULL)
            if (item->acc != NULL)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Container '%s': added '%s' item '%s'\n",
                        self->acc, item->itemClass, item->acc));
            else
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Container '%s': added '%s' item %u\n",
                        self->acc, item->itemClass, item->id));
        else
            if (item->acc != NULL)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Container %u: added '%s' item '%s'\n",
                        self->id, item->itemClass, item->acc));
            else
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Container %u: added '%s' item %u\n",
                        self->id, item->itemClass, item->id));
    }

    return 0;
}

/*static rc_t ContainerAddAcc (
    Container * self, const char * acc, Item ** newItem )
{
    if ( acc == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );;

    return ContainerAdd ( self, acc, 0, newItem );
}

static
rc_t ContainerAddId ( Container * self, uint32_t id, Item ** newItem )
{
    if ( id == 0 )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );;

    return ContainerAdd ( self, NULL, id, newItem );
}*/

/********************************* Response4 **********************************/

rc_t Response4Fini ( Response4 * self ) {
    rc_t rc = 0;

    uint32_t i = 0;

    if ( self == NULL )
        return 0;

    for ( i = 0; i < self -> nItems; ++ i ) {
        Container * item = & self -> items [ i ];
        assert ( item );
        RELEASE ( Container, item );
    }

    StatusFini(&self->status);

    free ( self -> items );
    free(self->nextToken);

    memset ( self, 0, sizeof * self );

    return rc;
}

rc_t Response4Release ( const Response4 * cself ) {
    rc_t rc = 0;
    
    Response4 * self = ( Response4 * ) cself;

    if ( self == NULL )
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Fini ( self );

    free ( self );

    return rc;
}

rc_t Response4SetNextToken(Response4 * self, const char * nextToken) {
    assert(self);
    self->nextToken = string_dup_measure(nextToken, NULL);
    if (self->nextToken == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted);
    return 0;
}

rc_t Response4AddAccOrId ( Response4 * self, const char * acc,
                           int64_t id, Container ** newItem )
{
    Container * item = NULL;

    void * tmp = NULL;

    uint32_t i = 0;

    if ( newItem == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * newItem = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( acc == NULL && id < 0 )
        return RC ( rcVFS, rcQuery, rcExecuting, rcId, rcNotFound );

    for ( i = 0; i < self -> nItems; ++ i ) {
        Container * item = & self -> items [ i ];
        assert ( item );
        if ( acc != NULL ) {
            if ( item -> acc != NULL )
                if ( strcmp ( item -> acc, acc ) == 0 ) {
                    * newItem = item;
                    return 0;
                }
        }
        else {
            if ( item -> id != 0 )
                if ( item -> id == id )
                    return 0;
        }
    }

    ++ self -> nItems;
    if ( self -> items == NULL ) {
        self -> items = ( Container * ) malloc ( sizeof * self -> items );
        self -> nItems = 1;
    }
    else {
        tmp = realloc ( self -> items,
                        self -> nItems * sizeof * self -> items );
        if ( tmp == NULL ) {
            -- self -> nItems;
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        }
        self -> items = ( Container * ) tmp;
    }

    item = & self -> items [ self -> nItems - 1 ];
    memset ( item, 0, sizeof * item );

    if ( acc != NULL ) {
        item -> acc = string_dup_measure( acc, NULL);
        if ( item -> acc == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        if (THRESHOLD > THRESHOLD_ERROR)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Added to response: container '%s'\n", item -> acc ) );
    }
    else {
        assert ( id >= 0 );
        item -> id = id;
        if (THRESHOLD > THRESHOLD_ERROR)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Added to response: container %u\n", item -> id ) );
    }

    * newItem = item;

    return 0;
}

rc_t Response4AppendUrl ( Response4 * self, const char * url ) {
    rc_t rc = 0;

    VPath * path = NULL;

    Container * box = NULL;
    Item * item = NULL;
    File * l = NULL;

    rc = VPathMake ( & path, url );
    if ( rc != 0 )
        return rc;

    rc = Response4AddAccOrId ( self, url, -1, & box );

    if ( rc == 0 )
        rc = ContainerAdd ( box, url, -1, & item, NULL );

    if ( rc == 0 )
        rc = ItemAddFormat ( item, "", NULL, & l, true );

    if ( rc == 0 )
        rc = FileAddVPath ( l, path, NULL, false, 0 );

    RELEASE ( VPath, path );

    return rc;
}

/*rc_t Response4AppendFile( Response4 * self, const char * acc,
                          struct Item ** file )
{
    rc_t rc = 0;
    Container * box = NULL;
    rc = Response4AddAccOrId ( self, acc, -1, & box );
    if ( rc == 0 )
        rc = ContainerAdd ( box, acc, -1, file );
    return rc;
}


static rc_t Response4AddAcc
    ( Response4 * self, const char * acc, Container ** newItem )
{
    if ( acc == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );;

    return Response4AddAccOrId ( self, acc, 0, newItem );
}

static rc_t Response4AddId
    ( Response4 * self, uint32_t id, Container ** newItem )
{
    if ( id == 0 )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    return Response4AddAccOrId ( self, NULL, id, newItem );
}*/

/******************************** Data setters ********************************/

rc_t IntSet ( int64_t * self, const KJsonValue * node,
              const char * name, JsonStack * path )
{
    rc_t rc = 0;

    assert ( self );

    if ( node == NULL )
        return 0;

    rc = KJsonGetNumber ( node, self );
    if ( rc != 0 )
        return rc;

    if (THRESHOLD > THRESHOLD_INFO)
        JsonStackPrintInt ( path, name, * self );

    return rc;
}

rc_t BulSet ( EState * self, const KJsonValue * node,
              const char * name, JsonStack * path )
{
    rc_t rc = 0;
    bool value = false;

    assert ( self );

    if ( node == NULL )
        return 0;

    rc = KJsonGetBool ( node, & value );
    if ( rc != 0 )
        return rc;

    if (THRESHOLD > THRESHOLD_INFO)
        JsonStackPrintBul ( path, name, value );

    * self = value ? eTrue : eFalse;
    return 0;
}

rc_t StrSet ( const char ** self, const KJsonValue * node,
              const char * name, JsonStack * path )
{
    rc_t rc = 0;
    const char * value = NULL;

    assert ( self );

    if ( node == NULL )
        return 0;

    rc = KJsonGetString ( node, & value );
    if ( rc != 0 )
        return rc;

    if ( value == NULL )
        return 0;

    if (THRESHOLD > THRESHOLD_INFO && path != NULL)
        JsonStackPrintStr(path, name, value);

    if ( value [ 0 ] == '\0' )
        return 0;

    * self = value;
    return 0;
}

/******************************** Data ********************************/

static void DataInit ( Data * self ) {
    assert ( self );

    memset ( self, 0, sizeof * self );

    self -> qual = eUnknown;

    self -> id  = -1;
    self -> exp = -1;
}

static void DataClone ( const Data * self, Data * clone ) {
    DataInit ( clone );

    if ( self == NULL )
        return;

    clone -> acc = self -> acc;
    clone -> id  = self -> id; /* oldCartObjId */
    clone -> cls = self -> cls ; /* itemClass */
    clone -> vsblt = self -> vsblt;
    clone -> name = self -> name;
    clone -> fmt  = self -> fmt; /* format */
    clone -> qual = self -> qual; /* hasOrigQuality */
    clone -> sz   = self -> sz; /* size */
    clone -> md5  = self -> md5;
    clone -> sha  = self -> sha; /* sha256 */
    clone -> mod  = self -> mod; /* modDate */
    clone -> exp  = self -> exp; /* expDate */
    clone -> srv  = self -> srv; /* service */
    clone -> reg  = self -> reg; /* region */
    clone -> link = self -> link; /* ???????????????????????????????????????? */
    clone -> tic  = self -> tic;
    clone ->objectType = self ->objectType;

    clone -> code = self -> code;
}

static rc_t DataUpdate ( const Data * self, Data * next,
                  const KJsonObject * node, JsonStack * path )
{
    const char * name = NULL;

    assert ( next );

    DataClone ( self, next );

    if ( node == NULL )
        return 0;

    name = "acc";
    StrSet ( & next -> acc  , KJsonObjectGetMember ( node, name ), name, path );

    name = "expDate";
    IntSet ( & next -> exp  , KJsonObjectGetMember ( node, name ), name, path );

    name = "format";
    StrSet ( & next -> fmt  , KJsonObjectGetMember ( node, name ), name, path );

    name = "hasOrigQuality";
    BulSet ( & next -> qual , KJsonObjectGetMember ( node, name ), name, path );

    name = "itemClass";
    StrSet ( & next -> cls  , KJsonObjectGetMember ( node, name ), name, path );

    name = "link";
    StrSet ( & next -> link , KJsonObjectGetMember ( node, name ), name, path );

    name = "md5";
    StrSet ( & next -> md5  , KJsonObjectGetMember ( node, name ), name, path );

    name = "modDate";
    IntSet ( & next -> mod  , KJsonObjectGetMember ( node, name ), name, path );

    name = "name";
    StrSet ( & next -> name , KJsonObjectGetMember ( node, name ), name, path );

    name = "id";
    IntSet ( & next -> id   , KJsonObjectGetMember ( node, name ), name, path );

    name = "region";
    StrSet ( & next -> reg  , KJsonObjectGetMember ( node, name ), name, path );

    name = "objectType";
    StrSet(&next->objectType, KJsonObjectGetMember(node, name), name, path);

    name = "service";
    StrSet ( & next -> srv  , KJsonObjectGetMember ( node, name ), name, path );

    name = "sha256";
    StrSet ( & next -> sha  , KJsonObjectGetMember ( node, name ), name, path );

    name = "size";
    IntSet ( & next -> sz   , KJsonObjectGetMember ( node, name ), name, path );

    name = "tic";
    StrSet ( & next -> tic  , KJsonObjectGetMember ( node, name ), name, path );

    name = "visibility";
    StrSet ( & next -> vsblt, KJsonObjectGetMember ( node, name ), name, path );

    return 0;
}

static rc_t DataGetFormat ( const Data * data, const char ** format ) {
    assert ( data && format );

    if ( data -> fmt != NULL )
        * format = data -> fmt;
// else TODO GUESS !!! ???

    return 0;
}

/********************************** File *********************************/

/* "link" is found in JSON: add "link" to Elm (File) using Data from dad */
static rc_t FileAddLink ( File * self, const KJsonValue * node,
                               const Data * dad, const char ** value )
{
    rc_t rc = 0;

    VPath * path = NULL;

    uint8_t md5 [ 16 ];
    bool hasMd5 = false;

    String url;

    String acc;
    memset ( & acc, 0, sizeof acc );

    assert ( self && dad && value );

    if ( node == NULL )
        return 0;

    rc = StrSet ( value, node, NULL, NULL );
    if ( rc != 0 || * value == NULL )
        return rc;

    assert ( * value );

    StringInitCString ( & url, * value );
    StringInitCString ( & acc, dad -> acc );

    if ( dad -> md5 != NULL ) {
        int i = 0;
        for ( i = 0; i < 16; ++ i ) {
            if ( dad -> md5 [ 2 * i ] == '\0' )
                break;
            if ( isdigit ( dad -> md5 [ 2 * i ] ) )
                md5 [ i ] = ( dad -> md5 [ 2 * i ] - '0' ) * 16;
            else
                md5 [ i ] = ( dad -> md5 [ 2 * i ] - 'a' + 10) * 16;
            if ( dad -> md5 [ 2 * i + 1 ] == '\0' )
                break;
            if ( isdigit ( dad -> md5 [ 2 * i + 1 ] ) )
                md5 [ i ] += dad -> md5 [ 2 * i + 1 ] - '0';
            else
                md5 [ i ] += dad -> md5 [ 2 * i + 1 ] - 'a' + 10;
        }
        if ( i == 16 )
            hasMd5 = true;
    }

    if ( dad -> tic == NULL ) {
        const String * objectType = NULL;
        rc = VPathMakeFromUrl ( & path, & url, NULL, true, & acc, dad -> sz,
            dad -> mod, hasMd5 ? md5 : NULL, 0, dad -> srv, objectType, NULL,
            false, false, NULL, -1, 0 );
    }
    else {
        const String * objectType = NULL;
        String ticket;
        StringInitCString ( & ticket, dad -> tic );
        rc = VPathMakeFromUrl ( & path, & url, & ticket, true, & acc, dad -> sz,
            dad -> mod, hasMd5 ? md5 : NULL, 0, dad -> srv, objectType, NULL,
            false, false, NULL, -1, 0 );
    }

    if ( rc == 0 )
        VPathMarkHighReliability ( path, true );

    if ( rc != 0 ) {
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... error: invalid 'link': '%s'\n", * value ) );

        return rc;
    }

    rc = FileAddVPath ( self, path, NULL, false, 0);

    RELEASE ( VPath, path );

    return rc;
}

void FileLogAddedLink(const File * self, const char * url) {
    assert(self);

    if (THRESHOLD > THRESHOLD_ERROR)
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
        ("File '%s': added 'link' = '%s'\n",
            self->cType, url));
}

/* We detected Item(Run)'s Elm(File)
   and keep scanning it down to find all links */
static rc_t FileAddLinks ( File * self, const KJsonObject * node,
                         const Data * dad, JsonStack * path )
{
    rc_t rc = 0;

    bool added = false;

    const KJsonValue * value = NULL;

/*  const char * name = "alternatives"; */

    Data data;
    DataUpdate ( dad, & data, node, path );

    assert ( self );

    if ( node == NULL )
        return 0;

    self->size = data.sz;

    value = KJsonObjectGetMember ( node, "link" );
    if ( value != NULL ) {
        const char * cValue = NULL;
        rc = FileAddLink ( self, value, & data, & cValue );
        if ( rc == 0 ) {
            FileLogAddedLink(self, cValue);
            added = true;
        }
    }
/*
    value = KJsonObjectGetMember ( node, name );
    if ( value == NULL ) {
        name = "sequence";
        value = KJsonObjectGetMember ( node, name );
    }
    if ( value == NULL ) {
        name = "group";
        value = KJsonObjectGetMember ( node, name );
    }

    if ( value != NULL ) {
        uint32_t i = 0;

        const KJsonArray * array = KJsonValueToArray ( value );
        uint32_t n = KJsonArrayGetLength ( array );

        rc = JsonStackPushArr ( path, name );
        if ( rc != 0 )
            return rc;

        for ( i = 0; i < n; ++ i ) {
            rc_t r2 = 0;

            const KJsonObject * object = NULL;

            value = KJsonArrayGetElement ( array, i );
            object = KJsonValueToObject ( value );

            r2 = FileAddLinks ( self, object, & data, path );
            if ( r2 != 0 && rc == 0 )
                rc = r2;

            if ( i + 1 < n )
                JsonStackArrNext ( path );
        }

        JsonStackPop ( path );
    }

    else*/ if ( ! added ) {
        const char * error = "warning";
        assert ( data . code );
        if ( data . code == 200 ) {
            error = "error";
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        }
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... %s: cannot find any link\n", error ) );
    }

    return rc;
}

static rc_t FileMappingByAcc(const File * self) {
    const char sra[] = "sra";
    const char vdbcache[] = "vdbcache";
    const char pileup[] = "pileup";
    const char realign[] = "realign";

    uint32_t l = 0;

    assert(self);

    l = string_measure(self->cType, NULL);

    if (string_cmp(self->cType, l, sra, sizeof sra - 1, 99) == 0
        ||
        string_cmp(self->cType, l, vdbcache, sizeof vdbcache - 1, 9) == 0
        ||
        string_cmp(self->cType, l, pileup, sizeof pileup - 1, 99) == 0
        ||
        string_cmp(self->cType, l, realign, sizeof realign - 1, 99) == 0
        )
    {
        return false;
    }

    return true;
}

/*
static rc_t ItemMappingByAcc(const Item * self) {
    const char sra[] = "sra";
    const char vdbcache[] = "vdbcache";
    const char pileup[] = "pileup";
    const char realign[] = "realign";
    
    assert( self );

    if (self->acc != NULL) {
        uint32_t l = string_measure(self->itemClass, NULL);

        if (string_cmp(self->itemClass, l, sra, sizeof sra - 1, 99) == 0
            ||
            string_cmp(self->itemClass, l, vdbcache, sizeof vdbcache - 1, 9) == 0
            ||
            string_cmp(self->acc, l, pileup, sizeof pileup - 1, 99) == 0
            ||
            string_cmp(self->acc, l, realign, sizeof realign - 1, 99) == 0
        )
        return true;
    }

    return false;
}
*/

static const char * ItemOrLocationGetName(const Item * item,
                                          const File * file)
{
    assert(item && file);

    return file->name != NULL ? file->name : item->name;
}

static /* don't free returned name !!! */
rc_t FileGetVdbcacheName ( const File * cself,
    const char ** name, const KSrvRespFile * file)
{
    rc_t rc = 0;

    assert ( cself && name );

    * name = NULL;

    if (cself->type == eSFFVdbcache) {
        if (cself->name == NULL
            && file != NULL && file->item != NULL && file->item->acc != NULL)
        {
            File * self = (File*)cself;
            uint32_t s = string_measure(file->item->acc, NULL) + 1 + 4 + 8 + 1;
            self->name = calloc(1, s);
            if (self->name == NULL)
                return RC(rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted);
            rc = string_printf(self->name, s, NULL,
                "%s.sra.vdbcache", file->item->acc);
        }
        *name = cself->name;
    }

    return rc;
}

static rc_t FileInitMapping ( File * self, const Item * item ) {
    rc_t rc = 0;

    const VPath * path = NULL;
    int64_t projectId = -1;

    String ticket;

    assert ( self && item );

    if ( self -> mapping != NULL )
        return 0;

    if ( self -> path [ 0 ] != NULL )
        path = self -> path [ 0 ];
    else
        return 0;

    projectId = path->projectId;

    memset ( & ticket, 0, sizeof ticket );

    if ( item -> tic != NULL )
        StringInitCString ( & ticket, item -> tic );

    rc = VPathCheckFromNamesCGI ( path, & ticket, projectId, NULL );

    if ( rc == 0 ) {
        const char * name = ItemOrLocationGetName(item, self);

        if ( item -> tic != NULL )
            if (FileMappingByAcc( self ) || name == NULL )
                rc = VPathMakeFmt ( & self -> mapping, "ncbi-acc:%s?tic=%s",
                                                    item -> acc, item -> tic );
            else
                rc = VPathMakeFmt ( & self -> mapping, "ncbi-file:%s?tic=%s",
                                                    name, item -> tic );
        else
            if (FileMappingByAcc(self) || name == NULL) {
                if (projectId < 0)
                    rc = VPathMakeFmt(&self->mapping, "ncbi-acc:%s", item->acc);
                else
                    rc = VPathMakeFmt(&self->mapping, "ncbi-acc:%s?pId=%d",
                        item->acc, projectId);
            }
            else {
                if (projectId < 0)
                    rc = VPathMakeFmt(&self->mapping, "ncbi-file:%s", name);
                else
                    rc = VPathMakeFmt(&self->mapping, "ncbi-file:%s?pId=%d",
                        name, projectId);
            }
    }

    return rc;
}

/********************************** Item **********************************/

/* We are scanning Item(Run) to find all its Elm-s(Files) -sra, vdbcache, ??? */
static rc_t ItemAddElms4 ( Item * self, const KJsonObject * node,
                   const Data * dad, JsonStack * path )
{
    rc_t rc = 0;

    const char * format = NULL;
    const KJsonValue * value = NULL;

    const char * name = "group";

    Data data;
    DataUpdate ( dad, & data, node, path );

    DataGetFormat ( & data, & format );

    value = KJsonObjectGetMember ( node, name );
/*  if ( value == NULL ) {
        name = "sequence";
        value = KJsonObjectGetMember ( node, name );
    }
*/
    if ( value == NULL ) {
        name = "alternatives";
        value = KJsonObjectGetMember ( node, name );
    }

    if ( /*format == NULL &&*/ value != NULL ) {
        uint32_t i = 0;

        const KJsonArray * array = KJsonValueToArray ( value );
        uint32_t n = KJsonArrayGetLength ( array );
        rc = JsonStackPushArr ( path, name );
        if ( rc != 0 )
            return rc;
        for ( i = 0; i < n; ++ i ) {
            rc_t r2 = 0;

            const KJsonObject * object = NULL;

            value = KJsonArrayGetElement ( array, i );
            object = KJsonValueToObject ( value );
            r2 = ItemAddElms4 ( self, object, & data, path );
            if ( r2 != 0 && rc == 0 )
                rc = r2;

            if ( i + 1 < n )
                JsonStackArrNext ( path );
        }

        JsonStackPop ( path );
    }
    else {
        value = KJsonObjectGetMember ( node, "link" );

        if ( format != NULL || value != NULL ) {
            File * elm = NULL;
            rc = ItemAddFormat ( self, format, & data, & elm, true );
            if ( rc == 0 && elm != NULL ) {
                if (THRESHOLD > THRESHOLD_ERROR)
                    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                        ( "Adding links to a file...\n" ) );
                rc = FileAddLinks ( elm, node, & data, path );
            }
        }

        else {
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
            if (THRESHOLD > THRESHOLD_NO_DEBUG)
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                    ( "... error: file 'format' was not set\n" ) );
        }
    }

    if (rc == 0)
        rc = ItemInitMapping(self);

    return rc;
}

rc_t ItemInitMapping(Item * self) {
    rc_t rc = 0;
    uint32_t i = 0;

    for (i = 0; rc == 0 && i < self->nElm; ++i)
        rc = FileInitMapping(&self->elm[i], self);

    return rc;
}


void ItemLogAdd(const Item * self) {
    assert(self);

    if (THRESHOLD > THRESHOLD_ERROR) {
        if (self->acc != NULL)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), ("Adding files to '%s' "
                "item '%s'...\n", self->itemClass, self->acc));
        else
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON), ("Adding files to '%s' "
                "item %u...\n", self->itemClass, self->id));
    }
}

/********************************** Container *********************************/

/* We are inside of Container (corresponds to request object),
   adding nested Items(runs, gdGaP files) */
static rc_t ContainerAddItem ( Container * self, const KJsonObject * node,
                        const Data * dad, JsonStack * path )
{
    rc_t rc = 0;

    const char * acc = NULL;
    int64_t id = -1;

    Item * item = NULL;

    Data data;
    DataUpdate ( dad, & data, node, path );

    acc = data . acc;
    id  = data . id;

    rc = ContainerAdd ( self, acc, id, & item, & data );

    if ( rc == 0 && item != NULL ) {
        ItemLogAdd(item);
        rc = ItemAddElms4 ( item, node, & data, path );
    }

    if ( rc == 0 && ! ItemHasLinks ( item ) && data . code == 200 ) {
        assert ( data . code );
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
    }

    return rc;
}

void ContainerProcessStatus(Container * self, const Data * data) {
    assert(self && data);

    if (self->status.code != 200) {
        KLogLevel lvl = klogInt;
        bool logError = true;

        switch (self->status.code / 100) {
        case 0:
            self->rc = RC(rcVFS, rcQuery, rcResolving, rcMessage, rcCorrupt);
            break;

        case 1:
            /* informational response
            not much we can do here */
            self->rc = RC(rcVFS, rcQuery, rcResolving, rcError, rcUnexpected);
            break;

        case 2:
            /* but can only handle 200 */
            self->rc = RC(rcVFS, rcQuery, rcResolving, rcError, rcUnexpected);
            break;

        case 3:
            /* redirection
            currently this is being handled by our request object */
            self->rc = RC(rcVFS, rcQuery, rcResolving, rcError, rcUnexpected);
            break;

        case 4:
            /* client error */
            lvl = klogErr;
            switch (self->status.code) {
            case 400:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcMessage, rcInvalid);
                break;
            case 401:
            case 403:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcQuery, rcUnauthorized);
                break;
            case 404: /* 404|no data :
                      If it is a real response then this assession is not found.
                      What if it is a DB failure?
                      Will be retried if configured to do so? */
                self->rc = RC(rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
                break;
            case 410:
                self->rc = RC(rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
                break;
            default:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcError, rcUnexpected);
            }
            break;

        case 5:
            /* server error */
            lvl = klogSys;
            switch (self->status.code) {
            case 503:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcDatabase, rcNotAvailable);
                break;
            case 504:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcTimeout, rcExhausted);
                break;
            default:
                self->rc = RC(rcVFS, rcQuery, rcResolving,
                    rcError, rcUnexpected);
            }
            break;

        default:
            self->rc = RC(rcVFS, rcQuery, rcResolving, rcError, rcUnexpected);
        }

        /* log message to user */
        if (logError)
            PLOGERR(lvl, (lvl, self->rc,
                "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
                "acc=%s,msg=%s,code=%u",
                data->acc, self->status.msg, self->status.code));
    }
}

bool ContainerIs200AndEmpty(const Container * self) {
    assert(self);

    return self->status.code == 200 && self->nFiles == 0;
}

/* We are inside or above of a Container
   and are looking for Items(runs, gdGaP files) to ddd */
static rc_t Response4AddItems4 ( Response4 * self, Container * aBox,
    const KJsonObject * node, const Data * dad, JsonStack * path )
{
    rc_t rc = 0;

    Container * box = aBox;

    const KJsonValue * value = NULL;

    Data data;
    DataUpdate ( dad, & data, node, path );

    if ( box == NULL ) {
        const char * acc = data . acc;
        int64_t id = data . id;
        rc = Response4AddAccOrId ( self, acc, id, & box );
        if ( box == NULL ) {
            if ( acc == NULL && id < 0 )
                if (THRESHOLD > THRESHOLD_NO_DEBUG)
                    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                        ( "... error: cannot find any 'acc' or 'id'\n" ) );
            return rc;
        }
        rc = StatusSet ( & box -> status, node, path );
        if ( rc == 0 )
            data . code = box -> status . code;
    }

    assert ( box );

    if (rc == 0)
        ContainerProcessStatus(box, &data);

    value = KJsonObjectGetMember ( node, "link" );

    if ( ( ( data . cls != NULL ) && ( strcmp ( data .cls, "run"  ) == 0 
                                    || strcmp ( data .cls, "file" ) == 0 ) )
        || value != NULL )
    {
        if (THRESHOLD > THRESHOLD_ERROR) {
            if (box->acc != NULL)
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Adding a '%s' item to container '%s'...\n",
                        data.cls, box->acc));
            else
                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_JSON),
                    ("Adding a '%s' item to container %u...\n",
                        data.cls, box->id));
        }
        rc = ContainerAddItem ( box, node, & data, path );
    }
    else {
        const char * name = "sequence";
        value = KJsonObjectGetMember ( node, name );
        if ( value == NULL ) {
            name = "group";
            value = KJsonObjectGetMember ( node, name );
        }
        if ( value == NULL ) {
            name = "alternatives";
            value = KJsonObjectGetMember ( node, name );
        }

        if ( value != NULL ) {
            uint32_t i = 0;

            const KJsonArray * array = KJsonValueToArray ( value );
            uint32_t n = KJsonArrayGetLength ( array );
            rc = JsonStackPushArr ( path, name );
            if ( rc != 0 )
                return rc;
            for ( i = 0; i < n; ++ i ) {
                rc_t r2 = 0;

                const KJsonObject * jObject = NULL;

                value = KJsonArrayGetElement ( array, i );
                jObject = KJsonValueToObject ( value );
                r2 = Response4AddItems4 ( self, box, jObject, & data, path );
                if ( r2 != 0 && rc == 0 )
                    rc = r2;

                if ( i + 1 < n )
                    JsonStackArrNext ( path );
            }

            JsonStackPop ( path );
        }
    }

    if ( aBox == NULL && ContainerIs200AndEmpty(box) ) {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... error: cannot find any container\n" ) );
    }

    return rc;
}

/* Add response document */
static rc_t Response4Init4 ( Response4 * self, const char * input ) {
    rc_t rc = 0;
    char error [ 99 ] = "";
    rc_t r2 = 0;

    KJsonValue * root = NULL;

    JsonStack path;
    Data data;

    const KJsonObject * object = NULL;
    const KJsonValue * value = NULL;

    const char name [] = "sequence";

    JsonStackPrintInput ( input );

    rc = KJsonValueMake ( & root, input, error, sizeof error );
    if ( rc != 0 ) {
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... error: invalid JSON\n" ) );
        return rc;
    }

    assert ( self );

    rc = JsonStackInit(&path);
    if (rc != 0)
        return rc;

    object = KJsonValueToObject ( root );

    DataUpdate ( NULL, & data, object, & path );

    value = KJsonObjectGetMember ( object, name );
    if ( value == NULL ) {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        if (THRESHOLD > THRESHOLD_NO_DEBUG)
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... error: cannot find '%s'\n", name ) );
    }
    else {
        const KJsonArray * array = KJsonValueToArray ( value );
        if ( array == NULL )
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcInvalid );
        else {
            uint32_t n = KJsonArrayGetLength ( array );

            rc = JsonStackPushArr ( & path, name );
            if ( rc != 0 )
                return rc;

            if ( n == 0 ) {
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
                if (THRESHOLD > THRESHOLD_NO_DEBUG)
                    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                        ( "... error: '%s' is empty\n", name ) );
            }
            else {
                uint32_t i = 0;
                for ( i = 0; i < n; ++ i ) {
                    rc_t r2 = 0;

                    value = KJsonArrayGetElement ( array, i );
                    object = KJsonValueToObject ( value );
                    r2 = Response4AddItems4
                        ( self, NULL, object, & data, & path );
                    if ( r2 != 0 && rc == 0 )
                        rc = r2;

                    if ( i + 1 < n )
                        JsonStackArrNext ( & path );
                }
            }
            JsonStackPop ( & path );

        }
    }

    KJsonValueWhack ( root );

    if ( rc != 0 )
        Response4Fini ( self );

    r2 = JsonStackRelease ( & path, rc != 0 );
    if ( r2 != 0 && rc == 0 )
        rc = r2;

    return rc;
}

rc_t Response4MakeEmpty ( Response4 ** self ) {
    const char * env = NULL;

    assert ( self );

    * self = ( Response4 * ) calloc ( 1, sizeof ** self );
    if ( * self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    env = getenv("NCBI_VDB_JSON");

    if (env != NULL) {
        int NCBI_VDB_JSON = 0;

        for (; *env != '\0'; ++env) {
            char c = *env;
            if (c < '0' || c > '9')
                break;

            NCBI_VDB_JSON = NCBI_VDB_JSON * 10 + c - '0';
        }

        THRESHOLD = NCBI_VDB_JSON;
    }
    else
        THRESHOLD = THRESHOLD_ERROR;

    atomic32_set(&(*self)->refcount, 1);

    return 0;
}

rc_t Response4Make4 ( Response4 ** self, const char * input ) {
    rc_t rc = 0;

    Response4 * r = NULL;

    assert ( self );

    rc = Response4MakeEmpty ( & r );
    if ( rc != 0 )
        return rc;

    rc = Response4Init4 ( r, input );
    if ( rc != 0 )
        free ( r );
    else
        * self = r;

    return rc;
}

/*rc_t JResponseMake ( JResponse ** self, const char * input ) {
    rc_t rc = 0;

    JResponse * r = ( JResponse * ) calloc ( 1, sizeof * self );
    if ( r == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    assert ( self );

    rc = Response4Init4 ( & r -> r, input );
    if ( rc != 0 )
        free ( r );
    else {
        atomic32_set ( & r -> refcount, 1 );
        * self = r;
    }

    return rc;
}

rc_t JResponseRelease ( const JResponse * cself ) {
    rc_t rc = 0;

    JResponse * self = ( JResponse * ) cself;

    if ( self == NULL )
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Fini ( & self -> r );

    memset ( self, 0, sizeof * self );

    free ( self );

    return rc;
}*/

rc_t Response4AddRef ( const Response4 * self ) {
    if ( self != NULL )
        atomic32_inc ( & ( ( Response4 * ) self ) -> refcount );

    return 0;
}

rc_t Response4StatusInit(Response4 * self, int64_t code, const char * msg,
    bool error)
{
    rc_t rc = 0;
    assert(self);
    rc = StatusInit(&self->status, code, msg);
    if (rc == 0) {
        if (code != 200 || error) {
            if (code == 440)
                self->rc = RC(rcVFS, rcQuery, rcResolving, rcDoc, rcCanceled);
            else
                self->rc = RC(
                    rcVFS, rcQuery, rcResolving, rcError, rcUnexpected);
        }
    }
    return rc;
}

rc_t Response4GetRc ( const Response4 * self, rc_t * rc ) {
    assert ( self && rc );

    * rc = self -> rc;

    return 0;
}

rc_t Response4GetNextToken(const Response4 * self, const char ** nextToken) {
    const char * dummy = NULL;
    if (nextToken == NULL)
        nextToken = &dummy;

    if (self != NULL)
        *nextToken = self->nextToken;
    else
        *nextToken = NULL;

    return *nextToken == NULL
        ? 0 : RC(rcVFS, rcQuery, rcExecuting, rcToken, rcUnexpected);
}

rc_t Response4GetKSrvRespObjCount ( const Response4 * self,
                                    uint32_t * n )
{
    assert ( self && n );

    * n = self -> nItems;

    return 0;
}

static rc_t Response4KSrvRespObjMake ( const Response4 * self, uint32_t idx,
                                       const KSrvRespObj ** box )
{
    rc_t rc = 0;

    KSrvRespObj * p = NULL;

    assert ( self && idx < self -> nItems && box );

    p = ( KSrvRespObj * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    rc = Response4AddRef ( self );
    if ( rc == 0 ) {
        p -> dad = self;
        p -> obj = self -> items + idx;
        atomic32_set ( & p -> refcount, 1 );
        * box = p;
    }
    else {
        free ( p );
        * box = NULL;
    }

    return rc;
}

rc_t Response4GetKSrvRespObjByIdx ( const Response4 * self, uint32_t idx,
                                    const KSrvRespObj ** box )
{
    assert ( self && box );

    if ( idx >= self -> nItems )
        return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcNotFound );

    return Response4KSrvRespObjMake ( self, idx, box );
}

rc_t Response4GetKSrvRespObjByAcc ( const Response4 * self, const char * aAcc,
                                    const KSrvRespObj ** box )
{
    uint32_t i = 0;

    assert ( self && aAcc );

    for ( i = 0; i < self -> nItems; ++ i ) {
        const char * acc = self -> items [ i ] . acc;
        if ( strcmp ( acc, aAcc ) == 0 )
            return Response4GetKSrvRespObjByIdx ( self, i, box );
    }

    return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcNotFound );
}

rc_t KSrvRespObjRelease ( const KSrvRespObj * cself ) {
    rc_t rc = 0;

    KSrvRespObj * self = ( KSrvRespObj * ) cself;

    if ( self  == NULL)
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Release ( self -> dad );

    memset ( self, 0, sizeof * self );

    free ( self );

    return rc;
}

rc_t KSrvRespObjGetAccOrId(const KSrvRespObj * self,
    const char ** acc, uint32_t * id)
{
    assert( self && self ->obj && acc && id );

    *acc = self->obj->acc;
    *id = self->obj->id;

    return 0;
}

rc_t KSrvRespObjGetError(const KSrvRespObj * self,
                         rc_t * rc, int64_t * code, const char ** msg)
{
    assert(self && self->obj && rc);

    *rc = self->obj->rc;

    if ( code != NULL )
        *code = self->obj->status.code;

    if (msg != NULL)
        *msg = self->obj->status.msg;

    return 0;
}

rc_t KSrvRespObjGetFileCount ( const KSrvRespObj * self,
                               uint32_t * aCount )
{
    rc_t rc = 0;

    uint32_t count = 0;

    KSrvRespObjIterator * it = NULL;

    if ( aCount == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * aCount = 0;

    if ( self  == NULL)
        return 0;

    rc = KSrvRespObjMakeIterator ( self, & it );
    while ( rc == 0 ) {
        KSrvRespFile * file = NULL;
        rc = KSrvRespObjIteratorNextFile ( it, & file );
        if ( file == NULL )
            break;
        ++ count;
        RELEASE ( KSrvRespFile, file );
    }

    RELEASE ( KSrvRespObjIterator, it );

    if ( rc == 0 )
       * aCount = count;

    return rc;
}

rc_t KSrvRespObjMakeIterator
    ( const KSrvRespObj * self, KSrvRespObjIterator ** it )
{
    rc_t rc = 0;

    KSrvRespObjIterator * p = NULL;

    if ( it == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * it = NULL;

    if ( self == NULL || self->obj == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if (self->obj->rc != 0)
        return self->obj->rc;

    p = ( KSrvRespObjIterator * ) calloc ( 1, sizeof * p );

    rc = Response4AddRef ( self -> dad );

    if ( rc == 0 ) {
        p -> dad = self -> dad;
        p -> obj = self -> obj;
        atomic32_set ( & p -> refcount, 1 );

        * it = p;
    }
    else
        free ( p );

    return rc;
}

rc_t KSrvRespObjIteratorRelease ( const KSrvRespObjIterator * cself ) {
    rc_t rc = 0;

    KSrvRespObjIterator * self = ( KSrvRespObjIterator * ) cself;

    if ( self == NULL)
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Release ( self -> dad );

    memset ( self, 0, sizeof * self );

    free ( self );

    return rc;
}

rc_t KSrvRespObjIteratorNextFile ( KSrvRespObjIterator * self,
                                   KSrvRespFile ** file )
{
    if ( file == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * file = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    while ( true ) {
        const Item * itm = NULL;

        if ( self -> iFile >= self -> obj -> nFiles )
            return 0;

        itm = & self -> obj -> files [ self -> iFile ];

        if ( self -> iElm >= itm -> nElm ) {
            self -> iElm = 0;
            ++ self -> iFile;
            continue;
        }
        else {
            KSrvRespFile * p = ( KSrvRespFile * ) calloc ( 1, sizeof * p );
            if ( p == NULL )
                return
                    RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
            else {
                rc_t rc = Response4AddRef ( self -> dad );
                if ( rc == 0 ) {
                    p -> dad = self -> dad;
                    p -> item = itm;
                    p -> file = & itm -> elm [ self -> iElm ++ ];
                    atomic32_set ( & p -> refcount, 1 );

                    * file = p;
                    return 0;
                }
                else {
                    free ( p );

                    return rc;
                }
            }
        }
    }

    assert ( 0 );
    return 1;
}

rc_t KSrvRespFileAddRef(const KSrvRespFile * self) {
    if (self != NULL)
        atomic32_inc(&((KSrvRespFile *)self)->refcount);
    return 0;
}

rc_t KSrvRespFileRelease ( const KSrvRespFile * cself ) {
    rc_t rc = 0;

    KSrvRespFile * self = ( KSrvRespFile * ) cself;

    if ( self  == NULL)
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Release ( self -> dad );

    memset ( self, 0, sizeof * self );

    free ( self );

    return rc;
}

rc_t KSrvRespFileGetFormat ( const KSrvRespFile * self,
                             ESrvFileFormat * ff )
{
    assert ( self && self -> file && ff );

    * ff = self -> file -> type;

    return 0;
}

rc_t KSrvRespFileGetClass(const KSrvRespFile * self, const char ** itemClass) {
    assert(self && self->item && itemClass);

    *itemClass = self->item->itemClass;

    return 0;
}

rc_t KSrvRespFileGetType(const KSrvRespFile * self, const char ** type) {
    assert(self && self->item && type);

    *type = self->file->cType;

    return 0;
}

rc_t KSrvRespFileGetSize(const KSrvRespFile * self, uint64_t *size) {
    assert(self && self->file && size);

    *size = self->file->size < 0 ? 0 : self->file->size;

    return 0;
}

rc_t KSrvRespFileGetAccOrId(const KSrvRespFile * self,
    const char ** acc, uint32_t * id)
{
    uint32_t iDummy = 0;
    if (id == NULL)
        id = &iDummy;

    assert(self && self -> item && acc);

    *acc = self->item->acc;
    *id = self->item->id;

    return 0;
}

static
rc_t KSrvRespFileGetAccNoTic ( const KSrvRespFile * self, const char ** acc )
{
    assert ( self && self -> item && acc );
    
    *acc = self->item->acc;

    if (self->item->id <= 0) {
        const char * name = ItemOrLocationGetName(self->item, self->file);
        if (name != NULL)
            * acc = name;
    }

    return 0;
}

/* don't free returned ptr-s !!! */
rc_t KSrvRespFileGetAccOrName ( const KSrvRespFile * self, const char ** out,
                                                           const char ** tic)
{
    rc_t rc = 0;
    const char *dummy = NULL;
    if (tic == NULL)
        tic = &dummy;
    *tic = *out = NULL;
    if (self == NULL || self->item == NULL)
        return 0;
    * tic = self -> item -> tic;
    rc = FileGetVdbcacheName ( self -> file, out, self );
    if ( * out != NULL )
        return rc;
    else
        return KSrvRespFileGetAccNoTic ( self, out );
}

rc_t KSrvRespFileGetName(const KSrvRespFile * self, const char ** name) {
    return KSrvRespFileGetAccOrName(self, name, NULL);
}

rc_t KSrvRespFileGetId ( const KSrvRespFile * self, uint64_t * id,
                                                const char ** tic )
{
    assert ( self && self -> item && id && tic );

    * id  = self -> item -> id;
    * tic = self -> item -> tic;

    return 0;
}
                                                           
rc_t KSrvRespFileGetHttp ( const KSrvRespFile * self,
                            const VPath ** path )
{
    rc_t rc = 0;

    assert ( self && self -> file && path );

    * path = NULL;

    rc = VPathAddRef ( self -> file -> http );

    if ( rc == 0 )
        * path = self -> file -> http;

    return rc;
}

rc_t KSrvRespFileGetCache ( const KSrvRespFile * self,
                            const VPath ** path )
{
    rc_t rc = 0;

    if (path == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);

    * path = NULL;

    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

    assert ( self -> file );

    if ( self -> file -> cacheRc != 0 )
        return self -> file -> cacheRc;

    rc = VPathAddRef ( self -> file -> cache );

    if ( rc == 0 )
        * path = self -> file -> cache;

    return rc;
}

rc_t KSrvRespFileGetLocal ( const KSrvRespFile * self,
                            const VPath ** path )
{
    rc_t rc = 0;

    if (path == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);

    * path = NULL;

    assert ( self && self -> file );

    if ( self -> file -> localRc != 0 )
        return self -> file -> localRc;

    rc = VPathAddRef ( self -> file -> local );

    if ( rc == 0 )
        * path = self -> file -> local;

    return rc;
}

rc_t KSrvRespFileAddLocalAndCache ( KSrvRespFile * self,
                                    const VPathSet * localAndCache )
{
    rc_t rc = 0, r2 = 0, aRc = 0;

    const VPath * path = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( localAndCache == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    aRc = VPathSetGetCache ( localAndCache, & path );
    rc = FileAddCache ( self -> file, path, aRc );
    RELEASE ( VPath, path );

    aRc = VPathSetGetLocal ( localAndCache, & path );
    r2 = FileAddLocal ( self -> file, path, aRc );
    if ( r2 != 0 && rc == 0 )
        rc = r2;
    RELEASE ( VPath, path );

    return rc;
}

rc_t KSrvRespFileGetMapping ( const KSrvRespFile * self,
                              const VPath ** mapping )
{
    rc_t rc = 0;

    assert ( self && mapping );

    * mapping = NULL;

    rc = VPathAddRef ( self ->file -> mapping );
    if ( rc == 0 )
        * mapping = self ->file -> mapping;

    return rc;
}

rc_t KSrvRespFileMakeIterator ( const KSrvRespFile * self,
    KSrvRespFileIterator ** it )
{
    rc_t rc = 0;

    const VPath * const  * path = NULL;
    KSrvRespFileIterator * p = NULL;

    if ( it == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * it = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    path = self->file->path;

    assert ( path );
    if ( path [ 0 ] == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcNotFound );

    p = ( KSrvRespFileIterator * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    rc = Response4AddRef ( self -> dad );

    if ( rc == 0 ) {
        p -> dad = self -> dad;
        p -> path = path;

        atomic32_set ( & p -> refcount, 1 );

        * it = p;
    }
    else
        free ( p );

    return rc;
}

rc_t KSrvRespFileIteratorRelease
    ( const KSrvRespFileIterator * cself )
{
    rc_t rc = 0;

    KSrvRespFileIterator * self = ( KSrvRespFileIterator * ) cself;

    if ( self  == NULL)
        return 0;

    if ( ! atomic32_dec_and_test ( & self -> refcount ) )
        return 0;

    rc = Response4Release ( self -> dad );

    memset ( self, 0, sizeof * self );

    free ( self );

    return rc;
}

rc_t KSrvRespFileIteratorNextPath ( KSrvRespFileIterator * self,
                                    const VPath ** aPath )
{
    if ( aPath == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * aPath = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    while ( true ) {
        const VPath * path = NULL;
        if ( self -> i >= MAX_PATHS )
            return 0;
        path = self -> path [ self -> i ];
        if ( path == NULL )
            return 0;
        else {
            rc_t rc = VPathAddRef ( path );
            if ( rc == 0 ) {
                * aPath = path;
                ++ self -> i;
            }
            return rc;
        }
    }
    return 0;
}
