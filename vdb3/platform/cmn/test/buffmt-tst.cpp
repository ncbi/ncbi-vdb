/*

  vdb3.cmn.buffmt-tst

 */

#include "buffmt.cpp"

#include "buffer.cpp"
#include "writer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "nconv.cpp"
#include "txt-writer.cpp"
#include "fmt.cpp"
#include "str-buffer.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    template < class T >
    size_t min ( const T a, const T b )
    {
        return ( a < b ) ? a : b;
    }

    struct BufferWriter
        : public Refcount
        , StreamWriterItf
    {
        METH ( CRgn write ( const CRgn & rgn ) )
        {
            // make sure the buffer can handle this stuff
            b . ensureAdditionalCapacity ( rgn . size () );

            // copy the source to the buffer after current content
            MRgn writ = b . buffer () . subRgn ( b . size () ) . copy ( rgn );

            // update the size of valid content
            b . setSize ( b . size () + writ . size () );

            // return any unwritten region - should be none
            assert ( writ . size () == rgn . size () );
            return writ;
        }

        METH ( void flush () )
        {
        }

        StreamWriterRef toStreamWriter ()
        { return StreamWriterRef ( this ); }

        BufferWriter ( const RsrcMem & rsrc )
            : b ( rsrc )
        {
        }

        ~ BufferWriter () {}

        Buffer b;
    };

    class BufferedFmtTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
            bw -> b . setSize ( 0 );
        }

        virtual void TearDown () override
        {
        }

        BufferedFmtTestFixture ()
            : bw ( new BufferWriter ( rsrc ) )
            , tw ( bw -> toStreamWriter () )
        {
        }

    protected:

        RsrcKfc rsrc;
        IRef < BufferWriter > bw;
        TextStreamWriter tw;

    };

    TEST_F ( BufferedFmtTestFixture, constructor_destructor )
    {
        BufferedFmt fmt ( rsrc, tw );
    }

    TEST_F ( BufferedFmtTestFixture, write_some_things )
    {
        BufferedFmt fmt ( rsrc, tw );
        EXPECT_NO_THROW ( fmt
            << "what will we, at "
            << 9
            << 'A'
            << 'M'
            << ", think of next?"
            << endm
            )
            ;
    }

    TEST_F ( BufferedFmtTestFixture, write_some_things_and_check )
    {
        BufferedFmt fmt ( rsrc, tw );
        EXPECT_NO_THROW ( fmt
            << "what will we, at "
            << 9
            << 'A'
            );

        EXPECT_EQ ( bw -> b . size (), 0U );

        EXPECT_NO_THROW ( fmt
            << 'M'
            << ", think of next?"
            << endm
            )
            ;

        const char exp [] = "what will we, at 9AM, think of next?";
        EXPECT_EQ ( bw -> b . size (), sizeof exp - 1 );
        EXPECT_EQ ( memcmp ( bw -> b . content () . addr (), exp, min ( bw -> b . size (), sizeof exp - 1 ) ), 0 );
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
