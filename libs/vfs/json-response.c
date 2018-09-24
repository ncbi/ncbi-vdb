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
#include <klib/rc.h> /* RC */

#include <vfs/manager.h> /* VFSManagerMake */
#include <vfs/path.h> /* VPath */
#include <vfs/services.h> /* KSrvRespObjIterator */

#include "json-response.h" /* Response4 */

#include "path-priv.h" /* VPathMake */
#include "resolver-priv.h" /* DEFAULT_PROTOCOLS */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (0)

typedef struct {
    const char * name;
    int32_t level;
} Node;

typedef struct {
    Node * nodes;
    size_t i;
    size_t n;
} Stack;

#define MAX_PATHS 6 /* Locations for Element (sra, vdbcache, ???) */
typedef struct {
    ESrvFileFormat type;
    char * cType;
    const VPath * fasp  [ MAX_PATHS ];
    const VPath * http  [ MAX_PATHS ];
    const VPath * https [ MAX_PATHS ];

    const VPath * local;
    rc_t localRc;

    const VPath * cache;
    rc_t cacheRc;
} Locations;

struct Item { /* Run ob dbGaP file */
    char * acc;
    int64_t id;
    Locations * elm;
    uint32_t nElm;
};

typedef struct {
    int64_t code;
} Status;

/* Corresponds to a request item, resolved to 1 or more Items (runs) */
struct Container {
    Status status;
    char * acc;
    uint32_t id;
    Item * files;
    uint32_t nFiles;
};

struct Response4 { /* Response object */
    atomic32_t refcount; 
    Container * items;
    uint32_t nItems;
};

typedef enum {
    eUnknown,
    eFalse,
    eTrue
} EState;

typedef struct {
    const char * acc;
    int64_t id; /* oldCartObjId */
    const char * cls; /* itemClass */
    const char * vsblt;
    const char * name;
    const char * fmt; /* format */
    EState qual; /* hasOrigQuality */
    int64_t sz; /* size */
    const char * md5;
    const char * sha; /* sha256 */
    int64_t mod; /* modDate */
    int64_t exp; /* expDate */
    const char * srv; /* service */
    const char * reg; /* region */
    const char * link; /* ??????????????????????????????????????????????????? */
    const char * tic;
} Data;

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
    Locations * file;
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

/********************************** Stack *********************************/

static void StackPrintInput ( const char * input ) {
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
        ( "Parsing \"%s\"\n", input ) );
}

static
void StackPrint ( const Stack * self, const char * msg, bool eol )
{
    size_t i = 0;

    assert ( self );
    assert ( msg || ! eol );

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

static void StackPrintBegin ( const Stack * self )
{   StackPrint ( self, "entering", true ); }

static void StackPrintEnd ( const Stack * self )
{   StackPrint ( self, "exiting", true ); }

static void StackPrintInt
    ( const Stack * self, const char * name, int64_t val )
{
    StackPrint ( self, NULL, false );
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
        ( "/%s\" = %d\n", name, val ) );
}

static void StackPrintBul
    ( const Stack * self, const char * name, bool val )
{
    StackPrint ( self, NULL, false );
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
        ( "/%s\" = %s\n", name, val ? "true" : "false" ) );
}

static void StackPrintStr
    ( const Stack * self, const char * name, const char * val )
{
    StackPrint ( self, NULL, false );
    DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
        ( "/%s\" = \"%s\"\n", name, val ) );
}

static rc_t StackRelease ( Stack * self, bool failed ) {
    assert ( self );

    assert ( self -> i == 0 );

    StackPrint ( self, failed ? "exiting (failure)\n"
                              : "exiting (success)\n", true );

    free ( self -> nodes );

    memset ( self, 0, sizeof * self );

    return 0;
}

static rc_t StackInit ( Stack * self ) {
    size_t nmemb = 1;

    assert ( self );

    memset ( self, 0, sizeof * self );

    self -> nodes = ( Node * ) calloc ( nmemb, sizeof * self -> nodes );
    if ( self -> nodes == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    self -> n = nmemb;

    StackPrintBegin ( self );

    return 0;
}

static void StackPop ( Stack * self ) {
    assert ( self );

    if ( self -> i == 0 )
        return;

    StackPrintEnd ( self );

    -- self -> i;
}

static
rc_t StackPush ( Stack * self, const char * name, int32_t level )
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

    StackPrintBegin ( self );

    return 0;
}

static rc_t StackPushObj ( Stack * self, const char * name )
{   return StackPush ( self, name, -1 ); }

