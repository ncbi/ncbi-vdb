/*

  vdb3.cmn.writer-tst

 */

#include "writer.cpp"

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
    
    TEST ( StreamWriterTest, constructor_destructor )
    {
        StreamWriterRef ref = new FakeStreamWriter ();
        StreamWriter w ( ref );
    }

    struct FakePosWriter : Refcount, PosWriterItf
    {
        METH ( bytes_t size () const )
        {
            return sz;
        }
        
        METH ( CRgn write ( const CRgn & rgn, bytes_t pos ) )
        {
            return rgn . subRgn ( 0 );
        }
        
        METH ( void flush () )
        {
        }

        FakePosWriter ( bytes_t _sz )
            : sz ( _sz )
        {
        }

        bytes_t sz;
    };
    
    TEST ( PosWriterTest, constructor_destructor )
    {
        PosWriterRef ref = new FakePosWriter ( 10 );
        PosWriter w ( ref );
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
