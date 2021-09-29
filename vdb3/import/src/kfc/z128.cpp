/*

  vdb3.kfc.z128


 */

#include <vdb3/kfc/z128.hpp>

#if ! HAVE_Z128

#include <stdexcept>

// construction parameter ordering macro!
// so that the C++ compiler doesn't yell at me...
#if BYTE_ORDER == LITTLE_ENDIAN
#define CNST( _hi, _low ) low ( _low ), hi ( _hi )
#else
#define CNST( _hi, _low ) hi ( _hi ), low ( _low )
#endif

// this is undefined, or we can say compiler-defined
#define NEG_SHIFT_REVERSES_DIRECTION 1

// assembly
template < class T > inline
N32 T_add ( T * a, T b ) noexcept
{
    T sum;
    N32 carry;
    __asm__ __volatile__
    (
#if armv7l
        "mov %0, #0;"
        "ldrd %1, [%3];"
        "adds %Q1, %Q1, %Q4;"
        "adcs %R1, %R1, %R4;"
        "adc %0, %0, #0;"
        "strd %1, [%3];"
#else
  #error "unknown architecture"
#endif
        : "=&r" ( carry ), "=&r" ( sum ), "+Qo" ( a )
        : "r" ( a ), "Ir" ( b )
        : "cc"
    );

    return carry;
}

inline
N32 Z64_add ( Z64 * a, Z64 b ) noexcept
{
    return T_add < Z64 > ( a, b );
}

inline
N32 N64_add ( N64 * a, N64 b ) noexcept
{
    return T_add < N64 > ( a, b );
}
    
template < class T > inline
N32 T_adc ( T * a, T b, N32 c ) noexcept
{
    T sum;
    N32 carry;
    __asm__ __volatile__
    (
#if armv7l
        "mov %0, #0;"
        "ldrd %1, [%3];"
        "lsrs %5, %5, #1;"
        "adcs %Q1, %Q1, %Q4;"
        "adcs %R1, %R1, %R4;"
        "adc %0, %0, #0;"
        "strd %1, [%3];"
#else
  #error "unknown architecture"
#endif
        : "=&r" ( carry ), "=&r" ( sum ), "+Qo" ( a )
        : "r" ( a ), "Ir" ( b ), "Ir" ( c )
        : "cc"
    );

    return carry;
}

inline
N32 Z64_adc ( Z64 * a, Z64 b, N32 c ) noexcept
{
    return T_adc < Z64 > ( a, b, c );
}

inline
N32 N64_adc ( N64 * a, N64 b, N32 c ) noexcept
{
    return T_adc < N64 > ( a, b, c );
}

template < class T > inline
N32 T_sub ( T * a, T b ) noexcept
{
    T diff;
    N32 carry;
    __asm__ __volatile__
    (
#if armv7l
        "mov %0, #0;"
        "ldrd %1, [%3];"
        "subs %Q1, %Q1, %Q4;"
        "sbcs %R1, %R1, %R4;"
        "adc %0, %0, #0;"
        "strd %1, [%3];"
#else
  #error "unknown architecture"
#endif
        : "=&r" ( carry ), "=&r" ( diff ), "+Qo" ( a )
        : "r" ( a ), "Ir" ( b )
        : "cc"
    );

    return carry;
}

inline
N32 Z64_sub ( Z64 * a, Z64 b ) noexcept
{
    return T_sub < Z64 > ( a, b );
}

inline
N32 N64_sub ( N64 * a, N64 b ) noexcept
{
    return T_sub < N64 > ( a, b );
}
    
template < class T > inline
N32 T_sbb ( T * a, T b, N32 c ) noexcept
{
    T diff;
    N32 carry;
    __asm__ __volatile__
    (
#if armv7l
        "mov %0, #0;"
        "ldrd %1, [%3];"
        "lsrs %5, %5, #1;"
        "sbcs %Q1, %Q1, %Q4;"
        "sbcs %R1, %R1, %R4;"
        "adc %0, %0, #0;"
        "strd %1, [%3];"
#else
  #error "unknown architecture"
#endif
        : "=&r" ( carry ), "=&r" ( diff ), "+Qo" ( a )
        : "r" ( a ), "Ir" ( b ), "Ir" ( c )
        : "cc"
    );

    return carry;
}

