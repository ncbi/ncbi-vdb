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
#include "trie-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <errno.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * TNode
 *  a node within text tree
 */

/* TNodeMake
 *  creates a TNode of variable size
 *
 *  "n" will contain the returned node
 *
 *  "size" must be at least the size of a TNode
 */
LIB_EXPORT rc_t CC TNodeMake ( TNode **np, size_t size )
{
    TNode *n;

    if ( np == NULL )
        return RC ( rcCont, rcNode, rcAllocating, rcParam, rcNull );

    if ( size < sizeof * n )
        return RC ( rcCont, rcNode, rcAllocating, rcParam, rcInvalid );

    n = malloc ( size );
    if ( n == NULL )
        return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );

    * np = n;
    return 0;
}

/* TNodeWhack
 *  performs final whacking of node
 */
LIB_EXPORT void CC TNodeWhack ( TNode *n )
{
    free ( n );
}

/* TNodeCmp
 */
static
int CC TNodeCmp ( const void *item, const void *n )
{
#define a ( ( const String* ) item )
#define b ( ( const TNode* ) n )

    return StringCompare ( a, & b -> key );

#undef a
#undef b
}

/* TNodeSort
 */
static
int CC TNodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const TNode* ) item )
#define b ( ( const TNode* ) n )

    return StringCompare ( & a -> key, & b -> key );

#undef a
#undef b
}

/*--------------------------------------------------------------------------
 * TTrans
 *  a text transition node
 *
 *  transitions to child nodes occur only if "tcnt" != 0 ( "child" != NULL )
 *  and are taken on the query string's character [ key . len ]
 */

/* TTransMake
 */
static
rc_t CC TTransMake ( TTrans **tp, uint32_t depth )
{
    TTrans *trans = malloc ( sizeof * trans );
    if ( trans == NULL )
        return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );

    trans -> child = NULL;
    trans -> tcnt = trans -> vcnt = 0;
    BSTreeInit ( & trans -> vals );
    trans -> depth = depth;

    * tp = trans;
    return 0;
}

/* TTransForEach
 */
void TTransForEach ( const TTrans *trans, uint32_t width,
    void ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data )
{
    TTransBacktrace bt;
    bt . par = NULL;
    bt . trans = trans;
    bt . idx = 0;

    TTransForEachBT ( & bt, width, f, data );
}

void TTransForEachBT ( const TTransBacktrace *bt, uint32_t width,
    void ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data )
{
    const TTrans *trans = bt -> trans;

    if ( f != NULL )
        ( * f ) ( bt, width, data );

    assert ( trans != NULL );
    if ( trans -> child != NULL )
    {
        uint32_t i;
        TTransBacktrace child;
        for ( child . par = bt, i = 0; i < width; ++ i )
        {
            if ( ( child . trans = trans -> child [ i ] ) != NULL )
            {
                child . idx = ( uint32_t ) i;
                TTransForEachBT ( & child, width, f, data );
            }
        }
    }
}

/* TTransDoUntil
 */
bool TTransDoUntil ( const TTrans *trans, uint32_t width,
    bool ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data )
{
    TTransBacktrace bt;
    bt . par = NULL;
    bt . trans = trans;
    bt . idx = 0;

    return TTransDoUntilBT ( & bt, width, f, data );
}

bool TTransDoUntilBT ( const TTransBacktrace *bt, uint32_t width,
    bool ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data )
{
    const TTrans *trans = bt -> trans;

    if ( f != NULL )
    {
        if ( ( * f ) ( bt, width, data ) )
            return true;
    }

    assert ( trans != NULL );
    if ( trans -> child != NULL )
    {
        uint32_t i;
        TTransBacktrace child;
        for ( child . par = bt, i = 0; i < width; ++ i )
        {
            if ( ( child . trans = trans -> child [ i ] ) != NULL )
            {
                child . idx = ( uint32_t ) i;
                if ( TTransDoUntilBT ( & child, width, f, data ) )
                    return true;
            }
        }
    }

    return false;
}

/* TTransWhack
 */
static
void TTransWhack ( TTrans *trans, uint32_t width,
    void ( CC * whack ) ( TNode *n, void *data ), void *data )
{
    if ( trans != NULL )
    {
        if ( trans -> child != NULL )
        {
            uint32_t i;
            for ( i = 0; i < width; ++ i )
            {
                TTrans *child = trans -> child [ i ];
                if ( child != NULL )
                    TTransWhack ( child, width, whack, data );
            }

            free ( trans -> child );
        }

        BSTreeWhack ( & trans -> vals,
            ( void ( CC * ) ( BSTNode*, void* ) ) whack, data );

        free ( trans );
    }
}


/*--------------------------------------------------------------------------
 * Trie
 *  a tree of text nodes
 */

