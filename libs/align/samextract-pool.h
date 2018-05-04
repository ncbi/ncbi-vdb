/* ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnologmsgy Information
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

#ifndef _h_samextract_pool_
#define _h_samextract_pool_
#include "samextract.h"

#define POOL_BLOCK_SZ (2 * 1024 * 1024)

#ifdef __cplusplus
extern "C" {
#endif
void pool_init(void);
void* pool_alloc(size_t alloc_size);
void pool_destroy(void);
char* pool_strdup(const char* str);
char* pool_memdup(const char* str, size_t len);
void morecore(size_t alloc_size);
extern void* cur_block;
extern size_t cur_block_remain;

#ifdef __cplusplus
}
#endif

#endif
