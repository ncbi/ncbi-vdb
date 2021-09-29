/*

  vdb3.cmn.serial

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/cmn/buffer.hpp>
#include <vdb3/kfc/zigzag.hpp>
#include <vdb3/kfc/time.hpp>
#include <vdb3/kfc/decflt.hpp>
#include <vdb3/cmn/text.hpp>

namespace vdb3
{
    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class Serializable;
    

    /*=====================================================*
     *                      SerialFmt                      *
     *=====================================================*/

    /**
     * @enum SerialFmt
     * @brief formats within wire protocol
     */
    enum SerialFmt
    {
        sfVarSize       = 0,  // base-128 variable size
        sfFixed16       = 1,  // 16-bit fixed size
        sfFixed32       = 2,  // 32-bit fixed size
        sfFixed48       = 3,  // 48-bit fixed size
        sfFixed64       = 4,  // 64-bit fixed size
        sfBlob          = 5,  // ( variable-sized size, octet-seq )
        sfSeq           = 6   // ( variable-sized count, elem-seq )
    };

    /*=====================================================*
     *                     SerialBase                      *
     *=====================================================*/

    /**
     * SerialBase
     *  @brief a serializing front end to a binary buffer
     */
    class SerialBase
    {
    public:

        /*=================================================*
         *   IMPLICITLY TYPED ( and formatted ) ELEMENTS   *
         *=================================================*/

        // binary Natural { sfVarSize }
        void putITypeVarN32 ( N32 val );
        void putITypeVarN64 ( N64 val );
        void putITypeVarN128 ( const N128 & val );

        // binary Natural { sfFixed16 .. sfFixed64 }
        void putITypeFixN16 ( N16 val );
        void putITypeFixN32 ( N32 val );
        void putITypeFixN48 ( N64 val );
        void putITypeFixN64 ( N64 val );

        // zigzag Integer { sfVarSize }
        void putITypeVarZ64 ( const ZigZag < N64 > & val );
        void putITypeVarZ128 ( const ZigZag < N128 > & val );

        // binary floating-point Real { sfFixed32, sfFixed64 }
        void putITypeFixRb32 ( R32 val );
        void putITypeFixRb64 ( R64 val );

        // Blob
        void putITypeBlob ( const CRgn & val );

        // Sequence
        void openUTypeSeq ( count_t count );

        /*=================================================*
         *  EXPLICITLY TYPED ( auto-formatted ) ELEMENTS   *
         *=================================================*/

        // binary Natural { sfVarSize, sfFixed16 .. sfFixed64 }
        void putXTypeN16 ( N32 type_or_mbr_id, N16 val );
        void putXTypeN32 ( N32 type_or_mbr_id, N32 val );
        void putXTypeN64 ( N32 type_or_mbr_id, N64 val );
        void putXTypeN128 ( N32 type_or_mbr_id, const N128 & val );

        // zigzag Integer { sfVarSize, sfFixed16 .. sfFixed64 }
        void putXTypeZ64 ( N32 type_or_mbr_id, const ZigZag < N64 > & val );
        void putXTypeZ128 ( N32 type_or_mbr_id, const ZigZag < N128 > & val );

        // binary floating-point Real { sfFixed32, sfFixed64 }
        void putXTypeRb32 ( N32 type_or_mbr_id, R32 val );
        void putXTypeRb64 ( N32 type_or_mbr_id, R64 val );

        // Blob
        void putXTypeBlob ( N32 type_or_mbr_id, const CRgn & val );

        // Sequence
        void openXTypeSeq ( N32 type_or_mbr_id, count_t count );
        
        /*=================================================*
         *                     RESULT                      *
         *=================================================*/
        
        // retrieve buffer contents
        CRgn content () const noexcept;

        // forget everything - useful for exception handling
        void reinitialize ();


        CXX_RULE_OF_SIX_VDNE ( SerialBase );
        
        SerialBase ( const RsrcKfc & rsrc );
        
    private:

        void prefix ( N32 type_or_mbr_id, SerialFmt fmt );
        
        Buffer b;
        
    };

    /*=====================================================*
     *                       Serial                        *
     *=====================================================*/

    /**
     * Serial
     *  @brief a serializing front end to a binary buffer
     *
     *  this class deals with the reality of C++ intrinsic
     *  types, which are ambiguous and make little sense.
     *  so this is the public class that acts as a driver
     *  for the implementation base class.
     */
    class Serial : protected SerialBase
    {
    public:

        typedef Serial S;
        typedef SerialBase SB;

        /*=================================================*
         *   IMPLICITLY TYPED ( and formatted ) ELEMENTS   *
         *=================================================*/

        // Boolean
        inline S & put ( bool val )
        { SB :: putITypeVarN32 ( ( N32 ) ( val != 0 ) ); return self; }

        // Natural numbers
        inline S & put ( unsigned short int val )
        { SB :: putITypeVarN32 ( ( N32 ) val ); return self; }
        inline S & put ( unsigned int val )
        { SB :: putITypeVarN32 ( ( N32 ) val ); return self; }
        inline S & put ( unsigned long int val )
        { SB :: putITypeVarN64 ( ( N64 ) val ); return self; }
        inline S & put ( unsigned long long int val )
        { SB :: putITypeVarN64 ( ( N64 ) val ); return self; }
        inline S & put ( const N128 & val )
        { SB :: putITypeVarN128 ( val ); return self; }

        // fixed-size Natural numbers
        inline S & putFixed ( unsigned short int val )
        { SB :: putITypeFixN16 ( ( N16 ) val ); return self; }
        inline S & putFixed ( unsigned int val )
        { SB :: putITypeFixN32 ( ( N32 ) val ); return self; }
        inline S & putFixed ( unsigned long int val )
        { SB :: putITypeFixN64 ( ( N64 ) val ); return self; }
        inline S & putFixed ( unsigned long long int val )
        { SB :: putITypeFixN64 ( ( N64 ) val ); return self; }

        // Integers
        inline S & put ( signed short int val )
        { SB :: putITypeVarZ64 ( ZigZag < N64 > ( val ) ); return self; }
        inline S & put ( signed int val )
        { SB :: putITypeVarZ64 ( ZigZag < N64 > ( val ) ); return self; }
        inline S & put ( signed long int val )
        { SB :: putITypeVarZ64 ( ZigZag < N64 > ( val ) ); return self; }
        inline S & put ( signed long long int val )
        { SB :: putITypeVarZ64 ( ZigZag < N64 > ( val ) ); return self; }
        inline S & put ( const Z128 & val )
        { SB :: putITypeVarZ128 ( ZigZag < N128 > ( val ) ); return self; }

        // zigzag-encoded Integers
        inline S & put ( const ZigZag < N64 > & val )
        { SB :: putITypeVarZ64 ( val ); return self; }
        inline S & put ( const ZigZag < N128 > & val )
        { SB :: putITypeVarZ128 ( val ); return self; }

        // fixed-size Integers
        S & putFixed ( signed short int val );
        S & putFixed ( signed int val );
        S & putFixed ( signed long int val );
        S & putFixed ( signed long long int val );

        // IEEE-754 binary floating point Real
        inline S & put ( float val )
        { SB :: putITypeFixRb32 ( ( R32 ) val ); return self; }
        inline S & put ( double val )
        { SB :: putITypeFixRb64 ( ( R64 ) val ); return self; }
        inline S & put ( long double val )
        { SB :: putITypeFixRb64 ( ( R64 ) val ); return self; }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        inline S & put ( R128 val )
        { SB :: putITypeFixRb64 ( ( R64 ) val ); return self; }
#endif
        // Blob
        inline S & put ( const CRgn & val )
        { SB :: putITypeBlob ( val ); return self; }
        inline S & put ( const Serial & val )
        { SB :: putITypeBlob ( val . content () ); return self; }

        // Serializable
        inline S & put ( const RsrcKfc & rsrc, const Serializable & val );

        // Sequence
        inline S & openSeq ( count_t count )
        { SB :: openUTypeSeq ( count ); return self; }
        
        /*=================================================*
         *  EXPLICITLY TYPED ( auto-formatted ) ELEMENTS   *
         *=================================================*/

        // Boolean
        inline S & putTyped( SerialType type, bool val )
        { SB :: putXTypeN32 ( type, ( N32 ) ( val != 0 ) ); return self; }

        // Natural numbers
        inline S & putTyped( SerialType type, unsigned short int val )
        { SB :: putXTypeN32 ( type, ( N32 ) val ); return self; }
        inline S & putTyped( SerialType type, unsigned int val )
        { SB :: putXTypeN32 ( type, ( N32 ) val ); return self; }
        inline S & putTyped( SerialType type, unsigned long int val )
        { SB :: putXTypeN64 ( type, ( N64 ) val ); return self; }
        inline S & putTyped( SerialType type, unsigned long long int val )
        { SB :: putXTypeN64 ( type, ( N64 ) val ); return self; }
        inline S & putTyped( SerialType type, const N128 & val )
        { SB :: putXTypeN128 ( type, val ); return self; }

        // Integers
        inline S & putTyped( SerialType type, signed short int val )
        { SB :: putXTypeZ64 ( type, ZigZag < N64 > ( val ) ); return self; }
        inline S & putTyped( SerialType type, signed int val )
        { SB :: putXTypeZ64 ( type, ZigZag < N64 > ( val ) ); return self; }
        inline S & putTyped( SerialType type, signed long int val )
        { SB :: putXTypeZ64 ( type, ZigZag < N64 > ( val ) ); return self; }
        inline S & putTyped( SerialType type, signed long long int val )
        { SB :: putXTypeZ64 ( type, ZigZag < N64 > ( val ) ); return self; }
        inline S & putTyped( SerialType type, const Z128 & val )
        { SB :: putXTypeZ128 ( type, ZigZag < N128 > ( val ) ); return self; }

        // zigzag-encoded Integers
        inline S & putTyped( SerialType type, const ZigZag < N64 > & val )
        { SB :: putXTypeZ64 ( type, val ); return self; }
        inline S & putTyped( SerialType type, const ZigZag < N128 > & val )
        { SB :: putXTypeZ128 ( type, val ); return self; }

        // IEEE-754 binary floating point Real
        inline S & putTyped( SerialType type, float val )
        { SB :: putXTypeRb32 ( type, ( R32 ) val ); return self; }
        inline S & putTyped( SerialType type, double val )
        { SB :: putXTypeRb64 ( type, ( R64 ) val ); return self; }
        inline S & putTyped( SerialType type, long double val )
        { SB :: putXTypeRb64 ( type, ( R64 ) val ); return self; }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        inline S & putTyped( SerialType type, R128 val )
        { SB :: putXTypeRb64 ( type, ( R64 ) val ); return self; }
#endif
        // Blob
        inline S & putTyped( SerialType type, const CRgn & val )
        { SB :: putXTypeBlob ( type, val ); return self; }
        inline S & putTyped( SerialType type, const Serial & val )
        { SB :: putXTypeBlob ( type, val . content () ); return self; }

        // Serializable
        inline S & putTyped ( const RsrcKfc & rsrc, SerialType type, const Serializable & val );

        // Sequence
        inline S & openTypedSeq ( SerialType type, count_t count )
        { SB :: openXTypeSeq ( type, count ); return self; }
        
        /*=================================================*
         *         EXTERNALLY TYPED TUPLE MEMBERS          *
         *=================================================*/

        // Boolean
        inline S & putMbr ( N32 mbr, bool val )
        { SB :: putXTypeN32 ( mbr, ( N32 ) ( val != 0 ) ); return self; }

        // Natural numbers
        inline S & putMbr ( N32 mbr, unsigned short int val )
        { SB :: putXTypeN32 ( mbr, ( N32 ) val ); return self; }
        inline S & putMbr ( N32 mbr, unsigned int val )
        { SB :: putXTypeN32 ( mbr, ( N32 ) val ); return self; }
        inline S & putMbr ( N32 mbr, unsigned long int val )
        { SB :: putXTypeN64 ( mbr, ( N64 ) val ); return self; }
        inline S & putMbr ( N32 mbr, unsigned long long int val )
        { SB :: putXTypeN64 ( mbr, ( N64 ) val ); return self; }
        inline S & putMbr ( N32 mbr, const N128 & val )
        { SB :: putXTypeN128 ( mbr, val ); return self; }

        // Integers
        inline S & putMbr ( N32 mbr, signed short int val )
        { SB :: putXTypeZ64 ( mbr, ZigZag < N64 > ( val ) ); return self; }
        inline S & putMbr ( N32 mbr, signed int val )
        { SB :: putXTypeZ64 ( mbr, ZigZag < N64 > ( val ) ); return self; }
        inline S & putMbr ( N32 mbr, signed long int val )
        { SB :: putXTypeZ64 ( mbr, ZigZag < N64 > ( val ) ); return self; }
        inline S & putMbr ( N32 mbr, signed long long int val )
        { SB :: putXTypeZ64 ( mbr, ZigZag < N64 > ( val ) ); return self; }
        inline S & putMbr ( N32 mbr, const Z128 & val )
        { SB :: putXTypeZ128 ( mbr, ZigZag < N128 > ( val ) ); return self; }

        // zigzag-encoded Integers
        inline S & putMbr ( N32 mbr, const ZigZag < N64 > & val )
        { SB :: putXTypeZ64 ( mbr, val ); return self; }
        inline S & putMbr ( N32 mbr, const ZigZag < N128 > & val )
        { SB :: putXTypeZ128 ( mbr, val ); return self; }
        
        // IEEE-754 binary floating point Real
        inline S & putMbr ( N32 mbr, float val )
        { SB :: putXTypeRb32 ( mbr, ( R32 ) val ); return self; }
        inline S & putMbr ( N32 mbr, double val )
        { SB :: putXTypeRb64 ( mbr, ( R64 ) val ); return self; }
        inline S & putMbr ( N32 mbr, long double val )
        { SB :: putXTypeRb64 ( mbr, ( R64 ) val ); return self; }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        inline S & putMbr ( N32 mbr, R128 val )
        { SB :: putXTypeRb64 ( mbr, ( R64 ) val ); return self; }
#endif
        // Blob
        inline S & putMbr ( N32 mbr, const CRgn & val )
        { SB :: putXTypeBlob ( mbr, val ); return self; }
        inline S & putMbr ( N32 mbr, const Serial & val )
        { SB :: putXTypeBlob ( mbr, val . content () ); return self; }

        // Serializable
        inline S & putMbr ( const RsrcKfc & rsrc, N32 mbr, const Serializable & val );

        // Sequence
        inline S & openMbrSeq ( N32 mbr, count_t count )
        { SB :: openXTypeSeq ( mbr, count ); return self; }
        
        /*=================================================*
         *                 INTRINSIC TYPES                 *
         *=================================================*/

        // Time
        S & put ( const TimeBase & val );
        S & putTyped ( SerialType type, const TimeBase & val );
        S & putMbr ( N32 mbr, const TimeBase & val );

        // decimal floating-point Real
        S & put ( const RsrcKfc & rsrc, const DecFltPoint < Z64 > & val );
        S & put ( const RsrcKfc & rsrc, const DecFltPoint < Z128 > & val );
        S & putTyped ( const RsrcKfc & rsrc, SerialType type, const DecFltPoint < Z64 > & val );
        S & putTyped ( const RsrcKfc & rsrc, SerialType type, const DecFltPoint < Z128 > & val );
        S & putMbr ( const RsrcKfc & rsrc, N32 mbr, const DecFltPoint < Z64 > & val );
        S & putMbr ( const RsrcKfc & rsrc, N32 mbr, const DecFltPoint < Z128 > & val );

        // Text
        S & put ( const RsrcKfc & rsrc, const CText & val );
        S & putTyped ( const RsrcKfc & rsrc, SerialType type, const CText & val );
        S & putMbr ( const RsrcKfc & rsrc, N32 mbr, const CText & val );
        
        /*=================================================*
         *                     RESULT                      *
         *=================================================*/

        // retrieve buffer contents
        CRgn content () const noexcept
        { return SB :: content (); }

        // forget everything - useful for exception handling
        void reinitialize ()
        { SB :: reinitialize (); }

        inline Serial & operator = ( Serial && s ) noexcept
        { SerialBase :: operator = ( std :: move ( s ) ); return self; }
        inline Serial & operator = ( const Serial & s ) noexcept
        { SerialBase :: operator = ( s ); return self; }

        Serial ( Serial && s ) noexcept
            : SerialBase ( std :: move ( s ) ) {}
        Serial ( const Serial & s ) noexcept
            : SerialBase ( s ) {}
        Serial () noexcept {}
        ~ Serial () noexcept {}
        
        Serial ( const RsrcKfc & rsrc )
            : SerialBase ( rsrc ) {}
    };
    

    /*=====================================================*
     *                   SerializableItf                   *
     *=====================================================*/

    
    /**
     * SerializableItf
     *  @brief a common interface for serializing objects
     */
    interface SerializableItf
    {

        /**
         * put
         *  @brief serialize self to Serial
         *  @param s a Serial buffer
         */
        MSG ( void put ( const RsrcKfc & rsrc, Serial & s ) const );

        /**
         * putTyped
         *  @brief serialize self to Serial with type id
         *  @param s a Serial buffer
         *  @param type a constant type id
         */
        MSG ( void putTyped ( const RsrcKfc & rsrc, Serial & s, SerialType type ) const );

        /**
         * putTyped
         *  @brief serialize self to Serial with mbr id
         *  @param s a Serial buffer
         *  @param mbr a 1-based mbr id constant within implementation
         */
        MSG ( void putMbr ( const RsrcKfc & rsrc, Serial & s, N32 mbr ) const );

        /**
         * ~SerializableItf
         *  @brief releases and destroys object
         */
        virtual ~ SerializableItf () noexcept {}
    };

    /**
     * @typedef SerializableRef
     * @brief a shared reference to an SerializableItf
     */
    typedef IRef < SerializableItf > SerializableRef;

    
    /*=====================================================*
     *                    Serializable                     *
     *=====================================================*/

    class Serializable
    {
    public:
        
        /**
         * put
         *  @brief serialize self to Serial
         *  @param s a Serial buffer
         */
        inline Serial & put ( const RsrcKfc & rsrc, Serial & s ) const
        { r -> put ( rsrc, s ); return s; }

        /**
         * putTyped
         *  @brief serialize self to Serial with type id
         *  @param s a Serial buffer
         *  @param type a constant type id
         */
        inline Serial & putTyped ( const RsrcKfc & rsrc, Serial & s, SerialType type ) const
        { r -> putTyped ( rsrc, s, type ); return s; }

        /**
         * putTyped
         *  @brief serialize self to Serial with mbr id
         *  @param s a Serial buffer
         *  @param mbr a 1-based mbr id constant within implementation
         */
        inline Serial & putMbr ( const RsrcKfc & rsrc, Serial & s, N32 mbr ) const
        { r -> putMbr ( rsrc, s, mbr ); return s; }

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_NE ( Serializable );

    private:
        
        SerializableRef r;
    };

    /*=====================================================*
     *                       INLINES                       *
     *=====================================================*/

    inline Serial & Serial :: put ( const RsrcKfc & rsrc, const Serializable & val )
    { return val . put ( rsrc, self ); }
    inline Serial & Serial :: putTyped ( const RsrcKfc & rsrc, SerialType type, const Serializable & val )
    { return val . putTyped ( rsrc, self, type ); }
    inline Serial & Serial :: putMbr ( const RsrcKfc & rsrc, N32 mbr, const Serializable & val )
    { return val . putMbr ( rsrc, self, mbr ); }


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
