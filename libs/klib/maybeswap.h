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

#ifndef _h_maybeswap_
#define _h_maybeswap_

/* technically, a whitelist would be safer */
#ifdef __sparc
#  if SWAP_PERSISTED
#    define GET_UNALIGNED( type, swap, dest, src )                  \
        do {                                                        \
            type tmp##__LINE__;                                     \
            memcpy ( & tmp##__LINE__, & ( src ), sizeof ( type ) ); \
            dest = swap ( tmp##__LINE__ );                          \
        } while (0)
#  else
#    define GET_UNALIGNED( type, swap, dest, src ) \
        memcpy ( & ( dest ), & ( src ), sizeof ( type ) )
#  endif
#elif SWAP_PERSISTED
#  define GET_UNALIGNED( type, swap, dest, src ) dest = swap ( src )
#else
#  define GET_UNALIGNED( type, swap, dest, src ) \
      dest = ( * ( const type* ) & ( src ) )
#endif

#include <byteswap.h>

#define GET16( dest, src ) GET_UNALIGNED ( uint16_t, bswap_16, dest, src )
#define GET32( dest, src ) GET_UNALIGNED ( uint32_t, bswap_32, dest, src )
#define GET64( dest, src ) GET_UNALIGNED ( uint64_t, bswap_64, dest, src )

#endif /* _h_maybeswap_ */

