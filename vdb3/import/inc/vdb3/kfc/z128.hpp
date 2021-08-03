/*

  vdb3.kfc.z128


 */

#pragma once

#include <vdb3/kfc/defs.hpp>

#if ! HAVE_Z128

/*=========================================================*
 *                        FORWARDS                         *
 *=========================================================*/

class N128;
struct N128Base;

/*=========================================================*
 *                        Z128Base                         *
 *=========================================================*/

struct Z128Base
{
    // casts
    Z64 toZ64 () const noexcept;
    N64 toN64 () const noexcept;

    // lt
    bool lt ( Z64 val ) const noexcept;
    bool lt ( const Z128Base & val ) const noexcept;
    bool lt ( const N128Base & val ) const noexcept;

    // eq
    bool eq ( Z64 val ) const noexcept;
    bool eq ( const Z128Base & val ) const noexcept;
    bool eq ( const N128Base & val ) const noexcept;

    // gt
    bool gt ( Z64 val ) const noexcept;
    bool gt ( const Z128Base & val ) const noexcept;
    bool gt ( const N128Base & val ) const noexcept;

    // unary
    void negate () noexcept;

    // add
    void add ( Z64 val ) noexcept;
    void add ( const Z128Base & val ) noexcept;
    void add ( const N128Base & val ) noexcept;

    // sub
    void sub ( Z64 val ) noexcept;
    void sub ( const Z128Base & val ) noexcept;
    void sub ( const N128Base & val ) noexcept;

    // mul
    void mul ( Z64 val ) noexcept;
    void mul ( const Z128Base & val ) noexcept;
    void mul ( const N128Base & val ) noexcept;

    // div
    void div ( Z64 val );
    void div ( const Z128Base & val );
    void div ( const N128Base & val );

    // mod
    void mod ( Z64 val );
    void mod ( const Z128Base & val );
    void mod ( const N128Base & val );

    // band
    void band ( Z64 val ) noexcept;
    void band ( const Z128Base & val ) noexcept;
    void band ( const N128Base & val ) noexcept;

    // bor
    void bor ( Z64 val ) noexcept;
    void bor ( const Z128Base & val ) noexcept;
    void bor ( const N128Base & val ) noexcept;

    // bxor
    void bxor ( Z64 val ) noexcept;
    void bxor ( const Z128Base & val ) noexcept;
    void bxor ( const N128Base & val ) noexcept;

    // shl
    void shl ( N32 val ) noexcept;
    void shl ( Z64 val ) noexcept;
    void shl ( const Z128Base & val ) noexcept;
    void shl ( const N128Base & val ) noexcept;

    // shr
    void shr ( N32 val ) noexcept;
    void shr ( Z64 val ) noexcept;
    void shr ( const Z128Base & val ) noexcept;
    void shr ( const N128Base & val ) noexcept;

    // cpy
    void cpy ( Z64 val ) noexcept;
    void cpy ( const Z128Base & val ) noexcept;
    void cpy ( const N128Base & val ) noexcept;

    explicit Z128Base ( Z64 val ) noexcept;
    explicit Z128Base ( const Z128Base & val ) noexcept;
    explicit Z128Base ( const N128Base & val ) noexcept;
    Z128Base ( Z64 hi, N64 low ) noexcept;

    Z128Base () noexcept;

#if BYTE_ORDER == LITTLE_ENDIAN
    N64 low;
    Z64 hi;
#else
    Z64 hi;
    N64 low;
#endif
};

/*=========================================================*
 *                        N128Base                         *
 *=========================================================*/

struct N128Base
{
    // casts
    Z64 toZ64 () const noexcept;
    N64 toN64 () const noexcept;

    // lt
    bool lt ( N64 val ) const noexcept;
    bool lt ( const Z128Base & val ) const noexcept;
    bool lt ( const N128Base & val ) const noexcept;

    // eq
    bool eq ( N64 val ) const noexcept;
    bool eq ( const Z128Base & val ) const noexcept;
    bool eq ( const N128Base & val ) const noexcept;

    // gt
    bool gt ( N64 val ) const noexcept;
    bool gt ( const Z128Base & val ) const noexcept;
    bool gt ( const N128Base & val ) const noexcept;

    // unary
    void comp () noexcept;

    // add
    void add ( N64 val ) noexcept;
    void add ( const Z128Base & val ) noexcept;
    void add ( const N128Base & val ) noexcept;

    // sub
    void sub ( N64 val ) noexcept;
    void sub ( const Z128Base & val ) noexcept;
    void sub ( const N128Base & val ) noexcept;

    // mul
    void mul ( N64 val ) noexcept;
    void mul ( const Z128Base & val ) noexcept;
    void mul ( const N128Base & val ) noexcept;

    // div
    void div ( N64 val );
    void div ( const Z128Base & val );
    void div ( const N128Base & val );

    // mod
    void mod ( N64 val );
    void mod ( const Z128Base & val );
    void mod ( const N128Base & val );

    // band
    void band ( N64 val ) noexcept;
    void band ( const Z128Base & val ) noexcept;
    void band ( const N128Base & val ) noexcept;

    // bor
    void bor ( N64 val ) noexcept;
    void bor ( const Z128Base & val ) noexcept;
    void bor ( const N128Base & val ) noexcept;

    // bxor
    void bxor ( N64 val ) noexcept;
    void bxor ( const Z128Base & val ) noexcept;
    void bxor ( const N128Base & val ) noexcept;

