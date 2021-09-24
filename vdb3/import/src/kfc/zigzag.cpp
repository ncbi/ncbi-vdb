/*

  vdb3.kfc.zigzag

 */

#include <vdb3/kfc/zigzag.hpp>
#include <vdb3/kfc/except.hpp>

namespace vdb3
{
    template <>
    N32 ZigZagBase < N64 > :: magnitude () const noexcept
    {
        if ( val == 0 )
            return 0;
        
        return 64 - __builtin_clzll ( val );
    }
    
    template <>
    N32 ZigZagBase < N128 > :: magnitude () const noexcept
    {
#if HAVE_Z128
        N64 hi = val >> 64;
        if ( hi != 0 )
            return 128 - __builtin_clzll ( hi );
#else
        N64 hi = val . high64 ();
        if ( hi != 0 )
            return 128 - __builtin_clzll ( hi );
#endif
        N64 lo = ( N64 ) val;
        if ( lo == 0 )
            return 0;
        
        return 64 - __builtin_clzll ( lo );
    }
    
    template <>
    N64 ZigZagBase < N64 > :: toRawN64 () const
    {
        return val;
    }
    
    template <>
    N64 ZigZagBase < N128 > :: toRawN64 () const
    {
        if (
#if HAVE_Z128
              ( val >> 64 )
#else
              val . high64 ()
#endif
                              != 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << xprob
                << "failed to cast ZigZag to raw binary"
                << xcause
                << "128-bit ZigZag overflows raw N64"
                );
        }
        
        return ( N64 ) val;
    }

    template < class T >
    N128 ZigZagBase < T > :: toRawN128 () const noexcept
    {
        return N128 ( val );
    }

    template <>
    Z64 ZigZagBase < N64 > :: toZ64 () const
    {
        return - ( Z64 ) ( val & 1 ) ^ ( val >> 1 );
    }

    template <>
    Z64 ZigZagBase < N128 > :: toZ64 () const
    {
        if (
#if HAVE_Z128
              ( val >> 64 )
#else
              val . high64 ()
#endif
                              != 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << xprob
                << "failed to cast ZigZag to 2's complement integer"
                << xcause
                << "128-bit ZigZag overflows Z64"
                );
        }
        
        N64 val64 = ( N64 ) val;
        return - ( Z64 ) ( val64 & 1 ) ^ ( val64 >> 1 );
    }

    template <>
    Z128 ZigZagBase < N64 > :: toZ128 () const noexcept
    {
        Z64 zval = - ( Z64 ) ( val & 1 ) ^ ( val >> 1 );
        return Z128 ( zval );
    }

    template <>
    Z128 ZigZagBase < N128 > :: toZ128 () const noexcept
    {
        Z128 sign = val & 1;
#if HAVE_Z128
        sign = - sign;
#else
        sign . negate ();
#endif
        return sign ^= ( val >> 1 );
    }

    template < class T >
    void ZigZagBase < T > :: cpy ( Z64 z ) noexcept
    {
        val = ( z >> 63 ) ^ ( z << 1 );
    }

    template <>
    void ZigZagBase < N64 > :: cpy ( const Z128 & z )
    {
        N128 val128 ( ( z >> 127 ) ^ ( z << 1 ) );
        if (
#if HAVE_Z128
              ( val128 >> 64 )
#else
              val128 . high64 ()
#endif
                                  != 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << xprob
                << "failed to assign 2's complement integer to ZigZag"
                << xcause
                << "Z128 overflows 64-bit ZigZag"
                );
        }

        val = ( N64 ) val128;
    }

    template <>
    void ZigZagBase < N128 > :: cpy ( const Z128 & z )
    {
        N128 val128 ( ( z >> 127 ) ^ ( z << 1 ) );
        val = val128;
    }

    template < class T >
    void ZigZagBase < T > :: raw ( N64 n ) noexcept
    {
        val = n;
    }

    template <>
    void ZigZagBase < N64 > :: raw ( const N128 & val128 )
    {
        if (
#if HAVE_Z128
              ( val128 >> 64 )
#else
              val128 . high64 ()
#endif
                                  != 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << xprob
                << "failed to assign larger natural integer to ZigZag"
                << xcause
                << "N128 overflows 64-bit ZigZag"
                );
        }

        val = ( N64 ) val128;
    }

    template <>
    void ZigZagBase < N128 > :: raw ( const N128 & val128 )
    {
        val = val128;
    }

    template < class T >
    void ZigZagBase < T > :: raw ( const ZigZagBase < N64 > & zz ) noexcept
    {
        val = zz . toRawN64 ();
    }

    template <>
    void ZigZagBase < N64 > :: raw ( const ZigZagBase < N128 > & zz )
    {
        N128 val128 ( zz . toRawN128 () );
        if (
#if HAVE_Z128
              ( val128 >> 64 )
#else
              val128 . high64 ()
#endif
                                  != 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << xprob
                << "failed to assign larger to smaller ZigZag"
                << xcause
                << "128-bit ZigZag overflows 64-bit ZigZag"
                );
        }

        val = ( N64 ) val128;
    }

    template <>
    void ZigZagBase < N128 > :: raw ( const ZigZagBase < N128 > & zz )
    {
        val = zz . toRawN128 ();
    }

    template < class T >
    ZigZagBase < T > :: ZigZagBase ( Z64 z ) noexcept
        : val ( ( z >> 63 ) ^ ( z << 1 ) )
    {
    }

    template <>
    ZigZagBase < N64 > :: ZigZagBase ( const Z128 & z )
    {
        cpy ( z );
    }

    template <>
    ZigZagBase < N128 > :: ZigZagBase ( const Z128 & z )
        : val ( ( z >> 127 ) ^ ( z << 1 ) )
    {
    }
    
    template < class T >
    ZigZagBase < T > :: ZigZagBase ( const ZigZagBase < N64 > & z ) noexcept
        : val ( z . toRawN64 () )
    {
    }
    
    template <>
    ZigZagBase < N64 > :: ZigZagBase ( const ZigZagBase < N128 > & z )
    {
        raw ( z );
    }
    
    template <>
    ZigZagBase < N128 > :: ZigZagBase ( const ZigZagBase < N128 > & z )
        : val ( z . val )
    {
    }
        
    template < class T >
    ZigZagBase < T > :: ZigZagBase () noexcept
        : val ( 0 )
    {
    }
    
    template < class T >
    ZigZagBase < T > :: ~ ZigZagBase () noexcept
    {
        val = 0;
    }

    template class ZigZagBase < N64 >;
    template class ZigZagBase < N128 >;
}
