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
#include <klib/token.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* for parser simplicity before the days of expressions,
   signed numerals were recognized as single tokens, i.e.
   (+/-)NUM with no separating white space was a single token */
#define SUPPORT_SIGNED_NUMERALS 0

/*--------------------------------------------------------------------------
 * KToken
 *  a string with an id and source information
 */

/* hex_to_int
 *  where 'c' is known to be hex
 */
static
unsigned int CC hex_to_int ( char c )
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
rc_t CC StringConvertHex ( const String *self, uint64_t *ip, int bits )
{
    uint32_t i;
    uint64_t i64 = hex_to_int ( self -> addr [ 2 ] );
    for ( i = 3; i < self -> len; ++ i )
    {
        i64 <<= 4;
        i64 += hex_to_int ( self -> addr [ i ] );
    }

    if ( bits < 64 && ( i64 >> bits ) != 0 )
        return RC ( rcVDB, rcToken, rcConverting, rcRange, rcExcessive );

    * ip = i64;
    return 0;
}

static
rc_t CC StringConvertOctal ( const String *self, uint64_t *ip, int bits )
{
    uint32_t i;
    uint64_t i64 = 0;
    for ( i = 1; i < self -> len; ++ i )
    {
        i64 <<= 3;
        i64 += self -> addr [ i ] - '0';
    }

    if ( bits < 64 && ( i64 >> bits ) != 0 )
        return RC ( rcVDB, rcToken, rcConverting, rcRange, rcExcessive );

    * ip = i64;
    return 0;
}

static
rc_t CC StringConvertDecimal ( const String *self, uint64_t *ip, int bits )
{
    uint32_t i;
    uint64_t i64;

    if ( bits > 32 )
    {
        i64 = self -> addr [ 0 ] - '0';
        for ( i = 1; i < self -> len; ++ i )
        {
            i64 *= 10;
            i64 += self -> addr [ i ] - '0';
        }
    }
    else
    {
        uint32_t i32 = self -> addr [ 0 ] - '0';
        for ( i = 1; i < self -> len; ++ i )
        {
            i32 *= 10;
            i32 += self -> addr [ i ] - '0';
        }
        i64 = i32;
    }

    if ( bits < 64 && ( i64 >> bits ) != 0 )
        return RC ( rcVDB, rcToken, rcConverting, rcRange, rcExcessive );

    * ip = i64;
    return 0;
}

LIB_EXPORT rc_t CC KTokenToI32 ( const KToken *self, int32_t *i )
{
    rc_t rc;
    int64_t i64;

    switch ( self -> id )
    {
    case eDecimal:
        rc = StringConvertDecimal ( & self -> str, ( uint64_t* ) & i64, 31 );
        break;
    case eHex:
        rc = StringConvertHex ( & self -> str, ( uint64_t* ) & i64, 31 );
        break;
    case eOctal:
        rc = StringConvertOctal ( & self -> str, ( uint64_t* ) & i64, 31 );
        break;
    default:
        return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
    }

    if ( rc == 0 )
        * i = ( int32_t ) i64;

    return rc;
}

LIB_EXPORT rc_t CC KTokenToU32 ( const KToken *self, uint32_t *i )
{
    rc_t rc;
    uint64_t i64;

    switch ( self -> id )
    {
    case eDecimal:
        rc = StringConvertDecimal ( & self -> str, & i64, 32 );
        break;
    case eHex:
        rc = StringConvertHex ( & self -> str, & i64, 32 );
        break;
    case eOctal:
        rc = StringConvertOctal ( & self -> str, & i64, 32 );
        break;
    default:
        return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
    }

    if ( rc == 0 )
        * i = ( uint32_t ) i64;

    return rc;
}

