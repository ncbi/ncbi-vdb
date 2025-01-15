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

#include <klib/rc.h>
#include <vdb/xform.h>

/*
 extern function U32 [ 3 ] NCBI:SRA:_454_:dynamic_read_desc #1 < * U32 edit_distance >
    ( NCBI:SRA:_454_:drdparam_set spot, NCBI:SRA:_454_:drdparam_set key * NCBI:SRA:_454_:drdparam_set linker );
 */
VTRANSFACT_IMPL ( NCBI_SRA__454__dynamic_read_desc, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    abort();
    // moved to sra-tools
    return RC ( rcSRA, rcFunction, rcConstructing, rcItem, rcUnsupported );
}
