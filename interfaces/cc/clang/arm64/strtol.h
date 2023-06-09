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

#ifndef _h_strtol_
#define _h_strtol_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * strtoi32
 * strtou32
 *  based upon actual usage
 */
#define strtoi32( str, endp, base ) \
    ( int32_t ) strtol ( str, endp, base )

#define strtou32( str, endp, base ) \
    ( uint32_t ) strtoul ( str, endp, base )


/*--------------------------------------------------------------------------
 * strtoi64
 * strtou64
 *  based upon actual usage
 */
#define strtoi64( str, endp, base ) \
    strtol ( str, endp, base )

#define strtou64( str, endp, base ) \
    strtoul ( str, endp, base )


#ifdef __cplusplus
}
#endif

#endif /* _h_strtol_ */
