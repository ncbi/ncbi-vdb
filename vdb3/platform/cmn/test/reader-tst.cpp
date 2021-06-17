/*

  vdb3.cmn.reader-tst

 */

#include "reader.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    struct FakeStreamReader : Refcount, StreamReaderItf
    {
        METH ( CRgn read ( const MRgn & rgn ) )
        {
            return rgn . subRgn ( -1 );
        }
    };
    
    TEST ( StreamReaderTest, constructor_destructor )
    {
        StreamReaderRef ref = new FakeStreamReader ();
        StreamReader w ( ref );
    }

    struct FakePosReader : Refcount, PosReaderItf
    {
        METH ( bytes_t size () const )
        {
            return sz;
        }
        
        METH ( CRgn read ( const MRgn & rgn, bytes_t pos ) )
        {
            return rgn . subRgn ( -1 );
        }

        FakePosReader ( bytes_t _sz )
            : sz ( _sz )
        {
        }

        bytes_t sz;
    };
    
    TEST ( PosReaderTest, constructor_destructor )
    {
        PosReaderRef ref = new FakePosReader ( 10 );
        PosReader w ( ref );
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
