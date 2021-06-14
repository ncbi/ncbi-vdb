/*

  vdb3.kfc.mmgr-tst

 */

#include "mmgr.cpp"

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

#include <gtest/gtest.h>

namespace vdb3
{

    struct FakeMemMgr
        : Refcount
        , MemMgrItf
        , RgnFactory
    {
        METH ( MemRef alloc ( size_t bytes, bool clear, bool wipe ) );

        void realloc ( MRgn & rgn, size_t new_size, bool clear, bool wipe );
        void free ( MRgn & rgn, bool wipe );

        FakeMemMgr () noexcept
        {
        }

        ~ FakeMemMgr () noexcept
        {
        }
    };

    struct FakeCRgnLock : Refcount, CRgnLockItf
    {
        METH ( CRgn getCRgn () const noexcept )
        {
            return rgn;
        }

        FakeCRgnLock ( const MemRef & _mem, const CRgn & _rgn ) noexcept
            : mem ( _mem )
            , rgn ( _rgn )
        {
        }

        ~ FakeCRgnLock () noexcept
        {
        }

        MemRef mem;
        CRgn rgn;
    };

    struct FakeMRgnLock : Refcount, MRgnLockItf
    {
        METH ( MRgn getMRgn () noexcept )
        {
            return rgn;
        }

        FakeMRgnLock ( const MemRef & _mem, const MRgn & _rgn ) noexcept
            : mem ( _mem )
            , rgn ( _rgn )
        {
        }

        ~ FakeMRgnLock () noexcept
        {
        }

        MemRef mem;
        MRgn rgn;
    };

    struct FakeMem
        : Refcount
        , MemItf
    {

        METH ( size_t size () const noexcept )
        {
            return rgn . size ();
        }

        METH ( bool isWritable () const noexcept )
        {
            return true;
        }

        METH ( bool isResizable () const noexcept )
        {
            return true;
        }

        METH ( bool wipeBeforeDelete () const noexcept )
        {
            return wipe;
        }

        METH ( void resize ( size_t new_size ) )
        {
            mgr -> realloc ( rgn, new_size, clear, wipe );
        }

        METH ( MRgnLockRef mapMRgn () )
        {
            return new FakeMRgnLock ( this, rgn );
        }

        METH ( CRgnLockRef mapCRgn () const )
        {
            return new FakeCRgnLock ( this, rgn );
        }

        FakeMem ( const IRef < FakeMemMgr > & _mgr, const MRgn & _rgn, bool _clr, bool _wipe ) noexcept
            : mgr ( _mgr )
            , rgn ( _rgn )
            , clear ( _clr )
            , wipe ( _wipe )
        {
        }

        ~ FakeMem () noexcept
        {
            mgr -> free ( rgn, clear );
        }

        IRef < FakeMemMgr > mgr;
        MRgn rgn;
        bool clear;
        bool wipe;
    };

    MemRef FakeMemMgr :: alloc ( size_t bytes, bool clear, bool wipe )
    {
        byte_t * ptr = ( byte_t * ) ( clear ? :: calloc ( 1, bytes ) : :: malloc ( bytes ) );
        if ( ptr == nullptr )
        {
            throw MemoryExhausted (
                XP ( XLOC )
                << "out of memory allocating "
                << bytes
                << " bytes"
                );
        }

        try
        {
            return new FakeMem ( this,  makeMRgn ( ptr, bytes ), clear, wipe );
        }
        catch ( ... )
        {
            :: free ( ( void * ) ptr );
            throw;
        }
    }

    void FakeMemMgr :: realloc ( MRgn & rgn, size_t new_size, bool clear, bool wipe )
    {
        size_t old_size = rgn . size ();
        if ( old_size != new_size )
        {
            byte_t * ptr = ( byte_t * ) :: realloc ( rgn . addr (), new_size );
            if ( ptr == nullptr )
            {
                throw MemoryExhausted (
                    XP ( XLOC )
                    << "out of memory reallocating "
                    << old_size
                    << " to "
                    << new_size
                    << " bytes"
                    );
            }

            if ( clear && new_size > old_size )
                :: memset ( & ptr [ old_size ], 0, new_size - old_size );

            rgn = makeMRgn ( ptr, new_size );
        }
    }

    void FakeMemMgr :: free ( MRgn & rgn, bool wipe )
    {
        byte_t * ptr = rgn . addr ();

        if ( wipe )
            memset_s ( ptr, rgn . size (), 0, rgn . size () );

        rgn . MRgn :: ~ MRgn ();

        :: free ( ( void * ) ptr );
    }

    TEST ( MemMgrTest, constructor_destructor )
    {
        MemMgrRef ref = new FakeMemMgr ();
        MemMgr mgr ( ref );
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
