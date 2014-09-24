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

#ifndef _h_klib_token_
#define _h_klib_token_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KSymbol;


/*--------------------------------------------------------------------------
 * KTokenText
 *  named text
 *
 *  it's likely that the name will need to become a refcounted object...
 */
typedef struct KTokenText KTokenText;
struct KTokenText
{
    String str;
    String path;

    rc_t ( CC * read ) ( void *self, KTokenText *tt, size_t save );
    void *data;
};


/* Init
 *  initialize structure
 */
#define KTokenTextInit( tt, text, fpath ) \
    ( void ) ( ( tt ) -> str = * ( text ), \
               ( tt ) -> path = * ( fpath ), \
               ( tt ) -> read = NULL, \
               ( tt ) -> data = NULL )
#ifndef KTokenTextInit
KLIB_EXTERN void CC KTokenTextInit ( KTokenText *tt, const String *text, const String *path );
#endif

#define KTokenTextInitCString( tt, text, fpath ) \
    ( void ) ( StringInitCString ( & ( tt ) -> str, text ), \
               StringInitCString ( & ( tt ) -> path, fpath ), \
               ( tt ) -> read = NULL, \
               ( tt ) -> data = NULL )
#ifndef KTokenTextInitCString
KLIB_EXTERN void CC KTokenTextInitCString ( KTokenText *tt, const char *text, const char *path );
#endif

/*--------------------------------------------------------------------------
 * KTokenID
 *  pre-defined ids for text tokens
 *  not every tokenizer will return all tokens
 */
enum KTokenID
{
    eEndOfInput, eUnrecognized, eUntermComment,

    /* space separators */
    eWhiteSpace, eEndOfLine,

    /* numeric */
    eDecimal, eHex, eOctal, eFloat, eExpFloat, eMajMinRel,

    /* string */
    eUntermString, eString, eUntermEscapedString, eEscapedString,

    /* names */
    eIdent, eName,

    /* punctuation */
    ePeriod, eComma, eColon, eSemiColon, eQuestion, eTilde, eExclam,
    eAtSign, eHash, eDollar, ePercent, eCaret, eAmpersand, eAsterisk,
    ePlus, eMinus, eAssign, eFwdSlash, eBackSlash, ePipe,

    /* paired punctuation */
    eLeftParen, eRightParen,
    eLeftCurly, eRightCurly,
    eLeftAngle, eRightAngle,
    eLeftSquare, eRightSquare,

    /* compound tokens */
    eDblPeriod, eDblColon, eEllipsis, eLogAnd, eLogOr,
    eEqual, eNotEqual, eColonAssign, ePlusAssign, eOverArrow,

    eDblLeftAngle, eDblRightAngle,
    eDblLeftSquare, eDblRightSquare,

    /* first free id */
    eNumTokenIDs,

    /* namespace type - needed by KSymTable */
    eNamespace = eNumTokenIDs,
    eNumSymtabIDs
};


/*--------------------------------------------------------------------------
 * KToken
 *  a string with an id and source information
 */
typedef struct KToken KToken;
struct KToken
{
    const KTokenText *txt;
    struct KSymbol *sym;
    String str;
    uint32_t id;
    uint32_t lineno;
};


/* conversion operators
 *  since the constants have a type id, the entire token is used
 */
KLIB_EXTERN rc_t CC KTokenToI32 ( const KToken *self, int32_t *i );
KLIB_EXTERN rc_t CC KTokenToU32 ( const KToken *self, uint32_t *i );
KLIB_EXTERN rc_t CC KTokenToI64 ( const KToken *self, int64_t *i );
KLIB_EXTERN rc_t CC KTokenToU64 ( const KToken *self, uint64_t *i );
KLIB_EXTERN rc_t CC KTokenToF64 ( const KToken *self, double *d );
KLIB_EXTERN rc_t CC KTokenToVersion ( const KToken *self, uint32_t *v );
KLIB_EXTERN rc_t CC KTokenToString ( const KToken *self, char *buffer, size_t bsize, size_t *size );
KLIB_EXTERN rc_t CC KTokenToWideString ( const KToken *self, uint32_t *buffer, uint32_t blen, uint32_t *len );


/*--------------------------------------------------------------------------
 * KTokenSource
 *  a modifiable source of tokens
 */
typedef struct KTokenSource KTokenSource;
struct KTokenSource
{
    const KTokenText *txt;
    String str;
    uint32_t lineno;
};

/* Init
 */
#define KTokenSourceInit( self, tt ) \
    ( void ) ( ( self ) -> str = ( tt ) -> str, \
               ( self ) -> txt = ( tt ), \
               ( self ) -> lineno = 1 )
#ifndef KTokenSourceInit
KLIB_EXTERN void CC KTokenSourceInit ( KTokenSource *self, const KTokenText *txt );
#endif

/* Return
 *  returns token to source
 */
KLIB_EXTERN void CC KTokenSourceReturn ( KTokenSource *self, const KToken *t );


/* Consume
 *  consumes all remaining data
 */
KLIB_EXTERN void CC KTokenSourceConsume ( KTokenSource *self );


/* Avail
 *  the number of characters available
 */
#define KTokenSourceAvail( self ) \
    ( ( ( const KTokenSource* ) ( self ) ) -> str . len )
#ifndef KTokenSourceAvail
KLIB_EXTERN uint32_t CC KTokenSourceAvail ( const KTokenSource *self );
#endif

/*--------------------------------------------------------------------------
 * KTokenizer
 *  an encapsulation of the tokenizer code
 */
typedef struct KTokenizer KTokenizer;


/* Next
 *  scan for next token
 *
 *  "src" [ IN ] - source of token text
 *
 *  "t" [ OUT ] - scanned token
 *
 *  returns a pointer to "t" for convenience
 */
KLIB_EXTERN KToken* CC KTokenizerNext ( const KTokenizer *self, KTokenSource *src, KToken *t );


/* kDefaultTokenizer
 *  a constant KTokenizer* to obtain default behavior
 */
#define kDefaultTokenizer ( ( const KTokenizer* ) 0 )


/* kLineTokenizer
 *  behaves like default tokenizer
 *  except that eEndOfLine tokens are returned
 */
#define kLineTokenizer ( ( const KTokenizer* ) 1 )


/* kPOSIXPathTokenizer
 *  tokenizes a POSIX path string
 */
#define kPOSIXPathTokenizer ( ( const KTokenizer* ) 2 )

/* kKfgTokenizer
 * tokenizes a KFG config file 
 * with its name value pairs and comments
 */
#define kKfgTokenizer  ( ( const KTokenizer* ) 3 )

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_token_ */
