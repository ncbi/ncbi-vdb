/*

  vdb3.cmn.base64

 */

#include <vdb3/cmn/base64.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/cmn/str-buffer.hpp>

// auto-generated tables
#include "base64-tables.hpp"

namespace vdb3
{

    String Base64 :: encode ( const RsrcMem & rsrc, const CRgn & r ) const
    {
        return encode ( rsrc, r, encode_std_table );
    }

    Buffer Base64 :: decode ( const RsrcMem & rsrc, const String & base64 ) const
    {
        return decode ( rsrc, base64, allow_whitespace ? decode_std_table_ws : decode_std_table );
    }

    String Base64 :: decodeText ( const RsrcMem & rsrc, const String & base64 ) const
    {
        Buffer payload = decode ( rsrc, base64, allow_whitespace ? decode_std_table_ws : decode_std_table );
        return payloadToUTF8 ( rsrc, payload );
    }
    
    String Base64 :: urlEncode ( const RsrcMem & rsrc, const CRgn & r ) const
    {
        return encode ( rsrc, r, encode_url_table );
    }

    Buffer Base64 :: urlDecode ( const RsrcMem & rsrc, const String & base64url ) const
    {
        return decode ( rsrc, base64url, allow_whitespace ? decode_url_table_ws : decode_url_table );
    }

    String Base64 :: urlDecodeText ( const RsrcMem & rsrc, const String & base64url ) const
    {
        Buffer payload = decode ( rsrc, base64url, allow_whitespace ? decode_url_table_ws : decode_url_table );
        return payloadToUTF8 ( rsrc, payload );
    }

    void Base64 :: setWhitespacePolicy ( bool _allow_whitespace ) noexcept
    {
        allow_whitespace = _allow_whitespace;
    }

    void Base64 :: setPaddingPolicy ( bool _trim_padding ) noexcept
    {
        trim_padding = _trim_padding;
    }

    Base64 :: Base64 ( bool _allow_whitespace, bool _trim_padding ) noexcept
        : allow_whitespace ( _allow_whitespace )
        , trim_padding ( _trim_padding )
    {
    }

    Base64 & Base64 :: operator = ( Base64 && b64 ) noexcept
    {
        allow_whitespace = b64 . allow_whitespace;
        trim_padding = b64 . trim_padding;
        return self;
    }
    
    Base64 & Base64 :: operator = ( const Base64 & b64 ) noexcept
    {
        allow_whitespace = b64 . allow_whitespace;
        trim_padding = b64 . trim_padding;
        return self;
    }

    Base64 :: Base64 ( Base64 && b64 ) noexcept
        : allow_whitespace ( b64 . allow_whitespace )
        , trim_padding ( b64 . trim_padding )
    {
    }
    
    Base64 :: Base64 ( const Base64 & b64 ) noexcept
        : allow_whitespace ( b64 . allow_whitespace )
        , trim_padding ( b64 . trim_padding )
    {
    }
        
    Base64 :: Base64 () noexcept
        : allow_whitespace ( do_allow_whitespace )
        , trim_padding ( dont_trim_padding )
    {
    }
    
    Base64 :: ~ Base64 () noexcept
    {
    }

