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

#ifndef _h_v128_
#define _h_v128_

#include <stdint.h>
#include <x86intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t   v128_i8_t  __attribute__ ((vector_size (16),__may_alias__));
typedef uint8_t  v128_u8_t  __attribute__ ((vector_size (16),__may_alias__));
typedef int16_t  v128_i16_t __attribute__ ((vector_size (16),__may_alias__));
typedef int32_t  v128_i32_t __attribute__ ((vector_size (16),__may_alias__));
typedef int64_t  v128_i64_t __attribute__ ((vector_size (16),__may_alias__));
typedef uint16_t v128_u16_t __attribute__ ((vector_size (16),__may_alias__));
typedef uint32_t v128_u32_t __attribute__ ((vector_size (16),__may_alias__));
typedef uint64_t v128_u64_t __attribute__ ((vector_size (16),__may_alias__));
typedef float    v128_f_t   __attribute__ ((vector_size (16),__may_alias__));
typedef double   v128_d_t   __attribute__ ((vector_size (16),__may_alias__));

/* NOTE:
 * these types are to hide perceived flaws in the Intel/GCC code
 * don't use them outside of this file.  uint8_t, int8_t and int64_t all
 * fail to compile so the above types can't be used within this file in some 
 * places.
 */
typedef char          v128_c_t   __attribute__ ((vector_size (16),__may_alias__));
typedef long long int v128_lli_t __attribute__ ((vector_size (16),__may_alias__));


#if USE_VEC_REG

/* SSE
  movss
  movaps
  movups
  movlps
  movhps
  movlhps
  movhlps
*/

#ifdef __MMX__
/* MMX
   v8qi __builtin_ia32_paddb (v8qi, v8qi)
   v4hi __builtin_ia32_paddw (v4hi, v4hi)
   v2si __builtin_ia32_paddd (v2si, v2si)
   v8qi __builtin_ia32_psubb (v8qi, v8qi)
   v4hi __builtin_ia32_psubw (v4hi, v4hi)
   v2si __builtin_ia32_psubd (v2si, v2si)
   v8qi __builtin_ia32_paddsb (v8qi, v8qi)
   v4hi __builtin_ia32_paddsw (v4hi, v4hi)
   v8qi __builtin_ia32_psubsb (v8qi, v8qi)
   v4hi __builtin_ia32_psubsw (v4hi, v4hi)
   v8qi __builtin_ia32_paddusb (v8qi, v8qi)
   v4hi __builtin_ia32_paddusw (v4hi, v4hi)
   v8qi __builtin_ia32_psubusb (v8qi, v8qi)
   v4hi __builtin_ia32_psubusw (v4hi, v4hi)
   v4hi __builtin_ia32_pmullw (v4hi, v4hi)
   v4hi __builtin_ia32_pmulhw (v4hi, v4hi)
   di __builtin_ia32_pand (di, di)
   di __builtin_ia32_pandn (di,di)
   di __builtin_ia32_por (di, di)
   di __builtin_ia32_pxor (di, di)
   v8qi __builtin_ia32_pcmpeqb (v8qi, v8qi)
   v4hi __builtin_ia32_pcmpeqw (v4hi, v4hi)
   v2si __builtin_ia32_pcmpeqd (v2si, v2si)
   v8qi __builtin_ia32_pcmpgtb (v8qi, v8qi)
   v4hi __builtin_ia32_pcmpgtw (v4hi, v4hi)
   v2si __builtin_ia32_pcmpgtd (v2si, v2si)
   v8qi __builtin_ia32_punpckhbw (v8qi, v8qi)
   v4hi __builtin_ia32_punpckhwd (v4hi, v4hi)
   v2si __builtin_ia32_punpckhdq (v2si, v2si)
   v8qi __builtin_ia32_punpcklbw (v8qi, v8qi)
   v4hi __builtin_ia32_punpcklwd (v4hi, v4hi)
   v2si __builtin_ia32_punpckldq (v2si, v2si)
   v8qi __builtin_ia32_packsswb (v4hi, v4hi)
   v4hi __builtin_ia32_packssdw (v2si, v2si)
   v8qi __builtin_ia32_packuswb (v4hi, v4hi)
     
   v4hi __builtin_ia32_psllw (v4hi, v4hi)
   v2si __builtin_ia32_pslld (v2si, v2si)
   v1di __builtin_ia32_psllq (v1di, v1di)
   v4hi __builtin_ia32_psrlw (v4hi, v4hi)
   v2si __builtin_ia32_psrld (v2si, v2si)
   v1di __builtin_ia32_psrlq (v1di, v1di)
   v4hi __builtin_ia32_psraw (v4hi, v4hi)
   v2si __builtin_ia32_psrad (v2si, v2si)
   v4hi __builtin_ia32_psllwi (v4hi, int)
   v2si __builtin_ia32_pslldi (v2si, int)
   v1di __builtin_ia32_psllqi (v1di, int)
   v4hi __builtin_ia32_psrlwi (v4hi, int)
   v2si __builtin_ia32_psrldi (v2si, int)
   v1di __builtin_ia32_psrlqi (v1di, int)
   v4hi __builtin_ia32_psrawi (v4hi, int)
   v2si __builtin_ia32_psradi (v2si, int)
*/
#endif

