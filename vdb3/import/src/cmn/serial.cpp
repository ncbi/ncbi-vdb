/*

  vdb3.cmn.serial

 */

#include <vdb3/cmn/serial.hpp>

namespace vdb3
{
    // sfVarSize formatting starts with the number of significant bits,
    // expands it by a factor of 8/7 in order to insert continuation bit.
    // the serialization is still in bytes. for small magnitudes, this
    // will result in a savings.
    //
    // so for every word that can be sent in "sizeof word - 1" bytes or
    // less, sfVarSize format saves space. naive use could result in it
    // using MORE bytes than the natural word, so it is only viable
    // when less than an upper limit.
    //
    // the general formula for the exclusive limit is:
    //    1 << ( sizeof ( T ) * 7 )
    
    const N16 LIM_VAR_N16 = ( N16 ) 1 << ( sizeof ( N16 ) * 7 );
    const N32 LIM_VAR_N32 = ( N32 ) 1 << ( sizeof ( N32 ) * 7 );
    const N64 LIM_VAR_N48 = ( N64 ) 1 << ( sizeof ( N16 ) * 3 * 7 );
    const N64 LIM_VAR_N64 = ( N64 ) 1 << ( sizeof ( N64 ) * 7 );

    const N64 N48_MAX = ( ( N64 ) 1 << 48 ) - 1;

    // to encode a sequence element type into its count, here is a limit
    const N64 LIM_SEQCNT_N64 = N64_MAX >> 3;

    /*=====================================================*
     *                     SerialBase                      *
     *=====================================================*/

    void SerialBase :: putITypeVarN32 ( N32 val )
    {
        // very unlikely to be worthwhile to optimize
        // for 32-bit operations, except for RPi
        putITypeVarN64 ( ( N64 ) val );
    }
    
