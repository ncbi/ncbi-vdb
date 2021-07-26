/*

  vdb3.kfc.zigzag

 */

#pragma once

#include <vdb3/kfc/defs.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     ZigZagBase                      *
     *=====================================================*/

    template < class T >
    class ZigZagBase
    {
    public:

        // significant bits, including sign
        N32 magnitude () const noexcept;

        // zigzag-encoded integer
        N64 toRawN64 () const;
        N128 toRawN128 () const noexcept;

        // 2's complement integer
        Z64 toZ64 () const;
        Z128 toZ128 () const noexcept;

        // assign 2's complement to zigzag
        void cpy ( Z64 z ) noexcept;
        void cpy ( const Z128 & z );

        // assign raw zigzag to zigzag
        void raw ( N64 n ) noexcept;
        void raw ( const N128 & n );
        void raw ( const ZigZagBase < N64 > & zz ) noexcept;
        void raw ( const ZigZagBase < N128 > & zz );

        // constructors
        ZigZagBase ( Z64 z ) noexcept;
        ZigZagBase ( const Z128 & z );
        ZigZagBase ( const ZigZagBase < N64 > & z ) noexcept;
        ZigZagBase ( const ZigZagBase < N128 > & z );
        
        ZigZagBase () noexcept;
        ~ ZigZagBase () noexcept;

    private:

        T val;
    };
    
    /*=====================================================*
     *                       ZigZag                        *
     *=====================================================*/

    template < class T = N64 >
    class ZigZag : protected ZigZagBase < T >
    {
    public:

        typedef ZigZagBase < T > ZZB;
        typedef ZigZag < T > ZZ;
        
        N32 magnitude () const noexcept
        { return ZZB :: magnitude (); }

        N64 toRawN64 () const
        { return ZZB :: toRawN64 (); }
        N128 toRawN128 () const noexcept
        { return ZZB :: toRawN128 (); }
        
        Z64 toZ64 () const
        { return ZZB :: toZ64 (); }
        Z128 toZ128 () const noexcept
        { return ZZB :: toZ128 (); }

        void raw ( N64 n ) noexcept
        { ZZB :: raw ( n ); }
        void raw ( const N128 & n )
        { ZZB :: raw ( n ); }
        void raw ( const ZigZagBase < N64 > & zz ) noexcept
        { ZZB :: raw ( zz ); }
        void raw ( const ZigZagBase < N128 > & zz )
        { ZZB :: raw ( zz ); }

        ZZ & operator = ( signed short int z ) noexcept
        { ZZB :: cpy ( ( Z64 ) z ); return self; }
        ZZ & operator = ( signed int z ) noexcept
        { ZZB :: cpy ( ( Z64 ) z ); return self; }
        ZZ & operator = ( signed long int z ) noexcept
        { ZZB :: cpy ( ( Z64 ) z ); return self; }
        ZZ & operator = ( signed long long int z ) noexcept
        { ZZB :: cpy ( ( Z64 ) z ); return self; }
        ZZ & operator = ( const Z128 & z )
        { ZZB :: cpy ( z ); return self; }
        ZZ & operator = ( const ZigZag < N64 > & z ) noexcept
        { ZZB :: cpy ( z ); return self; }
        ZZ & operator = ( const ZigZag < N128 > & z )
        { ZZB :: cpy ( z ); return self; }
        
        ZigZag ( signed short int z ) noexcept
            : ZZB ( ( Z64 ) z ) {}
        ZigZag ( signed int z ) noexcept
            : ZZB ( ( Z64 ) z ) {}
        ZigZag ( signed long int z ) noexcept
            : ZZB ( ( Z64 ) z ) {}
        ZigZag ( signed long long int z ) noexcept
            : ZZB ( ( Z64 ) z ) {}
        ZigZag ( const Z128 & z )
            : ZZB ( z ) {}
        ZigZag ( const ZigZag < N64 > & z ) noexcept
            : ZZB ( z ) {}
        ZigZag ( const ZigZag < N128 > & z )
            : ZZB ( z ) {}
        
        ZigZag () noexcept {}
    };
}
