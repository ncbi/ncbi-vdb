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
#ifndef _h_sra_sff_file_
#define _h_sra_sff_file_

#include <klib/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================
 * SFF defines an 8 bit value in the file that tells of the format of the 
 * data signal (flowgrams in Roche 454 SFF parlance).
 *
 * The only currently defined format is a 16 bit unsigned integer in
 * units one hundredths. This enum is to easily allow us to add other
 * formats if ever required.
 */
typedef enum SFFFormatCode
{
    SFFFormatCodeUnset = 0,
    /* values are 16 integers of hundreths of units: 0 = 0.00, 1 = 0.01, 2 = 0.02, ... */
    SFFFormatCodeUI16Hundreths,
    /* currently (SFF (00000001) yet this is the only one SFFFormatCode is defined */
    SFFFormatCodeUndefined
}	SFFFormatCode;

/* ----------------------------------------------------------------------
 * Common Header Section 
 * (Genome Sequencer Data Analysis Software Manual Section 13.3.8.1)
 */
#define SFFCommonHeader_size 31

typedef struct SFFCommonHeader_struct
{
    uint32_t magic_number;         /* four bytes ".sff" as string: with wrong endian it would be "ffs." */
    uint32_t version;              /* four bytes 0x00000001 */
    uint64_t index_offset;         /* index_offset and index_length are the offset and length of an */
    uint32_t index_length;         /* optional index of the reads in the file. If no index both are 0 */
    uint32_t number_of_reads;      /* The number of reads in the file (not individual datum) */
    uint16_t header_length;        /* length of all headers in this set.  31 + flow_length + key_length + pad to 8 byte boundary */
    uint16_t key_length;           /* length of the key sequence for these reads */
    uint16_t num_flows_per_read;   /* the number of flows for each read in this file */
    uint8_t  flowgram_format_code; /* SFFFormatCode between (SFFFormatCodeUnset..FormateCodeUndefined) exclusive */
    /* not included variable length portion of header:
        flow chars   - sequence of uint8_t, actual length is num_flows_per_read above
        key sequence - sequence of uint8_t, actual length is key_length above
        padding      - sequence of zeroed uint8_t to make total length of file header 8-byte padded
    */
} SFFCommonHeader;

/* ----------------------------------------------------------------------
 * Read Header Section 
 * (Genome Sequencer Data Analysis Software Manual Section 13.3.8.2)
 */
#define SFFReadHeader_size 16

typedef struct SFFReadHeader_struct
{
    uint16_t    header_length;            /* length in bytes of the full section including padding */
    uint16_t    name_length;            /* length of the name of this spot */
    uint32_t    number_of_bases;
    uint16_t    clip_quality_left;
    uint16_t    clip_quality_right;
    uint16_t    clip_adapter_left;
    uint16_t    clip_adapter_right;
    /* not included variable length portion of header:
        name    - sequence of uint8_t, actual length is name_length above
        padding - sequence of zeroed uint8_t to make total length of read header 8-byte padded

        read data section:

        signal - sequence of uint16_t (if flowgram_format_code == SFFFormatCodeUI16Hundreths, see enum above),
                 actual length is num_flows_per_read from file header above
        flow_index_per_base (position) - sequence of uint8_t, actual length in number_of_bases above
        bases - sequence of uint8_t, actual length in number_of_bases above
        quality_scores - sequence of uint8_t, actual length in number_of_bases above
        padding - sequence of zeroed uint8_t to make total length of read data section 8-byte padded
    */
} SFFReadHeader;

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_sff_file_ */
