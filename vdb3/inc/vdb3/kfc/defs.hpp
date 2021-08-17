/*

  vdb3.kfc.defs


 */

#pragma once

/* The ISO C99 standard specifies that in C++ implementations these
   macros should only be defined if explicitly requested.  */

// where "these macros" are definitions of limits
#if ! MAC && ! defined __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif

#if 0
// to get "secure" functions such as memset_s(), need to ask for them
#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cassert>

#include <memory>

#if LINUX

#include <endian.h>
#include <byteswap.h>

#elif MAC

#include <machine/endian.h>
#include <architecture/byte_order.h>

#endif

// Real numbers
typedef float                    R32;
typedef double                   R64;

#if HAVE_R128
typedef __float128              R128;
#endif

// Integers
typedef signed char               Z8;
typedef signed short int         Z16;
typedef signed int               Z32;
typedef signed long long int     Z64;

// Natural numbers
typedef unsigned char             N8;
typedef unsigned short int       N16;
typedef unsigned int             N32;
typedef unsigned long long int   N64;

// untyped byte
typedef N8                    byte_t;

// text
typedef char           ASCII7, ASCII;
typedef char                    UTF8;
typedef N16                    UTF16;
typedef N32                    UTF32;

// units
#if USE_UNIT_CLASSES
class bitsz_t;
class bytes_t;
#else
typedef N64                  bitsz_t;
typedef N64                  bytes_t;
#endif

// measurement and designation by elements
typedef Z64                  index_t;
typedef N64                  count_t;

// Real constants
const R32 R32_EQ_EPSILON     = ( R32 ) 0.000001;
const R64 R64_R32_EQ_EPSILON = ( R64 ) 0.000001;
const R64 R64_EQ_EPSILON     = ( R64 ) 0.00000000000001;

// Natural constants
const N8   N8_MAX = ~ ( N8  ) 0;
const N16 N16_MAX = ~ ( N16 ) 0;
const N32 N32_MAX = ~ ( N32 ) 0;
const N64 N64_MAX = ~ ( N64 ) 0;

// often used constant
#undef SIZE_T_MAX
const size_t SIZE_T_MAX = ~ ( size_t ) 0;

// Integer constants
// we continue to support asymmetric limits
const Z8   Z8_MAX = ( ( Z8  ) ( N8_MAX >> 1 ) );
const Z8   Z8_MIN = - Z8_MAX - 1;
const Z16 Z16_MAX = ( ( Z16 ) ( N16_MAX >> 1 ) );
const Z16 Z16_MIN = - Z16_MAX - 1;
const Z32 Z32_MAX = ( ( Z32 ) ( N32_MAX >> 1 ) );
const Z32 Z32_MIN = - Z32_MAX - 1;
const Z64 Z64_MAX = ( ( Z64 ) ( N64_MAX >> 1 ) );
const Z64 Z64_MIN = - Z64_MAX - 1;

#define self ( * this )
#define interface struct

#if HAVE_Z128

// 128-bit Integer and Natural types
typedef   signed __int128       Z128;
typedef unsigned __int128       N128;

// 128-bit Integer and Natural limits
const N128 N128_MAX = ~ ( N128 ) 0;
const Z128 Z128_MAX = ( ( Z128 ) ( N128_MAX >> 1 ) );
const Z128 Z128_MIN = - Z128_MAX - 1;

#else

#include <vdb3/kfc/z128.hpp>

#endif

#define MSG( decl ) virtual decl = 0
#define FMSG( decl ) static decl
#define IMSG( decl ) virtual decl
#define METH( decl ) virtual decl override

// debug execution that goes away when debugging is turned off
#if DEBUG
    #define DBGX( x ) x
#else
    #define DBGX( x ) ( ( void ) 0 )
#endif

#if MAC

#define __LITTLE_ENDIAN   LITTLE_ENDIAN
#define __BIG_ENDIAN      BIG_ENDIAN
#define __PDP_ENDIAN      PDP_ENDIAN
#define __BYTE_ORDER      BYTE_ORDER

#define __bswap_16(x) OSSwapInt16 (x)
#define __bswap_32(x) OSSwapInt32 (x)
#define __bswap_64(x) OSSwapInt64 (x)

#endif

#if HAVE_Z128
Z128 __bswap_128 ( N128 n ) noexcept;
#endif

#if USE_UNIT_CLASSES
#include <vdb3/kfc/integer.hpp>
#endif

namespace vdb3
{
    class Timestamp;
    struct RsrcTime;
    struct RsrcMem;
    typedef RsrcMem RsrcKfc;

    static inline bool R32_equal ( R32 a, R32 b )
    {
        R64 abs_diff = ( a >= b ) ? ( ( R64 ) a - ( R64 ) b ) : ( ( R64 ) b - ( R64 ) a );
        return abs_diff <= R64_R32_EQ_EPSILON;
    }

    static inline bool R64_equal ( R64 a, R64 b )
    {
        R64 abs_diff = ( a >= b ) ? ( a - b ) : ( b - a );
        return abs_diff <= R64_EQ_EPSILON;
    }

    template < class T >
    inline bool Real_equal ( T a, T b );