#ifdef __SSE__
/* SSE
   v4hi __builtin_ia32_pmulhuw (v4hi, v4hi)
   v8qi __builtin_ia32_pavgb (v8qi, v8qi)
   v4hi __builtin_ia32_pavgw (v4hi, v4hi)
   v1di __builtin_ia32_psadbw (v8qi, v8qi)
   v8qi __builtin_ia32_pmaxub (v8qi, v8qi)
   v4hi __builtin_ia32_pmaxsw (v4hi, v4hi)
   v8qi __builtin_ia32_pminub (v8qi, v8qi)
   v4hi __builtin_ia32_pminsw (v4hi, v4hi)
   int __builtin_ia32_pextrw (v4hi, int)
   v4hi __builtin_ia32_pinsrw (v4hi, int, int)
   int __builtin_ia32_pmovmskb (v8qi)
   void __builtin_ia32_maskmovq (v8qi, v8qi, char *)
   void __builtin_ia32_movntq (di *, di)
   void __builtin_ia32_sfence (void)

   int __builtin_ia32_comieq (v4sf, v4sf)
   int __builtin_ia32_comineq (v4sf, v4sf)
   int __builtin_ia32_comilt (v4sf, v4sf)
   int __builtin_ia32_comile (v4sf, v4sf)
   int __builtin_ia32_comigt (v4sf, v4sf)
   int __builtin_ia32_comige (v4sf, v4sf)
   int __builtin_ia32_ucomieq (v4sf, v4sf)
   int __builtin_ia32_ucomineq (v4sf, v4sf)
   int __builtin_ia32_ucomilt (v4sf, v4sf)
   int __builtin_ia32_ucomile (v4sf, v4sf)
   int __builtin_ia32_ucomigt (v4sf, v4sf)
   int __builtin_ia32_ucomige (v4sf, v4sf)
   v4sf __builtin_ia32_addps (v4sf, v4sf)
   v4sf __builtin_ia32_subps (v4sf, v4sf)
   v4sf __builtin_ia32_mulps (v4sf, v4sf)
   v4sf __builtin_ia32_divps (v4sf, v4sf)
   v4sf __builtin_ia32_addss (v4sf, v4sf)
   v4sf __builtin_ia32_subss (v4sf, v4sf)
   v4sf __builtin_ia32_mulss (v4sf, v4sf)
   v4sf __builtin_ia32_divss (v4sf, v4sf)
   v4si __builtin_ia32_cmpeqps (v4sf, v4sf)
   v4si __builtin_ia32_cmpltps (v4sf, v4sf)
   v4si __builtin_ia32_cmpleps (v4sf, v4sf)
   v4si __builtin_ia32_cmpgtps (v4sf, v4sf)
   v4si __builtin_ia32_cmpgeps (v4sf, v4sf)
   v4si __builtin_ia32_cmpunordps (v4sf, v4sf)
   v4si __builtin_ia32_cmpneqps (v4sf, v4sf)
   v4si __builtin_ia32_cmpnltps (v4sf, v4sf)
   v4si __builtin_ia32_cmpnleps (v4sf, v4sf)
   v4si __builtin_ia32_cmpngtps (v4sf, v4sf)
   v4si __builtin_ia32_cmpngeps (v4sf, v4sf)
   v4si __builtin_ia32_cmpordps (v4sf, v4sf)
   v4si __builtin_ia32_cmpeqss (v4sf, v4sf)
   v4si __builtin_ia32_cmpltss (v4sf, v4sf)
   v4si __builtin_ia32_cmpless (v4sf, v4sf)
   v4si __builtin_ia32_cmpunordss (v4sf, v4sf)
   v4si __builtin_ia32_cmpneqss (v4sf, v4sf)
   v4si __builtin_ia32_cmpnlts (v4sf, v4sf)
   v4si __builtin_ia32_cmpnless (v4sf, v4sf)
   v4si __builtin_ia32_cmpordss (v4sf, v4sf)
   v4sf __builtin_ia32_maxps (v4sf, v4sf)
   v4sf __builtin_ia32_maxss (v4sf, v4sf)
   v4sf __builtin_ia32_minps (v4sf, v4sf)
   v4sf __builtin_ia32_minss (v4sf, v4sf)
   v4sf __builtin_ia32_andps (v4sf, v4sf)
   v4sf __builtin_ia32_andnps (v4sf, v4sf)
   v4sf __builtin_ia32_orps (v4sf, v4sf)
   v4sf __builtin_ia32_xorps (v4sf, v4sf)
   v4sf __builtin_ia32_movss (v4sf, v4sf)
   v4sf __builtin_ia32_movhlps (v4sf, v4sf)
   v4sf __builtin_ia32_movlhps (v4sf, v4sf)
   v4sf __builtin_ia32_unpckhps (v4sf, v4sf)
   v4sf __builtin_ia32_unpcklps (v4sf, v4sf)
   v4sf __builtin_ia32_cvtpi2ps (v4sf, v2si)
   v4sf __builtin_ia32_cvtsi2ss (v4sf, int)
   v2si __builtin_ia32_cvtps2pi (v4sf)
   int __builtin_ia32_cvtss2si (v4sf)
   v2si __builtin_ia32_cvttps2pi (v4sf)
   int __builtin_ia32_cvttss2si (v4sf)
   v4sf __builtin_ia32_rcpps (v4sf)
   v4sf __builtin_ia32_rsqrtps (v4sf)
   v4sf __builtin_ia32_sqrtps (v4sf)
   v4sf __builtin_ia32_rcpss (v4sf)
   v4sf __builtin_ia32_rsqrtss (v4sf)
   v4sf __builtin_ia32_sqrtss (v4sf)
   v4sf __builtin_ia32_shufps (v4sf, v4sf, int)
*/
static __inline
v128_u8_t op_SHUFPS ( v128_u8_t vin1, v128_u8_t vin2, const int mask )
    __attribute__ ((always_inline));

static __inline
v128_u8_t op_SHUFPS (v128_u8_t vin1, v128_u8_t vin2, const int mask)
{
    return ( v128_u8_t ) __builtin_ia32_shufps ((v128_f_t)vin1, (v128_f_t)vin2, mask);
}

 /*
   void __builtin_ia32_movntps (float *, v4sf)
   int __builtin_ia32_movmskps (v4sf)

v4sf __builtin_ia32_loadaps (float *)
Generates the movaps machine instruction as a load from memory. 
void __builtin_ia32_storeaps (float *, v4sf)
Generates the movaps machine instruction as a store to memory. 
v4sf __builtin_ia32_loadups (float *)
Generates the movups machine instruction as a load from memory. 
void __builtin_ia32_storeups (float *, v4sf)
Generates the movups machine instruction as a store to memory. 
v4sf __builtin_ia32_loadsss (float *)
Generates the movss machine instruction as a load from memory. 
void __builtin_ia32_storess (float *, v4sf)
Generates the movss machine instruction as a store to memory. 
v4sf __builtin_ia32_loadhps (v4sf, const v2sf *)
Generates the movhps machine instruction as a load from memory. 
v4sf __builtin_ia32_loadlps (v4sf, const v2sf *)
Generates the movlps machine instruction as a load from memory 
void __builtin_ia32_storehps (v2sf *, v4sf)
Generates the movhps machine instruction as a store to memory. 
void __builtin_ia32_storelps (v2sf *, v4sf)
Generates the movlps machine instruction as a store to memory.
*/
#endif

