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
#include <vdb/extern.h>
#include <vdb/xform.h>
#include <insdc/insdc.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

static
rc_t CC lower_case_tech_reads ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    uint32_t i, j, k;

    char *mixed;
    const char *read_text = argv [ 0 ] . u . data . base;
    const INSDC_coord_len *read_len = argv [ 1 ] . u . data . base;
    const INSDC_read_type *read_type = argv [ 2 ] . u . data . base;

    rc_t rc = KDataBufferResize ( rslt -> data, argv [ 0 ] . u . data . elem_count );
    if ( rc != 0 )
        return rc;

    rslt -> elem_count = argv [ 0 ] . u . data . elem_count;

    mixed = rslt -> data -> base;
    read_text += argv [ 0 ] . u . data . first_elem;
    read_len += argv [ 1 ] . u . data . first_elem;
    read_type += argv [ 2 ] . u . data . first_elem;

    for ( i = j = 0; i < argv [ 0 ] . u . data . elem_count; ++ j )
    {
        /* should still have reads described */
        if ( j == argv [ 1 ] . u . data . elem_count || j == argv [ 2 ] . u . data . elem_count )
            break;

        /* get read length */
        k = read_len [ j ];
        if ( i + k > argv [ 0 ] . u . data . elem_count )
            k = argv [ 0 ] . u . data . elem_count - i;

        /* decide upon upper or lower case */
        if ( ( read_type [ j ] & 1 ) != READ_TYPE_BIOLOGICAL )
        {
            for ( ; k > 0; ++ i, -- k )
                mixed [ i ] = tolower ( read_text [ i ] );
        }
        else
        {
            for ( ; k > 0; ++ i, -- k )
                mixed [ i ] = toupper ( read_text [ i ] );
        }
    }

    for ( ; i < argv [ 0 ] . u . data . elem_count; ++ i )
        mixed [ i ] = toupper ( read_text [ i ] );

    return 0;
}


/* lower_case_tech_reads
 *  locates technical reads within an IUPAC sequence
 *  converts them to lower-case
 *
 *  "read_text" [ DATA ] - IUPAC source text
 *
 *  "read_len" [ DATA ] - ordered array of read lengths
 *
 *  "read_type" [ DATA ] - describes which reads are technical
 *  and which are biological. determines the case of each read segment
 *
 * extern function
 * INSDC:dna:mixed_case:text NCBI:lower_case_tech_reads #1 ( INSDC:dna:text read_text,
 *     INSDC:coord:len read_len, INSDC:read:type read_type );
 */
VTRANSFACT_IMPL ( NCBI_lower_case_tech_reads, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> u . rf = lower_case_tech_reads;
    rslt -> variant = vftRow;
    return 0;
}
