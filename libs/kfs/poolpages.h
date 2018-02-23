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
 */

#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/recorder.h>

struct ThePool;
struct PoolPage;
 
rc_t make_pool ( struct ThePool ** pool, uint32_t block_size, uint32_t page_count );
void pool_release ( struct ThePool * self );

uint32_t pool_page_blocks ( const struct PoolPage * self );

uint32_t pool_page_idx ( const struct PoolPage * self );

uint32_t pool_page_usage ( const struct PoolPage * self );

rc_t pool_page_find ( struct ThePool * self, struct PoolPage ** found, uint64_t pos );

rc_t pool_page_get ( const struct PoolPage * self, uint64_t pos, void *buffer,
                     size_t bsize, size_t *num_read );
                     
rc_t pool_page_find_new ( struct ThePool * self, struct PoolPage ** found );

rc_t pool_page_release ( struct PoolPage * self );

rc_t pool_page_prepare( struct PoolPage * self, uint32_t count, uint64_t pos );

rc_t pool_page_read_from_file( struct PoolPage * self, const struct KFile * f, size_t * read );

rc_t pool_page_write_to_file( const struct PoolPage * self, struct KFile * f, size_t to_write, size_t * written );

rc_t pool_page_write_to_recorder( const struct PoolPage * self, struct Recorder * rec );
