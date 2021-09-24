/*

  vdb3.cmn.obj-tst

 */

#include <vdb3/kfc/ktrace.hpp>

#include "obj.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    struct FakeObjReader : Refcount, PosReaderItf
    {
        METH ( bytes_t size () const )
        {
            return rgn . size ();
        }
        
        METH ( CRgn read ( const MRgn & dst, bytes_t pos ) )
        {
            // this creates the readable src region
            CRgn src = rgn . subRgn ( pos );

            // this actually transfers the bytes
            return dst . copy ( src );
        }

        FakeObjReader ( const ObjRef & _obj, const CRgn & _rgn )
            : obj ( _obj )
            , rgn ( _rgn )
        {
        }

        ~ FakeObjReader ()
        {
        }

        ObjRef obj;
        CRgn rgn;
    };
    
    struct FakeObjWriter : Refcount, PosWriterItf
    {
        METH ( bytes_t size () const )
        {
            return rgn . size ();
        }
        
        METH ( CRgn write ( const CRgn & src, bytes_t pos ) )
        {
            // this creates the writable dst region
            MRgn dst = rgn . subRgn ( pos );

            // this actually transfers the bytes
            return dst . copy ( src );
        }
        
        METH ( void flush () )
        {
        }

        FakeObjWriter ( const MObjRef & _obj, const MRgn & _rgn )
            : obj ( _obj )
            , rgn ( _rgn )
        {
        }

        ~ FakeObjWriter ()
        {
        }

        MObjRef obj;
        MRgn rgn;
    };

    struct FakeCRgnLock : Refcount, CRgnLockItf
    {
        METH ( CRgn getCRgn () const noexcept )
        {
            return crgn;
        }
        
        FakeCRgnLock ( const ObjRef & _obj, const CRgn & _rgn )
            : obj ( _obj )
            , crgn ( _rgn )
        {
        }

        ~ FakeCRgnLock ()
        {
        }

        ObjRef obj;
        CRgn crgn;
    };

    struct FakeMRgnLock : Refcount, MRgnLockItf
    {
        METH ( MRgn getMRgn () noexcept )
        {
            return mrgn;
        }
        
        FakeMRgnLock ( const MObjRef & _obj, const MRgn & _rgn )
            : obj ( _obj )
            , mrgn ( _rgn )
        {
        }

        ~ FakeMRgnLock ()
        {
        }

        MObjRef obj;
        MRgn mrgn;
    };
    
    struct FakeObj
        : Refcount
        , ObjItf
        , MObjItf
        , RgnFactory
    {
        METH ( bytes_t size () const )
        {
            return sizeof buffer;
        }
        
        METH ( bool isReadable () const noexcept )
        {
            return true;
        }
        
        METH ( PosReaderRef getReader () )
        {
            return new FakeObjReader ( this, makeCRgn ( buffer, sizeof buffer ) );
        }
        
        METH ( CRgnLockRef mapCRgn ( bytes_t offset ) )
        {
            CRgn rgn = makeCRgn ( buffer, sizeof buffer );
            return new FakeCRgnLock ( this, rgn . subRgn ( offset ) );
        }
        
        METH ( CRgnLockRef mapCRgn ( bytes_t offset, size_t bytes ) )
        {
            CRgn rgn = makeCRgn ( buffer, sizeof buffer );
            return new FakeCRgnLock ( this, rgn . subRgn ( offset, bytes ) );
        }

        METH ( ObjRef toObj () const noexcept )
        {
            return this;
        }
        
        METH ( bool isResizable () const noexcept )
        {
            return false;
        }
        
        METH ( void resize ( bytes_t new_size ) )
        {
            throw ConstraintViolation ( XP ( XLOC ) << "object is not resizeable" );
        }
        
        METH ( bool isWritable () const noexcept )
        {
            return true;
        }
        
        METH ( PosWriterRef getWriter () )
        {
            return new FakeObjWriter ( this, makeMRgn ( buffer, sizeof buffer ) );
        }
        
        METH ( MRgnLockRef mapMRgn ( bytes_t offset ) )
        {
            MRgn rgn = makeMRgn ( buffer, sizeof buffer );
            return new FakeMRgnLock ( this, rgn . subRgn ( offset ) );
        }
        
        METH ( MRgnLockRef mapMRgn ( bytes_t offset, size_t bytes ) )
        {
            MRgn rgn = makeMRgn ( buffer, sizeof buffer );
            return new FakeMRgnLock ( this, rgn . subRgn ( offset, bytes ) );
        }

        FakeObj () noexcept
        {
        }

        byte_t buffer [ 4096 ];
    };
    
    TEST ( ObjTest, constructor_destructor )
    {
        ObjRef cref = new FakeObj ();
        Obj cobj ( cref );
        MObjRef mref = new FakeObj ();
        MObj mobj ( mref );
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