/* TrieInit
 *  initialize a text-tree structure
 *
 *  "accept" is a character-set string where each character
 *  represents itself. the only exception is the '-' character,
 *  which - when sandwiched between two other characters - is
 *  interpreted en tot as a range expression. an example would be
 *  "-0-9" is identical to "-0123456789"
 *
 *  "limit" sets the number of values to be accumulated in any
 *  container. if this limit would be exceeded by any insertion,
 *  the node is expanded and its values are re-distributed among
 *  its children.
 *
 *  if "cs_expand" is 1, then any valid characters observed
 *  during insertions are automatically added to the accept
 *  character set. doing so will ensure that there are no
 *  ambiguous transitions.
 *
 *  returns status codes:
 *    0      => success
 *    EINVAL => an invalid parameter
 *    ENOMEM => failed to allocate internal structures
 *    EDOM   => either the limit was too high or the accept string too wide
 */
LIB_EXPORT rc_t CC TrieInit ( Trie *tt, const char *accept,
    uint32_t limit, bool cs_expand )
{
    int ch_len;
    const char *src, *end;

    uint32_t ch, ch2;
    uint32_t first, last, width;

    uint16_t *map;
    uint32_t *rmap;

    if ( tt == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcSelf, rcNull );

    if ( accept == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcString, rcNull );

    if ( accept [ 0 ] == 0 )
        return RC ( rcCont, rcTrie, rcConstructing, rcString, rcEmpty );

    if ( ( limit >> 16 ) != 0 )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcExcessive );

    memset ( tt, 0, sizeof * tt );
    tt -> limit = ( uint16_t ) limit;
    tt -> cs_expand = cs_expand;

    /* determine width of accept string
       N.B. we don't detect overlaps at this point */
    src = accept;
    end = src + strlen ( accept );

    for ( last = 0, first = ~ 0; src < end; )
    {
        /* get a character */
        ch_len = utf8_utf32 ( & ch, src, end );
        if ( ch_len <= 0 )
            return RC ( rcCont, rcTrie, rcConstructing, rcChar, rcInvalid );
        src += ch_len;

        /* this character represents itself */
        if ( ch < first )
            first = ch;
        if ( ch > last )
            last = ch;

        /* check for a range expression */
        if ( src < end && src [ 0 ] == '-' )
        {
            ch_len = utf8_utf32 ( & ch2, ++ src, end );
            if ( ch_len <= 0 )
                return RC ( rcCont, rcTrie, rcConstructing, rcChar, rcInvalid );
            src += ch_len;

            /* this character represents
               hopefully a reasonable range */
            if ( ch2 < ch )
                return RC ( rcCont, rcTrie, rcConstructing, rcChar, rcInvalid );
            if ( ch2 > last )
                last = ch2;
        }
    }

    /* must have had something */
    if ( last < first )
        return RC ( rcCont, rcTrie, rcConstructing, rcString, rcInvalid );

    tt -> first_char = first;
    tt -> last_char = last;

    /* create a character map of pure unknown index */
    map = calloc ( last - first + 1, sizeof map [ 0 ] );
    if ( map == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcMemory, rcExhausted );
    tt -> map = map;

    /* mark the character map */
    for ( src = accept; src < end; )
    {
        /* get a character */
        ch_len = utf8_utf32 ( & ch, src, end );
        src += ch_len;

        /* mark character */
#if REJECT_ACCEPT_OVERLAP
        if ( map [ ch - first ] != 0 )
        {
            free ( map );
            return RC ( rcCont, rcTrie, rcConstructing, rcChar, rcInvalid );
        }
#endif
        map [ ch - first ] = 1;

        /* check for a range expression */
        if ( src < end && src [ 0 ] == '-' )
        {
            ch_len = utf8_utf32 ( & ch2, ++ src, end );
            src += ch_len;

            /* mark range */
            for ( ++ ch; ch <= ch2; ++ ch )
            {
#if REJECT_ACCEPT_OVERLAP
                if ( map [ ch - first ] != 0 )
                {
                    free ( map );
                    return RC ( rcCont, rcTrie, rcConstructing, rcChar, rcInvalid );
                }
#endif
                map [ ch - first ] = 1;
            }
        }
    }

    /* fill in the character map and determine width */
    for ( width = 0, ch = first; ch <= last; ++ ch )
    {
        if ( map [ ch - first ] != 0 )
            map [ ch - first ] = ++ width;
    }

    /* enforce maximum width */
    assert ( width > 0 );
    if ( ( ++ width >> 16 ) != 0 )
    {
        free ( map );
        return RC ( rcCont, rcTrie, rcConstructing, rcRange, rcExcessive );
    }

    /* record trans width accounting for unmapped slot */
    tt -> width = ( uint16_t ) width;

    /* now create a reverse map to produce characters
       from an index */
#if _DEBUGGING
    rmap = calloc ( width, sizeof rmap [ 0 ] );
#else
    rmap = malloc ( width * sizeof rmap [ 0 ] );
#endif
    if ( rmap == NULL )
    {
        free ( map );
        return RC ( rcCont, rcTrie, rcConstructing, rcMemory, rcExhausted );
    }
    tt -> rmap = rmap;

    for ( rmap [ 0 ] = 0, ch = first; ch <= last; ++ ch )
    {
        uint32_t idx = map [ ch - first ];
        if ( idx != 0 )
            rmap [ idx ] = ch;
    }

    return 0;
}

