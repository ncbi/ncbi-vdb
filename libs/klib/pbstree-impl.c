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
#include "pbstree-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>
#include "maybeswap.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * PBSTreeImpl
 *  a flattened b-tree
 */

/* Count
 *  returns number of elements in b-tree
 */
static
uint32_t CC PBSTreeImplCount ( const PBSTree *self )
{
    uint32_t n;
    GET32 ( n, self -> pt -> num_nodes );
    return n;
}

/* Depth
 *  returns number of layers in b-tree
 */
static
uint32_t CC PBSTreeImplDepth ( const PBSTree *self )
{
    uint32_t depth, num_nodes;

    GET32 ( num_nodes, self -> pt -> num_nodes );

    for ( depth = 0; num_nodes != 0; ++ depth )
        num_nodes >>= 1;

    return depth;
}

/* Size
 *  returns the size in bytes
 *  of the PBSTree image
 */
static
size_t CC PBSTreeImplSize ( const PBSTree *oself )
{
    size_t bytes;
    const P_BSTree *self = oself -> pt;
    uint32_t data_size, bytes32;

    GET32 ( bytes32, self -> num_nodes );
    if ( bytes32 == 0 ) /* empty tree */
        return sizeof self -> num_nodes;
    else
        bytes = bytes32;

    GET32 ( data_size, self -> data_size );
    if ( data_size > 256 )
    {
        if ( data_size <= 65536 )
            bytes += bytes;
        else
            bytes <<= 2;
    }

    return sizeof * self - sizeof self -> data_idx +
        bytes + data_size;
}

/* GetNodeData
 *  finds node data boundaries
 */
static
rc_t CC PBSTreeImplGetNodeData8 ( const PBSTree *oself,
    const void **addr, size_t *size, uint32_t id )
{
    const P_BSTree *self = oself -> pt;

    uint32_t num_nodes;
    const uint8_t *data_start;
    uint32_t off, end;

    GET32 ( num_nodes, self -> num_nodes );
    GET32 ( end, self -> data_size );

    if ( id == 0 )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNull );
    if ( id > num_nodes )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNotFound );

    data_start = ( const uint8_t* )
      & self -> data_idx . v8 [ num_nodes ];
    off = self -> data_idx . v8 [ id - 1 ];
    if ( id != num_nodes )
      end = self -> data_idx . v8 [ id ];

    * addr = & data_start [ off ];
    * size = end - off;
    return 0;
}

static
rc_t CC PBSTreeImplGetNodeData16 ( const PBSTree *oself,
    const void **addr, size_t *size, uint32_t id )
{
    const P_BSTree *self = oself -> pt;

    uint32_t num_nodes, end;
    const uint8_t *data_start;
    uint16_t off;

    GET32 ( num_nodes, self -> num_nodes );
    GET32 ( end, self -> data_size );

    if ( id == 0 )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNull );
    if ( id > num_nodes )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNotFound );

    data_start = ( const uint8_t* )
      & self -> data_idx . v16 [ num_nodes ];
    GET16 ( off, self -> data_idx . v16 [ id - 1 ] );
    if ( id != num_nodes )
    {
      uint16_t end16;
      GET16 ( end16, self -> data_idx . v16 [ id ] );
      end = end16;
    }

    * addr = & data_start [ off ];
    * size = end - off;
    return 0;
}

static
rc_t CC PBSTreeImplGetNodeData32 ( const PBSTree *oself,
    const void **addr, size_t *size, uint32_t id )
{
    const P_BSTree *self = oself -> pt;

    uint32_t num_nodes;
    const uint8_t *data_start;
    uint32_t off, end;

    GET32 ( num_nodes, self -> num_nodes );
    GET32 ( end, self -> data_size );

    if ( id == 0 )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNull );
    if ( id > num_nodes )
        return RC ( rcCont, rcTree, rcAccessing, rcId, rcNotFound );

    data_start = ( const uint8_t* )
      & self -> data_idx . v32 [ num_nodes ];
    GET32 ( off, self -> data_idx . v32 [ id - 1 ] );
    if ( id != num_nodes )
      GET32 ( end, self -> data_idx . v32 [ id ] );

    * addr = & data_start [ off ];
    * size = end - off;
    return 0;
}

