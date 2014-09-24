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

#ifndef _SRA_DECOMPRESS_LOCAL_HEADER
#define _SRA_DECOMPRESS_LOCAL_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define SRA_DECOMPRESS_USE_REVERSE_LOOKUP 1

#include <assert.h>
#include <byteswap.h>
#include <limits.h>
#include <math.h> 
/*#include <netinet/in.h>*/
#include <search.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <unistd.h>*/
#include <zlib.h>
#include <stdint.h>

#define SIG_HUFF_TREESIZE         512
#define INT_HUFF_TREESIZE         512
#define NSE_HUFF_TREESIZE         512
#define PRB_SLX_HUFF_TREESIZE       7
#define PRB_SLX_SPAN               81
#define PRB_SLX_MIN                40
 
#define PRB_454_HUFF_TREESIZE     100
#define POS_454_HUFF_TREESIZE      64

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
#define REV_HUFF_TREESIZE_16    65536
#define REV_HUFF_TREESIZE_08      256
#endif

#define SIG_MID_POINT            (SIG_HUFF_TREESIZE/2-1)
#define SIG_MAX_POINT            (SIG_HUFF_TREESIZE-1)
#define INT_MID_POINT            (INT_HUFF_TREESIZE/2-1)
#define INT_MAX_POINT            (INT_HUFF_TREESIZE-1)
#define NSE_MID_POINT            (NSE_HUFF_TREESIZE/2-1)
#define NSE_MAX_POINT            (NSE_HUFF_TREESIZE-1)
#define PRB_SLX_MAX_POINT        (PRB_SLX_HUFF_TREESIZE-1)
#define PRB_454_MAX_POINT        (PRB_454_HUFF_TREESIZE-1)
#define POS_454_MAX_POINT        (POS_454_HUFF_TREESIZE-1) 

#define LOG_MAIN_MULTIPLIER     2048
#define LOG_SUB_MULTIPLIER       256

#ifndef _UCHAR_DEFINED
#define _UCHAR_DEFINED
typedef unsigned char  uchar_t;
#endif

/* clearing malloc
 *  NB - in all cases, clearing memory before use
 *  is unnecessary, since the buffers are allocated
 *  to exact sizes and completely overwritten.
 */
void* sra_zalloc (unsigned int);
#define sra_zalloc( bytes ) \
    malloc ( bytes )

/*
 */
struct SRALookup;
struct SRALocal;

typedef struct SRALookup* SRALookup_t;
typedef struct SRALocal*  SRALocal_t;

/* huffman lookup table
 */
typedef struct SRALookup
{
  uchar_t   nbits; /* number of bits for an entry */
  uint32_t  bits;  /* actual bits */
} SRALookup;

/* local data storage
 */
typedef struct SRALocal
{
  SRALookup    lookup_slx_sig [SIG_HUFF_TREESIZE];
  SRALookup    lookup_slx_int [INT_HUFF_TREESIZE];
  SRALookup    lookup_slx_nse [NSE_HUFF_TREESIZE];
  SRALookup    lookup_slx_prb [PRB_SLX_HUFF_TREESIZE];

  SRALookup    lookup_454_sig [SIG_HUFF_TREESIZE];
  SRALookup    lookup_454_prb [PRB_454_HUFF_TREESIZE];
  SRALookup    lookup_454_pos [POS_454_HUFF_TREESIZE];

  uint16_t     idx_454_sig [SIG_HUFF_TREESIZE];
  uint16_t     idx_454_prb [PRB_454_HUFF_TREESIZE];
  uint16_t     idx_454_pos [POS_454_HUFF_TREESIZE];

  uint16_t     idx_slx_sig [SIG_HUFF_TREESIZE];
  uint16_t     idx_slx_int [INT_HUFF_TREESIZE];
  uint16_t     idx_slx_nse [NSE_HUFF_TREESIZE];
  uint16_t     idx_slx_prb [PRB_SLX_HUFF_TREESIZE];

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t     rlookup_slx_sig [REV_HUFF_TREESIZE_16];
  uint16_t     rlookup_slx_int [REV_HUFF_TREESIZE_16];
  uint16_t     rlookup_slx_nse [REV_HUFF_TREESIZE_16];
  uchar_t      rlookup_slx_prb [REV_HUFF_TREESIZE_08];

  uint16_t     rlookup_454_sig [REV_HUFF_TREESIZE_16];
  uint16_t     rlookup_454_prb [REV_HUFF_TREESIZE_16];
#endif

  /* sequence encoding/decoding tables
   */
  uchar_t      conv2to4na [256][4];  /* backward conversion to nucleotides */
  uchar_t      conv2to4cs [256][4];  /* backward conversion to color space */
  uchar_t      convqs4to1 [PRB_SLX_SPAN];

  int          lookup_slx_sig_initialized;
  int          lookup_slx_int_initialized;
  int          lookup_slx_nse_initialized;
  int          lookup_slx_prb_initialized;

  int          lookup_454_sig_initialized;
  int          lookup_454_prb_initialized;
  int          lookup_454_pos_initialized;
  int          lookup_454_seq_initialized;
} SRALocal;


