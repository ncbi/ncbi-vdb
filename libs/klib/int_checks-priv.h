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

#ifndef _h_lib_int_checks_

#include <stdint.h>

#define MASK_PROHIBITED_BITS_INT  ( ~((uint64_t)( (unsigned int)~0 >> 1 )) )
#define MASK_PROHIBITED_BITS_INT32  ( ~((uint64_t)( (uint32_t)~0 >> 1 )) )
#define MASK_PROHIBITED_BITS_INT16  ( ~((uint64_t)( (uint16_t)~0 >> 1 )) )
#define MASK_PROHIBITED_BITS_INT8  ( ~((uint64_t)( (uint8_t)~0 >> 1 )) )

/* works in an assumption that size_t is always unsigned, which, it seems, is not explicitly guaranteed by the standard */
#define MASK_PROHIBITED_BITS_SIZE_T  ( ~((uint64_t)( (size_t)~0 >> 1 )) )

/* v can have bits set only for int32_t/size_t [0, 0x7f..ff] */
#define FITS_INTO_INT(v) ( 0 == ((v) & MASK_PROHIBITED_BITS_INT ) )
#define FITS_INTO_INT32(v) ( 0 == ((v) & MASK_PROHIBITED_BITS_INT32 ) )
#define FITS_INTO_INT16(v) ( 0 == ((v) & MASK_PROHIBITED_BITS_INT16 ) )
#define FITS_INTO_INT8(v) ( 0 == ((v) & MASK_PROHIBITED_BITS_INT8 ) )
#define FITS_INTO_SIZE_T(v) ( 0 == ((v) & MASK_PROHIBITED_BITS_SIZE_T ) )

#endif /* _lib_int_checks_h_ */