/*

  vdb3.cmn.dserial

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/cmn/buffer.hpp>
#include <vdb3/cmn/reader.hpp>
#include <vdb3/kfc/zigzag.hpp>
#include <vdb3/kfc/time.hpp>
#include <vdb3/kfc/decflt.hpp>
#include <vdb3/cmn/text.hpp>

namespace vdb3
{
    /*=====================================================*
     *                    DeserialBase                     *
     *=====================================================*/

    /**
     * DeserialBase
     *  @brief a deserializing front end to a CRgn
     *
     *  NB - see <vdb3/cmn/serial.hpp> for SerialFmt definition.
     */
    class DeserialBase
    {
    public:

        /*=================================================*
         *            IMPLICITLY TYPED ELEMENTS            *
         *=================================================*/

        // binary Natural { sfVarSize }
        void getITypeVarN16 ( N16 & val );
        void getITypeVarN32 ( N32 & val );
        void getITypeVarN64 ( N64 & val );
        void getITypeVarN128 ( N128 & val );

        // binary Natural { sfFixed16 .. sfFixed64 }
        void getITypeFixN16 ( N16 & val );
        void getITypeFixN32 ( N32 & val );
        void getITypeFixN48 ( N64 & val );
        void getITypeFixN64 ( N64 & val );

        // zigzag Integer { sfVarSize }
        void getITypeVarZ16 ( Z16 & val );
        void getITypeVarZ32 ( Z32 & val );
        void getITypeVarZ64 ( Z64 & val );
        void getITypeVarZ128 ( Z128 & val );

        // zigzag Integer { sfFixed16 .. sfFixed64 }
        void getITypeFixZ16 ( Z16 & val );
        void getITypeFixZ32 ( Z32 & val );
        void getITypeFixZ48 ( Z64 & val );
        void getITypeFixZ64 ( Z64 & val );

        // binary floating-point Real { sfFixed32, sfFixed64 }
        void getITypeFixRb32 ( R32 & val );
        void getITypeFixRb64 ( R64 & val );

        // Blob
        void getITypeBlob ( CRgn & val );

        // Sequence
        void getUTypeSeqCount ( count_t & count );

        /*=================================================*
         *            EXPLICITLY TYPED ELEMENTS            *
         *=================================================*/

        // the type or member code and fmt
        N32 getXTypeFmt ( N32 & fmt );

        // binary Natural { sfVarSize, sfFixed16 .. sfFixed64 }
        void getXTypeN16 ( N32 fmt, N16 & val );
        void getXTypeN32 ( N32 fmt, N32 & val );
        void getXTypeN64 ( N32 fmt, N64 & val );
        void getXTypeN128 ( N32 fmt, N128 & val );

        // zigzag Integer { sfVarSize, sfFixed16 .. sfFixed64 }
        void getXTypeZ16 ( N32 fmt, Z16 & val );
        void getXTypeZ32 ( N32 fmt, Z32 & val );
        void getXTypeZ64 ( N32 fmt, Z64 & val );
        void getXTypeZ128 ( N32 fmt, Z128 & val );

        // binary floating-point Real { sfFixed32, sfFixed64 }
        void getXTypeRb32 ( N32 fmt, R32 & val );
        void getXTypeRb64 ( N32 fmt, R64 & val );

        // Blob
        void getXTypeBlob ( N32 fmt, CRgn & val );

        // Sequence
        void getXTypeSeqCount ( N32 fmt, count_t & count );

        // has more data?
        bool hasMoreData () const noexcept;
        
        CXX_RULE_OF_SIX_VDNE ( DeserialBase );

        DeserialBase & operator = ( const CRgn & rgn ) noexcept;
        DeserialBase ( const CRgn & rgn ) noexcept;
        
    protected:

        virtual size_t moreData ( size_t requested );

        CRgn rgn;
        size_t offset;
    };

    /*=====================================================*
     *                      Deserial                       *
     *=====================================================*/

    /**
     * Deserial
     *  @brief a deserializing front end to a CRgn
     *
     *  NB - see <vdb3/cmn/serial.hpp> for SerialFmt definition.
     */
    class Deserial : protected DeserialBase, public TextFactory
    {
    public:

        typedef Deserial D;
        typedef DeserialBase DB;
        
        /*=================================================*
         *            IMPLICITLY TYPED ELEMENTS            *
         *=================================================*/

        // Boolean
        D & get ( bool & val );

        // Natural numbers
        inline D & get ( unsigned short int & val )
        { DB :: getITypeVarN16 ( val ); return self; }
        inline D & get ( unsigned int & val )
        { DB :: getITypeVarN32 ( val ); return self; }
        D & get ( unsigned long int & val );
        inline D & get ( unsigned long long int & val )
        { DB :: getITypeVarN64 ( val ); return self; }
        inline D & get ( N128 & val )
        { DB :: getITypeVarN128 ( val ); return self; }

        // fixed-size Natural numbers
        inline D & getFixed ( unsigned short int & val )
        { DB :: getITypeFixN16 ( val ); return self; }
        inline D & getFixed ( unsigned int & val )
        { DB :: getITypeFixN32 ( val ); return self; }
        D & getFixed ( unsigned long int & val );
        inline D & getFixed ( unsigned long long int & val )
        { DB :: getITypeFixN64 ( val ); return self; }

        // Integers
        inline D & get ( signed short int & val )
        { DB :: getITypeVarZ16 ( val ); return self; }
        inline D & get ( signed int & val )
        { DB :: getITypeVarZ32 ( val ); return self; }
        D & get ( signed long int & val );
        inline D & get ( signed long long int & val )
        { DB :: getITypeVarZ64 ( val ); return self; }
        inline D & get ( Z128 & val )
        { DB :: getITypeVarZ128 ( val ); return self; }

        // fixed-size Integers
        inline D & getFixed ( signed short int & val )
        { DB :: getITypeFixZ16 ( val ); return self; }
        inline D & getFixed ( signed int & val )
        { DB :: getITypeFixZ32 ( val ); return self; }
        D & getFixed ( signed long int & val );
        inline D & getFixed ( signed long long int & val )
        { DB :: getITypeFixZ64 ( val ); return self; }

        // IEEE-754 binary floating point Real
        inline D & get ( float & val )
        { DB :: getITypeFixRb32 ( val ); return self; }
        inline D & get ( double & val )
        { DB :: getITypeFixRb64 ( val ); return self; }
        
        D & get ( long double & val );
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        D & get ( R128 & val );
#endif
        // Blob
        inline D & get ( CRgn & val )
        { DB :: getITypeBlob ( val ); return self; }
        D & get ( Deserial & val );

        // Sequence
        inline D & getSeqCount ( count_t & count )
        { DB :: getUTypeSeqCount ( count ); return self; }
        
        /*=================================================*
         *            EXPLICITLY TYPED ELEMENTS            *
         *=================================================*/

        // SerialType
        inline SerialType getTypeId ( N32 & fmt )
        { return DB :: getXTypeFmt ( fmt ); }
            
        // Boolean
        void getTyped ( N32 fmt, bool & val );

        // Natural numbers
        inline void getTyped ( N32 fmt, unsigned short int & val )
        { DB :: getXTypeN16 ( fmt, val ); }
        inline void getTyped ( N32 fmt, unsigned int & val )
        { DB :: getXTypeN32 ( fmt, val ); }
        void getTyped ( N32 fmt, unsigned long int & val );
        inline void getTyped ( N32 fmt, unsigned long long int & val )
        { DB :: getXTypeN64 ( fmt, val ); }
        inline void getTyped ( N32 fmt, N128 & val )
        { DB :: getXTypeN128 ( fmt, val ); }

        // Integers
        inline void getTyped ( N32 fmt, signed short int & val )
        { DB :: getXTypeZ16 ( fmt, val ); }
        inline void getTyped ( N32 fmt, signed int & val )
        { DB :: getXTypeZ32 ( fmt, val ); }
        void getTyped ( N32 fmt, signed long int & val );
        inline void getTyped ( N32 fmt, signed long long int & val )
        { DB :: getXTypeZ64 ( fmt, val ); }
        inline void getTyped ( N32 fmt, Z128 & val )
        { DB :: getXTypeZ128 ( fmt, val ); }

        // IEEE-754 binary floating point Real
        inline void getTyped ( N32 fmt, float & val )
        { DB :: getXTypeRb32 ( fmt, val ); }
        inline void getTyped ( N32 fmt, double & val )
        { DB :: getXTypeRb64 ( fmt, val ); }
        
        void getTyped ( N32 fmt, long double & val );
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        void getTyped ( N32 fmt, R128 & val );
#endif
        // Blob
        inline void getTyped ( N32 fmt, CRgn & val )
        { DB :: getXTypeBlob ( fmt, val ); }
        void getTyped ( N32 fmt, Deserial & val );

        // Sequence
        inline void getTypedSeqCount ( N32 fmt, count_t & count )
        { DB :: getXTypeSeqCount ( fmt, count ); }
        
        /*=================================================*
         *                  TUPLE MEMBERS                  *
         *=================================================*/

        // Member
        inline N32 getMbrId ( N32 & fmt )
        { return DB :: getXTypeFmt ( fmt ); }

        // Boolean
        void getMbr ( N32 fmt, bool & val );

        // Natural numbers
        inline void getMbr ( N32 fmt, unsigned short int & val )
        { DB :: getXTypeN16 ( fmt, val ); }
        inline void getMbr ( N32 fmt, unsigned int & val )
        { DB :: getXTypeN32 ( fmt, val ); }
        void getMbr ( N32 fmt, unsigned long int & val );
        inline void getMbr ( N32 fmt, unsigned long long int & val )
        { DB :: getXTypeN64 ( fmt, val ); }
        inline void getMbr ( N32 fmt, N128 & val )
        { DB :: getXTypeN128 ( fmt, val ); }

        // Integers
        inline void getMbr ( N32 fmt, signed short int & val )
        { DB :: getXTypeZ16 ( fmt, val ); }
        inline void getMbr ( N32 fmt, signed int & val )
        { DB :: getXTypeZ32 ( fmt, val ); }
        void getMbr ( N32 fmt, signed long int & val );
        inline void getMbr ( N32 fmt, signed long long int & val )
        { DB :: getXTypeZ64 ( fmt, val ); }
        inline void getMbr ( N32 fmt, Z128 & val )
        { DB :: getXTypeZ128 ( fmt, val ); }

        // IEEE-754 binary floating point Real
        inline void getMbr ( N32 fmt, float & val )
        { DB :: getXTypeRb32 ( fmt, val ); }
        inline void getMbr ( N32 fmt, double & val )
        { DB :: getXTypeRb64 ( fmt, val ); }
        
        void getMbr ( N32 fmt, long double & val );
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
        void getMbr ( N32 fmt, R128 & val );
#endif
        // Blob
        inline void getMbr ( N32 fmt, CRgn & val )
        { DB :: getXTypeBlob ( fmt, val ); }
        void getMbr ( N32 fmt, Deserial & val );

        // Sequence
        inline void getMbrSeqCount ( N32 fmt, count_t & count )
        { DB :: getXTypeSeqCount ( fmt, count ); }
        
        /*=================================================*
         *                 INTRINSIC TYPES                 *
         *=================================================*/

        // Time
        void get ( Ticks & val );
        void getTyped ( N32 fmt, Ticks & val );
        void getMbr ( N32 fmt, Ticks & val );

        // decimal floating-point Real
        void get ( DecFltPoint < Z64 > & val );
        void get ( DecFltPoint < Z128 > & val );
        void getTyped ( N32 fmt, DecFltPoint < Z64 > & val );
        void getTyped ( N32 fmt, DecFltPoint < Z128 > & val );
        void getMbr ( N32 fmt, DecFltPoint < Z64 > & val );
        void getMbr ( N32 fmt, DecFltPoint < Z128 > & val );

        // Text
        // NB: the region returned is volatile!
        // it should be used before the Deserial goes away
        // and captured as a String for longer term.
        void get ( CText & val );
        void getTyped ( N32 fmt, CText & val );
        void getMbr ( N32 fmt, CText & val );
        
        bool hasMoreData () const noexcept
        { return DB :: hasMoreData (); }

        
        Deserial & operator = ( Deserial && d ) noexcept
        { DB :: operator = ( std :: move ( d ) ); return self; }
        Deserial & operator = ( const Deserial & d ) noexcept
        { DB :: operator = ( d ); return self; }
        Deserial ( Deserial && d ) noexcept
            : DB ( std :: move ( d ) ) {}
        Deserial ( const Deserial & d ) noexcept
            : DB ( d ) {}
        Deserial () noexcept {}

        Deserial & operator = ( const CRgn & rgn ) noexcept
        { DB :: operator = ( rgn ); return self; }
        Deserial ( const CRgn & rgn ) noexcept
            : DeserialBase ( rgn ) {}
    };

    
    /*=====================================================*
     *                   DeserialStream                    *
     *=====================================================*/

    /**
     * DeserialStream
     *  @brief a deserializing front end to a StreamReader
     *
     *  NB - see <vdb3/cmn/serial.hpp> for SerialFmt definition.
     */
    class DeserialStream : public Deserial
    {
    public:

        // 2020-10-02
        // for today, assume that this is being driven as the
        // result of a pollset event, meaning that we've been
        // told a read will not block. rather than reading on
        // demand within "moreData()", read into "b" up front
        // retain the current offset, then allow for use, catching
        // a "TransferIncomplete" exception for rollback, or
        // committing if successful... I think I need to revamp
        // the I/O timeout.

        size_t bytesAvailable () const;
        size_t readAhead ();
        
        void begin ();
        void commit ();
        void rollback ();
        
        CXX_RULE_OF_FOUR_NE ( DeserialStream );
        
        DeserialStream ( const RsrcKfc & rsrc, const StreamReader & r );
        DeserialStream ( const RsrcKfc & rsrc, size_t bsize, const StreamReader & r );
        virtual ~ DeserialStream () noexcept;

    protected:
        
        virtual size_t moreData ( size_t requested );

        StreamReader r;
        Buffer b;
        size_t restore;
    };
}