LIB_EXPORT rc_t CC KTokenToI64 ( const KToken *self, int64_t *i )
{
    switch ( self -> id )
    {
    case eDecimal:
        return StringConvertDecimal ( & self -> str, ( uint64_t* ) i, 63 );
    case eHex:
        return StringConvertHex ( & self -> str, ( uint64_t* ) i, 63 );
    case eOctal:
        return StringConvertOctal ( & self -> str, ( uint64_t* ) i, 63 );
    }

    return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
}

LIB_EXPORT rc_t CC KTokenToU64 ( const KToken *self, uint64_t *i )
{
    switch ( self -> id )
    {
    case eDecimal:
        return StringConvertDecimal ( & self -> str, i, 64 );
    case eHex:
        return StringConvertHex ( & self -> str, i, 64 );
    case eOctal:
        return StringConvertOctal ( & self -> str, i, 64 );
    }

    return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
}

LIB_EXPORT rc_t CC KTokenToF64 ( const KToken *self, double *d )
{
    char buffer [ 256 ], *end;

    switch ( self -> id )
    {
    case eDecimal:
    case eFloat:
    case eExpFloat:
        break;
    case eOctal:
        if ( self -> str . len == 1 )
            break;
    default:
        return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
    }

    if ( string_copy ( buffer, sizeof buffer,
             self -> str . addr, self -> str . size ) == sizeof buffer )
    {
        return RC ( rcVDB, rcToken, rcConverting, rcToken, rcExcessive );
    }

    * d = strtod ( buffer, & end );
    if ( ( end - buffer ) != self -> str . size )
        return RC ( rcVDB, rcToken, rcConverting, rcToken, rcInvalid );

    return 0;
}

LIB_EXPORT rc_t CC KTokenToVersion ( const KToken *self, uint32_t *vp )
{
    rc_t rc;
    String str;
    uint64_t i;
    uint32_t v = 0;

    const char *dot;
    const char *start = self -> str . addr;
    const char *end = start + self -> str . size;

    switch ( self -> id )
    {
        /* three-part versions */
    case eMajMinRel:
        dot = string_rchr ( start, end - start, '.' ) + 1;
        assert ( dot > start && dot < end );
        StringInit ( & str, dot, (size_t)( end - dot ), (uint32_t)( end - dot ) );
        rc = StringConvertDecimal ( & str, & i, 16 );
        if ( rc != 0 )
            break;

        v = ( uint32_t ) i;
        end = dot - 1;

        /* two-part versions */
    case eFloat:
        dot = string_rchr ( start, end - start, '.' );
        if ( dot == start )
            return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
        if ( ++ dot == end )
            return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
        StringInit ( & str, dot, (size_t)( end - dot ), (uint32_t)( end - dot ) );
        rc = StringConvertDecimal ( & str, & i, 8 );
        if ( rc != 0 )
            break;

        v |= ( uint32_t ) ( i << 16 );
        end = dot - 1;

        /* single-part versions */
    case eOctal:
    case eDecimal:
        StringInit ( & str, start, (size_t)( end - start ), (uint32_t)( end - start ) );
        rc = StringConvertDecimal ( & str, & i, 8 );
        if ( rc != 0 )
            break;

        v |= ( uint32_t ) ( i << 24 );
        break;

    default:
        return RC ( rcVDB, rcToken, rcConverting, rcType, rcIncorrect );
    }

    if ( rc == 0 )
        * vp = v;

    return rc;
}