inline
N32 Z64_sbb ( Z64 * a, Z64 b, N32 c ) noexcept
{
    return T_sbb < Z64 > ( a, b, c );
}

inline
N32 N64_sbb ( N64 * a, N64 b, N32 c ) noexcept
{
    return T_sbb < N64 > ( a, b, c );
}

/*=========================================================*
 *                        Z128Base                         *
 *=========================================================*/

// casts
Z64 Z128Base :: toZ64 () const noexcept
{
    // this will lop off top 64 bits unconditionally
    return ( Z64 ) self . low;
}

N64 Z128Base :: toN64 () const noexcept
{
    // this will lop off top 64 bits unconditionally
    return self . low;
}

// lt
bool Z128Base :: lt ( Z64 val ) const noexcept
{
    return self . lt ( Z128Base ( val ) );
}

bool Z128Base :: lt ( const Z128Base & val ) const noexcept
{
    if ( self . hi != val . hi )
        return self . hi < val . hi;
    return self . low < val . low;
}

bool Z128Base :: lt ( const N128Base & val ) const noexcept
{
    if ( ( N64 ) self . hi != val . hi )
        return ( N64 ) self . hi < val . hi;
    return self . low < val . low;
}


// eq
bool Z128Base :: eq ( Z64 val ) const noexcept
{
    return self . eq ( Z128Base ( val ) );
}

bool Z128Base :: eq ( const Z128Base & val ) const noexcept
{
    return self . hi == val . hi && self . low == val . low;
}

bool Z128Base :: eq ( const N128Base & val ) const noexcept
{
    return ( N64 ) self . hi == val . hi && self . low == val . low;
}


// gt
bool Z128Base :: gt ( Z64 val ) const noexcept
{
    return self . gt ( Z128Base ( val ) );
}

bool Z128Base :: gt ( const Z128Base & val ) const noexcept
{
    if ( self . hi != val . hi )
        return self . hi > val . hi;
    return self . low > val . low;
}

bool Z128Base :: gt ( const N128Base & val ) const noexcept
{
    if ( ( N64 ) self . hi != val . hi )
        return ( N64 ) self . hi > val . hi;
    return self . low > val . low;
}


// unary
void Z128Base :: negate () noexcept
{
    Z128Base zero ( 0LL, 0ULL );
    zero . sub ( self );
    self . cpy ( zero );
}


// add
void Z128Base :: add ( Z64 val ) noexcept
{
    N32 carry = N64_add ( & self . low, val );
    Z64_adc ( & self . hi, val < 0 ? -1LL : 0LL, carry );
}

void Z128Base :: add ( const Z128Base & val ) noexcept
{
    N32 carry = N64_add ( & self . low, val . low );
    Z64_adc ( & self . hi, val . hi, carry );
}

void Z128Base :: add ( const N128Base & val ) noexcept
{
    N32 carry = N64_add ( & self . low, val . low );
    Z64_adc ( & self . hi, ( Z64 ) val . hi, carry );
}


// sub
void Z128Base :: sub ( Z64 val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val );
    Z64_sbb ( & self . hi, val < 0 ? -1LL : 0LL, borrow );
}

void Z128Base :: sub ( const Z128Base & val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val . low );
    Z64_sbb ( & self . hi, val . hi, borrow );
}

void Z128Base :: sub ( const N128Base & val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val . low );
    Z64_sbb ( & self . hi, ( Z64 ) val . hi, borrow );
}


// mul
void Z128Base :: mul ( Z64 val ) noexcept
{
    Z128Base copy ( val );
    self . mul ( copy );
}

