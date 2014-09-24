/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was readten as part of
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
#ifndef _h_align_reader_refseq_
#define _h_align_reader_refseq_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct VDBManager;
struct VTable;

enum ETableReaderRefSeq_Options {
    errefseq_4NA = 0x01 /* read in INSDC:4na:bin, INSDC:dna:text is default */
};

typedef struct TableReaderRefSeq TableReaderRefSeq;

ALIGN_EXTERN rc_t CC TableReaderRefSeq_MakeTable(const TableReaderRefSeq** cself, const struct VDBManager* vmgr,
                                                 const struct VTable* table, uint32_t options, size_t cache);

ALIGN_EXTERN rc_t CC TableReaderRefSeq_MakePath(const TableReaderRefSeq** cself, const VDBManager* vmgr,
                                                const char* path, uint32_t options, size_t cache);

ALIGN_EXTERN void CC TableReaderRefSeq_Whack(const TableReaderRefSeq* cself);

ALIGN_EXTERN rc_t CC TableReaderRefSeq_SeqId(const TableReaderRefSeq* cself, const char** id, uint32_t* id_sz);

ALIGN_EXTERN rc_t CC TableReaderRefSeq_SeqLength(const TableReaderRefSeq* cself, INSDC_coord_len* len);

ALIGN_EXTERN rc_t CC TableReaderRefSeq_Circular(const TableReaderRefSeq* cself, bool* circular);

/* *md5 is NULL if not present */
ALIGN_EXTERN rc_t CC TableReaderRefSeq_MD5(const TableReaderRefSeq* cself, const uint8_t** md5);

/* read a chunk of refseq into buffer from offset up to offset + len
   if offset is beyond non-circular refseq size error is returned
 */
ALIGN_EXTERN rc_t CC TableReaderRefSeq_Read(const TableReaderRefSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                            uint8_t* buffer, INSDC_coord_len* written);
#ifdef __cplusplus
}
#endif

#endif /* _h_align_reader_refseq_ */