#ifdef __SSE2__
/* sse2
     int __builtin_ia32_comisdeq (v2df, v2df)
     int __builtin_ia32_comisdlt (v2df, v2df)
     int __builtin_ia32_comisdle (v2df, v2df)
     int __builtin_ia32_comisdgt (v2df, v2df)
     int __builtin_ia32_comisdge (v2df, v2df)
     int __builtin_ia32_comisdneq (v2df, v2df)
     int __builtin_ia32_ucomisdeq (v2df, v2df)
     int __builtin_ia32_ucomisdlt (v2df, v2df)
     int __builtin_ia32_ucomisdle (v2df, v2df)
     int __builtin_ia32_ucomisdgt (v2df, v2df)
     int __builtin_ia32_ucomisdge (v2df, v2df)
     int __builtin_ia32_ucomisdneq (v2df, v2df)
     v2df __builtin_ia32_cmpeqpd (v2df, v2df)
     v2df __builtin_ia32_cmpltpd (v2df, v2df)
     v2df __builtin_ia32_cmplepd (v2df, v2df)
     v2df __builtin_ia32_cmpgtpd (v2df, v2df)
     v2df __builtin_ia32_cmpgepd (v2df, v2df)
     v2df __builtin_ia32_cmpunordpd (v2df, v2df)
     v2df __builtin_ia32_cmpneqpd (v2df, v2df)
     v2df __builtin_ia32_cmpnltpd (v2df, v2df)
     v2df __builtin_ia32_cmpnlepd (v2df, v2df)
     v2df __builtin_ia32_cmpngtpd (v2df, v2df)
     v2df __builtin_ia32_cmpngepd (v2df, v2df)
     v2df __builtin_ia32_cmpordpd (v2df, v2df)
     v2df __builtin_ia32_cmpeqsd (v2df, v2df)
     v2df __builtin_ia32_cmpltsd (v2df, v2df)
     v2df __builtin_ia32_cmplesd (v2df, v2df)
     v2df __builtin_ia32_cmpunordsd (v2df, v2df)
     v2df __builtin_ia32_cmpneqsd (v2df, v2df)
     v2df __builtin_ia32_cmpnltsd (v2df, v2df)
     v2df __builtin_ia32_cmpnlesd (v2df, v2df)
     v2df __builtin_ia32_cmpordsd (v2df, v2df)
     v2di __builtin_ia32_paddq (v2di, v2di)
     v2di __builtin_ia32_psubq (v2di, v2di)
     v2df __builtin_ia32_addpd (v2df, v2df)
     v2df __builtin_ia32_subpd (v2df, v2df)
     v2df __builtin_ia32_mulpd (v2df, v2df)
     v2df __builtin_ia32_divpd (v2df, v2df)
     v2df __builtin_ia32_addsd (v2df, v2df)
     v2df __builtin_ia32_subsd (v2df, v2df)
     v2df __builtin_ia32_mulsd (v2df, v2df)
     v2df __builtin_ia32_divsd (v2df, v2df)
     v2df __builtin_ia32_minpd (v2df, v2df)
     v2df __builtin_ia32_maxpd (v2df, v2df)
     v2df __builtin_ia32_minsd (v2df, v2df)
     v2df __builtin_ia32_maxsd (v2df, v2df)
     v2df __builtin_ia32_andpd (v2df, v2df)
     v2df __builtin_ia32_andnpd (v2df, v2df)
     v2df __builtin_ia32_orpd (v2df, v2df)
     v2df __builtin_ia32_xorpd (v2df, v2df)
     v2df __builtin_ia32_movsd (v2df, v2df)
     v2df __builtin_ia32_unpckhpd (v2df, v2df)
     v2df __builtin_ia32_unpcklpd (v2df, v2df)
     v16qi __builtin_ia32_paddb128 (v16qi, v16qi)
     v8hi __builtin_ia32_paddw128 (v8hi, v8hi)
     v4si __builtin_ia32_paddd128 (v4si, v4si)
     v2di __builtin_ia32_paddq128 (v2di, v2di)
     v16qi __builtin_ia32_psubb128 (v16qi, v16qi)
     v8hi __builtin_ia32_psubw128 (v8hi, v8hi)
     v4si __builtin_ia32_psubd128 (v4si, v4si)
     v2di __builtin_ia32_psubq128 (v2di, v2di)
     v8hi __builtin_ia32_pmullw128 (v8hi, v8hi)
     v8hi __builtin_ia32_pmulhw128 (v8hi, v8hi)
     v2di __builtin_ia32_pand128 (v2di, v2di)
     v2di __builtin_ia32_pandn128 (v2di, v2di)
     v2di __builtin_ia32_por128 (v2di, v2di)
     v2di __builtin_ia32_pxor128 (v2di, v2di)
     v16qi __builtin_ia32_pavgb128 (v16qi, v16qi)
     v8hi __builtin_ia32_pavgw128 (v8hi, v8hi)
     v16qi __builtin_ia32_pcmpeqb128 (v16qi, v16qi)
     v8hi __builtin_ia32_pcmpeqw128 (v8hi, v8hi)
     v4si __builtin_ia32_pcmpeqd128 (v4si, v4si)
     v16qi __builtin_ia32_pcmpgtb128 (v16qi, v16qi)
     v8hi __builtin_ia32_pcmpgtw128 (v8hi, v8hi)
     v4si __builtin_ia32_pcmpgtd128 (v4si, v4si)
     v16qi __builtin_ia32_pmaxub128 (v16qi, v16qi)
     v8hi __builtin_ia32_pmaxsw128 (v8hi, v8hi)
     v16qi __builtin_ia32_pminub128 (v16qi, v16qi)
     v8hi __builtin_ia32_pminsw128 (v8hi, v8hi)
     v16qi __builtin_ia32_punpckhbw128 (v16qi, v16qi)
     v8hi __builtin_ia32_punpckhwd128 (v8hi, v8hi)
     v4si __builtin_ia32_punpckhdq128 (v4si, v4si)
     v2di __builtin_ia32_punpckhqdq128 (v2di, v2di)
     v16qi __builtin_ia32_punpcklbw128 (v16qi, v16qi)
     v8hi __builtin_ia32_punpcklwd128 (v8hi, v8hi)
     v4si __builtin_ia32_punpckldq128 (v4si, v4si)
     v2di __builtin_ia32_punpcklqdq128 (v2di, v2di)
     v16qi __builtin_ia32_packsswb128 (v8hi, v8hi)
     v8hi __builtin_ia32_packssdw128 (v4si, v4si)
     v16qi __builtin_ia32_packuswb128 (v8hi, v8hi)
     v8hi __builtin_ia32_pmulhuw128 (v8hi, v8hi)
     void __builtin_ia32_maskmovdqu (v16qi, v16qi)
     v2df __builtin_ia32_loadupd (double *)
     void __builtin_ia32_storeupd (double *, v2df)
     v2df __builtin_ia32_loadhpd (v2df, double const *)
     v2df __builtin_ia32_loadlpd (v2df, double const *)
     int __builtin_ia32_movmskpd (v2df)
     int __builtin_ia32_pmovmskb128 (v16qi)
     void __builtin_ia32_movnti (int *, int)
     void __builtin_ia32_movntpd (double *, v2df)
     void __builtin_ia32_movntdq (v2df *, v2df)
     v4si __builtin_ia32_pshufd (v4si, int)
     v8hi __builtin_ia32_pshuflw (v8hi, int)
     v8hi __builtin_ia32_pshufhw (v8hi, int)
     v2di __builtin_ia32_psadbw128 (v16qi, v16qi)
     v2df __builtin_ia32_sqrtpd (v2df)
     v2df __builtin_ia32_sqrtsd (v2df)
     v2df __builtin_ia32_shufpd (v2df, v2df, int)
*/
static __inline
v128_u8_t op_SHUFPD (v128_u8_t vin1, v128_u8_t vin2, const int mask)
    __attribute__ ((always_inline));

static __inline
v128_u8_t op_SHUFPD (v128_u8_t vin1, v128_u8_t vin2, const int mask)
{
    return ( v128_u8_t ) __builtin_ia32_shufpd ((v128_d_t)vin1, (v128_d_t)vin2, mask);
}

/*
     v2df __builtin_ia32_cvtdq2pd (v4si)
     v4sf __builtin_ia32_cvtdq2ps (v4si)
     v4si __builtin_ia32_cvtpd2dq (v2df)
     v2si __builtin_ia32_cvtpd2pi (v2df)
     v4sf __builtin_ia32_cvtpd2ps (v2df)
     v4si __builtin_ia32_cvttpd2dq (v2df)
     v2si __builtin_ia32_cvttpd2pi (v2df)
     v2df __builtin_ia32_cvtpi2pd (v2si)
     int __builtin_ia32_cvtsd2si (v2df)
     int __builtin_ia32_cvttsd2si (v2df)
     long long __builtin_ia32_cvtsd2si64 (v2df)
     long long __builtin_ia32_cvttsd2si64 (v2df)
     v4si __builtin_ia32_cvtps2dq (v4sf)
     v2df __builtin_ia32_cvtps2pd (v4sf)
     v4si __builtin_ia32_cvttps2dq (v4sf)
     v2df __builtin_ia32_cvtsi2sd (v2df, int)
     v2df __builtin_ia32_cvtsi642sd (v2df, long long)
     v4sf __builtin_ia32_cvtsd2ss (v4sf, v2df)
     v2df __builtin_ia32_cvtss2sd (v2df, v4sf)
     void __builtin_ia32_clflush (const void *)
     void __builtin_ia32_lfence (void)
     void __builtin_ia32_mfence (void)
     v16qi __builtin_ia32_loaddqu (const char *)
     void __builtin_ia32_storedqu (char *, v16qi)
     v1di __builtin_ia32_pmuludq (v2si, v2si)
     v2di __builtin_ia32_pmuludq128 (v4si, v4si)
     v8hi __builtin_ia32_psllw128 (v8hi, v8hi)
     v4si __builtin_ia32_pslld128 (v4si, v4si)
     v2di __builtin_ia32_psllq128 (v2di, v2di)
     v8hi __builtin_ia32_psrlw128 (v8hi, v8hi)
     v4si __builtin_ia32_psrld128 (v4si, v4si)
     v2di __builtin_ia32_psrlq128 (v2di, v2di)
     v8hi __builtin_ia32_psraw128 (v8hi, v8hi)
     v4si __builtin_ia32_psrad128 (v4si, v4si)
     v2di __builtin_ia32_pslldqi128 (v2di, int)
     v8hi __builtin_ia32_psllwi128 (v8hi, int)
     v4si __builtin_ia32_pslldi128 (v4si, int)
*/

