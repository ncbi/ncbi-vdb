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

#ifndef _SRA_DECOMPRESS_HEADER
#define _SRA_DECOMPRESS_HEADER

#ifdef __cplusplus
extern "C" {
#endif


/* sra compression types
 */
#define SRA_CT_UNCOMPRESSED      0
#define SRA_CT_HUFFMAN           1
#define SRA_CT_ZLIB              2
#define SRA_CT_NCBI2NA           3
#define SRA_CT_NORMALIZED        4
#define SRA_CT_BASELINED         5
#define SRA_CT_PACKED            6
#define SRA_CT_TRUNCATED         7
#define SRA_CT_EXP10             8
#define SRA_CT_TRANSLATE         9


/* forward of KDataBuffer */
struct KDataBuffer;


/* blob header pseudo descriptions
 *  NB - all structs are single-byte aligned
 *
struct HDR_454_POSITION
{
    uint8_t type;
    uint16_t osize; -- double compressed, 2 headers total
};

struct HDR_454_READ
{
    uint8_t type;
    uint16_t osize; -- 1 byte per base, i.e. row-length
};

struct HDR_454_QUALITY
{
    uint8_t type;
    uint16_t osize; -- double compressed
};

struct HDR_454_CLIP
{
    uint8_t type;
    uint32_t osize; -- zlib compressed
};

struct HDR_454_SIGNAL
{
    uint8_t type;
    uint16_t osize; -- double compressed
};

struct HDR_SLX_ZLIB
{
    uint8_t type;
    uint32_t osize; -- double compressed
};

struct HDR_SLX_QUALITY
{
    uint8_t type;
    uint32_t osize;
    uint8_t qmin;
    uint8_t qmax;
    uint16_t fixed_len;
};

struct HDR_SLX_READ
{
    uint8_t type;
    uint32_t osize;
    uint16_t fixed_len;
};

struct HDR_SLX_SIG_HUFFMAN
{
    uint8_t type;
    uint32_t osize;
    uint16_t fixed_len;
    float sim;
    float snm;
};

struct HDR_SLX_SIG_BASELINE -- or uncompressed
{
    uint8_t type;
    uint32_t osize;
    uint16_t fixed_len;
    uint32_t baseline;
};

struct HDR_SLX_SIG_NORMALIZED
{
    uint8_t type;
    uint32_t osize;
    uint16_t fixed_len;
    uint16_t min;
    float scale;
};

*/

/* decompression routines:
 * return a pointer to an allocated memory buffer and the size of the buffer in bytes
 * you would need to free(*dst) after its use
 *
 * parameters:
 *   platform = 1 (454), 2 (solexa)
 *   dst    - a pointer to a buffer into which an uncompressed data is written
 *   dsize  - the size of the incompressed data
 *   src    - compressed data source
 *   ssize  - size of compressed data
 *
 * return values: 0 - OK, 1 - error
 */
void sra_decompress_init  (void);

/* SIGNAL */
int sra_decompress_sig_454(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
int sra_decompress_sig_slx(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
/* INTENSITY */
int sra_decompress_int(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
/* NOISE */
int sra_decompress_nse(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
/* QUALITY */
int sra_decompress_prb_454(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
int sra_decompress_prb_slx(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
/* POSITION */
int sra_decompress_pos(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);
/* CLIP_QUALITY_ */
int sra_decompress_clp(struct KDataBuffer *dst, struct KDataBuffer *alt, const void* src, int ssize);

#ifdef __cplusplus
}
#endif

#endif
