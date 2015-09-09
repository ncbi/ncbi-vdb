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
#include <kapp/main.h>
#include <kapp/args.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/vector.h>
#include <sysalloc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define OPTION_COUNT    "count"
#define ALIAS_COUNT     "c"

static const char * count_usage[]     = { "how many items", NULL };

OptDef JudyTestOptions[] =
{
/*    name             alias        fkt.  usage-txt,  cnt, needs value, required */
    { OPTION_COUNT,    ALIAS_COUNT, NULL, count_usage,  1, true,  false }
};

const char UsageDefaultName[] = "judytest";

rc_t CC UsageSummary ( const char * progname )
{
    return KOutMsg ("\n"
                    "Usage:\n"
                    "  %s [options]\n"
                    "\n", progname);
}

rc_t CC Usage ( const Args * args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if ( args == NULL )
        rc = RC ( rcApp, rcArgv, rcAccessing, rcSelf, rcNull );
    else
        rc = ArgsProgram ( args, &fullpath, &progname );

    if ( rc )
        progname = fullpath = UsageDefaultName;

    UsageSummary ( progname );

    KOutMsg ( "Options:\n" );

    HelpOptionLine ( ALIAS_COUNT, OPTION_COUNT, "count", count_usage );

    HelpOptionsStandard ();
    HelpVersion ( fullpath, KAppVersion() );
    return rc;
}


/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void )
{
    return 0x01010000;
}


static uint32_t get_int_option( const Args *my_args,
                                const char *name,
                                const uint32_t def )
{
    uint32_t count, res = def;
    rc_t rc = ArgsOptionCount( my_args, name, &count );
    if ( ( rc == 0 )&&( count > 0 ) )
    {
        const char *s;
        rc = ArgsOptionValue( my_args, name, 0,  (const void **)&s );
        if ( rc == 0 ) res = atoi( s );
    }
    return res;
}


typedef struct random_numbers
{
    uint32_t count;
    uint64_t *numbers;
} random_numbers;


static bool contains_number( random_numbers *self, const uint32_t up_to, const uint64_t value )
{
    uint32_t i;
    for ( i = 0; i < up_to; ++i )
    {
        if ( self->numbers[ i ] == value )
        {
            return true;
        }
    }
    return false;
}


static rc_t make_random_numbers( random_numbers *self, const uint32_t count, const uint32_t max )
{
    rc_t rc = 0;
    self->numbers = calloc( count, sizeof *self->numbers );
    if ( self->numbers == NULL )
    {
        rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        uint32_t i;
        for ( i = 0; i < count; ++i )
        {
            do
            {
                self->numbers[ i ] = ( rand() % max ) + 1;
            } while ( contains_number( self, i, self->numbers[ i ] ) );
        }
        self->count = count;
    }
    return rc;
}


static void free_random_numbers( random_numbers *self )
{
    free( self->numbers );
}


typedef void * ptr;
typedef const void * const_ptr;

/*
#define key_and_value( TYPENAME ) \
typedef struct TYPENAME ## _key_and_value \
{ \
    uint64_t key; \
    TYPENAME value; \
} TYPENAME ## _key_and_value; \

*/

typedef struct uint64_t_key_and_value
{
    uint64_t key;
    uint64_t value;
} uint64_t_key_and_value;


typedef struct uint32_t_key_and_value
{
    uint64_t key;
    uint32_t value;
} uint32_t_key_and_value;


typedef struct uint16_t_key_and_value
{
    uint64_t key;
    uint16_t value;
} uint16_t_key_and_value;


typedef struct uint8_t_key_and_value
{
    uint64_t key;
    uint8_t value;
} uint8_t_key_and_value;


typedef struct int64_t_key_and_value
{
    uint64_t key;
    int64_t value;
} int64_t_key_and_value;


typedef struct int32_t_key_and_value
{
    uint64_t key;
    int32_t value;
} int32_t_key_and_value;


typedef struct int16_t_key_and_value
{
    uint64_t key;
    int16_t value;
} int16_t_key_and_value;


