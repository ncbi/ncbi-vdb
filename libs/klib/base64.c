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
#include <klib/base64.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>

/* don't yet have any indication of how else to do it */
#define BASE64_PAD_ENCODING 1

/* from binary 0..63 to standard BASE64 encoding */
static
const char encode_std_table [] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "+/";

/* from octet stream ( presumed standard BASE64 encoding )
 * to binary, where
 *   0..63 is valid output
 *   -1 is invalid output
 *   -2 is padding
 *   -3 would normally mean ignore
 */
static
const char decode_std_table [] =
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x00 .. \x0F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x10 .. \x1F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x3e\xff\xff\xff\x3f" /* \x20 .. \x2F */
    "\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\xff\xff\xff\xfe\xff\xff" /* \x30 .. \x3F */
    "\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e" /* \x40 .. \x4F */
    "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\xff\xff\xff\xff\xff" /* \x50 .. \x5F */
    "\xff\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28" /* \x60 .. \x6F */
    "\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\xff\xff\xff\xff\xff" /* \x70 .. \x7F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x80 .. \x8F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x90 .. \x9F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xA0 .. \xAF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xB0 .. \xBF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xC0 .. \xCF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xD0 .. \xDF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xE0 .. \xEF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xF0 .. \xFF */
    ;

/* from binary 0..63 to BASE64-URL encoding */
static
const char encode_url_table [] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_";

/* from octet stream ( presumed BASE64-URL encoding )
 * to binary, where
 *   0..63 is valid output
 *   -1 is invalid output
 *   -2 is padding
 *   -3 would normally mean ignore
 */
static
const char decode_url_table [] =
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x00 .. \x0F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x10 .. \x1F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x3e\xff\xff" /* \x20 .. \x2F */
    "\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\xff\xff\xff\xfe\xff\xff" /* \x30 .. \x3F */
    "\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e" /* \x40 .. \x4F */
    "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\xff\xff\xff\xff\x3f" /* \x50 .. \x5F */
    "\xff\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28" /* \x60 .. \x6F */
    "\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\xff\xff\xff\xff\xff" /* \x70 .. \x7F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x80 .. \x8F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \x90 .. \x9F */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xA0 .. \xAF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xB0 .. \xBF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xC0 .. \xCF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xD0 .. \xDF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xE0 .. \xEF */
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" /* \xF0 .. \xFF */
    ;

