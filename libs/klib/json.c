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

#include <limits.h>
#include <errno.h>

#include <strtol.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/container.h>
#include <klib/namelist.h>
#include <klib/vector.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>

#include "json-lex.h"
#include "json-tokens.h"
#include "json-priv.h"

/* copy, convert the escapes and NUL-terminate */
static rc_t CopyAndUnescape ( const char * p_value, size_t p_size, char * p_target, size_t p_targetSize );

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
    if ( ret == NULL )
        rc = RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
    else
    {
        ret -> name = malloc ( p_name_size + 1 );
        if ( ret -> name == NULL )
            rc = RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
        else
        {
            rc = CopyAndUnescape ( p_name, p_name_size, ret -> name, p_name_size + 1 );
            if ( rc == 0 )
            {
                ret -> value = p_value;
                * p_val = ret;
                return 0;
            }
            free ( ret -> name );
        }
        free ( ret );
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

static
int StringCmp( const char * a, const char * b )
{
    size_t sizeA = string_size ( a );
    size_t sizeB = string_size ( b );
    return string_cmp ( a, sizeA,
                        b, sizeB,
                        sizeA < sizeB ? sizeB : sizeA );
}

int64_t CC NameValueSort ( const BSTNode * p_item, const BSTNode * p_n )
{
    const NameValue *a = ( const NameValue* ) p_item;
    const NameValue *b = ( const NameValue* ) p_n;
    return StringCmp ( a -> name, b -> name );
}

int64_t CC NameValueCompare ( const void * p_item, const BSTNode * p_n )
{
    const char * a = ( const char * ) p_item;
    const NameValue * b = ( const NameValue* ) p_n;
    return StringCmp ( a, b -> name );
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
    uint32_t    type; /* enum jsType */
    union
    {
        char *      str;
        bool        boolean;
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
    Vector elements; /* KJsonValue* */
};

/* Public API, read only */

LIB_EXPORT
rc_t CC
KJsonValueMake ( KJsonValue ** p_root, const char * p_input, char * p_error, size_t p_error_size )
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

            // sb . error is NUL-terminated, so strstr is safe
            if ( strstr ( sb . error, "unexpected end of source" ) != NULL )
            {
                rc = RC ( rcCont, rcNode, rcParsing, rcDoc, rcIncomplete );
            }
            else
            {
                rc = RC ( rcCont, rcNode, rcParsing, rcFormat, rcUnrecognized );
            }
        }
        JsonScan_yylex_destroy ( & sb );
    }
    return rc;
}

void CC KJsonObjectWhack ( KJsonObject * p_root )
{
    if ( p_root != NULL )
    {
        BSTreeWhack ( & p_root -> members, NameValueWhack, NULL);
        free ( p_root );
    }
}

static
void CC
WhackValue( void * item, void * data )
{
    KJsonValueWhack ( ( KJsonValue * ) item );
}

void KJsonArrayWhack ( KJsonArray * p_arr )
{
    if ( p_arr != NULL )
    {
        VectorWhack ( & p_arr -> elements, WhackValue, NULL );
        free ( p_arr );
    }
}

LIB_EXPORT
rc_t CC
KJsonGetString ( const KJsonValue * p_node, const char ** p_value )
{
    rc_t rc;
    if ( p_node == NULL )
    {
        rc = RC ( rcCont, rcNode, rcAccessing, rcSelf, rcNull );
    }
    else if ( p_value == NULL )
    {
        rc = RC ( rcCont, rcNode, rcAccessing, rcParam, rcNull );
    }
    else
    {
        switch ( p_node -> type )
        {
        case jsString:
        case jsNumber:
            * p_value = p_node -> u . str;
            rc = 0;
            break;
        default:
            rc = RC ( rcCont, rcNode, rcAccessing, rcType, rcIncorrect );
            break;
        }
    }
    return rc;
}