    void SerialBase :: putITypeVarN64 ( N64 val )
    {
        // always going to put at least 1 byte,
        // even if the value is zero
        size_t bytes = 1;
        if ( val != 0 )
        {
            // get number of significant bits
            N32 bits = 64 - __builtin_clzll ( val );

            // convert to bytes with 7 significant bits each
            bytes = ( bits + 6 ) / 7;
        }

        // tell storage to make room; usually a noop
        b . ensureAdditionalCapacity ( bytes );

        // get the updatable region of buffer
        // from current "size", which is number of valid bytes
        // to the end of the buffer which we ensured has space
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // treat region as byte array
        byte_t * a = mrgn . addr ();

        // lay out in big-endian order
        // where LSByte has clear MSBit
        a [ bytes - 1 ] = ( byte_t ) val & 0x7F;
        val >>= 7;

        // write remaining bytes with upper bit set
        for ( size_t i = bytes - 1; i != 0; val >>= 7 )
        {
            assert ( val != 0 );
            a [ -- i ] = ( byte_t ) val | 0x80;
        }

        // should have written everything
        assert ( val == 0 );

        // update size in buffer
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeVarN128 ( const N128 & cval )
    {
        N128 val ( cval );
        
        // in the case where the upper 64 bits
        // are zero, just treat as 64-bit word
#if HAVE_Z128
        N64 hi = val >> 64;
#else
        N64 hi = val . high64 ();
#endif
        if ( hi == 0ULL )
        {
            putITypeVarN64 ( ( N64 ) val );
            return;
        }
        
        // get number of significant bits
        N32 bits = 128 - __builtin_clzll ( hi );

        // convert to bytes with 7 significant bits each
        size_t bytes = ( bits + 6 ) / 7;

        // the rest is similar to the 64-bit version above
        b . ensureAdditionalCapacity ( bytes );

        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        byte_t * a = mrgn . addr ();

        a [ bytes - 1 ] = ( byte_t ) ( N64 ) val & 0x7F;
        val >>= 7;

        for ( size_t i = bytes - 1; i != 0; val >>= 7 )
        {
            assert ( val != 0 );
            a [ -- i ] = ( byte_t ) ( N64 ) val | 0x80;
        }

        assert ( val == 0 );

        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeFixN16 ( N16 val )
    {
        const size_t bytes = sizeof ( N16 );

        b . ensureAdditionalCapacity ( bytes );
        
        // get bytes in which to write
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // lay out in big-endian order
        byte_t * a = mrgn . addr ();
        for ( size_t i = bytes; i != 0; val >>= 8 )
            a [ -- i ] = ( byte_t ) val;
        assert ( val == 0 );

        // update buffer size
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeFixN32 ( N32 val )
    {
        const size_t bytes = sizeof ( N32 );

        b . ensureAdditionalCapacity ( bytes );
        
        // get bytes in which to write
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // lay out in big-endian order
        byte_t * a = mrgn . addr ();
        for ( size_t i = bytes; i != 0; val >>= 8 )
            a [ -- i ] = ( byte_t ) val;
        assert ( val == 0 );

        // update buffer size
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeFixN48 ( N64 val )
    {
        if ( val > N48_MAX )
        {
            throw OverflowException (
                XP ( XLOC )
                << "unable to represent value "
                << val
                << " in 48-bits"
                );
        }
        
        const size_t bytes = 3 * sizeof ( N16 );

        b . ensureAdditionalCapacity ( bytes );
        
        // get bytes in which to write
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // lay out in big-endian order
        byte_t * a = mrgn . addr ();
        for ( size_t i = bytes; i != 0; val >>= 8 )
            a [ -- i ] = ( byte_t ) val;
        assert ( val == 0 );

        // update buffer size
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeFixN64 ( N64 val )
    {
        const size_t bytes = sizeof ( N64 );

        b . ensureAdditionalCapacity ( bytes );
        
        // get bytes in which to write
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // lay out in big-endian order
        byte_t * a = mrgn . addr ();
        for ( size_t i = bytes; i != 0; val >>= 8 )
            a [ -- i ] = ( byte_t ) val;
        assert ( val == 0 );

        // update buffer size
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: putITypeVarZ64 ( const ZigZag < N64 > & val )
    {
        putITypeVarN64 ( val . toRawN64 () );
    }
    
    void SerialBase :: putITypeVarZ128 ( const ZigZag < N128 > & val )
    {
        putITypeVarN128 ( val . toRawN128 () );
    }
    
    void SerialBase :: putITypeFixRb32 ( R32 val )
    {
        N32 n;
        :: memcpy ( & n, & val, sizeof n );
        putITypeFixN32 ( n );
    }
    
    void SerialBase :: putITypeFixRb64 ( R64 val )
    {
        N64 n;
        :: memcpy ( & n, & val, sizeof n );
        putITypeFixN64 ( n );
    }
    
    void SerialBase :: putITypeBlob ( const CRgn & val )
    {
        size_t bytes = val . size ();
        
        putITypeVarN64 ( bytes );

        b . ensureAdditionalCapacity ( bytes );
        
        // get bytes in which to write
        MRgn mrgn = b . buffer () . subRgn ( b . size (), bytes );
        assert ( mrgn . size () == bytes );

        // transfer bytes
        mrgn . copy ( val );
        
        // update buffer size
        b . setSize ( b . size () + bytes );
    }
    
    void SerialBase :: openUTypeSeq ( count_t count )
    {
        putITypeVarN64 ( count );
    }
    

    void SerialBase :: putXTypeN16 ( SerialType type, N16 val )
    {
        if ( val < LIM_VAR_N16 )
        {
            prefix ( type, sfVarSize );
            putITypeVarN32 ( ( N32 ) val );
        }
        else
        {
            prefix ( type, sfFixed16 );
            putITypeFixN16 ( val );
        }
    }
    
    void SerialBase :: putXTypeN32 ( SerialType type, N32 val )
    {
        if ( val <= N16_MAX )
            putXTypeN16 ( type, ( N16 ) val );
        else if ( val < LIM_VAR_N32 )
        {
            prefix ( type, sfVarSize );
            putITypeVarN32 ( val );
        }
        else
        {
            prefix ( type, sfFixed32 );
            putITypeFixN32 ( val );
        }
    }
    
    void SerialBase :: putXTypeN64 ( SerialType type, N64 val )
    {
        if ( val <= N32_MAX )
            putXTypeN32 ( type, ( N32 ) val );
        else if ( val < LIM_VAR_N48 )
        {
            prefix ( type, sfVarSize );
            putITypeVarN64 ( val );
        }
        else if ( val <= N48_MAX )
        {
            prefix ( type, sfFixed48 );
            putITypeFixN48 ( val );
        }
        else if ( val < LIM_VAR_N64 )
        {
            prefix ( type, sfVarSize );
            putITypeVarN64 ( val );
        }
        else
        {
            prefix ( type, sfFixed64 );
            putITypeFixN64 ( val );
        }
    }
    
    void SerialBase :: putXTypeN128 ( SerialType type, const N128 & val )
    {
        if ( val <= N64_MAX )
            putXTypeN64 ( type, ( N64 ) val );
        else
        {
            prefix ( type, sfVarSize );
            putITypeVarN128 ( val );
        }
    }
    
    void SerialBase :: putXTypeZ64 ( SerialType type, const ZigZag < N64 > & val )
    {
        putXTypeN64 ( type, val . toRawN64 () );
    }
    
    void SerialBase :: putXTypeZ128 ( SerialType type, const ZigZag < N128 > & val )
    {
        putXTypeN128 ( type, val . toRawN128 () );
    }
    
    void SerialBase :: putXTypeRb32 ( SerialType type, R32 val )
    {
        prefix ( type, sfFixed32 );
        putITypeFixRb32 ( val );
    }
    
    void SerialBase :: putXTypeRb64 ( SerialType type, R64 val )
    {
        prefix ( type, sfFixed64 );
        putITypeFixRb64 ( val );
    }
    
    void SerialBase :: putXTypeBlob ( SerialType type, const CRgn & val )
    {
        prefix ( type, sfBlob );
        putITypeBlob ( val );
    }
    
    void SerialBase :: openXTypeSeq ( SerialType type, count_t count )
    {
        prefix ( type, sfSeq );
        openUTypeSeq ( count );
    }
    

    CRgn SerialBase :: content () const noexcept
    {
        return b . content ();
    }

    void SerialBase :: reinitialize ()
    {
        b . reinitialize ();
    }

    SerialBase & SerialBase :: operator = ( SerialBase && s ) noexcept
    {
        b = std :: move ( s . b );
        return self;
    }
    
    SerialBase & SerialBase :: operator = ( const SerialBase & s ) noexcept
    {
        b = s . b;
        return self;
    }

    SerialBase :: SerialBase ( SerialBase && s ) noexcept
        : b ( std :: move ( s . b ) )
    {
    }
    
    SerialBase :: SerialBase ( const SerialBase & s ) noexcept
        : b ( s . b )
    {
    }
    
    SerialBase :: SerialBase () noexcept
    {
    }
    
    SerialBase :: ~ SerialBase () noexcept
    {
    }
    
    SerialBase :: SerialBase ( const RsrcKfc & rsrc )
        : b ( rsrc )
    {
    }
    
    void SerialBase :: prefix ( N32 id, SerialFmt fmt )
    {
        putITypeVarN64 ( ( ( N64 ) id << 3 ) | fmt );
    }

    /*=====================================================*
     *                       Serial                        *
     *=====================================================*/

    Serial & Serial :: putFixed ( signed short int val )
    {
        ZigZag < N64 > zz ( val );
        putITypeFixN16 ( ( N16 ) zz . toRawN64 () );
        return self;
    }
    
    Serial & Serial :: putFixed ( signed int val )
    {
        ZigZag < N64 > zz ( val );
        putITypeFixN32 ( ( N32 ) zz . toRawN64 () );
        return self;
    }
    
    Serial & Serial :: putFixed ( signed long int val )
    {
        ZigZag < N64 > zz ( val );
        putITypeFixN64 ( ( N64 ) zz . toRawN64 () );
        return self;
    }
    
    Serial & Serial :: putFixed ( signed long long int val )
    {
        ZigZag < N64 > zz ( val );
        putITypeFixN64 ( zz . toRawN64 () );
        return self;
    }

    // Time
    Serial & Serial :: put ( const TimeBase & val )
    {
        Ticks tm ( val );
        return put ( tm . toZ64 () );
    }
    Serial & Serial :: putTyped ( SerialType type, const TimeBase & val )
    {
        Ticks tm ( val );
        return putTyped ( type, tm . toZ64 () );
    }
    Serial & Serial :: putMbr ( N32 mbr, const TimeBase & val )
    {
        Ticks tm ( val );
        return putMbr ( mbr, tm . toZ64 () );
    }
    
    // decimal floating-point Real
    Serial & Serial :: put ( const RsrcKfc & rsrc, const DecFltPoint < Z64 > & val )
    {
        DecFltPoint < Z64 > copy ( val );
        copy . denormalize ();
        return put ( Serial ( rsrc )
                     . putMbr ( 1, copy . getMantissa64 () )
                     . putMbr ( 2, copy . getExponent () )
            );
    }
    Serial & Serial :: put ( const RsrcKfc & rsrc, const DecFltPoint < Z128 > & val )
    {
        DecFltPoint < Z128 > copy ( val );
        copy . denormalize ();
        return put ( Serial ( rsrc )
                     . putMbr ( 1, copy . getMantissa128 () )
                     . putMbr ( 2, copy . getExponent () )
            );
    }
    Serial & Serial :: putTyped ( const RsrcKfc & rsrc, SerialType type, const DecFltPoint < Z64 > & val )
    {
        DecFltPoint < Z64 > copy ( val );
        copy . denormalize ();
        return putTyped ( type, Serial ( rsrc )
                          . putMbr ( 1, copy . getMantissa64 () )
                          . putMbr ( 2, copy . getExponent () )
            );
    }
    Serial & Serial :: putTyped ( const RsrcKfc & rsrc, SerialType type, const DecFltPoint < Z128 > & val )
    {
        DecFltPoint < Z128 > copy ( val );
        copy . denormalize ();
        return putTyped ( type, Serial ( rsrc )
                          . putMbr ( 1, copy . getMantissa128 () )
                          . putMbr ( 2, copy . getExponent () )
            );
    }
    Serial & Serial :: putMbr ( const RsrcKfc & rsrc, N32 mbr, const DecFltPoint < Z64 > & val )
    {
        DecFltPoint < Z64 > copy ( val );
        copy . denormalize ();
        return putMbr ( mbr, Serial ( rsrc )
                        . putMbr ( 1, copy . getMantissa64 () )
                        . putMbr ( 2, copy . getExponent () )
            );
    }
    Serial & Serial :: putMbr ( const RsrcKfc & rsrc, N32 mbr, const DecFltPoint < Z128 > & val )
    {
        DecFltPoint < Z128 > copy ( val );
        copy . denormalize ();
        return putMbr ( mbr, Serial ( rsrc )
                        . putMbr ( 1, copy . getMantissa128 () )
                        . putMbr ( 2, copy . getExponent () )
            );
    }

    // Text
    Serial & Serial :: put ( const RsrcKfc & rsrc, const CText & val )
    {
        return put ( Serial ( rsrc )
                     . putMbr ( 2, val . count () )
                     . putMbr ( 1, val . toCRgn () )
            );
    }
    
    Serial & Serial :: putTyped ( const RsrcKfc & rsrc, SerialType type, const CText & val )
    {
        return putTyped ( type, Serial ( rsrc )
                          . putMbr ( 2, val . count () )
                          . putMbr ( 1, val . toCRgn () )
            );
    }
    
    Serial & Serial :: putMbr ( const RsrcKfc & rsrc, N32 mbr, const CText & val )
    {
        return putMbr ( mbr, Serial ( rsrc )
                        . putMbr ( 2, val . count () )
                        . putMbr ( 1, val . toCRgn () )
            );
    }

    
    /*=====================================================*
     *                    Serializable                     *
     *=====================================================*/

    Serializable & Serializable :: operator = ( Serializable && ser ) noexcept
    {
        r = std :: move ( ser . r );
        return self;
    }

    Serializable & Serializable :: operator = ( const Serializable & ser ) noexcept
    {
        r = ser . r;
        return self;
    }

    Serializable :: Serializable ( Serializable && ser ) noexcept
        : r ( std :: move ( ser . r ) )
    {
    }

    Serializable :: Serializable ( const Serializable & ser ) noexcept
        : r ( ser . r )
    {
    }

    Serializable :: Serializable () noexcept
    {
    }

    Serializable :: ~ Serializable () noexcept
    {
    }

    Serializable & Serializable :: operator = ( const SerializableRef & ser ) noexcept
    {
        r = ser;
        return self;
    }

    Serializable :: Serializable ( const SerializableRef & ser ) noexcept
        : r ( ser )
    {
    }
}
