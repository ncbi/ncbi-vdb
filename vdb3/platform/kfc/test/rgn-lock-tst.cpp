/*

  vdb3.kfc.rgn-lock-tst

 */

#include "rgn-lock.cpp"

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

#include <gtest/gtest.h>

namespace vdb3
{
    struct FakeMRgnLock
        : Refcount
        , MRgnLockItf
        , RgnFactory
    {
        METH ( MRgn getMRgn () noexcept )
        {
            return makeMRgn ( buffer, sizeof buffer );
        }

        FakeMRgnLock ( bool & _locked )
            : locked ( _locked )
        {
            locked = true;
        }

        ~ FakeMRgnLock ()
        {
            locked = false;
        }

        byte_t buffer [ 4096 ];
        bool & locked;
    };

    TEST ( MRgnLockerTest, constructor_destructor )
    {
        bool locked = false;
        EXPECT_EQ ( locked, false );
        MRgnLockRef lock = new FakeMRgnLock ( locked );
        EXPECT_EQ ( locked, true );
        MRgnLocker locker ( lock );
        EXPECT_EQ ( locked, true );
        MRgn mrgn = locker . rgn ();
        CRgn crgn = mrgn;
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
