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
#ifndef _h_sra_illumina_
#define _h_sra_illumina_

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
 * IlluminaReader
 */
typedef struct IlluminaReader IlluminaReader;

/* Make
 *  create IlluminaReader reference based
 *  on opened table and properties:
 *     table      - SRATable
 *     accession  - accession
 *     read       - output reads
 *     qual1      - output quality1
 *     qual4      - output quality4
 *     intensity  - output intensity
 *     noise      - output noise
 *     signal     - output signal
 *     qseq       - output QSEQ format
 *     minSpotId  - starting spot id, 0 - ignored (run information used)
 *     maxSpotId  - ending spot id, 0 - ignored (run information used)
 *                  you cannot seek out of range of [minSpotId:maxSpotId]
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderMake(const IlluminaReader** self, const SRATable* table, const char* accession,
                                         bool read, bool qual1, bool qual4,
                                         bool intensity, bool noise, bool signal, bool qseq,
                                         spotid_t minSpotId, spotid_t maxSpotId);
/* Whack
 *  releases object obtained from IlluminaReaderMake
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderWhack(const IlluminaReader* self);

/* FirstSpot
 *  set current spot to first in the run
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderFirstSpot(const IlluminaReader* self);

/* SeekSpot
 *  set current spot
 * if error occured current spot position becomes 0
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderSeekSpot(const IlluminaReader* self, spotid_t spot);

/* NextSpot
 *  Seek to next spot from current
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderNextSpot(const IlluminaReader* self);

/* CurrentSpot
 *  Get current spot
 *  sopt [OUT] - pointer to assign value of the current spot
 * returns GetRCState(rc) == rcExhausted when out of spots
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderCurrentSpot(const IlluminaReader* self, spotid_t *spot);

/* SpotInfo
 *  Get current spot information
 *  spotname [OUT] - pointer to assign value of the current spot name (can be NULL)
 *  spot_len [OUT] - pointer to assign value of the current spot length (can be NULL)
 *  num_reads[OUT] - pointer to assign value of the number of reads in current spot (can be NULL)
 */
SRA_RD_EXTERN rc_t CC IlluminaReader_SpotInfo(const IlluminaReader* self,
                                              const char** spotname, size_t* spotname_sz,
                                              INSDC_coord_val* lane, INSDC_coord_val* tile, 
                                              INSDC_coord_val* x, INSDC_coord_val* y,
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
SRA_RD_EXTERN rc_t CC IlluminaReader_SpotReadInfo(const IlluminaReader* self, uint32_t readId, SRAReadTypes* read_type,
                                                  const char** read_label, INSDC_coord_len* read_label_sz,
                                                  INSDC_coord_zero* read_start, INSDC_coord_len* read_len);

/* Base
 *  retrieve bases for the spot, result always has '\0' at the and (asciiz string)
 *  data   [IN] - pointer to buffer for printing
 *  dsize  [IN] - data buffer size
 *  written [IN,OUT] - optional number of bytes occupied by string
 *                     (not including the trailing '\0' used to end output), may by more than dsize
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderBase(const IlluminaReader* self, char* data, size_t dsize, size_t* written);

/* Noise
 *  retrieve noise in 4 floats per base format
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderNoise(const IlluminaReader* self, char* data, size_t dsize, size_t* written);

/* Intensity
 *  retrieve intensity in 4 floats per base format
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderIntensity(const IlluminaReader* self, char* data, size_t dsize, size_t* written);

/* Signal
 *  retrieve signal in 4 floats per base format
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderSignal(const IlluminaReader* self, char* data, size_t dsize, size_t* written);

/* Quality (BINARY)
 *  retrieve quality as single bytes per bases
 *  readid [IN] - 1-based, if <= 0 than the whole spot, otherwise particular read
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderQuality1(const IlluminaReader* self, uint32_t readId, char* data, size_t dsize, size_t* written);

/* Quality
 *  retrieve quality in 4 integers per base format
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderQuality4(const IlluminaReader* self, char* data, size_t dsize, size_t* written);

/* QSeq
 *  spot information as in QSEQ format
 *  other parameters see description for IlluminaReaderBase above
 */
SRA_RD_EXTERN rc_t CC IlluminaReaderQSeq(const IlluminaReader* self, uint32_t readId, bool spot_group,
                                         char* data, size_t dsize, size_t* written);

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_illumina_ */
