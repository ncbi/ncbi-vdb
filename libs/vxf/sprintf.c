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
#include <vdb/extern.h>

#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>


/* the number of PrintFmt, PrintArg and String
   elements to allocate in function-local storage */
#define LOCAL_FMT_COUNT 64


typedef struct Sprintf Sprintf;
struct Sprintf
{
    const PrintFmt *fmt;
    PrintArg *args;
    String *str;
};

static uint8_t const radix2_size [ 4 ] = { 8, 16, 32, 64 };
static uint8_t const radix8_size [ 4 ] = { 3, 6, 11, 22 };
static uint8_t const radix10_size [ 4 ] = { 3, 5, 10, 20 };
static uint8_t const radix16_size [ 4 ] = { 2, 4, 8, 16 };

#if _DEBUGGING
static
rc_t validate_obj ( const Sprintf *obj, bool fmt_only )
{
    uint32_t i;

    /* validate the format blocks */
    for ( i = 0; obj -> fmt [ i ] . type != sptTerm; ++ i )
    {
        /* at this point, only looking for bad formats */
        if ( obj -> fmt [ i ] . type == sptLiteral )
        {
            if ( obj -> fmt [ i ] . u . l . text == NULL )
                return RC ( rcXF, rcFunction, rcConstructing, rcString, rcNull );
            if ( obj -> fmt [ i ] . u . l . size == 0 )
                return RC ( rcXF, rcFunction, rcConstructing, rcString, rcEmpty );
            if ( string_chr ( obj -> fmt [ i ] . u . l . text, obj -> fmt [ i ] . u . l . size, 0 ) != NULL )
                return RC ( rcXF, rcFunction, rcConstructing, rcString, rcInvalid );
        }
    }

    return 0;
}
#endif

static
size_t get_radix_size ( const PrintFmt *fmt, uint32_t selector )
{
    const uint8_t *radix_size;
    switch ( fmt -> radix )
    {
    case 0:
        return 0;
    case 10:
        radix_size = radix10_size;
        break;
    case 16:
        radix_size = radix16_size;
        break;
    case 8:
        radix_size = radix8_size;
        break;
    default:
        radix_size = radix2_size;
    }
    return radix_size [ selector ];
}

typedef struct FuncArgs FuncArgs;
struct FuncArgs
{
    const VRowData *argv;
    uint32_t argc;
    uint32_t idx;
};

static
const void *access_cell_data ( FuncArgs *args, uint64_t *row_len, size_t *size )
{
    size_t elem_bytes;
    const uint8_t *data;

    if ( args -> idx == args -> argc )
        return NULL;

    elem_bytes = args -> argv [ args -> idx ] . u . data . elem_bits / 8;
    assert ( elem_bytes * 8 == args -> argv [ args -> idx ] . u . data . elem_bits );

    data = args -> argv [ args -> idx ] . u . data . base;
    * row_len = args -> argv [ args -> idx ] . u . data . elem_count;

    data += args -> argv [ args -> idx ] . u . data . first_elem * elem_bytes;

    if ( size != NULL )
        * size = ( size_t ) ( * row_len * elem_bytes );

    ++ args -> idx;
    return data;
}

static
rc_t capture_uint_scalar ( FuncArgs *args, uint64_t *val )
{
    uint64_t row_len;
    const void *data = access_cell_data ( args, & row_len, NULL );

    if ( data == NULL )
        return RC ( rcXF, rcFunction, rcExecuting, rcData, rcNull );

    if ( row_len != 1 )
        return RC ( rcXF, rcFunction, rcExecuting, rcRange, rcExcessive );

    switch ( args -> argv [ args -> idx - 1 ] . u . data . elem_bits )
    {
    case 8:
        * val = * ( const uint8_t* ) data;
        break;
    case 16:
        * val = * ( const uint16_t* ) data;
        break;
    case 32:
        * val = * ( const uint32_t* ) data;
        break;
    case 64:
        * val = * ( const uint64_t* ) data;
        break;
    }

    return 0;
}