static
rc_t encodeBase64Impl ( const String ** encoded, const void * data, size_t bytes,
                        const char encode_table [] )
{
    char *buff;
    size_t i, j, esize;
    
    /* gather encoded output in a string - this is why we wanted to limit the data size */
    String *encoding;

    if ( encoded == NULL )
        return RC ( rcRuntime, rcString, rcEncoding, rcParam, rcNull );

    * encoded = NULL;
    
    /* allow an empty source */
    if ( bytes == 0 )
    {
        String empty;
        CONST_STRING ( & empty, "" );
        return StringCopy ( encoded, & empty );
    }
    
    /* this exception represents an internal error in any case */
    if ( data == NULL )
        return RC ( rcRuntime, rcString, rcEncoding, rcParam, rcNull );
    
    /* TBD - place an upper limit on data size
     * a very large payload will create a very large string allocation
     * and may indicate garbage that could result in a segfault */
    if ( bytes >= 0x40000000 )
        return RC ( rcRuntime, rcString, rcEncoding, rcData, rcExcessive );

    esize = ( ( bytes + 2 ) / 3 ) * 4;
    
    encoding = malloc ( sizeof *encoding + esize + 1 );
    if ( encoding == NULL )
        return RC ( rcRuntime, rcString, rcEncoding, rcMemory, rcExhausted );

    /* perform work in a stack-local buffer to reduce
    * memory manager thrashing when adding to "encoding" string */
    buff = ( char * ) ( encoding + 1 );

    { /* scope trickery */
        
        /* accumulate 24 bits of input at a time into a 32-bit accumulator */
        uint32_t acc;
        
        /* walk across data as a block of octets */
        const unsigned char * js = ( const unsigned char * ) data;
        
        /* consume 3 octets of input while producing 4 output characters */
        for ( i = j = 0; i + 3 <= bytes; i += 3, j += 4 )
        {
            /* bring in 24 bits in specific order */
            acc
                = ( ( uint32_t ) js [ i + 0 ] << 16 )
                | ( ( uint32_t ) js [ i + 1 ] <<  8 )
                | ( ( uint32_t ) js [ i + 2 ] <<  0 )
                ;
                        
            /* produce the 4 bytes of output through the provided encoding table
             * each index MUST be 0..63 */
            buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
            buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
            buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];
            buff [ j + 3 ] = encode_table [ ( acc >>  0 ) & 0x3F ];
        }
    
        /* at this point, the data block is either completely converted
        * or has 1 or 2 bytes left over, since ( bytes % 3 ) is in { 0, 1, 2 }
        * we know "i" is a multiple of 3, and ( bytes - i ) == ( bytes % 3 ) */
        switch ( bytes - i )
        {
        case 0:
            /* everything is complete */
            break;
            
        case 1:
            
            /* 1 octet left over
             * place it in the highest part of 24-bit accumulator */
            acc
                = ( ( uint32_t ) js [ i + 0 ] << 16 )
                ;
                        
            /* emit single octet split between two encoding characters */
            buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
            buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
            
            /* pad the remaining two with padding character */
            buff [ j + 2 ] = '=';
            buff [ j + 3 ] = '=';
            
#if BASE64_PAD_ENCODING
            /* total number of characters in buffer includes padding */
            j += 4;
#else
            /* total number of characters in buffer does not include padding */
            j += 2;
#endif
            
            break;
            
        case 2:
            
            /* 2 octets left over
             * place them in the upper part of 24-bit accumulator */
            acc
                = ( ( uint32_t ) js [ i + 0 ] << 16 )
                | ( ( uint32_t ) js [ i + 1 ] <<  8 )
                ;
                        
            /* emit the two octets split across three encoding characters */
            buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
            buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
            buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];
            
            /* pad the remainder with padding character */
            buff [ j + 3 ] = '=';
            
#if BASE64_PAD_ENCODING
            /* total number of characters in buffer includes padding */
            j += 4;
#else
            /* total number of characters in buffer does not include padding */
            j += 3;
#endif
            
            break;
            
        default:
        
            /* this is theoretically impossible */
            free ( encoding );
            return RC ( rcRuntime, rcString, rcEncoding, rcConstraint, rcViolated );
        }
    }
        
    buff [ j ] = 0;
    StringInit ( encoding, buff, j, ( uint32_t ) j );
    
    /* done. */
    *encoded = encoding;
    return 0;
}

