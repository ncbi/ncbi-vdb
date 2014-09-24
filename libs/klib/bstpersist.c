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
#include "pbstree-priv.h"
#include <klib/container.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * PBSTree
 * PBSTreeData
 */

typedef struct PBSTreeData PBSTreeData;
struct PBSTreeData
{
    PTWriteFunc write;
    void *write_param;

    PTAuxFunc aux;
    void *aux_param;

    P_BSTree *pt;
    void ( CC * record ) ( P_BSTree*, unsigned int, size_t );

    size_t num_writ;
    size_t data_size;
    uint32_t num_nodes;

    rc_t rc;
};

static
bool CC PBSTreeGatherInfo ( BSTNode *n, void *data )
{
    PBSTreeData *pb = data;

    size_t node_size;
    pb -> rc = ( * pb -> aux ) ( pb -> aux_param, n, & node_size, NULL, NULL );
    if ( pb -> rc != 0 )
        return true;

    if ( pb -> pt != NULL )
        ( * pb -> record ) ( pb -> pt, pb -> num_nodes, pb -> data_size );

    pb -> data_size += node_size;
    ++ pb -> num_nodes;

    return false;
}

static
void CC PBSTreeRecordU8 ( P_BSTree *pt, unsigned int idx, size_t offset )
{
    pt -> data_idx . v8 [ idx ] = ( uint8_t ) offset;
}

static
void CC PBSTreeRecordU16 ( P_BSTree *pt, unsigned int idx, size_t offset )
{
    pt -> data_idx . v16 [ idx ] = ( uint16_t ) offset;
}

static
void CC PBSTreeRecordU32 ( P_BSTree *pt, unsigned int idx, size_t offset )
{
    pt -> data_idx . v32 [ idx ] = ( uint32_t ) offset;
}

static
bool CC PBSTreeWriteNodes ( BSTNode *n, void *data )
{
    PBSTreeData *pb = data;

    size_t node_size;
    pb -> rc = ( * pb -> aux ) ( pb -> aux_param,
        n, & node_size, pb -> write, pb -> write_param );
    if ( pb -> rc != 0 )
        return true;

    pb -> num_writ += node_size;
    return false;
}

/*--------------------------------------------------------------------------
 * BSTree
 */

/* BSTreePersist
 *  write a b-tree to some storage location
 *
 *  the b-tree is persisted by making between one and three passes
 *  over its nodes, see description of "write" parameter.
 *
 *  the first pass examines internal tree structure and invokes
 *  a user-supplied function to determine overall size.
 *
 *  the second pass persists the internal structure in a packed
 *  format, using the user-supplied generic "write" function.
 *
 *  the third pass invokes another user-supplied function to write
 *  auxiliary node data to output.
 *
 *  "num_writ" returns the number of bytes written as a result of
 *  persisting the b-tree. this will be the actual bytes written
 *  regardless of return status.
 *
 *  "write" is a generic output streaming function used for all
 *  operations. if NULL, then the function will exit after its
 *  first pass with the number of bytes required in "num_writ".
 *
 *  "aux" is a specialized function for streaming auxiliary node
 *  data to output using the supplied "write" function. it is invoked
 *  during the first pass with a NULL write function for gathering
 *  size data, and during the third pass with a non-NULL write function.
 */
KLIB_EXTERN rc_t CC BSTreePersist ( const BSTree *bt, size_t *num_writ,
    PTWriteFunc write, void *write_param, PTAuxFunc aux, void *aux_param )
{
    PBSTreeData pb;

    if ( num_writ != NULL )
        * num_writ = 0;

    if ( bt == NULL )
        return RC ( rcCont, rcTree, rcPersisting, rcSelf, rcNull );
    if ( aux == NULL )
        return RC ( rcCont, rcTree, rcPersisting, rcFunction, rcNull );

    pb . num_writ = 0;

    /* handle the trivial case */
    if ( bt -> root == NULL )
    {
        P_BSTree pt;
        if ( write == NULL )
        {
            pb . num_writ = sizeof pt . num_nodes;
            pb . rc = 0;
        }
        else
        {
            pt . num_nodes = 0;
            pb . rc = ( * write ) ( write_param,
                & pt, sizeof pt . num_nodes, & pb . num_writ );
        }
    }
    else
    {
        /* initialize callback param block */
        pb . write = write;
        pb . write_param = write_param;
        pb . aux = aux;
        pb . aux_param = aux_param;
        pb . rc = 0;

        /* count nodes and tally data size */
        pb . pt = NULL;
        pb . data_size = 0;
        pb . num_nodes = 0;
        BSTreeDoUntil ( bt, 0, PBSTreeGatherInfo, & pb );
        if ( pb . rc == 0 )
        {
            size_t pt_size;

            /* determine object size */
            if ( pb . data_size <= 256 )
            {
                pt_size = 1;
                pb . record = PBSTreeRecordU8;
            }
            else if ( pb . data_size <= 65536 )
            {
                pt_size = 2;
                pb . record = PBSTreeRecordU16;
            }
            else
            {
                pt_size = 4;
                pb . record = PBSTreeRecordU32;
            }

            pt_size = sizeof * pb . pt - sizeof pb . pt -> data_idx +
                pb . num_nodes * pt_size;

            if ( write == NULL )
                pb . num_writ = pt_size + pb . data_size;

            else
            {
                pb . pt = malloc ( pt_size );
                if ( pb . pt != NULL )
                {
                    pb . pt -> num_nodes = pb . num_nodes;
                    pb . pt -> data_size = ( uint32_t ) pb . data_size;
                    
                    /* record node offsets */
                    pb . data_size = 0;
                    pb . num_nodes = 0;
                    BSTreeDoUntil ( bt, 0, PBSTreeGatherInfo, & pb );
                    if ( pb . rc != 0 )
                        free ( pb . pt );
                    else
                    {
                        pb . rc = ( write ) ( write_param,
                            pb . pt, pt_size, & pb . num_writ );
                        free ( pb . pt );
                        
                        if ( pb . rc == 0 )
                            BSTreeDoUntil ( bt, 0, PBSTreeWriteNodes, & pb );
                    }
                }
            }
        }
    }

    if ( num_writ != NULL )
        * num_writ = pb . num_writ;

    return pb . rc;
}
