/*

  vdb3.kfc.pmmgr-tst

 */

#include "pmmgr.cpp"
#include "pmem.cpp"
#include "pmem-rgn-lock.cpp"

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
#include "mem.cpp"
#include "mmgr.cpp"
#include "busy.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    
    TEST ( PMemMgrTest, constructor_destructor )
    {
        MemMgrRef ref = new PMemMgrImpl ();
        MemMgr mmgr ( ref );
    }
    
    TEST ( PMemMgrTest, release_reference )
    {
        MemMgr mmgr = MemMgrRef ( new PMemMgrImpl () );
    }

    class PMemMgrTestFixture : public :: testing :: Test
    {
    public:

        METH ( void SetUp () )
        {
        }

        METH ( void TearDown () )
        {
        }

        PMemMgrTestFixture ()
            : mmgr ( MemMgrRef ( new PMemMgrImpl () ) )
        {
        }

    protected:

        MemMgr mmgr;
    };

    TEST_F ( PMemMgrTestFixture, constructor_destructor )
    {
    }

    TEST_F ( PMemMgrTestFixture, alloc_mem )
    {
        Mem mem = mmgr . alloc ( 123 );
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
