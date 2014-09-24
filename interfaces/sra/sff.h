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
#ifndef _h_sra_sff_
#define _h_sra_sff_

#ifndef _h_sra_rd_extern_
#include <sra/rd-extern.h>
#endif

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * SFFReader
 */
typedef struct SFFReader SFFReader;

/* Make
 *  create SFFReader reference based
 *  on table and properties:
 *     table      - SRATable
 *     accession  - accession
 *     minSpotId  - starting spot id, 0 - ignored (run information used)
 *     maxSpotId  - ending spot id, 0 - ignored (run information used)
 *                  you cannot seek out of range of [minSpotId:maxSpotId]
 */
SRA_RD_EXTERN rc_t CC SFFReaderMake(const SFFReader** self, const SRATable* table, const char* accession,
                                    spotid_t minSpotId, spotid_t maxSpotId);

/* Whack
 *  releases object obtained from SFFReaderMake
 */
SRA_RD_EXTERN rc_t CC SFFReaderWhack(const SFFReader* self);

/* FirstSpot
 *  set current spot to first in the run
 */
SRA_RD_EXTERN rc_t CC SFFReaderFirstSpot(const SFFReader* self);

/* SeekSpot
 *  set current spot
 * if error occured current spot position becomes 0
 */
SRA_RD_EXTERN rc_t CC SFFReaderSeekSpot(const SFFReader* self, spotid_t spot);

/* NextSpot
 *  Seek to next spot from current
 */
SRA_RD_EXTERN rc_t CC SFFReaderNextSpot(const SFFReader* self);

/* CurrentSpot
 *  Get current spot
 *  sopt [OUT] - pointer to assign value of the current spot
 * returns GetRCState(rc) == rcExhausted when out of spots
 */
SRA_RD_EXTERN rc_t CC SFFReaderCurrentSpot(const SFFReader* self, spotid_t* spot);

/* SpotInfo
 *  Get current spot information
 *  spotname [OUT] - pointer to assign value of the current spot name (can be NULL)
 *  spot_len [OUT] - pointer to assign value of the current spot length (can be NULL)
 *  num_reads[OUT] - pointer to assign value of the number of reads in current spot (can be NULL)
 */
SRA_RD_EXTERN rc_t CC SFFReader_SpotInfo(const SFFReader* self, const char** spotname, size_t* spotname_sz,
                                         uint32_t* spot_len, uint32_t* num_reads);

/* SpotReadInfo
 *  Get read information for current spot
 *  readid    [IN]  - 1-based read id
 *  read_label[OUT] - pointer to assign value of the read label (can be NULL)
 *  read_type [OUT] - pointer to assign value of the read type (can be NULL)
 *  read_start[OUT] - pointer to assign value of the read start in spot (can be NULL)
 *  read_len  [OUT] - pointer to assign value of the read length (can be NULL)
 *  cskey     [OUT] - pointer to assign value of the read color space key (can be NULL)
 *  if start and len is == 0 read is empty
 */
SRA_RD_EXTERN rc_t CC SFFReader_SpotReadInfo(const SFFReader* self, uint32_t readId, SRAReadTypes* read_type,
                                             const char** read_label, INSDC_coord_len* read_label_sz,
                                             INSDC_coord_zero* read_start, INSDC_coord_len* read_len);

/* Header
 *  retrieve binary file header
 *  spots [IN,OPT]   - sets number of reads in header, ignored if spots == 0
 *  data [IN]        - pointer to buffer for printing
 *  dsize [IN]       - data buffer size
 *  written [IN,OUT] - number of bytes occupied by data, may by more than dsize
 */
SRA_RD_EXTERN rc_t CC SFFReaderHeader(const SFFReader* self, spotid_t spots, char* data, size_t dsize, size_t* written);

/* Base
 *  retrieve bases for the spot
 *  other parameters see description for SFFReaderHeader above
 */
SRA_RD_EXTERN rc_t CC SFFReaderReadHeader(const SFFReader* self, char* data, size_t dsize, size_t* written);

/* QualityName
 *  retrieve quality name string for the spot
 *  other parameters see description for SFFReaderHeader above
 */
SRA_RD_EXTERN rc_t CC SFFReaderReadData(const SFFReader* self, char* data, size_t dsize, size_t* written);

/* GetCurrentSpotData
 *  retrieve current spot data as a blob
 *  other parameters see description for SFFReaderHeader above
 */
SRA_RD_EXTERN rc_t CC SFFReader_GetCurrentSpotData(const SFFReader* self, char* data, size_t dsize, size_t* written);

/* GetNextSpotData (streaming function)
 *  move to the next spot and retrieve current spot data as a blob
 *  other parameters see description for SFFReaderHeader above
 */ 
SRA_RD_EXTERN rc_t CC SFFReader_GetNextSpotData(const SFFReader* self, char* data, size_t dsize, size_t* written);

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_sff_ */
