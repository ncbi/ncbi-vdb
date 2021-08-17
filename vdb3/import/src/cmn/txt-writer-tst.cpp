/*

  vdb3.cmn.txt-writer-tst

 */

#include "txt-writer.cpp"

#include "writer.cpp"
#include "text-utf8.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    struct FakeStreamWriter : Refcount, StreamWriterItf
    {
        METH ( CRgn write ( const CRgn & rgn ) )
        {
            return rgn . subRgn ( 0 );
        }
        
        METH ( void flush () )
        {
        }

    };

    struct FakeTxtStreamWriter : Refcount, TextStreamWriterItf
    {
        METH ( CText write ( const CText & rgn ) )
        {
            return rgn . subRgn ( 0 );
        }
        
        METH ( void flush () )
        {
        }
    };
    
    TEST ( TxtStreamWriterTest, constructor_destructor )
    {
        TextStreamWriterRef ref = new FakeTxtStreamWriter ();
        TextStreamWriter w ( ref );
    }
    
    TEST ( TxtStreamWriterTest, constructor_destructor2 )
    {
        StreamWriterRef ref = new FakeStreamWriter ();
        TextStreamWriter w ( ref );
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