/* TrieMapChar
 *  maps a character to an index
 */
uint32_t TrieMapChar ( const Trie *tt, uint32_t ch )
{
    if ( ch >= tt -> first_char && ch <= tt -> last_char )
        return tt -> map [ ch - tt -> first_char ];
    return 0;
}

/* TrieNextIdx
 *  pops a character from key string
 *  maps it to an index
 */
rc_t TrieNextIdx ( const Trie *tt, String *key, uint32_t *idx )
{
    uint32_t ch;
    rc_t rc = StringPopHead ( key, & ch );
    if ( rc == 0 )
        * idx = TrieMapChar ( tt, ch );
    return rc;
}

/* TrieAutoExpand
 *  incorporates a new character into the accept charset
 *
 *    1. extends all existing "child" arrays
 *    2. reallocs "map" if necessary, invalidating new space
 *    3. reallocs "rmap"
 *    4. installs character <-> index into map
 *    5. increments width
 *
 *  the order of execution is designed to be benign if
 *  failures occur, due only to memory exhaustion.
 */
typedef struct AutoExpandData AutoExpandData;
struct AutoExpandData
{
    rc_t rc;
};

static
bool CC TTransAutoExpand ( const TTransBacktrace *bt, uint32_t width, void *data )
{
    TTrans **child, *trans = ( TTrans* ) bt -> trans;

    assert ( trans != NULL );
    if ( trans -> child == NULL )
        return false;

    child = realloc ( trans -> child, ( width + 1 ) * sizeof * child );
    if ( child == NULL )
    {
        ( ( AutoExpandData* ) data ) -> rc =
            RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
        return true;
    }

    child [ width ] = NULL;
    trans -> child = child;
    return false;
}

static
rc_t CC TrieAutoExpand ( Trie *tt, uint32_t ch )
{
    AutoExpandData pb;
    pb . rc = 0;

    /* extend node child arrays */
    TTransDoUntil ( tt -> root, tt -> width, TTransAutoExpand, & pb );
    if ( pb . rc == 0 )
    {
        uint16_t *map = ( uint16_t* ) tt -> map;
        uint32_t *rmap = ( uint32_t* ) tt -> rmap;

        /* realloc charset to index map if necessary */
        if ( ch < tt -> first_char )
        {
            map = malloc ( ( tt -> last_char - ch + 1 ) * sizeof * map );
            if ( map == NULL )
                return RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
            memset ( map, 0, ( tt -> first_char - ch ) * sizeof * map );
            memcpy ( & map [ tt -> first_char - ch ], tt -> map,
                ( tt -> last_char - tt -> first_char + 1 ) * sizeof * map );
            free ( ( void* ) tt -> map );
            tt -> first_char = ch;
        }
        else if ( ch > tt -> last_char )
        {
            map = realloc ( map, ( ch - tt -> first_char + 1 ) * sizeof * map );
            if ( map == NULL )
                return RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
            memset ( & map [ tt -> last_char - tt -> first_char + 1 ], 0, ( ch - tt -> last_char ) * sizeof * map );
            tt -> last_char = ch;
        }

        /* record any change */
        tt -> map = map;

        /* extend index to charset map */
        rmap = realloc ( rmap, ( tt -> width + 1 ) * sizeof * rmap );
        if ( rmap == NULL )
            return RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );

        tt -> rmap = rmap;

        /* it is done */
        rmap [ tt -> width ] = ch;
        map [ ch - tt -> first_char ] = tt -> width ++;
    }

    return pb . rc;
}

/* TrieValidateRemainder
 *  ensures that all characters remaining in key
 *  are valid and, in the case of expanding the char set,
 *  are in the character set.
 */
static
rc_t CC TrieValidateRemainder ( Trie *tt, const String *key )
{
    rc_t rc;
    String rem = * key;

    while ( 1 )
    {
        uint32_t ch;
        rc = StringPopHead ( & rem, & ch );
        if ( GetRCState ( rc ) == rcEmpty )
            return 0;

        if ( tt -> cs_expand )
        {
            uint32_t idx = TrieMapChar ( tt, ch );
            if ( idx == 0 )
            {
                /* get the new width */
                uint16_t width = tt -> width + 1;
                
                /* get a new value for idx */
                idx = tt -> width;
                
                /* make sure the expansion would not exceed
                   the 16 bit limit on array width */
                if ( width == 0 )
                {
                    rc = RC ( rcCont, rcTrie, rcInserting, rcRange, rcExcessive );
                    break;
                }
                
                /* incorporate the character */
                rc = TrieAutoExpand ( tt, ch );
                if ( rc != 0 )
                    break;
            }
        }
    }

    return rc;
}

/* TrieExplodeTrans
 *  a TTrans has exceeded its limit of TNode values
 */
typedef struct ExplodeTransData ExplodeTransData;
struct ExplodeTransData
{
    const Trie *tt;
    TTrans *trans;
    size_t tsize;
    uint32_t tcnt;
    uint32_t cnt;
    rc_t rc;
    bool over;
};