static
rc_t decodeBase64Impl ( KDataBuffer *decoded, const String *encoding, const char decode_table [] )
{
    rc_t rc;
    
    /* base the estimate of decoded size on input size
     * this can be over-estimated due to embedded padding or formatting characters */
    size_t i, j, len;
    
    uint32_t acc, ac;
    const unsigned char * en;
    
    /* the returned buffer should be 3/4 the size of the input string,
     * provided that there are no padding bytes in the input */
    size_t bytes;
    unsigned char * buff;

    if ( decoded == NULL || encoding == NULL )
        return RC ( rcRuntime, rcString, rcDecoding, rcParam, rcNull );

    len = StringSize ( encoding );
    en = ( const unsigned char * ) encoding -> addr;
    bytes = ( ( len + 3 ) / 4 ) * 3;
    
    /* create an output buffer */
    rc = KDataBufferMakeBytes ( decoded, bytes );
    if ( rc != 0 )
        return rc;
    
    buff = decoded -> base;
    
    /* walk across the input string a byte at a time
     * avoid temptation to consume 4 bytes at a time,
     * in order to be robust to any allowed stray characters
     * NB - if this proves to be a performance issue, it can
     * be optimized in the future. */
    for ( i = j = 0, acc = ac = 0; i < len; ++ i )
    {
        /* return from table is a SIGNED entity */
        int byte = decode_table [ en [ i ] ];
        
        /* non-negative lookups are valid translations */
        if ( byte >= 0 )
        {
            /* must be 0..63 */
            assert ( byte < 64 );
            
            /* shift 6 bits into accumulator */
            acc <<= 6;
            acc |= byte;
            
            /* if the number of codes in accumulator is 4 ( i.e. 24 bits ) */
            if ( ++ ac == 4 )
            {                
                /* put 3 octets into payload */
                buff [ j + 0 ] = ( unsigned char ) ( acc >> 16 );
                buff [ j + 1 ] = ( unsigned char ) ( acc >>  8 );
                buff [ j + 2 ] = ( unsigned char ) ( acc >>  0 );
                
                /* clear the accumulator */
                ac = 0;
                
                /* keep track of size
                 * NB - this is not YET reflected in payload */
                 j += 3; 
            }
        }
        
        /* NEGATIVE lookups have to be interpreted */
        else
        {
            /* the special value -2 means padding
             * which indicates the end of the input */
            if ( byte == -2 )
                break;
            
            /* the special value -3 would indicate ignore
             * but it's not allowed in JWT and so is not expected to be in table
             * any other value ( notably -1 ) is illegal */
            if ( byte != -3 )
            {
                KDataBufferWhack ( decoded );
                return RC ( rcRuntime, rcString, rcDecoding, rcData, rcInvalid );
            }
        }
    }
    
    /* test the number of 6-bit codes remaining in the accumulator */
    switch ( ac )
    {
    case 0:
        /* none - everything has been converted */
        break;
        
    case 1:
        /* encoding granularity is an octet - 8 bits
         * it MUST be split across two codes - 6 bits each, i.e. 12 bits
         * having only 6 bits in accumulator is illegal */
        KDataBufferWhack ( decoded );
        return RC ( rcRuntime, rcString, rcDecoding, rcConstraint, rcViolated );
        
    case 2:
        
        /* fill accumulator with two padding codes
         * NB - not strictly necessary, but keeps code regular and readable */
        acc <<= 12;
                
        /* set additional octet */
        buff [ j + 0 ] = ( char ) ( acc >> 16 );
        
        /* account for size
         * NB - this is not YET reflected in payload */
        j += 1;
        break;
        
    case 3:
        
        /* fill accumulator with padding */
        acc <<= 6;
                
        /* set additional bytes */
        buff [ j + 0 ] = ( char ) ( acc >> 16 );
        buff [ j + 1 ] = ( char ) ( acc >>  8 );
        
        j += 2;
        break;
        
    default:
        
        /* theoretically impossible */
        KDataBufferWhack ( decoded );
        return RC ( rcRuntime, rcString, rcDecoding, rcConstraint, rcViolated );
    }

    assert ( j <= bytes );
    return KDataBufferResize ( decoded, j );
}

/* encodeBase64
 *  encode a buffer of binary data as string of base64 ASCII characters
 *
 *  "encoded" [ OUT ] - base64 encoded string representing input data
 *  must be freed with StringWhack() [ see <klib/text.h> ]
 *
 *  "data" [ IN ] and "bytes" [ IN ] - buffer of binary data to be encoded
 */
LIB_EXPORT rc_t CC encodeBase64 ( const String ** encoded, const void * data, size_t bytes )
{
    return encodeBase64Impl ( encoded, data, bytes, encode_std_table );
}

LIB_EXPORT rc_t CC encodeBase64URL ( const String ** encoded, const void * data, size_t bytes )
{
    return encodeBase64Impl ( encoded, data, bytes, encode_url_table );
}

/* decodeBase64
 *  decode a string of base64 ASCII characters into a buffer of binary data
 *
 *  "decoded" [ OUT ] - pointer to an UNINITIALIZED KDataBuffer structure that
 *  will be initialized by the function to contain decoded binary data. must be
 *  freed with KDataBufferWhack() [ see <klib/data-buffer.h> ]
 *
 *  "encoding" [ IN ] - base64-encoded text representation of data
 */
LIB_EXPORT rc_t CC decodeBase64 ( KDataBuffer * decoded, const String * encoding )
{
    return decodeBase64Impl ( decoded, encoding, decode_std_table );
}

LIB_EXPORT rc_t CC decodeBase64URL ( KDataBuffer * decoded, const String * encoding )
{
    return decodeBase64Impl ( decoded, encoding, decode_url_table );
}