LIB_EXPORT rc_t CC KTokenToString ( const KToken *self, char *buffer, size_t bsize, size_t *size )
{
    size_t i, j, sz = self -> str . size;

    switch ( self -> id )
    {
    case eEndOfInput:
        * size = 0;
        return 0;

    case eString:
        -- sz;
    case eUntermString:
        * size = sz - 1;

        if ( sz >= bsize )
            return RC ( rcVDB, rcToken, rcConverting, rcBuffer, rcInsufficient );

        string_copy ( buffer, bsize, self -> str . addr + 1, sz - 1 );
        return 0;

    case eEscapedString:
        -- sz;
    case eUntermEscapedString:
        /* leave initial quote at str[0] in count */
        break;

    default:
        * size = sz;

        if ( sz + 1 >= bsize )
            return RC ( rcVDB, rcToken, rcConverting, rcBuffer, rcInsufficient );

        string_copy ( buffer, bsize, self -> str . addr, sz );
        return 0;
    }

    /* walk across string starting at offset 1
       to skip initial quote that was left in "sz" */
    for ( i = 1, j = 0, -- bsize; i < sz; ++ i )
    {
        if ( j == bsize )
            return RC ( rcVDB, rcToken, rcConverting, rcBuffer, rcInsufficient );

        if ( ( buffer [ j ] = self -> str . addr [ i ] ) == '\\' )
        {
            if ( ++ i == sz )
                break;
            switch ( self -> str . addr [ i ] )
            {
                /* control characters */
            case 'n':
                buffer [ j ] = '\n';
                break;
            case 't':
                buffer [ j ] = '\t';
                break;
            case 'r':
                buffer [ j ] = '\r';
                break;
            case '0':
                buffer [ j ] = '\0';
                break;

            case 'a':
                buffer [ j ] = '\a';
                break;
            case 'b':
                buffer [ j ] = '\b';
                break;
            case 'v':
                buffer [ j ] = '\v';
                break;
            case 'f':
                buffer [ j ] = '\f';
                break;

            case 'x': case 'X':
                /* expect 2 additional hex characters */
                if ( ( i + 2 ) < sz &&
                     isxdigit ( self -> str . addr [ i + 1 ] ) &&
                     isxdigit ( self -> str . addr [ i + 2 ] ) )
                {
                    /* go ahead and convert */
                    buffer [ j ] = ( char )
                        ( ( hex_to_int ( self -> str . addr [ i + 1 ] ) << 4 ) |
                          hex_to_int ( self -> str . addr [ i + 2 ] ) );
                    i += 2;
                    break;
                }
                /* no break */

                /* just quote self */
            default:
                buffer [ j ] = self -> str . addr [ i ];
            }
        }

        ++ j;
    }

    * size = j;
    return 0;
}

static
rc_t CC utf8_utf32_cvt_string_copy ( uint32_t *dst, uint32_t blen, uint32_t *dlen, const char *src, size_t ssize )
{
    int rslt, len;
    const char *end = src + ssize;

    for ( len = 0; src < end; ++ len, src += rslt )
    {
        if ( len == blen )
            return RC ( rcVDB, rcToken, rcConverting, rcBuffer, rcInsufficient );

        rslt = utf8_utf32 ( & dst [ len ], src, end );
        if ( rslt <= 0 )
        {
            if ( rslt == 0 )
                return RC ( rcVDB, rcToken, rcConverting, rcString, rcInsufficient );

            return RC ( rcVDB, rcToken, rcConverting, rcString, rcCorrupt );
        }
    }

    * dlen = len;

    return 0;
}