typedef struct int8_t_key_and_value
{
    uint64_t key;
    int8_t value;
} int8_t_key_and_value;


typedef struct float_key_and_value
{
    uint64_t key;
    float value;
} float_key_and_value;


typedef struct double_key_and_value
{
    uint64_t key;
    double value;
} double_key_and_value;


typedef struct bool_key_and_value
{
    uint64_t key;
    bool value;
} bool_key_and_value;


typedef struct ptr_key_and_value
{
    uint64_t key;
    ptr value;
} ptr_key_and_value;


/*
#define generic_find( TYPENAME ) \
static TYPENAME ## _key_and_value * find_ ## TYPENAME( Vector * v, uint64_t key ) \
{ \
    TYPENAME ## _key_and_value * res = NULL; \
    uint32_t i, n = VectorLength( v ); \
    for ( i = 0; i < n && res == NULL; ++i ) \
    { \
        TYPENAME ## _key_and_value * tmp = VectorGet ( v, i ); \
        if ( tmp != NULL ) \
        { \
            if ( tmp->key == key ) \
            { \
                res = tmp; \
            } \
        } \
    } \
    return res; \
}
*/

static uint64_t_key_and_value * find_uint64_t( Vector * v, uint64_t key )
{
    uint64_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        uint64_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static uint32_t_key_and_value * find_uint32_t( Vector * v, uint64_t key )
{
    uint32_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        uint32_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static uint16_t_key_and_value * find_uint16_t( Vector * v, uint64_t key )
{
    uint16_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        uint16_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static uint8_t_key_and_value * find_uint8_t( Vector * v, uint64_t key )
{
    uint8_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        uint8_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static int64_t_key_and_value * find_int64_t( Vector * v, uint64_t key )
{
    int64_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        int64_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static int32_t_key_and_value * find_int32_t( Vector * v, uint64_t key )
{
    int32_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        int32_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static int16_t_key_and_value * find_int16_t( Vector * v, uint64_t key )
{
    int16_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        int16_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static int8_t_key_and_value * find_int8_t( Vector * v, uint64_t key )
{
    int8_t_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        int8_t_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static float_key_and_value * find_float( Vector * v, uint64_t key )
{
    float_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        float_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static double_key_and_value * find_double( Vector * v, uint64_t key )
{
    double_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        double_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static bool_key_and_value * find_bool( Vector * v, uint64_t key )
{
    bool_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        bool_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


static ptr_key_and_value * find_ptr( Vector * v, uint64_t key )
{
    ptr_key_and_value * res = NULL;
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i ) \
    {
        ptr_key_and_value * tmp = VectorGet ( v, i );
        if ( tmp != NULL )
        {
            if ( tmp->key == key )
            {
                res = tmp;
            }
        }
    }
    return res;
}


typedef struct lookup_context
{
    KVector *v;
    Vector *control;
    uint32_t found;
} lookup_context;


static void CC key_and_value_whack( void *item, void *data )
{
    free( item );
}


/*
#define lookup( TYPENAME, LOOKUPFUNC ) \
static void CC TYPENAME ## _lookup( void *item, void *data ) \
{ \
    TYPENAME ## _key_and_value *kv = item; \
    lookup_context *ctx = data; \
    TYPENAME value; \
    if ( LOOKUPFUNC ( ctx->v, kv->key, &value ) == 0 ) \
    { \
        if ( kv->value == value ) \
        { \
            ( ctx->found )++; \
        } \
    } \
}
*/


static void CC uint64_t_lookup( void * item, void * data )
{
    uint64_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    uint64_t value;
    if ( KVectorGetU64 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC uint32_t_lookup( void * item, void * data )
{
    uint32_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    uint32_t value;
    if ( KVectorGetU32 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC uint16_t_lookup( void * item, void * data )
{
    uint16_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    uint16_t value;
    if ( KVectorGetU16 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC uint8_t_lookup( void * item, void * data )
{
    uint8_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    uint8_t value;
    if ( KVectorGetU8 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC int64_t_lookup( void * item, void * data )
{
    int64_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    int64_t value;
    if ( KVectorGetI64 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC int32_t_lookup( void * item, void * data )
{
    int32_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    int32_t value;
    if ( KVectorGetI32 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC int16_t_lookup( void * item, void * data )
{
    int16_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    int16_t value;
    if ( KVectorGetI16 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC int8_t_lookup( void * item, void * data )
{
    int8_t_key_and_value *kv = item;
    lookup_context *ctx = data;
    int8_t value;
    if ( KVectorGetI8 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC float_lookup( void * item, void * data )
{
    float_key_and_value *kv = item;
    lookup_context *ctx = data;
    float value;
    if ( KVectorGetF32 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC double_lookup( void * item, void * data )
{
    double_key_and_value *kv = item;
    lookup_context *ctx = data;
    double value;
    if ( KVectorGetF64 ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC bool_lookup( void * item, void * data )
{
    bool_key_and_value *kv = item;
    lookup_context *ctx = data;
    bool value;
    if ( KVectorGetBool ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


static void CC ptr_lookup( void * item, void * data )
{
    ptr_key_and_value *kv = item;
    lookup_context *ctx = data;
    ptr value;
    if ( KVectorGetPtr ( ctx->v, kv->key, &value ) == 0 )
    {
        if ( kv->value == value )
        {
            ( ctx->found )++;
        }
    } 
}


/*
#define generic_on_visit( TYPENAME, VALUE_TYPE ) \
static rc_t CC on_visit_ ## TYPENAME ( uint64_t key, VALUE_TYPE value, void *user_data ) \
{ \
    lookup_context * ctx = user_data; \
    TYPENAME ## _key_and_value * kv = find_ ## TYPENAME ( ctx->control, key ); \
    if ( kv != NULL ) \
    { \
        if ( kv->value == (TYPENAME) value ) \
        { \
            ctx->found ++; \
        } \
        else \
        { \
            OUTMSG(( "values %lx vs %lx mismatch\n", (uint64_t)value, (uint64_t)kv->value )); \
        } \
    } \
    else \
    { \
        OUTMSG(( "key %lu not found\n", key )); \
    } \
    return 0; \
}
*/

static rc_t CC on_visit_uint64_t ( uint64_t key, uint64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    uint64_t_key_and_value * kv = find_uint64_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %lx mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_uint32_t ( uint64_t key, uint64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    uint32_t_key_and_value * kv = find_uint32_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_uint16_t ( uint64_t key, uint64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    uint16_t_key_and_value * kv = find_uint16_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_uint8_t ( uint64_t key, uint64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    uint8_t_key_and_value * kv = find_uint8_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_int64_t ( uint64_t key, int64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    int64_t_key_and_value * kv = find_int64_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %lx mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_int32_t ( uint64_t key, int64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    int32_t_key_and_value * kv = find_int32_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_int16_t ( uint64_t key, int64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    int16_t_key_and_value * kv = find_int16_t( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_int8_t ( uint64_t key, int64_t value, void *user_data )
{
    lookup_context * ctx = user_data;
    int8_t_key_and_value * kv = find_int8_t( ctx->control, key );
    if ( kv != NULL )
    {
        int8_t v = ( int8_t )value;
        if ( kv->value == v )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %lx vs %x mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_double ( uint64_t key, double value, void *user_data )
{
    lookup_context * ctx = user_data;
    double_key_and_value * kv = find_double( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %f vs %f mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_bool ( uint64_t key, bool value, void *user_data )
{
    lookup_context * ctx = user_data;
    bool_key_and_value * kv = find_bool( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %s vs %s mismatch\n", value ? "true" : "false", kv->value ? "true" : "false" ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t CC on_visit_ptr ( uint64_t key, const_ptr value, void *user_data )
{
    lookup_context * ctx = user_data;
    ptr_key_and_value * kv = find_ptr( ctx->control, key );
    if ( kv != NULL )
    {
        if ( kv->value == value )
        {
            ctx->found ++;
        }
        else
        {
            OUTMSG( ( "values %p vs %p mismatch\n", value, kv->value ) );
        }
    }
    else
    {
        OUTMSG( ( "key %lu not found\n", key ) );
    }
    return 0;
}


static rc_t perform_uint64_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: uint64_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            uint64_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (uint64_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetU64 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, uint64_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitU64( v, false, on_visit_uint64_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_uint32_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: uint32_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            uint32_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (uint32_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetU32 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, uint32_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitU64( v, false, on_visit_uint32_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_uint16_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: uint16_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            uint16_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (uint16_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetU16 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, uint16_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitU64( v, false, on_visit_uint16_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_uint8_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: uint8_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            uint8_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (uint8_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetU8 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, uint8_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitU64( v, false, on_visit_uint8_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_int64_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: int64_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            int64_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (int64_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetI64 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, int64_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitI64( v, false, on_visit_int64_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_int32_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: int32_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            int32_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (int32_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetI32 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, int32_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitI64( v, false, on_visit_int32_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_int16_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: int16_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            int16_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (int16_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetI16 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, int16_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitI64( v, false, on_visit_int16_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_int8_t_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: int8_t - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            int8_t_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (int8_t)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetI8 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, int8_t_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitI64( v, false, on_visit_int8_t, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_float_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: float - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            float_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (float)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetF32 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, float_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            /* there is no KVectorVisitF32(), validating with KVectorVisitF64() would fail! */
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_double_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: double - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            double_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = nrand->numbers[ i ];
                kv->value = (double)( rand() % ( nrand->count + 1 ) );
                rc = KVectorSetF64 ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, double_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitF64( v, false, on_visit_double, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_bool_test( random_numbers *nrand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: bool - test( count = %u )\n", nrand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < nrand->count && rc == 0; ++i )
        {
            bool_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                uint32_t rv = rand() % ( nrand->count + 1 );
                kv->key = nrand->numbers[ i ];
                kv->value = ( rv & 1 );
                rc = KVectorSetBool ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", nrand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, bool_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            /* there is no visit-bool */
        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t perform_ptr_test( random_numbers *rand ) /* */
{
    KVector *v;
    rc_t rc;
    OUTMSG(( "KVector: ptr - test( count = %u )\n", rand->count )); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        Vector control;
        uint32_t i;
        VectorInit ( &control, 0, 1000 );
        for ( i = 0; i < rand->count && rc == 0; ++i )
        {
            ptr_key_and_value *kv = malloc( sizeof *kv ); /* */
            if ( kv == NULL )
            {
                rc = RC ( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                kv->key = rand->numbers[ i ];
                kv->value = kv;
                rc = KVectorSetPtr ( v, kv->key, kv->value ); /* */
                if ( rc == 0 )
                    rc = VectorAppend ( &control, NULL, kv );
                if ( rc != 0)
                    free( kv );
            }
        }
        OUTMSG(( "%u key-value-pairs inserted\n", rand->count ));
        if ( rc == 0 )
        {
            lookup_context ctx;
            ctx.found = 0;
            ctx.v = v;
            VectorForEach ( &control, false, ptr_lookup, &ctx ); /* */
            OUTMSG(( "%u values found\n", ctx.found ));

            ctx.found = 0;
            ctx.control = &control;
            rc = KVectorVisitPtr( v, false, on_visit_ptr, &ctx ); /* */
            OUTMSG(( "%u values verified\n", ctx.found ));

        }
        VectorWhack ( &control, key_and_value_whack, NULL );
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));
    return rc;
}


static rc_t test_prev_single_i32 ( KVector const* v,
    uint64_t const key, uint64_t const prev_key_ref, int32_t const prev_value_ref )
{
    uint64_t prev_key;
    int32_t prev_value;
    rc_t rc = KVectorGetPrevI32 ( v, &prev_key, key, &prev_value );
    if (rc)
    {
        OUTMSG(("KVectorGetPrevI32(key=%lu) failed with e...r %R\n", key, rc));
    }
    else
        OUTMSG(("The value previous to key=%lu is (%lu, %d); must be (%lu, %d)%s\n",
            key, prev_key, prev_value, prev_key_ref, prev_value_ref,
            prev_key != prev_key_ref || prev_value != prev_value_ref ? " MISMATCH" : ""));
    return rc;
}

static rc_t test_next_single_i32 ( KVector const* v,
    uint64_t const key, uint64_t const next_key_ref, int32_t const next_value_ref )
{
    uint64_t next_key;
    int32_t next_value;
    rc_t rc = KVectorGetNextI32 ( v, &next_key, key, &next_value );
    if (rc)
        OUTMSG(("KVectorGetNextI32(key=%lu) failed with e...r %R\n", key, rc));
    else
        OUTMSG(("The value following key=%lu is (%lu, %d); must be (%lu, %d)%s\n",
            key, next_key, next_value, next_key_ref, next_value_ref,
            next_key != next_key_ref || next_value != next_value_ref ? " MISMATCH" : ""));
    return rc;
}

static rc_t perform_prev_next_i32_test()
{
    rc_t rc;
    KVector *v;
    OUTMSG(( "KVector: prev element int32 - test\n")); /* */
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        rc = KVectorSetI32 (v, 1, 11 );
        rc = rc || KVectorSetI32 (v, 2, 22 );
        rc = rc || KVectorSetI32 (v, 3, 33 );
        rc = rc || KVectorSetI32 (v, 4, 44 );
        rc = rc || KVectorSetI32 (v, 0, 100 );
        rc = rc || KVectorSetI32 (v, 32, 320 );
        rc = rc || KVectorSetI32 (v, 64, 640 );

        if (rc)
            OUTMSG(("KVectorSetI32 failed with e...r %R\n", rc));
        else
        {
            test_prev_single_i32 (v, 2, 1, 11);
            test_prev_single_i32 (v, 1, 0, 100);
            test_prev_single_i32 (v, 32, 4, 44);
            test_prev_single_i32 (v, 64, 32, 320);
            test_prev_single_i32 (v, 0, 0, 0);
            test_prev_single_i32 (v, 123, 64, 640);
            test_prev_single_i32 (v, 33, 32, 320);

            OUTMSG(("\n"));

            test_next_single_i32 (v, 2, 3, 33);
            test_next_single_i32 (v, 1, 2, 22);
            test_next_single_i32 (v, 32, 64, 640);
            test_next_single_i32 (v, 64, 0, 0);
            test_next_single_i32 (v, 0, 1, 11);
            test_next_single_i32 (v, 123, 0, 0);
            test_next_single_i32 (v, 33, 64, 640);
        }
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));

    return rc;
}

static rc_t test_prev_single_bool ( KVector const* v,
    uint64_t key, uint64_t prev_key_ref, bool prev_value_ref )
{
    uint64_t prev_key;
    bool prev_value;
    rc_t rc = KVectorGetPrevBool ( v, &prev_key, key, &prev_value );
    if (rc)
        OUTMSG(("KVectorGetPrevBool(key=%lu) failed with e...r %R\n", key, rc));
    else
        OUTMSG(("The value previous to key=%lu is (%lu, %d); must be (%lu, %d)%s\n",
            key, prev_key, prev_value, prev_key_ref, prev_value_ref,
            prev_key != prev_key_ref || prev_value != prev_value_ref ? " MISMATCH" : ""));
    return rc;
}

static rc_t test_next_single_bool ( KVector const* v,
    uint64_t key, uint64_t next_key_ref, bool next_value_ref )
{
    uint64_t next_key;
    bool next_value;
    rc_t rc = KVectorGetNextBool ( v, &next_key, key, &next_value );
    if (rc)
        OUTMSG(("KVectorGetNextBool(key=%lu) failed with e...r %R\n", key, rc));
    else
        OUTMSG(("The value following key=%lu is (%lu, %d); must be (%lu, %d)%s\n",
            key, next_key, next_value, next_key_ref, next_value_ref,
            next_key != next_key_ref || next_value != next_value_ref ? " MISMATCH" : ""));
    return rc;
}

static rc_t perform_prev_next_bool_test()
{
    rc_t rc;
    KVector *v;
    OUTMSG(( "KVector: prev element bool - test\n"));
    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        rc = KVectorSetBool (v, 1, true );
        rc = rc || KVectorSetBool (v, 2, false );
        rc = rc || KVectorSetBool (v, 3, true );
        rc = rc || KVectorSetBool (v, 4, false );
        rc = rc || KVectorSetBool (v, 0, true );
        rc = rc || KVectorSetBool (v, 32, false );
        rc = rc || KVectorSetBool (v, 64, true );

        if (rc)
            OUTMSG(("KVectorSetBool failed with e...r %R\n", rc));
        else
        {
            test_prev_single_bool (v, 2, 1, true);
            test_prev_single_bool (v, 1, 0, true);
            test_prev_single_bool (v, 32, 4, false);
            test_prev_single_bool (v, 64, 32, false);
            test_prev_single_bool (v, 0, 4, false);
            test_prev_single_bool (v, 123, 64, true);
            test_prev_single_bool (v, 33, 32, false);

            OUTMSG(("\n"));

            test_next_single_bool (v, 2, 3, true);
            test_next_single_bool (v, 1, 2, false);
            test_next_single_bool (v, 32, 64, true);
            test_next_single_bool (v, 64, 0, true);
            test_next_single_bool (v, 0, 1, true);
            test_next_single_bool (v, 123, 0, true);
            test_next_single_bool (v, 33, 64, true);
        }
        KVectorRelease ( v );
    }
    OUTMSG(( "\n" ));

    return rc;
}


rc_t perform_test( const uint32_t count )
{
    random_numbers rnum;
    rc_t rc = make_random_numbers( &rnum, count, count * 2 );
    if ( rc != 0 )
    {
        OUTMSG(( "error making random numbers %u\n", rc ));
    }
    else
    {
#if _ARCH_BITS == 64
        if ( rc == 0 )
            rc = perform_uint64_t_test( &rnum );
#endif
        if ( rc == 0 )
            rc = perform_uint32_t_test( &rnum );
        if ( rc == 0 )
            rc = perform_uint16_t_test( &rnum );
        if ( rc == 0 )
            rc = perform_uint8_t_test( &rnum );

#if _ARCH_BITS == 64
        if ( rc == 0 )
            rc = perform_int64_t_test( &rnum );
#endif
        if ( rc == 0 )
            rc = perform_int32_t_test( &rnum );
        if ( rc == 0 )
            rc = perform_int16_t_test( &rnum );
        if ( rc == 0 )
            rc = perform_int8_t_test( &rnum );

        if ( rc == 0 )
            rc = perform_float_test( &rnum );
#if _ARCH_BITS == 64
        if ( rc == 0 )
            rc = perform_double_test( &rnum );
#endif
        if ( rc == 0 )
            rc = perform_bool_test( &rnum );
        if ( rc == 0 )
            rc = perform_ptr_test( &rnum );

        free_random_numbers( &rnum );

        perform_prev_next_i32_test();
        perform_prev_next_bool_test();
    }

    return rc;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;

    rc_t rc = ArgsMakeAndHandle ( &args, argc, argv, 1,
                JudyTestOptions, sizeof ( JudyTestOptions ) / sizeof ( OptDef ) );
    if ( rc == 0 )
    {
        srand ( ( unsigned int ) time(NULL) );
        rc = perform_test( get_int_option( args, OPTION_COUNT, 10000 ) );

        ArgsWhack ( args );
    }
    else
        OUTMSG( ( "ArgsMakeAndHandle() failed %R\n", rc ) );

    return rc;
}