static
rc_t CC sprintf_func ( void *obj,
     const VXformInfo *info, int64_t row_id, VRowResult *rslt,
     uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    size_t cell_size;
    uint32_t str_idx, fmt_idx, arg_idx;

    Sprintf *self = obj;

    const PrintFmt *fmt = self -> fmt;
    PrintArg *args = self -> args;
    String *str = self -> str;

    FuncArgs vargs;
    vargs . argv = argv;
    vargs . argc = argc;
    vargs . idx = 0;

    str_idx = fmt_idx = arg_idx = 0;

#if _DEBUGGING
    rc = validate_obj ( self, false );
    if ( rc != 0 )
        KDbgMsg ( "%s - self is bad on entry: %R\n", __func__, rc );
#endif

    /* initial pass - prepare argument list & estimate cell size */
    for ( rc = 0, cell_size = 0; fmt [ fmt_idx ] . type != sptTerm; ++ fmt_idx )
    {
        size_t arg_size;
        bool check_index;
        uint64_t row_len, start_idx, select_len;
        uint32_t ext_start_index, ext_stop_index;

        row_len = 0;

        /* test for external field width, precision, index */
        if ( fmt [ fmt_idx ] . ext_field_width )
        {
            rc = capture_uint_scalar ( & vargs, & args [ arg_idx ++ ] . u );
            if ( rc != 0 )
                return rc;
        }
        if ( fmt [ fmt_idx ] . ext_precision )
        {
            rc = capture_uint_scalar ( & vargs, & args [ arg_idx ++ ] . u );
            if ( rc != 0 )
                return rc;
        }

        if ( ! fmt [ fmt_idx ] . ext_start_index )
            start_idx = fmt [ fmt_idx ] . u . f . start_idx;
        else
        {
            /* if the start index is the last in vector,
               this needs to be suspended until later */
            if ( ! fmt [ fmt_idx ] . inf_start_index )
            {
                rc = capture_uint_scalar ( & vargs, & start_idx );
                if ( rc != 0 )
                    return rc;
                args [ arg_idx ] . u = start_idx;
            }

            ext_start_index = arg_idx ++;
        }

        if ( ! fmt [ fmt_idx ] . ext_stop_index && ! fmt [ fmt_idx ] . ext_select_len )
            select_len = fmt [ fmt_idx ] . u . f . select_len;
        else
        {
            /* if the stop index is the last in vector,
               this needs to be suspended until later */
            if ( ! fmt [ fmt_idx ] . inf_stop_index )
            {
                rc = capture_uint_scalar ( & vargs, & select_len );
                if ( rc != 0 )
                    return rc;
                args [ arg_idx ] . u = select_len;
            }
            ext_stop_index = arg_idx ++;
        }

        /* make an estimate of the bytes needed by sprintf engine for argument
           get row_len at the same time */
        check_index = false;
        switch ( fmt [ fmt_idx ] . type )
        {
        case sptLiteral:                     /* char literal        - arg in fmt   */
            assert ( fmt [ fmt_idx ] . u . l . text != NULL );
            arg_size = fmt [ fmt_idx ] . u . l . size;
            row_len = arg_size;
            -- arg_idx;
            break;
        case sptSignedInt8Vect:              /* signed vector int   - arg is d8    */
            args [ arg_idx ] . d8 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 0 ) + 1;
            break;
        case sptSignedInt16Vect:             /* signed vector int   - arg is d16   */
            args [ arg_idx ] . d16 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 1 ) + 1;
            break;
        case sptSignedInt32Vect:             /* signed vector int   - arg is d32   */
            args [ arg_idx ] . d32 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 2 ) + 1;
            break;
        case sptSignedInt64Vect:             /* signed vector int   - arg is d64   */
            args [ arg_idx ] . d64 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 3 ) + 1;
            break;
        case sptUnsignedInt8Vect:            /* unsigned vector int - arg is u8    */
            args [ arg_idx ] . u8 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 0 ) + 2;
            break;
        case sptUnsignedInt16Vect:           /* unsigned vector int - arg is u16   */
            args [ arg_idx ] . u16 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 1 ) + 2;
            break;
        case sptUnsignedInt32Vect:           /* unsigned vector int - arg is u32   */
            args [ arg_idx ] . u32 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 2 ) + 2;
            break;
        case sptUnsignedInt64Vect:           /* unsigned vector int - arg is u64   */
            args [ arg_idx ] . u64 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 3 ) + 2;
            break;
        case sptFloat32Vect:                 /* vector float        - arg is f32   */
            args [ arg_idx ] . f32 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = 32;
            break;
        case sptFloat64Vect:                 /* vector float        - arg is f64   */
            args [ arg_idx ] . f64 = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = 32;
            break;
        case sptFloatLongVect:               /* vector float        - arg is flong */
            args [ arg_idx ] . flong = access_cell_data ( & vargs, & row_len, NULL );
            arg_size = 32;
            break;
        case sptString:                      /* vector character    - arg is S     */
            str [ str_idx ] . addr = access_cell_data ( & vargs, & row_len, & str [ str_idx ] . size );
            str [ str_idx ] . len = ( uint32_t ) str [ str_idx ] . size;
            arg_size = str [ str_idx ] . size;
            args [ arg_idx ] . S = & str [ str_idx ++ ];
            check_index = true;
            break;
        case sptUCS2String:                  /* vector character    - arg is S     */
        case sptUTF32String:                 /* vector character    - arg is S     */
            return RC ( rcXF, rcFunction, rcExecuting, rcType, rcUnsupported );
        case sptRowId:                       /* current row id      - arg is d     */
            args [ arg_idx ] . d = row_id;
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 3 ) + 1;
            ++ arg_idx;
            continue;
        case sptRowLen:                      /* current row length  - arg is u     */
            access_cell_data ( & vargs, & args [ arg_idx ] . u, NULL );
            arg_size = get_radix_size ( & fmt [ fmt_idx ], 3 ) + 2;
            ++ arg_idx;
            continue;
        }

        /* back-fill based upon row-length in case of non-empty rows */
        if ( row_len != 0 )
        {
            /* start index is last in row */
            if ( fmt [ fmt_idx ] . inf_start_index )
                start_idx = args [ ext_start_index ] . u = row_len - 1;

            /* end index is last in row */
            if ( fmt [ fmt_idx ] . inf_stop_index )
                select_len = args [ ext_stop_index ] . u = row_len - 1;
        }

        /* detect runtime error condition */
        if ( start_idx >= row_len )
            args [ arg_idx ] . p = NULL;
        else
        {
            /* adjust size by index */
            if ( check_index )
            {
                /* the select_len should be correct,
                   unless the end is a stop coordinate */
                if ( fmt [ fmt_idx ] . ext_stop_index )
                {
                    if ( select_len <= start_idx )
                        select_len = 1;
                    else
                        select_len -= start_idx - 1;
                }

                /* select_len of 0 means infinite */
                if ( select_len == 0 )
                    select_len = row_len - start_idx;

                /* real size after sub-str */
                arg_size = argv [ vargs . idx - 1 ] . u . data . elem_bits * select_len / 8;
            }

            /* incorporate size into calcuation */
            cell_size += arg_size;
        }

        /* done with argument */
        ++ arg_idx;
    }

    /* round cell-size up to nearest 4K */
    cell_size = ( cell_size + 4095 ) & ~ ( size_t ) 4095;

    /* set output buffer size */
    rc = KDataBufferResize( rslt -> data, cell_size );
    if ( rc == 0 )
    {
#if _DEBUGGING
        rc = validate_obj ( self, false );
        if ( rc != 0 )
            KDbgMsg ( "%s - self is bad before invoking structured_sprintf: %R\n", __func__, rc );
#endif
        /* invoke sprintf engine */
        rc = structured_sprintf ( rslt -> data -> base,
            rslt -> data -> elem_count, & cell_size, fmt, args );

        /* recover from buffer insufficient */
        if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == (enum RCObject)rcBuffer )
        {
            rc = KDataBufferResize ( rslt -> data, cell_size + 1 );
            if ( rc == 0 )
            {
                rc = structured_sprintf ( rslt -> data -> base,
                    rslt -> data -> elem_count, & cell_size, fmt, args );
            }
        }

        if ( rc == 0 )
        {
            /* set the return parameters */
            rc = KDataBufferResize ( rslt -> data, cell_size );
            if ( rc == 0 )
            {
                rslt -> elem_count = cell_size;
                rslt -> elem_bits = 8;
            }
        }
    }

    return rc;
}