LIB_EXPORT rc_t CC KTokenToWideString ( const KToken *self, uint32_t *buffer, uint32_t blen, uint32_t *len )
{
    int rslt;
    const char *start, *end;
    size_t i, sz = self -> str . size;

    switch ( self -> id )
    {
    case eEndOfInput:
        * len = 0;
        return 0;

    case eString:
        -- sz;
    case eUntermString:
        return utf8_utf32_cvt_string_copy ( buffer, blen, len, self -> str . addr + 1, sz - 1 );

    case eEscapedString:
        -- sz;
    case eUntermEscapedString:
        -- sz;
        break;

    default:
        return utf8_utf32_cvt_string_copy ( buffer, blen, len, self -> str . addr, sz );
    }

    /* get start and end pointers for utf8 string */
    start = self -> str . addr + 1;
    end = start + sz;

    for ( i = 0; start < end; ++ i, start += rslt )
    {
        if ( i == blen )
            return RC ( rcVDB, rcToken, rcConverting, rcBuffer, rcInsufficient );

        rslt = utf8_utf32 ( & buffer [ i ], start, end );
        if ( rslt <= 0 )
        {
            if ( rslt == 0 )
                return RC ( rcVDB, rcToken, rcConverting, rcString, rcInsufficient );

            return RC ( rcVDB, rcToken, rcConverting, rcString, rcCorrupt );
        }

        if ( buffer [ i ] == ( uint32_t ) '\\' )
        {
            rslt = utf8_utf32 ( & buffer [ i ], start += rslt, end );
            if ( rslt <= 0 )
            {
                if ( rslt == 0 )
                    return RC ( rcVDB, rcToken, rcConverting, rcString, rcInsufficient );

                return RC ( rcVDB, rcToken, rcConverting, rcString, rcCorrupt );
            }

            if ( rslt == 1 ) switch ( ( char ) buffer [ i ] )
            {
                /* control characters */
            case 'r':
                buffer [ i ] = '\r';
                break;
            case 'n':
                buffer [ i ] = '\n';
                break;
            case 't':
                buffer [ i ] = '\t';
                break;
            case '0':
                buffer [ i ] = '\0';
                break;

            case 'a':
                buffer [ i ] = '\a';
                break;
            case 'b':
                buffer [ i ] = '\b';
                break;
            case 'v':
                buffer [ i ] = '\v';
                break;
            case 'f':
                buffer [ i ] = '\f';
                break;

            case 'x': case 'X':
                /* expect 2 additional hex characters */
                if ( ( start + 2 ) < end &&
                     isxdigit ( start [ 1 ] ) &&
                     isxdigit ( start [ 2 ] ) )
                {
                    /* go ahead and convert */
                    buffer [ i ] =
                        ( ( hex_to_int ( start [ 1 ] ) << 4 ) |
                            hex_to_int ( start [ 2 ] ) );
                    start += 2;
                }
                break;

            case 'u': case 'U':
                /* expect 4 additional hex characters */
                if ( ( start + 4 ) < end &&
                     isxdigit ( start [ 1 ] ) &&
                     isxdigit ( start [ 2 ] ) &&
                     isxdigit ( start [ 3 ] ) &&
                     isxdigit ( start [ 4 ] ) )
                {
                    /* go ahead and convert */
                    buffer [ i ] =
                        ( ( hex_to_int ( start [ 1 ] ) << 12 ) |
                          ( hex_to_int ( start [ 2 ] ) << 8 ) |
                          ( hex_to_int ( start [ 3 ] ) << 4 ) |
                            hex_to_int ( start [ 4 ] ) );
                    start += 4;
                }
                break;
            }
        }
    }

    * len = (uint32_t)i;
    return 0;
}


/*--------------------------------------------------------------------------
 * KTokenSource
 */


/* Return
 *  returns token to source
 */
LIB_EXPORT void CC KTokenSourceReturn ( KTokenSource *self, const KToken *t )
{
    if ( self != NULL && t != NULL )
    {
        if ( t -> str . addr + t -> str . size == self -> str . addr )
        {
            self -> str . addr = t -> str . addr;
            self -> str . size += t -> str . size;
            self -> str . len += t -> str . len;
        }
    }
}


/* Consume
 *  consumes all remaining data
 */
LIB_EXPORT void CC KTokenSourceConsume ( KTokenSource *self )
{
    if ( self != NULL )
    {
        self -> str . addr += self -> str . size;
        self -> str . len = 0;
        self -> str . size = 0;
    }
}


/* Extend
 *  try to make more data available
 */
static
rc_t CC KTokenSourceExtend ( KTokenSource *src )
{
    KTokenText *tt = ( KTokenText* ) src -> txt;
    if ( tt -> read != NULL )
    {
        rc_t rc = ( * tt -> read ) ( tt -> data, tt, src -> str . size );
        if ( rc != 0 )
            return rc;
        src -> str = tt -> str;
    }
    return 0;
}


