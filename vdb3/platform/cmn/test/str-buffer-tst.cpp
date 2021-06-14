/*

  vdb3.cmn.str-buffer-tst

 */

#include <vdb3/kfc/rsrc-mem.hpp>

#include "str-buffer.cpp"

#include "buffer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    RsrcMem rsrc;

    /*=================================================*
     *                StringBufferTest                 *
     *=================================================*/

    TEST ( StringBufferTest, constructor_destructor )
    {
        Buffer buff ( rsrc, 1024 );
        StringBuffer sb ( buff );
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
