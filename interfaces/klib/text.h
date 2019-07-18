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

#ifndef _h_klib_text_
#define _h_klib_text_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * UTF-8
 *  UNICODE TRANFORMATION FORMAT into sequences of 8-bit bytes
 *
 *  An ASCII character encoded with UTF-8 formatting will have a bit
 *  pattern of 0b0xxxxxxx.
 *
 *  A non-ASCII UNICODE character encoded with UTF-8 formatting will
 *  use 2..6 bytes, where the character length will be encoded into
 *  the number of contiguous bits in the leading byte:
 *
 *    0b110xxxxx => 2 byte character
 *    0b1110xxxx => 3 byte character
 *    0b11110xxx => 4 byte character
 *    0b111110xx => 5 byte character
 *    0b1111110x => 6 byte character
 *
 *  all UTF-8 characters uniformly represent their non-leading bytes
 *  by having the MSB set and the next bit 0:
 *
 *    0b10xxxxxx => non-leading byte
 *
 *  this allows a pointer to an arbitrary byte within UTF-8 to
 *  be used to synchronize on the start of a character, since
 *  non-starting bytes cannot be confused with start bytes. The test
 *
 *    if ( ( * utf8 & 0xC0 ) == 0x80 )
 *      start_byte = false;
 *
 *  is sufficient for detecting this.
 *
 *  an understanding that string LENGTH ( the count of characters )
 *  and string SIZE ( the count of bytes ) are different is critical.
 */

    
/*--------------------------------------------------------------------------
 * String
 *  pseudo-intrinsic string
 *
 *  "addr" gives a pointer to constant UTF-8 data
 *
 *  "size" gives the number of BYTES in the UTF-8 string,
 *   NOT the number of characters.
 *
 *  "len" gives the number of CHARACTERS in the UTF-8 string,
 *   NOT the number of bytes.
 *
 *  with UTF-8 encoding, only ASCII-7 characters will be represented
 *  using a single byte per character. In the case that a string is
 *  composed of 100% ASCII-7 characters, String.size == String.len,
 *  while if there is even a single UTF-8 character,
 *  String.size > String.len by definition.
 *
 *  indexing into a string by BYTES is ridiculous because it may
 *  land in the middle of a character. String operations index by
 *  CHARACTER so as to always designate an entire character.
 */
typedef struct String String;
struct String
{
    const char *addr;
    size_t size;
    uint32_t len;
};

/* StringInit
 *  initializes a String object
 */
#define StringInit( s, val, sz, length ) \
    ( void ) \
        ( ( s ) -> addr = ( val ), \
          ( s ) -> size = ( sz ), \
          ( s ) -> len = ( length ) )

#define StringInitCString( s, cstr ) \
    ( void ) \
        ( ( s ) -> len = string_measure \
          ( ( s ) -> addr = ( cstr ), & ( s ) -> size ) )

/* CONST_STRING
 *  initialize a string from a manifest constant
 */
#define CONST_STRING( s, val ) \
    StringInit ( s, val, sizeof val - 1, sizeof val - 1 )

/* StringSize
 *  size of string in BYTES
 */
#define StringSize( s ) \
    ( s ) -> size

/* StringLength
 *  length of string in CHARACTERS
 */
#define StringLength( s ) \
    ( s ) -> len

/* StringCopy
 *  allocates a copy of a string
 */
KLIB_EXTERN rc_t CC StringCopy ( const String **cpy, const String *str );

/* StringConcat
 *  concatenate one string onto another
 */
KLIB_EXTERN rc_t CC StringConcat ( const String **cat,
    const String *a, const String *b );

/* StringSubstr
 *  creates a substring of an existing one
 *  note that the substring is always a non-allocated copy
 *  and is dependent upon the lifetime of its source
 *
 *  returns "sub" if "idx" was valid
 *  or NULL otherwise
 *
 *  "len" may be 0 to indicate infinite length
 *  or may extend beyond end of source string.
 */
KLIB_EXTERN String* CC StringSubstr ( const String *str,
    String *sub, uint32_t idx, uint32_t len );

/* StringTrim
 *  trims ascii white-space from both ends
 *  returns trimmed string in "trimmed"
 */
