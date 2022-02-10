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
#include <klib/log.h>
#include <rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>


/*--------------------------------------------------------------------------
 * TTrans
 *  a text transition node
 *
 *  transitions to child nodes occur only if "tcnt" != 0 ( "child" != NULL )
 *  and are taken on the query string's character [ key . len ]
 */

/* TTransValidate
 */
typedef struct TTValNodeCntData TTValNodeCntData;
struct TTValNodeCntData
{
    unsigned int depth;
    unsigned int vcnt;
    unsigned int tcnt;
};

static
void CC TTransCountTNodes ( BSTNode *n, void *data )
{
    TTValNodeCntData *pb = data;
    const TNode *node = ( const TNode* ) n;

    if ( strlen ( node -> key . addr ) == pb -> depth )
        ++ pb -> vcnt;
    else
        ++ pb -> tcnt;
}

/* TTransValidate
 *  recursively checks structure
 */
int CC TTransValidate ( const TTrans *trans, const Trie *tt )
{
    int status = 0;

    TTValNodeCntData pb;
    unsigned int i, width;

    width = tt -> width;

    pb . depth = trans -> depth;
    pb . vcnt = pb . tcnt = 0;

    /* count transition array */
    if ( trans -> child != NULL )
    {
        for ( i = 0; i < width; ++ i )
        {
            if ( trans -> child [ i ] != NULL )
                ++ pb . tcnt;
        }
    }

    /* count value tree */
    BSTreeForEach ( & trans -> vals, 0, TTransCountTNodes, & pb );

    rc_t rc = 0;
    char msg [256];
    /* check validity */
    if ( pb . tcnt != trans -> tcnt )
    {
        snprintf ( msg, sizeof ( msg ), "bad trans -> tcnt => %u, should be %u",
                   trans -> tcnt, pb . tcnt );
        rc = RC ( rcRuntime, rcTrie, rcValidating, rcFunctParam, rcInvalid );
        LOGERR ( klogErr, rc, msg );
    }

    /* should have a non-zero tcnt if has child array */
    else if ( pb . tcnt == 0 && trans -> child != NULL )
    {
        rc = RC ( rcRuntime, rcTrie, rcValidating, rcFunctParam, rcEmpty );
        LOGERR ( klogErr, rc, "empty trans -> child" );
    }

    if ( pb . vcnt != trans -> vcnt )
    {
        snprintf ( msg, sizeof ( msg ), "bad trans -> vcnt => %u, should be %u",
                   trans -> vcnt, pb . vcnt );
        rc = RC ( rcRuntime, rcTrie, rcValidating, rcFunctParam, rcInvalid );
        LOGERR ( klogErr, rc, msg );
    }

    if ( trans -> child == NULL && pb . tcnt != 0 )
    {
        if ( pb . tcnt + pb . vcnt > tt -> limit )
        {
            snprintf ( msg, sizeof ( msg ), "WARNING: over-limit actual value count => %u, should be %u\n",
                       pb . tcnt + pb . vcnt, tt -> limit );
            LOGMSG ( klogInfo, msg );
        }
    }

    if ( status == 0 && trans -> child != NULL )
    {
        for ( i = 0; i < width; ++ i )
        {
            const TTrans *child = trans -> child [ i ];
            if ( child != NULL )
            {
                if ( child -> depth != trans -> depth + 1 )
                {
                    snprintf ( msg, sizeof ( msg ), "bad trans -> depth => %u, should be %u",
                               child -> depth, trans -> depth + 1 );
                    rc = RC ( rcRuntime, rcTrie, rcValidating, rcFunctParam, rcInvalid );
                    LOGERR ( klogErr, rc, msg );
                }
                else
                {
                    int err = TTransValidate ( child, tt );
                    if ( status == 0 )
                        status = err;
                }
            }
        }
    }

    return status;
}

/*--------------------------------------------------------------------------
 * Trie
 *  a tree of text nodes
 */

/* TrieValidate
 *  run validation check on tree structure
 */
LIB_EXPORT int CC TrieValidate ( const Trie *tt )
{
    if ( tt == NULL )
        return EINVAL;
    if ( tt -> root == NULL )
        return 0;
    return TTransValidate ( tt -> root, tt );
}