static
bool CC CreateKids ( BSTNode *n, void *data )
{
    ExplodeTransData *pb = data;
    TNode *item = ( TNode* ) n;
    const TTrans *trans = pb -> trans;

    if ( item -> key . len > trans -> depth )
    {
        uint32_t idx;
        String key = item -> key;
        const Trie *tt = pb -> tt;

        /* perform a sub-string based upon
           knowledge that the entry key matches
           the first "trans->depth" characters */
        key . addr += pb -> tsize;
        key . len -= trans -> depth;
        key . size -= pb -> tsize;

        /* get the next transition index */
        pb -> rc = TrieNextIdx ( tt, & key, & idx );
        if ( pb -> rc != 0 )
            return true;

        /* test the transition */
        assert ( trans -> child != NULL );
        if ( trans -> child [ idx ] == NULL )
        {
            /* create the transition */
            pb -> rc = TTransMake ( & trans -> child [ idx ], trans -> depth + 1 );
            if ( pb -> rc != 0 )
                return false;

            ++ pb -> tcnt;
        }

#if _DEBUGGING
        /* accounting for non-terminal key */
        ++ pb -> cnt;
#else
        /* early exit */
        if ( ++ pb -> cnt == trans -> tcnt )
            return true;
#endif
    }

    return false;
}

static
bool CC WhackKids ( BSTNode *n, void *data )
{
    ExplodeTransData *pb = data;
    TNode *item = ( TNode* ) n;
    const TTrans *trans = pb -> trans;

    if ( item -> key . len > trans -> depth )
    {
        TTrans *child;
        uint32_t idx;
        String key = item -> key;
        const Trie *tt = pb -> tt;

        /* perform a sub-string based upon
           knowledge that the entry key matches
           the first "trans->depth" characters */
        key . addr += pb -> tsize;
        key . len -= trans -> depth;
        key . size -= pb -> tsize;

        /* get the next transition character */
        pb -> rc = TrieNextIdx ( tt, & key, & idx );
        if ( pb -> rc != 0 )
            return false;

        /* test the transition - due to early
           exit on error from child creation,
           as well as from this function, we can
           assert that this transition is not NULL */
        assert ( trans -> child != NULL );
        assert ( trans -> child [ idx ] != NULL );
        child = trans -> child [ idx ];

        /* if the child is totally empty, it was just
           created and should be doused */
        if ( child -> tcnt + child -> vcnt == 0 )
        {
            free ( child );
            trans -> child [ idx ] = NULL;

#if _DEBUGGING
            /* not strictly required, because no changes
               have been committed yet */
            -- pb -> tcnt;
#else
            /* early exit when there could be no further work */
            if ( -- pb -> tcnt == 0 )
                return true;
#endif
        }

        /* mandatory early exit when we detect limit */
        if ( -- pb -> cnt == 0 )
            return true;
    }

    return false;
}

static
bool CC MoveKids ( BSTNode *n, void *data )
{
    ExplodeTransData *pb = data;
    TNode *item = ( TNode* ) n;
    TTrans *trans = pb -> trans;

    if ( item -> key . len > trans -> depth )
    {
        TTrans *child;
        uint32_t idx;
        String key = item -> key;
        const Trie *tt = pb -> tt;

        /* perform a sub-string based upon
           knowledge that the entry key matches
           the first "trans->depth" characters */
        key . addr += pb -> tsize;
        key . len -= trans -> depth;
        key . size -= pb -> tsize;

        /* get the next transition character */
        pb -> rc = TrieNextIdx ( tt, & key, & idx );
        assert ( pb -> rc == 0 );

        /* test the transition */
        assert ( trans -> child != NULL );
        assert ( trans -> child [ idx ] != NULL );
        child = trans -> child [ idx ];

        /* pop this guy from its current location */
        BSTreeUnlink ( & trans -> vals, & item -> n );

        /* insert into its new location */
        pb -> rc = BSTreeInsert ( & child -> vals, & item -> n, TNodeSort );
        assert ( pb -> rc == 0 );

        if ( item -> key . len == child -> depth )
            ++ child -> vcnt;
        else
            ++ child -> tcnt;

        /* detect an over-limit child */
        assert ( child -> child == NULL );
        if ( child -> tcnt != 0 && ( child -> tcnt + child -> vcnt > tt -> limit ) )
            pb -> over = true;

#if _DEBUGGING
        ++ pb -> cnt;
#else
        /* early exit */
        if ( ++ pb -> cnt == trans -> tcnt )
            return true;
#endif
    }

    return false;
}

