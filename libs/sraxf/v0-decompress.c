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
#include <vdb/extern.h>

#include "v0-decompress.h"
#include "v0-decompress-local.h"
#include <klib/data-buffer.h>
#include <klib/sort.h>
#include <sysalloc.h>

#include <math.h>

/* local data
 */
SRALocal     gv_local;

static
void *sra_alloc ( KDataBuffer *dst, KDataBuffer *alt, size_t bytes )
{
    rc_t rc;

    /* whatever was in dst, put it in alt */
    KDataBufferWhack ( alt );
#if 0
    *alt = *dst;
#else
    KDataBufferSub ( dst, alt, 0, UINT64_MAX );
    KDataBufferWhack(dst);
#endif

    /* allocate a new space */
    rc = (uint32_t)KDataBufferMakeBytes ( dst, (uint32_t)bytes );
    if ( rc != 0 )
        return NULL;

#if 1
#define BZERO_SIZE 256
    if (bytes > BZERO_SIZE)
        memset(&((char *)dst->base)[bytes-BZERO_SIZE], 0, BZERO_SIZE);
    else
        memset(dst->base, 0, bytes);
#undef BZERO_SIZE
#endif
    
    /* return a pointer to it just like malloc */
    return dst -> base;
}

/*
 */
static void _s_undelta_4_channel (int slen, int tlen, void* src, int ssize)
{
  int     i, j;
  char*   cptr = (char*)src;
  short*  sptr = (short*)src;
  int*    iptr = (int*)src;

  assert (tlen);
  assert ((ssize&3) == 0); /* make sure we have all 4 channels in it */

  for (i=0, j=0; i<ssize/tlen; i+=4, j++)
  {
    /* after each slen the actual data is stored as is,
     * this shift happens at the begining as well
     */
    if (j%slen == 0) { i += 4; j++; }

    switch (tlen)
    {
      case 4: *(iptr+i+0) += *(iptr+i-4);
              *(iptr+i+1) += *(iptr+i-3);
              *(iptr+i+2) += *(iptr+i-2);
              *(iptr+i+3) += *(iptr+i-1);
              break;

      case 2: *(sptr+i+0) += *(sptr+i-4);
              *(sptr+i+1) += *(sptr+i-3);
              *(sptr+i+2) += *(sptr+i-2);
              *(sptr+i+3) += *(sptr+i-1);
              break;

      default:
      case 1: *(cptr+i+0) += *(cptr+i-4);
              *(cptr+i+1) += *(cptr+i-3);
              *(cptr+i+2) += *(cptr+i-2);
              *(cptr+i+3) += *(cptr+i-1);
              break;
    }
  }
}

/*
 */
static void _s_exponent_4_channel (int slen, void* dst, int dsize, const void* src, int ssize, float sim, float snm)
{
  int     i;
  float*  fptr = (float*)dst;
  short*  sptr = (short*)src;

  assert ((ssize&3) == 0); /* make sure we have all 4 channels in it */
  assert (ssize*2 == dsize);

  for (i=0; i<ssize>>1; i+=4) /* the source buffer is full of short values, therefore ssize>>1 */
  {
    *(fptr+i+0) = expf ((float)*(sptr+i+0)/LOG_MAIN_MULTIPLIER)/10 + sim;
    *(fptr+i+1) = expf ((float)*(sptr+i+1)/LOG_SUB_MULTIPLIER )/10 + snm;
    *(fptr+i+2) = expf ((float)*(sptr+i+2)/LOG_SUB_MULTIPLIER )/10 + snm;
    *(fptr+i+3) = expf ((float)*(sptr+i+3)/LOG_SUB_MULTIPLIER )/10 + snm;
  }
}

/*
 */