LIB_EXPORT
const KJsonObject * CC  KJsonValueToObject ( const KJsonValue * p_value )
{
    if ( p_value == NULL || p_value -> type != jsObject )
    {
        return NULL;
    }

    return ( const KJsonObject * ) p_value;
}

LIB_EXPORT
const KJsonValue * CC
KJsonObjectToValue ( const KJsonObject * p_object )
{
    if ( p_object == NULL )
    {
        return NULL;
    }

    return & p_object -> dad;
}

LIB_EXPORT
rc_t CC
KJsonObjectGetNames ( const KJsonObject * p_node, VNamelist * p_names )
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

LIB_EXPORT
const KJsonValue * CC
KJsonObjectGetMember ( const KJsonObject * p_node, const char * p_name )
{
    if ( p_node == NULL || p_name == 0 )
    {
        return 0;
    }
	else
	{
		const BSTNode * node = BSTreeFind ( & p_node -> members, p_name, NameValueCompare );
		if ( node == NULL )
		{
			return NULL;
		}
		return ( ( const NameValue * ) node ) -> value;
	}
}

LIB_EXPORT
const KJsonArray * CC
KJsonValueToArray ( const KJsonValue * p_value )
{
    if ( p_value == NULL || p_value -> type != jsArray )
    {
        return NULL;
    }

    return ( const KJsonArray * ) p_value;
}

LIB_EXPORT
const KJsonValue * CC
KJsonArrayToValue ( const KJsonArray * p_array )
{
    if ( p_array == NULL )
    {
        return NULL;
    }

    return & p_array -> dad;
}

/* Construction methods */

rc_t KJsonMakeObject ( KJsonObject ** obj )
{
    KJsonObject * ret;
    assert ( obj != NULL );
    ret = calloc ( 1, sizeof * ret );
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
        if ( rc != 0 )
        {
            NameValueWhack ( & nv -> node, NULL );
        }
    }
    else
    {
        KJsonValueWhack ( p_value );
    }
    return rc;
}

