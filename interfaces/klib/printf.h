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

#ifndef _h_klib_printf_
#define _h_klib_printf_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif



/*--------------------------------------------------------------------------
 * forwards
 */
struct String;
struct KSymbol;
struct KDataBuffer;
struct KWrtHandler;


/*--------------------------------------------------------------------------
 * FORMAT
 *  a description of the string_printf formatting
 */

#define SUPPORT_PERCENT_N 1


/*
  The standard C library formatting approach was taken as a basis.

  This interface differs in some ways, in that it presents a sub-set of
  the std. C format convention, and then extends it for klib.

  The general substitution parameter format is:

    '%' [ <flags> ] [ <field-width> ] [ '.' <precision> ] [ ':' <index> ] \
        [ <storage-class-width> ] <storage-class>

  where:

    flags
        = ' '           : prepend space to a numeral if it does not have a sign
        | '+'           : always produce a sign on numeric conversion
        | '-'           : left-align parameter within field
        | '0'           : left-pad with zeros rather than spaces
        | '#'           : use "alternate" representation
        | ','           : produce comma-separated triples
        | '\''          :  "      "
        ;

    field-width *(1)
        = DECIMAL       : an unsigned base-10 numeral
        | '*'           : take field width from args as type 'uint32_t'
        ;

    precision *(1)(2)(3)
        = DECIMAL       : an unsigned base-10 numeral
        | '*'           : take precision from args as type 'uint32_t'
        |               : an empty precision means 0
        ;

    index
        = idx           : a single, zero-based vector element
        | idx '-' idx   : a fully-closed, zero-based interval
        | idx '/' len   : a start index plus length
        ;

    idx
        = DECIMAL       : an unsigned base-10 numeral
        | '*'           : take index from args as type 'uint32_t'
        | '$'           : last vector element
        |               : an empty index means 0 or $
        ;

    len
        = DECIMAL       : a base-10 numeral
        | '*'           : take length from args as type 'uint32_t'
        | '$'           : length-of ( vector )
        |               : an empty length means $
        ;

    storage-class-width
        = 't'           : tiny integer ( i.e. byte )
        | 'h'           : half the normal size
        | 'l'           : twice the normal size
        | 'z'           : sizeof size_t
        | time-modifier
        ;

    time-modifier
        = 'h'           : date only
        | 'l'           : date and time
        | 'z'           : date, time and zone
        ;

    scalar storage-class
        = 'd' | 'i'     : decimal signed integer
        | 'u'           : decimal unsigned integer
        | 'x'           : lower-case hex integer
        | 'X'           : upper-case hex integer
        | 'o'           : octal integer
        | 'b'           : binary integer
        | 'p'           : hex void*
        | 'f'           : double
        | 'e'           : scientific notation double
        | 'g'           : general double
        | 'c'           : UTF-32 character
        | 'N'           : const KSymbol* [ <klib/symbol.h> ]
        | 'V' *(2)      : tri-part version [ ver_t ]
        | 'R'           : return code [ rc_t ]
        | 'T'           : const KTime*  [ <klib/time.h> ]
        | '!'           ; operating specific error code ( i.e. errno or GetLastError() )
        | 'n' *(5)      ; output of number of characters printed so far to uint32_t*
        ;

    single-index vector storage-class *(6)(7)
        = 'd' | 'i'     : as above
        | 'u' | 'x'     : index range is ignored
        | 'X' | 'o'     : start index is used
        | 'b' | 'p'     : to select element
        | 'f' | 'e' | 'g'
        | 'N'           : const KSymbol* [ <klib/symbol.h> ]
        | 'V' *(2)      : tri-part version [ ver_t ]
        | 'R'           : return code [ rc_t ]
        | 'T'           : const KTime*  [ <klib/time.h> ]
        ;

    index-range vector storage-class *(8)
        = 'c' *(9)      : unbounded character vector
        | 's' | 'S'     : bounded character vector
        ;

  Notes:
     1. field-width and precision measure characters, not bytes
     2. for version numbers, precision gives the number of fields,
        where 1 = major, 2 = major.minor and 3 = major.minor.release.
     3. in the absence of precision, versions are written with the
        minimum number of fields required.
     4. the storage-class-width is interpreted differently for storage-class
        'T' ( const KTime* )
     5. %n may not be supported for security purposes
     6. when an index is specified, the argument is taken as an
        unbounded vector. if an index range is given, only the
        starting index is used, and only a single element is selected.
     7. the vector reference class is determined by both the format
        and the size modifier.
     8. an index for character classes may specify a string as the
        selection. when missing, the default start index is 0 while
        the default end index is $.
     9. a character vector is NOT assumed to be NUL-terminated,
        and in this case the default end index is the start index.

 */