    // shl
    void shl ( N32 val ) noexcept;
    void shl ( N64 val ) noexcept;
    void shl ( const Z128Base & val ) noexcept;
    void shl ( const N128Base & val ) noexcept;

    // shr
    void shr ( N32 val ) noexcept;
    void shr ( N64 val ) noexcept;
    void shr ( const Z128Base & val ) noexcept;
    void shr ( const N128Base & val ) noexcept;

    // cpy
    void cpy ( N64 val ) noexcept;
    void cpy ( const Z128Base & val ) noexcept;
    void cpy ( const N128Base & val ) noexcept;

    explicit N128Base ( N64 val ) noexcept;
    explicit N128Base ( const Z128Base & val ) noexcept;
    explicit N128Base ( const N128Base & val ) noexcept;
    N128Base ( N64 hi, N64 low ) noexcept;

    N128Base () noexcept;

#if BYTE_ORDER == LITTLE_ENDIAN
    N64 low;
    N64 hi;
#else
    N64 hi;
    N64 low;
#endif
};

/*=========================================================*
 *                          Z128                           *
 *=========================================================*/

class Z128 : protected Z128Base
{
public:

    typedef Z128Base B;

    // casts
    operator Z64 () const noexcept
    { return B :: toZ64 (); }
    operator N64 () const noexcept
    { return B :: toN64 (); }

    // useful ONLY when ! HAVE_Z128
    inline Z64 high64 () const noexcept
    { return hi; }
    inline N64 low64 () const noexcept
    { return low; }

    // unary
    Z128 operator - () const noexcept
    { return Z128 ( self ) . negate (); }

    Z128 & negate () noexcept
    { B :: negate (); return self; }