void Z128Base :: mul ( const Z128Base & val ) noexcept
{
    Z128Base abs_self ( self );
    Z128Base abs_val ( val );
    
    bool need_negate;
    if ( self . hi < 0 )
    {
        need_negate = true;
        abs_self . negate ();

        if ( val . hi < 0 )
        {
            need_negate = false;
            abs_val . negate ();
        }
    }
    else
    {
        need_negate = false;

        if ( val . hi < 0 )
        {
            need_negate = true;
            abs_val . negate ();
        }
    }

    N128Base nself ( abs_self );
    N128Base nval ( abs_val );

    nself . mul ( nval );

    abs_self . cpy ( nself );

    if ( need_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}

void Z128Base :: mul ( const N128Base & nval ) noexcept
{
    Z128Base abs_self ( self );
    
    bool need_negate = false;
    if ( self . hi < 0 )
    {
        need_negate = true;
        abs_self . negate ();
    }

    N128Base nself ( abs_self );
    nself . mul ( nval );

    abs_self . cpy ( nself );

    if ( need_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}


// div
void Z128Base :: div ( Z64 val )
{
    Z128Base copy ( val );
    self . div ( copy );
}

void Z128Base :: div ( const Z128Base & val )
{
    Z128Base abs_self ( self );
    Z128Base abs_val ( val );
    
    bool need_negate;
    if ( self . hi < 0 )
    {
        need_negate = true;
        abs_self . negate ();

        if ( val . hi < 0 )
        {
            need_negate = false;
            abs_val . negate ();
        }
    }
    else
    {
        need_negate = false;

        if ( val . hi < 0 )
        {
            need_negate = true;
            abs_val . negate ();
        }
    }

    N128Base nself ( abs_self );
    N128Base nval ( abs_val );

    nself . div ( nval );

    abs_self . cpy ( nself );

    if ( need_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}

void Z128Base :: div ( const N128Base & nval )
{
    Z128Base abs_self ( self );
    
    bool need_negate = false;
    if ( self . hi < 0 )
    {
        need_negate = true;
        abs_self . negate ();
    }

    N128Base nself ( abs_self );

    nself . div ( nval );

    abs_self . cpy ( nself );

    if ( need_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}


// mod
void Z128Base :: mod ( Z64 val )
{
    Z128Base copy ( val );
    self . mod ( copy );
}

void Z128Base :: mod ( const Z128Base & val )
{
    Z128Base abs_self ( self );
    Z128Base abs_val ( val );

    bool needs_negate;
    if ( self . hi < 0 )
    {
        needs_negate = true;
        abs_self . negate ();

        if ( val . hi < 0 )
            abs_val . negate ();
    }
    else
    {
        needs_negate = false;

        if ( val . hi < 0 )
            abs_val . negate ();
    }

    N128Base nself ( abs_self );
    N128Base nval ( abs_val );

    nself . mod ( nval );

    abs_self . cpy ( nself );

    if ( needs_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}

void Z128Base :: mod ( const N128Base & nval )
{
    Z128Base abs_self ( self );

    bool needs_negate = false;
    if ( self . hi < 0 )
    {
        needs_negate = true;
        abs_self . negate ();
    }

    N128Base nself ( abs_self );

    nself . mod ( nval );

    abs_self . cpy ( nself );

    if ( needs_negate )
        abs_self . negate ();

    self . cpy ( abs_self );
}


// band
void Z128Base :: band ( Z64 val ) noexcept
{
    self . low &= ( N64 ) val;
    if ( val >= 0 )
        self . hi = 0LL;
}

void Z128Base :: band ( const Z128Base & val ) noexcept
{
    self . low &= val . low;
    self . hi &= val . hi;
}

void Z128Base :: band ( const N128Base & val ) noexcept
{
    self . low &= val . low;
    self . hi &= ( Z64 ) val . hi;
}


// bor
void Z128Base :: bor ( Z64 val ) noexcept
{
    self . low |= ( N64 ) val;
    if ( val < 0 )
        self . hi = -1LL;
}

void Z128Base :: bor ( const Z128Base & val ) noexcept
{
    self . low |= val . low;
    self . hi |= val . hi;
}

void Z128Base :: bor ( const N128Base & val ) noexcept
{
    self . low |= val . low;
    self . hi |= ( Z64 ) val . hi;
}


// bxor
void Z128Base :: bxor ( Z64 val ) noexcept
{
    self . low ^= ( N64 ) val;
    if ( val < 0 )
        self . hi = ~ self . hi;
}

void Z128Base :: bxor ( const Z128Base & val ) noexcept
{
    self . low ^= val . low;
    self . hi ^= val . hi;
}

void Z128Base :: bxor ( const N128Base & val ) noexcept
{
    self . low ^= val . low;
    self . hi ^= ( Z64 ) val . hi;
}


// shl
void Z128Base :: shl ( N32 val ) noexcept
{
    if ( val >= 128 )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else if ( val >= 64 )
    {
        self . hi = ( Z64 ) ( self . low << ( 64 - val ) );
        self . low = 0ULL;
    }
    else
    {
        self . hi <<= val;
        self . hi |= ( Z64 ) ( self . low >> ( 64 - val ) );
        self . low <<= val;
    }
}

void Z128Base :: shl ( Z64 val ) noexcept
{
    if ( val < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        self . shr ( - val );
#else
        self . hi = 0LL;
        self . low = 0ULL;
#endif
        return;
    }

    if ( val >= 128LL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else if ( ( N32 ) val >= 64 )
    {
        self . hi = ( Z64 ) ( self . low << ( 64 - ( N32 ) val ) );
        self . low = 0ULL;
    }
    else
    {
        self . hi <<= ( N32 ) val;
        self . hi |= ( Z64 ) ( self . low >> ( 64 - ( N32 ) val ) );
        self . low <<= ( N32 ) val;
    }
}

void Z128Base :: shl ( const Z128Base & val ) noexcept
{
    if ( val . hi < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        Z128Base copy ( val );
        copy . negate ();
        self . shr ( copy );
#else
        self . hi = 0LL;
        self . low = 0ULL;
#endif
        return;
    }

    if ( val . hi != 0LL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else
    {
        self . shl ( ( Z64 ) val . low );
    }
}

void Z128Base :: shl ( const N128Base & val ) noexcept
{
    if ( val . hi != 0ULL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else
    {
        self . shl ( ( Z64 ) val . low );
    }
}


// shr
void Z128Base :: shr ( N32 val ) noexcept
{
    if ( val >= 128 )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else if ( val >= 64 )
    {
        self . low = ( N64 ) ( self . hi >> ( val - 64 ) );
        self . hi >>= 63; // preserve sign
    }
    else
    {
        self . low >>= val;
        self . low |= ( N64 ) ( self . hi << ( 64 - val ) );
        self . hi >>= val;
    }
}

void Z128Base :: shr ( Z64 val ) noexcept
{
    if ( val < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        self . shl ( - val );
#else
        self . hi = 0LL;
        self . low = 0ULL;
#endif
        return;
    }

    if ( val >= 128LL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else if ( ( N32 ) val >= 64 )
    {
        self . low = ( N64 ) ( self . hi >> ( ( N32 ) val - 64 ) );
        self . hi >>= 63; // preserve sign
    }
    else
    {
        self . low >>= ( N32 ) val;
        self . low |= ( N64 ) ( self . hi << ( 64 - ( N32 ) val ) );
        self . hi >>= ( N32 ) val;
    }
}

void Z128Base :: shr ( const Z128Base & val ) noexcept
{
    if ( val . hi < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        Z128Base copy ( val );
        copy . negate ();
        self . shl ( copy );
#else
        self . hi = 0LL;
        self . low = 0ULL;
#endif
        return;
    }

    if ( val . hi != 0LL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else
    {
        self . shr ( ( Z64 ) val . low );
    }
}

void Z128Base :: shr ( const N128Base & val ) noexcept
{
    if ( val . hi != 0ULL )
    {
        self . hi = 0LL;
        self . low = 0ULL;
    }
    else
    {
        self . shr ( ( Z64 ) val . low );
    }
}


// cpy
void Z128Base :: cpy ( Z64 val ) noexcept
{
    self . low = val;
    self . hi = val < 0 ? -1LL : 0LL;
}

void Z128Base :: cpy ( const Z128Base & val ) noexcept
{
    self . low = val . low;
    self . hi = val . hi;
}

void Z128Base :: cpy ( const N128Base & val ) noexcept
{
    self . low = val . low;
    self . hi = ( Z64 ) val . hi;
}


Z128Base :: Z128Base ( Z64 val ) noexcept
    : CNST ( ( val < 0 ? -1LL : 0LL ), val )
{
}

Z128Base :: Z128Base ( const Z128Base & val ) noexcept
    : CNST ( val . hi, val . low )
{
}

Z128Base :: Z128Base ( const N128Base & val ) noexcept
    : CNST ( ( Z64 ) val . hi, val . low )
{
}

Z128Base :: Z128Base ( Z64 _hi, N64 _low ) noexcept
    : CNST ( _hi, _low )
{
}

Z128Base :: Z128Base () noexcept
    : CNST ( 0LL, 0ULL )
{
}

/*=========================================================*
 *                        N128Base                         *
 *=========================================================*/

// casts
Z64 N128Base :: toZ64 () const noexcept
{
    // this will lop off top 64 bits unconditionally
    return ( Z64 ) self . low;
}

N64 N128Base :: toN64 () const noexcept
{
    // this will lop off top 64 bits unconditionally
    return self . low;
}

// lt
bool N128Base :: lt ( N64 val ) const noexcept
{
    if ( self . hi != 0LL )
        return false;
    return self . low < val;
}

bool N128Base :: lt ( const Z128Base & val ) const noexcept
{
    if ( self . hi != ( N64 ) val . hi )
        return self . hi < ( N64 ) val . hi;
    return self . low < val . low;
}

bool N128Base :: lt ( const N128Base & val ) const noexcept
{
    if ( self . hi != val . hi )
        return self . hi < val . hi;
    return self . low < val . low;
}


// eq
bool N128Base :: eq ( N64 val ) const noexcept
{
    return self . hi == 0ULL && self . low == val;
}

bool N128Base :: eq ( const Z128Base & val ) const noexcept
{
    return self . hi == ( N64 ) val . hi && self . low == val . low;
}

bool N128Base :: eq ( const N128Base & val ) const noexcept
{
    return self . hi == val . hi && self . low == val . low;
}


// gt
bool N128Base :: gt ( N64 val ) const noexcept
{
    if ( self . hi != 0LL )
        return true;
    return self . low > val;
}

bool N128Base :: gt ( const Z128Base & val ) const noexcept
{
    if ( self . hi != ( N64 ) val . hi )
        return self . hi > ( N64 ) val . hi;
    return self . low > val . low;
}

bool N128Base :: gt ( const N128Base & val ) const noexcept
{
    if ( self . hi != val . hi )
        return self . hi > val . hi;
    return self . low > val . low;
}


// unary
void N128Base :: comp () noexcept
{
    self . hi = ~ self . hi;
    self . low = ~ self . low;
}


// add
void N128Base :: add ( N64 val ) noexcept
{
    N32 carry = N64_add ( & self . low, val );
    N64_adc ( & self . hi, 0ULL, carry );
}

void N128Base :: add ( const Z128Base & val ) noexcept
{
    N32 carry = N64_add ( & self . low, val . low );
    N64_adc ( & self . hi, ( N64 ) val . hi, carry );
}

void N128Base :: add ( const N128Base & val ) noexcept
{
    N32 carry = N64_add ( & self . low, val . low );
    N64_adc ( & self . hi, val . hi, carry );
}


// sub
void N128Base :: sub ( N64 val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val );
    N64_sbb ( & self . hi, 0ULL, borrow );
}

void N128Base :: sub ( const Z128Base & val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val . low );
    N64_sbb ( & self . hi, ( N64 ) val . hi, borrow );
}

void N128Base :: sub ( const N128Base & val ) noexcept
{
    N32 borrow = N64_sub ( & self . low, val . low );
    N64_sbb ( & self . hi, val . hi, borrow );
}


// mul
void N128Base :: mul ( N64 val ) noexcept
{
    self . mul ( N128Base ( val ) );
}

void N128Base :: mul ( const Z128Base & val ) noexcept
{
    self . mul ( N128Base ( val ) );
}

void N128Base :: mul ( const N128Base & val ) noexcept
{
    N128Base mself ( self );
    N128Base sum ( 0ULL, 0ULL );
    
    for ( N32 i = 0; i < 64; ++ i )
    {
        if ( ( val . low & ( 1ULL << i ) ) != 0 )
            sum . add ( mself );
        mself . add ( mself );
    }

    if ( val . hi != 0ULL )
    {
        for ( N32 i = 0; i < 63; ++ i )
        {
            if ( ( val . hi & ( 1ULL << i ) ) != 0 )
                sum . add ( mself );
            mself . add ( mself );
        }
    }

    self . cpy ( sum );
}


// div
void N128Base :: div ( N64 val )
{
    self . div ( N128Base ( val ) );
}

void N128Base :: div ( const Z128Base & val )
{
    self . div ( N128Base ( val ) );
}

void N128Base :: div ( const N128Base & val )
{
    if ( val . hi == 0ULL && val . low == 0ULL )
        throw std :: runtime_error ( "divide by zero" );
        
    N128Base a ( self );
    N128Base b ( val );

    N32 i;
    for ( i = 0; ! b . gt ( a ); ++ i )
    {
        if ( b . eq ( a ) )
        {
            // we already have the answer
            // and it's an even power of 2
            if ( i < 64 )
            {
                self . hi = 0ULL;
                self . low = ( 1ULL << i );
            }
            else
            {
                self . low = 0ULL;
                self . hi = ( 1ULL << ( i - 64 ) );
            }
            return;
        }
        
        if ( ( Z64 ) b . hi < 0 )
            break;
        
        b . add ( b );
    }

    N128Base rslt ( 0ULL, 0ULL );
    for ( ; ; -- i )
    {
        if ( ! b . gt ( a ) )
        {
            a . sub ( b );
            rslt . add ( 1ULL );
        }

        if ( i == 0 )
            break;

        rslt . add ( rslt );
        b . shr ( 1U );
    }

    self . cpy ( rslt );
}


// mod
void N128Base :: mod ( N64 val )
{
    self . mod ( N128Base ( val ) );
}

void N128Base :: mod ( const Z128Base & val )
{
    self . mod ( N128Base ( val ) );
}

void N128Base :: mod ( const N128Base & val )
{
    if ( val . hi == 0ULL && val . low == 0ULL )
        throw std :: runtime_error ( "divide by zero" );
        
    N128Base a ( self );
    N128Base b ( val );

    N32 i;
    for ( i = 0; ! b . gt ( a ); ++ i )
    {
        if ( b . eq ( a ) )
        {
            // we already have the answer
            // there is no remainder
            self . hi = self . low = 0ULL;
            return;
        }
        
        if ( ( Z64 ) b . hi < 0 )
            break;
        
        b . add ( b );
    }

    for ( ; ; -- i )
    {
        if ( ! b . gt ( a ) )
            a . sub ( b );

        if ( i == 0 )
            break;

        b . shr ( 1U );
    }

    self . cpy ( a );
}


// band
void N128Base :: band ( N64 val ) noexcept
{
    self . low &= val;
    self . hi = 0ULL;
}

void N128Base :: band ( const Z128Base & val ) noexcept
{
    self . low &= val . low;
    self . hi &= ( N64 ) val . hi;
}

void N128Base :: band ( const N128Base & val ) noexcept
{
    self . low &= val . low;
    self . hi &= val . hi;
}


// bor
void N128Base :: bor ( N64 val ) noexcept
{
    self . low |= val;
}

void N128Base :: bor ( const Z128Base & val ) noexcept
{
    self . low |= val . low;
    self . hi |= ( N64 ) val . hi;
}

void N128Base :: bor ( const N128Base & val ) noexcept
{
    self . low |= val . low;
    self . hi |= val . hi;
}


// bxor
void N128Base :: bxor ( N64 val ) noexcept
{
    self . low ^= val;
}

void N128Base :: bxor ( const Z128Base & val ) noexcept
{
    self . low ^= val . low;
    self . hi ^= ( N64 ) val . hi;
}

void N128Base :: bxor ( const N128Base & val ) noexcept
{
    self . low ^= val . low;
    self . hi ^= val . hi;
}


// shl
void N128Base :: shl ( N32 val ) noexcept
{
    if ( val >= 128 )
        self . hi = self . low = 0ULL;
    else if ( val >= 64 )
    {
        self . hi = self . low << ( 64 - val );
        self . low = 0ULL;
    }
    else
    {
        self . hi <<= val;
        self . hi |= self . low >> ( 64 - val );
        self . low <<= val;
    }
}

void N128Base :: shl ( N64 val ) noexcept
{
    if ( val >= 128ULL )
        self . hi = self . low = 0ULL;
    else if ( ( N32 ) val >= 64 )
    {
        self . hi = self . low << ( 64 - ( N32 ) val );
        self . low = 0ULL;
    }
    else
    {
        self . hi <<= ( N32 ) val;
        self . hi |= self . low >> ( 64 - ( N32 ) val );
        self . low <<= ( N32 ) val;
    }
}

void N128Base :: shl ( const Z128Base & val ) noexcept
{
    if ( val . hi < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        Z128Base copy ( val );
        copy . negate ();
        self . shr ( copy );
#else
        self . hi = self . low = 0ULL;
#endif
        return;
    }

    if ( val . hi != 0LL )
        self . hi = self . low = 0ULL;
    else
    {
        self . shl ( val . low );
    }
}

void N128Base :: shl ( const N128Base & val ) noexcept
{
    if ( val . hi != 0ULL )
        self . hi = self . low = 0ULL;
    else
    {
        self . shl ( val . low );
    }
}


// shr
void N128Base :: shr ( N32 val ) noexcept
{
    if ( val >= 128 )
        self . hi = self . low = 0ULL;
    else if ( val >= 64 )
    {
        self . low = self . hi >> ( val - 64 );
        self . hi = 0ULL;
    }
    else
    {
        self . low >>= val;
        self . low |= self . hi << ( 64 - val );
        self . hi >>= val;
    }
}

void N128Base :: shr ( N64 val ) noexcept
{
    if ( val >= 128LL )
        self . hi = self . low = 0ULL;
    else if ( ( N32 ) val >= 64 )
    {
        self . low = self . hi >> ( ( N32 ) val - 64 );
        self . hi = 0ULL;
    }
    else
    {
        self . low >>= ( N32 ) val;
        self . low |= self . hi << ( 64 - ( N32 ) val );
        self . hi >>= ( N32 ) val;
    }
}

void N128Base :: shr ( const Z128Base & val ) noexcept
{
    if ( val . hi < 0 )
    {
#if NEG_SHIFT_REVERSES_DIRECTION
        Z128Base copy ( val );
        copy . negate ();
        self . shl ( copy );
#else
        self . hi = self . low = 0ULL;
#endif
        return;
    }

    if ( val . hi != 0LL )
        self . hi = self . low = 0ULL;
    else
    {
        self . shr ( val . low );
    }
}

void N128Base :: shr ( const N128Base & val ) noexcept
{
    if ( val . hi != 0ULL )
        self . hi = self . low = 0ULL;
    else
    {
        self . shr ( val . low );
    }
}


// cpy
void N128Base :: cpy ( N64 val ) noexcept
{
    self . low = val;
    self . hi = 0LL;
}

void N128Base :: cpy ( const Z128Base & val ) noexcept
{
    self . low = val . low;
    self . hi = val . hi;
}

void N128Base :: cpy ( const N128Base & val ) noexcept
{
    self . low = val . low;
    self . hi = val . hi;
}


N128Base :: N128Base ( N64 val ) noexcept
    : CNST ( 0ULL, val )
{
}

N128Base :: N128Base ( const Z128Base & val ) noexcept
    : CNST ( val . hi, val . low )
{
}

N128Base :: N128Base ( const N128Base & val ) noexcept
    : CNST ( val . hi, val . low )
{
}

N128Base :: N128Base ( N64 _hi, N64 _low ) noexcept
    : CNST ( _hi, _low )
{
}

N128Base :: N128Base () noexcept
    : CNST ( 0ULL, 0ULL )
{
}

const N128 N128_MAX ( N64_MAX, N64_MAX );
const Z128 Z128_MIN ( Z64_MIN, 0ULL );
const Z128 Z128_MAX ( Z64_MAX, N64_MAX );

Z128 __bswap_128 ( const Z128 & n ) noexcept
{
    return Z128 ( ( Z64 ) __bswap_64 ( n . low64 () ), __bswap_64 ( ( N64 ) n . high64 () ) );
}

N128 __bswap_128 ( const N128 & n ) noexcept
{
    return N128 ( __bswap_64 ( n . low64 () ), __bswap_64 ( n . high64 () ) );
}

#endif // HAVE_Z128