static
rc_t CC TrieExplodeTrans ( const Trie *tt, TTrans *trans, size_t tsize )
{
    rc_t rc;
    ExplodeTransData pb;

    /* create a child array */
    assert ( trans -> child == NULL );
    trans -> child = calloc ( tt -> width, sizeof trans -> child [ 0 ] );
    if ( trans -> child == NULL )
        return RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );

    /* loop data structure */
    pb . tt = tt;
    pb . trans = trans;
    pb . tsize = tsize;
    pb . tcnt = 0;
    pb . cnt = 0;
    pb . rc = 0;

    /* create new child nodes */
    BSTreeDoUntil ( & trans -> vals, 0, CreateKids, & pb );
    assert ( pb . tcnt <= tt -> width );
    rc = pb . rc;

    /* roll-back changes on error */
    if ( rc != 0 && pb . cnt != trans -> tcnt )
    {
        if ( pb . tcnt != 0 )
        {
            BSTreeDoUntil ( & trans -> vals, 0, WhackKids, & pb );
            assert ( pb . tcnt == 0 );
        }

        free ( trans -> child );
        trans -> child = NULL;

        return rc;
    }

    /* commit changes */
    pb . cnt = 0;
    pb . over = false;

    BSTreeDoUntil ( & trans -> vals, 0, MoveKids, & pb );
    assert ( pb . rc == 0 );
    assert ( pb . cnt == trans -> tcnt );

    assert ( pb . tcnt <= tt -> width );
    trans -> tcnt = pb . tcnt;

    /* handle over-limit kids */
    if ( pb . over )
    {
        uint32_t i;
        for ( i = 0; i < tt -> width; ++ i )
        {
            TTrans *child = trans -> child [ i ];
            if ( trans -> child [ i ] != NULL )
            {
                assert ( child -> child == NULL );
                if ( child -> tcnt != 0 && ( child -> tcnt + child -> vcnt > tt -> limit ) )
                {
                    /* re-calculate transition key string size */
                    uint32_t ch;
                    const TNode *item = ( const TNode* ) child -> vals . root;
                    String key = item -> key;

                    key . addr += tsize;
                    key . len -= trans -> tcnt;
                    key . size -= tsize;

                    rc = StringPopHead ( & key, & ch );
                    assert ( rc == 0 );

                    rc = TrieExplodeTrans ( tt, child, item -> key . size - key . size );
                    if ( rc != 0 )
                        break;
                }
            }
        }
    }

    return rc;
}

/* TrieInsertEngine
 *  inserts an item into tree
 *
 *  "item" is a TNode with an embedded key string.
 *
 *  "exist" is an optional return parameter for prior nodes
 *  and indicates whether to insert unique or not
 *
 *  returns status codes:
 *    0      => success
 *    EINVAL => an invalid parameter
 *    ENOMEM => failed to allocate internal structures
 */
static
rc_t CC TrieInsertEngine ( Trie *tt, TNode *item, TNode **exist )
{
    rc_t rc;
    String key;
    TTrans *trans;

    /* quick parameter check */
    if ( tt == NULL )
        return RC ( rcCont, rcTrie, rcInserting, rcSelf, rcNull );
    if ( item == NULL )
        return RC ( rcCont, rcTrie, rcInserting, rcParam, rcNull );
    if ( item -> key . len == 0 )
        return RC ( rcCont, rcTrie, rcInserting, rcString, rcEmpty );

    /* starting key */
    key = item -> key;

    /* get the root TTrans */
    if ( tt -> root == NULL )
    {
        rc = TTransMake ( & tt -> root, 0 );
        if ( rc != 0 )
            return rc;
    }
    trans = tt -> root;

    while ( 1 )
    {
        /* if there is a child array and the key is non-terminal */
        if ( trans -> child != NULL && trans -> depth < item -> key . len )
        {
            /* get transition character and index */
            uint32_t ch, idx;
            rc = StringPopHead ( & key, & ch );
            if ( rc != 0 )
                break;
            idx = TrieMapChar ( tt, ch );

            /* check for auto-expand on unmapped character */
            if ( idx == 0 && tt -> cs_expand )
            {
                /* get the new width */
                uint16_t width = tt -> width + 1;

                /* get a new value for idx */
                idx = tt -> width;

                /* make sure the expansion would not exceed
                   the 16 bit limit on array width */
                if ( width == 0 )
                {
                    rc = RC ( rcCont, rcTrie, rcInserting, rcRange, rcExcessive );
                    break;
                }
                rc = TrieAutoExpand ( tt, ch );
                if ( rc != 0 )
                    break;
            }

            /* take transition if there */
            if ( trans -> child [ idx ] != NULL )
            {
                trans = trans -> child [ idx ];
                continue;
            }

            /* create new transition */
            rc = TTransMake ( & trans -> child [ idx ], trans -> depth + 1 );
            if ( rc != 0 )
                break;

            /* account for and take the new transition */
            ++ trans -> tcnt;
            trans = trans -> child [ idx ];
        }

        /* item should never be shorter than node */
        assert ( trans -> depth <= item -> key . len );
        if ( trans -> depth < item -> key . len )
        {
            rc = TrieValidateRemainder ( tt, & key );
            if ( rc != 0 )
                break;
        }

        /* going to install into value tree */
        if ( exist == NULL )
            rc = BSTreeInsert ( & trans -> vals, & item -> n, TNodeSort );
        else
        {
            rc = BSTreeInsertUnique ( & trans -> vals,
                & item -> n, ( BSTNode** ) exist, TNodeSort );
        }
        if ( rc == 0 )
        {
            /* account for type of insert */
            if ( trans -> depth == item -> key . len )
                ++ trans -> vcnt;
            else
                ++ trans -> tcnt;

            /* now test for overflow */
            if ( trans -> child == NULL && trans -> tcnt != 0 )
            {
                if ( trans -> vcnt + trans -> tcnt > tt -> limit )
                {
                    /* it's time to convert to a full transition node */
                    TrieExplodeTrans ( tt, trans, item -> key . size - key . size );
                }
            }
        }
        break;
    }

#if _DEBUGGING && 0
    if ( rc == 0 && tt -> validate )
        rc = TTransValidate ( tt -> root, tt );
#endif

    return rc;
}