/* Find
 *  find an object within tree
 *  "cmp" function returns equivalent of "item" - "n"
 */
static
uint32_t CC PBSTreeImplFind8 ( const PBSTree *self, PBSTNode *n, const void *item,
    int ( CC * cmp ) ( const void *item, const PBSTNode *n, void * data ), void * data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t right;
    GET32 ( right, pt -> num_nodes );
    if ( right != 0 )
    {
        int diff;
        const uint8_t *data_start;
        uint32_t off, end;
        uint32_t id, left = 1;
        uint32_t max_right = right;

        GET32 ( end, pt -> data_size );
	data_start = & pt -> data_idx . v8 [ right ];
	
	do
	  {
	    id = ( left + right ) >> 1;
	    off = pt -> data_idx . v8 [ id - 1 ];
	    n -> id = id;
	    n -> data . addr = & data_start [ off ];
	    if ( id == max_right )
	      n -> data . size = end - off;
	    else
	      n -> data . size = pt -> data_idx . v8 [ id ] - off;
	    diff = ( * cmp ) ( item, n, data );
	    if ( diff == 0 )
	      return id;
	    if ( diff < 0 )
	      right = id - 1;
	    else
	      left = id + 1;
	  }
	while ( left <= right );
    }

    n -> data . addr = NULL;
    n -> data . size = 0;
    return n -> id = 0;
}

static
uint32_t CC PBSTreeImplFind16 ( const PBSTree *self, PBSTNode *n, const void *item,
    int ( CC * cmp ) ( const void *item, const PBSTNode *n, void * data ), void * data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t right;
    GET32 ( right, pt -> num_nodes );
    if ( right != 0 )
    {
        int diff;
        const uint8_t *data_start;
        uint32_t end;
        uint32_t id, left = 1;
        uint32_t max_right = right;
        uint16_t off;

        GET32 ( end, pt -> data_size );
	data_start = ( const uint8_t* )
	  & pt -> data_idx . v16 [ right ];
	
	do
	  {
	    id = ( left + right ) >> 1;
	    GET16 ( off, pt -> data_idx . v16 [ id - 1 ] );
	    n -> id = id;
	    n -> data . addr = & data_start [ off ];
	    if ( id == max_right )
	      n -> data . size = end - off;
	    else
	    {
	      uint16_t size;
	      GET16 ( size, pt -> data_idx . v16 [ id ] );
	      n -> data . size = size - off;
	    }
	    diff = ( * cmp ) ( item, n, data );
	    if ( diff == 0 )
	      return id;
	    if ( diff < 0 )
	      right = id - 1;
	    else
	      left = id + 1;
	  }
	while ( left <= right );
    }

    n -> data . addr = NULL;
    n -> data . size = 0;
    return n -> id = 0;
}

static
uint32_t CC PBSTreeImplFind32 ( const PBSTree *self, PBSTNode *n, const void *item,
    int ( CC * cmp ) ( const void *item, const PBSTNode *n, void * data ), void * data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t right;
    GET32 ( right, pt -> num_nodes );
    if ( right != 0 )
    {
        int diff;
        const uint8_t *data_start;
        uint32_t off, end;
        uint32_t id, left = 1;
        uint32_t max_right = right;

        GET32 ( end, pt -> data_size );
	data_start = ( const uint8_t* )
	  & pt -> data_idx . v32 [ right ];
	
	do
	  {
	    id = ( left + right ) >> 1;
	    GET32 ( off, pt -> data_idx . v32 [ id - 1 ] );
	    n -> id = id;
	    n -> data . addr = & data_start [ off ];
	    if ( id == max_right )
	      n -> data . size = end - off;
	    else
	    {
	      GET32 ( n -> data . size, pt -> data_idx . v32 [ id ] );
	      n -> data . size -= off;
	    }
	    diff = ( * cmp ) ( item, n, data );
	    if ( diff == 0 )
	      return id;
	    if ( diff < 0 )
	      right = id - 1;
	    else
	      left = id + 1;
	  }
	while ( left <= right );
    }

    n -> data . addr = NULL;
    n -> data . size = 0;
    return n -> id = 0;
}

