/*

  vdb3.cmn.stream-tst

 */

#include <vdb3/kfc/ktrace.hpp>

#include "stream.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    struct FakeStreamReader : Refcount, StreamReaderItf
    {
        METH ( CRgn read ( const MRgn & dst ) )
        {
            return dst . copy ( rgn );
        }

        FakeStreamReader ( const StreamRef & _strm, const CRgn & _rgn )
            : strm ( _strm )
            , rgn ( _rgn )
        {
        }

        ~ FakeStreamReader ()
        {
        }

        StreamRef strm;
        CRgn rgn;
    };
    
    struct FakeStreamWriter : Refcount, StreamWriterItf
    {
        METH ( CRgn write ( const CRgn & src ) )
        {
            return rgn . copy ( src );
        }
        
        METH ( void flush () )
        {
        }

        FakeStreamWriter ( const MStreamRef & _strm, const MRgn & _rgn )
            : strm ( _strm )
            , rgn ( _rgn )
        {
        }

        ~ FakeStreamWriter ()
        {
        }

        MStreamRef strm;
        MRgn rgn;
    };
    
    struct FakeStream
        : Refcount
        , StreamItf
        , MStreamItf
        , RgnFactory
    {
        METH ( bool isReadable () const noexcept )
        {
            return true;
        }
        
        METH ( StreamReaderRef getReader () )
        {
            return new FakeStreamReader ( this, makeCRgn ( buffer, sizeof buffer ) );
        }
        
        METH ( StreamRef toStream () const noexcept )
        {
            return this;
        }
        
        METH ( bool isWritable () const noexcept )
        {
            return true;
        }
        
        METH ( StreamWriterRef getWriter () )
        {
            return new FakeStreamWriter ( this, makeMRgn ( buffer, sizeof buffer ) );
        }

        FakeStream ()
        {
        }

        byte_t buffer [ 4096 ];
    };
    
    TEST ( StreamTest, constructor_destructor )
    {
        StreamRef cref = new FakeStream ();
        Stream cstrm ( cref );
        MStreamRef mref = new FakeStream ();
        MStream mstrm ( mref );
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
