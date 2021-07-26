/*

  vdb3.cmn.ptrace-tst

 */

#include "ptracer.cpp"
#include "trace-impl.cpp"

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

#include <vdb3/test/gtest.hpp>

#include <map>

namespace vdb3
{
    class PTraceTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        PTraceTestFixture ()
            : tracer ( new PTracerImpl )
            , trace ( new TraceMgrImpl )
        {
        }

    protected:

        RsrcMem rsrc;
        Tracer tracer;
        TraceMgr trace;

    };
    
    TEST_F ( PTraceTestFixture, constructor_destructor )
    {
    }
    
    TEST_F ( PTraceTestFixture, set_trace_level_prg )
    {
        EXPECT_NO_THROW ( trace . setThreshold ( TRACE_QA ) );
        EXPECT_EQ ( trace . getThreshold (), TRACE_QA );
    }
    
    TEST_F ( PTraceTestFixture, say_howdy )
    {
        EXPECT_NO_THROW ( trace . setThreshold ( TRACE_QA ) );
        trace . msg ( rsrc, tracer, TRACE_USR, TLOC,
                [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( PTraceTestFixture, say_silent_howdy )
    {
        EXPECT_NO_THROW ( trace . setThreshold ( TRACE_USR ) );
        trace . msg ( rsrc, tracer, TRACE_QA, TLOC,
                [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( PTraceTestFixture, create_queue )
    {
        EXPECT_NO_THROW ( TraceQueueId qid = tracer . makeQueue ( "blark" ) );
    }
    
    TEST_F ( PTraceTestFixture, say_howdy_to_queue )
    {
        EXPECT_NO_THROW ( trace . setThreshold ( 0 ) );
        EXPECT_EQ ( trace . getThreshold (), 0 );

        TraceQueueId qid = tracer . makeQueue ( "blark" );
        trace . msg ( rsrc, tracer, qid, TRACE_USR, TLOC,
                [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "this should never print"
                        << endm
                        ;
                }
            );

        EXPECT_NO_THROW ( trace . setThreshold ( qid, TRACE_QA ) );
        trace . msg ( rsrc, tracer, qid, TRACE_USR, TLOC,
                [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo to the new queue"
                        << endm
                        ;
                }
            );
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