static rc_t StackPushArr ( Stack * self, const char * name ) {
    return StackPush ( self, name, 0 );
}

static rc_t StackArrNext ( Stack * self ) {
    assert ( self && self -> i > 0 );
    assert ( self -> nodes [ self -> i - 1 ] . level >= 0 );

    ++ self -> nodes [ self -> i - 1 ] . level;

    return 0;
}

/********************************** Locations *********************************/

static rc_t LocationsRelease ( Locations * self ) {

#define TYPES_OF_SCHEMAS 3

    rc_t rc = 0;

    int i = 0, j = 0;

    if ( self == NULL )
        return 0;

    for ( j = 0; j < TYPES_OF_SCHEMAS; ++ j ) {
        const VPath ** p = NULL;
        switch ( j ) {
            case 0: p = self -> fasp ; break;
            case 1: p = self -> http ; break;
            case 2: p = self -> https; break;
            default: assert ( 0 );
        }

        assert ( p );

        for ( i = 0; i < MAX_PATHS; ++ i ) {
            if ( p [ i ] == NULL )
                break;
            RELEASE ( VPath, p [ i ] );
        }
    }

    RELEASE ( VPath, self -> local );
    RELEASE ( VPath, self -> cache );
    free ( self -> cType );

    memset ( self, 0, sizeof * self );

    return rc;
}

static bool LocationsEmpty ( const Locations * self ) {
    int i = 0, j = 0;

    if ( self == NULL )
        return true;

    for ( j = 0; j < TYPES_OF_SCHEMAS; ++ j ) {
        const VPath ** p = NULL;
        switch ( j ) {
            case 0: p = ( const VPath** ) self -> fasp ; break;
            case 1: p = ( const VPath** ) self -> http ; break;
            case 2: p = ( const VPath** ) self -> https; break;
            default: assert ( 0 );
        }

        assert ( p );

        for ( i = 0; i < MAX_PATHS; ++ i ) {
            if ( p [ i ] == NULL )
                break;
            return false;
        }
    }

    return true;
}

static rc_t LocationsAddVPath ( Locations * self, const VPath * path ) {
    rc_t rc = 0;

    int i = 0;
    char scheme [ 6 ] = "";
    const VPath ** p = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( path == NULL )
        return 0;

    rc = VPathReadScheme ( path, scheme, sizeof scheme, NULL );
    if ( rc != 0 )
        return rc;

    if      ( strcmp ( scheme, "https" ) == 0 )
        p = self -> https;
    else if ( strcmp ( scheme, "fasp"  ) == 0 )
        p = self -> fasp;
    else if ( strcmp ( scheme, "http"  ) == 0 )
        p = self -> http;
    else
        return 0;

    assert ( p );

    for ( i = 0; i < MAX_PATHS; ++ i ) {
        if ( p [ i ] == NULL ) {
            rc_t rc = VPathAddRef ( path );
            if ( rc != 0 )
                return rc;

            p [ i ] = path;

            return 0;
        }
    }

    return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcInsufficient );
}

static rc_t LocationsAddCache
    ( Locations * self, const VPath * path, rc_t aRc )
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

static rc_t LocationsAddLocal
    ( Locations * self, const VPath * path, rc_t aRc )
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
        rc_t r2 = LocationsRelease ( & self -> elm [ i ] );
        if ( r2 != 0 && rc == 0 )
            rc = r2;
    }

    free ( self -> elm );
    free ( self -> acc );

    memset ( self, 0, sizeof * self );

    return rc;
}

static bool ItemHasLinks ( const Item * self ) {
    uint32_t i = 0;

    if ( self == NULL )
        return false;

    for ( i = 0; i < self -> nElm; ++ i )
        if ( ! LocationsEmpty ( & self -> elm [ i ] ) )
            return true;

    return false;
}

