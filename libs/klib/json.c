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

#include <klib/extern.h>

#include <klib/json.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/container.h>
#include <klib/namelist.h>

#include "json-lex.h"
#include "json-tokens.h"
#include "json-priv.h"

enum jsType
{
    jsObject,
    jsArray,
    jsString,
    jsUint,
    jsInt,
    jsDouble,
    jsBool,
    jsNull
};

/* NameValue */

typedef struct NameValue NameValue;
struct NameValue
{
    BSTNode node;
    char * name;
    KJsonValue * value;
};

static rc_t MakeNameValue ( NameValue ** p_val, const char * p_name, size_t p_name_size, KJsonValue * p_value )
{
    rc_t rc;
    NameValue * ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> name = string_dup ( p_name, p_name_size );
        if ( ret -> name != NULL )
        {
            ret -> value = p_value;
            * p_val = ret;
            rc = 0;
        }
        else
        {
            free ( ret );
            return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
        }
    }
    else
    {
        return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
    }
    return rc;
}

static void CC NameValueWhack ( BSTNode * p_n, void * p_data )
{
    NameValue * self = ( NameValue * ) p_n;
    free ( self -> name );
    KJsonValueWhack ( self -> value );
    free ( self );
}

int64_t CC NameValueSort ( const BSTNode * p_item, const BSTNode * p_n )
{
    const NameValue *a = ( const NameValue* ) p_item;
    const NameValue *b = ( const NameValue* ) p_n;
    size_t size = string_size ( a -> name );
    return string_cmp ( a -> name, size,
                        b -> name, string_size ( b -> name ),
                        string_len ( a -> name, size ) );
}

typedef struct AddKeyBlock AddKeyBlock;
struct AddKeyBlock
{
    VNamelist * names;
    rc_t rc;
};

bool CC NameValueAddKey ( BSTNode * p_n, void * p_data )
{
    const NameValue *a = ( const NameValue* ) p_n;
    AddKeyBlock * bl = ( AddKeyBlock * ) p_data;
    bl -> rc = VNamelistAppend ( bl -> names, a -> name );
    return bl -> rc != 0;
}

struct KJsonValue
{
    uint32_t type; /* enum jsType */
    union
    {
        char * str;
    } u;
};

struct KJsonObject
{
    KJsonValue dad;
    BSTree members; /* NameValue */
};

struct KJsonArray
{
    KJsonValue dad;
    /*TODO*/
};

/* Public API, read only */

rc_t CC KJsonMake ( KJsonObject ** p_root, const char * p_input, char * p_error, size_t p_error_size )
{
    rc_t rc;
    if ( p_root == NULL )
    {
        rc = RC ( rcCont, rcNode, rcParsing, rcSelf, rcNull );
    }
    else if ( p_input == NULL )
    {
        rc = RC ( rcCont, rcNode, rcParsing, rcParam, rcNull );
    }
    else
    {
        JsonScanBlock sb;
        JsonScan_yylex_init ( & sb, p_input, string_size ( p_input ) );
        if ( Json_parse ( p_root, & sb ) == 0 )
        {
            rc = 0;
        }
        else
        {
            if ( p_error != NULL )
            {
                if ( string_copy ( p_error, p_error_size, sb . error, string_size ( sb . error ) ) == p_error_size )
                {
                    p_error [ p_error_size - 1 ] = 0;
                }
            }
            rc = RC ( rcCont, rcNode, rcParsing, rcFormat, rcUnrecognized );
        }
        JsonScan_yylex_destroy ( & sb );
    }
    return rc;
}

void CC KJsonWhack ( KJsonObject * p_root )
{
    if ( p_root != NULL )
    {
        BSTreeWhack ( & p_root -> members, NameValueWhack, NULL);
        free ( p_root );
    }
}

void KJsonArrayWhack ( KJsonArray * p_arr )
{
    free ( p_arr );
}

bool CC KJsonIsObject ( const KJsonValue * node )
{
    return node != NULL && node -> type == jsObject;
}

bool CC KJsonIsArray ( const KJsonValue * node )
{
    return false;
}

bool CC KJsonIsString ( const KJsonValue * node )
{
    return false;
}

bool CC KJsonIsNumber ( const KJsonValue * node )
{
    return false;
}

bool CC KJsonIsTrue ( const KJsonValue * node )
{
    return false;
}

bool CC KJsonIsFalse ( const KJsonValue * node )
{
    return false;
}

bool CC KJsonIsNull ( const KJsonValue * node )
{
    return false;
}

