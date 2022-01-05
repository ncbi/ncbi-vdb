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

#ifndef _h_samextract_priv_
#define _h_samextract_priv_
#include <align/samextract-lib.h>
#include <kfs/buffile.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/vector.h>
#include <sys/types.h>
#include <zlib.h>

// #define READBUF_SZ 65536

#ifdef __cplusplus
extern "C" {
#endif

bool inrange(const char* str, i64 low, i64 high);
bool ismd5(const char* str);
bool isfloworder(const char* str);
bool filter(const SAMExtractor* state, String* srname, ssize_t pos);
void decode_seq(const u8* seqbytes, size_t l_seq, char* seq);
void decode_qual(const u8* qualbytes, size_t l_seq, char* qual);
size_t fast_u32toa(char* buf, u32 val);
size_t fast_i32toa(char* buf, i32 val);
i64 fast_strtoi64(const char* p);
char* decode_cigar(u32* cigar, u16 n_cigar_op);

bool check_cigar(const char* cigar, const char* seq);

#ifdef __cplusplus
}
#endif
#endif