typedef struct ParseData ParseData;
struct ParseData
{
    KDataBuffer overflow;
    size_t fmt_size;
    size_t lit_size;
    PrintFmt *fmt;
    uint32_t str_idx;
    uint32_t fmt_idx;
    uint32_t arg_idx;
};

/* create_overflow
 *  create a buffer so large that it cannot overflow again
 *  copy in existing structures
 */
static
rc_t create_overflow ( ParseData *pd, uint32_t fmt_idx )
{
    /* infer that we can never have more format items than characters in the string... */

    /* our size will create a format for every byte.
       this should be a total overkill. */
    size_t buff_size = sizeof pd -> fmt [ 0 ] * pd -> fmt_size;

    /* make the buffer */
    rc_t rc = KDataBufferMakeBytes ( & pd -> overflow, buff_size );
    if ( rc == 0 )
    {
        /* capture pointers to stack structures */
        const PrintFmt *fmt = pd -> fmt;

        /* destination pointer */
        pd -> fmt = pd -> overflow . base;

        /* copy existing data */
        memmove ( pd -> fmt, fmt, fmt_idx * sizeof pd -> fmt [ 0 ] );
   }

    pd -> fmt_idx = fmt_idx;

    return rc;
}

typedef struct VAList VAList;
struct VAList
{
    const VFunctionParams *dp;
    uint32_t idx;
};

