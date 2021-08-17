/*

  vdb3.cmn.dserial

 */

#include <vdb3/cmn/dserial.hpp>
#include <vdb3/cmn/serial.hpp>
#include <vdb3/kfc/rsrc-mem.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                    DeserialBase                     *
     *=====================================================*/

    void DeserialBase :: getITypeVarN16 ( N16 & val )
    {
        N64 v64;
        getITypeVarN64 ( v64 );
        if ( v64 > N16_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "failed to deserialize"
                << xcause
                << "variable-size format exceeds word size"
                );
        }
        val = ( N16 ) v64;
    }
    
    void DeserialBase :: getITypeVarN32 ( N32 & val )
    {
        N64 v64;
        getITypeVarN64 ( v64 );
        if ( v64 > N32_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "failed to deserialize"
                << xcause
                << "variable-size format exceeds word size"
                );
        }
        val = ( N32 ) v64;
    }
    
    void DeserialBase :: getITypeVarN64 ( N64 & val )
    {
        // accumulator
        N64 acc = 0;

        // overflow detection mask
        const N64 over_mask = ( N64 ) 0x7F << 57;

        // get remainder of region
        CRgn r = rgn . subRgn ( offset );
        const byte_t * a = r . addr ();
        size_t sz = r . size ();

        // scan region
        for ( size_t i = 0; ; )
        {
            for ( ; i < sz; ++ i )
            {
                // detect overflow
                if ( ( acc & over_mask ) != 0 )
                {
                    throw OverflowException (
                        XP ( XLOC )
                        << xprob
                        << "failed to deserialize"
                        << xcause
                        << "variable-size format exceeds word size"
                        );
                }
                
                // accumulate 7-bit byte
                acc <<= 7;
                acc |= a [ i ] & 0x7F;

                // detect end byte
                if ( ( a [ i ] & 0x80 ) == 0 )
                {
                    // >>> SUCCESS <<< //
                    offset += i + 1;
                    val = acc;
                    return;
                }
            }

            // safety check
            if ( i >= 18 )
            {
                throw InvalidDataException (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "variable-size format does not terminate"
                    );
            }

            // need more data
            size_t received = moreData ( 18 - i );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "variable-size format does not terminate"
                    );
            }

            // refresh sub-region
            r = rgn . subRgn ( offset );
            a = r . addr ();
            sz = r . size ();
        }
    }
    
    void DeserialBase :: getITypeVarN128 ( N128 & val )
    {
        // accumulator
        N128 acc = 0;

        // overflow detection mask
#if HAVE_Z128
        const N128 over_mask = ( N128 ) 0x7FU << 121;
#else
        const N128 over_mask ( 0x7FULL << 57, 0 );
#endif
        // get remainder of region
        CRgn r = rgn . subRgn ( offset );
        const byte_t * a = r . addr ();
        size_t sz = r . size ();

        // scan region
        for ( size_t i = 0; ; )
        {
            for ( ; i < sz; ++ i )
            {
                // detect overflow
                if ( ( acc & over_mask ) != 0 )
                {
                    throw OverflowException (
                        XP ( XLOC )
                        << xprob
                        << "failed to deserialize"
                        << xcause
                        << "variable-size format exceeds word size"
                        );
                }
                
                // accumulate 7-bit byte
                acc <<= 7;
                acc |= a [ i ] & 0x7F;

                // detect end byte
                if ( ( a [ i ] & 0x80 ) == 0 )
                {
                    // >>> SUCCESS <<< //
                    offset += i + 1;
                    val = acc;
                    return;
                }
            }

            // safety check
            if ( i >= 18 )
            {
                throw InvalidDataException (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "variable-size format does not terminate"
                    );
            }

            // need more data
            size_t received = moreData ( 18 - i );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "variable-size format does not terminate"
                    );
            }

            // refresh sub-region
            r = rgn . subRgn ( offset );
            a = r . addr ();
            sz = r . size ();
        }
    }
    
    void DeserialBase :: getITypeFixN16 ( N16 & val )
    {
        const size_t word_size = sizeof ( N16 );
        CRgn r = rgn . subRgn ( offset );
        while ( r . size () < word_size )
        {
            size_t received = moreData ( word_size - r . size () );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "incomplete word"
                    );
            }
            r = rgn . subRgn ( offset );
        }
        const byte_t * a = r . addr ();
        N16 acc = a [ 0 ];
        acc = ( acc << 8 ) | a [ 1 ];
        offset += word_size;
        val = acc;
    }
    
    void DeserialBase :: getITypeFixN32 ( N32 & val )
    {
        const size_t word_size = sizeof ( N32 );
        CRgn r = rgn . subRgn ( offset );
        while ( r . size () < word_size )
        {
            size_t received = moreData ( word_size - r . size () );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "incomplete word"
                    );
            }
            r = rgn . subRgn ( offset );
        }
        const byte_t * a = r . addr ();
        N32 acc = a [ 0 ];
        acc = ( acc << 8 ) | a [ 1 ];
        acc = ( acc << 8 ) | a [ 2 ];
        acc = ( acc << 8 ) | a [ 3 ];
        offset += word_size;
        val = acc;
    }
    
    void DeserialBase :: getITypeFixN48 ( N64 & val )
    {
        const size_t word_size = 3 * sizeof ( N16 );
        CRgn r = rgn . subRgn ( offset );
        while ( r . size () < word_size )
        {
            size_t received = moreData ( word_size - r . size () );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "incomplete word"
                    );
            }
            r = rgn . subRgn ( offset );
        }
        const byte_t * a = r . addr ();
        N64 acc = a [ 0 ];
        acc = ( acc << 8 ) | a [ 1 ];
        acc = ( acc << 8 ) | a [ 2 ];
        acc = ( acc << 8 ) | a [ 3 ];
        acc = ( acc << 8 ) | a [ 4 ];
        acc = ( acc << 8 ) | a [ 5 ];
        offset += word_size;
        val = acc;
    }
    
    void DeserialBase :: getITypeFixN64 ( N64 & val )
    {
        const size_t word_size = sizeof ( N64 );
        CRgn r = rgn . subRgn ( offset );
        while ( r . size () < word_size )
        {
            size_t received = moreData ( word_size - r . size () );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "incomplete word"
                    );
            }
            r = rgn . subRgn ( offset );
        }
        const byte_t * a = r . addr ();
        N64 acc = a [ 0 ];
        acc = ( acc << 8 ) | a [ 1 ];
        acc = ( acc << 8 ) | a [ 2 ];
        acc = ( acc << 8 ) | a [ 3 ];
        acc = ( acc << 8 ) | a [ 4 ];
        acc = ( acc << 8 ) | a [ 5 ];
        acc = ( acc << 8 ) | a [ 6 ];
        acc = ( acc << 8 ) | a [ 7 ];
        offset += word_size;
        val = acc;
    }
    
    void DeserialBase :: getITypeVarZ16 ( Z16 & val )
    {
        Z64 v64;
        getITypeVarZ64 ( v64 );
        if ( Z16_MIN > v64 || v64 > Z16_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "failed to deserialize"
                << xcause
                << "variable-size format exceeds word size"
                );
        }
        val = ( Z16 ) v64;
    }
    
    void DeserialBase :: getITypeVarZ32 ( Z32 & val )
    {
        Z64 v64;
        getITypeVarZ64 ( v64 );
        if ( Z32_MIN > v64 || v64 > Z32_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "failed to deserialize"
                << xcause
                << "variable-size format exceeds word size"
                );
        }
        val = ( Z32 ) v64;
    }
    
    void DeserialBase :: getITypeVarZ64 ( Z64 & val )
    {
        N64 raw;
        getITypeVarN64 ( raw );
        ZigZag < N64 > zz;
        zz . raw ( raw );
        val = zz . toZ64 ();
    }
    
    void DeserialBase :: getITypeVarZ128 ( Z128 & val )
    {
        N128 raw;
        getITypeVarN128 ( raw );
        ZigZag < N128 > zz;
        zz . raw ( raw );
        val = zz . toZ128 ();
    }
    
    void DeserialBase :: getITypeFixZ16 ( Z16 & val )
    {
        N16 raw;
        getITypeFixN16 ( raw );
        ZigZag < N64 > zz;
        zz . raw ( ( N64 ) raw );
        val = ( Z16 ) zz . toZ64 ();
    }
    
    void DeserialBase :: getITypeFixZ32 ( Z32 & val )
    {
        N32 raw;
        getITypeFixN32 ( raw );
        ZigZag < N64 > zz;
        zz . raw ( ( N64 ) raw );
        val = ( Z32 ) zz . toZ64 ();
    }
    
    void DeserialBase :: getITypeFixZ48 ( Z64 & val )
    {
        N64 raw;
        getITypeFixN48 ( raw );
        ZigZag < N64 > zz;
        zz . raw ( raw );
        val = zz . toZ64 ();
    }
    
    void DeserialBase :: getITypeFixZ64 ( Z64 & val )
    {
        N64 raw;
        getITypeFixN64 ( raw );
        ZigZag < N64 > zz;
        zz . raw ( raw );
        val = zz . toZ64 ();
    }
    
    void DeserialBase :: getITypeFixRb32 ( R32 & val )
    {
        N32 raw;
        getITypeFixN32 ( raw );
        :: memcpy ( & val, & raw, sizeof ( R32 ) );
    }
    
    void DeserialBase :: getITypeFixRb64 ( R64 & val )
    {
        N64 raw;
        getITypeFixN64 ( raw );
        :: memcpy ( & val, & raw, sizeof ( R64 ) );
    }
    
    void DeserialBase :: getITypeBlob ( CRgn & val )
    {
        N64 raw;
        getITypeVarN64 ( raw );
#if ARCHBITS == 32
        if ( raw > ( N64 ) SIZE_T_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "failed to deserialize"
                << xcause
                << "blob size exceeds architecture virtual memory space"
                );
        }