    String Base64 :: encode ( const RsrcMem & rsrc,  const CRgn & r, const ASCII encode_table [] ) const
    {
        // allow an empty source
        if ( r . isEmpty () )
            return String ();
        
        // this exception represents an internal error in any case
        if ( r . addr () == nullptr )
            throw NullArgumentException ( XP ( XLOC ) << "invalid source region" );

        // TBD - place an upper limit on data size
        // a very large payload will create a very large string allocation
        // and may indicate garbage that could result in a segfault
        if ( r . size () >= 0x40000000 )
        {
            throw SizeViolation (
                XP ( XLOC )
                << "source region too large: "
                << r . size ()
                << " bytes"
                );
        }

        KTRACE ( TRACE_PRG, "encoding %zu bytes of binary data", r . size () );

        // gather encoded output in a string
        // this is why we wanted to limit the data size
        Buffer eb ( rsrc, 1024 );
        StringBuffer encoding ( eb );

        // perform work in a stack-local buffer to reduce
        // memory manager thrashing when adding to "encoding" string
        size_t i, j;
        ASCII buff [ 4096 ];

        // accumulate 24 bits of input at a time into a 32-bit accumulator
        N32 acc;

        // walk across data as a block of octets
        const byte_t * js = r . addr ();
        size_t bytes = r . size ();

        // consume 3 octets of input while producing 4 output characters
        // JWT does not allow formatting, so no line breaks are involved
        for ( i = j = 0; i + 3 <= bytes; i += 3, j += 4 )
        {
            // bring in 24 bits in specific order
            acc
                = ( ( N32 ) js [ i + 0 ] << 16 )
                | ( ( N32 ) js [ i + 1 ] <<  8 )
                | ( ( N32 ) js [ i + 2 ] <<  0 )
                ;
            
            // we need to emit 4 bytes of output
            // flush the local buffer if it cannot hold them all
            if ( j > ( sizeof buff - 4 ) )
            {
                encoding += CTextLiteral ( buff, j );
                j = 0;
            }

            // produce the 4 bytes of output through the provided encoding table
            // each index MUST be 0..63
            buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
            buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
            buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];
            buff [ j + 3 ] = encode_table [ ( acc >>  0 ) & 0x3F ];
        }

        // at this point, the data block is either completely converted
        // or has 1 or 2 bytes left over, since ( bytes % 3 ) is in { 0, 1, 2 }
        // we know "i" is a multiple of 3, and ( bytes - i ) == ( bytes % 3 )
        switch ( bytes - i )
        {
            case 0:
                // everything is complete
                break;
                
            case 1:

                // 1 octet left over
                // place it in the highest part of 24-bit accumulator
                acc
                    = ( ( N32 ) js [ i + 0 ] << 16 )
                    ;

                // we need to emit 4 bytes of output
                // flush buffer if insufficient space is available
                if ( j > ( sizeof buff - 4 ) )
                {
                    encoding += CTextLiteral ( buff, j );
                    j = 0;
                }

                // emit single octet split between two encoding characters
                buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
                buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];

                // pad the remaining two with padding character
                buff [ j + 2 ] = '=';
                buff [ j + 3 ] = '=';

                // total number of characters in buffer with or without padding
                j += trim_padding ? 2 : 4;
                
                break;
                
            case 2:
                
                // 2 octets left over
                // place them in the upper part of 24-bit accumulator
                acc
                    = ( ( N32 ) js [ i + 0 ] << 16 )
                    | ( ( N32 ) js [ i + 1 ] <<  8 )
                    ;

                // test for buffer space as above
                if ( j > ( sizeof buff - 4 ) )
                {
                    encoding += CTextLiteral ( buff, j );
                    j = 0;
                }