static __inline
v128_u8_t op_PSLLDI128 ( v128_u8_t vin, const int bit_count )
    __attribute__ ((always_inline));

static __inline
v128_u8_t op_PSLLDI128 (v128_u8_t vin, const int bit_count)
{
    return (v128_u8_t)__builtin_ia32_pslldi128 ((v128_i32_t)vin, bit_count);
}

/*
     v2di __builtin_ia32_psllqi128 (v2di, int)
     v2di __builtin_ia32_psrldqi128 (v2di, int)
     v8hi __builtin_ia32_psrlwi128 (v8hi, int)
     v4si __builtin_ia32_psrldi128 (v4si, int)
*/

static __inline
v128_u8_t op_PSRLDI128 ( v128_u8_t vin, const int bit_count )
    __attribute__ ((always_inline));

static __inline
v128_u8_t op_PSRLDI128 (v128_u8_t vin, const int bit_count)
{
    return (v128_u8_t)__builtin_ia32_psrldi128 ((v128_i32_t)vin, bit_count);
}

/*
     v2di __builtin_ia32_psrlqi128 (v2di, int)
     v8hi __builtin_ia32_psrawi128 (v8hi, int)
     v4si __builtin_ia32_psradi128 (v4si, int)
     v4si __builtin_ia32_pmaddwd128 (v8hi, v8hi)
     v2di __builtin_ia32_movq128 (v2di)
*/
static __inline__
v128_u8_t op_PSHUFD128 ( v128_u8_t vin, const int mask )
    __attribute__ ((always_inline));
     
static __inline__
v128_u8_t op_PSHUFD128 (v128_u8_t vin, const int mask)
{
    return (v128_u8_t)__builtin_ia32_pshufd ((v128_i32_t)vin, mask);
}
#endif

#ifdef __SSE3__
/* sse3
     v2df __builtin_ia32_addsubpd (v2df, v2df)
     v4sf __builtin_ia32_addsubps (v4sf, v4sf)
     v2df __builtin_ia32_haddpd (v2df, v2df)
     v4sf __builtin_ia32_haddps (v4sf, v4sf)
     v2df __builtin_ia32_hsubpd (v2df, v2df)
     v4sf __builtin_ia32_hsubps (v4sf, v4sf)
     v16qi __builtin_ia32_lddqu (char const *)
     void __builtin_ia32_monitor (void *, unsigned int, unsigned int)
     v2df __builtin_ia32_movddup (v2df)
     v4sf __builtin_ia32_movshdup (v4sf)
     v4sf __builtin_ia32_movsldup (v4sf)
     void __builtin_ia32_mwait (unsigned int, unsigned int)

v2df __builtin_ia32_loadddup (double const *)

*/

#endif

#ifdef __SSSE3__

/* ssse3
 *
 * The following built-in functions are available when -mssse3 is used. All of
 * them generate the machine instruction that is part of the name with MMX
 * registers.
 *
 * 
     v2si __builtin_ia32_phaddd (v2si, v2si)
     v4hi __builtin_ia32_phaddw (v4hi, v4hi)
     v4hi __builtin_ia32_phaddsw (v4hi, v4hi)
     v2si __builtin_ia32_phsubd (v2si, v2si)
     v4hi __builtin_ia32_phsubw (v4hi, v4hi)
     v4hi __builtin_ia32_phsubsw (v4hi, v4hi)
     v4hi __builtin_ia32_pmaddubsw (v8qi, v8qi)
     v4hi __builtin_ia32_pmulhrsw (v4hi, v4hi)
     v8qi __builtin_ia32_pshufb (v8qi, v8qi)
     v8qi __builtin_ia32_psignb (v8qi, v8qi)
     v2si __builtin_ia32_psignd (v2si, v2si)
     v4hi __builtin_ia32_psignw (v4hi, v4hi)
     v1di __builtin_ia32_palignr (v1di, v1di, int)
     v8qi __builtin_ia32_pabsb (v8qi)
     v2si __builtin_ia32_pabsd (v2si)
     v4hi __builtin_ia32_pabsw (v4hi)
The following built-in functions are available when -mssse3 is used.
All of them generate the machine instruction that is part of the name
with SSE registers.

     v4si __builtin_ia32_phaddd128 (v4si, v4si)
     v8hi __builtin_ia32_phaddw128 (v8hi, v8hi)
     v8hi __builtin_ia32_phaddsw128 (v8hi, v8hi)
     v4si __builtin_ia32_phsubd128 (v4si, v4si)
     v8hi __builtin_ia32_phsubw128 (v8hi, v8hi)
     v8hi __builtin_ia32_phsubsw128 (v8hi, v8hi)
     v8hi __builtin_ia32_pmaddubsw128 (v16qi, v16qi)
     v8hi __builtin_ia32_pmulhrsw128 (v8hi, v8hi)
+    v16qi __builtin_ia32_pshufb128 (v16qi, v16qi)
     v16qi __builtin_ia32_psignb128 (v16qi, v16qi)
     v4si __builtin_ia32_psignd128 (v4si, v4si)
     v8hi __builtin_ia32_psignw128 (v8hi, v8hi)
     v2di __builtin_ia32_palignr128 (v2di, v2di, int)
     v16qi __builtin_ia32_pabsb128 (v16qi)
     v4si __builtin_ia32_pabsd128 (v4si)
     v8hi __builtin_ia32_pabsw128 (v8hi)
 */

static __inline__
v128_u8_t op_PSHUFB128 (register v128_u8_t vin, register const v128_u8_t vtab)
    __attribute__ ((always_inline));

static __inline__
v128_u8_t op_PSHUFB128 (register v128_u8_t vin, register const v128_u8_t vtab)
{
    return (v128_u8_t)__builtin_ia32_pshufb128 ((v128_c_t)vin, (v128_c_t)vtab);
}
#endif