/*--------------------------------------------------------------------------
 * KTokenizer
 *  an encapsulation of the tokenizer code
 */

/* Next
 *  get next token
 */
#if CHECK_TOKENIZER
static KToken * CC KTokenizerNextInt
#else
LIB_EXPORT KToken * CC KTokenizerNext
#endif
 ( const KTokenizer *self, KTokenSource *src, KToken *t )
{
    int id;
    const unsigned char *start, *stop;
    bool return_eoln, allow_slash_slash, allow_hash_cmt, try_extend, path_ident;

#define ispathident(c) ( isalnum (c) || (c) == '.' || (c) == '-' || (c) == '_' )

    if ( t == NULL || src == NULL )
        return NULL;

    start = ( const unsigned char* ) src -> str . addr;
    stop = start + src -> str . size;

    /* glue token to text */
    t -> txt = src -> txt;

    /* no symbol */
    t -> sym = NULL;

    /* detect returning eoln */
    return_eoln = ( self == kLineTokenizer );

    /* the POSIX tokenizer doesn't want // comments */
    allow_slash_slash = ( self != kPOSIXPathTokenizer );
    allow_hash_cmt = ( self == kPOSIXPathTokenizer );
    path_ident =  ( self == kPOSIXPathTokenizer );

restart:

    /* skip over blanks */
    while ( start < stop && isspace ( * start ) )
    {
        switch ( * start )
        {
        case '\r':
            t -> str . addr = ( const char* ) start;
            if ( start + 1 < stop && start [ 1 ] == '\n' )
                ++ start;
            if ( ! return_eoln )
            {
                ++ src -> lineno;
                break;
            }
            else
            {
        case '\n':
                t -> str . addr = ( const char* ) start;
                if ( start + 1 < stop && start [ 1 ] == '\r' )
                    ++ start;
                if ( ! return_eoln )
                {
                    ++ src -> lineno;
                    break;
                }
            }

            t -> str . len = (uint32_t)( start - ( const unsigned char* ) t -> str . addr );
            t -> str . size = t -> str . len;
            t -> lineno = src -> lineno ++;
            t -> id = eEndOfLine;

            src -> str . size -= start - ( const unsigned char* ) src -> str . addr;
            src -> str . len -= (uint32_t)( start - ( const unsigned char* ) src -> str . addr );
            src -> str . addr = ( const char* ) start;

            return t;
        }

        ++ start;
    }

    /* detect end */
    if ( start == stop )
    {
        src -> str . addr = ( const char* ) start;
        src -> str . size = 0;
        src -> str . len = 0;

        KTokenSourceExtend ( src );
        start = ( const unsigned char* ) src -> str . addr;
        stop = start + src -> str . size;
        if ( start != stop )
            goto restart;

        t -> str = src -> str;
        t -> lineno = src -> lineno;
        t -> id = eEndOfInput;
        return t;
    }

    /* record start of token */
    t -> str . addr = ( const char* ) start;
    t -> lineno = src -> lineno;

    /* many symbols do not need extend */
    try_extend = false;

    /* tokenize */
    if ( isdigit ( * start ) )
    {
        try_extend = true;

        /* assume numeral */
        if ( * start != '0' )
        {
            id = eDecimal;
#if SUPPORT_SIGNED_NUMERALS
tokenize_decimal:
#endif
            while ( ++ start < stop )
            {
                if ( ! isdigit ( * start ) )
                {
                    if ( isalpha ( * start ) || * start == '_' )
                    {
                        if ( id != eDecimal )
                            break;
                        id = eName;
                        goto tokenize_name;
                    }
                    if ( * start == '.' )
                        goto tokenize_float;
                    break;
                }
            }
        }
        else
        {
            if ( ( stop - start ) >= 3 &&
                 ( start [ 1 ] == 'x' || start [ 1 ] == 'X' ) &&
                 isxdigit ( start [ 2 ] ) )
            {
                id = eHex;
                for ( start += 3; start < stop; ++ start )
                {
                    if ( ! isxdigit ( * start ) )
                    {
                        if ( isalpha ( * start ) || * start == '_' )
                        {
                            id = eName;
                            goto tokenize_name;
                        }
                        break;
                    }
                }
            }
            else
            {
                id = eOctal;
                while ( ++ start < stop )
                {
                    if ( ! isdigit ( * start ) )
                    {
                        if ( isalpha ( * start ) || * start == '_' )
                        {
                            id = eName;
                            goto tokenize_name;
                        }
                        if ( * start == '.' )
                            goto tokenize_float;
                        break;
                    }

                    if ( * start > '7' )
                        id = eDecimal;
                }
            }
        }
    }
    else if ( isalpha ( * start ) )
    {
tokenize_ident:
        try_extend = true;

        id = eIdent;

tokenize_name:
        while ( ++ start < stop )
        {
            if ( path_ident && ( *start  == '.' || *start == '-' ) )
                continue;

            if ( ! isalnum ( * start ) && * start != '_' )
                break;
        }
    }
    else switch ( * start ++ )
    {
    case '_':
        -- start;
        goto tokenize_ident;
    case '\'':
        try_extend = true;
        for ( id = eUntermString; start < stop; ++ start )
        {
            if ( * start == '\'' )
            {
                ++ id;
                start += 1;
                break;
            }

            if ( * start == '\\' )
            {
                id = eUntermEscapedString;
                if ( ++ start == stop )
                    break;
            }
        }
        break;

    case '"':
        try_extend = true;
        for ( id = eUntermString; start < stop; ++ start )
        {
            if ( * start == '"' )
            {
                ++ id;
                start += 1;
                break;
            }

            if ( * start == '\\' )
            {
                id = eUntermEscapedString;
                if ( ++ start == stop )
                    break;
            }
        }
        break;

    case '.':
        if ( path_ident ) /* posix path parsing enabled */
        {
            /* find special identifier ".." by finding two periods not followed by 
             * another identifier character
             */
            if ( start [ 0 ] == '.' && ! ispathident ( start [ 1 ]))
            {
                id = eDblPeriod;
                break;
            }
            if ( isalnum ( * start ) || * start == '.' || * start == '_' || * start == '-' )
            {
                id = eName;
                goto tokenize_name;
            }
        }
        if ( stop > start && isdigit ( * start ) )
        {
tokenize_float:
            try_extend = true;
            id = eFloat;
            while ( ++ start < stop )
            {
                if ( ! isdigit ( * start ) )
                {
                    if ( id != eFloat )
                        break;
                    if ( ( stop - start ) < 2 || ! isdigit ( start [ 1 ] ) )
                        break;
                    if ( * start == '.' )
                        id = eMajMinRel;
                    else if ( * start == 'e' || * start == 'E' )
                        id = eExpFloat;
                    else
                    {
                        break;
                    }
                    ++ start;
                }
            }
            break;
        }

        if ( ( stop - start ) < 2 )
            try_extend = true;
        else if ( start [ 0 ] == '.' && start [ 1 ] == '.'  )
        {
            id = eEllipsis;
            start += 2;
            break;
        }

        id = ePeriod;
        break;

    case ',':
        id = eComma;
        break;
    case ':':
        id = eColon;
        if ( start == stop )
            try_extend = true;
        else
        {
            switch ( start [ 0 ] )
            {
            case ':':
                id = eDblColon;
                start += 1;
                break;
            case '=':
                id = eColonAssign;
                start += 1;
                break;
            }
        }
        break;
    case ';':
        id = eSemiColon;
        break;
    case '?':
        id = eQuestion;
        break;
    case '~':
        id = eTilde;
        break;
    case '!':
        id = eExclam;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '=' )
        {
            id = eNotEqual;
            start += 1;
        }
        break;
    case '@':
        id = eAtSign;
        break;
    case '#':
        if ( allow_hash_cmt )
            goto line_cmt;
        id = eHash;
        break;
    case '$':
        id = eDollar;
        break;
    case '%':
        id = ePercent;
        break;
    case '^':
        id = eCaret;
        break;
    case '&':
        id = eAmpersand;
        if ( start == stop )
            try_extend = true;
        if ( start [ 0 ] == '&' )
        {
            id = eLogAnd;
            start += 1;
        }
        break;
    case '*':
        id = eAsterisk;
        break;
    case '+':
#if SUPPORT_SIGNED_NUMERALS
        if ( start == stop )
            try_extend = true;
        else
        {
            if ( isdigit ( * start ) )
            {
                id = eSignedDec;
                try_extend = true;
                goto tokenize_decimal;
            }
            if ( start [ 0 ] == '=' )
            {
                id = ePlusAssign;
                start += 1;
                break;
            }
        }
#endif
        id = ePlus;
        break;
    case '-':
#if SUPPORT_SIGNED_NUMERALS
        if ( start == stop )
            try_extend = true;
        else
        {
            if ( isdigit ( * start ) )
            {
                id = eSignedDec;
                try_extend = true;
                goto tokenize_decimal;
            }
            if ( * start == '>' )
            {
                id = eOverArrow;
                start += 1;
                break;
            }
        }
#endif
        id = eMinus;
        break;
    case '=':
        id = eAssign;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '=' )
        {
            id = eEqual;
            start += 1;
        }
        break;
    case '/':
        id = eFwdSlash;
        if ( start == stop )
            try_extend = true;
        else switch ( start [ 0 ] )
        {
        case '/':
            if ( ! allow_slash_slash )
                break;
            ++ start;
line_cmt:
            try_extend = true;
            for ( ; start < stop; ++ start )
            {
                switch ( start [ 0 ] )
                {
                case '\r':
                case '\n':
                    goto comment_restart;
                }
            }
comment_restart:
            src -> str . len -= string_len ( src -> str . addr, ( const char* ) start - src -> str . addr );
            src -> str . size -= ( const char* ) start - src -> str . addr;
            src -> str . addr = ( const char* ) start;
            goto restart;

        case '*':
            try_extend = true;
            id = eUntermComment;
            if ( ++ start < stop ) switch ( start [ 0 ] )
            {
            case '\r':
                if ( start + 1 < stop && start [ 1 ] == '\n' )
                    ++ start;
                ++ src -> lineno;
                break;
            case '\n':
                if ( start + 1 < stop && start [ 1 ] == '\r' )
                    ++ start;
                ++ src -> lineno;
                break;
            }
            for ( ++ start; start < stop; ++ start )
            {
                switch ( start [ 0 ] )
                {
                case '/':
                    if ( start [ -1 ] == '*' )
                    {
                        ++ start;
                        goto comment_restart;
                    }
                    break;
                case '\r':
                    if ( start + 1 < stop && start [ 1 ] == '\n' )
                        ++ start;
                    ++ src -> lineno;
                    break;
                case '\n':
                    if ( start + 1 < stop && start [ 1 ] == '\r' )
                        ++ start;
                    ++ src -> lineno;
                    break;
                }
            }
            break;
        }
        break;
    case '\\':
        id = eBackSlash;
        break;
    case '|':
        id = ePipe;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '|' )
        {
            id = eLogOr;
            start += 1;
        }
        break;
    case '(':
        id = eLeftParen;
        break;
    case ')':
        id = eRightParen;
        break;
    case '{':
        id = eLeftCurly;
        break;
    case '}':
        id = eRightCurly;
        break;
    case '<':
        id = eLeftAngle;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '<' )
        {
            id = eDblLeftAngle;
            start += 1;
        }
        break;
    case '>':
        id = eRightAngle;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '>' )
        {
            id = eDblRightAngle;
            start += 1;
        }
        break;
    case '[':
        id = eLeftSquare;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == '[' )
        {
            id = eDblLeftSquare;
            start += 1;
        }
        break;
    case ']':
        id = eRightSquare;
        if ( start == stop )
            try_extend = true;
        else if ( start [ 0 ] == ']' )
        {
            id = eDblRightSquare;
            start += 1;
        }
        break;
    default:
        /* skip over single UTF-8 character */
        if ( ( ( const char* ) start ) [ -1 ] < 0 )
        {
            while ( start < stop )
            {
                if ( ( * start & 0xC0 ) != 0x80 )
                    break;
                ++ start;
            }
        }

        t -> str . len = 1;
        t -> str . size = start - ( const unsigned char* ) t -> str . addr;

        src -> str . addr = ( const char* ) start;
        src -> str . len -= 1;
        src -> str . size -= t -> str . size;

        t -> id = eUnrecognized;
        return t;
    }

    t -> str . len = (uint32_t)( start - ( const unsigned char* ) t -> str . addr );
    t -> str . size = t -> str . len;
    t -> id = id;

    if ( ! try_extend || start < stop )
    {
        src -> str . size -= start - ( const unsigned char* ) src -> str . addr;
        src -> str . len -= (uint32_t)( start - ( const unsigned char* ) src -> str . addr );
        src -> str . addr = ( const char* ) start;
    }
    else
    {
        /* state that the amount left in source
           is exactly what was tokenized */
        src -> str = t -> str;

        /* try to extend, which is successful
           only if src ends up with more data */
        KTokenSourceExtend ( src );
        if ( src -> str . size > t -> str . size )
        {
            start = ( const unsigned char* ) src -> str . addr;
            stop = start + src -> str . size;
            goto restart;
        }

        /* out of data on source */
        src -> str . addr = ( const char* ) stop;
        src -> str . size = 0;
        src -> str . len = 0;
    }

    return t;
}

