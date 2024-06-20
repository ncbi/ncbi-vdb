/*==============================================================================
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
*/
#include <align/extern.h>

#include <klib/rc.h>
#include <insdc/insdc.h>

#include "reference-cmn.h"
#include <sysalloc.h>

#include "../klib/int_checks-priv.h"

rc_t CC ReferenceSeq_ReOffset(bool circular, INSDC_coord_len seq_len, INSDC_coord_zero* offset)
{
    /* TODO: INSDC_coord_len - unsigned is used for an arithmetic type,
       consider changing it in the interfaces */
    assert ( FITS_INTO_INT32 ( *offset ) );
    assert ( FITS_INTO_INT32 ( seq_len ) );
    if( !circular && (*offset < 0 || *offset >= (int32_t)seq_len) ) {
        return RC(rcAlign, rcType, rcReading, rcOffset, rcOutofrange);
    } else if( *offset < 0 ) {
        *offset = (int32_t)seq_len - ((-(*offset)) % seq_len);
    } else if( circular && *offset > (int32_t)seq_len ) {
        *offset %= seq_len;
    }
    return 0;
}
