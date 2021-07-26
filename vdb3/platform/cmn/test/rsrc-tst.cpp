/*

  vdb3.cmn.rsrc-tst

 */

#include "rsrc.cpp"

#include "rsrc-log.cpp"
#include "rsrc-trace.cpp"
#include "plogger.cpp"
#include "logger.cpp"
#include "log.cpp"
#include "log-impl.cpp"
#include "trace-impl.cpp"
#include "trace.cpp"
#include "tracer.cpp"
#include "ptracer.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "text-utf8.cpp"
#include "dserial.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    TEST ( RsrcTest, constructor_destructor )
    {
        Rsrc rsrc;
    }
    TEST ( RsrcTest, extra_instance )
    {
        Rsrc rsrc1;
        EXPECT_THROW( { Rsrc rsrc2; }, vdb3 :: Exception );
    }
    TEST ( RsrcTest, copy_constructor )
    {
        Rsrc rsrc1;
        Rsrc rsrc2 ( rsrc1 );
    }
    TEST ( RsrcTest, move_constructor )
    {
        Rsrc rsrc1;
        Rsrc rsrc2 ( std::move(rsrc1) );
    }
    TEST ( RsrcTest, copy_assign_self )
    {
        Rsrc rsrc1;
        rsrc1 = rsrc1;
    }
    TEST ( RsrcTest, copy_assign )
    {
        Rsrc rsrc1;
        Rsrc rsrc2 ( rsrc1 );
        rsrc1 = rsrc2;
    }
    TEST ( RsrcTest, move_assign_self )
    {
        Rsrc rsrc1;
        rsrc1 = std::move( rsrc1 );
    }
    TEST ( RsrcTest, move_assign )
    {
        Rsrc rsrc1;
        Rsrc rsrc2 ( rsrc1 );
        rsrc1 = std::move( rsrc2 );
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