#ifdef __SSE4_1__
/* sse4.1
The following built-in functions are available when -msse4.1 is used.
All of them generate the machine instruction that is part of the name.

     v2df __builtin_ia32_blendpd (v2df, v2df, const int)
     v4sf __builtin_ia32_blendps (v4sf, v4sf, const int)
     v2df __builtin_ia32_blendvpd (v2df, v2df, v2df)
     v4sf __builtin_ia32_blendvps (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_dppd (v2df, v2df, const int)
     v4sf __builtin_ia32_dpps (v4sf, v4sf, const int)
     v4sf __builtin_ia32_insertps128 (v4sf, v4sf, const int)
     v2di __builtin_ia32_movntdqa (v2di *);
     v16qi __builtin_ia32_mpsadbw128 (v16qi, v16qi, const int)
     v8hi __builtin_ia32_packusdw128 (v4si, v4si)
     v16qi __builtin_ia32_pblendvb128 (v16qi, v16qi, v16qi)
     v8hi __builtin_ia32_pblendw128 (v8hi, v8hi, const int)
     v2di __builtin_ia32_pcmpeqq (v2di, v2di)
     v8hi __builtin_ia32_phminposuw128 (v8hi)
     v16qi __builtin_ia32_pmaxsb128 (v16qi, v16qi)
     v4si __builtin_ia32_pmaxsd128 (v4si, v4si)
     v4si __builtin_ia32_pmaxud128 (v4si, v4si)
     v8hi __builtin_ia32_pmaxuw128 (v8hi, v8hi)
     v16qi __builtin_ia32_pminsb128 (v16qi, v16qi)
     v4si __builtin_ia32_pminsd128 (v4si, v4si)
     v4si __builtin_ia32_pminud128 (v4si, v4si)
     v8hi __builtin_ia32_pminuw128 (v8hi, v8hi)
     v4si __builtin_ia32_pmovsxbd128 (v16qi)
     v2di __builtin_ia32_pmovsxbq128 (v16qi)
     v8hi __builtin_ia32_pmovsxbw128 (v16qi)
     v2di __builtin_ia32_pmovsxdq128 (v4si)
     v4si __builtin_ia32_pmovsxwd128 (v8hi)
     v2di __builtin_ia32_pmovsxwq128 (v8hi)
     v4si __builtin_ia32_pmovzxbd128 (v16qi)
     v2di __builtin_ia32_pmovzxbq128 (v16qi)
     v8hi __builtin_ia32_pmovzxbw128 (v16qi)
     v2di __builtin_ia32_pmovzxdq128 (v4si)
     v4si __builtin_ia32_pmovzxwd128 (v8hi)
     v2di __builtin_ia32_pmovzxwq128 (v8hi)
     v2di __builtin_ia32_pmuldq128 (v4si, v4si)
     v4si __builtin_ia32_pmulld128 (v4si, v4si)
     int __builtin_ia32_ptestc128 (v2di, v2di)
     int __builtin_ia32_ptestnzc128 (v2di, v2di)
     int __builtin_ia32_ptestz128 (v2di, v2di)
     v2df __builtin_ia32_roundpd (v2df, const int)
     v4sf __builtin_ia32_roundps (v4sf, const int)
     v2df __builtin_ia32_roundsd (v2df, v2df, const int)
     v4sf __builtin_ia32_roundss (v4sf, v4sf, const int)
The following built-in functions are available when -msse4.1 is used.

v4sf __builtin_ia32_vec_set_v4sf (v4sf, float, const int)
Generates the insertps machine instruction. 
int __builtin_ia32_vec_ext_v16qi (v16qi, const int)
Generates the pextrb machine instruction. 
v16qi __builtin_ia32_vec_set_v16qi (v16qi, int, const int)
Generates the pinsrb machine instruction. 
v4si __builtin_ia32_vec_set_v4si (v4si, int, const int)
Generates the pinsrd machine instruction. 
*/
static __inline__
v128_u8_t op_PINSRUD (register v128_u8_t vin, uint32_t ii, const int off)
    __attribute__ ((always_inline));
static __inline__
v128_u8_t op_PINSRUD (register v128_u8_t vin, uint32_t ii, const int off)
{
    return (v128_u8_t)__builtin_ia32_vec_set_v4si ((v128_i32_t)vin, ii, ( const uint8_t ) off );
}
#define op_PINSRUD( vin, ii, off ) \
    (v128_u8_t) __builtin_ia32_vec_set_v4si ((v128_i32_t)(vin), ii, off)

/*
v2di __builtin_ia32_vec_set_v2di (v2di, long long, const int)
Generates the pinsrq machine instruction in 64bit mode.
The following built-in functions are changed to generate new SSE4.1 instructions when -msse4.1 is used.

float __builtin_ia32_vec_ext_v4sf (v4sf, const int)
Generates the extractps machine instruction. 
int __builtin_ia32_vec_ext_v4si (v4si, const int)
Generates the pextrd machine instruction. 
long long __builtin_ia32_vec_ext_v2di (v2di, const int)
Generates the pextrq machine instruction in 64bit mode.
 */

#endif

#ifdef __SSE4_2__

/* sse4.2
The following built-in functions are available when -msse4.2 is used.
All of them generate the machine instruction that is part of the name.

     v16qi __builtin_ia32_pcmpestrm128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestri128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestria128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestric128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestrio128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestris128 (v16qi, int, v16qi, int, const int)
     int __builtin_ia32_pcmpestriz128 (v16qi, int, v16qi, int, const int)
     v16qi __builtin_ia32_pcmpistrm128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistri128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistria128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistric128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistrio128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistris128 (v16qi, v16qi, const int)
     int __builtin_ia32_pcmpistriz128 (v16qi, v16qi, const int)
     v2di __builtin_ia32_pcmpgtq (v2di, v2di)
The following built-in functions are available when -msse4.2 is used.

unsigned int __builtin_ia32_crc32qi (unsigned int, unsigned char)
Generates the crc32b machine instruction. 
unsigned int __builtin_ia32_crc32hi (unsigned int, unsigned short)
Generates the crc32w machine instruction. 
unsigned int __builtin_ia32_crc32si (unsigned int, unsigned int)
Generates the crc32l machine instruction. 
unsigned long long __builtin_ia32_crc32di (unsigned long long, unsigned long long)
Generates the crc32q machine instruction.
The following built-in functions are changed to generate new SSE4.2 instructions when -msse4.2 is used.

int __builtin_popcount (unsigned int)
Generates the popcntl machine instruction. 
int __builtin_popcountl (unsigned long)
Generates the popcntl or popcntq machine instruction, depending on the size of unsigned long. 
int __builtin_popcountll (unsigned long long)
Generates the popcntq machine instruction.

*/

#endif