/* string_printf
 *  provides a facility similar to snprintf
 *  formatting is similar but differs somewhat [ see FORMAT at bottom ]
 *
 *  "dst" [ OUT ] and "bsize" [ IN ] - output buffer for string
 *  will be NUL-terminated if possible
 *
 *  "num_writ" [ OUT, NULL OKAY ] - returns the number of non-NUL bytes
 *  written to "dst" or the required "bsize" to complete successfully,
 *  not including the NUL termination.
 *
 *  "fmt" [ IN ] and "args" [ IN, OPTIONAL ] - data to write
 *
 *  returns 0 if all bytes were successfully written and a NUL-byte was
 *  written into the buffer.
 *
 *  returns rcBuffer, rcInsufficient if the buffer was too small. in this
 *  case, it is possible that the only missing byte would be the NUL
 *  termination, and the output string may still be usable since "num_writ"
 *  indicates the actual number of text bytes.
 */
KLIB_EXTERN rc_t CC string_printf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, ... );
KLIB_EXTERN rc_t CC string_vprintf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, va_list args );


/* KDataBufferPrintf
 *  provides a facility similar to string_printf
 *  except that it appends into a KDataBuffer
 *
 *  "buf" [ IN ] - previously initialized buffer
 *  MUST be initialized, or results are unpredictable
 *  ( except for the prediction that all will fail )
 */
KLIB_EXTERN rc_t CC KDataBufferPrintf ( struct KDataBuffer * buf,
    const char * fmt, ... );
KLIB_EXTERN rc_t CC KDataBufferVPrintf ( struct KDataBuffer * buf,
    const char * fmt, va_list args );


/*--------------------------------------------------------------------------
 * structured_printf
 *  the engine behind string_printf
 */

/* formatting
 */
enum
{
    spfNone,                                    /* 'n', sptTerm                       */
    spfText,                                    /* 'csS', literal                     */
    spfSignedInt,                               /* 'di'                               */
    spfUnsigned,                                /* 'bouxX'                            */
    spfStdFloat,                                /* 'f'                                */
    spfGenFloat,                                /* 'g'                                */
    spfSciFloat,                                /* 'e'                                */
    spfVersion,                                 /* 'V'                                */
    spfSymbol,                                  /* 'N'                                */
    spfTime,                                    /* [ 'hlz' + ] 'T'                    */
    spfRC,                                      /* 'R'                                */
    spfOSErr                                    /* '!'                                */
};

/* types
 */
enum
{
    sptTerm,                                    /* terminates format                  */
    sptLiteral,                                 /* char literal        - arg in fmt   */
    sptSignedInt,                               /* signed scalar int   - arg is d     */
    sptSignedInt8Vect,                          /* signed vector int   - arg is d8    */
    sptSignedInt16Vect,                         /* signed vector int   - arg is d16   */
    sptSignedInt32Vect,                         /* signed vector int   - arg is d32   */
    sptSignedInt64Vect,                         /* signed vector int   - arg is d64   */
    sptUnsignedInt,                             /* unsigned scalar int - arg is u     */
    sptUnsignedInt8Vect,                        /* unsigned vector int - arg is u8    */
    sptUnsignedInt16Vect,                       /* unsigned vector int - arg is u16   */
    sptUnsignedInt32Vect,                       /* unsigned vector int - arg is u32   */
    sptUnsignedInt64Vect,                       /* unsigned vector int - arg is u64   */
    sptFloat,                                   /* scalar float        - arg is f     */
    sptFloat32Vect,                             /* vector float        - arg is f32   */
    sptFloat64Vect,                             /* vector float        - arg is f64   */
    sptFloatLongVect,                           /* vector float        - arg is flong */
    sptChar,                                    /* scalar character    - arg is c     */
    sptNulTermString,                           /* vector character    - arg is s     */
    sptString,                                  /* vector character    - arg is S     */
    sptUCS2String,                              /* vector character    - arg is S     */
    sptUTF32String,                             /* vector character    - arg is S     */
    sptPointer,                                 /* object reference    - arg is p     */
    sptRowId,                                   /* current row id      - arg is d     */
    sptRowLen                                   /* current row length  - arg is u     */
#if SUPPORT_PERCENT_N
    , sptBytesPrinted                           /* output parameter    - arg is n     */
#endif
};