KLIB_EXTERN String * CC StringTrim ( const String * str, String * trimmed );

/* StringHead
 *  access the first character
 *
 *  this is an efficient enough function to be included.
 *  the generic functions of accessing characters by index
 *  are apt to be extremely inefficient with UTF-8, and
 *  as such are not included.
 *
 *  returns EINVAL if the character is bad,
 *  or ENODATA if the string is empty
 */
KLIB_EXTERN rc_t CC StringHead ( const String *str, uint32_t *ch );

/* StringPopHead
 *  remove and return the first character
 *
 *  returns EINVAL if the character is bad,
 *  or ENODATA if the string is empty
 */
KLIB_EXTERN rc_t CC StringPopHead ( String *str, uint32_t *ch );

/* StringEqual
 *  compare strings for equality
 *  not lexical for all characters
 */
#define StringEqual( a, b ) \
    ( ( a ) -> size == ( b ) -> size && \
    memcmp ( ( a ) -> addr, ( b ) -> addr, ( a ) -> size ) == 0 )

/* StringCompare
 *  compare strings for relative ordering
 *  not strictly lexical - generally orders by character code
 */
KLIB_EXTERN int CC StringCompare ( const String *a, const String *b );

/* StringCaseEqual
 *  compare strings for case-insensitive equality
 */
KLIB_EXTERN bool CC StringCaseEqual ( const String *a, const String *b );

/* StringCaseCompare
 *  compare strings for relative case-insensitive ordering
 */
KLIB_EXTERN int CC StringCaseCompare ( const String *a, const String *b );

/* StringOrder
 * StringOrderNoNullCheck
 *  compares strings as quickly as possible for
 *  deterministic ordering: first by length, then
 *  by binary ( byte-wise ) content.
 *
 *  performs more quickly than StringCompare for cases
 *  where only deterministic ordering is needed ( e.g. symbol table ).
 *
 *  the "NoNullCheck" version will crash if either a or b are NULL
 *  but avoids the overhead of checking when both are known to be good
 */
KLIB_EXTERN int64_t CC StringOrder ( const String *a, const String *b );
KLIB_EXTERN int64_t CC StringOrderNoNullCheck ( const String *a, const String *b );

/* StringMatch
 *  creates a substring of "a" in "match"
 *  for all of the sequential matching characters between "a" and "b"
 *  starting from character [ 0 ].
 *
 *  returns the number of characters that match.
 */
KLIB_EXTERN uint32_t CC StringMatch ( String *match,
    const String *a, const String *b );

/* StringMatchExtend
 *  extends a substring of "a" in "match"
 *  for all of the sequential matching characters between "a" and "b"
 *  starting from character [ match -> len ].
 *
 *  returns the number of matching characters that were extended.
 */
KLIB_EXTERN uint32_t CC StringMatchExtend ( String *match,
    const String *a, const String *b );

/* StringHash
 *  hash value for string
 */
#define StringHash( s ) \
    string_hash ( ( s ) -> addr, ( s ) -> size )

/* StringCopyUTF...
 *  creates a String from UTF16 or UTF32 UNICODE input
 *  wchar_t is one or the other, depending upon OS and compiler.
 */
KLIB_EXTERN rc_t CC StringCopyUTF16 ( const String **cpy,
    const uint16_t *text, size_t bytes );
KLIB_EXTERN rc_t CC StringCopyUTF32 ( const String **cpy,
    const uint32_t *text, size_t bytes );

/* StringCopyWChar_t
 *  wchar_t is alternately defined as 16 or 32 bits
 */
#define StringCopyWChar_t( cpy, text, bytes ) \
    ( ( sizeof ( wchar_t ) == sizeof ( uint16_t ) ) ? \
      StringCopyUTF16 ( cpy, ( const uint16_t* ) ( text ), bytes ) : \
      StringCopyUTF32 ( cpy, ( const uint32_t* ) ( text ), bytes ) )

/* StringWhack
 *  deallocates a string
 *  ignores strings not allocated by this library
 */
KLIB_EXTERN void CC StringWhack ( const String* self );


