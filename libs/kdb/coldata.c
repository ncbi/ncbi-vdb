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

#include "coldata.h"

#include <kfs/file.h>

/* Read
 *  reads from the file using a blob map
 */
rc_t KColumnDataRead ( size_t pgsize, struct KFile const * f, uint64_t pg,
    size_t offset, void *buffer, size_t bsize, size_t *num_read )
{
    uint64_t pos;

    if ( bsize == 0 )
    {
        assert ( num_read != NULL );
        * num_read = 0;
        return 0;
    }

    pos = pg * pgsize;
    return KFileRead ( f, pos + offset, buffer, bsize, num_read );
}


