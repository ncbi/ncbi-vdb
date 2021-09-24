/*

  vdb3.kfc.rgn-tst

 */

#define BRUTE_FORCE_SIZE_LIMIT 8

#include "rgn.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"
#include "xbt.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
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
    
    TEST ( RgnTest, constructor_destructor )
    {
        MRgn mrgn;
        CRgn crgn ( mrgn );
    }
    
    TEST ( RgnTest, default_rgn_is_null_and_empty )
    {
        MRgn mrgn;
        CRgn crgn;

        EXPECT_EQ ( mrgn . addr (), nullptr );
        EXPECT_EQ ( crgn . addr (), nullptr );

        EXPECT_EQ ( mrgn . size (), 0U );
        EXPECT_EQ ( crgn . size (), 0U );
    }

    TEST ( RgnTest, find_small_sub_region )
    {
        CRgn haystack ( RgnMaker :: gimmeCRgn ( "This is a small haystack" ) );
        CRgn needle ( RgnMaker :: gimmeCRgn ( "small" ) );
        size_t loc = haystack . fwdFind ( needle );
        EXPECT_EQ ( loc, 10U );
    }

    TEST ( RgnTest, fwd_ref_find_small_sub_region )
    {
        CRgn haystack ( RgnMaker :: gimmeCRgn ( "This is a small haystack" ) );
        CRgn needle ( RgnMaker :: gimmeCRgn ( "is" ) );
        size_t loc = haystack . fwdFind ( needle );
        EXPECT_EQ ( loc, 2U );
        loc = haystack . revFind ( needle );
        EXPECT_EQ ( loc, 5U );
    }


    TEST ( RgnTest, find_small_sub_region_in_slightly_larger_region )
    {
        CRgn haystack ( RgnMaker :: gimmeCRgn ( "This is a slightly larger but still pretty small haystack" ) );
        CRgn needle ( RgnMaker :: gimmeCRgn ( "haystack" ) );
        size_t loc = haystack . fwdFind ( needle . subRgn ( 0, 7 ) );
        EXPECT_EQ ( loc, 49U );
        loc = haystack . fwdFind ( needle );
        EXPECT_EQ ( loc, 49U );
    }


    TEST ( RgnTest, rev_find_small_sub_region_in_slightly_larger_region )
    {
        CRgn haystack ( RgnMaker :: gimmeCRgn ( "This is a slightly larger but still pretty small haystack" ) );
        CRgn needle ( RgnMaker :: gimmeCRgn ( "is a slightly" ) );
        size_t loc = haystack . revFind ( needle . subRgn ( 0, 7 ) );
        EXPECT_EQ ( loc, 5U );
        loc = haystack . revFind ( needle );
        EXPECT_EQ ( loc, 5U );
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
