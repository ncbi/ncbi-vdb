/*

  vdb3.cmn.dserial-tst

 */

#include "dserial.cpp"
#include "dserial-stream.cpp"

#include "buffer.cpp"
#include "text-utf8.cpp"
#include "reader.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{

    class DeserialTestFixture : public :: testing :: Test, public RgnFactory
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        DeserialTestFixture ()
        {
        }

    protected:

        RsrcMem rsrc;

    };

    TEST_F ( DeserialTestFixture, constructor_destructor )
    {
        Deserial d;
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