/* StringToInt
 *  simple string conversion functions
 *
 *  these functions are defined to consume the entire string.
 *  leading spaces are tolerated, repeated signs are accepted for signed conversion,
 *  decimal and hex encodings are accepted for unsigned conversion,
 *  decimal only for signed conversion.
 *
 *  "optional_rc" [ OUT, NULL OKAY ] - if non-null, user is interested
 *  in error conditions. if the parameter is present, the string must be
 *  completely consumed without overflow.
 *
 *  optional return values ( with { GetRCObject ( rc ), GetRCState ( rc ) }:
 *   0                            : no error
 *   { rcRange, rcExcessive }     : integer overflow
 *   { rcTransfer, rcIncomplete } : extra characters remain in string
 *   { rcData, rcInsufficient }   : no numeric text was found
 *
 *  return values - regardless of "optional_rc":
 *    val             : when no error
 *    val             : on incomplete transfer
 *    +/- max int64_t : when signed overflow occurs ( StringToI64 only )
 *    max uint64_t    : when unsigned overflow occurs ( StringToU64 only )
 *    0               : when no input text is found
 */
KLIB_EXTERN int64_t StringToI64 ( const String * self, rc_t * optional_rc );
KLIB_EXTERN uint64_t StringToU64 ( const String * self, rc_t * optional_rc );


/*--------------------------------------------------------------------------
 * raw text strings
 *  the internal representation of text strings is implementation
 *  dependent. it is assumed to be ASCII-7 or UTF-8, although
 *  this is determined by the implementation library of these functions.
 *
 * NB - ASCII implementations are no longer being provided
 *  all text handling is UTF-8 unless explictly stated otherwise
 */

/* string_size
 *  length of string in bytes
 */
KLIB_EXTERN size_t CC string_size ( const char *str );

/* string_len
 *  length of string in characters, when the size is known
 */
KLIB_EXTERN uint32_t CC string_len ( const char *str, size_t size );

/* string_measure
 *  measures length of string in both characters and bytes
 */
KLIB_EXTERN uint32_t CC string_measure ( const char *str, size_t *size );

/* string_copy
 *  copies whole character text into a buffer
 *  terminates with NUL byte if possible
 *  returns the number of bytes copied
 */
KLIB_EXTERN size_t CC string_copy ( char *dst, size_t dst_size,
    const char *src, size_t src_size );

/* string_copy_measure
 *  copies whole character text into a buffer
 *  terminates with NUL byte if possible
 *  returns the number of bytes copied
 */
KLIB_EXTERN size_t CC string_copy_measure ( char *dst, size_t dst_size, const char *src );

/* string_dup
 *  replaces the broken C library strndup
 *  creates a NUL-terminated malloc'd string
 */
KLIB_EXTERN char* CC string_dup ( const char *str, size_t size );

/* string_dup_measure
 *  replaces the broken C library strdup
 *  creates a NUL-terminated malloc'd string
 *  returns size of string unless "size" is NULL
 */
KLIB_EXTERN char* CC string_dup_measure ( const char *str, size_t *size );

/* tolower_copy
 *  copies whole character text in lower-case
 *  terminates with NUL byte if possible
 *  returns the number of bytes copied
 */
KLIB_EXTERN size_t CC tolower_copy ( char *dst, size_t dst_size,
    const char *src, size_t src_size );

/* toupper_copy
 *  copies whole character text in upper-case
 *  terminates with NUL byte if possible
 *  returns the number of bytes copied
 */
KLIB_EXTERN size_t CC toupper_copy ( char *dst, size_t dst_size,
    const char *src, size_t src_size );

/* string_cmp
 *  performs a safe strncmp
 *
 *  "max_chars" limits the extent of the comparison
 *  to not exceed supplied value, i.e. the number of
 *  characters actually compared will be the minimum
 *  of asize, bsize and max_chars.
 *
 *  if either string size ( or both ) < max_chars and
 *  all compared characters match, then the result will
 *  be a comparison of asize against bsize.
 */
KLIB_EXTERN int CC string_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars );

/* strcase_cmp
 *  like string_cmp except case insensitive
 */
KLIB_EXTERN int CC strcase_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars );

/* string_match
 *  returns the number of matching characters
 *
 *  "max_chars" limits the extent of the comparison
 *  to not exceed supplied value, i.e. the number of
 *  characters actually compared will be the minimum
 *  of asize, bsize and max_chars.
 *
 *  "msize" will be set to the size of the matched string
 *  if not NULL
 */
