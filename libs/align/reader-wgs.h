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

#ifdef __cplusplus
extern "C" {
#endif

struct VDBManager;
struct VTable;

typedef struct TableReaderWGS TableReaderWGS;

rc_t TableReaderWGS_MakeTable(const TableReaderWGS ** cself,
                              const struct VDBManager* vmgr,
                              const struct VTable* table,
                              uint32_t options, size_t cache);

void TableReaderWGS_Whack(const TableReaderWGS* cself);

rc_t TableReaderWGS_SeqLength(const TableReaderWGS* cself, int64_t row, INSDC_coord_len* len);

rc_t TableReaderWGS_Circular(const TableReaderWGS* cself, int64_t row, bool* circular);

/* *md5 is NULL if not present */
rc_t CC TableReaderWGS_MD5(const TableReaderWGS* cself, int64_t row, const uint8_t** md5);

/* read a chunk of refseq into buffer from offset up to offset + len
   if offset is beyond non-circular refseq size error is returned
 */
rc_t CC TableReaderWGS_Read(const TableReaderWGS* cself, int64_t row,
                            INSDC_coord_zero offset, INSDC_coord_len len,
                            uint8_t* buffer, INSDC_coord_len* written);
#ifdef __cplusplus
}
#endif
