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

#ifndef _h_byteswap_
#define _h_byteswap_

#ifdef _BYTESWAP_H
#warning "GNU byteswap.h being used"
#else
#define _BYTESWAP_H	1234

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* perform single instruction byte swap */
static __inline__ uint16_t bswap_16 ( uint16_t i )
{
    register uint16_t rtn;
    __asm__
    (
        "rorw $8, %w0"
        : "=r" ( rtn )
        : "0" ( i )
        : "cc"
    );
    return rtn;
}

/* perform single instruction byte swap */
static __inline__ uint32_t bswap_32 ( uint32_t i )
{
    register uint32_t rtn;
    __asm__
    (
        "bswap %0"
        : "=r" ( rtn )
        : "0" ( i )
        : "cc"
    );
    return rtn;
}

/* perform multi-instruction byte swap */
static __inline__ uint64_t bswap_64 ( uint64_t i )
{
    register uint64_t rtn;
    __asm__
    (
        "bswap %q0"
        : "=r" ( rtn )
        : "0" ( i )
        : "cc"
    );
    return rtn;
}


#ifdef __cplusplus
}
#endif

#endif /* _BYTESWAP_H */
#endif /* _h_byteswap_ */