KLIB_EXTERN uint32_t CC string_match ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars, size_t *msize );

/* strcase_match
 *  like string_match except case insensitive
 */
KLIB_EXTERN uint32_t CC strcase_match ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars, size_t *msize );

/* string_chr
 *  performs a safe strchr
 *  "ch" is in UTF32
 */
KLIB_EXTERN char* CC string_chr ( const char *str, size_t size, uint32_t ch );

/* string_rchr
 *  performs a safe strrchr
 */
KLIB_EXTERN char* CC string_rchr ( const char *str, size_t size, uint32_t ch );

/* string_brk
 *  performs a safe strpbrk
 */
#if 0
KLIB_EXTERN char* CC string_brk ( const char *str, size_t size,
    const char *accept, size_t asize );
#endif

/* string_rbrk
 */
#if 0
KLIB_EXTERN char* CC string_rbrk ( const char *str, size_t size,
    const char *accept, size_t asize );
#endif

/* string_hash
 *  hashes a string
 */
KLIB_EXTERN uint32_t CC string_hash ( const char *str, size_t size );

/* string_idx
 *  seek an indexed character
 *
 *  the efficiency is based upon chosen internal
 *  string representation, which, when using single byte chars,
 *  is simple and efficient.
 *
 *  on the other hand, UTF-8 has a variable character width,
 *  requiring scanning of the entire string until the indexed
 *  character is found.
 */
KLIB_EXTERN char* CC string_idx ( const char *str, size_t size, uint32_t idx );


/* string_to_int
 *  simple string conversion functions
 *
 *  these functions are defined to consume the entire string.
 *  leading spaces are tolerated, repeated signs are accepted for signed conversion,
 *  decimal and hex encodings are accepted for unsigned conversion,
 *  decimal only for signed conversion.
 *
 *  "optional_rc" [ OUT, NULL OKAY ] - if non-null, user is interested
 *  in error conditions. if the parameter is present, the string must be
 *  completely consumed without overflow.
 *
 *  optional return values ( with { GetRCObject ( rc ), GetRCState ( rc ) }:
 *   0                            : no error
 *   { rcRange, rcExcessive }     : integer overflow
 *   { rcTransfer, rcIncomplete } : extra characters remain in string
 *   { rcData, rcInsufficient }   : no numeric text was found
 *
 *  return values - regardless of "optional_rc":
 *    val             : when no error
 *    val             : on incomplete transfer
 *    +/- max int64_t : when signed overflow occurs ( StringToI64 only )
 *    max uint64_t    : when unsigned overflow occurs ( StringToU64 only )
 *    0               : when no input text is found
 */
KLIB_EXTERN int64_t string_to_I64 ( const char * str, size_t size, rc_t * optional_rc );
KLIB_EXTERN uint64_t string_to_U64 ( const char * str, size_t size, rc_t * optional_rc );


/*--------------------------------------------------------------------------
 * conversion between UTF-32 and UTF-8 UNICODE
 */

/* utf8_utf32
 *  converts UTF-8 text to a single UTF-32 character
 *  returns the number of UTF8 bytes consumed, such that:
 *    return > 0 means success
 *    return == 0 means insufficient input
 *    return < 0 means bad input
 */
KLIB_EXTERN int CC utf8_utf32 ( uint32_t *ch, const char *begin, const char *end );

/* utf32_utf8
 *  converts a single UTF-32 character to UTF-8 text
 *  returns the number of UTF8 bytes generated, such that:
 *    return > 0 means success
 *    return == 0 means insufficient output
 *    return < 0 means bad character
 */
KLIB_EXTERN int CC utf32_utf8 ( char *begin, char *end, uint32_t ch );


/*--------------------------------------------------------------------------
 * support for 16 and 32-bit UTF formats
 */

/* utf16_string_size/len/measure
 *  measures UTF-16 strings
 */
KLIB_EXTERN size_t CC utf16_string_size ( const uint16_t *str );
KLIB_EXTERN uint32_t CC utf16_string_len ( const uint16_t *str, size_t size );
KLIB_EXTERN uint32_t CC utf16_string_measure ( const uint16_t *str, size_t *size );