/* some useful macros
 *
 */
#define _put_short(data, dst, shift, bitpos) \
  do { \
    unsigned int   _shift =*(shift); \
    unsigned int   _bitpos= (bitpos); \
    unsigned char* _dst   = (unsigned char*)(dst); \
    unsigned char  _byte; \
    if(_bitpos) \
    { \
      _byte = (unsigned char)((data) >> 8); \
      *(_dst+_shift)   |= (_byte >> _bitpos); \
      *(_dst+_shift+1) |= (_byte << (8-_bitpos)); \
      _byte = (unsigned char)((data) & 0xFF); \
      *(_dst+_shift+1) |= (_byte >> _bitpos); \
      *(_dst+_shift+2) |= (_byte << (8-_bitpos)); \
    } \
    else \
    { \
      _byte = (unsigned char)((data) >> 8); \
      *(_dst+_shift)   = _byte; \
      _byte = (unsigned char)((data) & 0xFF); \
      *(_dst+_shift+1) = _byte; \
    } \
    *(shift)=_shift+2; \
  } while(0)
/*      
 */   
#define _get_byte(src, shift, bitpos, byte, update) \
  do { \
    unsigned int _shift=*(shift); \
    *(byte) = 0; \
    if(bitpos) \
    { \
      *(byte)  = *((src)+_shift)<<(bitpos); \
      *(byte) |= *((src)+_shift+1)>>(8-(bitpos)); \
    } \
    else *(byte) = *((src)+_shift); \
    if(update) *(shift)=_shift+1; \
  } while(0)

/*
 */
#define _get_short(src, shift, bitpos, data, update) \
  do { \
    unsigned int   _shift=*(shift); \
    unsigned short _data =0; \
    if(bitpos) \
    { \
      _data  = ((*((src)+_shift)<<(bitpos))<<8); \
      _data |= ((*((src)+_shift+1)>>(8-(bitpos)))<<8); \
      _data |= ((*((src)+_shift+1)<<(bitpos))); \
      _data |= ((*((src)+_shift+2)>>(8-(bitpos)))); \
    } \
    else \
    { \
      _data  = (*((src)+_shift)<<8); \
      _data |= (*((src)+_shift+1) & 0xFF); \
    } \
    *(data) = _data; \
    if(update) *(shift)=_shift+2; \
  } while(0)
    
/*
 */
#if _DEBUGGING
#define _get_long(src, nbits, shift, bitpos, ldata, update, max_bits)	\
    do {								\
        unsigned int   _shift =*(shift);				\
        unsigned int   _bitpos=*(bitpos);				\
        unsigned char* _src=(unsigned char*)(src);                      \
        uint32_t _l = 0;						\
        int _bits_done = 0;						\
        int _nbits = nbits;						\
                                                                        \
        if (_bitpos + _nbits + (_shift << 3) > max_bits) {              \
            fprintf(stderr, "legacy code trying to read %u bits beyond end of data at %s, line %d.\n", _bitpos + _nbits + (_shift << 3) - max_bits, __FILE__, __LINE__); \
            _nbits = max_bits - (_bitpos + (_shift << 3));              \
        }                                                               \
        while (_nbits > 0) {                                            \
            unsigned _bits;						\
            int _bitsl;                                                 \
                                                                        \
            _bits = _src[_shift];					\
            _bitsl = 8;                                                 \
                                                                        \
            if (_bitpos) {                                              \
                _bits &= 0xFFu >> _bitpos;                              \
                _bitsl = 8 - _bitpos;                                   \
            }                                                           \
                                                                        \
            if (_bitsl > _nbits) {                                      \
                _bits >>= _bitsl - _nbits;                              \
                _bitsl = _nbits;                                        \
            }                                                           \
                                                                        \
            _bitpos += _bitsl;                                          \
                                                                        \
            if (_bitpos == 8) {                                         \
                _bitpos = 0;                                            \
                _shift++;                                               \
            }                                                           \
                                                                        \
            _l |= _bits << (32 - (_bits_done + _bitsl));		\
                                                                        \
            _bits_done += _bitsl;					\
                                                                        \
            _nbits -= _bitsl;                                           \
        }								\
                                                                        \
        *ldata = _l;                                                    \
                                                                        \
        if(update) {                                                    \
            *(shift) =_shift;                                           \
            *(bitpos)=_bitpos;                                          \
        }								\
    } while(0)