static int _s_decompress_zlib ( void* dst, const void* src, int dsize, int ssize )
{
    z_stream c_stream; /* decompression stream */
    
    memset( &c_stream, 0, sizeof( c_stream ) );
    c_stream.next_in   = ( Bytef* )src;
    c_stream.next_out  = ( Bytef* )dst;
    c_stream.avail_in  = ( uInt )ssize;
    c_stream.avail_out = ( uInt )dsize;
    
    if ( Z_OK != inflateInit ( &c_stream ) )
    {
        inflateEnd ( &c_stream );
        return 1; 
    }
    
    if ( Z_STREAM_END != inflate ( &c_stream, Z_FINISH ) &&
         Z_OK != inflateSync( &c_stream ) )
    {
        inflateEnd ( &c_stream );
        return 1; 
    }
    
    if ( Z_OK != inflateEnd ( &c_stream ) )
    {
        return 1;
    }
    
    return 0;
}

/*
 */
static int CC _s_cmp_key (const void* keyptr, const void* idxptr, void *data)
{
  SRALookup_t plook = data;
  static uint32_t mask [33] = {0x00000000, 0x80000000, 0xC0000000, 0xE0000000,
                               0xF0000000, 0xF8000000, 0xFC000000, 0xFE000000,
                               0xFF000000, 0xFF800000, 0xFFC00000, 0xFFE00000,
                               0xFFF00000, 0xFFF80000, 0xFFFC0000, 0xFFFE0000,
                               0xFFFF0000, 0xFFFF8000, 0xFFFFC000, 0xFFFFE000,
                               0xFFFFF000, 0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00,
                               0xFFFFFF00, 0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0,
                               0xFFFFFFF0, 0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,
                               0xFFFFFFFF};

  uint16_t   index      = *(uint16_t*)idxptr;
  uint32_t   keyBits    = *(uint32_t*)keyptr & mask[plook[index].nbits];
  uint32_t   lookupBits = plook[index].bits;

  if (keyBits < lookupBits) return -1;
  if (keyBits > lookupBits) return  1;

  return 0;
}

/*
 * decompresses positions for 454 platform, and restores their actual values
 *
 */