/* avx
The following built-in functions are available when -mavx is used.
All of them generate the machine instruction that is part of the name.

     v4df __builtin_ia32_addpd256 (v4df,v4df)
     v8sf __builtin_ia32_addps256 (v8sf,v8sf)
     v4df __builtin_ia32_addsubpd256 (v4df,v4df)
     v8sf __builtin_ia32_addsubps256 (v8sf,v8sf)
     v4df __builtin_ia32_andnpd256 (v4df,v4df)
     v8sf __builtin_ia32_andnps256 (v8sf,v8sf)
     v4df __builtin_ia32_andpd256 (v4df,v4df)
     v8sf __builtin_ia32_andps256 (v8sf,v8sf)
     v4df __builtin_ia32_blendpd256 (v4df,v4df,int)
     v8sf __builtin_ia32_blendps256 (v8sf,v8sf,int)
     v4df __builtin_ia32_blendvpd256 (v4df,v4df,v4df)
     v8sf __builtin_ia32_blendvps256 (v8sf,v8sf,v8sf)
     v2df __builtin_ia32_cmppd (v2df,v2df,int)
     v4df __builtin_ia32_cmppd256 (v4df,v4df,int)
     v4sf __builtin_ia32_cmpps (v4sf,v4sf,int)
     v8sf __builtin_ia32_cmpps256 (v8sf,v8sf,int)
     v2df __builtin_ia32_cmpsd (v2df,v2df,int)
     v4sf __builtin_ia32_cmpss (v4sf,v4sf,int)
     v4df __builtin_ia32_cvtdq2pd256 (v4si)
     v8sf __builtin_ia32_cvtdq2ps256 (v8si)
     v4si __builtin_ia32_cvtpd2dq256 (v4df)
     v4sf __builtin_ia32_cvtpd2ps256 (v4df)
     v8si __builtin_ia32_cvtps2dq256 (v8sf)
     v4df __builtin_ia32_cvtps2pd256 (v4sf)
     v4si __builtin_ia32_cvttpd2dq256 (v4df)
     v8si __builtin_ia32_cvttps2dq256 (v8sf)
     v4df __builtin_ia32_divpd256 (v4df,v4df)
     v8sf __builtin_ia32_divps256 (v8sf,v8sf)
     v8sf __builtin_ia32_dpps256 (v8sf,v8sf,int)
     v4df __builtin_ia32_haddpd256 (v4df,v4df)
     v8sf __builtin_ia32_haddps256 (v8sf,v8sf)
     v4df __builtin_ia32_hsubpd256 (v4df,v4df)
     v8sf __builtin_ia32_hsubps256 (v8sf,v8sf)
     v32qi __builtin_ia32_lddqu256 (pcchar)
     v32qi __builtin_ia32_loaddqu256 (pcchar)
     v4df __builtin_ia32_loadupd256 (pcdouble)
     v8sf __builtin_ia32_loadups256 (pcfloat)
     v2df __builtin_ia32_maskloadpd (pcv2df,v2df)
     v4df __builtin_ia32_maskloadpd256 (pcv4df,v4df)
     v4sf __builtin_ia32_maskloadps (pcv4sf,v4sf)
     v8sf __builtin_ia32_maskloadps256 (pcv8sf,v8sf)
     void __builtin_ia32_maskstorepd (pv2df,v2df,v2df)
     void __builtin_ia32_maskstorepd256 (pv4df,v4df,v4df)
     void __builtin_ia32_maskstoreps (pv4sf,v4sf,v4sf)
     void __builtin_ia32_maskstoreps256 (pv8sf,v8sf,v8sf)
     v4df __builtin_ia32_maxpd256 (v4df,v4df)
     v8sf __builtin_ia32_maxps256 (v8sf,v8sf)
     v4df __builtin_ia32_minpd256 (v4df,v4df)
     v8sf __builtin_ia32_minps256 (v8sf,v8sf)
     v4df __builtin_ia32_movddup256 (v4df)
     int __builtin_ia32_movmskpd256 (v4df)
     int __builtin_ia32_movmskps256 (v8sf)
     v8sf __builtin_ia32_movshdup256 (v8sf)
     v8sf __builtin_ia32_movsldup256 (v8sf)
     v4df __builtin_ia32_mulpd256 (v4df,v4df)
     v8sf __builtin_ia32_mulps256 (v8sf,v8sf)
     v4df __builtin_ia32_orpd256 (v4df,v4df)
     v8sf __builtin_ia32_orps256 (v8sf,v8sf)
     v2df __builtin_ia32_pd_pd256 (v4df)
     v4df __builtin_ia32_pd256_pd (v2df)
     v4sf __builtin_ia32_ps_ps256 (v8sf)
     v8sf __builtin_ia32_ps256_ps (v4sf)
     int __builtin_ia32_ptestc256 (v4di,v4di,ptest)
     int __builtin_ia32_ptestnzc256 (v4di,v4di,ptest)
     int __builtin_ia32_ptestz256 (v4di,v4di,ptest)
     v8sf __builtin_ia32_rcpps256 (v8sf)
     v4df __builtin_ia32_roundpd256 (v4df,int)
     v8sf __builtin_ia32_roundps256 (v8sf,int)
     v8sf __builtin_ia32_rsqrtps_nr256 (v8sf)
     v8sf __builtin_ia32_rsqrtps256 (v8sf)
     v4df __builtin_ia32_shufpd256 (v4df,v4df,int)
     v8sf __builtin_ia32_shufps256 (v8sf,v8sf,int)
     v4si __builtin_ia32_si_si256 (v8si)
     v8si __builtin_ia32_si256_si (v4si)
     v4df __builtin_ia32_sqrtpd256 (v4df)
     v8sf __builtin_ia32_sqrtps_nr256 (v8sf)
     v8sf __builtin_ia32_sqrtps256 (v8sf)
     void __builtin_ia32_storedqu256 (pchar,v32qi)
     void __builtin_ia32_storeupd256 (pdouble,v4df)
     void __builtin_ia32_storeups256 (pfloat,v8sf)
     v4df __builtin_ia32_subpd256 (v4df,v4df)
     v8sf __builtin_ia32_subps256 (v8sf,v8sf)
     v4df __builtin_ia32_unpckhpd256 (v4df,v4df)
     v8sf __builtin_ia32_unpckhps256 (v8sf,v8sf)
     v4df __builtin_ia32_unpcklpd256 (v4df,v4df)
     v8sf __builtin_ia32_unpcklps256 (v8sf,v8sf)
     v4df __builtin_ia32_vbroadcastf128_pd256 (pcv2df)
     v8sf __builtin_ia32_vbroadcastf128_ps256 (pcv4sf)
     v4df __builtin_ia32_vbroadcastsd256 (pcdouble)
     v4sf __builtin_ia32_vbroadcastss (pcfloat)
     v8sf __builtin_ia32_vbroadcastss256 (pcfloat)
     v2df __builtin_ia32_vextractf128_pd256 (v4df,int)
     v4sf __builtin_ia32_vextractf128_ps256 (v8sf,int)
     v4si __builtin_ia32_vextractf128_si256 (v8si,int)
     v4df __builtin_ia32_vinsertf128_pd256 (v4df,v2df,int)
     v8sf __builtin_ia32_vinsertf128_ps256 (v8sf,v4sf,int)
     v8si __builtin_ia32_vinsertf128_si256 (v8si,v4si,int)
     v4df __builtin_ia32_vperm2f128_pd256 (v4df,v4df,int)
     v8sf __builtin_ia32_vperm2f128_ps256 (v8sf,v8sf,int)
     v8si __builtin_ia32_vperm2f128_si256 (v8si,v8si,int)
     v2df __builtin_ia32_vpermil2pd (v2df,v2df,v2di,int)
     v4df __builtin_ia32_vpermil2pd256 (v4df,v4df,v4di,int)
     v4sf __builtin_ia32_vpermil2ps (v4sf,v4sf,v4si,int)
     v8sf __builtin_ia32_vpermil2ps256 (v8sf,v8sf,v8si,int)
     v2df __builtin_ia32_vpermilpd (v2df,int)
     v4df __builtin_ia32_vpermilpd256 (v4df,int)
     v4sf __builtin_ia32_vpermilps (v4sf,int)
     v8sf __builtin_ia32_vpermilps256 (v8sf,int)
     v2df __builtin_ia32_vpermilvarpd (v2df,v2di)
     v4df __builtin_ia32_vpermilvarpd256 (v4df,v4di)
     v4sf __builtin_ia32_vpermilvarps (v4sf,v4si)
     v8sf __builtin_ia32_vpermilvarps256 (v8sf,v8si)
     int __builtin_ia32_vtestcpd (v2df,v2df,ptest)
     int __builtin_ia32_vtestcpd256 (v4df,v4df,ptest)
     int __builtin_ia32_vtestcps (v4sf,v4sf,ptest)
     int __builtin_ia32_vtestcps256 (v8sf,v8sf,ptest)
     int __builtin_ia32_vtestnzcpd (v2df,v2df,ptest)
     int __builtin_ia32_vtestnzcpd256 (v4df,v4df,ptest)
     int __builtin_ia32_vtestnzcps (v4sf,v4sf,ptest)
     int __builtin_ia32_vtestnzcps256 (v8sf,v8sf,ptest)
     int __builtin_ia32_vtestzpd (v2df,v2df,ptest)
     int __builtin_ia32_vtestzpd256 (v4df,v4df,ptest)
     int __builtin_ia32_vtestzps (v4sf,v4sf,ptest)
     int __builtin_ia32_vtestzps256 (v8sf,v8sf,ptest)
     void __builtin_ia32_vzeroall (void)
     void __builtin_ia32_vzeroupper (void)
     v4df __builtin_ia32_xorpd256 (v4df,v4df)
     v8sf __builtin_ia32_xorps256 (v8sf,v8sf)



*/

#ifdef __AES__
/*
 * AES-NI instruction set
 * includes PCMUL
 */

/* AES-NI aes
 */

   
/* v2di __builtin_ia32_aesenc128 (v2di, v2di) */
static __inline__
v128_u8_t op_AESENC (register v128_u8_t state, register const v128_u8_t round_key)
{
    return (v128_u8_t)__builtin_ia32_aesenc128 ((v128_lli_t)state,
                                               (v128_lli_t)round_key);
}


/* v2di __builtin_ia32_aesenclast128 (v2di, v2di) */
static __inline__
v128_u8_t op_AESENCLAST (register v128_u8_t state,
                        register const v128_u8_t round_key)
{
    return (v128_u8_t)__builtin_ia32_aesenclast128 ((v128_lli_t)state,
                                                   (v128_lli_t)round_key);
}