static rc_t ItemAddFormat ( Item * self, const char * cType,
                     Locations ** added )
{
    ESrvFileFormat type = eSFFInvalid;
    int idx = -1;
    Locations * elm = NULL;
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    assert ( added );
    if ( type >= eSFFMax )
        type = eSFFInvalid;
    if ( cType == NULL ) {
        if ( type == eSFFInvalid )
            return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
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
            case eSFFSra     : idx = 0; n = 1; break;
            case eSFFVdbcache: idx = 0; n = 1; break;
            case eSFFMax     : idx = 0; n = 1; break;
            default         :  assert ( 0 );
        }
        self -> elm = ( Locations * ) calloc ( n, sizeof * self -> elm );
        if ( self -> elm == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        self -> nElm = n;
    }
    else {
/*      switch ( type ) {
            case eSFFSra     : idx = 0; break;
            case eSFFVdbcache: idx = 0; break;
            case eSFFMax     : {*/
        uint32_t i =0 ;
        for ( i = 0; i < self -> nElm; ++ i ) {
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
            self -> elm = ( Locations * ) tmp;
            idx = self -> nElm ++;
            elm = & self -> elm [ idx ];
            memset ( elm, 0, sizeof * elm );
        }
/*              break;
            }
            default       :  assert ( 0 );
        }*/
    }
    assert ( idx >= 0 );
    elm = & self -> elm [ idx ];
    if ( elm -> cType == NULL ) {
        elm -> cType = strdup ( cType );
        if ( elm -> cType == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
        elm -> type = type;
    }
    * added = & self -> elm [ idx ];
    return 0;
}

rc_t ItemAddVPath ( Item * self, const char * type, const VPath * path ) {
    rc_t rc = 0;
    Locations * l = NULL;
    rc = ItemAddFormat ( self, type, & l );
    if ( rc == 0 )
        rc = LocationsAddVPath ( l, path );
    return rc;
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
    Locations * elm = NULL;
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
        self -> elm = ( Locations * ) calloc ( n, sizeof * self -> elm );
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
                    self -> elm = ( Locations * ) tmp;
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
    return LocationsAddVCache ( & self -> elm [ idx ], path );
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

    free ( self -> files );
    free ( self -> acc );

    memset ( self, 0, sizeof * self );

    return rc;
}

rc_t ContainerAdd ( Container * self,
                    const char * acc, int64_t id, Item ** newItem )
{
    Item * item = NULL;
    void * tmp = NULL;
    uint32_t i = 0;

    if ( newItem == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * newItem = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    for ( i = 0; i < self -> nFiles; ++ i ) {
        Item * item = & self -> files [ i ];
        assert ( item );
        if ( acc != NULL ) {
            if ( item -> acc != NULL )
                if ( strcmp ( item -> acc, acc ) == 0 )
                    return 0;
        }
        else {
            if ( item -> id != 0 )
                if ( item -> id == id )
                    return 0;
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
        item -> acc = strdup ( acc );
        if ( item -> acc == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }
    else
        item -> id = id;

    * newItem = item;

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

static rc_t Response4Fini ( Response4 * self ) {
    rc_t rc = 0;

    uint32_t i = 0;

    if ( self == NULL )
        return 0;

    for ( i = 0; i < self -> nItems; ++ i ) {
        Container * item = & self -> items [ i ];
        assert ( item );
        RELEASE ( Container, item );
    }

    free ( self -> items );

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
        item -> acc = strdup ( acc );
        if ( item -> acc == NULL )
            return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );
    }
    else {
        assert ( id >= 0 );
        item -> id = id;
    }

    * newItem = item;

    return 0;
}

rc_t Response4AppendUrl ( Response4 * self, const char * url ) {
    rc_t rc = 0;

    VPath * path = NULL;

    Container * box = NULL;
    Item * item = NULL;
    Locations * l = NULL;

    rc = VPathMake ( & path, url );
    if ( rc != 0 )
        return rc;

    rc = Response4AddAccOrId ( self, url, -1, & box );

    if ( rc == 0 )
        rc = ContainerAdd ( box, url, -1, & item );

    if ( rc == 0 )
        rc = ItemAddFormat ( item, "", & l );

    if ( rc == 0 )
        rc = LocationsAddVPath ( l, path );

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

static rc_t IntSet ( int64_t * self, const KJsonValue * node,
              const char * name, Stack * path )
{
    rc_t rc = 0;

    assert ( self );

    if ( node == NULL )
        return 0;

    rc = KJsonGetNumber ( node, self );
    if ( rc != 0 )
        return rc;

    StackPrintInt ( path, name, * self );

    return rc;
}

static rc_t BulSet ( EState * self, const KJsonValue * node,
              const char * name, Stack * path )
{
    rc_t rc = 0;
    bool value = false;

    assert ( self );

    if ( node == NULL )
        return 0;

    rc = KJsonGetBool ( node, & value );
    if ( rc != 0 )
        return rc;

    StackPrintBul ( path, name, value );

    * self = value ? eTrue : eFalse;
    return 0;
}

static rc_t StrSet ( const char ** self, const KJsonValue * node,
              const char * name, Stack * path )
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

    if ( path != NULL )
        StackPrintStr ( path, name, value );

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
    self -> sz  = -1;
    self -> mod = -1;
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
}

static rc_t DataUpdate ( const Data * self, Data * next,
                  const KJsonObject * node, Stack * path )
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

    name = "oldCartObjId";
    IntSet ( & next -> id   , KJsonObjectGetMember ( node, name ), name, path );

    name = "region";
    StrSet ( & next -> reg  , KJsonObjectGetMember ( node, name ), name, path );

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

/******************************** Status ********************************/

static
void StatusInit ( Status * self, int64_t code, const char * msg )
{
    assert ( self );
    self -> code = code;
}

static rc_t StatusSet
    ( Status * self, const KJsonObject * node, Stack * path )
{
    rc_t rc = 0;

    const KJsonValue * value = NULL;
    const KJsonObject * object = NULL;;
    int64_t code = -1;
    const char * name = "status";
    assert ( self );
    StatusInit ( self, -1, NULL );
    if ( node == NULL )
        return rc;
    value = KJsonObjectGetMember ( node, name );
    if ( value == NULL ) {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: cannot find '%s'\n", name ) );
        return RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
    }
    rc = StackPushObj ( path, name );
    if ( rc != 0 )
        return rc;
    object = KJsonValueToObject ( value );
    if ( object == NULL )
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );

    if ( rc == 0 ) {
        name = "code";
        value = KJsonObjectGetMember ( object, name );
        if ( value == NULL ) {
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "... error: cannot find 'status/code'\n" ) );
        }
    }

    if ( rc == 0 ) {
        rc = KJsonGetNumber ( value, & code );

    if ( rc == 0 )
        StackPrintInt ( path, name, code );
        StatusInit ( self, code, NULL );
    }

    StackPop ( path );

    return rc;
}