static
rc_t check_integer_arg ( VAList *vargs )
{
    if ( vargs -> idx == vargs -> dp -> argc )
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcInsufficient );

    switch ( vargs -> dp -> argv [ vargs -> idx ] . desc . domain )
    {
    case vtdBool:
    case vtdInt:
    case vtdUint:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }

    switch ( VTypedescSizeof ( & vargs -> dp -> argv [ vargs -> idx ] . desc ) )
    {
    case 8:
    case 16:
    case 32:
    case 64:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }

    ++ vargs -> idx;
    return 0;
}

static
rc_t extract_size_modifier ( VAList *vargs, char *size_modifier )
{
    switch ( vargs -> dp -> argv [ vargs -> idx ] . desc . domain )
    {
    case vtdBool:
        * size_modifier = 0;
        break;
    case vtdUint:
    case vtdInt:
        switch ( VTypedescSizeof ( & vargs -> dp -> argv [ vargs -> idx ] . desc ) )
        {
        case 8:
            * size_modifier = 't';
            break;
        case 16:
            * size_modifier = 'h';
            break;
        case 32:
            * size_modifier = 0;
            break;
        case 64:
            * size_modifier = 'l';
            break;
        default:
            return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
        }
        break;
    case vtdFloat:
        switch ( VTypedescSizeof ( & vargs -> dp -> argv [ vargs -> idx ] . desc ) )
        {
        case 32:
            * size_modifier = 'h';
            break;
        case 64:
            * size_modifier = 0;
            break;
        default:
            return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
        }
        break;
    case vtdAscii:
        * size_modifier = 0;
        break;
    case vtdUnicode:
        switch ( vargs -> dp -> argv [ vargs -> idx ] . desc . intrinsic_bits )
        {
        case 8:
            * size_modifier = 0;
            break;
        case 16:
            * size_modifier = 'h';
            break;
        case 32:
            * size_modifier = 'l';
            break;
        default:
            return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
        }
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcParam, rcIncorrect );
    }
    return 0;
}


/* parse_format_string
 *  parse format string and args into structured format
 */