rc_t KJsonMakeArray ( KJsonArray ** obj )
{
    KJsonArray * ret;
    assert ( obj != NULL );
    ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> dad . type = jsArray;
        VectorInit ( & ret -> elements, 0, 16 );
        * obj = ret;
        return 0;
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

LIB_EXPORT
uint32_t CC
KJsonArrayGetLength ( const KJsonArray * p_node )
{
    if ( p_node == NULL )
    {
        return 0;
    }
    return VectorLength ( & p_node -> elements );
}

rc_t KJsonArrayAddElement ( KJsonArray * p_arr, KJsonValue * p_element )
{
    assert ( p_arr != NULL && p_element != NULL );
    return VectorAppend ( & p_arr -> elements, NULL, p_element );
}

LIB_EXPORT
const KJsonValue * CC
KJsonArrayGetElement ( const KJsonArray * p_node, uint32_t p_index )
{
    if ( p_node == NULL )
    {
        return NULL;
    }
    return ( const KJsonValue * ) VectorGet( & p_node -> elements, p_index );
}

/* hex_to_int
 *  where 'c' is known to be hex
 */
static
unsigned int
hex_to_int ( char c )
{
    int i = c - '0';
    if ( c > '9' )
    {
        if ( c < 'a' )
            i = c - 'A' + 10;
        else
            i = c - 'a' + 10;
    }

    assert ( i >= 0 && i < 16 );
    return i;
}

static
rc_t
CopyAndUnescape ( const char * p_value, size_t p_size, char * p_target, size_t p_targetSize )
{   /* copy, convert the escapes and NUL-terminate */
    uint32_t out = 0;
    uint32_t i = 0;
    assert ( p_size < p_targetSize );
    while ( i < p_size )
    {
        if ( p_value [ i ] == '\\' )
        {
            ++i;
            switch ( p_value [ i ] )
            {
            case 'u':
                assert ( i + 4 < p_size );
                {   /* treat 4-digit hex code as UTF16 */
                    uint64_t u64 = hex_to_int ( p_value [ i + 1 ]);
                    u64 <<= 4;
                    u64 += hex_to_int ( p_value [ i + 2 ]);
                    u64 <<= 4;
                    u64 += hex_to_int ( p_value [ i + 3 ]);
                    u64 <<= 4;
                    u64 += hex_to_int ( p_value [ i + 4 ]);

                    if ( u64 >= 0xD800 && u64 <= 0xDFFF )
                    {   /* require a valid surrogate pair */
                        if ( i + 10 < p_size && p_value [ i + 5 ] == '\\' && p_value [ i + 6 ] == 'u' )
                        {
                            uint64_t high = u64;
                            uint64_t low = hex_to_int ( p_value [ i + 7 ]);
                            low <<= 4;
                            low += hex_to_int ( p_value [ i + 8 ]);
                            low <<= 4;
                            low += hex_to_int ( p_value [ i + 9 ]);
                            low <<= 4;
                            low += hex_to_int ( p_value [ i + 10 ]);
                            if ( low >= 0xDC00 && low <= 0xDFFF )
                            {
                                u64 = 0x10000;
                                u64 += ( high & 0x03FF ) << 10;
                                u64 += ( low & 0x03FF );
                                i += 6;
                            }
                            else
                            {
                                return RC ( rcCont, rcNode, rcParsing, rcString, rcInvalid );
                            }
                        }
                        else
                        {
                            return RC ( rcCont, rcNode, rcParsing, rcString, rcInvalid );
                        }
                    }

                    {
                        int ch_len = utf32_utf8 ( p_target + out, p_target + p_size, (uint32_t)u64 );
                        assert ( ch_len > 0 );
                        i += 4;
                        out += ch_len - 1;
                    }
                }
                break;
            case '\\':
            case '/':
            case '"':
                p_target [ out ] = p_value [ i ];
                break;
            case 'b':
                p_target[ out ] = '\b';
                break;
            case 'f':
                p_target [ out ] = '\f';
                break;
            case 'n':
                p_target [ out ] = '\n';
                break;
            case 'r':
                p_target [ out ] = '\r';
                break;
            case 't':
                p_target [ out ] = '\t';
                break;
            }
        }
        else
        {
            p_target [ out ] = p_value [ i ];
        }
        ++i;
        ++ out;
    }
    p_target [ out ] = 0;
    return 0;
}

rc_t KJsonMakeString ( KJsonValue ** p_val, const char * p_value, size_t p_size )
{
    KJsonValue * ret;
    assert ( p_val != NULL && p_value != NULL );
    ret = malloc ( sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsString;
        ret -> u . str = malloc ( p_size + 1 );
        if ( ret -> u . str != NULL )
        {
            rc_t rc = CopyAndUnescape ( p_value, p_size, ret -> u . str, p_size + 1 );
            if ( rc == 0 )
            {
                * p_val = ret;
                return 0;
            }
            KJsonValueWhack ( ret );
            return rc;
        }
        free ( ret );
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

/*
rc_t KJsonMakeBool ( KJsonValue ** obj, bool value );
*/

rc_t KJsonMakeNull ( KJsonValue ** p_val )
{
    KJsonValue * ret;
    assert ( p_val != NULL );
    ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsNull;
        * p_val = ret;
        return 0;
    }
    return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );
}

rc_t KJsonMakeBool ( KJsonValue ** p_val, bool p_bool )
{
    KJsonValue * ret;
    assert ( p_val != NULL );
    ret = calloc ( 1, sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsBool;
        ret -> u . boolean = p_bool;
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
        case jsNumber:
        {
            free ( p_value -> u . str );
            free ( p_value );
            break;
        }
        case jsObject:
        {
            KJsonObjectWhack ( ( KJsonObject * ) p_value );
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

rc_t KJsonMakeNumber ( KJsonValue ** p_val, const char * p_value, size_t p_size )
{
    KJsonValue * ret;
    assert ( p_val != NULL && p_value != NULL );
    ret = malloc ( sizeof * ret );
    if ( ret != NULL )
    {
        ret -> type = jsNumber;
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

LIB_EXPORT
rc_t CC
KJsonGetNumber ( const KJsonValue * p_node, int64_t * p_value )
{
    rc_t rc;
    if ( p_node == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( p_value == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcParam, rcNull );
    }
    else if ( p_node -> type != jsNumber )
    {
        rc = RC ( rcCont, rcNode, rcAccessing, rcType, rcIncorrect );
    }
    else
    {
        char * endptr;
        int64_t value;
        errno = 0;
        value = strtoi64 ( p_node -> u . str, & endptr, 10 );
        if ( errno == ERANGE )
        {
            rc = RC ( rcCont, rcNode, rcAccessing, rcSize, rcExcessive );
        }
        else if ( *endptr != 0 )
        {
            rc = RC ( rcCont, rcNode, rcAccessing, rcFormat, rcIncorrect );
        }
        else
        {
            * p_value = value;
            rc = 0;
        }
    }
    return rc;
}

LIB_EXPORT
rc_t CC
KJsonGetDouble ( const KJsonValue * p_node, double * p_value )
{
    rc_t rc;
    if ( p_node == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( p_value == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcParam, rcNull );
    }
    else if ( p_node -> type != jsNumber )
    {
        rc = RC ( rcCont, rcNode, rcAccessing, rcType, rcIncorrect );
    }
    else
    {
        char * endptr;
        double value;
        errno = 0;
        value = strtod ( p_node -> u . str, & endptr );
        if ( errno == ERANGE )
        {
            rc = RC ( rcCont, rcNode, rcAccessing, rcSize, rcExcessive );
        }
        else
        {
            * p_value = value;
            rc = 0;
        }
    }
    return rc;
}

LIB_EXPORT
rc_t CC
KJsonGetBool ( const KJsonValue * p_node, bool * p_value )
{
    rc_t rc;
    if ( p_node == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( p_value == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcParam, rcNull );
    }
    else if ( p_node -> type != jsBool )
    {
        rc = RC ( rcCont, rcNode, rcAccessing, rcType, rcIncorrect );
    }
    else
    {
        * p_value = p_node -> u . boolean;
        rc = 0;
    }
    return rc;
}

LIB_EXPORT
enum jsType CC
KJsonGetValueType ( const KJsonValue * p_value )
{
    if ( p_value == NULL )
    {
        return jsInvalid;
    }
    return p_value -> type;
}

// Conversion to a JSON formatted string

typedef struct PrintData PrintData;
struct PrintData
{
    KDataBuffer *   output;
    size_t          increment;
    size_t          offset;
    rc_t            rc;
    const void *    last; /* last element in a container */
    bool            pretty;
    uint32_t        indentTabs;
};

static rc_t ObjectToJson ( const KJsonObject * root, PrintData * pd );
static rc_t ArrayToJson ( const KJsonArray * node, PrintData * pd );

static
rc_t IncreaseBuffer ( PrintData * p_data )
{
    assert ( p_data != NULL );
    return KDataBufferResize ( p_data -> output, KDataBufferBytes ( p_data -> output ) + p_data -> increment );
}

static
rc_t
PrintWithSize ( PrintData * p_pd, const char * p_text, size_t p_size )
{
    /* grow the buffer if necessary */
    rc_t rc = 0;
    while ( rc == 0 && p_pd -> offset + p_size >= KDataBufferBytes ( p_pd -> output ) )
    {
        rc = IncreaseBuffer ( p_pd );
    }

    if ( rc == 0 )
    {
        size_t num_writ;
        rc = string_printf ( ( char * ) p_pd -> output -> base + p_pd -> offset, KDataBufferBytes ( p_pd -> output ) - p_pd -> offset, & num_writ, "%.*s", p_size, p_text );
        if ( rc == 0 )
        {
            p_pd -> offset += num_writ;
        }
    }
    return rc;
}

static
rc_t
Print( PrintData * p_pd, const char * p_text )
{
    return PrintWithSize ( p_pd, p_text, string_size ( p_text ) );
}

static
rc_t
PrintNewLine( PrintData * p_pd )
{
    rc_t rc = Print ( p_pd, "\n" );

    uint32_t i = p_pd -> indentTabs;
    while ( rc == 0 && i > 0 )
    {
        rc = Print ( p_pd, "\t" );
        --i;
    }

    return rc;
}

static
rc_t
PrintString ( PrintData * p_pd, const char * p_str )
{
    const char * begin = p_str;
    const char * end = p_str + string_size ( p_str );
    rc_t rc = 0;
    while ( rc == 0 && begin < end )
    {
        uint32_t ch;
        int bytes = utf8_utf32 ( & ch, begin, end );
        assert ( bytes > 0 );
        if ( ch < 32 )
        {
            switch (ch)
            {
            case 8:
                rc = Print ( p_pd, "\\b");
                break;
            case 9:
                rc = Print ( p_pd, "\\t");
                break;
            case 10:
                rc = Print ( p_pd, "\\n");
                break;
            case 13:
                rc = Print ( p_pd, "\\r");
                break;
            default:
                {
                    const char to_hex[16] = "0123456789abcdef";
                    char hex [7] = { '\\', '\\', 'u' };
                    hex [3] = to_hex [ ( ch >> 24 ) & 0xff ];
                    hex [4] = to_hex [ ( ch >> 16 ) & 0xff ];
                    hex [5] = to_hex [ ( ch >> 8 ) & 0xff ];
                    hex [6] = to_hex [ ch & 0xff ];
                    rc = PrintWithSize ( p_pd, hex, 7);
                    break;
                }
            }
        }
        else if ( ch > 255 )
        {   /* UTF-8 encoding; copy bytes from input */
            rc = PrintWithSize ( p_pd, begin, bytes );
        }
        else
        {   /* non-control ASCII */
            switch (ch)
            {
            case '\\': rc = Print ( p_pd, "\\\\" ); break;
            case '/':  rc = Print ( p_pd, "\\/" ); break;
            case '"':  rc = Print ( p_pd, "\\\"" ); break;
            default:
                rc = PrintWithSize ( p_pd, begin, 1 );
                break;
            }
        }

        begin += bytes;
    }
    return rc;
}

static
rc_t
ValueToJson ( const KJsonValue * p_value, PrintData * p_pd )
{
    size_t saved_offset = p_pd -> offset;
    rc_t rc;
    switch ( p_value -> type )
    {
    case jsString:
        rc = Print ( p_pd, "\"" );
        if ( rc == 0 ) rc = PrintString ( p_pd, p_value -> u . str );
        if ( rc == 0 ) rc = Print ( p_pd, "\"" );
        break;
    case jsNumber:
        rc = Print ( p_pd, p_value -> u . str );
        break;
    case jsBool:
        rc = Print ( p_pd, p_value -> u . boolean ? "true" : "false" );
        break;
    case jsNull:
        rc = Print ( p_pd, "null");
        break;
    case jsObject:
        {
            const KJsonObject * obj = KJsonValueToObject ( p_value );
            assert ( obj != 0 );
            rc = ObjectToJson ( obj, p_pd );
        }
        break;
    case jsArray:
        {
            const KJsonArray * arr = KJsonValueToArray ( p_value );
            assert ( arr != 0 );
            rc = ArrayToJson ( arr, p_pd );
        }
        break;
    default:
        assert ( false );
        break;
    }

    if ( rc != 0 )
    {
        p_pd -> offset = saved_offset;
    }
    return rc;
}

static
void CC
NameValueToJson ( BSTNode *n, void *data )
{
    const NameValue * node = (const NameValue *) n;
    PrintData * pd = (PrintData *) data;
    size_t saved_offset = pd -> offset;

    rc_t rc = Print ( pd, "\"" );
    if ( rc == 0 ) rc = Print ( pd, node -> name );
    if ( rc == 0 ) rc = Print ( pd, "\"" );
    if ( rc == 0 ) rc = Print ( pd, pd -> pretty ? " : " : ":" );
    if ( rc == 0 ) rc = ValueToJson ( node -> value, pd );
    if ( rc == 0 )
    {
        if ( n == pd -> last )
        {   /* restore indent before printing the closing '}', so that it aligns with the member name */
            -- pd -> indentTabs;
        }
        else
        {
            rc = Print ( pd, "," );
        }
    }
    if ( rc == 0 && pd -> pretty ) rc = PrintNewLine ( pd );

    pd -> rc = rc;
    if ( rc != 0 )
    {
        pd -> offset = saved_offset;
    }
}

static
rc_t
ObjectToJson ( const KJsonObject * p_root, PrintData * pd )
{
    rc_t rc;
    size_t saved_offset;
    const void * saved_last;

    assert ( p_root != NULL && pd != NULL );
    saved_last = pd -> last;
    saved_offset = pd -> offset;

    rc = Print ( pd, "{" );
    ++ pd -> indentTabs;
    if ( rc == 0 && pd -> pretty ) rc = PrintNewLine ( pd );
    if ( rc == 0 )
    {
        pd -> last = BSTreeLast ( & p_root -> members );
        BSTreeForEach ( & p_root -> members, false, NameValueToJson, pd);
    }
    if ( rc == 0 ) rc = Print ( pd, "}" );

    pd -> last = saved_last;
    if ( rc != 0 )
    {
        pd -> offset = saved_offset;
    }
    return rc;
}

static
bool CC
ArrayElementToJson ( void * item, void *data )
{
    const KJsonValue * value =  (const KJsonValue *) item;
    PrintData * pd = (PrintData *) data;

    pd -> rc = ValueToJson ( value, pd );
    if ( pd -> rc == 0 )
    {
        if ( value == pd -> last )
        {   /* restore indent before printing the closing ']' */
            -- pd -> indentTabs;
        }
        else
        {
            pd -> rc = Print ( pd, "," );
        }
    }
    if ( pd -> pretty ) pd -> rc = PrintNewLine ( pd );
    return pd -> rc != 0;
}

static
rc_t
ArrayToJson ( const KJsonArray * p_node, PrintData * pd )
{
    rc_t rc;
    size_t saved_offset;
    const void * saved_last;
    assert ( p_node != NULL && pd != NULL );
    saved_last = pd -> last;
    saved_offset = pd -> offset;

    rc = Print ( pd, "[" );
    ++ pd -> indentTabs;
    if ( pd -> pretty ) rc = PrintNewLine ( pd );
    if ( rc == 0 )
    {
        pd -> last = VectorLast ( & p_node -> elements );
        VectorDoUntil ( & p_node -> elements, false, ArrayElementToJson, pd );
    }
    if ( rc == 0 ) rc = Print ( pd, "]" );

    pd -> last = saved_last;
    if ( rc != 0 )
    {
        pd -> offset = saved_offset;
    }
    return rc;
}

LIB_EXPORT
rc_t CC
KJsonToJsonString ( const KJsonValue * p_root, struct KDataBuffer * p_output, size_t p_increment, bool p_pretty )
{
    rc_t rc;
    if ( p_root == NULL )
    {
        rc = RC ( rcCont, rcNode, rcProcessing, rcSelf, rcNull );
    }
    else if ( p_output == NULL )
    {
        rc = RC ( rcCont, rcNode, rcReading, rcParam, rcNull );
    }
    else
    {
        rc = KDataBufferMake ( p_output, 8, p_increment == 0 ? 256 : p_increment );
        if ( rc == 0 )
        {
            PrintData pd;
            pd . output = p_output;
            pd . increment = p_increment == 0 ? 1024 : p_increment;
            pd . offset = 0;
            pd . rc = 0;
            pd . last = NULL;
            pd . pretty = p_pretty;
            pd . indentTabs = 0;
            rc = ValueToJson ( p_root, & pd );
        }
    }
    return rc;
}