/* TrieInsert
 *  inserts an item into tree
 *
 *  "item" is a TNode with an embedded key string.
 *
 *  returns status codes:
 *    0      => success
 *    EINVAL => an invalid parameter
 *    ENOMEM => failed to allocate internal structures
 */
LIB_EXPORT rc_t CC TrieInsert ( Trie *tt, TNode *item )
{
    return TrieInsertEngine ( tt, item, NULL );
}

/* TrieInsertUnique
 *  behaves like TrieInsert and returns 0 if insert succeeded
 *
 *  if insert failed with a return value of EEXIST, the existing
 *  object is returned in "exist".
 */
LIB_EXPORT rc_t CC TrieInsertUnique ( Trie *tt, TNode *item, TNode **exist )
{
    TNode *dummy;
    if ( exist == NULL )
        exist = & dummy;

    return TrieInsertEngine ( tt, item, exist );
}

/* TrieFindTrans
 */
static
rc_t CC TrieFindTrans ( const Trie *tt, TTrans *trans, const String *item, TTrans **buffer )
{
    String key = * item;

    while ( trans -> child != NULL )
    {
        rc_t rc;
        uint32_t idx;

        assert ( trans -> tcnt != 0 );

        rc = TrieNextIdx ( tt, & key, & idx );
	/* This might actually happen if the trie has more internal
	   nodes than one might think, and the string is short 
	   (i.e. it terminates in an "internal" node */
	/* This needs to know the reconstructed RC for better readability */
	if (rc == RC( rcText, rcChar, rcRemoving, rcString, rcEmpty ) ) {
	  *buffer = trans;
	  return 0;
	}
        if ( rc != 0 )
	  return rc;

        if ( trans -> child [ idx ] == NULL )
            break;

        trans = trans -> child [ idx ];
    }

    * buffer = trans;
    return 0;
}

/* TrieFindTransChain
 *  finds chain of TTrans objects having single transitions
 *
 *  explanation and justification:
 *
 *  - when TrieUnlink removes the last bucket from a value BSTree,
 *    the containing TTrans node may become empty and should be
 *    collected.
 *
 *  - to collect a TTrans object, a backlink to its parent or a
 *    forward list from parent(s) to node are required, in order
 *    to remove the parents' outgoing transition to the dead node.
 *
 *  - backlinks have been excluded from the design for many reasons,
 *    among them the expense of 64 bit pointers. a list of forward
 *    transitions only requires a head and tail, since the internal
 *    transitions are found with the key and may be recovered with
 *    the same key.
 *
 *  - this chain is defined as the head and tail of a list having
 *    a single line of transitions eminating from a remaining parent,
 *    such that "head" will be the deepest parent in the chain having
 *    a vcnt + tcnt > 1, or NULL if none are found. all subsequent nodes
 *    will be subject to collection.
 */
static
rc_t CC TrieFindTransChain ( const Trie *tt, TTrans *trans,
    const String *item, TTrans **head, TTrans **tail )
{
    String key = * item;

    * head = NULL;

    while ( trans -> child != NULL )
    {
        rc_t rc;
        uint32_t idx;

        assert ( trans -> tcnt != 0 );

        rc = TrieNextIdx ( tt, & key, & idx );
        if ( rc != 0 )
            return rc;

        /* detect end of line */
        if ( trans -> child [ idx ] == NULL )
            break;

        /* record new head */
        if ( trans -> tcnt + trans -> vcnt > 1 )
            * head = trans;

        /* take transition */
        trans = trans -> child [ idx ];
    }

    * tail = trans;
    return 0;
}

/* TrieUnlink
 *  remove an object from the tree
 *
 *  returns "true" if node was actually removed from tree
 */
