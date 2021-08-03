/*

  vdb3.cmn.rsrc-trace-tst

 */

#include "rsrc-trace.cpp"

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
#include "buffmt.cpp"
#include "trace.cpp"
#include "tracer.cpp"
#include "ptracer.cpp"
#include "trace-impl.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{

    class RsrcTraceFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        RsrcTraceFixture ()
        {
        }

    protected:

        RsrcTrace rsrc;

    };
    
    TEST ( RsrcTraceFix, constructor_destructor )
    {
        RsrcTrace rsrc;
    }
    
    TEST_F ( RsrcTraceFixture, trace_macro_lambda )
    {
        rsrc . trace . setThreshold ( TRACE_QA );
        EXPECT_EQ ( rsrc . trace . getThreshold (), TRACE_QA );
        TRACE ( rsrc, TRACE_USR, [&] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "changed trace level to "
                        << rsrc . trace . getThreshold ()
                        << " and issued message at "
                        << TRACE_USR
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( RsrcTraceFixture, trace_macro_zstr )
    {
        rsrc . trace . setThreshold ( TRACE_QA );
        EXPECT_EQ ( rsrc . trace . getThreshold (), TRACE_QA );
        TRACE ( rsrc, TRACE_USR, "just tracing away" );
    }
    
    TEST_F ( RsrcTraceFixture, trace_queue_macro_lambda )
    {
        TraceQueueId qid1 = rsrc . tracer . makeQueue ( "blarky" );
        TraceQueueId qid = rsrc . tracer . makeQueue ( "flarky" );
        rsrc . trace . setThreshold ( qid, TRACE_QA );
        EXPECT_EQ ( rsrc . trace . getThreshold ( qid ), TRACE_QA );
        TRACEQ ( rsrc, qid, TRACE_USR, [&] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "changed trace level for queue '"
                        << qid -> getName ()
                        << "' to "
                        << rsrc . trace . getThreshold ()
                        << " and issued message at "
                        << TRACE_USR
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( RsrcTraceFixture, trace_queue_macro_zstr )
    {
        TraceQueueId qid1 = rsrc . tracer . makeQueue ( "blarky" );
        TraceQueueId qid = rsrc . tracer . makeQueue ( "flarky" );
        rsrc . trace . setThreshold ( qid, TRACE_QA );
        EXPECT_EQ ( rsrc . trace . getThreshold ( qid ), TRACE_QA );
        TRACEQ ( rsrc, qid, TRACE_USR, "tracing away again" );
    }
    
    TEST_F ( RsrcTraceFixture, trace_queue_level_no_global )
    {
        EXPECT_NO_THROW ( rsrc . trace . setThreshold ( 0 ) );
        EXPECT_EQ ( rsrc . trace . getThreshold (), 0 );
        
        TraceQueueId qid = rsrc . tracer . makeQueue ( "bing" );
        EXPECT_EQ ( rsrc . trace . getThreshold (), 0 );
        EXPECT_EQ ( rsrc . trace . getThreshold ( qid ), -1 );
        TRACE ( rsrc, TRACE_USR, "should never see this" );
        TRACEQ ( rsrc, qid, TRACE_USR, "should never see this" );

        EXPECT_NO_THROW ( rsrc . trace . setThreshold ( qid, TRACE_QA ) );
        EXPECT_EQ ( rsrc . trace . getThreshold (), 0 );
        EXPECT_EQ ( rsrc . trace . getThreshold ( qid ), TRACE_QA );
        TRACE ( rsrc, TRACE_USR, "should never see this" );
        TRACEQ ( rsrc, qid, TRACE_USR, "this should be visible" );
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
