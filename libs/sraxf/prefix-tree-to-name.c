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

#include <sysalloc.h>
#include <sra/sradb.h>
#include <vdb/xform.h>
#include <vdb/table.h>
#include <kdb/index.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>

#include <string.h>
#include <assert.h>

/* ascii NCBI:SRA:prefix_tree_to_name #1 < ascii idx > ( * ascii name );
 *  use prefix tree index data in reverse lookup
 *  as a substitute for physical NAME column
 *
 *  "idx" [ CONST ] - name of prefix-tree text index
 *
 *  "name" [ DATA, OPTIONAL ] - source of unindexed names
 *  when a reverse lookup on "idx" fails and this input is
 *  present, its row is returned.
 */
static
rc_t prefix_tree_to_name ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const KIndex *idx = ( const void* ) self;
    KDataBuffer *dst = rslt -> data;

    /* the buffer should have already been given the correct element size */
    if ( dst -> elem_bits != 8 )
    {
        rc = KDataBufferCast ( dst, dst, 8, true );
        if ( rc != 0 )
            return rc;
    }

    /* the actual size depends upon whether the buffer is new or was resized very small */
    if ( dst -> elem_count < 8 )
    {
        rc = KDataBufferResize ( dst, 4 * 1024 );
        if ( rc != 0 )
            return rc;
    }

    /* try to get the text with current size */
    rc = KIndexProjectText ( idx, row_id, NULL, NULL, dst -> base, dst -> elem_count, NULL );
    if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcBuffer )
    {
        /* go to 4K buffer - should never fail */
        rc = KDataBufferResize ( dst, 4 * 1024 );
        if ( rc == 0 )
        {
            rc = KIndexProjectText ( idx, row_id, NULL, NULL, dst -> base, dst -> elem_count, NULL );
            if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcBuffer )
            {
                /* go to 16K buffer - this is absurd */
                rc = KDataBufferResize ( dst, 16 * 1024 );
                if ( rc == 0 )
                {
                    rc = KIndexProjectText ( idx, row_id, NULL, NULL, dst -> base, dst -> elem_count, NULL );
                    if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcBuffer )

                        /* reject this name, it is likely garbage */
                        rc = RC ( rcSRA, rcIndex, rcProjecting, rcName, rcExcessive );
                }
            }
        }
    }

    /* test to see if we got the name */
    if ( rc == 0 )
    {
        rslt -> elem_count = (uint32_t)strlen ( ( const char* ) dst -> base );
        return 0;
    }

    /* if id was not found, check for alternate input */
    if ( GetRCState ( rc ) == rcNotFound && argc == 1 && argv [ 0 ] . u . data . elem_count != 0 )
    {
        if ( dst -> elem_count < argv [ 0 ] . u . data . elem_count )
        {
            rc = KDataBufferResize ( dst, argv [ 0 ] . u . data . elem_count );
            if ( rc != 0 )
                return rc;
        }

        rc = 0;

        string_copy ( dst -> base, dst -> elem_count,
            argv [ 0 ] . u . data . base, argv [ 0 ] . u . data . elem_count );
    }

    return rc;
}

static
void release_prefix_tree ( void *item )
{
    KIndexRelease ( ( const void* ) item );
}

VTRANSFACT_IMPL( NCBI_SRA_prefix_tree_to_name, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* the first thing to do is to open the prefix tree */
    const KIndex *idx;
    rc_t rc = VTableOpenIndexRead ( info -> tbl, & idx, "%.*s",
        ( int ) cp -> argv [ 0 ] . count, cp -> argv [ 0 ] . data . ascii );
    if ( rc == 0 )
    {
        KIdxType type;
        rc = KIndexType ( idx, & type );
        if ( rc == 0 )
        {
            if ( type == ( kitText | kitProj ) )
            {
                rslt -> self = ( void* ) idx;
                rslt -> whack = release_prefix_tree;
                rslt -> u . rf = prefix_tree_to_name;
                rslt -> variant = vftRow;
                return 0;
            }

            rc = RC ( rcSRA, rcFunction, rcConstructing, rcIndex, rcIncorrect );
        }

        KIndexRelease ( idx );
    }
    else {
        OUTMSG("VTableOpenIndexRead failed: %R", rc);
    }

    return rc;
}