LIB_EXPORT bool CC TrieUnlink ( Trie *tt, TNode *item )
{
    if ( tt != NULL && item != NULL && tt -> root != NULL )
    {
        TTrans *head, *tail;
        rc_t rc = TrieFindTransChain ( tt, tt -> root, & item -> key, & head, & tail );
        if ( rc == 0 && BSTreeUnlink ( & tail -> vals, & item -> n ) )
        {
            assert ( item -> key . len >= tail -> depth );
            if ( item -> key . len == tail -> depth )
            {
                assert ( tail -> vcnt > 0 );
                -- tail -> vcnt;
            }
            else
            {
                assert ( tail -> tcnt > 0 );
                -- tail -> tcnt;
            }

            /* check for need to whack TTrans */
            if ( tail -> vcnt + tail -> tcnt == 0 )
            {
                String key;
                TTrans *trans;
                uint32_t idx;

                if ( head == NULL )
                {
                    /* going to whack everything */
                    key = item -> key;
                    trans = tt -> root;
                }
                else
                {
                    /* remove the leading characters of the key */
                    StringSubstr ( & item -> key, & key, head -> depth, 0 );
                    rc = TrieNextIdx ( tt, & key, & idx );
                    assert ( rc == 0 );
                    assert ( head -> child != NULL );
                    trans = head -> child [ idx ];

                    /* sever the transition */
                    assert ( trans != NULL );
                    head -> child [ idx ] = NULL;
                    assert ( head -> vcnt > 0 );
                    -- head -> vcnt;
                }

                /* whack all of the trans nodes down to tail */
                while ( 1 )
                {
                    TTrans *next;

                    /* get the next character index */
                    rc = TrieNextIdx ( tt, & key, & idx );
                    assert ( rc == 0 );

                    /* get the next node */
                    assert ( trans -> child != NULL );
                    next = trans -> child [ idx ];
                    assert ( next != NULL );

                    /* whack the current node */
                    assert ( trans -> vcnt + trans -> tcnt == 0 );
                    assert ( trans -> vals . root == NULL );
                    free ( trans -> child );
                    free ( trans );

                    /* get out if this was the tail */
                    if ( trans == tail )
                        break;

                    /* take the next transition */
                    trans = next;
                }
            }

            return true;
        }
    }

    return false;
}

/* TrieFind
 *  find a single object within tree
 *
 *  "item" is an exact match text string
 *
 *  returns an arbitrarily selected TNode* from matching set
 *  or NULL if none found.
 */
LIB_EXPORT TNode * CC TrieFind ( const Trie *tt, const String *item )
{
    if ( tt != NULL && item != NULL && tt -> root != NULL && item -> len != 0 )
    {
        TTrans *trans;
        rc_t rc = TrieFindTrans ( tt, tt -> root, item, & trans );
	if (rc == 0)
        {
	    return ( TNode* ) BSTreeFind ( & trans -> vals, item,
                ( int ( CC * ) ( const void*, const BSTNode* ) ) TNodeCmp );
	}
    }

    return NULL;
}

/* TrieFindAll
 *  find multiple objects within tree
 *
 *  "item" is an exact match text string
 *
 *  "buffer" is a user-supplied array of TNode* with a
 *  capacity of "capacity" elements. if successful, the
 *  entire found set will be returned unordered within.
 *
 *  "num_found" ( out ) indicates the size of the found set,
 *  regardless of return value, such that if the supplied
 *  buffer were to be too small, the required size is returned.
 *
 *  returns status codes:
 *    0       => success
 *    EINVAL  => an invalid parameter
 *    ENOENT  => the found set was empty
 *    ENOBUFS => the found set was too large
 */
LIB_EXPORT rc_t CC TrieFindAll ( const Trie *tt, const String *item,
    TNode *buffer [], uint32_t capacity,
    uint32_t *num_found )
{
    rc_t rc;
    TTrans *trans;
    uint32_t count;

    if ( tt == NULL )
        return RC ( rcCont, rcTrie, rcSelecting, rcSelf, rcNull );
    if ( item == NULL )
        return RC ( rcCont, rcTrie, rcSelecting, rcString, rcNull );
    if ( item -> len == 0 )
        return RC ( rcCont, rcTrie, rcSelecting, rcString, rcEmpty );

    if ( buffer == NULL && capacity != 0 )
        return RC ( rcCont, rcTrie, rcSelecting, rcBuffer, rcNull );

    count = 0;

    rc = TrieFindTrans ( tt, tt -> root, item, & trans );
    if ( rc == 0 )
    {
        TNode *node = ( TNode* ) BSTreeFind ( & trans -> vals, item,
            ( int ( CC * ) ( const void*, const BSTNode* ) ) TNodeCmp );
        if ( node == NULL )
            rc = RC ( rcCont, rcTrie, rcSelecting, rcString, rcNotFound );
        else
        {
            TNode *nbr;

            /* back up as far as we can */
            for ( nbr = ( TNode* ) BSTNodePrev ( & node -> n );
                  nbr != NULL && StringEqual ( & nbr -> key, item );
                  nbr = ( TNode* ) BSTNodePrev ( & node -> n ) )
            {
                ++ count;
                node = nbr;
            }

            /* recover to original point */
            if ( count != 0 )
            {
                uint32_t i;
                for ( i = 0; i < count; ++ i )
                {
                    if ( i < capacity )
                        buffer [ i ] = node;
                    node = ( TNode* ) BSTNodeNext ( & node -> n );
                }
            }

            /* count the original */
            if ( count < capacity )
                buffer [ count ] = node;

            /* now continue forward */
            for ( ++ count, node = ( TNode* ) BSTNodeNext ( & node -> n );
                  node != NULL && StringEqual ( & node -> key, item );
                  ++ count, node = ( TNode* ) BSTNodeNext ( & node -> n ) )
            {
                if ( count < capacity )
                    buffer [ count ] = node;
            }

            /* detect no buffer space */
            if ( count > capacity )
                rc = RC ( rcCont, rcTrie, rcSelecting, rcBuffer, rcInsufficient );
        }
    }

    if ( num_found != NULL )
        * num_found = count;

    return rc;
}

