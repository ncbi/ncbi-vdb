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
#ifndef _h_sra_fastq_
#define _h_sra_fastq_

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
 * FastqReader
 */
typedef struct FastqReader FastqReader;

/* Make
 *  create FastqReader reference based
 *  on opened table and properties:
 *     table      - SRATable
 *     accession  - accession
 *     minReadLen - minimal length of the outputed read, 0 - no minimum (FUSE)
 *     colorSpace - color space sequence
 *     origFormat - excludes SRR accession & length on defline
 *     fasta      - prepare only reads, no qualities
 *     readLabel  - prints read label on the name line
 *     readId     - prints .read in spot id on the name line (not in origFormat)
 *     noclip     - don't clip quality left and right
 *     offset     - offset to use for quality conversion (default is 33)
 *     csKey      - desired color space key, '\0' - do not convert
 *     minSpotId  - starting spot id, 0 - ignored (run information used)
 *     maxSpotId  - ending spot id, 0 - ignored (run information used)
 *                  you cannot seek out of range of [minSpotId:maxSpotId]
 */
SRA_RD_EXTERN rc_t CC FastqReaderMake(const FastqReader** self, const SRATable* table, const char* accession,
                                      bool colorSpace, bool origFormat, bool fasta,
                                      bool printLabel, bool printReadId,
                                      bool noClip, bool SuppressQualForCSKey, uint32_t minReadLen,
                                      char offset, char csKey,
                                      spotid_t minSpotId, spotid_t maxSpotId);

/* Whack
 *  releases object obtained from FastqReaderMake
 */
SRA_RD_EXTERN rc_t CC FastqReaderWhack(const FastqReader* self);

/* FirstSpot
 *  set current spot to first in the run
 */
SRA_RD_EXTERN rc_t CC FastqReaderFirstSpot(const FastqReader* self);

/* SeekSpot
 *  set current spot
 * if error occured current spot position becomes 0
 */
SRA_RD_EXTERN rc_t CC FastqReaderSeekSpot(const FastqReader* self, spotid_t spot);

/* NextSpot
 *  Seek to next spot from current
 */
SRA_RD_EXTERN rc_t CC FastqReaderNextSpot(const FastqReader* self);

/* CurrentSpot
 *  Get current spot
 *  sopt [OUT] - pointer to assign value of the current spot
 * returns GetRCState(rc) == rcExhausted when out of spots
 */
SRA_RD_EXTERN rc_t CC FastqReaderCurrentSpot(const FastqReader* self, spotid_t* spot);

/* SpotInfo
 *  Get current spot information
 *  spotname [OUT] - pointer to assign value of the current spot name (can be NULL)
 *  spotname_sz [OUT] - pointer to assign value of the length of the spot name (can be NULL)
 *  spotgroup [OUT] - pointer to assign value of the current spot group (can be NULL)
 *  spotgroup_sz [OUT] - pointer to assign value of the length of the spot group (can be NULL)
 *  spot_len [OUT] - pointer to assign value of the current spot length (can be NULL)
 *  num_reads[OUT] - pointer to assign value of the number of reads in current spot (can be NULL)
 */
SRA_RD_EXTERN rc_t CC FastqReader_SpotInfo(const FastqReader* self,
                                           const char** spotname, size_t* spotname_sz,
                                           const char** spotgroup, size_t* spotgroup_sz,
                                           uint32_t* spot_len, uint32_t* num_reads);

/* SpotReadInfo
 *  Get read information for current spot
 *  readid    [IN]  - 1-based read id
 *  read_type [OUT] - pointer to assign value of the read type (can be NULL)
 *  read_label[OUT] - pointer to assign value of the read label (can be NULL)
 *  read_label_sz [OUT] - pointer to assign value of the length of the read label (can be NULL)
 *  read_start[OUT] - pointer to assign value of the read start in spot (can be NULL)
 *  read_len  [OUT] - pointer to assign value of the read length (can be NULL)
 *  cskey     [OUT] - pointer to assign value of the read color space key (can be NULL)
 *  if start and len is == 0 read is empty
 */
SRA_RD_EXTERN rc_t CC FastqReader_SpotReadInfo(const FastqReader* self, uint32_t readId, SRAReadTypes* read_type, 
                                               const char** read_label, INSDC_coord_len* read_label_sz,
                                               INSDC_coord_zero* read_start, INSDC_coord_len* read_len);

/* BaseName
 *  retrieve name string for the spot, result always has '\0' at the and (asciiz string)
 *  readid [IN] - 1-based, if <= 0 than the whole spot, otherwise particular read
 *  label  [IN] - overrides read label appending (printLabel), set to NULL by default
 *  data   [IN] - pointer to buffer for printing
 *  dsize  [IN] - data buffer size
 *  written [IN,OUT] - optional number of bytes occupied by string
 *                     (not including the trailing '\0' used to end output), may by more than dsize
 */
SRA_RD_EXTERN rc_t CC FastqReaderBaseName(const FastqReader* self, uint32_t readId,
                                          bool* label, char* data, size_t dsize, size_t* written);

/* Base
 *  retrieve bases for the spot
 *  other parameters see description for FastqReaderBaseName above
 */
SRA_RD_EXTERN rc_t CC FastqReaderBase(const FastqReader* self, uint32_t readId,
                                      char* data, size_t dsize, size_t* written);

/* QualityName
 *  retrieve quality name string for the spot
 *  other parameters see description for FastqReaderBaseName above
 */
SRA_RD_EXTERN rc_t CC FastqReaderQualityName(const FastqReader* self, uint32_t readId,
                                             bool* label, char* data, size_t dsize, size_t* written);

/* Quality
 *  retrieve quality string for the spot
 *  other parameters see description for FastqReaderBaseName above
 */
SRA_RD_EXTERN rc_t CC FastqReaderQuality(const FastqReader* self, uint32_t readId,
                                         char* data, size_t dsize, size_t* written);

/* GetCurrentSpotData
 *  retrieve current spot data as a blob
 *  other parameters see description for FastqReaderBaseName above
 */
SRA_RD_EXTERN rc_t CC FastqReader_GetCurrentSpotData(const FastqReader* self,
                                                     char* data, size_t dsize, size_t* written);

/* GetCurrentSpotSplitData
 * same as above but split spot into line with individual reads
 */
SRA_RD_EXTERN rc_t CC FastqReader_GetCurrentSpotSplitData(const FastqReader* self,
                                                          char* data, size_t dsize, size_t* written);

/* GetNextSpotData (streaming function)
 *  move to the next spot and retrieve current spot data as a blob
 *  other parameters see description for FastqReaderBaseName above
 */
SRA_RD_EXTERN rc_t CC FastqReader_GetNextSpotData(const FastqReader* self,
                                                  char* data, size_t dsize, size_t* written);

/* GetNextSpotSplitData (streaming function)
 * same as above but split spot into line with individual reads
 */
SRA_RD_EXTERN rc_t CC FastqReader_GetNextSpotSplitData(const FastqReader* self,
                                                       char* data, size_t dsize, size_t* written);

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_fastq_ */