/********************************** Locations *********************************/

/* "link" is found in JSON: add "link" to Elm (File) using Data from dad */
static rc_t LocationsAddLink ( Locations * self, const KJsonValue * node,
                        const Data * dad, const char ** value )
{
    rc_t rc = 0;

    VPath * path = NULL;

    assert ( self && dad && value );

    if ( node == NULL )
        return 0;

    rc = StrSet ( value, node, NULL, NULL );
    if ( rc != 0 )
        return rc;

    rc = VPathMake ( & path, * value );
    if ( rc != 0 ) {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: invalid 'link': '%s'\n", * value ) );
        return rc;
    }

    rc = LocationsAddVPath ( self, path );

    RELEASE ( VPath, path );

    return rc;
}

/* We detected Item(Run)'s Elm(File)
   and keep scanning it down to find all links */
static rc_t LocationsAddLinks ( Locations * self, const KJsonObject * node,
                         const Data * dad, Stack * path )
{
    rc_t rc = 0;

    bool added = false;

    const KJsonValue * value = NULL;

    const char * name = "alternatives";

    Data data;
    DataUpdate ( dad, & data, node, path );

    assert ( self );

    if ( node == NULL )
        return 0;

    value = KJsonObjectGetMember ( node, "link" );
    if ( value != NULL ) {
        const char * cValue = NULL;
        rc = LocationsAddLink ( self, value, & data, & cValue );
        if ( rc == 0 ) {
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Added 'link' = '%s'\n", cValue ) );
            added = true;
        }
    }

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

        rc = StackPushArr ( path, name );
        if ( rc != 0 )
            return rc;

        for ( i = 0; i < n; ++ i ) {
            rc_t r2 = 0;

            const KJsonObject * object = NULL;

            value = KJsonArrayGetElement ( array, i );
            object = KJsonValueToObject ( value );

            r2 = LocationsAddLinks ( self, object, & data, path );
            if ( r2 != 0 && rc == 0 )
                rc = r2;

            if ( i + 1 < n )
                StackArrNext ( path );
        }

        StackPop ( path );
    }

    else if ( ! added ) {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: cannot find any link\n" ) );
    }

    return rc;
}

/********************************** Item **********************************/