/* v2di __builtin_ia32_aesdec128 (v2di, v2di) */
static __inline__
v128_u8_t op_AESDEC (register v128_u8_t state, register const v128_u8_t round_key)
{
    return (v128_u8_t)__builtin_ia32_aesdec128 ((v128_lli_t)state,
                                               (v128_lli_t)round_key);
}


/* v2di __builtin_ia32_aesdeclast128 (v2di, v2di) */
static __inline__
v128_u8_t op_AESDECLAST (register v128_u8_t state,
                        register const v128_u8_t round_key)
{
    return (v128_u8_t)__builtin_ia32_aesdeclast128 ((v128_lli_t)state,
                                                   (v128_lli_t)round_key);
}


/* v2di __builtin_ia32_aeskeygenassist128 (v2di, const int) */
static __inline__
v128_u8_t op_AESKEYGENASSIST (register const v128_u8_t round_key,
                             const int rcon ) __attribute__ ((always_inline));
static __inline__
v128_u8_t op_AESKEYGENASSIST (register const v128_u8_t round_key,
                             const int rcon )
{
    return (v128_u8_t)__builtin_ia32_aeskeygenassist128 ((v128_lli_t)round_key, rcon);
}

#define op_AESKEYGENASSIST( round_key, rcon ) \
    (v128_u8_t) __builtin_ia32_aeskeygenassist128 ((v128_lli_t) (round_key), rcon)


/* v2di __builtin_ia32_aesimc128 (v2di) */
static __inline__
v128_u8_t op_AESIMC (register const v128_u8_t round_key)
{
    return (v128_u8_t)__builtin_ia32_aesimc128 ((v128_lli_t)round_key);
}

/* pclmul
v2di __builtin_ia32_pclmulqdq128 (v2di, v2di, const int)
*/
#endif