#if CHECK_TOKENIZER
#include <stdio.h>
static
void CC examine_src ( const KTokenSource *src, bool before )
{
    const KTokenText *tt = src -> txt;
    const char *stage = before ? "before" : "after";

    /* first test - are src addr and size correct */
    if ( src -> str . addr + src -> str . size !=
         tt -> str . addr + tt -> str . size )
    {
        /* in the case of console input before tokenization, this is allowed */
        if ( before && src -> str . size == 0 && tt -> read != NULL )
            return;

        /* check for src addr being within text */
        if ( src -> str . addr < tt -> str . addr ||
             src -> str . addr >= tt -> str . addr + tt -> str . size )
        {
            fprintf ( stderr, "KTokenizerNext: bad src address %s scan\n", stage );
        }
        else
        {
            fprintf ( stderr, "KTokenizerNext: bad src size ( %lu ) %s scan "
                      "at file \"%.*s\" offset %lu\n"
                      , ( long unsigned ) src -> str . size
                      , stage
                      , ( int ) tt -> path . size, tt -> path . addr
                      , ( long unsigned ) ( src -> str . addr - tt -> str . addr ) );
        }
    }

    /* test number of characters */
    else
    {
        uint32_t len = string_len ( src -> str . addr, src -> str . size );
        if ( len != src -> str . len )
        {
            fprintf ( stderr, "KTokenizerNext: bad src character count "
                      "( %u measured, %u stated ) %s scan\n", len, src -> str . len, stage );
        }
    }
}

LIB_EXPORT KToken * CC KTokenizerNext ( const KTokenizer *self, KTokenSource *src, KToken *t )
{
    KToken *rt;
    examine_src ( src, true );
    rt = KTokenizerNextInt ( self, src, t );
    examine_src ( src, false );
    return rt;
}
#endif