#else
#define _get_long(src, nbits, shift, bitpos, ldata, update, max_bits)	\
    do {								\
        unsigned int   _shift =*(shift);				\
        unsigned int   _bitpos=*(bitpos);				\
        unsigned char* _src=(unsigned char*)(src);                      \
        uint32_t _l = 0;						\
        int _bits_done = 0;						\
        int _nbits = nbits;						\
                                                                        \
        if (_bitpos + _nbits + (_shift << 3) > max_bits) {              \
            _nbits = max_bits - (_bitpos + (_shift << 3));              \
        }                                                               \
        while (_nbits > 0) {                                            \
            unsigned _bits;						\
            int _bitsl;                                                 \
                                                                        \
            _bits = _src[_shift];					\
            _bitsl = 8;                                                 \
                                                                        \
            if (_bitpos) {                                              \
                _bits &= 0xFFu >> _bitpos;                              \
                _bitsl = 8 - _bitpos;                                   \
            }                                                           \
                                                                        \
            if (_bitsl > _nbits) {                                      \
                _bits >>= _bitsl - _nbits;                              \
                _bitsl = _nbits;                                        \
            }                                                           \
                                                                        \
            _bitpos += _bitsl;                                          \
                                                                        \
            if (_bitpos == 8) {                                         \
                _bitpos = 0;                                            \
                _shift++;                                               \
            }                                                           \
                                                                        \
            _l |= _bits << (32 - (_bits_done + _bitsl));		\
                                                                        \
            _bits_done += _bitsl;					\
                                                                        \
            _nbits -= _bitsl;                                           \
        }								\
                                                                        \
        *ldata = _l;                                                    \
                                                                        \
        if(update) {                                                    \
            *(shift) =_shift;                                           \
            *(bitpos)=_bitpos;                                          \
        }								\
    } while(0)
#endif

/* various rotation routines
 */
#define _rotate_1(type,array) \
  do { \
    type* _ptr = (type*)(array); \
    type  _val = *(_ptr+0); \
    *(_ptr+0) = *(_ptr+1); *(_ptr+1) = *(_ptr+2); \
    *(_ptr+2) = *(_ptr+3); *(_ptr+3) = _val; \
  } while(0)
/*
 */
#define _rotate_2(type,array) \
  do { \
    type* _ptr = (type*)(array); \
    type  _val = *(_ptr+0); \
    *(_ptr+0) = *(_ptr+2); *(_ptr+2) = _val; \
    _val = *(_ptr+1); \
    *(_ptr+1) = *(_ptr+3); *(_ptr+3) = _val; \
  } while(0)
/*
 */
#define _rotate_3(type,array) \
  do { \
    type* _ptr = (type*)(array); \
    type  _val = *(_ptr+3); \
    *(_ptr+3) = *(_ptr+2); *(_ptr+2) = *(_ptr+1); \
    *(_ptr+1) = *(_ptr+0); *(_ptr+0) = _val; \
  } while(0)

/*
 */
#define _cmpbits(src1, src2, size, result) \
  do { \
    unsigned int _mask=0xFFFFFFFFL<<(32-(size)); \
    if (((unsigned int)(src1)&_mask)^((unsigned int)(src2)&_mask)) *(result)=0; else *(result)=1; \
  } while(0)

#ifdef __cplusplus
}
#endif
#endif