    template <> inline bool Real_equal < R32 > ( R32 a, R32 b )
    { return R32_equal ( a, b ); }

    template <> inline bool Real_equal < R64 > ( R64 a, R64 b )
    { return R64_equal ( a, b ); }
}

#define CPP_CONCAT( a, b )                                              \
    a ## b

#define CXX_COPY_REF( cls )                                             \
    cls ( const CPP_CONCAT ( cls, Ref ) &  );                           \
    cls & operator = ( const CPP_CONCAT ( cls, Ref ) & r )

#define CXX_COPY_REF_NE( cls )                                          \
    cls ( const CPP_CONCAT ( cls, Ref ) &  ) noexcept;                  \
    cls & operator = ( const CPP_CONCAT ( cls, Ref ) & r ) noexcept

#define CXX_RULE_OF_FOUR( cls )                                         \
    cls ( const cls & o );                                              \
    cls & operator = ( const cls & o );                                 \
    cls ( cls && o );                                                   \
    cls & operator = ( cls && o )

#define CXX_RULE_OF_FIVE( cls )                                         \
    CXX_RULE_OF_FOUR ( cls );                                           \
    cls ()

#define CXX_RULE_OF_SIX( cls )                                          \
    CXX_RULE_OF_FIVE ( cls );                                           \
    ~ cls () noexcept

#define CXX_RULE_OF_SIX_VD( cls )                                       \
    CXX_RULE_OF_FIVE ( cls );                                           \
    virtual ~ cls () noexcept

#define CXX_RULE_OF_EIGHT( cls )                                       \
    CXX_RULE_OF_SIX ( cls );                                           \
    CXX_COPY_REF ( cls )

#define CXX_RULE_OF_EIGHT_VD( cls )                                     \
    CXX_RULE_OF_SIX_VD ( cls );                                         \
    CXX_COPY_REF ( cls )

#define CXX_RULE_OF_FOUR_NE( cls )                                      \
    cls ( const cls & o ) noexcept;                                     \
    cls & operator = ( const cls & o ) noexcept;                        \
    cls ( cls && o ) noexcept;                                          \
    cls & operator = ( cls && o ) noexcept

#define CXX_RULE_OF_FIVE_NE( cls )                                      \
    CXX_RULE_OF_FOUR_NE ( cls );                                        \
    cls () noexcept

#define CXX_RULE_OF_SIX_NE( cls )                                       \
    CXX_RULE_OF_FIVE_NE ( cls );                                        \
    ~ cls () noexcept

#define CXX_RULE_OF_SIX_VDNE( cls )                                     \
    CXX_RULE_OF_FIVE_NE ( cls );                                        \
    virtual ~ cls () noexcept

#define CXX_REF_EQ_OPS( cls, ref )                                      \
    inline bool operator ! () const noexcept                            \
    { return ! ref; }                                                   \
    inline bool operator == ( const cls & obj ) const noexcept          \
    { return ref == obj . ref; }                                        \
    inline bool operator != ( const cls & obj ) const noexcept          \
    { return ref != ( obj . ref ); }

#define CXX_REF_CMP_OPS( cls, ref )                                     \
    CXX_REF_EQ_OPS ( cls, ref )                                         \
    inline bool operator < ( const cls & obj ) const noexcept           \
    { return ref < obj . ref; }                                         \
    inline bool operator <= ( const cls & obj ) const noexcept          \
    { return ref <= ( obj . ref ); }                                    \
    inline bool operator >= ( const cls & obj ) const noexcept          \
    { return ref >= ( obj . ref ); }                                    \
    inline bool operator > ( const cls & obj ) const noexcept           \
    { return ref > obj . ref; }

#define CXX_RULE_OF_EIGHT_NE( cls )                                     \
    CXX_RULE_OF_SIX_NE ( cls );                                         \
    CXX_COPY_REF_NE ( cls )

#define CXX_RULE_OF_EIGHT_NEVD( cls )                                   \
    CXX_RULE_OF_SIX_NEVD ( cls );                                       \
    CXX_COPY_REF_NE ( cls )

#define CXX_RULE_OF_EIGHT_EQOPS_NE( cls, ref )                          \
    CXX_REF_EQ_OPS ( cls, ref )                                         \
    CXX_RULE_OF_SIX_NE ( cls );                                         \
    CXX_COPY_REF_NE ( cls )

#define CXX_RULE_OF_EIGHT_EQOPS_NEVD( cls, ref )                        \
    CXX_REF_EQ_OPS ( cls, ref )                                         \
    CXX_RULE_OF_SIX_NEVD ( cls );                                       \
    CXX_COPY_REF_NE ( cls )

#define CXX_RULE_OF_EIGHT_CMPOPS_NE( cls, ref )                         \
    CXX_REF_CMP_OPS ( cls, ref )                                        \
    CXX_RULE_OF_SIX_NE ( cls );                                         \
    CXX_COPY_REF_NE ( cls )

#define CXX_RULE_OF_EIGHT_CMPOPS_NEVD( cls, ref )                       \
    CXX_REF_CMP_OPS ( cls, ref )                                        \
    CXX_RULE_OF_SIX_NEVD ( cls );                                       \
    CXX_COPY_REF_NE ( cls )
