/*

  vdb3.kfc.mem-tst

 */

#include "mem.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"
#include "xbt.cpp"
#include "ref.cpp"
#include "rgn.cpp"
#include "rgn-lock.cpp"

#include <gtest/gtest.h>

namespace vdb3
{

    struct FakeCRgnLock : Refcount, CRgnLockItf
    {
        METH ( CRgn getCRgn () const noexcept )
        {
            return crgn;
        }

        FakeCRgnLock ( const MemRef & _mem, const CRgn & _rgn ) noexcept
            : mem ( _mem )
            , crgn ( _rgn )
        {
        }

        ~ FakeCRgnLock () noexcept
        {
        }

        MemRef mem;
        CRgn crgn;
    };

    struct FakeMRgnLock : Refcount, MRgnLockItf
    {
        METH ( MRgn getMRgn () noexcept )
        {
            return mrgn;
        }

        FakeMRgnLock ( const MemRef & _mem, const MRgn & _rgn ) noexcept
            : mem ( _mem )
            , mrgn ( _rgn )
        {
        }

        ~ FakeMRgnLock () noexcept
        {
        }

        MemRef mem;
        MRgn mrgn;
    };

    struct FakeMem
        : Refcount
        , MemItf
        , RgnFactory
    {
        METH ( size_t size () const noexcept )
        {
            return sizeof buffer;
        }

        METH ( bool isWritable () const noexcept )
        {
            return true;
        }

        METH ( bool isResizable () const noexcept )
        {
            return false;
        }

        METH ( bool wipeBeforeDelete () const noexcept )
        {
            return false;
        }

        METH ( void resize ( size_t new_size ) )
        {
            throw ConstraintViolation ( XP ( XLOC ) << "mem region is not resizeable" );
        }

        METH ( MRgnLockRef mapMRgn () )
        {
            MRgn rgn = makeMRgn ( buffer, sizeof buffer );
            return new FakeMRgnLock ( this, rgn );
        }

        METH ( CRgnLockRef mapCRgn () const )
        {
            CRgn rgn = makeCRgn ( buffer, sizeof buffer );
            return new FakeCRgnLock ( this, rgn );
        }

        FakeMem () noexcept
        {
        }

        byte_t buffer [ 4096 ];
    };

    TEST ( MemTest, constructor_destructor )
    {
        MemRef ref = new FakeMem ();
        Mem mem ( ref );
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
