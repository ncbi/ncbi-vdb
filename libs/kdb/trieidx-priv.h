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

#ifndef _h_trieidx_priv_
#define _h_trieidx_priv_

#include "index-priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * KPTrieIndexHdr_v1
 */
typedef KIndexFileHeader_v1_v2 KPTrieIndexHdr_v1;

/*--------------------------------------------------------------------------
 * KPTrieIndexHdr_v2
 */
typedef struct KPTrieIndexHdr_v2 KPTrieIndexHdr_v2;
struct KPTrieIndexHdr_v2
{
    KIndexFileHeader_v1_v2 dad;
    int64_t first;
    int64_t last;
    uint16_t id_bits;
    uint16_t span_bits;
    uint16_t align [ 2 ];
};

/*--------------------------------------------------------------------------
 * KPTrieIndexHdr_v3
 */
typedef struct KPTrieIndexHdr_v3 KPTrieIndexHdr_v3;
struct KPTrieIndexHdr_v3
{
    struct KIndexFileHeader_v3 dad;
    int64_t first;
    int64_t last;
    uint16_t id_bits;
    uint16_t span_bits;
    uint16_t align [ 2 ];
};

#ifdef __cplusplus
}
#endif


#endif /* _h_trieidx_priv_ */
