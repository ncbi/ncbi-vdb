/*

  vdb3.cmn.rsrc-log-tst

 */

#include "rsrc-log.cpp"

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
#include "plogger.cpp"
#include "log-impl.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{

    class RsrcLogFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        RsrcLogFixture ()
        {
        }

    protected:

        RsrcLog rsrc;

    };
    
    TEST ( RsrcLogFix, constructor_destructor )
    {
        RsrcLog rsrc;
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
