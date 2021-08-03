/*

  vdb3.cmn.fmt-tst

 */

#include "fmt.cpp"

#include "buffer.cpp"
#include "writer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "nconv.cpp"
#include "txt-writer.cpp"

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

    class FmtTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
            bw -> b . setSize ( 0 );
        }

        virtual void TearDown () override
        {
        }

        FmtTestFixture ()
            : bw ( new BufferWriter ( rsrc ) )
            , tw ( bw -> toStreamWriter () )
        {
        }

    protected:

        RsrcKfc rsrc;
        IRef < BufferWriter > bw;
        TextStreamWriter tw;

    };
    
    TEST_F ( FmtTestFixture, constructor_destructor )
    {
        Fmt fmt ( tw );
    }
    
    TEST_F ( FmtTestFixture, write_some_things )
    {
        Fmt fmt ( tw );
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
    
    TEST_F ( FmtTestFixture, write_some_things_and_check )
    {
        Fmt fmt ( tw );
        EXPECT_NO_THROW ( fmt
            << "what will we, at "
            << 9
            << 'A'
            << 'M'
            << ", think of next?"
            << endm
            )
            ;
        
        const char exp [] = "what will we, at 9AM, think of next?";
        EXPECT_EQ ( bw -> b . size (), sizeof exp - 1 );
        EXPECT_EQ ( memcmp ( bw -> b . content () . addr (), exp, min ( bw -> b . size (), sizeof exp - 1 ) ), 0 );
    }
    
    TEST_F ( FmtTestFixture, indent_and_write_something )
    {
        Fmt fmt ( tw );

        EXPECT_NO_THROW ( fmt
            << "okay\n"
            << ind
            << "indented\n"
            << outd
            << "done"
            << endm
            )
            ;
    }
    
    TEST_F ( FmtTestFixture, indent_and_write_empty_line )
    {
        Fmt fmt ( tw );

        EXPECT_NO_THROW ( fmt
            << "okay\n"
            << ind
            << '\n'
            << outd
            << "done"
            << endm
            )
            ;
    }
    
    TEST_F ( FmtTestFixture, write_time_values )
    {
        Fmt fmt ( tw );

        Ticks ticks ( 10 );
        NanoSeconds nS ( 20 );
        MicroSeconds uS ( 30 );
        MilliSeconds mS ( 40 );
        Seconds secs ( 50 );
        Minutes mins ( 60 );
        Hours hrs ( 70 );
        Days days ( 80 );

        fmt
            << "ticks: " << ticks << '\n'
            << "nS: " << nS << '\n'
            << "uS: " << uS << '\n'
            << "mS: " << mS << '\n'
            << "secs: " << secs << '\n'
            << "mins: " << mins << '\n'
            << "hrs: " << hrs << '\n'
            << "days: " << days << '\n'
            ;
        //fwrite ( bw -> b . content () . addr (), 1, bw -> b . content () . size (), stderr );
    }
    
    TEST_F ( FmtTestFixture, write_timestamp )
    {
        Fmt fmt ( tw );

        Timestamp ts = rsrc . time . now ();

        fmt
            << "local time: "
            << rsrc . time . local ( ts )
            << '\n'
            << "UTC: "
            << rsrc . time . utc ( ts )
            << '\n'
            ;
        //fwrite ( bw -> b . content () . addr (), 1, bw -> b . content () . size (), stderr );
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