#endif
        size_t sz = ( size_t ) raw;
        CRgn r = rgn . subRgn ( offset, sz );
        while ( r . size () < sz )
        {
            size_t received = moreData ( sz - r . size () );
            if ( received == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to deserialize"
                    << xcause
                    << "incomplete blob"
                    );
            }
            r = rgn . subRgn ( offset, sz );
        }
        val = r;
        offset += sz;
    }
    
    void DeserialBase :: getUTypeSeqCount ( count_t & count )
    {
        N64 raw;
        getITypeVarN64 ( raw );
        count = ( count_t ) raw;
    }
    
    N32 DeserialBase :: getXTypeFmt ( N32 & fmt )
    {
        N32 prefix;
        getITypeVarN32 ( prefix );
        fmt = prefix & 0x7U;
        return prefix >> 3;
    }
    
    void DeserialBase :: getXTypeN16 ( N32 fmt, N16 & val )
    {
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarN16 ( val );
            break;
        case sfFixed16:
            getITypeFixN16 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeN32 ( N32 fmt, N32 & val )
    {
        N16 n16;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarN32 ( val );
            break;
        case sfFixed16:
            getITypeFixN16 ( n16 );
            val = n16;
            break;
        case sfFixed32:
            getITypeFixN32 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeN64 ( N32 fmt, N64 & val )
    {
        N16 n16;
        N32 n32;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarN64 ( val );
            break;
        case sfFixed16:
            getITypeFixN16 ( n16 );
            val = n16;
            break;
        case sfFixed32:
            getITypeFixN32 ( n32 );
            val = n32;
            break;
        case sfFixed48:
            getITypeFixN48 ( val );
            break;
        case sfFixed64:
            getITypeFixN64 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeN128 ( N32 fmt, N128 & val )
    {
        N16 n16;
        N32 n32;
        N64 n64;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarN128 ( val );
            break;
        case sfFixed16:
            getITypeFixN16 ( n16 );
            val = ( N64 ) n16;
            break;
        case sfFixed32:
            getITypeFixN32 ( n32 );
            val = ( N64 ) n32;
            break;
        case sfFixed48:
            getITypeFixN48 ( n64 );
            val = n64;
            break;
        case sfFixed64:
            getITypeFixN64 ( n64 );
            val = n64;
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeZ16 ( N32 fmt, Z16 & val )
    {
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarZ16 ( val );
            break;
        case sfFixed16:
            getITypeFixZ16 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeZ32 ( N32 fmt, Z32 & val )
    {
        Z16 z16;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarZ32 ( val );
            break;
        case sfFixed16:
            getITypeFixZ16 ( z16 );
            val = z16;
            break;
        case sfFixed32:
            getITypeFixZ32 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeZ64 ( N32 fmt, Z64 & val )
    {
        Z16 z16;
        Z32 z32;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarZ64 ( val );
            break;
        case sfFixed16:
            getITypeFixZ16 ( z16 );
            val = z16;
            break;
        case sfFixed32:
            getITypeFixZ32 ( z32 );
            val = z32;
            break;
        case sfFixed48:
            getITypeFixZ48 ( val );
            break;
        case sfFixed64:
            getITypeFixZ64 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeZ128 ( N32 fmt, Z128 & val )
    {
        Z16 z16;
        Z32 z32;
        Z64 z64;
        switch ( fmt )
        {
        case sfVarSize:
            getITypeVarZ128 ( val );
            break;
        case sfFixed16:
            getITypeFixZ16 ( z16 );
            val = ( Z64 ) z16;
            break;
        case sfFixed32:
            getITypeFixZ32 ( z32 );
            val = ( Z64 ) z32;
            break;
        case sfFixed48:
            getITypeFixZ48 ( z64 );
            val = z64;
            break;
        case sfFixed64:
            getITypeFixZ64 ( z64 );
            val = z64;
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeRb32 ( N32 fmt, R32 & val )
    {
        switch ( fmt )
        {
        case sfFixed32:
            getITypeFixRb32 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeRb64 ( N32 fmt, R64 & val )
    {
        switch ( fmt )
        {
        case sfFixed64:
            getITypeFixRb64 ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeBlob ( N32 fmt, CRgn & val )
    {
        switch ( fmt )
        {
        case sfBlob:
            getITypeBlob ( val );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }
    
    void DeserialBase :: getXTypeSeqCount ( N32 fmt, count_t & count )
    {
        switch ( fmt )
        {
        case sfSeq:
            getUTypeSeqCount ( count );
            break;
        default:
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to deserialize"
                << xcause
                << "bad format parameter"
                );
        }
    }

    bool DeserialBase :: hasMoreData () const noexcept
    {
        return offset < rgn . size ();
    }
    

    DeserialBase & DeserialBase :: operator = ( DeserialBase && d ) noexcept
    {
        rgn = std :: move ( d . rgn );
        offset = std :: move ( d . offset );
        return self;
    }

    DeserialBase & DeserialBase :: operator = ( const DeserialBase & d ) noexcept
    {
        rgn = d . rgn;
        offset = d . offset;
        return self;
    }

    DeserialBase :: DeserialBase ( DeserialBase && d ) noexcept
        : rgn ( std :: move ( d . rgn ) )
        , offset (std :: move ( d . offset ) )
    {
    }

    DeserialBase :: DeserialBase ( const DeserialBase & d ) noexcept
        : rgn ( d . rgn )
        , offset ( d . offset )
    {
    }

    DeserialBase :: DeserialBase () noexcept
        : offset ( 0 )
    {
    }

    DeserialBase :: ~ DeserialBase () noexcept
    {
    }

    DeserialBase & DeserialBase :: operator = ( const CRgn & r ) noexcept
    {
        rgn = r;
        offset = 0;
        return self;
    }

    DeserialBase :: DeserialBase ( const CRgn & r ) noexcept
        : rgn ( r )
        , offset ( 0 )
    {
    }

    size_t DeserialBase :: moreData ( size_t requested )
    {
        return 0;
    }

    /*=====================================================*
     *                      Deserial                       *
     *=====================================================*/
    
    Deserial & Deserial :: get ( bool & val )
    {
        N16 n;
        DB :: getITypeVarN16 ( n );
        val = ( n != 0 );
        return self;
    }

    // "long" int types are treated as 64-bit
    // regardless of their actual size
    Deserial & Deserial :: get ( unsigned long int & val )
    {
        N64 v;
        DB :: getITypeVarN64 ( v );
        val = ( unsigned long int ) v;
        return self;
    }
    Deserial & Deserial :: getFixed ( unsigned long int & val )
    {
        N64 v;
        DB :: getITypeFixN64 ( v );
        val = ( unsigned long int ) v;
        return self;
    }
    
    Deserial & Deserial :: get ( signed long int & val )
    {
        Z64 v;
        DB :: getITypeVarZ64 ( v );
        val = ( signed long int ) v;
        return self;
    }
    Deserial & Deserial :: getFixed ( signed long int & val )
    {
        Z64 v;
        DB :: getITypeFixZ64 ( v );
        val = ( signed long int ) v;
        return self;
    }
    
    Deserial & Deserial :: get ( long double & val )
    {
        R64 r;
        DB :: getITypeFixRb64 ( r );
        val = r;
        return self;
    }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
    Deserial & Deserial :: get ( R128 & val )
    {
        R64 r;
        DB :: getITypeFixRb64 ( r );
        val = r;
        return self;
    }
#endif
    Deserial & Deserial :: get ( Deserial & val )
    {
        CRgn r;
        DB :: getITypeBlob ( r );
        val = r;
        return self;
    }
    
    void Deserial :: getTyped ( N32 fmt, bool & val )
    {
        N16 n;
        DB :: getXTypeN16 ( fmt, n );
        val = ( n != 0 );
    }

    void Deserial :: getTyped ( N32 fmt, unsigned long int & val )
    {
        N64 v;
        DB :: getXTypeN64 ( fmt, v );
        val = ( unsigned long int ) v;
    }
    void Deserial :: getTyped ( N32 fmt, signed long int & val )
    {
        Z64 v;
        DB :: getXTypeZ64 ( fmt, v );
        val = ( signed long int ) v;
    }
    
    void Deserial :: getTyped ( N32 fmt, long double & val )
    {
        R64 r;
        DB :: getXTypeRb64 ( fmt, r );
        val = r;
    }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
    void Deserial :: getTyped ( N32 fmt, R128 & val )
    {
        R64 r;
        DB :: getXTypeRb64 ( fmt, r );
        val = r;
    }
#endif
    void Deserial :: getTyped ( N32 fmt, Deserial & val )
    {
        CRgn r;
        DB :: getXTypeBlob ( fmt, r );
        val = r;
    }

    void Deserial :: getMbr ( N32 fmt, bool & val )
    {
        N16 n;
        DB :: getXTypeN16 ( fmt, n );
        val = ( n != 0 );
    }
    void Deserial :: getMbr ( N32 fmt, unsigned long int & val )
    {
        N64 v;
        DB :: getXTypeN64 ( fmt, v );
        val = ( unsigned long int ) v;
    }
    void Deserial :: getMbr ( N32 fmt, signed long int & val )
    {
        Z64 v;
        DB :: getXTypeZ64 ( fmt, v );
        val = ( signed long int ) v;
    }
    void Deserial :: getMbr ( N32 fmt, long double & val )
    {
        R64 r;
        DB :: getXTypeRb64 ( fmt, r );
        val = r;
    }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
    void Deserial :: getMbr ( N32 fmt, R128 & val )
    {
        R64 r;
        DB :: getXTypeRb64 ( fmt, r );
        val = r;
    }
#endif
    void Deserial :: getMbr ( N32 fmt, Deserial & val )
    {
        CRgn r;
        DB :: getXTypeBlob ( fmt, r );
        val = r;
    }

    /*=====================================================*
     *                   INTRINSIC TYPES                   *
     *=====================================================*/

    // time - Ticks
    void Deserial :: get ( Ticks & val )
    {
        Z64 tm;
        get ( tm );
        val = tm;
    }
    void Deserial :: getTyped ( N32 fmt, Ticks & val )
    {
        Z64 tm;
        getTyped ( fmt, tm );
        val = tm;
    }
    void Deserial :: getMbr ( N32 fmt, Ticks & val )
    {
        Z64 tm;
        getMbr ( fmt, tm );
        val = tm;
    }

    // decimal floating-point Real
    void Deserial :: get ( DecFltPoint < Z64 > & val )
    {
        Deserial d;
        get ( d );
            
        Z64 mant = 0; Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }
    void Deserial :: get ( DecFltPoint < Z128 > & val )
    {
        Deserial d;
        get ( d );
            
        Z128 mant ( 0 ); Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }
    void Deserial :: getTyped ( N32 fmt, DecFltPoint < Z64 > & val )
    {
        Deserial d;
        getTyped ( fmt, d );
            
        Z64 mant = 0; Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }
    void Deserial :: getTyped ( N32 fmt, DecFltPoint < Z128 > & val )
    {
        Deserial d;
        getTyped ( fmt, d );
            
        Z128 mant ( 0 ); Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }
    void Deserial :: getMbr ( N32 fmt, DecFltPoint < Z64 > & val )
    {
        Deserial d;
        getMbr ( fmt, d );
            
        Z64 mant = 0; Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }
    void Deserial :: getMbr ( N32 fmt, DecFltPoint < Z128 > & val )
    {
        Deserial d;
        getMbr ( fmt, d );
            
        Z128 mant ( 0 ); Z32 exp = 0;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, mant ); break;
            case 2: d . getMbr ( fmt, exp ); break;
            }
        }
        val = DecFltPoint < Z64 > ( mant, exp );
    }

    // Text
    void Deserial :: get ( CText & val )
    {
        Deserial d;
        get ( d );
            
        CRgn data; count_t count;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, data ); break;
            case 2: d . getMbr ( fmt, count ); break;
            }
        }
        val = makeCText ( data, count );
    }
    
    void Deserial :: getTyped ( N32 fmt, CText & val )
    {
        Deserial d;
        getTyped ( fmt, d );
            
        CRgn data; count_t count;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, data ); break;
            case 2: d . getMbr ( fmt, count ); break;
            }
        }
        val = makeCText ( data, count );
    }
    
    void Deserial :: getMbr ( N32 fmt, CText & val )
    {
        Deserial d;
        getMbr ( fmt, d );
            
        CRgn data; count_t count;
        for ( int i = 0; i < 2; ++ i )
        {
            N32 fmt, mbr = d . getMbrId ( fmt );
            switch ( mbr )
            {
            case 1: d . getMbr ( fmt, data ); break;
            case 2: d . getMbr ( fmt, count ); break;
            }
        }
        val = makeCText ( data, count );
    }
    
}