                // emit the two octets split across three encoding characters
                buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
                buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
                buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];

                // pad the remainder with padding character
                buff [ j + 3 ] = '=';
                
                // total number of characters in buffer with or without padding
                j += trim_padding ? 3 : 4;
                
                break;
                
            default:

                // this is theoretically impossible
                throw InternalError ( XP ( XLOC ) << "1 - aaaah!" );
        }

        // if "j" is not 0 at this point, it means
        // there are encoding characters in the stack-local buffer
        // append them to the encoding string
        if ( j != 0 )
            encoding += CTextLiteral ( buff, j );

        // done.
        return encoding . stealString ();
    }
    
    Buffer Base64 :: decode ( const RsrcMem & rsrc, const String & encoding, const char decode_table [] ) const
    {
        // base the estimate of decoded size on input size
        // this can be over-estimated due to embedded padding or formatting characters
        // however, these are prohibited in JWT so the size should be nearly exact
        size_t i, j, len = encoding . size ();

        KTRACE ( TRACE_PRG, "decoding %zu bytes of ASCII data", len );
        
        N32 acc, ac;
        const byte_t * en = ( const byte_t * ) encoding . data () . addr ();
        
        // the returned buffer should be 3/4 the size of the input string,
        // provided that there are no padding bytes in the input
        size_t bytes = ( ( len + 3 ) / 4 ) * 3;
        
        // create an output buffer
        Buffer payload ( rsrc, bytes );
        byte_t * buff = payload . buffer () . addr ();

        // generate a capacity limit, beyond which
        // the buffer must be extended
        size_t cap_limit = payload . capacity () - 3;

        // walk across the input string a byte at a time
        // avoid temptation to consume 4 bytes at a time,
        // in order to be robust to any allowed stray characters
        // NB - if this proves to be a performance issue, it can
        // be optimized in the future.
        for ( i = j = 0, acc = ac = 0; i < len; ++ i )
        {
            // return from table is a SIGNED entity
            int byte = decode_table [ en [ i ] ];

            // non-negative lookups are valid translations
            if ( byte >= 0 )
            {
                // must be 0..63
                assert ( byte < 64 );

                // shift 6 bits into accumulator
                acc <<= 6;
                acc |= byte;

                // if the number of codes in accumulator is 4 ( i.e. 24 bits )
                if ( ++ ac == 4 )
                {
                    // test capacity of output
                    if ( j > cap_limit )
                    {
                        payload . increaseCapacity ();
                        buff = ( byte_t * ) payload . buffer () . addr ();
                        cap_limit = payload . capacity () - 3;
                    }

                    // put 3 octets into payload
                    buff [ j + 0 ] = ( byte_t ) ( acc >> 16 );
                    buff [ j + 1 ] = ( byte_t ) ( acc >>  8 );
                    buff [ j + 2 ] = ( byte_t ) ( acc >>  0 );

                    // clear the accumulator
                    ac = 0;

                    // keep track of size
                    // NB - this is not YET reflected in payload
                    j += 3;
                }
            }

            // NEGATIVE lookups have to be interpreted
            else
            {
                // the special value -2 means padding
                // which indicates the end of the input
                if ( byte == -2 )
                {
                    // TBD - ensure that these are 1 or 2 trailing '=' bytes
                    // and that is the end of the string. Only break then,
                    // and throw an exception otherwise.
                    break;
                }

                // the special value -3 would indicate ignore
                // but it's not allowed in JWT and so is not expected to be in table
                // any other value ( notably -1 ) is illegal
                if ( byte != -3 )
                    throw InvalidInputException ( XP ( XLOC ) << "illegal input characters" );
            }
        }

        // test the number of 6-bit codes remaining in the accumulator
        switch ( ac )
        {
            case 0:
                // none - everything has been converted
                break;
                
            case 1:
                // encoding granularity is an octet - 8 bits
                // it MUST be split across two codes - 6 bits each, i.e. 12 bits
                // having only 6 bits in accumulator is illegal
                throw InvalidInputException ( XP ( XLOC ) << "malformed input - group with 1 base64 encode character" );
                
            case 2:
                
                // fill accumulator with two padding codes
                // NB - not strictly necessary, but keeps code regular and readable
                acc <<= 12;
                
                // check buffer for space
                if ( j >= payload . capacity () )
                {
                    payload . increaseCapacity ( 1 );
                    buff = ( byte_t * ) payload . buffer () . addr ();
                }
                
                // set additional octet
                buff [ j + 0 ] = ( byte_t ) ( acc >> 16 );

                // account for size
                // NB - this is not YET reflected in payload
                j += 1;
                break;
                
            case 3:
                
                // fill accumulator with padding
                acc <<= 6;
                
                // check buffer for space
                if ( j + 1 >= payload . capacity () )
                {
                    payload . increaseCapacity ( 2 );
                    buff = ( byte_t * ) payload . buffer () . addr ();
                }

                // set additional bytes
                buff [ j + 0 ] = ( byte_t ) ( acc >> 16 );
                buff [ j + 1 ] = ( byte_t ) ( acc >>  8 );
                
                j += 2;
                break;
                
            default:

                // theoretically impossible
                throw InternalError ( XP ( XLOC ) << "2 - aaah!" );
        }

        // NOW store size on object
        payload . setSize ( j );

        // return the object
        return payload;
    }

    String Base64 :: payloadToUTF8 ( const RsrcMem & rsrc, const Buffer & payload )
    {
        const UTF8 * buff = ( const UTF8 * ) payload . buffer () . addr ();
        size_t size = payload . size ();
        return String ( rsrc, CTextLiteral ( buff, size ) );
    }
    
}