/* TrieForEach
 *  executes a function on each tree element
 */
typedef struct TrieForEachData TrieForEachData;
struct TrieForEachData
{
    void ( CC * f ) ( TNode*, void* );
    void *data;
};

static
void CC TrieForEachElem ( const TTransBacktrace *bt, uint32_t width, void *data )
{
    const TTrans *trans = bt -> trans;
    TrieForEachData *pb = data;
    if ( trans -> vals . root != NULL )
    {
        BSTreeForEach ( & trans -> vals, 0,
            ( void ( CC * ) ( BSTNode*, void* ) ) pb -> f, pb -> data );
    }
}

LIB_EXPORT void CC TrieForEach ( const Trie *tt,
    void ( CC * f ) ( TNode *n, void *data ), void *data )
{
    if ( tt != NULL && tt -> root != NULL )
    {
        TrieForEachData pb;
        pb . f = f;
        pb . data = data;
        TTransForEach ( tt -> root, tt -> width, TrieForEachElem, & pb );
    }
}

/* TrieDoUntil
 *  executes a function on each tree element
 *  until the function returns true
 */
typedef struct TrieDoUntilData TrieDoUntilData;
struct TrieDoUntilData
{
    bool ( CC * f ) ( TNode*, void* );
    void *data;
};

static
bool CC TrieDoUntilElem ( const TTransBacktrace *bt, uint32_t width, void *data )
{
    const TTrans *trans = bt -> trans;
    TrieDoUntilData *pb = data;
    if ( trans -> vals . root != NULL )
    {
        return BSTreeDoUntil ( & trans -> vals, 0,
            ( bool ( CC * ) ( BSTNode*, void* ) ) pb -> f, pb -> data );
    }
    return false;
}

LIB_EXPORT bool CC TrieDoUntil ( const Trie *tt,
    bool ( CC * f ) ( TNode *n, void *data ), void *data )
{
    if ( tt != NULL && tt -> root != NULL )
    {
        TrieDoUntilData pb;
        pb . f = f;
        pb . data = data;
        return TTransDoUntil ( tt -> root, tt -> width, TrieDoUntilElem, & pb );
    }
    return false;
}

/* TrieExplore
 *  executes a function on each element in the path of the key
 *  from furthest node to closest node
 *  until the function returns true or end of chain
 */
typedef struct TrieExploreData_struct {
    bool ( CC * f ) ( TNode*, void* );
    void *data;
    const String* item;
} TrieExploreData;

static
bool CC TrieExploreBSTree( BSTNode *n, void *data )
{
    TrieExploreData* pb = data;
    TNode* node = (TNode*)n;
    String sub = *pb->item;
    sub.len = node->key.len;

    if( StringCompare(&node->key, &sub) == 0 ) {
        if( pb->f(node, pb->data) ) {
            return true;
        }
    }
    return false;
}

static
bool CC TrieExploreTrans( const Trie *tt, TTrans *trans, String *key, TrieExploreData* data)
{
    /* first dive deeper if possible */
    while( trans->child != NULL ) {
        uint32_t idx;
        if( TrieNextIdx(tt, key, &idx) != 0 ) {
            break;
        }
        if( trans->child[idx] != NULL ) {
            /* do not recur if current trans has no values */
            if( trans->vcnt > 0 ) {
                if( TrieExploreTrans(tt, trans->child[idx], key, data) ) {
                    return true;
                }
                break;
            } else {
                trans = trans->child[idx];
            }
        }
    }
    /* than search local BSTree on the way out */
    return BSTreeDoUntil(&trans->vals, true, TrieExploreBSTree, data);
}

LIB_EXPORT bool CC TrieExplore( const Trie *tt, const String *item, 
    bool ( CC * f ) ( TNode *n, void *data ), void *data )
{
    if ( tt != NULL && item != NULL && tt->root != NULL && item->len != 0 ) {
        TrieExploreData pb;
        String key = *item;
        pb.f = f;
        pb.data = data;
        pb.item = item;
        return TrieExploreTrans(tt, tt->root, &key, &pb);
    }
    return false;
}

/* TrieWhack
 *  tears down internal structure
 *
 *  any parameter, including"tt," may be NULL
 */
LIB_EXPORT void CC TrieWhack ( Trie *tt,
    void ( CC * whack ) ( TNode *n, void *data ), void *data )
{
    if ( tt != NULL )
    {
        if ( tt -> root != NULL )
            TTransWhack ( tt -> root, tt -> width, whack, data );

        free ( ( void* ) tt -> map );
        free ( ( void* ) tt -> rmap );
    }
}