/* We are scanning Item(Run) to find all its Elm-s(Files) -sra, vdbcache, ??? */
static rc_t ItemAddElms ( Item * self, const KJsonObject * node,
                   const Data * dad, Stack * path )
{
    rc_t rc = 0;

    const char * format = NULL;
    const KJsonValue * value = NULL;

    const char * name = "group";

    Data data;
    DataUpdate ( dad, & data, node, path );

    DataGetFormat ( & data, & format );

    value = KJsonObjectGetMember ( node, name );
    if ( value == NULL ) {
        name = "sequence";
        value = KJsonObjectGetMember ( node, name );
    }

    if ( value == NULL ) {
        name = "alternatives";
        value = KJsonObjectGetMember ( node, name );
    }

    if ( data . fmt == NULL && value != NULL ) {
        uint32_t i = 0;

        const KJsonArray * array = KJsonValueToArray ( value );
        uint32_t n = KJsonArrayGetLength ( array );
        rc = StackPushArr ( path, name );
        if ( rc != 0 )
            return rc;
        for ( i = 0; i < n; ++ i ) {
            rc_t r2 = 0;

            const KJsonObject * object = NULL;

            value = KJsonArrayGetElement ( array, i );
            object = KJsonValueToObject ( value );
            r2 = ItemAddElms ( self, object, & data, path );
            if ( r2 != 0 && rc == 0 )
                rc = r2;

            if ( i + 1 < n )
                StackArrNext ( path );
        }

        StackPop ( path );
    }

    else if ( format != NULL ) {
        Locations * elm = NULL;
        rc = ItemAddFormat ( self, format, & elm );
        if ( rc == 0 && elm != NULL ) {
            DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                ( "Adding links to a file...\n" ) );
            rc = LocationsAddLinks ( elm, node, & data, path );
        }
    }

    else {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: file 'format' was not set\n" ) );
    }

    return rc;
}

/********************************** Container *********************************/

/* We are inside of Container (corresponds to request object),
   adding nested Items(runs, gdGaP files) */
static rc_t ContainerAddItem ( Container * self, const KJsonObject * node,
                        const Data * dad, Stack * path )
{
    rc_t rc = 0;

    const char * acc = NULL;
    int64_t id = -1;

    Item * item = NULL;

    Data data;
    DataUpdate ( dad, & data, node, path );

    acc = data . acc;
    id  = data . id;

    rc = ContainerAdd ( self, acc, id, & item );

    if ( rc == 0 && item != NULL ) {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "Adding files to an item...\n" ) );
        rc = ItemAddElms ( item, node, & data, path );
    }

    if ( rc == 0 && ! ItemHasLinks ( item ) )
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );

    return rc;
}

/* We are inside or above of a Container
   and are llooking for Items(runs, gdGaP files) to ddd */
static rc_t Response4AddItems ( Response4 * self, Container * aBox,
    const KJsonObject * node, const Data * dad, Stack * path )
{
    rc_t rc = 0;

    Container * box = aBox;

    Data data;
    DataUpdate ( dad, & data, node, path );

    if ( box == NULL ) {
        const char * acc = data . acc;
        int64_t id = data . id;
        rc = Response4AddAccOrId ( self, acc, id, & box );
        if ( box == NULL ) {
            if ( acc == NULL && id < 0 )
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                    ( "... error: cannot find any 'acc' or 'id'\n" ) );
            return rc;
        }
        rc = StatusSet ( & box -> status, node, path );
    }

    assert ( box );

    if ( ( data . cls != NULL ) && ( strcmp ( data .cls, "run"  ) == 0 
                            || strcmp ( data .cls, "file" ) == 0 ) )
    {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "Adding a '%s' item to container...\n", data .cls ) );
        rc = ContainerAddItem ( box, node, & data, path );
    }
    else {
        const char * name = "sequence";
        const KJsonValue * value = KJsonObjectGetMember ( node, name );
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
            rc = StackPushArr ( path, name );
            if ( rc != 0 )
                return rc;
            for ( i = 0; i < n; ++ i ) {
                rc_t r2 = 0;

                const KJsonObject * jObject = NULL;

                value = KJsonArrayGetElement ( array, i );
                jObject = KJsonValueToObject ( value );
                r2 = Response4AddItems ( self, box, jObject, & data, path );
                if ( r2 != 0 && rc == 0 )
                    rc = r2;

                if ( i + 1 < n )
                    StackArrNext ( path );
            }

            StackPop ( path );
        }
    }

    if ( aBox == NULL && box -> status . code == 200 && box -> nFiles == 0 ) {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: cannot find any container\n" ) );
    }

    return rc;
}