static
rc_t parse_format_string ( const char *fmt_str, ParseData *pd, VAList *vargs )
{
    rc_t rc;
    uint32_t i, fmt_idx;

    PrintFmt *fmt = pd -> fmt;

    /* initialize returned counters */
    pd -> lit_size = 0;
    pd -> str_idx = pd -> fmt_idx = pd -> arg_idx = 0;

    /* loop over format string */
    for ( rc = 0, i = fmt_idx = 0; fmt_str [ i ] != 0; ++ i )
    {
        uint32_t domain;
        bool alternate, numeric;
        char size_modifier, time_modifier;
        bool has_precision/*, has_width, has_index*/;

        /* loop to gather literal portions */
        uint32_t start;
        for ( start = i; ; ++ i )
        {
            /* run until we hit start of substitution token
               or until we hit end of format string */
            if ( fmt_str [ i ] != 0 && fmt_str [ i ] != '%' )
                continue;

            /* detect a literal string */
            if ( i != start )
            {
                /* expand into overflow */
                if ( fmt_idx == LOCAL_FMT_COUNT )
                {
                    rc = create_overflow ( pd, fmt_idx );
                    if ( rc != 0 )
                        return rc;

                    fmt = pd -> fmt;
                }

                /* create a text-literal format */
                memset ( & fmt [ fmt_idx ], 0, sizeof fmt [ 0 ] );
                fmt [ fmt_idx ] . u . l . text = & fmt_str [ start ];
                fmt [ fmt_idx ] . u . l . size = i - start;
                pd -> lit_size += i - start;
                fmt [ fmt_idx ] . fmt = spfText;
                fmt [ fmt_idx ] . type = sptLiteral;

                /* increment counter */
                ++ fmt_idx;
            }

            /* detect escape sequence */
            if ( fmt_str [ i ] == 0 || fmt_str [ i + 1 ] != '%' )
                break;

            /* skip over escape */
            start = ++ i;
        }

        /* done when NUL byte is seen */
        if ( fmt_str [ i ] == 0 )
            break;

        /* detect overflow */
        if ( fmt_idx == LOCAL_FMT_COUNT )
        {
            rc = create_overflow ( pd, fmt_idx );
            if ( rc != 0 )
                return rc;

            fmt = pd -> fmt;
        }

        /* initial format
         *  thousands_separate    = false
         *  add_prefix            = false
         *  force_decimal_point   = false
         *  leave_trailing_zeros  = false
         *  print_time            = false
         *  print_date            = false
         *  print_weekday         = false
         *  print_timezone        = false
         *  hour_24               = false
         *  sign                  = 0
         *  left_fill             = space
         */
        memset ( & fmt [ fmt_idx ], 0, sizeof fmt [ 0 ] );
        fmt [ fmt_idx ] . left_fill = ' ';

        /* scan flags */
        alternate = false;
        while ( 1 )
        {
            switch ( fmt_str [ ++ i ] )
            {
                /* plus and space modify application of sign
                   to signed integer and floating point conversions.
                   plus overrides space. */
            case ' ':
                if ( fmt [ fmt_idx ] . sign == 0 )
            case '+':
                    fmt [ fmt_idx ] . sign = fmt_str [ i ];
                continue;

                /* dash indicates left-alignment. indicate this
                   by setting "left_fill" to NUL. */
            case '-':
                fmt [ fmt_idx ] . left_fill = 0;
                continue;

                /* zero indicates an alternate left-fill for
                   numeric conversions. the zero is inserted before
                   any sign character in { '+', '-' or ' ' }.
                   since "left_fill" is also used to indicate
                   alignment, only store when right aligning. */
            case '0':
                if ( fmt [ fmt_idx ] . left_fill != 0 )
                    fmt [ fmt_idx ] . left_fill = '0';
                continue;

                /* hash indicates that the formatter should use an
                   "alternate" approach. that approach is specific
                   to the format. */
            case '#':
                alternate = true;
                continue;

                /* comma ( or apostrophe outside of US ) indicates
                   that the integer portion of a numeral should use
                   a comma as a thousands separator for legibility. */
            case ',':
            case '\'':
                fmt [ fmt_idx ] . thousands_separate = 1;
                continue;
            }

            /* we've hit a non-flag character */
            break;
        }

        /* minimum field width */
        /* has_width = false; */
        if ( isdigit ( fmt_str [ i ] ) )
        {
            /* literal */
            /*has_width = true;*/
            fmt [ fmt_idx ] . u . f . min_field_width = fmt_str [ i ] - '0';
            while ( isdigit ( fmt_str [ ++ i ] ) )
            {
                fmt [ fmt_idx ] . u . f . min_field_width *= 10;
                fmt [ fmt_idx ] . u . f . min_field_width += fmt_str [ i ] - '0';
            }
        }
        else if ( fmt_str [ i ] == '*' )
        {
            /* external */
            rc = check_integer_arg ( vargs );
            if ( rc != 0 )
                return rc;

            /*has_width = true;*/
            fmt [ fmt_idx ] . ext_field_width = 1;
            ++ pd -> arg_idx;
            ++ i;
        }

        /* precision */
        has_precision = false;
        if ( fmt_str [ i ] == '.' )
        {
            /* a single dot implies a precision value of 0 */
            has_precision = true;

            if ( isdigit ( fmt_str [ ++ i ] ) )
            {
                /* a literal precision */
                fmt [ fmt_idx ] . u . f . precision = fmt_str [ i ] - '0';
                while ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    fmt [ fmt_idx ] . u . f . precision *= 10;
                    fmt [ fmt_idx ] . u . f . precision += fmt_str [ i ] - '0';
                }
            }
            else if ( fmt_str [ i ] == '*' )
            {
                /* external */
                rc = check_integer_arg ( vargs );
                if ( rc != 0 )
                    return rc;

                fmt [ fmt_idx ] . ext_precision = 1;
                ++ pd -> arg_idx;
                ++ i;
            }
            else if ( fmt_str [ i ] == '-' )
            {
                /* eat a negative precision - treat as 0 */
                while ( isdigit ( fmt_str [ ++ i ] ) )
                    ( void ) 0;
            }
        }

        /* index - given when parameter is a vector */
        /*has_index = false;*/
        if ( fmt_str [ i ] == ':' )
        {
            bool has_start, has_len, has_end, end_is_stop;
            has_start = has_len = has_end = end_is_stop = false;

            /* parameter is taken as a vector,
               with a default index starting at 0 */
            /*has_index = true;*/

            if ( isdigit ( fmt_str [ ++ i ] ) )
            {
                /* literal index */
                fmt [ fmt_idx ] . u . f . start_idx = fmt_str [ i ] - '0';
                while ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    fmt [ fmt_idx ] . u . f . start_idx *= 10;
                    fmt [ fmt_idx ] . u . f . start_idx += fmt_str [ i ] - '0';
                }
                has_start = true;
            }
            else switch ( fmt_str [ i ] )
            {
            case '*':
                /* external */
                rc = check_integer_arg ( vargs );
                if ( rc != 0 )
                    return rc;

                fmt [ fmt_idx ] . ext_start_index = 1;
                ++ pd -> arg_idx;
                ++ i;
                has_start = true;
                break;
            case '$':
                fmt [ fmt_idx ] . inf_start_index = 1;
                fmt [ fmt_idx ] . ext_start_index = 1;
                ++ pd -> arg_idx;
                ++ i;
                has_start = true;
                break;
            }

            /* detect range */
            switch ( fmt_str [ i ] )
            {
                /* given as start-stop */
            case '-':
                end_is_stop = true;

                /* given as start/len */
            case '/':

                has_len = true;

                if ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    /* literal selection length or end */
                    fmt [ fmt_idx ] . u . f . select_len = fmt_str [ i ] - '0';
                    while ( isdigit ( fmt_str [ ++ i ] ) )
                    {
                        fmt [ fmt_idx ] . u . f . select_len *= 10;
                        fmt [ fmt_idx ] . u . f . select_len += fmt_str [ i ] - '0';
                    }
                    has_end = true;
                }
                else switch ( fmt_str [ i ] )
                {
                case '*':
                    /* external */
                    rc = check_integer_arg ( vargs );
                    if ( rc != 0 )
                        return rc;

                    /* external selection length or end */
                    fmt [ fmt_idx ] . ext_stop_index = end_is_stop;
                    fmt [ fmt_idx ] . ext_select_len = ! end_is_stop;
                    ++ pd -> arg_idx;
                    ++ i;
                    has_end = true;
                    break;
                case '$':
                    /* ignore index end if start is infinite */
                    if ( ! fmt [ fmt_idx ] . inf_start_index )
                    {
                        fmt [ fmt_idx ] . inf_stop_index = 1;
                        fmt [ fmt_idx ] . ext_stop_index = 1;
                        ++ pd -> arg_idx;
                        end_is_stop = has_end = true;
                    }
                    ++ i;
                    break;
                case '-':
                    /* negatives are garbage */
                    while ( isdigit ( fmt_str [ ++ i ] ) )
                        ( void ) 0;
                    break;
                default:
                    end_is_stop = false;
                }
                break;
            }

            if ( ! has_len && has_start )
                fmt [ fmt_idx ] . u . f . select_len = 1;
        }

        /* size - accept for brownie-points and for KTime */
        size_modifier = time_modifier = 0;
        switch ( fmt_str [ i ] )
        {
            /* "Tiny" modifier - like "hh" in C format */
        case 't':
            /* "Half" modifier - same as C format */
        case 'h':
            /* "Long" modifier - means 64-bit for integers, otherwise like C */
        case 'l':
            size_modifier = time_modifier = fmt_str [ i ++ ];
            break;
            /* "siZe" modifier - whatever the size of size_t is */
        case 'z':
            ++ i;
            time_modifier = 'z';
            if ( sizeof ( size_t ) == sizeof ( uint64_t ) )
                size_modifier = 'l';
            break;
        }

        /* output format
           describes the formatting to apply on output
           if precision has not been set, give it a default value */
        domain = 0;
        numeric = false;
        switch ( fmt_str [ i ] )
        {
            /* decimal signed integer */
        case 'd':
        case 'i':
            fmt [ fmt_idx ] . radix = 10;
            fmt [ fmt_idx ] . fmt = spfSignedInt;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 1;
            else if ( fmt [ fmt_idx ] . left_fill == '0' )
                fmt [ fmt_idx ] . left_fill = ' ';
            domain = vtdInt;
            break;

            /* decimal unsigned integer */
        case 'u':
            fmt [ fmt_idx ] . radix = 10;
        unsigned_int:
            fmt [ fmt_idx ] . fmt = spfUnsigned;
            fmt [ fmt_idx ] . sign = 0;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 1;
            else if ( fmt [ fmt_idx ] . left_fill == '0' )
                fmt [ fmt_idx ] . left_fill = ' ';
            domain = vtdUint;
            break;

            /* hex unsigned integer */
        case 'x':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 16;
            goto unsigned_int;

            /* upper-case hex unsigned integer */
        case 'X':
            fmt [ fmt_idx ] . upper_case_num = 1;
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 16;
            goto unsigned_int;

            /* octal unsigned integer */
        case 'o':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 8;
            goto unsigned_int;

            /* binary unsigned integer */
        case 'b':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 2;
            goto unsigned_int;

            /* decimal signed floating point */
        case 'f':
            fmt [ fmt_idx ] . fmt = spfStdFloat;
        fmt_float:
            fmt [ fmt_idx ] . radix = 10;
            fmt [ fmt_idx ] . force_decimal_point = alternate;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 6;
            domain = vtdFloat;
            break;

            /* scientific notation floating point */
        case 'e':
            fmt [ fmt_idx ] . fmt = spfSciFloat;
            goto fmt_float;

            /* "general" floating point */
        case 'g':
            fmt [ fmt_idx ] . leave_trailing_zeros = alternate;
            fmt [ fmt_idx ] . fmt = spfGenFloat;
            goto fmt_float;

            /* character data */
        case 's':
            if ( ! has_precision )
            /* no break */
        case 'c':
                fmt [ fmt_idx ] . u . f . precision = -1;
            fmt [ fmt_idx ] . fmt = spfText;
            domain = vtdUnicode;
            break;

        default:
            return RC ( rcXF, rcString, rcFormatting, rcFormat, rcUnrecognized );
        }

        /* handle zero padding for non-numeric cases */
        if ( ! numeric && fmt [ fmt_idx ] . left_fill == '0' )
            fmt [ fmt_idx ] . left_fill = ' ';

        /* take size from actual parameter */
        rc = extract_size_modifier ( vargs, & size_modifier );
        if ( rc != 0 )
            return rc;

        /* determine type from argument */
        switch ( vargs -> dp -> argv [ vargs -> idx ] . desc . domain )
        {
        case vtdBool:
        case vtdUint:
            switch ( domain )
            {
            case vtdBool:
            case vtdUint:
            case vtdInt:
                break;
            case vtdFloat:
                fmt [ fmt_idx ] . type_cast = 1;
                break;
            default:
                return RC ( rcXF, rcString, rcFormatting, rcParam, rcIncorrect );
            }

            switch ( size_modifier )
            {
            case 't':
                fmt [ fmt_idx ] . type = sptUnsignedInt8Vect;
                break;
            case 'h':
                fmt [ fmt_idx ] . type = sptUnsignedInt16Vect;
                break;
            case 0:
                fmt [ fmt_idx ] . type = sptUnsignedInt32Vect;
                break;
            case 'l':
                fmt [ fmt_idx ] . type = sptUnsignedInt64Vect;
                break;
            }
            break;

        case vtdInt:
            switch ( domain )
            {
            case vtdBool:
            case vtdUint:
            case vtdInt:
                break;
            case vtdFloat:
                fmt [ fmt_idx ] . type_cast = 1;
                break;
            default:
                return RC ( rcXF, rcString, rcFormatting, rcParam, rcIncorrect );
            }

            switch ( size_modifier )
            {
            case 't':
                fmt [ fmt_idx ] . type = sptSignedInt8Vect;
                break;
            case 'h':
                fmt [ fmt_idx ] . type = sptSignedInt16Vect;
                break;
            case 0:
                fmt [ fmt_idx ] . type = sptSignedInt32Vect;
                break;
            case 'l':
                fmt [ fmt_idx ] . type = sptSignedInt64Vect;
                break;
            }
            break;

        case vtdFloat:
            switch ( domain )
            {
            case vtdBool:
            case vtdUint:
            case vtdInt:
                fmt [ fmt_idx ] . type_cast = 1;
                break;
            case vtdFloat:
                break;
            default:
                return RC ( rcXF, rcString, rcFormatting, rcParam, rcIncorrect );
            }

            switch ( size_modifier )
            {
            case 'h':
                fmt [ fmt_idx ] . type = sptFloat32Vect;
                break;
            case 0:
                fmt [ fmt_idx ] . type = sptFloat64Vect;
                break;
            }
            break;

        case vtdAscii:
            switch ( domain )
            {
            case vtdAscii:
            case vtdUnicode:
                if ( size_modifier != 0 )
                    fmt [ fmt_idx ] . type_cast = 1;
                break;
            default:
                return RC ( rcXF, rcString, rcFormatting, rcParam, rcIncorrect );
            }

            fmt [ fmt_idx ] . type = sptString;

            ++ pd -> str_idx;
            break;

        case vtdUnicode:
            switch ( domain )
            {
            case vtdAscii:
            case vtdUnicode:
                switch ( size_modifier )
                {
                case 0:
                    fmt [ fmt_idx ] . type = sptString;
                    break;
                case 'h':
                    fmt [ fmt_idx ] . type_cast = 1;
                    fmt [ fmt_idx ] . type = sptUCS2String;
                    break;
                case 'l':
                    fmt [ fmt_idx ] . type_cast = 1;
                    fmt [ fmt_idx ] . type = sptUTF32String;
                    break;
                }
                break;
            default:
                return RC ( rcXF, rcString, rcFormatting, rcParam, rcIncorrect );
            }

            ++ pd -> str_idx;
            break;
        }

        /* account for format argument */
        ++ fmt_idx;
        ++ pd -> arg_idx;
        ++ vargs -> idx;
    }

    /* record final fmt */
    if ( rc == 0 )
    {
        if ( fmt_idx == LOCAL_FMT_COUNT )
        {
            rc = create_overflow ( pd, fmt_idx );
            if ( rc != 0 )
                return rc;

            fmt = pd -> fmt;
        }

        memset ( & fmt [ fmt_idx ++ ], 0, sizeof fmt [ 0 ] );

        /* if not all arguments were consumed, should this be an error? */
        if ( vargs -> idx != vargs -> dp -> argc )
        {
            /* produce warning */
        }
    }

    pd -> fmt_idx = fmt_idx;

    return rc;
}