    // operator '+='
    Z128 & operator += ( signed short int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( unsigned short int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( signed int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( unsigned int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( signed long int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( unsigned long int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( signed long long int val ) noexcept
    { B :: add ( ( Z64 ) val ); return self; }
    Z128 & operator += ( unsigned long long int val ) noexcept
    { B :: add ( N128Base ( val ) ); return self; }
    Z128 & operator += ( const Z128 & val ) noexcept
    { B :: add ( val ); return self; }
    inline Z128 & operator += ( const N128 & val ) noexcept;

    // operator '-='
    Z128 & operator -= ( signed short int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( unsigned short int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( signed int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( unsigned int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( signed long int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( unsigned long int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( signed long long int val ) noexcept
    { B :: sub ( ( Z64 ) val ); return self; }
    Z128 & operator -= ( unsigned long long int val ) noexcept
    { B :: sub ( N128Base ( val ) ); return self; }
    Z128 & operator -= ( const Z128 & val ) noexcept
    { B :: sub ( val ); return self; }
    inline Z128 & operator -= ( const N128 & val ) noexcept;

    // operator '*='
    Z128 & operator *= ( signed short int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( unsigned short int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( signed int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( unsigned int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( signed long int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( unsigned long int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( signed long long int val ) noexcept
    { B :: mul ( ( Z64 ) val ); return self; }
    Z128 & operator *= ( unsigned long long int val ) noexcept
    { B :: mul ( N128Base ( val ) ); return self; }
    Z128 & operator *= ( const Z128 & val ) noexcept
    { B :: mul ( val ); return self; }
    inline Z128 & operator *= ( const N128 & val ) noexcept;

    // operator '/='
    Z128 & operator /= ( signed short int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( unsigned short int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( signed int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( unsigned int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( signed long int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( unsigned long int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( signed long long int val )
    { B :: div ( ( Z64 ) val ); return self; }
    Z128 & operator /= ( unsigned long long int val )
    { B :: div ( N128Base ( val ) ); return self; }
    Z128 & operator /= ( const Z128 & val )
    { B :: div ( val ); return self; }
    inline Z128 & operator /= ( const N128 & val );

    // operator '%='
    Z128 & operator %= ( signed short int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( unsigned short int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( signed int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( unsigned int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( signed long int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( unsigned long int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( signed long long int val )
    { B :: mod ( ( Z64 ) val ); return self; }
    Z128 & operator %= ( unsigned long long int val )
    { B :: mod ( N128Base ( val ) ); return self; }
    Z128 & operator %= ( const Z128 & val )
    { B :: mod ( val ); return self; }
    inline Z128 & operator %= ( const N128 & val );

    // operator '&='
    Z128 & operator &= ( signed short int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( unsigned short int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( signed int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( unsigned int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( signed long int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( unsigned long int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( signed long long int val ) noexcept
    { B :: band ( ( Z64 ) val ); return self; }
    Z128 & operator &= ( unsigned long long int val ) noexcept
    { B :: band ( N128Base ( val ) ); return self; }
    Z128 & operator &= ( const Z128 & val ) noexcept
    { B :: band ( val ); return self; }
    inline Z128 & operator &= ( const N128 & val ) noexcept;

    // operator '|='
    Z128 & operator |= ( signed short int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( unsigned short int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( signed int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( unsigned int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( signed long int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( unsigned long int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( signed long long int val ) noexcept
    { B :: bor ( ( Z64 ) val ); return self; }
    Z128 & operator |= ( unsigned long long int val ) noexcept
    { B :: bor ( N128Base ( val ) ); return self; }
    Z128 & operator |= ( const Z128 & val ) noexcept
    { B :: bor ( val ); return self; }
    inline Z128 & operator |= ( const N128 & val ) noexcept;

    // operator '^='
    Z128 & operator ^= ( signed short int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( unsigned short int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( signed int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( unsigned int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( signed long int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( unsigned long int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( signed long long int val ) noexcept
    { B :: bxor ( ( Z64 ) val ); return self; }
    Z128 & operator ^= ( unsigned long long int val ) noexcept
    { B :: bxor ( N128Base ( val ) ); return self; }
    Z128 & operator ^= ( const Z128 & val ) noexcept
    { B :: bxor ( val ); return self; }
    inline Z128 & operator ^= ( const N128 & val ) noexcept;

    // operator '<<='
    Z128 & operator <<= ( signed short int val ) noexcept
    { B :: shl ( ( Z64 ) val ); return self; }
    Z128 & operator <<= ( unsigned short int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    Z128 & operator <<= ( signed int val ) noexcept
    { B :: shl ( ( Z64 ) val ); return self; }
    Z128 & operator <<= ( unsigned int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    Z128 & operator <<= ( signed long int val ) noexcept
    { B :: shl ( ( Z64 ) val ); return self; }
    Z128 & operator <<= ( unsigned long int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    Z128 & operator <<= ( signed long long int val ) noexcept
    { B :: shl ( ( Z64 ) val ); return self; }
    Z128 & operator <<= ( unsigned long long int val ) noexcept
    { B :: shl ( N128Base ( val ) ); return self; }
    Z128 & operator <<= ( const Z128 & val ) noexcept
    { B :: shl ( val ); return self; }
    inline Z128 & operator <<= ( const N128 & val ) noexcept;

    // operator '>>='
    Z128 & operator >>= ( signed short int val ) noexcept
    { B :: shr ( ( Z64 ) val ); return self; }
    Z128 & operator >>= ( unsigned short int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    Z128 & operator >>= ( signed int val ) noexcept
    { B :: shr ( ( Z64 ) val ); return self; }
    Z128 & operator >>= ( unsigned int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    Z128 & operator >>= ( signed long int val ) noexcept
    { B :: shr ( ( Z64 ) val ); return self; }
    Z128 & operator >>= ( unsigned long int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    Z128 & operator >>= ( signed long long int val ) noexcept
    { B :: shr ( ( Z64 ) val ); return self; }
    Z128 & operator >>= ( unsigned long long int val ) noexcept
    { B :: shr ( N128Base ( val ) ); return self; }
    Z128 & operator >>= ( const Z128 & val ) noexcept
    { B :: shr ( val ); return self; }
    inline Z128 & operator >>= ( const N128 & val ) noexcept;

    // operator '+'
    Z128 operator + ( signed short int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( unsigned short int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( signed int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( unsigned int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( signed long int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( unsigned long int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( signed long long int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) += val; }
    Z128 operator + ( const Z128 & val ) const noexcept
    { return Z128 ( self ) += val; }

    // operator '-'
    Z128 operator - ( signed short int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( unsigned short int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( signed int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( unsigned int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( signed long int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( unsigned long int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( signed long long int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) -= val; }
    Z128 operator - ( const Z128 & val ) const noexcept
    { return Z128 ( self ) -= val; }

    // operator '*'
    Z128 operator * ( signed short int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( unsigned short int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( signed int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( unsigned int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( signed long int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( unsigned long int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( signed long long int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) *= val; }
    Z128 operator * ( const Z128 & val ) const noexcept
    { return Z128 ( self ) *= val; }

    // operator '/'
    Z128 operator / ( signed short int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( unsigned short int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( signed int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( unsigned int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( signed long int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( unsigned long int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( signed long long int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( unsigned long long int val ) const
    { return Z128 ( self ) /= val; }
    Z128 operator / ( const Z128 & val ) const
    { return Z128 ( self ) /= val; }

    // operator '%'
    Z128 operator % ( signed short int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( unsigned short int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( signed int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( unsigned int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( signed long int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( unsigned long int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( signed long long int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( unsigned long long int val ) const
    { return Z128 ( self ) %= val; }
    Z128 operator % ( const Z128 & val ) const
    { return Z128 ( self ) %= val; }

    // operator '&'
    Z128 operator & ( signed short int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( unsigned short int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( signed int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( unsigned int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( signed long int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( unsigned long int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( signed long long int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) &= val; }
    Z128 operator & ( const Z128 & val ) const noexcept
    { return Z128 ( self ) &= val; }

    // operator '|'
    Z128 operator | ( signed short int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( unsigned short int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( signed int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( unsigned int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( signed long int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( unsigned long int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( signed long long int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) |= val; }
    Z128 operator | ( const Z128 & val ) const noexcept
    { return Z128 ( self ) |= val; }

    // operator '^'
    Z128 operator ^ ( signed short int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( unsigned short int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( signed int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( unsigned int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( signed long int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( unsigned long int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( signed long long int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) ^= val; }
    Z128 operator ^ ( const Z128 & val ) const noexcept
    { return Z128 ( self ) ^= val; }

    // operator '<<'
    Z128 operator << ( signed short int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( unsigned short int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( signed int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( unsigned int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( signed long int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( unsigned long int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( signed long long int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) <<= val; }
    Z128 operator << ( const Z128 & val ) const noexcept
    { return Z128 ( self ) <<= val; }
    inline Z128 operator << ( const N128 & val ) const noexcept;

    // operator '>>'
    Z128 operator >> ( signed short int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( unsigned short int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( signed int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( unsigned int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( signed long int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( unsigned long int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( signed long long int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( unsigned long long int val ) const noexcept
    { return Z128 ( self ) >>= val; }
    Z128 operator >> ( const Z128 & val ) const noexcept
    { return Z128 ( self ) >>= val; }
    inline Z128 operator >> ( const N128 & val ) const noexcept;

    // operator '<'
    bool operator < ( signed short int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( unsigned short int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( signed int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( unsigned int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( signed long int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( unsigned long int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( signed long long int val ) const noexcept
    { return B :: lt ( ( Z64 ) val ); }
    bool operator < ( unsigned long long int val ) const noexcept
    { return B :: lt ( N128Base ( val ) ); }
    bool operator < ( const Z128 & val ) const noexcept
    { return B :: lt ( val ); }
    inline bool operator < ( const N128 & val ) const noexcept;

    // operator '<='
    bool operator <= ( signed short int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( unsigned short int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( signed int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( unsigned int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( signed long int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( unsigned long int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( signed long long int val ) const noexcept
    { return ! B :: gt ( ( Z64 ) val ); }
    bool operator <= ( unsigned long long int val ) const noexcept
    { return ! B :: gt ( N128Base ( val ) ); }
    bool operator <= ( const Z128 & val ) const noexcept
    { return ! B :: gt ( val ); }
    inline bool operator <= ( const N128 & val ) const noexcept;

    // operator '=='
    bool operator == ( signed short int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( unsigned short int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( signed int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( unsigned int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( signed long int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( unsigned long int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( signed long long int val ) const noexcept
    { return B :: eq ( ( Z64 ) val ); }
    bool operator == ( unsigned long long int val ) const noexcept
    { return B :: eq ( N128Base ( val ) ); }
    bool operator == ( const Z128 & val ) const noexcept
    { return B :: eq ( val ); }
    inline bool operator == ( const N128 & val ) const noexcept;

    // operator '!='
    bool operator != ( signed short int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( unsigned short int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( signed int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( unsigned int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( signed long int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( unsigned long int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( signed long long int val ) const noexcept
    { return ! B :: eq ( ( Z64 ) val ); }
    bool operator != ( unsigned long long int val ) const noexcept
    { return ! B :: eq ( N128Base ( val ) ); }
    bool operator != ( const Z128 & val ) const noexcept
    { return ! B :: eq ( val ); }
    inline bool operator != ( const N128 & val ) const noexcept;

    // operator '>='
    bool operator >= ( signed short int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( unsigned short int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( signed int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( unsigned int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( signed long int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( unsigned long int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( signed long long int val ) const noexcept
    { return ! B :: lt ( ( Z64 ) val ); }
    bool operator >= ( unsigned long long int val ) const noexcept
    { return ! B :: lt ( N128Base ( val ) ); }
    bool operator >= ( const Z128 & val ) const noexcept
    { return ! B :: lt ( val ); }
    inline bool operator >= ( const N128 & val ) const noexcept;

    // operator '>'
    bool operator > ( signed short int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( unsigned short int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( signed int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( unsigned int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( signed long int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( unsigned long int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( signed long long int val ) const noexcept
    { return B :: gt ( ( Z64 ) val ); }
    bool operator > ( unsigned long long int val ) const noexcept
    { return B :: gt ( N128Base ( val ) ); }
    bool operator > ( const Z128 & val ) const noexcept
    { return B :: gt ( val ); }
    inline bool operator > ( const N128 & val ) const noexcept;

    // operator '='
    Z128 & operator = ( signed short int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( unsigned short int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( signed int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( unsigned int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( signed long int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( unsigned long int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( signed long long int val ) noexcept
    { B :: cpy ( ( Z64 ) val ); return self; }
    Z128 & operator = ( unsigned long long int val ) noexcept
    { B :: cpy ( N128Base ( val ) ); return self; }
    Z128 & operator = ( const Z128 & val ) noexcept
    { B :: cpy ( val ); return self; }
    inline Z128 & operator = ( const N128 & val ) noexcept;

    Z128 () noexcept {}
    Z128 ( const Z128 & val ) noexcept
        : B ( val ) {}
    Z128 ( Z64 _hi, N64 _lo ) noexcept
        : B ( _hi, _lo ) {}

    explicit Z128 ( signed short int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( unsigned short int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( signed int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( unsigned int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( signed long int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( unsigned long int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( signed long long int val ) noexcept
        : B ( ( Z64 ) val ) {}
    explicit Z128 ( unsigned long long int val ) noexcept
        : B ( N128Base ( val ) ) {}
    inline explicit Z128 ( const N128 & val ) noexcept;

    friend class N128;
};


/*=========================================================*
 *                          N128                           *
 *=========================================================*/

class N128 : protected N128Base
{
public:

    typedef N128Base B;

    // casts
    operator Z64 () const noexcept
    { return B :: toZ64 (); }
    operator N64 () const noexcept
    { return B :: toN64 (); }

    // useful ONLY when ! HAVE_Z128
    inline N64 high64 () const noexcept
    { return hi; }
    inline N64 low64 () const noexcept
    { return low; }

    // unary
    N128 operator ~ () const noexcept
    { return N128 ( self ). comp (); }

    N128 & comp () noexcept
    { B :: comp (); return self; }

    // operator '+='
    N128 & operator += ( signed short int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( unsigned short int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( signed int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( unsigned int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( signed long int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( unsigned long int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( signed long long int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( unsigned long long int val ) noexcept
    { B :: add ( ( N64 ) val ); return self; }
    N128 & operator += ( const Z128 & val ) noexcept
    { B :: add ( val ); return self; }
    N128 & operator += ( const N128 & val ) noexcept
    { B :: add ( val ); return self; }

    // operator '-='
    N128 & operator -= ( signed short int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( unsigned short int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( signed int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( unsigned int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( signed long int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( unsigned long int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( signed long long int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( unsigned long long int val ) noexcept
    { B :: sub ( ( N64 ) val ); return self; }
    N128 & operator -= ( const Z128 & val ) noexcept
    { B :: sub ( val ); return self; }
    N128 & operator -= ( const N128 & val ) noexcept
    { B :: sub ( val ); return self; }

    // operator '*='
    N128 & operator *= ( signed short int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( unsigned short int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( signed int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( unsigned int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( signed long int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( unsigned long int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( signed long long int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( unsigned long long int val ) noexcept
    { B :: mul ( ( N64 ) val ); return self; }
    N128 & operator *= ( const Z128 & val ) noexcept
    { B :: mul ( val ); return self; }
    N128 & operator *= ( const N128 & val ) noexcept
    { B :: mul ( val ); return self; }

    // operator '/='
    N128 & operator /= ( signed short int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( unsigned short int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( signed int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( unsigned int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( signed long int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( unsigned long int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( signed long long int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( unsigned long long int val )
    { B :: div ( ( N64 ) val ); return self; }
    N128 & operator /= ( const Z128 & val )
    { B :: div ( val ); return self; }
    N128 & operator /= ( const N128 & val )
    { B :: div ( val ); return self; }

    // operator '%='
    N128 & operator %= ( signed short int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( unsigned short int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( signed int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( unsigned int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( signed long int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( unsigned long int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( signed long long int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( unsigned long long int val )
    { B :: mod ( ( N64 ) val ); return self; }
    N128 & operator %= ( const Z128 & val )
    { B :: mod ( val ); return self; }
    N128 & operator %= ( const N128 & val )
    { B :: mod ( val ); return self; }

    // operator '&='
    N128 & operator &= ( signed short int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( unsigned short int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( signed int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( unsigned int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( signed long int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( unsigned long int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( signed long long int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( unsigned long long int val ) noexcept
    { B :: band ( ( N64 ) val ); return self; }
    N128 & operator &= ( const Z128 & val ) noexcept
    { B :: band ( val ); return self; }
    N128 & operator &= ( const N128 & val ) noexcept
    { B :: band ( val ); return self; }

    // operator '|='
    N128 & operator |= ( signed short int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( unsigned short int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( signed int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( unsigned int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( signed long int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( unsigned long int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( signed long long int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( unsigned long long int val ) noexcept
    { B :: bor ( ( N64 ) val ); return self; }
    N128 & operator |= ( const Z128 & val ) noexcept
    { B :: bor ( val ); return self; }
    N128 & operator |= ( const N128 & val ) noexcept
    { B :: bor ( val ); return self; }

    // operator '^='
    N128 & operator ^= ( signed short int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( unsigned short int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( signed int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( unsigned int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( signed long int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( unsigned long int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( signed long long int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( unsigned long long int val ) noexcept
    { B :: bxor ( ( N64 ) val ); return self; }
    N128 & operator ^= ( const Z128 & val ) noexcept
    { B :: bxor ( val ); return self; }
    N128 & operator ^= ( const N128 & val ) noexcept
    { B :: bxor ( val ); return self; }

    // operator '<<='
    N128 & operator <<= ( signed short int val ) noexcept
    { B :: shl ( Z128Base ( val ) ); return self; }
    N128 & operator <<= ( unsigned short int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    N128 & operator <<= ( signed int val ) noexcept
    { B :: shl ( Z128Base ( val ) ); return self; }
    N128 & operator <<= ( unsigned int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    N128 & operator <<= ( signed long int val ) noexcept
    { B :: shl ( Z128Base ( val ) ); return self; }
    N128 & operator <<= ( unsigned long int val ) noexcept
    { B :: shl ( ( N32 ) val ); return self; }
    N128 & operator <<= ( signed long long int val ) noexcept
    { B :: shl ( Z128Base ( val ) ); return self; }
    N128 & operator <<= ( unsigned long long int val ) noexcept
    { B :: shl ( ( N64 ) val ); return self; }
    N128 & operator <<= ( const Z128 & val ) noexcept
    { B :: shl ( val ); return self; }
    N128 & operator <<= ( const N128 & val ) noexcept
    { B :: shl ( val ); return self; }

    // operator '>>='
    N128 & operator >>= ( signed short int val ) noexcept
    { B :: shr ( Z128Base ( val ) ); return self; }
    N128 & operator >>= ( unsigned short int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    N128 & operator >>= ( signed int val ) noexcept
    { B :: shr ( Z128Base ( val ) ); return self; }
    N128 & operator >>= ( unsigned int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    N128 & operator >>= ( signed long int val ) noexcept
    { B :: shr ( Z128Base ( val ) ); return self; }
    N128 & operator >>= ( unsigned long int val ) noexcept
    { B :: shr ( ( N32 ) val ); return self; }
    N128 & operator >>= ( signed long long int val ) noexcept
    { B :: shr ( Z128Base ( val ) ); return self; }
    N128 & operator >>= ( unsigned long long int val ) noexcept
    { B :: shr ( ( N64 ) val ); return self; }
    N128 & operator >>= ( const Z128 & val ) noexcept
    { B :: shr ( val ); return self; }
    N128 & operator >>= ( const N128 & val ) noexcept
    { B :: shr ( val ); return self; }

    // operator '+'
    N128 operator + ( signed short int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( unsigned short int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( signed int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( unsigned int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( signed long int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( unsigned long int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( signed long long int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( unsigned long long int val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( const Z128 & val ) const noexcept
    { return N128 ( self ) += val; }
    N128 operator + ( const N128 & val ) const noexcept
    { return N128 ( self ) += val; }

    // operator '-'
    N128 operator - ( signed short int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( unsigned short int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( signed int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( unsigned int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( signed long int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( unsigned long int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( signed long long int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( unsigned long long int val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( const Z128 & val ) const noexcept
    { return N128 ( self ) -= val; }
    N128 operator - ( const N128 & val ) const noexcept
    { return N128 ( self ) -= val; }

    // operator '*'
    N128 operator * ( signed short int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( unsigned short int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( signed int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( unsigned int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( signed long int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( unsigned long int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( signed long long int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( unsigned long long int val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( const Z128 & val ) const noexcept
    { return N128 ( self ) *= val; }
    N128 operator * ( const N128 & val ) const noexcept
    { return N128 ( self ) *= val; }

    // operator '/'
    N128 operator / ( signed short int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( unsigned short int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( signed int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( unsigned int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( signed long int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( unsigned long int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( signed long long int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( unsigned long long int val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( const Z128 & val ) const
    { return N128 ( self ) /= val; }
    N128 operator / ( const N128 & val ) const
    { return N128 ( self ) /= val; }

    // operator '%'
    N128 operator % ( signed short int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( unsigned short int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( signed int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( unsigned int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( signed long int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( unsigned long int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( signed long long int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( unsigned long long int val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( const Z128 & val ) const
    { return N128 ( self ) %= val; }
    N128 operator % ( const N128 & val ) const
    { return N128 ( self ) %= val; }

    // operator '&'
    N128 operator & ( signed short int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( unsigned short int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( signed int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( unsigned int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( signed long int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( unsigned long int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( signed long long int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( unsigned long long int val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( const Z128 & val ) const noexcept
    { return N128 ( self ) &= val; }
    N128 operator & ( const N128 & val ) const noexcept
    { return N128 ( self ) &= val; }

    // operator '|'
    N128 operator | ( signed short int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( unsigned short int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( signed int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( unsigned int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( signed long int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( unsigned long int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( signed long long int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( unsigned long long int val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( const Z128 & val ) const noexcept
    { return N128 ( self ) |= val; }
    N128 operator | ( const N128 & val ) const noexcept
    { return N128 ( self ) |= val; }

    // operator '^'
    N128 operator ^ ( signed short int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( unsigned short int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( signed int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( unsigned int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( signed long int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( unsigned long int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( signed long long int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( unsigned long long int val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( const Z128 & val ) const noexcept
    { return N128 ( self ) ^= val; }
    N128 operator ^ ( const N128 & val ) const noexcept
    { return N128 ( self ) ^= val; }

    // operator '<<'
    N128 operator << ( signed short int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( unsigned short int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( signed int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( unsigned int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( signed long int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( unsigned long int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( signed long long int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( unsigned long long int val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( const Z128 & val ) const noexcept
    { return N128 ( self ) <<= val; }
    N128 operator << ( const N128 & val ) const noexcept
    { return N128 ( self ) <<= val; }

    // operator '>>'
    N128 operator >> ( signed short int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( unsigned short int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( signed int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( unsigned int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( signed long int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( unsigned long int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( signed long long int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( unsigned long long int val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( const Z128 & val ) const noexcept
    { return N128 ( self ) >>= val; }
    N128 operator >> ( const N128 & val ) const noexcept
    { return N128 ( self ) >>= val; }

    // operator '<'
    bool operator < ( signed short int val ) const noexcept
    { return B :: lt ( Z128Base ( val ) ); }
    bool operator < ( unsigned short int val ) const noexcept
    { return B :: lt ( ( N64 ) val ); }
    bool operator < ( signed int val ) const noexcept
    { return B :: lt ( Z128Base ( val ) ); }
    bool operator < ( unsigned int val ) const noexcept
    { return B :: lt ( ( N64 ) val ); }
    bool operator < ( signed long int val ) const noexcept
    { return B :: lt ( Z128Base ( val ) ); }
    bool operator < ( unsigned long int val ) const noexcept
    { return B :: lt ( ( N64 ) val ); }
    bool operator < ( signed long long int val ) const noexcept
    { return B :: lt ( Z128Base ( val ) ); }
    bool operator < ( unsigned long long int val ) const noexcept
    { return B :: lt ( ( N64 ) val ); }
    bool operator < ( const Z128 & val ) const noexcept
    { return B :: lt ( val ); }
    bool operator < ( const N128 & val ) const noexcept
    { return B :: lt ( val ); }

    // operator '<='
    bool operator <= ( signed short int val ) const noexcept
    { return ! B :: gt ( Z128Base ( val ) ); }
    bool operator <= ( unsigned short int val ) const noexcept
    { return ! B :: gt ( ( N64 ) val ); }
    bool operator <= ( signed int val ) const noexcept
    { return ! B :: gt ( Z128Base ( val ) ); }
    bool operator <= ( unsigned int val ) const noexcept
    { return ! B :: gt ( ( N64 ) val ); }
    bool operator <= ( signed long int val ) const noexcept
    { return ! B :: gt ( Z128Base ( val ) ); }
    bool operator <= ( unsigned long int val ) const noexcept
    { return ! B :: gt ( ( N64 ) val ); }
    bool operator <= ( signed long long int val ) const noexcept
    { return ! B :: gt ( Z128Base ( val ) ); }
    bool operator <= ( unsigned long long int val ) const noexcept
    { return ! B :: gt ( ( N64 ) val ); }
    bool operator <= ( const Z128 & val ) const noexcept
    { return ! B :: gt ( val ); }
    bool operator <= ( const N128 & val ) const noexcept
    { return ! B :: gt ( val ); }

    // operator '=='
    bool operator == ( signed short int val ) const noexcept
    { return B :: eq ( Z128Base ( val ) ); }
    bool operator == ( unsigned short int val ) const noexcept
    { return B :: eq ( ( N64 ) val ); }
    bool operator == ( signed int val ) const noexcept
    { return B :: eq ( Z128Base ( val ) ); }
    bool operator == ( unsigned int val ) const noexcept
    { return B :: eq ( ( N64 ) val ); }
    bool operator == ( signed long int val ) const noexcept
    { return B :: eq ( Z128Base ( val ) ); }
    bool operator == ( unsigned long int val ) const noexcept
    { return B :: eq ( ( N64 ) val ); }
    bool operator == ( signed long long int val ) const noexcept
    { return B :: eq ( Z128Base ( val ) ); }
    bool operator == ( unsigned long long int val ) const noexcept
    { return B :: eq ( ( N64 ) val ); }
    bool operator == ( const Z128 & val ) const noexcept
    { return B :: eq ( val ); }
    bool operator == ( const N128 & val ) const noexcept
    { return B :: eq ( val ); }

    // operator '!='
    bool operator != ( signed short int val ) const noexcept
    { return ! B :: eq ( Z128Base ( val ) ); }
    bool operator != ( unsigned short int val ) const noexcept
    { return ! B :: eq ( ( N64 ) val ); }
    bool operator != ( signed int val ) const noexcept
    { return ! B :: eq ( Z128Base ( val ) ); }
    bool operator != ( unsigned int val ) const noexcept
    { return ! B :: eq ( ( N64 ) val ); }
    bool operator != ( signed long int val ) const noexcept
    { return ! B :: eq ( Z128Base ( val ) ); }
    bool operator != ( unsigned long int val ) const noexcept
    { return ! B :: eq ( ( N64 ) val ); }
    bool operator != ( signed long long int val ) const noexcept
    { return ! B :: eq ( Z128Base ( val ) ); }
    bool operator != ( unsigned long long int val ) const noexcept
    { return ! B :: eq ( ( N64 ) val ); }
    bool operator != ( const Z128 & val ) const noexcept
    { return ! B :: eq ( val ); }
    bool operator != ( const N128 & val ) const noexcept
    { return ! B :: eq ( val ); }

    // operator '>='
    bool operator >= ( signed short int val ) const noexcept
    { return ! B :: lt ( Z128Base ( val ) ); }
    bool operator >= ( unsigned short int val ) const noexcept
    { return ! B :: lt ( ( N64 ) val ); }
    bool operator >= ( signed int val ) const noexcept
    { return ! B :: lt ( Z128Base ( val ) ); }
    bool operator >= ( unsigned int val ) const noexcept
    { return ! B :: lt ( ( N64 ) val ); }
    bool operator >= ( signed long int val ) const noexcept
    { return ! B :: lt ( Z128Base ( val ) ); }
    bool operator >= ( unsigned long int val ) const noexcept
    { return ! B :: lt ( ( N64 ) val ); }
    bool operator >= ( signed long long int val ) const noexcept
    { return ! B :: lt ( Z128Base ( val ) ); }
    bool operator >= ( unsigned long long int val ) const noexcept
    { return ! B :: lt ( ( N64 ) val ); }
    bool operator >= ( const Z128 & val ) const noexcept
    { return ! B :: lt ( val ); }
    bool operator >= ( const N128 & val ) const noexcept
    { return ! B :: lt ( val ); }

    // operator '>'
    bool operator > ( signed short int val ) const noexcept
    { return B :: gt ( Z128Base ( val ) ); }
    bool operator > ( unsigned short int val ) const noexcept
    { return B :: gt ( ( N64 ) val ); }
    bool operator > ( signed int val ) const noexcept
    { return B :: gt ( Z128Base ( val ) ); }
    bool operator > ( unsigned int val ) const noexcept
    { return B :: gt ( ( N64 ) val ); }
    bool operator > ( signed long int val ) const noexcept
    { return B :: gt ( Z128Base ( val ) ); }
    bool operator > ( unsigned long int val ) const noexcept
    { return B :: gt ( ( N64 ) val ); }
    bool operator > ( signed long long int val ) const noexcept
    { return B :: gt ( Z128Base ( val ) ); }
    bool operator > ( unsigned long long int val ) const noexcept
    { return B :: gt ( ( N64 ) val ); }
    bool operator > ( const Z128 & val ) const noexcept
    { return B :: gt ( val ); }
    bool operator > ( const N128 & val ) const noexcept
    { return B :: gt ( val ); }

    // operator '='
    N128 & operator = ( signed short int val ) noexcept
    { B :: cpy ( Z128Base ( val ) ); return self; }
    N128 & operator = ( unsigned short int val ) noexcept
    { B :: cpy ( ( N64 ) val ); return self; }
    N128 & operator = ( signed int val ) noexcept
    { B :: cpy ( Z128Base ( val ) ); return self; }
    N128 & operator = ( unsigned int val ) noexcept
    { B :: cpy ( ( N64 ) val ); return self; }
    N128 & operator = ( signed long int val ) noexcept
    { B :: cpy ( Z128Base ( val ) ); return self; }
    N128 & operator = ( unsigned long int val ) noexcept
    { B :: cpy ( ( N64 ) val ); return self; }
    N128 & operator = ( signed long long int val ) noexcept
    { B :: cpy ( Z128Base ( val ) ); return self; }
    N128 & operator = ( unsigned long long int val ) noexcept
    { B :: cpy ( ( N64 ) val ); return self; }
    N128 & operator = ( const Z128 & val ) noexcept
    { B :: cpy ( val ); return self; }
    N128 & operator = ( const N128 & val ) noexcept
    { B :: cpy ( val ); return self; }

    N128 () noexcept {}
    N128 ( const N128 & val ) noexcept
        : B ( val ) {}
    N128 ( N64 _hi, N64 _lo ) noexcept
        : B ( _hi, _lo ) {}

    explicit N128 ( signed short int val ) noexcept
        : B ( Z128Base ( val ) ) {}
    explicit N128 ( unsigned short int val ) noexcept
        : B ( ( N64 ) val ) {}
    explicit N128 ( signed int val ) noexcept
        : B ( Z128Base ( val ) ) {}
    explicit N128 ( unsigned int val ) noexcept
        : B ( ( N64 ) val ) {}
    explicit N128 ( signed long int val ) noexcept
        : B ( Z128Base ( val ) ) {}
    explicit N128 ( unsigned long int val ) noexcept
        : B ( ( N64 ) val ) {}
    explicit N128 ( signed long long int val ) noexcept
        : B ( Z128Base ( val ) ) {}
    explicit N128 ( unsigned long long int val ) noexcept
        : B ( ( N64 ) val ) {}
    explicit N128 ( const Z128 & val ) noexcept
        : B ( val ) {}

    friend class Z128;
};


/*=========================================================*
 *                         INLINES                         *
 *=========================================================*/

inline Z128 & Z128 :: operator += ( const N128 & val ) noexcept
{ B :: add ( val ); return self; }
inline Z128 & Z128 :: operator -= ( const N128 & val ) noexcept
{ B :: sub ( val ); return self; }
inline Z128 & Z128 :: operator *= ( const N128 & val ) noexcept
{ B :: mul ( val ); return self; }
inline Z128 & Z128 :: operator /= ( const N128 & val )
{ B :: div ( val ); return self; }
inline Z128 & Z128 :: operator %= ( const N128 & val )
{ B :: mod ( val ); return self; }
inline Z128 & Z128 :: operator &= ( const N128 & val ) noexcept
{ B :: band ( val ); return self; }
inline Z128 & Z128 :: operator |= ( const N128 & val ) noexcept
{ B :: bor ( val ); return self; }
inline Z128 & Z128 :: operator ^= ( const N128 & val ) noexcept
{ B :: bxor ( val ); return self; }
inline Z128 & Z128 :: operator <<= ( const N128 & val ) noexcept
{ B :: shl ( val ); return self; }
inline Z128 & Z128 :: operator >>= ( const N128 & val ) noexcept
{ B :: shr ( val ); return self; }
inline Z128 Z128 :: operator << ( const N128 & val ) const noexcept
{ return Z128 ( self ) <<= val; }
inline Z128 Z128 :: operator >> ( const N128 & val ) const noexcept
{ return Z128 ( self ) >>= val; }
inline bool Z128 :: operator < ( const N128 & val ) const noexcept
{ return B :: lt ( val ); }
inline bool Z128 :: operator <= ( const N128 & val ) const noexcept
{ return ! B :: gt ( val ); }
inline bool Z128 :: operator == ( const N128 & val ) const noexcept
{ return B :: eq ( val ); }
inline bool Z128 :: operator != ( const N128 & val ) const noexcept
{ return ! B :: eq ( val ); }
inline bool Z128 :: operator >= ( const N128 & val ) const noexcept
{ return ! B :: lt ( val ); }
inline bool Z128 :: operator > ( const N128 & val ) const noexcept
{ return B :: gt ( val ); }
inline Z128 & Z128 :: operator = ( const N128 & val ) noexcept
{ B :: cpy ( val ); return self; }
inline Z128 :: Z128 ( const N128 & val ) noexcept
    : B ( val ) {}

// these operators perform C/C++ signed "promotion"
inline N128 operator + ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) += b; }
inline N128 operator - ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) -= b; }
inline N128 operator * ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) *= b; }
inline N128 operator / ( const Z128 & a, const N128 & b )
{ return N128 ( a ) /= b; }
inline N128 operator % ( const Z128 & a, const N128 & b )
{ return N128 ( a ) %= b; }
inline N128 operator & ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) &= b; }
inline N128 operator | ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) |= b; }
inline N128 operator ^ ( const Z128 & a, const N128 & b ) noexcept
{ return N128 ( a ) ^= b; }

extern const N128 N128_MAX;
extern const Z128 Z128_MIN;
extern const Z128 Z128_MAX;

Z128 __bswap_128 ( const Z128 & n ) noexcept;
N128 __bswap_128 ( const N128 & n ) noexcept;

#endif // HAVE_Z128