/* Add response document */
static rc_t Response4Init ( Response4 * self, const char * input ) {
    rc_t r2 = 0;

    KJsonValue * root = NULL;

    Stack path;
    Data data;

    const KJsonObject * object = NULL;
    const KJsonValue * value = NULL;

    const char name [] = "sequence";

    StackPrintInput ( input );

    rc_t rc = KJsonValueMake ( & root, input, NULL, 0 );
    if ( rc != 0 ) {
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: invalid JSON\n" ) );
        return rc;
    }

    assert ( self );

    memset ( self, 0, sizeof * self );

    object = KJsonValueToObject ( root );

    DataUpdate ( NULL, & data, object, & path );

    rc = StackInit ( & path );
    if ( rc != 0 )
        return rc;

    value = KJsonObjectGetMember ( object, name );
    if ( value == NULL ) {
        rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
            ( "... error: cannot find '%s'\n", name ) );
    }
    else {
        const KJsonArray * array = KJsonValueToArray ( value );
        if ( array == NULL )
            rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcInvalid );
        else {
            uint32_t n = KJsonArrayGetLength ( array );

            rc = StackPushArr ( & path, name );
            if ( rc != 0 )
                return rc;

            if ( n == 0 ) {
                rc = RC ( rcVFS, rcQuery, rcExecuting, rcDoc, rcIncomplete );
                DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS_JSON ),
                    ( "... error: '%s' is empty\n", name ) );
            }
            else {
                uint32_t i = 0;
                for ( i = 0; i < n; ++ i ) {
                    rc_t r2 = 0;

                    value = KJsonArrayGetElement ( array, i );
                    object = KJsonValueToObject ( value );
                    r2 = Response4AddItems
                        ( self, NULL, object, & data, & path );
                    if ( r2 != 0 && rc == 0 )
                        rc = r2;

                    if ( i + 1 < n )
                        StackArrNext ( & path );
                }
            }
            StackPop ( & path );

        }
    }

    KJsonValueWhack ( root );

    if ( rc != 0 )
        Response4Fini ( self );

    r2 = StackRelease ( & path, rc != 0 );
    if ( r2 != 0 && rc == 0 )
        rc = r2;

    atomic32_set ( & self -> refcount, 1 );

    return rc;
}

rc_t Response4MakeEmpty ( Response4 ** self ) {
    assert ( self );

    * self = ( Response4 * ) calloc ( 1, sizeof ** self );
    if ( * self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcMemory, rcExhausted );

    return 0;
}

rc_t Response4Make ( Response4 ** self, const char * input ) {
    rc_t rc = 0;

    Response4 * r = NULL;

    assert ( self );

    rc = Response4MakeEmpty ( & r );
    if ( rc != 0 )
        return rc;

    rc = Response4Init ( r, input );
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

    rc = Response4Init ( & r -> r, input );
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

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

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

rc_t KSrvRespFileGetId ( const KSrvRespFile * self, const char ** acc ) {
    assert ( self && self -> item && acc );

    * acc = self -> item -> acc;

    return 0;
}

rc_t KSrvRespFileGetCache ( const KSrvRespFile * self,
                            const VPath ** path )
{
    rc_t rc = 0;

    assert ( self && self -> file && path );

    * path = NULL;

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

    assert ( self && self -> file && path );

    * path = NULL;

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
    rc = LocationsAddCache ( self -> file, path, aRc );
    RELEASE ( VPath, path );

    aRc = VPathSetGetLocal ( localAndCache, & path );
    r2 = LocationsAddLocal ( self -> file, path, aRc );
    if ( r2 != 0 && rc == 0 )
        rc = r2;
    RELEASE ( VPath, path );

    return rc;
}

rc_t KSrvRespFileMakeIterator ( const KSrvRespFile * self,
    VRemoteProtocols protocols, KSrvRespFileIterator ** it )
{
    rc_t rc = 0;

    VRemoteProtocols protocol = protocols;

    const VPath * const  * path = NULL;
    KSrvRespFileIterator * p = NULL;

    if ( it == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * it = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( protocol == eProtocolDefault )
        protocol = DEFAULT_PROTOCOLS;

    assert ( protocol != 0 );

    for ( ; protocol != 0; protocol >>= 3 ) {
        switch ( protocol & eProtocolMask ) {
            case eProtocolFasp : path = self -> file -> fasp ; break;
            case eProtocolHttp : path = self -> file -> http ; break;
            case eProtocolHttps: path = self -> file -> https; break;
            default:
                return RC ( rcVFS, rcQuery, rcExecuting, rcSchema, rcInvalid );
        }
        assert ( path );
        if ( path [ 0 ] != NULL )
            break;
    }

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
