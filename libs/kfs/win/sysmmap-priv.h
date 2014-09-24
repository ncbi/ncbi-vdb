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

#ifndef _h_sysmmap_priv_
#define _h_sysmmap_priv_

#ifndef _h_mmap_priv_
#include "../mmap-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */
struct KMMap
{
    uint64_t off;
    uint64_t pos;
    size_t size;
    char *addr;

    struct KFile *f;
    size_t pg_size;

    /* file mapping handle */
    HANDLE handle;

    uint32_t addr_adj;
    uint32_t size_adj;

    KRefcount refcount;

    bool read_only;
    bool sys_mmap;
    bool dirty;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_sysmmap_priv_ */
