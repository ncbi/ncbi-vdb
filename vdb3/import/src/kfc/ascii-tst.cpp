/*

  vdb3.kfc.ascii-tst

 */

#define MAX_RSRC_MGR 0
#include "ascii.cpp"

#include "ktrace.cpp"

#include <google/gtest.h>

namespace vdb3
{
    TEST ( ASCIITextTest, good_pointer_and_size )
    {
        const ASCII7 val [] = "I'm so happy!";
        ASCIIText t1 ( val, sizeof val - 1 );
        EXPECT_EQ ( t1 . constAddr (), val );
        EXPECT_EQ ( t1 . size (), sizeof val - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val - 1 ) );
    }

    TEST ( ASCIITextTest, good_pointer_nul_term )
    {
        const ASCII7 val [] = "I'm so happy!";
        ASCIIText t1 ( val );
        EXPECT_EQ ( t1 . constAddr (), val );
        EXPECT_EQ ( t1 . size (), sizeof val - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val - 1 ) );
    }

    TEST ( ASCIITextTest, good_pointer_assign )
    {
        const ASCII7 val1 [] = "I'm so happy!";
        const ASCII7 val2 [] = "snappy, flappy, whappy.";
        ASCIIText t1 ( val1 );
        EXPECT_EQ ( t1 . constAddr (), val1 );
        EXPECT_EQ ( t1 . size (), sizeof val1 - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val1 - 1 ) );

        t1 = val2;
        EXPECT_EQ ( t1 . constAddr (), val2 );
        EXPECT_EQ ( t1 . size (), sizeof val2 - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val2 - 1 ) );
    }

    TEST ( ASCIITextTest, null_pointer_and_size )
    {
        const ASCII7 val [] = "I'm so happy!";
        ASCIIText t1 ( nullptr, sizeof val - 1 );
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), val );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
    }

    TEST ( ASCIITextTest, null_pointer_nul_term )
    {
        const ASCII7 val [] = "I'm so happy!";
        ASCIIText t1 ( nullptr );
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), val );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
    }

    TEST ( ASCIITextTest, null_pointer_assign )
    {
        const ASCII7 val1 [] = "I'm so happy!";
        ASCIIText t1 ( val1 );
        EXPECT_EQ ( t1 . constAddr (), val1 );
        EXPECT_EQ ( t1 . size (), sizeof val1 - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val1 - 1 ) );

        t1 = ( const ASCII7 * ) nullptr;
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), val1 );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
    }

    TEST ( ASCIITextTest, utf8_pointer_and_size )
    {
        const UTF8 utf8 [] = "Einige Bücher";
        ASCIIText t1 ( utf8, sizeof utf8 - 1 );
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), utf8 );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
    }

    TEST ( ASCIITextTest, utf8_pointer_nul_term )
    {
        const UTF8 utf8 [] = "Einige Bücher";
        ASCIIText t1 ( utf8 );
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), utf8 );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
    }

    TEST ( ASCIITextTest, utf8_pointer_assign )
    {
        const ASCII7 val1 [] = "I'm so happy!";
        const UTF8 utf8 [] = "Einige Bücher";
        ASCIIText t1 ( val1 );
        EXPECT_EQ ( t1 . constAddr (), val1 );
        EXPECT_EQ ( t1 . size (), sizeof val1 - 1 );
        EXPECT_EQ ( t1 . count (), ( count_t ) ( sizeof val1 - 1 ) );

        t1 = utf8;
        EXPECT_NE ( t1 . constAddr (), nullptr );
        EXPECT_NE ( t1 . constAddr (), val1 );
        EXPECT_NE ( t1 . constAddr (), utf8 );
        EXPECT_NE ( t1 . size (), 0U );
        EXPECT_NE ( t1 . count (), 0U );
        EXPECT_EQ ( ( count_t ) t1 . size (), t1 . count () );
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