VTRANSFACT_IMPL ( vdb_sprintf, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    Sprintf *obj;

    /* const format string */
    VAList vargs;
    const char *fmt_str = cp -> argv [ 0 ] . data . ascii;

    /* local formatting storage */
    PrintFmt fmt [ LOCAL_FMT_COUNT ];

    /* data block for parse */
    ParseData pd;
    pd . fmt_size = cp -> argv [ 0 ] . count;
    pd . fmt = fmt;

    /* packaged va_list */
    vargs . dp = dp;
    vargs . idx = 0;

    /* parse the format string */
    rc = parse_format_string ( fmt_str, & pd, & vargs );
    if ( rc == 0 )
    {
        /* the object size:
           literal data bytes +
           space for PrintFmt +
           space for PrintArg */
        size_t obj_extra = pd . lit_size +
            pd . fmt_idx * sizeof ( PrintFmt ) +
            pd . arg_idx * sizeof ( PrintArg ) +
            pd . str_idx * sizeof ( String );
        obj = malloc ( sizeof * obj + 1 + obj_extra );
        if ( obj == NULL )
            rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
        else
        {
            uint32_t i;

            char *lit;
            PrintFmt *dfmt;
            size_t lit_size;

            obj -> args = ( void* ) ( obj + 1 );
            dfmt = ( void* ) & obj -> args [ pd . arg_idx ];
            obj -> str = ( void* ) & dfmt [ pd . fmt_idx ];
            lit = ( void* ) & obj -> str [ pd . str_idx ];
            obj -> fmt = dfmt;

            /* not necessary or even helpful, but doesn't cost much */
            memset ( obj -> args, 0, pd . arg_idx * sizeof obj -> args [ 0 ] );
            memset ( obj -> str, 0, pd . str_idx * sizeof obj -> str [ 0 ] );

            /* copy format and literals */
            for ( lit_size = 0, i = 0; i < pd . fmt_idx; ++ i )
            {
                /* simple copy */
                dfmt [ i ] = pd . fmt [ i ];
                if ( pd . fmt [ i ] . type == sptLiteral )
                {
                    /* copy over literal data */
                    dfmt [ i ] . u . l . text = & lit [ lit_size ];
                    memmove ( & lit [ lit_size ],
                        pd . fmt [ i ] . u . l . text,
                        pd . fmt [ i ] . u . l . size );
                    lit_size += pd . fmt [ i ] . u . l . size;
                }
            }

            /* NUL-terminate the literal text - again, doesn't help but doesn't hurt */
            lit [ lit_size ] = 0;
        }
    }

    /* douse any overflow memory used */
    if ( pd . fmt != fmt )
        KDataBufferWhack ( & pd . overflow );

#if _DEBUGGING
    if ( rc == 0 )
    {
        rc = validate_obj ( obj, true );
        if ( rc != 0 )
            KDbgMsg ( "%s - self is bad on factory exit: %R\n", __func__, rc );
    }
#endif

    if ( rc == 0 )
    {
        rslt -> self = obj;
        rslt -> whack = free;
        rslt -> u . rf = sprintf_func;
        rslt -> variant = vftRow;
    }

    return rc;
}
