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

#ifndef _h_klib_logfmt_
#define _h_klib_logfmt_

#ifdef __cplusplus
extern "C" {
#endif

#define PLOG_NAME(name)   #name
#define	PLOG_C(name)	  PLOG_NAME(name) "=%c"
#define	PLOG_S(name)	  PLOG_NAME(name) "=%s"
#define	PLOG_I8(name)	  PLOG_NAME(name) "=%hhd"
#define	PLOG_U8(name)	  PLOG_NAME(name) "=%hhu"
#define	PLOG_X8(name)	  PLOG_NAME(name) "=0x%2.2hhX"
#define	PLOG_I16(name)	  PLOG_NAME(name) "=%hd"
#define	PLOG_U16(name)	  PLOG_NAME(name) "=%hu"
#define	PLOG_X16(name)	  PLOG_NAME(name) "=0x%4.4hX"
#define	PLOG_I32(name)	  PLOG_NAME(name) "=%d"
#define	PLOG_U32(name)	  PLOG_NAME(name) "=%u"
#define	PLOG_X32(name)	  PLOG_NAME(name) "=0x%8.8X"
#define	PLOG_I64(name)	  PLOG_NAME(name) "=%ld"
#define	PLOG_U64(name)	  PLOG_NAME(name) "=%lu"
#define	PLOG_X64(name)	  PLOG_NAME(name) "=0x%16.16lX"
#define	PLOG_PDIFF(name)  PLOG_NAME(name) "=%zd"
#define	PLOG_SIZE(name)	  PLOG_NAME(name) "=0x%zu"
#define PLOG_P(name)      PLOG_NAME(name) "=%p"
/* use the above to fill in the below */
#define PLOG_2(a,b)		    a "," b
#define PLOG_3(a,b,c)		a "," b "," c
#define PLOG_4(a,b,c,d)		a "," b "," c "," d
#define PLOG_5(a,b,c,d,e)	a "," b "," c "," d "," e

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_logfmt_ */
