/*

  vdb3.cmn.serial-tst

 */

#include "serial.cpp"

#include "buffer.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{

    class SerialTestFixture : public :: testing :: Test, public RgnFactory
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        SerialTestFixture ()
        {
        }

    protected:

        RsrcMem rsrc;

    };

    TEST_F ( SerialTestFixture, constructor_destructor )
    {
        Serial s ( rsrc );
    }

    TEST_F ( SerialTestFixture, single_byte_natural )
    {
        Serial s ( rsrc );

        const byte_t vals [] = { 0, 1, 2, 3, 4, 5, 6, 127 };
        EXPECT_EQ ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );

        s . put ( 0U );
        EXPECT_NE ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        s . put ( 1U );
        EXPECT_NE ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        s . put ( 2U );
        EXPECT_NE ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 3 ) . compare ( s . content () ), 0 );
        s . put ( 3U );
        EXPECT_EQ ( makeCRgn ( vals, 4 ) . compare ( s . content () ), 0 );
        s . put ( 4U );
        EXPECT_EQ ( makeCRgn ( vals, 5 ) . compare ( s . content () ), 0 );
        s . put ( 5U );
        EXPECT_EQ ( makeCRgn ( vals, 6 ) . compare ( s . content () ), 0 );
        s . put ( 6U );
        EXPECT_EQ ( makeCRgn ( vals, 7 ) . compare ( s . content () ), 0 );
        s . put ( 127U );
        EXPECT_EQ ( makeCRgn ( vals, 8 ) . compare ( s . content () ), 0 );
    }

    TEST_F ( SerialTestFixture, multi_byte_natural )
    {
        Serial s ( rsrc );

        const byte_t vals [] =
            {
                0,                                                               // 0U
                127,                                                             // 127U
                1 | 0x80, 0,                                                     // 128U
                127 | 0x80, 127,                                                 // 16383U
                61 | 0x80, 4 | 0x80, 64,                                         // 1000000U
                3 | 0x80, 92 | 0x80, 107 | 0x80, 20 | 0x80, 0,                   // 1000000000U
                // 59, 154, 202, 0 if fixed
                29 | 0x80, 13 | 0x80, 37 | 0x80, 20 | 0x80, 32 | 0x80, 0,        // 1000000000000ULL
                29 | 0x80, 13 | 0x80, 37 | 0x80, 20 | 0x80, 32 | 0x80, 0 | 0x80,
                0x80, 0x80, 0x80, 0
            };

        // empty
        EXPECT_EQ ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );

        // single byte
        s . put ( 0U );
        EXPECT_EQ ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        s . put ( 127U );

        // two byte
        EXPECT_EQ ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        s . put ( 128U );
        EXPECT_EQ ( makeCRgn ( vals, 4 ) . compare ( s . content () ), 0 );
        s . put ( 16383U );
        EXPECT_EQ ( makeCRgn ( vals, 6 ) . compare ( s . content () ), 0 );

        // three byte
        s . put ( 1000000U );
        EXPECT_EQ ( makeCRgn ( vals, 9 ) . compare ( s . content () ), 0 );

        // five byte ( where variable encoding is wasteful )
        s . put ( 1000000000U );
        EXPECT_EQ ( makeCRgn ( vals, 14 ) . compare ( s . content () ), 0 );

        // six byte exercising N128 within 64-bit range
        N128 expect ( 1000000000U );
        expect *= 1000U;
        s . put ( expect );
        EXPECT_EQ ( makeCRgn ( vals, 20 ) . compare ( s . content () ), 0 );

        // 10 byte only within 128-bit range
        expect <<= 7 * 4;
        s . put ( expect );
        EXPECT_EQ ( makeCRgn ( vals, 30 ) . compare ( s . content () ), 0 );
    }

    TEST_F ( SerialTestFixture, single_byte_positive_integer )
    {
        Serial s ( rsrc );

        // zig-zag encoding puts sign bit into LSB
        const byte_t vals [] = { 0, 2, 4, 6, 8, 10, 12, 126 };
        EXPECT_EQ ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );

        s . put ( 0 );
        EXPECT_NE ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        s . put ( 1 );
        EXPECT_NE ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        s . put ( 2 );
        EXPECT_NE ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 3 ) . compare ( s . content () ), 0 );
        s . put ( 3 );
        EXPECT_EQ ( makeCRgn ( vals, 4 ) . compare ( s . content () ), 0 );
        s . put ( 4 );
        EXPECT_EQ ( makeCRgn ( vals, 5 ) . compare ( s . content () ), 0 );
        s . put ( 5 );
        EXPECT_EQ ( makeCRgn ( vals, 6 ) . compare ( s . content () ), 0 );
        s . put ( 6 );
        EXPECT_EQ ( makeCRgn ( vals, 7 ) . compare ( s . content () ), 0 );
        s . put ( 63 );
        EXPECT_EQ ( makeCRgn ( vals, 8 ) . compare ( s . content () ), 0 );
    }

    TEST_F ( SerialTestFixture, single_byte_negative_integer )
    {
        Serial s ( rsrc );

        // zig-zag encoding puts sign bit into LSB
        const byte_t vals [] = { 0, 1, 3, 5, 7, 9, 11, 125, 127 };
        EXPECT_EQ ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );

        s . put ( 0 );
        EXPECT_NE ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        s . put ( -1 );
        EXPECT_NE ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        s . put ( -2 );
        EXPECT_NE ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        EXPECT_EQ ( makeCRgn ( vals, 3 ) . compare ( s . content () ), 0 );
        s . put ( -3 );
        EXPECT_EQ ( makeCRgn ( vals, 4 ) . compare ( s . content () ), 0 );
        s . put ( -4 );
        EXPECT_EQ ( makeCRgn ( vals, 5 ) . compare ( s . content () ), 0 );
        s . put ( -5 );
        EXPECT_EQ ( makeCRgn ( vals, 6 ) . compare ( s . content () ), 0 );
        s . put ( -6 );
        EXPECT_EQ ( makeCRgn ( vals, 7 ) . compare ( s . content () ), 0 );
        s . put ( -63 );
        EXPECT_EQ ( makeCRgn ( vals, 8 ) . compare ( s . content () ), 0 );
        s . put ( -64 );
        EXPECT_EQ ( makeCRgn ( vals, 9 ) . compare ( s . content () ), 0 );
    }


    static
    void dumpIntegerContent ( const Serial & s, size_t start, size_t count )
    {
#if 0
        N64 n64;
        N128 n128;
        Z64 z64;
        Z128 z128;
#endif
        CRgn rgn = s . content () . subRgn ( start, count );
        const byte_t * a = rgn . addr ();
        const char * sep = "";
        std :: cout << "{ ";
        for ( size_t i = 0; i < count; ++ i )
        {
            if ( ( a [ i ] & 0x80 ) != 0 )
                std :: cout << sep << ( N32 ) ( a [ i ] & 0x7F ) << " | 0x80";
            else
                std :: cout << sep << ( N32 ) a [ i ];
            sep = ", ";
        }
        std :: cout << " }\n";
    }
    
    TEST_F ( SerialTestFixture, multi_byte_integer )
    {
        Serial s ( rsrc );

        const byte_t vals [] =
            {
                0,                                                               // 0
                126,                                                             // 63
                127,                                                             // -64
                1 | 0x80, 0,                                                     // 64
                1 | 0x80, 1,                                                     // -65
                127 | 0x80, 126,                                                 // 8191
                127 | 0x80, 127,                                                 // -8192
                1 | 0x80, 0 | 0x80, 0,                                           // 8192
                1 | 0x80, 0 | 0x80, 1,                                           // -8193
                0
            };

        // empty
        EXPECT_EQ ( makeCRgn ( vals, 0 ) . compare ( s . content () ), 0 );

        // single byte
        s . put ( 0 );
        EXPECT_EQ ( makeCRgn ( vals, 1 ) . compare ( s . content () ), 0 );
        s . put ( 63 );
        EXPECT_EQ ( makeCRgn ( vals, 2 ) . compare ( s . content () ), 0 );
        s . put ( -64 );
        EXPECT_EQ ( makeCRgn ( vals, 3 ) . compare ( s . content () ), 0 );

        // two byte
        s . put ( 64 );
        dumpIntegerContent ( s, 3, 2 );
        EXPECT_EQ ( makeCRgn ( vals, 5 ) . compare ( s . content () ), 0 );
        s . put ( -65 );
        dumpIntegerContent ( s, 5, 2 );
        EXPECT_EQ ( makeCRgn ( vals, 7 ) . compare ( s . content () ), 0 );
        s . put ( 8191 );
        dumpIntegerContent ( s, 7, 2 );
        EXPECT_EQ ( makeCRgn ( vals, 9 ) . compare ( s . content () ), 0 );
        s . put ( -8192 );
        dumpIntegerContent ( s, 9, 2 );
        EXPECT_EQ ( makeCRgn ( vals, 11 ) . compare ( s . content () ), 0 );

        // three byte
        s . put ( 8192 );
        dumpIntegerContent ( s, 11, 3 );
        EXPECT_EQ ( makeCRgn ( vals, 14 ) . compare ( s . content () ), 0 );
        s . put ( -8193 );
        dumpIntegerContent ( s, 14, 3 );
        EXPECT_EQ ( makeCRgn ( vals, 17 ) . compare ( s . content () ), 0 );
    }
    
    TEST_F ( SerialTestFixture, text )
    {
        Serial s ( rsrc );
    }

}

extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        CAPTURE_TOOL ( argv [ 0 ] );
        CAPTURE_EARLY_TRACE_LEVEL ( argc, argv );
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
