/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* Unit tests of functions potentially implemented using asm
*
* For now, only cover operations with jumps inside (to test transition from relative offsets to labels)
*
*/

#include <ktst/unit_test.hpp>

#include <atomic.h>
#include <atomic32.h>
#include <arch-impl.h>

TEST_SUITE(AsmTestSuite);

// 32 bit operations

TEST_CASE(a32_read_and_add)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add ( & v, 3 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}

TEST_CASE(a32_add_and_read)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_add_and_read ( & v, 3 ), 5 );
}

TEST_CASE(a32_read_and_add_lt_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_lt ( & v, 3 , 4 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_lt_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_lt ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_le_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_le ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_le_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_le ( & v, 3 , 1 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_eq_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_eq ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_eq_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_eq ( & v, 3 , 1 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_ne_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_ne ( & v, 3 , 1 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_ne_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_ne ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_ge_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_ge ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_ge_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_ge ( & v, 3 , 3 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_gt_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_gt ( & v, 3 , 1 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_gt_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_gt ( & v, 3 , 2 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_odd_true)
{
    atomic32_t v;
    atomic32_set ( & v, 1 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_odd ( & v, 3 ), 1 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 4 );
}
TEST_CASE(a32_read_and_add_odd_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_odd ( & v, 3 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 2 );
}

TEST_CASE(a32_read_and_add_even_true)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_even ( & v, 3 ), 2 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 5 );
}
TEST_CASE(a32_read_and_add_even_false)
{
    atomic32_t v;
    atomic32_set ( & v, 1 );
    REQUIRE_EQ ( ( int ) atomic32_read_and_add_even ( & v, 3 ), 1 );
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 1 );
}

TEST_CASE(atomic32_inc_and_test_true)
{
    atomic32_t v;
    atomic32_set ( & v, -1 );
    REQUIRE ( atomic32_inc_and_test ( & v ) );
}
TEST_CASE(atomic32_inc_and_test_false)
{
    atomic32_t v;
    atomic32_set ( & v, 1 );
    REQUIRE ( ! atomic32_inc_and_test ( & v ) );
}

TEST_CASE(atomic32_dec_and_test_true)
{
    atomic32_t v;
    atomic32_set ( & v, 1 );
    REQUIRE ( atomic32_dec_and_test ( & v ) );
}
TEST_CASE(atomic32_dec_and_test_false)
{
    atomic32_t v;
    atomic32_set ( & v, 2 );
    REQUIRE ( ! atomic32_dec_and_test ( & v ) );
}

TEST_CASE(atomic32_test_and_set_true)
{
    atomic32_t v;
    atomic32_set ( & v, 0 );
    REQUIRE_EQ ( ( int ) 0, ( int ) atomic32_test_and_set ( & v, 1, 0 ) );  // returns prior value of v
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 1 );
}
TEST_CASE(atomic32_test_and_set_false)
{
    atomic32_t v;
    atomic32_set ( & v, 0 );
    REQUIRE_EQ ( ( int ) 0,  ( int ) atomic32_test_and_set ( & v, 1, 2 ) ); // returns prior value of v
    REQUIRE_EQ ( ( int ) atomic32_read ( & v ), 0 );
}

#ifdef USE_GCC_BUILTIN
#undef USE_GCC_BUILTIN
TEST_CASE(uint16_lsbit_0_asm)
{
    REQUIRE_EQ ( ( int16_t ) -1, uint16_lsbit ( 0 ) );
}
TEST_CASE(uint16_lsbit_not_0_asm)
{
    REQUIRE_EQ ( ( int16_t ) 1, uint16_lsbit ( 0xfffe ) );
}
TEST_CASE(uint32_lsbit_0_asm)
{
    REQUIRE_EQ ( ( int32_t ) -1, uint32_lsbit ( 0 ) );
}
TEST_CASE(uint32_lsbit_not_0_asm)
{
    REQUIRE_EQ ( ( int32_t ) 17, uint32_lsbit ( 0xfffe0000 ) );
}
#define USE_GCC_BUILTIN
TEST_CASE(uint16_lsbit_0_builtin)
{
    REQUIRE_EQ ( ( int16_t ) -1, uint16_lsbit ( 0 ) );
}
TEST_CASE(uint16_lsbit_not_0_builtin)
{
    REQUIRE_EQ ( ( int16_t ) 1, uint16_lsbit ( 0xfffe ) );
}
TEST_CASE(uint32_lsbit_0_builtin)
{
    REQUIRE_EQ ( ( int32_t ) -1, uint32_lsbit ( 0 ) );
}
TEST_CASE(uint32_lsbit_not_0_builtin)
{
    REQUIRE_EQ ( ( int32_t ) 17, uint32_lsbit ( 0xfffe0000 ) );
}
#else
#define USE_GCC_BUILTIN
TEST_CASE(uint16_lsbit_0_asm)
{
    REQUIRE_EQ ( ( int16_t ) -1, uint16_lsbit ( 0 ) );
}
TEST_CASE(uint16_lsbit_not_0_asm)
{
    REQUIRE_EQ ( ( int16_t ) 1, uint16_lsbit ( 0xfffe ) );
}
TEST_CASE(uint32_lsbit_0_asm)
{
    REQUIRE_EQ ( ( int32_t ) -1, uint32_lsbit ( 0 ) );
}
TEST_CASE(uint32_lsbit_not_0_asm)
{
    REQUIRE_EQ ( ( int32_t ) 17, uint32_lsbit ( 0xfffe0000 ) );
}
#undef USE_GCC_BUILTIN
TEST_CASE(uint16_lsbit_0_builtin)
{
    REQUIRE_EQ ( ( int16_t ) -1, uint16_lsbit ( 0 ) );
}
TEST_CASE(uint16_lsbit_not_0_builtin)
{
    REQUIRE_EQ ( ( int16_t ) 1, uint16_lsbit ( 0xfffe ) );
}
TEST_CASE(uint32_lsbit_0_builtin)
{
    REQUIRE_EQ ( ( int32_t ) -1, uint32_lsbit ( 0 ) );
}
TEST_CASE(uint32_lsbit_not_0_builtin)
{
    REQUIRE_EQ ( ( int32_t ) 17, uint32_lsbit ( 0xfffe0000 ) );
}
#endif


#if _ARCH_BITS == 64

// 64 bit operations

#include <atomic64.h>

TEST_CASE(a64_read_and_add)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add ( & v, 3 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}

TEST_CASE(a64_add_and_read)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_add_and_read ( & v, 3 ), ( int64_t ) 5 );
}

TEST_CASE(a64_read_and_add_lt_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_lt ( & v, 3 , 4 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_lt_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_lt ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_le_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_le ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_le_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_le ( & v, 3 , 1 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_eq_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_eq ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_eq_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_eq ( & v, 3 , 1 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_ne_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_ne ( & v, 3 , 1 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_ne_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_ne ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_ge_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_ge ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_ge_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_ge ( & v, 3 , 3 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_gt_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_gt ( & v, 3 , 1 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_gt_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_gt ( & v, 3 , 2 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ),( int64_t )  2 );
}

TEST_CASE(a64_read_and_add_odd_true)
{
    atomic64_t v;
    atomic64_set ( & v, 1 );
    REQUIRE_EQ ( ( int64_t )atomic64_read_and_add_odd ( & v, 3 ), ( int64_t ) 1 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 4 );
}
TEST_CASE(a64_read_and_add_odd_false)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_odd ( & v, 3 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 2 );
}

TEST_CASE(a64_read_and_add_even_true)
{
    atomic64_t v;
    atomic64_set ( & v, 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_even ( & v, 3 ), ( int64_t ) 2 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 5 );
}
TEST_CASE(a64_read_and_add_even_false)
{
    atomic64_t v;
    atomic64_set ( & v, 1 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read_and_add_even ( & v, 3 ), ( int64_t ) 1 );
    REQUIRE_EQ ( ( int64_t ) atomic64_read ( & v ), ( int64_t ) 1 );
}

TEST_CASE(a64_test_and_set_ptr_success)
{
    atomic_ptr_t v = { (void*)0 };
    REQUIRE_EQ ( (void*)0, (void*) atomic_test_and_set_ptr ( & v, (void*)1, (void*)0 ) ); // v.ptr == 0; replace and return the old value
    REQUIRE_EQ ( (void*)1, (void*) v . ptr );
}

TEST_CASE(a64_test_and_set_ptr_failure)
{
    atomic_ptr_t v = { (void*)0 };
    REQUIRE_EQ ( (void*)0, (void*) atomic_test_and_set_ptr ( & v, (void*)1, (void*)2 ) ); // v.ptr != 2, not changed
    REQUIRE_EQ ( (void*)0, (void*) v . ptr ); // did not change
}

#endif

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-asm";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=AsmTestSuite(argc, argv);
    return rc;
}

}


