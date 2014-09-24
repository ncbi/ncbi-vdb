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
#ifndef _h_sra_reader_
#define _h_sra_reader_

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif

#define SRAREADER_COL_MANDATORY 0x00
#define SRAREADER_COL_OPTIONAL 0x01

typedef struct SRAReaderColumn_struct {
    uint32_t opt;
    const char* name;
    const char* datatype;
    const SRAColumn* col;
    const void* base;
    bitsz_t size;
} SRAReaderColumn;

typedef struct SRAReader_struct {
    char* accession;
    const SRATable* table;
    spotid_t minSpotId;
    spotid_t maxSpotId;
    uint32_t options;
    SRAReaderColumn cols[64]; /* max of 64 columns, must be enough, last one must have NULL name */
    /* current spot data */
    spotid_t spot;
    const SRAReaderColumn* spot_name;
    const INSDC_coord_len** spot_len;
    const INSDC_coord_zero** read_start;
    const INSDC_coord_len** read_len;
    const bitsz_t* read_len_size;
    const SRAReadTypes** read_type;
    const char** label;
    const INSDC_coord_zero** label_start;
    const INSDC_coord_len** label_len;
} SRAReader;

/* Common macro for self pointer and current spot state validation */
#define CHECK_SELF(ObjType) \
                   ObjType* me = (ObjType*)self; \
                   if(self == NULL) { \
                       return RC(rcSRA, rcFormatter, rcAccessing, rcSelf, rcNull); \
                   }

#define CHECK_SPOT(Obj) \
                   if( (Obj).spot < (Obj).minSpotId ) { \
                       return RC(rcSRA, rcFormatter, rcAccessing, rcRow, rcUnknown); \
                   } \
                   if( (Obj).spot > (Obj).maxSpotId ) { \
                       return RC(rcSRA, rcFormatter, rcAccessing, rcRow, rcExhausted); \
                   }

rc_t SRAReaderMake(const SRAReader** self, size_t size, const SRATable* table,
                   const char* accession, spotid_t minSpotId, spotid_t maxSpotId);

rc_t SRAReaderInit(const SRAReader* self, int options, const SRAReaderColumn* const cols);

/* Whack
 *  releases object obtained from SRAReaderMake
 *  or AddRef ( see above )
 */
rc_t SRAReaderWhack(const SRAReader *self);

/* FindColData
 * find OPENED column by it's definition and return it's base pointer or col struct
 */
rc_t SRAReader_FindColData(const SRAReader* self, const SRAReaderColumn* definition, const SRAReaderColumn** col, const void*** base);

/* FirstSpot
 *  set current spot to first in the run
 */
rc_t SRAReaderFirstSpot(const SRAReader* self);

/* SeekSpot
 *  set current spot
 * if error occured current spot position becomes 0
 */
rc_t SRAReaderSeekSpot(const SRAReader* self, spotid_t spot);

/* NextSpot
 *  Seek to next spot from current
 */
rc_t SRAReaderNextSpot(const SRAReader* self);

/* CurrentSpot
 *  Get current spot
 *  sopt [OUT] - pointer to assign value of the current spot
 * returns GetRCState(rc) == rcExhausted when out of spots
 */
rc_t SRAReaderCurrentSpot(const SRAReader* self, spotid_t* spot);

/* SpotInfo
 *  Get current spot information
 *  spotname [OUT] - pointer to assign value of the current spot name (can be NULL)
 *  spotname_sz [OUT] - pointer to assign value of the length of the spot name (can be NULL)
 *  spot_len [OUT] - pointer to assign value of the current spot length (can be NULL)
 *  num_reads[OUT] - pointer to assign value of the number of reads in current spot (can be NULL)
 */
rc_t SRAReader_SpotInfo(const SRAReader* self, const char** spotname, size_t* spotname_sz,
                                               uint32_t* spot_len, uint32_t* num_reads);

/* SpotReadInfo
 *  Get read information for current spot
 *  readid    [IN]  - 1-based read id
 *  read_type [OUT] - pointer to assign value of the read type (can be NULL)
 *  read_label[OUT] - pointer to assign value of the read label (can be NULL)
 *  read_label_sz [OUT] - pointer to assign value of the length of the read label (can be NULL)
 *  read_start[OUT] - pointer to assign value of the read start in spot (can be NULL)
 *  read_len  [OUT] - pointer to assign value of the read length (can be NULL)
 *  if start and len is == 0 read is empty
 */
rc_t SRAReader_SpotReadInfo(const SRAReader* self, uint32_t readId, SRAReadTypes* read_type,
                            const char** read_label, INSDC_coord_len* read_label_sz,
                            INSDC_coord_zero* read_start, INSDC_coord_len* read_len);

/* GetCurrentSpotData
 *  retrieve current spot data as a blob
 *  data   [IN] - pointer to buffer for printing
 *  dsize  [IN] - data buffer size
 *  written [IN,OUT] - optional number of bytes occupied by buffer, may by more than dsize
 *  MUST BE SEPARATELY DEFINED for each reader format
 */
rc_t SRAReader_GetCurrentSpotData(const SRAReader* self, char *data, size_t dsize, size_t* written);

/* GetNextSpotData (streaming function)
 *  move to the next spot and retrieve current spot data as a blob
 *  data   [IN] - pointer to buffer for printing
 *  dsize  [IN] - data buffer size
 *  written [IN,OUT] - optional number of bytes occupied by buffer, may by more than dsize
 *  MUST BE SEPARATELY DEFINED for each reader format
 */
rc_t SRAReader_GetNextSpotData(const SRAReader* self, char *data, size_t dsize, size_t* written);

#endif /* _h_sra_reader_ */