/* format
 *  structured and constant format information
 *  takes the place of format string
 *  flags indicate whether fields are active
 *  and in some cases whether the value is external, in the next arg
 */
typedef struct PrintFmt PrintFmt;
struct PrintFmt
{
    union
    {
        struct
        {
            uint64_t min_field_width;           /* minimum field width in characters  */
            uint64_t precision;                 /* precision in characters            */
            uint64_t start_idx;                 /* zero-based starting vector index   */
            uint64_t select_len;                /* length of selection or 0 for inf   */
        } f;
        struct
        {
            const char *text;                   /* string literal text pointer        */
            size_t size;                        /* string literal text size           */
        } l;
    } u;
    unsigned int upper_case_num       : 1;      /* upper-case numerals                */
    unsigned int reverse_alnum        : 1;      /* reverse ordering of alpha-numeric  */
    unsigned int thousands_separate   : 1;      /* separate numerals by thousands     */
    unsigned int ext_field_width      : 1;      /* field width is external argument   */
    unsigned int ext_precision        : 1;      /* precision is external argument     */
    unsigned int inf_start_index      : 1;      /* start index is last in vector      */
    unsigned int ext_start_index      : 1;      /* start index is external argument   */
    unsigned int inf_stop_index       : 1;      /* stop index is last in vector       */
    unsigned int ext_stop_index       : 1;      /* stop index is an external argument */
    unsigned int ext_select_len       : 1;      /* selection length is external       */
    unsigned int add_prefix           : 1;      /* '#' flag on 'boxX'                 */
    unsigned int force_decimal_point  : 1;      /* '#' flag on 'efg'                  */
    unsigned int leave_trailing_zeros : 1;      /* '#' flag on 'g'                    */
    unsigned int print_time           : 1;
    unsigned int print_date           : 1;
    unsigned int print_weekday        : 1;
    unsigned int print_timezone       : 1;
    unsigned int hour_24              : 1;
    unsigned int min_vers_components  : 1;
    unsigned int explain_rc           : 1;
    unsigned int type_cast            : 1;      /* if true, convert type for fmt      */
    unsigned int pointer_arg          : 1;      /* argument is a pointer              */
    uint32_t radix;                             /* 0 default, 2, 8, 10, 16, .. 36     */
    uint8_t fmt;                                /* spf... from above                  */
    uint8_t type;                               /* spt... from above                  */
    char sign;                                  /* 0, ' ' or '+'                      */
    char left_fill;                             /* 0 for left-align, ' ' or '0'       */
};


/* argument
 *  union of argument values
 *  passed as an array, much like va_arg
 */
typedef union PrintArg PrintArg;
union PrintArg
{
    int64_t d;                                  /* signed integer scalar              */
    const int8_t *d8;                           /* signed integer vectors             */
    const int16_t *d16;
    const int32_t *d32;
    const int64_t *d64;

    uint64_t u;                                 /* unsigned integer scalar            */
    const uint8_t *u8;                          /* unsigned integer vectors           */
    const uint16_t *u16;
    const uint32_t *u32;
    const uint64_t *u64;

    double f;                                   /* floating point scalar              */
    const float *f32;                           /* floating point vectors             */
    const double *f64;
    const long double *flong;

    uint32_t c;                                 /* character scalar                   */
    const char *s;                              /* NUL-terminated char vector         */
    struct String const *S;                     /* character vector                   */

    const void *p;                              /* object reference                   */

#if SUPPORT_PERCENT_N
    uint32_t *n;                                /* output parameter                   */
#endif
};


/* structured_printf
 *  uses constant format descriptors and argument block
 *  prints to "out" handler
 */
KLIB_EXTERN rc_t CC structured_printf ( struct KWrtHandler const *out,
    size_t *num_writ, const PrintFmt *fmt, const PrintArg *args );


/* structured_sprintf
 *  uses constant format descriptors and argument block
 *  prints to UTF-8 character buffer "dst"
 */
KLIB_EXTERN rc_t CC structured_sprintf ( char *dst, size_t bsize,
    size_t *num_writ, const PrintFmt *fmt, const PrintArg *args );


/* TEMPORARY */
KLIB_EXTERN rc_t CC new_string_printf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, ... );
KLIB_EXTERN rc_t CC new_string_vprintf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, va_list args );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_printf_ */
