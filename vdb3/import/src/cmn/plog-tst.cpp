/*

  vdb3.cmn.plog-tst

 */

#include "plogger.cpp"
#include "log-impl.cpp"

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
#include "rsrc-trace.cpp"
#include "log.cpp"
#include "logger.cpp"

#include <vdb3/test/gtest.hpp>

#include <map>

#include <time.h>

namespace vdb3
{
    class PLogTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        PLogTestFixture ()
            : logger ( new PLoggerImpl ( "fake-host", "fake-proc" ) )
            , log ( new LogMgrImpl )
        {
        }

    protected:

        RsrcTrace rsrc;
        Logger logger;
        LogMgr log;

    };
    
    TEST_F ( PLogTestFixture, constructor_destructor )
    {
    }
    
    TEST_F ( PLogTestFixture, set_log_level_debug )
    {
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        EXPECT_EQ ( log . getThreshold (), LOG_DEBUG );
    }
    
    TEST_F ( PLogTestFixture, say_howdy )
    {
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, LOG_INFO, [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( PLogTestFixture, say_silent_howdy )
    {
        EXPECT_NO_THROW ( log . setThreshold ( LOG_ERR ) );
        log . msg ( rsrc, logger, LOG_INFO, [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( PLogTestFixture, create_queue )
    {
        EXPECT_NO_THROW ( LogQueueId qid = logger . makeQueue ( "blark" ) );
    }
    
    TEST_F ( PLogTestFixture, say_howdy_to_queue )
    {
        LogQueueId qid = logger . makeQueue ( "blark" );
        EXPECT_NO_THROW ( log . setThreshold ( qid, LOG_DEBUG ) );
        log . msg ( rsrc, logger, qid, LOG_INFO, [] ( Fmt & fmt )
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