/* utf32_string_size/len/measure
 */
KLIB_EXTERN size_t CC utf32_string_size ( const uint32_t *str );
KLIB_EXTERN uint32_t CC utf32_string_len ( const uint32_t *str, size_t size );
KLIB_EXTERN uint32_t CC utf32_string_measure ( const uint32_t *str, size_t *size );

/* wchar_string_size/len/measure
 *  measures wchar_t strings
 */
KLIB_EXTERN size_t CC wchar_string_size ( const wchar_t *str );
KLIB_EXTERN uint32_t CC wchar_string_len ( const wchar_t *str, size_t size );
KLIB_EXTERN uint32_t CC wchar_string_measure ( const wchar_t *str, size_t *size );

/* conversion from UTF-16 to internal standard */
KLIB_EXTERN uint32_t CC utf16_cvt_string_len ( const uint16_t *src,
    size_t src_size, size_t *dst_size );
KLIB_EXTERN uint32_t CC utf16_cvt_string_measure ( const uint16_t *src,
    size_t *src_size, size_t *dst_size );
KLIB_EXTERN size_t CC utf16_cvt_string_copy ( char *dst, size_t dst_size,
    const uint16_t *src, size_t src_size );

/* conversion from UTF-32 to internal standard */
KLIB_EXTERN uint32_t CC utf32_cvt_string_len ( const uint32_t *src,
    size_t src_size, size_t *dst_size );
KLIB_EXTERN uint32_t CC utf32_cvt_string_measure ( const uint32_t *src,
    size_t *src_size, size_t *dst_size );
KLIB_EXTERN size_t CC utf32_cvt_string_copy ( char *dst, size_t dst_size,
    const uint32_t *src, size_t src_size );

/* conversion from wchar_t to internal standard */
KLIB_EXTERN uint32_t CC wchar_cvt_string_len ( const wchar_t *src,
    size_t src_size, size_t *dst_size );
KLIB_EXTERN uint32_t CC wchar_cvt_string_measure ( const wchar_t *src,
    size_t *src_size, size_t *dst_size );
KLIB_EXTERN size_t CC wchar_cvt_string_copy ( char *dst, size_t dst_size,
    const wchar_t *src, size_t src_size );

/* conversion to wchar_t from internal standard */
KLIB_EXTERN size_t CC string_cvt_wchar_copy ( wchar_t *dst, size_t dst_size,
    const char *src, size_t src_size );

/*--------------------------------------------------------------------------
 * support for ISO-8859-x 8-bit character sets
 */

/* iso8859_utf32
 *  converts 8-bit text to a single UTF-32 character
 *  returns the number of 8-bit bytes consumed, such that:
 *    return > 0 means success
 *    return == 0 means insufficient input
 *    return < 0 means bad input
 */
KLIB_EXTERN int CC iso8859_utf32 ( const uint32_t map [ 128 ],
    uint32_t *ch, const char *begin, const char *end );

/* iso8859_string_size/len/measure
 *  measures UTF-16 strings
 */
KLIB_EXTERN size_t CC iso8859_string_size ( const uint32_t map [ 128 ],
    const char *str );
KLIB_EXTERN uint32_t CC iso8859_string_len ( const uint32_t map [ 128 ],
    const char *str, size_t size );
KLIB_EXTERN uint32_t CC iso8859_string_measure ( const uint32_t map [ 128 ],
    const char *str, size_t *size );

/* conversion from ISO-8859-x to internal standard */
KLIB_EXTERN uint32_t CC iso8859_cvt_string_len ( const uint32_t map [ 128 ],
    const char *src, size_t src_size, size_t *dst_size );
KLIB_EXTERN uint32_t CC iso8859_cvt_string_measure ( const uint32_t map [ 128 ],
    const char *src, size_t *src_size, size_t *dst_size );
KLIB_EXTERN size_t CC iso8859_cvt_string_copy ( const uint32_t map [ 128 ],
    char *dst, size_t dst_size, const char *src, size_t src_size );

/* some externally defined character maps */
KLIB_EXTERN_DATA const uint32_t iso8859_1 [ 128 ];
KLIB_EXTERN_DATA const uint32_t cp1252 [ 128 ];


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_text_ */