/*
rc_t CC KJsonToString ( const KJsonValue * root, char * error, size_t error_size );

rc_t CC KJsonGetString ( const KJsonValue * node, const char ** value );
rc_t CC KJsonGetNumber ( const KJsonValue * node, uint64_t * value );
rc_t CC KJsonGetDouble ( const KJsonValue * node, double * value );

*/

const KJsonObject * CC  KJsonValueToObject ( const KJsonValue * p_value )
{
    if ( p_value == NULL || p_value -> type != jsObject )
    {
        return NULL;
    }

    return ( const KJsonObject * ) p_value;
}

const KJsonValue * CC   KJsonObjectToValue ( const KJsonObject * p_object )
{
    if ( p_object == NULL )
    {
        return NULL;
    }

    return & p_object -> dad;
}

rc_t CC KJsonObjectGetNames ( const KJsonObject * p_node, VNamelist * p_names )
{
    rc_t rc;
    if ( p_node == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( p_names == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcParam, rcNull );
    }
    else
    {
        AddKeyBlock bl;
        bl . names = p_names;
        bl . rc = 0;
        BSTreeDoUntil ( & p_node -> members, false, NameValueAddKey, & bl );
        rc = bl . rc;
    }
    return rc;
}

const KJsonValue * CC KJsonObjectGetValue ( const KJsonObject * p_node, const char * p_name )
{
    if ( p_node == NULL || p_name == 0 )
    {
        return 0;
    }

    return 0;
}

const KJsonArray * CC KJsonValueToArray ( const KJsonValue * p_value )
{
    if ( p_value == NULL || p_value -> type != jsArray )
    {
        return NULL;
    }

    return ( const KJsonArray * ) p_value;
}

const KJsonValue * CC KJsonArrayToValue ( const KJsonArray * p_array )
{
    if ( p_array == NULL )
    {
        return NULL;
    }

    return & p_array -> dad;
}

/*
uint32_t CC KJsonArrayLength ( const KJsonArray * node );
KJsonValue * CC KJsonArrayGetElement ( const KJsonArray * node, uint32_t index );
*/

/* Construction methods */

rc_t KJsonMakeObject ( KJsonObject ** obj )
{
    assert ( obj != NULL );
    KJsonObject * ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> dad . type = jsObject;
        BSTreeInit ( & ret -> members );
        * obj = ret;
        return 0;
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

rc_t KJsonObjectAddMember ( KJsonObject * p_obj, const char * p_name, size_t p_name_size, KJsonValue * p_value )
{
    rc_t rc;
    NameValue * nv;

    assert ( p_obj != NULL && p_name != NULL && p_value != NULL );

    rc = MakeNameValue ( & nv, p_name, p_name_size, p_value );
    if ( rc == 0 )
    {
        rc = BSTreeInsertUnique ( & p_obj -> members, & nv -> node, NULL, NameValueSort );
    }
    return rc;
}

rc_t KJsonMakeArray ( KJsonArray ** obj )
{
    assert ( obj != NULL );
    KJsonArray * ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> dad . type = jsArray;
        * obj = ret;
        return 0;
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

rc_t KJsonArrayAddElement ( KJsonArray * p_arr, KJsonValue * p_element )
{
    assert ( p_arr != NULL && p_element != NULL );
    return 0;
}

rc_t KJsonMakeString ( KJsonValue ** p_val, const char * p_value, size_t p_size )
{
    assert ( p_val != NULL && p_value != NULL );
    KJsonValue * ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsString;
        ret -> u . str = string_dup ( p_value, p_size );
        if ( ret -> u . str != NULL )
        {
            * p_val = ret;
            return 0;
        }
        free ( ret );
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

/*
rc_t KJsonMakeNumber ( KJsonValue ** obj, const char * value, size_t p_size );
rc_t KJsonMakeBool ( KJsonValue ** obj, bool value );
*/

rc_t KJsonMakeNull ( KJsonValue ** p_val )
{
    assert ( p_val != NULL );
    KJsonValue * ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsNull;
        * p_val = ret;
        return 0;
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

void KJsonValueWhack ( KJsonValue * p_value )
{
    if ( p_value != NULL )
    {
        switch ( p_value -> type )
        {
        case jsString:
        {
            free ( p_value -> u . str );
            free ( p_value );
            break;
        }
        case jsObject:
        {
            KJsonWhack ( ( KJsonObject * ) p_value );
            break;
        }
        case jsArray:
        {
            KJsonArrayWhack ( ( KJsonArray * ) p_value );
            break;
        }
        default:
            free ( p_value );
            break;
        }
    }
}