int sra_decompress_pos ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;              /* compression type magic */
  const uchar_t* pepos;              /* ptr to encoded positions */
  uchar_t*       popos;              /* ptr to output positions */
  uint16_t*      pspos;              /* ptr to short values after restoration */
  uchar_t*       iptr;               /* ptr to index in lookup table */
  uchar_t        indx;               /* actual index */
  uint16_t       offset, shift, osize, pos, rbpos, nbits, i;
  uint32_t       lval;               /* _get_long reads into it */

  assert (gv_local.lookup_454_pos_initialized);

  rbpos  = 0;
  offset = 0;
  pepos  = src;

  /* get magic and size of data
   */
  _get_byte  (pepos, &offset, rbpos, &magic, 1);
  _get_short (pepos, &offset, rbpos, &osize, 1);

  assert (magic == SRA_CT_ZLIB || magic == SRA_CT_HUFFMAN);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB) /* it is always applied on top of huffman */
  {
    shift = 3; /* magic + length */
    popos = sra_alloc ( dst, alt, osize + 100 ); /* generosity */

    if (_s_decompress_zlib(popos, pepos+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    pepos  = popos;
    _get_byte  (pepos, &offset, rbpos, &magic, 1);
    _get_short (pepos, &offset, rbpos, &osize, 1);
    assert (magic == SRA_CT_HUFFMAN);
    magic = SRA_CT_ZLIB; /* to make sure an extra allocated buffer will be destroyed */
  }

  popos = sra_alloc (dst, alt, osize );


  /* decompress huffman
   */
  pos = 0;
  while (pos < osize)
  {
    if (ssize-offset > 4) nbits = 32;
    else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

    _get_long ( pepos, nbits, &offset, &rbpos, &lval, 0, (ssize << 3) );

    iptr = (uchar_t*) kbsearch ( &lval, gv_local.idx_454_pos,
                                 POS_454_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_454_pos );
    assert (iptr);
    indx = *iptr;

    offset += ((rbpos+(uint16_t)gv_local.lookup_454_pos[indx].nbits) >> 3);
    rbpos   = ((rbpos+(uint16_t)gv_local.lookup_454_pos[indx].nbits) &  7);

    if (indx < POS_454_MAX_POINT)
      *(popos+pos) = indx;
    else
    {
      uchar_t val;
      _get_byte (pepos, &offset, rbpos, &val, 1);
      *(popos+pos) = val;
    }
    pos++;
  }

  /* undelta
   */
  pspos = sra_alloc ( dst, alt, (osize*sizeof(uint16_t)+3)&~3); /** align to 4-byte **/
  assert (pspos);

  *(pspos+0) = *(popos+0);
  for (i=1; i<osize; i++)
    *(pspos+i) = *(pspos+i-1) + *(popos+i);

  /* set element size to 16 bits */
  KDataBufferCast ( dst, dst, 16, true );

  /* crop size to osize */
  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}


int sra_decompress_prb_454 ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* peprb;
  uchar_t*       poprb;
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t*      iptr;
#endif
  uint16_t       offset, shift, osize, pos, rbpos, nbits;
  uchar_t        indx;
  uint32_t       lval;

  assert (gv_local.lookup_454_prb_initialized);

  rbpos  = 0; /* read bit position */
  offset = 0;
  peprb  = src;

  /* get magic and size of data
   */
  _get_byte  (peprb, &offset, rbpos, &magic, 1);
  _get_short (peprb, &offset, rbpos, &osize, 1);

  assert (magic == SRA_CT_ZLIB || magic == SRA_CT_HUFFMAN);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 3; /* magic + length */
/****  HACK: in some runs osize was truncated/corrupted????  Giving it some more space *****/
    poprb = sra_alloc ( dst, alt, osize+200 );

    if (_s_decompress_zlib(poprb, peprb+shift, (int)osize + 200, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    peprb  = poprb;
    _get_byte  (peprb, &offset, rbpos, &magic, 1);
    _get_short (peprb, &offset, rbpos, &osize, 1);
    assert (magic == SRA_CT_HUFFMAN);
  }

  poprb = sra_alloc (dst, alt, (osize+3)&~3); /** align to 4-byte boundary **/


  /* decompress huffman
   */
  pos = 0;
  while (pos < osize)
  {
    if (ssize-offset > 4) nbits = 32;
    else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

    if ((offset > (uint32_t)ssize) || (nbits == 0))
      return 1;

    _get_long (peprb, nbits, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
    indx = gv_local.rlookup_454_prb [lval>>16];
#else
    iptr = (uint16_t*) kbsearch (&lval, gv_local.idx_454_prb,
                                PRB_454_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_454_prb);
    assert (iptr);
    indx = *iptr;
#endif

    offset += ((rbpos+(uint16_t)gv_local.lookup_454_prb[indx].nbits) >> 3);
    rbpos   = ((rbpos+(uint16_t)gv_local.lookup_454_prb[indx].nbits) &  7);

    if (indx < PRB_454_MAX_POINT)
      *(poprb+pos) = indx;
    else
    {
      uchar_t val;
      _get_byte (peprb, &offset, rbpos, &val, 1);
      *(poprb+pos) = val;
    }
    pos++;
  }

  /* crop size to osize */
  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

int sra_decompress_clp  ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* peclp;
  uchar_t*       poclp;
  uint32_t       osize, offset;
  uint16_t       rbpos, shift;

  rbpos  = 0;
  offset = 0;
  peclp  = src;

  /* get magic and size of data
   */
  _get_byte  (peclp,     &offset,  rbpos, &magic, 1);
  _get_long  (peclp, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

  assert (magic == SRA_CT_ZLIB);

  shift = 5; /* magic + length */
  poclp = sra_alloc ( dst, alt, osize + 100 );

  if (_s_decompress_zlib(poclp, peclp+shift, (int)osize + 100, ssize-shift))
    return 1;

  /* crop size to osize */
  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

int sra_decompress_sig_454 ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* pesig;
  uchar_t*       posig;
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t*      iptr;
#endif
  uint16_t       offset, shift, osize, pos, rbpos, wbpos, indx, nbits;
  uint32_t       lval;

  assert (gv_local.lookup_454_sig_initialized);

  rbpos  = 0;
  offset = 0;
  pesig  = src;

  /* get magic and size of data
   */
  _get_byte  (pesig, &offset, rbpos, &magic, 1);
  _get_short (pesig, &offset, rbpos, &osize, 1);

  assert (magic == SRA_CT_ZLIB || magic == SRA_CT_HUFFMAN);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 3; /* magic + length */
    posig = sra_alloc ( dst, alt, osize + 100 );

    if (_s_decompress_zlib(posig, pesig+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    pesig  = posig;
    _get_byte  (pesig, &offset, rbpos, &magic, 1);
    _get_short (pesig, &offset, rbpos, &osize, 1);
    assert (magic == SRA_CT_HUFFMAN);
  }
 
  posig = sra_alloc ( dst, alt, osize );

  /* decompress huffman
   */
  pos   = 0;
  wbpos = 0; /* write bit position */
  while (pos < osize)
  {
    if (ssize-offset > 4) nbits = 32;
    else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

    _get_long (pesig, nbits, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
    indx = gv_local.rlookup_454_sig [lval>>16];
#else
    iptr = (uint16_t*) kbsearch (&lval, gv_local.idx_454_sig,
                                SIG_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_454_sig);
    assert (iptr);
    indx = *iptr;
#endif

    offset += ((rbpos+(uint16_t)gv_local.lookup_454_sig[indx].nbits) >> 3);
    rbpos   = ((rbpos+(uint16_t)gv_local.lookup_454_sig[indx].nbits) &  7);

    if (indx < SIG_MAX_POINT)
    {
      indx = bswap_16 (indx);
      _put_short (indx, posig, &pos, wbpos);
    }
    else
    {
      uint16_t val;
      if ((int)nbits - (int)gv_local.lookup_454_sig[indx].nbits >= 16)
      {
        val = (uint16_t)((lval >> (16-gv_local.lookup_454_sig[indx].nbits)) & 0xFFFF);
        offset += 2;
      }
      else
        _get_short (pesig, &offset, rbpos, &val, 1);

      val = bswap_16 (val);
      _put_short (val, posig, &pos, wbpos);
    }
  }

  /* crop size to osize */
  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

int sra_decompress_prb_slx ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  char           byte;
  char           qmin, qmax;
  const uchar_t* peprb;
  uchar_t*       poprb;
  uint32_t       osize, offset, rbpos, pos;
  uint16_t       fix_seq_len, shift;

  assert (gv_local.lookup_slx_prb_initialized);

  rbpos  = 0;
  offset = 0;
  peprb  = src;

  /* get magic and size of data
   */
  _get_byte  (peprb,     &offset,  rbpos, &magic, 1);
  _get_long  (peprb, 32, &offset, &rbpos, &osize, 1, (ssize << 3));
    
  assert (magic == SRA_CT_ZLIB || magic == SRA_CT_HUFFMAN || magic == SRA_CT_UNCOMPRESSED);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 5; /* magic + length */
    poprb = sra_alloc ( dst, alt, osize + 100);
    if (_s_decompress_zlib(poprb, peprb+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0;
    offset = 0;
    ssize  = osize;
    peprb  = poprb;
    _get_byte  (peprb,     &offset,  rbpos, &magic, 1);
    _get_long  (peprb, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

    assert (magic == SRA_CT_HUFFMAN || magic == SRA_CT_UNCOMPRESSED);
  }
    
  _get_byte  (peprb, &offset,  rbpos, &qmin,        1);
  _get_byte  (peprb, &offset,  rbpos, &qmax,        1);
  _get_short (peprb, &offset,  rbpos, &fix_seq_len, 1);

  poprb = sra_alloc (dst, alt, osize+100); /** huffman decode can read too much ***/

  /* decompress huffman
   */
  if (magic ==  SRA_CT_HUFFMAN)
  {
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
    uchar_t*   iptr;
#endif
    uchar_t    indx;
    union      { char c[4]; uint32_t val; } uval;
    uint32_t   lval;


    pos = 0;
    while (pos < osize)
    {
      if (offset > (uint32_t)ssize)
        return 1;

      _get_long (peprb, 4, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
      indx = gv_local.rlookup_slx_prb [lval>>24];
#else
      iptr = (uchar_t*) kbsearch (&lval, gv_local.idx_slx_prb,
                                 PRB_SLX_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_slx_prb);
      assert (iptr);
      indx = *iptr;
#endif

      offset += ((rbpos+(uint16_t)gv_local.lookup_slx_prb[indx].nbits) >> 3);
      rbpos   = ((rbpos+(uint16_t)gv_local.lookup_slx_prb[indx].nbits) &  7);

      switch (indx)
      {
        case 0: /* max min min min */
          uval.c[3] = uval.c[2] = uval.c[1] = qmin; uval.c[0] = qmax;
          break;

        case 1: /* -5 -5 -5 -5 */
          uval.c[3] = uval.c[2] = uval.c[1] = uval.c[0] = -5;
          break;

        case 2: /* val -val min min */
          _get_long (peprb, 6, &offset, &rbpos, &lval, 1, (ssize << 3));
          byte = (char)(lval >> 26); /* only 6 bits are needed */
          uval.c[3] = uval.c[2] = qmin; uval.c[1] = -byte; uval.c[0] = byte;
          break;

        case 3: /* val min -val min */
          _get_long (peprb, 6, &offset, &rbpos, &lval, 1, (ssize << 3));
          byte = (char)(lval >> 26); /* only 6 bits are needed */
          uval.c[3] = uval.c[1] = qmin; uval.c[2] = -byte; uval.c[0] = byte;
          break;

        case 4: /* val min min -val */
          _get_long (peprb, 6, &offset, &rbpos, &lval, 1, (ssize << 3));
          byte = (char)(lval >> 26); /* only 6 bits are needed */
          uval.c[2] = uval.c[1] = qmin; uval.c[3] = -byte; uval.c[0] = byte;
          break;

        case 5:
        default:
          _get_long (peprb, 32, &offset, &rbpos, (uint32_t *)&uval.c[0], 1, (ssize << 3));
          /*uval.val = lval*/;
          break;
      }
      memcpy (poprb+pos, &uval.c[0], sizeof(uint32_t));
      pos += 4;
    }
  }
  else
  if (magic == SRA_CT_UNCOMPRESSED)
  {
      memcpy (poprb, peprb+9, osize);
  }

  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

static void _s_decode_slx (uchar_t magic, const uchar_t* pesig, uchar_t* posig, uint32_t osize, uint32_t offset, uint32_t rbpos, int ssize)
{
  uint32_t pos = 0;
  uint16_t fix_seq_len;

  if (magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_BASELINED)
  {
    int32_t  baseline = 0;
    float    fval;
    uint16_t uval;

    _get_short (pesig,     &offset,  rbpos, &fix_seq_len,  1);
    _get_long  (pesig, 32, &offset, &rbpos, &baseline,     1, (ssize << 3));

    rbpos = 0; /* read bit position */


    while (pos < osize)
    {
      uval = 0;
      _get_short (pesig, &offset, rbpos, &uval, 1);
      fval =     (float) (uval-baseline);
      memcpy     (posig+pos, &fval, sizeof(float));
      pos += sizeof(float);
    }
  }
  else
  if (magic == SRA_CT_NORMALIZED)
  {
    union  { float fval; uint32_t uval; } uf;
    float  fval;
    float  fmin;
    short  sval;

    _get_short (pesig,     &offset,  rbpos, &fix_seq_len,  1);
    _get_short (pesig,     &offset,  rbpos, &sval,         1);
    _get_long  (pesig, 32, &offset, &rbpos, &uf.uval,      1, (ssize << 3));

    fmin = (float) sval;
    rbpos = 0; /* read bit position */


    while (pos < osize)
    {
      sval = 0;
      fval = uf.fval;
      _get_short (pesig, &offset, rbpos, &sval, 1);
      fval *= sval; fval += fmin;
      memcpy     (posig+pos, &fval, sizeof(float));
      pos += sizeof(float);
    }
  }
}

int sra_decompress_sig_slx ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* pesig;
  uchar_t*       posig;
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t*      iptr;
#endif
  uint16_t       indx, fix_seq_len, shift;
  uint32_t       pos, rbpos, wbpos, nbits;
  uint32_t       offset, osize, lval;
  union { float fval; uint32_t uval; } sim, snm; /* minimum level of signal and noise */

  assert (gv_local.lookup_slx_sig_initialized);

  rbpos  = 0;
  offset = 0;
  pesig  = src;

  /* get magic and size of data
   */
  _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
  _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

  assert (magic == SRA_CT_ZLIB         || magic == SRA_CT_HUFFMAN ||
          magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED ||
          magic == SRA_CT_BASELINED);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 5; /* magic + length */
    posig = sra_alloc ( dst, alt, osize + 100 );

    if (_s_decompress_zlib(posig, pesig+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    pesig  = posig;
    _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

    assert (magic == SRA_CT_HUFFMAN    || magic == SRA_CT_UNCOMPRESSED ||
            magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED);
  }

  /* decompress huffman
   */
  if (magic == SRA_CT_HUFFMAN)
  {
    _get_short (pesig,     &offset,  rbpos, &fix_seq_len, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &sim.uval,    1, (ssize << 3));
    _get_long  (pesig, 32, &offset, &rbpos, &snm.uval,    1, (ssize << 3));

    assert ((osize&3) == 0); /* make shure it can be devided by 4 */

    posig = sra_alloc (dst, alt, osize);

    pos   = 0;
    wbpos = 0; /* write bit position */

    while (pos < osize)
    {
      if (ssize-offset > 4) nbits = 32;
      else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

      if (offset > (uint32_t)ssize)
        return 1;

      _get_long (pesig, nbits, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
      indx = gv_local.rlookup_slx_sig [lval>>16];
#else
      iptr = (uint16_t*) kbsearch (&lval, gv_local.idx_slx_sig,
                                  SIG_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_slx_sig);
      assert (iptr);
      indx = *iptr;
#endif

      offset += ((rbpos+(uint32_t)gv_local.lookup_slx_sig[indx].nbits) >> 3);
      rbpos   = ((rbpos+(uint32_t)gv_local.lookup_slx_sig[indx].nbits)  & 7);

      if (indx < SIG_MAX_POINT)
      {
        indx = bswap_16 ((uint16_t)(indx-SIG_MID_POINT));
        _put_short (indx, posig, &pos, wbpos);
      }
      else
      {
        uint16_t val;
        if ((int)nbits - (int)gv_local.lookup_slx_sig[indx].nbits >= 16)
        {
          val = (short)((lval >> (16-gv_local.lookup_slx_sig[indx].nbits)) & 0xFFFF);
          offset += 2;
        }
        else
          _get_short (pesig, &offset, rbpos, &val, 1);

        val = bswap_16 (val);
        _put_short (val, posig, &pos, wbpos);
      }
    }

    _s_undelta_4_channel (fix_seq_len, sizeof(uint16_t), posig, osize);

    pesig = posig;
    ssize = osize;
    osize = osize << 1;
    posig = sra_alloc ( dst, alt, osize );

    _s_exponent_4_channel (fix_seq_len, posig, osize, pesig, ssize, sim.fval, snm.fval);
  }
  else
  if (magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED)
  {
    assert ((osize&3) == 0); /* make shure it can be devided by 4 */
    osize *= 2;              /* desination size is 4 bytes per channel, but the original is 2 */

    posig = sra_alloc ( dst, alt, osize );

    _s_decode_slx (magic, pesig, posig, osize, offset, rbpos, ssize);
  }

  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

int sra_decompress_int ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* pesig;
  uchar_t*       posig;
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t*      iptr;
#endif
  uint16_t       indx, fix_seq_len, shift;
  uint32_t       pos, rbpos, wbpos, nbits;
  uint32_t       offset, osize, lval;
  union { float fval; uint32_t uval; } sim, snm; /* minimum level of signal and noise */

  assert (gv_local.lookup_slx_int_initialized);

  rbpos  = 0;
  offset = 0;
  pesig  = src;

  /* get magic and size of data
   */
  _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
  _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

  assert (magic == SRA_CT_ZLIB         || magic == SRA_CT_HUFFMAN ||
          magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED ||
          magic == SRA_CT_BASELINED);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 5; /* magic + length */
    posig = sra_alloc ( dst, alt, osize + 100 );

    if (_s_decompress_zlib(posig, pesig+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    pesig  = posig;
    _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

    assert (magic == SRA_CT_HUFFMAN    || magic == SRA_CT_UNCOMPRESSED ||
            magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED);
  }

  /* decompress huffman
   */
  if (magic == SRA_CT_HUFFMAN)
  {
    _get_short (pesig,     &offset,  rbpos, &fix_seq_len, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &sim.uval,    1, (ssize << 3));
    _get_long  (pesig, 32, &offset, &rbpos, &snm.uval,    1, (ssize << 3));

    assert ((osize&3) == 0); /* make shure it can be devided by 4 */

    posig = sra_alloc (dst, alt, osize);

    pos   = 0;
    wbpos = 0; /* write bit position */

    while (pos < osize)
    {
      if (ssize-offset > 4) nbits = 32;
      else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

      if (offset > (uint32_t)ssize)
        return 1;

      _get_long (pesig, nbits, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
      indx = gv_local.rlookup_slx_int [lval>>16];
#else
      iptr = (uint16_t*) kbsearch (&lval, gv_local.idx_slx_int,
                                  INT_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_slx_int);
      assert (iptr);
      indx = *iptr;
#endif

      offset += ((rbpos+(uint32_t)gv_local.lookup_slx_int[indx].nbits) >> 3);
      rbpos   = ((rbpos+(uint32_t)gv_local.lookup_slx_int[indx].nbits)  & 7);

      if (indx < INT_MAX_POINT)
      {
        indx = bswap_16 ((uint16_t)(indx-INT_MID_POINT));
        _put_short (indx, posig, &pos, wbpos);
      }
      else
      {
        uint16_t val;
        if ((int)nbits - (int)gv_local.lookup_slx_int[indx].nbits >= 16)
        {
          val = (short)((lval >> (16-gv_local.lookup_slx_int[indx].nbits)) & 0xFFFF);
          offset += 2;
        }
        else
          _get_short (pesig, &offset, rbpos, &val, 1);

        val = bswap_16 (val);
        _put_short (val, posig, &pos, wbpos);
      }
    }

    _s_undelta_4_channel (fix_seq_len, sizeof(uint16_t), posig, osize);

    pesig = posig;
    ssize = osize;
    osize = osize << 1;
    posig = sra_alloc (dst, alt, osize);

    _s_exponent_4_channel (fix_seq_len, posig, osize, pesig, ssize, sim.fval, snm.fval);
  }
  else
  if (magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED)
  {
    assert ((osize&3) == 0); /* make shure it can be devided by 4 */
    osize *= 2;              /* desination size is 4 bytes per channel, but the original is 2 */

    posig = sra_alloc (dst, alt, osize);

    _s_decode_slx (magic, pesig, posig, osize, offset, rbpos, ssize);
  }

  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

int sra_decompress_nse ( KDataBuffer *dst, KDataBuffer *alt, const void* src, int ssize )
{
  uchar_t        magic;
  const uchar_t* pesig;
  uchar_t*       posig;
#if !SRA_DECOMPRESS_USE_REVERSE_LOOKUP
  uint16_t*      iptr;
#endif
  uint16_t       indx, fix_seq_len, shift;
  uint32_t       pos, rbpos, wbpos, nbits;
  uint32_t       offset, osize, lval;
  union { float fval; uint32_t uval; } sim, snm; /* minimum level of signal and noise */

  assert (gv_local.lookup_slx_nse_initialized);

  rbpos  = 0;
  offset = 0;
  pesig  = src;

  /* get magic and size of data
   */
  _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
  _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

  assert (magic == SRA_CT_ZLIB         || magic == SRA_CT_HUFFMAN ||
          magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED ||
          magic == SRA_CT_BASELINED);

  /* decompress zlib
   */
  if (magic == SRA_CT_ZLIB)
  {
    shift = 5; /* magic + length */
    posig = sra_alloc ( dst, alt, osize + 100 );

    if (_s_decompress_zlib(posig, pesig+shift, (int)osize + 100, ssize-shift))
      return 1;

    rbpos  = 0; /* read bit position */
    offset = 0;
    ssize  = osize;
    pesig  = posig;
    _get_byte  (pesig,     &offset,  rbpos, &magic, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &osize, 1, (ssize << 3));

    assert (magic == SRA_CT_HUFFMAN    || magic == SRA_CT_UNCOMPRESSED ||
            magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED);
  }

  /* decompress huffman
   */
  if (magic == SRA_CT_HUFFMAN)
  {
    _get_short (pesig,     &offset,  rbpos, &fix_seq_len, 1);
    _get_long  (pesig, 32, &offset, &rbpos, &sim.uval,    1, (ssize << 3));
    _get_long  (pesig, 32, &offset, &rbpos, &snm.uval,    1, (ssize << 3));

    assert ((osize&3) == 0); /* make shure it can be devided by 4 */

    posig = sra_alloc (dst, alt, osize);

    /* decompress huffman
     */
    pos   = 0;
    wbpos = 0; /* write bit position */

    while (pos < osize)
    {
      if (ssize-offset > 4) nbits = 32;
      else                  nbits = ((ssize-offset)==4?32:((ssize-offset)==3?24:((ssize-offset)==2?16:8)))-rbpos;

      if (offset > (uint32_t)ssize)
        return 1;

      _get_long (pesig, nbits, &offset, &rbpos, &lval, 0, (ssize << 3));

#if SRA_DECOMPRESS_USE_REVERSE_LOOKUP
      indx = gv_local.rlookup_slx_nse [lval>>16];
#else
      iptr = (uint16_t*) kbsearch (&lval, gv_local.idx_slx_nse,
                                  NSE_HUFF_TREESIZE, sizeof(uint16_t), _s_cmp_key, gv_local.lookup_slx_nse);
      assert (iptr);
      indx = *iptr;
#endif

      offset += ((rbpos+(uint32_t)gv_local.lookup_slx_nse[indx].nbits) >> 3);
      rbpos   = ((rbpos+(uint32_t)gv_local.lookup_slx_nse[indx].nbits)  & 7);

      if (indx < NSE_MAX_POINT)
      {
        indx = bswap_16 ((uint16_t)(indx-NSE_MID_POINT));
        _put_short (indx, posig, &pos, wbpos);
      }
      else
      {
        uint16_t val;
        if ((int)nbits - (int)gv_local.lookup_slx_int[indx].nbits >= 16)
        {
          val = (short)((lval >> (16-gv_local.lookup_slx_nse[indx].nbits)) & 0xFFFF);
          offset += 2;
        }
        else
          _get_short (pesig, &offset, rbpos, &val, 1);

        val = bswap_16 (val);
        _put_short (val, posig, &pos, wbpos);
      }
    }

    _s_undelta_4_channel (fix_seq_len, sizeof(uint16_t), posig, osize);

    pesig = posig;
    ssize = osize;
    osize = osize << 1;
    posig = sra_alloc (dst, alt, osize);

    _s_exponent_4_channel (fix_seq_len, posig, osize, pesig, ssize, sim.fval, snm.fval);
  }
  else
  if (magic == SRA_CT_UNCOMPRESSED || magic == SRA_CT_NORMALIZED || magic == SRA_CT_BASELINED)
  {
    assert ((osize&3) == 0); /* make shure it can be devided by 4 */
    osize *= 2;              /* desination size is 4 bytes per channel, but the original is 2 */

    posig = sra_alloc (dst, alt, osize);

    _s_decode_slx (magic, pesig, posig, osize, offset, rbpos, ssize);
  }

  assert ( osize <= dst -> elem_count );
  KDataBufferResize ( dst, osize );

  return 0;
}

