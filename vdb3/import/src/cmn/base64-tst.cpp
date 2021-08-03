/*

  vdb3.cmn.base64-tst

 */

#include "base64.cpp"

#include "text-utf8.cpp"
#include "buffer.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "str-buffer.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    template < class T > 
    size_t min ( const T a, const T b )
    {
        return ( a > b ) ? b : a;
    }
    
    struct RgnMaker : RgnFactory
    {
        static MRgn gimmeMRgn ( byte_t * a, size_t sz ) noexcept
        { return makeMRgn ( a, sz ); }

        static CRgn gimmeCRgn ( const byte_t * a, size_t sz ) noexcept
        { return makeCRgn ( a, sz ); }

        static CRgn gimmeCRgn ( const UTF8 * a ) noexcept
        { return makeCRgn ( a, :: strlen ( a ) ); }
        static CRgn gimmeCRgn ( const UTF8 * a, size_t sz ) noexcept
        { return makeCRgn ( a, sz ); }
    };
    
    RsrcMem rsrc;

    TEST ( Base64Test, encode_decode_no_padding )
    {
        Base64 b64 ( Base64 :: dont_allow_whitespace, Base64 :: do_trim_padding );
        
        CRgn input = RgnMaker :: gimmeCRgn ( "\04\03\02\01", 5 ); ; // compiler includes last byte of value 0
        String encoded = b64 . encode ( rsrc, input );
        EXPECT_EQ ( encoded . size (), ( input . size () * 4 + 2 ) / 3 );
        EXPECT_EQ ( encoded . toSTLString (), std::string ( "BAMCAQA" ) );

        Buffer output = b64 . decode ( rsrc, encoded );
        EXPECT_EQ ( output . size (), input . size () );
        EXPECT_EQ ( :: memcmp ( output . buffer () . addr ()
                                , input . addr ()
                                , min ( output . size (), input . size () ) )
                    , 0 );
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