/* ForEach
 *  executes a function on each tree element
 */
static
void CC PBSTreeImplForEach8 ( const PBSTree *self, bool reverse,
    void ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id;
        uint32_t off, end;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

	data_start = & pt -> data_idx . v8 [ num_nodes ];
	
	if ( ! reverse )
	  {
	    for ( off = pt -> data_idx . v8 [ 0 ],
		    id = 1; id < num_nodes; off = end, ++ id )
	      {
		end = pt -> data_idx . v8 [ id ];
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
	    
	    GET32 ( end, pt -> data_size );
	  }
	
	off = pt -> data_idx . v8 [ num_nodes - 1 ];
	n . internal = pt;
	n . id = num_nodes;
	n . data . addr = & data_start [ off ];
	n . data . size = end - off;
	( * f ) ( & n, data );
	
	if ( reverse )
	  {
	    for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
	      {
		off = pt -> data_idx . v8 [ id - 1 ];
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
	  }
    }
}

static
void CC PBSTreeImplForEach16 ( const PBSTree *self, bool reverse,
    void ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id, end;
        uint16_t off;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

	data_start = ( const uint8_t* )
	  & pt -> data_idx . v16 [ num_nodes ];
	
	if ( ! reverse )
	  {
	    GET16 ( off, pt -> data_idx . v16 [ 0 ] );
	    for ( id = 1; id < num_nodes; off = end, ++ id )
	      {
		uint16_t end16;
		GET16 ( end16, pt -> data_idx . v16 [ id ] );
		end = end16;
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
	    
	    GET32 ( end, pt -> data_size );
	  }
	
	GET16 ( off, pt -> data_idx . v16 [ num_nodes - 1 ] );
	n . internal = pt;
	n . id = num_nodes;
	n . data . addr = & data_start [ off ];
	n . data . size = end - off;
	( * f ) ( & n, data );
        
	if ( reverse )
	  {
	    for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
	      {
		GET16 ( off, pt -> data_idx . v16 [ id - 1 ] );
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
	  }
    }
}

static
void CC PBSTreeImplForEach32 ( const PBSTree *self, bool reverse,
    void ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id;
        uint32_t off, end;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

	data_start = ( const uint8_t* )
	  & pt -> data_idx . v32 [ num_nodes ];
	
	if ( ! reverse )
	  {
	    GET32 ( off, pt -> data_idx . v32 [ 0 ] );
	    for ( id = 1; id < num_nodes; off = end, ++ id )
	      {
		GET32( end, pt -> data_idx . v32 [ id ] );
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
                
	    GET32 ( end, pt -> data_size );
	  }

	GET32 ( off, pt -> data_idx . v32 [ num_nodes - 1 ] );
	n . internal = pt;
	n . id = num_nodes;
	n . data . addr = & data_start [ off ];
	n . data . size = end - off;
	( * f ) ( & n, data );

	if ( reverse )
	  {
	    for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
	      {
		GET32 ( off, pt -> data_idx . v32 [ id - 1 ] );
		n . internal = pt;
		n . id = id;
		n . data . addr = & data_start [ off ];
		n . data . size = end - off;
		( * f ) ( & n, data );
	      }
	  }
    }
}

/* DoUntil
 *  executes a function on each element
 *  until the function returns true
 */
static
bool CC PBSTreeImplDoUntil8 ( const PBSTree *self, bool reverse,
    bool ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id;
        uint32_t off, end;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

        data_start = & pt -> data_idx . v8 [ num_nodes ];
	
        if ( ! reverse )
        {
            for ( off = pt -> data_idx . v8 [ 0 ],
                  id = 1; id < num_nodes; off = end, ++ id )
            {
                end = pt -> data_idx . v8 [ id ];
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
	    
            GET32 ( end, pt -> data_size );
        }

        off = pt -> data_idx . v8 [ num_nodes - 1 ];
        n . internal = pt;
        n . id = num_nodes;
        n . data . addr = & data_start [ off ];
        n . data . size = end - off;
        if ( ( * f ) ( & n, data ) )
            return true;

        if ( reverse )
        {
            for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
            {
                off = pt -> data_idx . v8 [ id - 1 ];
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
        }
    }

    return 0;
}

static
bool CC PBSTreeImplDoUntil16 ( const PBSTree *self, bool reverse,
    bool ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id, end;
        uint16_t off;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

        data_start = ( const uint8_t* )
            & pt -> data_idx . v16 [ num_nodes ];
            
        if ( ! reverse )
        {
            GET16 ( off, pt -> data_idx . v16 [ 0 ] );
            for ( id = 1; id < num_nodes; off = end, ++ id )
            {
                uint16_t end16;
                GET16 ( end16, pt -> data_idx . v16 [ id ] );
                end = end16;
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
                
            GET32 ( end, pt -> data_size );
        }

        GET16 ( off, pt -> data_idx . v16 [ num_nodes - 1 ] );
        n . internal = pt;
        n . id = num_nodes;
        n . data . addr = & data_start [ off ];
        n . data . size = end - off;
        if ( ( * f ) ( & n, data ) )
            return true;
            
        if ( reverse )
        {
            for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
            {
                GET16 ( off, pt -> data_idx . v16 [ id - 1 ] );
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
        }
    }
    return 0;
}

static
bool CC PBSTreeImplDoUntil32 ( const PBSTree *self, bool reverse,
    bool ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    const P_BSTree *pt = self -> pt;
    uint32_t num_nodes;
    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes != 0 )
    {
        PBSTNode n;
        uint32_t id;
        uint32_t off, end;
        const uint8_t *data_start;

        GET32 ( end, pt -> data_size );

        data_start = ( const uint8_t* )
            & pt -> data_idx . v32 [ num_nodes ];
            
        if ( ! reverse )
        {
            GET32 ( off, pt -> data_idx . v32 [ 0 ] );
            for ( id = 1; id < num_nodes; off = end, ++ id )
            {
                GET32 ( end, pt -> data_idx . v32 [ id ] );
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
                
            GET32 ( end, pt -> data_size );
        }
            
        GET32 ( off, pt -> data_idx . v32 [ num_nodes - 1 ] );
        n . internal = pt;
        n . id = num_nodes;
        n . data . addr = & data_start [ off ];
        n . data . size = end - off;
        if ( ( * f ) ( & n, data ) )
            return true;
            
        if ( reverse )
        {
            for ( end = off, id = num_nodes - 1; id > 0; end = off, -- id )
            {
                GET32 ( off, pt -> data_idx . v32 [ id - 1 ] );
                n . internal = pt;
                n . id = id;
                n . data . addr = & data_start [ off ];
                n . data . size = end - off;
                if ( ( * f ) ( & n, data ) )
                    return true;
            }
        }
    }

    return 0;
}

/* Whack
 *  whacks PBSTree object
 *  does not free memory reference
 */
static
void CC PBSTreeImplWhack ( PBSTree *self )
{
    free ( self );
}


static PBSTree_vt_v1 vtPBSTreeImpl8 =
{
    /* version 1.0 */
    1, 0,

    /* required methods */
    PBSTreeImplWhack,
    PBSTreeImplCount,
    PBSTreeImplDepth,
    PBSTreeImplSize,
    PBSTreeImplGetNodeData8,
    PBSTreeImplFind8,
    PBSTreeImplForEach8,
    PBSTreeImplDoUntil8
};

static PBSTree_vt_v1 vtPBSTreeImpl16 =
{
    /* version 1.0 */
    1, 0,

    /* required methods */
    PBSTreeImplWhack,
    PBSTreeImplCount,
    PBSTreeImplDepth,
    PBSTreeImplSize,
    PBSTreeImplGetNodeData16,
    PBSTreeImplFind16,
    PBSTreeImplForEach16,
    PBSTreeImplDoUntil16
};

static PBSTree_vt_v1 vtPBSTreeImpl32 =
{
    /* version 1.0 */
    1, 0,

    /* required methods */
    PBSTreeImplWhack,
    PBSTreeImplCount,
    PBSTreeImplDepth,
    PBSTreeImplSize,
    PBSTreeImplGetNodeData32,
    PBSTreeImplFind32,
    PBSTreeImplForEach32,
    PBSTreeImplDoUntil32
};


/* GetVTable
 */
static
PBSTree_vt * CC PBSTreeImplGetVTable ( const P_BSTree *pt )
{
    uint32_t data_size;
    GET32 ( data_size, pt -> data_size );
    if ( data_size <= 256 )
      return (PBSTree_vt *) & vtPBSTreeImpl8;
    if ( data_size <= 65536 )
      return (PBSTree_vt *) & vtPBSTreeImpl16;
    return (PBSTree_vt *) & vtPBSTreeImpl32;
}


/* CheckPersisted
 *  check persisted image for apparent correctness
 */
static
rc_t CC PBSTreeImplCheckPersisted ( const P_BSTree *pt, size_t size )
{
    uint32_t num_nodes;

    if ( pt == NULL)
        return RC ( rcCont, rcTree, rcConstructing, rcParam, rcNull );
    if ( size == 0 )
        return 0;
    if ( size < sizeof pt -> num_nodes )
        return RC ( rcCont, rcTree, rcConstructing, rcData, rcInvalid );

    GET32 ( num_nodes, pt -> num_nodes );
    if ( num_nodes > 0 )
    {
        uint32_t data_size;
        const uint8_t *end, *data_start;

        GET32 ( data_size, pt -> data_size );
        if ( size < sizeof * pt || data_size == 0 )
            return RC ( rcCont, rcTree, rcConstructing, rcData, rcIncomplete );

        end = ( const uint8_t* ) pt + size;

        if ( data_size <= 256 )
            data_start = & pt -> data_idx . v8 [ num_nodes ];
        else  if ( data_size <= 65536 )
        {
            data_start = ( const uint8_t* )
                & pt -> data_idx . v16 [ num_nodes ];
        }
        else
        {
            data_start = ( const uint8_t* )
                & pt -> data_idx . v32 [ num_nodes ];
        }
        
        if ( & data_start [ data_size ] > end )
            return RC ( rcCont, rcTree, rcConstructing, rcData, rcCorrupt );
    }
    
    return 0;
}

/* PBSTreeMake
 *  make a PBSTree structure
 *  the memory reference passed in is borrowed
 */
#if SWAP_PERSISTED
#define PBSTREEMAKE PBSTreeMakeSwapped
#else
#define PBSTREEMAKE PBSTreeMakeNative
#endif

rc_t PBSTREEMAKE ( PBSTree *pt, const void *addr, size_t size )
{
    rc_t rc = PBSTreeImplCheckPersisted ( addr, size );
    if ( rc == 0 )
    {
        PBSTree_vt *vt = PBSTreeImplGetVTable ( addr );
        rc = PBSTreeInit ( pt, vt, addr );
    }
    return rc;
}