#ifdef __SSE5__
/* sse5
     v2df __builtin_ia32_comeqpd (v2df, v2df)
     v2df __builtin_ia32_comeqps (v2df, v2df)
     v4sf __builtin_ia32_comeqsd (v4sf, v4sf)
     v4sf __builtin_ia32_comeqss (v4sf, v4sf)
     v2df __builtin_ia32_comfalsepd (v2df, v2df)
     v2df __builtin_ia32_comfalseps (v2df, v2df)
     v4sf __builtin_ia32_comfalsesd (v4sf, v4sf)
     v4sf __builtin_ia32_comfalsess (v4sf, v4sf)
     v2df __builtin_ia32_comgepd (v2df, v2df)
     v2df __builtin_ia32_comgeps (v2df, v2df)
     v4sf __builtin_ia32_comgesd (v4sf, v4sf)
     v4sf __builtin_ia32_comgess (v4sf, v4sf)
     v2df __builtin_ia32_comgtpd (v2df, v2df)
     v2df __builtin_ia32_comgtps (v2df, v2df)
     v4sf __builtin_ia32_comgtsd (v4sf, v4sf)
     v4sf __builtin_ia32_comgtss (v4sf, v4sf)
     v2df __builtin_ia32_comlepd (v2df, v2df)
     v2df __builtin_ia32_comleps (v2df, v2df)
     v4sf __builtin_ia32_comlesd (v4sf, v4sf)
     v4sf __builtin_ia32_comless (v4sf, v4sf)
     v2df __builtin_ia32_comltpd (v2df, v2df)
     v2df __builtin_ia32_comltps (v2df, v2df)
     v4sf __builtin_ia32_comltsd (v4sf, v4sf)
     v4sf __builtin_ia32_comltss (v4sf, v4sf)
     v2df __builtin_ia32_comnepd (v2df, v2df)
     v2df __builtin_ia32_comneps (v2df, v2df)
     v4sf __builtin_ia32_comnesd (v4sf, v4sf)
     v4sf __builtin_ia32_comness (v4sf, v4sf)
     v2df __builtin_ia32_comordpd (v2df, v2df)
     v2df __builtin_ia32_comordps (v2df, v2df)
     v4sf __builtin_ia32_comordsd (v4sf, v4sf)
     v4sf __builtin_ia32_comordss (v4sf, v4sf)
     v2df __builtin_ia32_comtruepd (v2df, v2df)
     v2df __builtin_ia32_comtrueps (v2df, v2df)
     v4sf __builtin_ia32_comtruesd (v4sf, v4sf)
     v4sf __builtin_ia32_comtruess (v4sf, v4sf)
     v2df __builtin_ia32_comueqpd (v2df, v2df)
     v2df __builtin_ia32_comueqps (v2df, v2df)
     v4sf __builtin_ia32_comueqsd (v4sf, v4sf)
     v4sf __builtin_ia32_comueqss (v4sf, v4sf)
     v2df __builtin_ia32_comugepd (v2df, v2df)
     v2df __builtin_ia32_comugeps (v2df, v2df)
     v4sf __builtin_ia32_comugesd (v4sf, v4sf)
     v4sf __builtin_ia32_comugess (v4sf, v4sf)
     v2df __builtin_ia32_comugtpd (v2df, v2df)
     v2df __builtin_ia32_comugtps (v2df, v2df)
     v4sf __builtin_ia32_comugtsd (v4sf, v4sf)
     v4sf __builtin_ia32_comugtss (v4sf, v4sf)
     v2df __builtin_ia32_comulepd (v2df, v2df)
     v2df __builtin_ia32_comuleps (v2df, v2df)
     v4sf __builtin_ia32_comulesd (v4sf, v4sf)
     v4sf __builtin_ia32_comuless (v4sf, v4sf)
     v2df __builtin_ia32_comultpd (v2df, v2df)
     v2df __builtin_ia32_comultps (v2df, v2df)
     v4sf __builtin_ia32_comultsd (v4sf, v4sf)
     v4sf __builtin_ia32_comultss (v4sf, v4sf)
     v2df __builtin_ia32_comunepd (v2df, v2df)
     v2df __builtin_ia32_comuneps (v2df, v2df)
     v4sf __builtin_ia32_comunesd (v4sf, v4sf)
     v4sf __builtin_ia32_comuness (v4sf, v4sf)
     v2df __builtin_ia32_comunordpd (v2df, v2df)
     v2df __builtin_ia32_comunordps (v2df, v2df)
     v4sf __builtin_ia32_comunordsd (v4sf, v4sf)
     v4sf __builtin_ia32_comunordss (v4sf, v4sf)
     v2df __builtin_ia32_fmaddpd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fmaddps (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fmaddsd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fmaddss (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fmsubpd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fmsubps (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fmsubsd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fmsubss (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fnmaddpd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fnmaddps (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fnmaddsd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fnmaddss (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fnmsubpd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fnmsubps (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_fnmsubsd (v2df, v2df, v2df)
     v4sf __builtin_ia32_fnmsubss (v4sf, v4sf, v4sf)
     v2df __builtin_ia32_frczpd (v2df)
     v4sf __builtin_ia32_frczps (v4sf)
     v2df __builtin_ia32_frczsd (v2df, v2df)
     v4sf __builtin_ia32_frczss (v4sf, v4sf)
     v2di __builtin_ia32_pcmov (v2di, v2di, v2di)
     v2di __builtin_ia32_pcmov_v2di (v2di, v2di, v2di)
     v4si __builtin_ia32_pcmov_v4si (v4si, v4si, v4si)
     v8hi __builtin_ia32_pcmov_v8hi (v8hi, v8hi, v8hi)
     v16qi __builtin_ia32_pcmov_v16qi (v16qi, v16qi, v16qi)
     v2df __builtin_ia32_pcmov_v2df (v2df, v2df, v2df)
     v4sf __builtin_ia32_pcmov_v4sf (v4sf, v4sf, v4sf)
     v16qi __builtin_ia32_pcomeqb (v16qi, v16qi)
     v8hi __builtin_ia32_pcomeqw (v8hi, v8hi)
     v4si __builtin_ia32_pcomeqd (v4si, v4si)
     v2di __builtin_ia32_pcomeqq (v2di, v2di)
     v16qi __builtin_ia32_pcomequb (v16qi, v16qi)
     v4si __builtin_ia32_pcomequd (v4si, v4si)
     v2di __builtin_ia32_pcomequq (v2di, v2di)
     v8hi __builtin_ia32_pcomequw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomeqw (v8hi, v8hi)
     v16qi __builtin_ia32_pcomfalseb (v16qi, v16qi)
     v4si __builtin_ia32_pcomfalsed (v4si, v4si)
     v2di __builtin_ia32_pcomfalseq (v2di, v2di)
     v16qi __builtin_ia32_pcomfalseub (v16qi, v16qi)
     v4si __builtin_ia32_pcomfalseud (v4si, v4si)
     v2di __builtin_ia32_pcomfalseuq (v2di, v2di)
     v8hi __builtin_ia32_pcomfalseuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomfalsew (v8hi, v8hi)
     v16qi __builtin_ia32_pcomgeb (v16qi, v16qi)
     v4si __builtin_ia32_pcomged (v4si, v4si)
     v2di __builtin_ia32_pcomgeq (v2di, v2di)
     v16qi __builtin_ia32_pcomgeub (v16qi, v16qi)
     v4si __builtin_ia32_pcomgeud (v4si, v4si)
     v2di __builtin_ia32_pcomgeuq (v2di, v2di)
     v8hi __builtin_ia32_pcomgeuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomgew (v8hi, v8hi)
     v16qi __builtin_ia32_pcomgtb (v16qi, v16qi)
     v4si __builtin_ia32_pcomgtd (v4si, v4si)
     v2di __builtin_ia32_pcomgtq (v2di, v2di)
     v16qi __builtin_ia32_pcomgtub (v16qi, v16qi)
     v4si __builtin_ia32_pcomgtud (v4si, v4si)
     v2di __builtin_ia32_pcomgtuq (v2di, v2di)
     v8hi __builtin_ia32_pcomgtuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomgtw (v8hi, v8hi)
     v16qi __builtin_ia32_pcomleb (v16qi, v16qi)
     v4si __builtin_ia32_pcomled (v4si, v4si)
     v2di __builtin_ia32_pcomleq (v2di, v2di)
     v16qi __builtin_ia32_pcomleub (v16qi, v16qi)
     v4si __builtin_ia32_pcomleud (v4si, v4si)
     v2di __builtin_ia32_pcomleuq (v2di, v2di)
     v8hi __builtin_ia32_pcomleuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomlew (v8hi, v8hi)
     v16qi __builtin_ia32_pcomltb (v16qi, v16qi)
     v4si __builtin_ia32_pcomltd (v4si, v4si)
     v2di __builtin_ia32_pcomltq (v2di, v2di)
     v16qi __builtin_ia32_pcomltub (v16qi, v16qi)
     v4si __builtin_ia32_pcomltud (v4si, v4si)
     v2di __builtin_ia32_pcomltuq (v2di, v2di)
     v8hi __builtin_ia32_pcomltuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomltw (v8hi, v8hi)
     v16qi __builtin_ia32_pcomneb (v16qi, v16qi)
     v4si __builtin_ia32_pcomned (v4si, v4si)
     v2di __builtin_ia32_pcomneq (v2di, v2di)
     v16qi __builtin_ia32_pcomneub (v16qi, v16qi)
     v4si __builtin_ia32_pcomneud (v4si, v4si)
     v2di __builtin_ia32_pcomneuq (v2di, v2di)
     v8hi __builtin_ia32_pcomneuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomnew (v8hi, v8hi)
     v16qi __builtin_ia32_pcomtrueb (v16qi, v16qi)
     v4si __builtin_ia32_pcomtrued (v4si, v4si)
     v2di __builtin_ia32_pcomtrueq (v2di, v2di)
     v16qi __builtin_ia32_pcomtrueub (v16qi, v16qi)
     v4si __builtin_ia32_pcomtrueud (v4si, v4si)
     v2di __builtin_ia32_pcomtrueuq (v2di, v2di)
     v8hi __builtin_ia32_pcomtrueuw (v8hi, v8hi)
     v8hi __builtin_ia32_pcomtruew (v8hi, v8hi)
     v4df __builtin_ia32_permpd (v2df, v2df, v16qi)
     v4sf __builtin_ia32_permps (v4sf, v4sf, v16qi)
     v4si __builtin_ia32_phaddbd (v16qi)
     v2di __builtin_ia32_phaddbq (v16qi)
     v8hi __builtin_ia32_phaddbw (v16qi)
     v2di __builtin_ia32_phadddq (v4si)
     v4si __builtin_ia32_phaddubd (v16qi)
     v2di __builtin_ia32_phaddubq (v16qi)
     v8hi __builtin_ia32_phaddubw (v16qi)
     v2di __builtin_ia32_phaddudq (v4si)
     v4si __builtin_ia32_phadduwd (v8hi)
     v2di __builtin_ia32_phadduwq (v8hi)
     v4si __builtin_ia32_phaddwd (v8hi)
     v2di __builtin_ia32_phaddwq (v8hi)
     v8hi __builtin_ia32_phsubbw (v16qi)
     v2di __builtin_ia32_phsubdq (v4si)
     v4si __builtin_ia32_phsubwd (v8hi)
     v4si __builtin_ia32_pmacsdd (v4si, v4si, v4si)
     v2di __builtin_ia32_pmacsdqh (v4si, v4si, v2di)
     v2di __builtin_ia32_pmacsdql (v4si, v4si, v2di)
     v4si __builtin_ia32_pmacssdd (v4si, v4si, v4si)
     v2di __builtin_ia32_pmacssdqh (v4si, v4si, v2di)
     v2di __builtin_ia32_pmacssdql (v4si, v4si, v2di)
     v4si __builtin_ia32_pmacsswd (v8hi, v8hi, v4si)
     v8hi __builtin_ia32_pmacssww (v8hi, v8hi, v8hi)
     v4si __builtin_ia32_pmacswd (v8hi, v8hi, v4si)
     v8hi __builtin_ia32_pmacsww (v8hi, v8hi, v8hi)
     v4si __builtin_ia32_pmadcsswd (v8hi, v8hi, v4si)
     v4si __builtin_ia32_pmadcswd (v8hi, v8hi, v4si)
     v16qi __builtin_ia32_pperm (v16qi, v16qi, v16qi)
     v16qi __builtin_ia32_protb (v16qi, v16qi)
     v4si __builtin_ia32_protd (v4si, v4si)
     v2di __builtin_ia32_protq (v2di, v2di)
     v8hi __builtin_ia32_protw (v8hi, v8hi)
     v16qi __builtin_ia32_pshab (v16qi, v16qi)
     v4si __builtin_ia32_pshad (v4si, v4si)
     v2di __builtin_ia32_pshaq (v2di, v2di)
     v8hi __builtin_ia32_pshaw (v8hi, v8hi)
     v16qi __builtin_ia32_pshlb (v16qi, v16qi)
     v4si __builtin_ia32_pshld (v4si, v4si)
     v2di __builtin_ia32_pshlq (v2di, v2di)
     v8hi __builtin_ia32_pshlw (v8hi, v8hi)
The following builtin-in functions are available when -msse5 is used.
The second argument must be an integer constant and generate the machine
instruction that is part of the name with the `_imm' suffix removed.

     v16qi __builtin_ia32_protb_imm (v16qi, int)
     v4si __builtin_ia32_protd_imm (v4si, int)
     v2di __builtin_ia32_protq_imm (v2di, int)
     v8hi __builtin_ia32_protw_imm (v8hi, int)


*/
#endif

#endif /* if USE_VECREG */

#ifdef __cplusplus
}
#endif

#endif /* _h_v128_ */
