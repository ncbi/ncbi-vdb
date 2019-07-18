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

#include <klib/time.h> /* KTime */
#include <ktst/unit_test.hpp> // TEST_SUITE

using std::string;

TEST_SUITE ( TestTimeSuite );

TEST_CASE ( test ) {

////////////////////////////////////////////////////////////////////////////////

    TEST_MESSAGE (
       "Checking roundtrip conversion Iso8601 string -> KTime -> Iso8601 string"
    );

    const string date1 ( "2015-04-07T21:54:15Z" );

    KTime kt1;
    const KTime * t
        = KTimeFromIso8601 ( & kt1, date1 . c_str (), date1 . size () );
    REQUIRE ( t );

    KTime_t ts = KTimeMakeTime ( & kt1 );

    char date2 [ 64 ];
    size_t sz = KTimeIso8601 ( ts, date2, sizeof date2 );

    REQUIRE ( sz );
    REQUIRE_EQ ( sz, date1 . size () );
    REQUIRE_EQ ( date2 [ sz ], '\0' );
    REQUIRE_EQ ( string ( date2 ), date1 );

////////////////////////////////////////////////////////////////////////////////

    TEST_MESSAGE ( "Checking roundtrip conversion 'KTime kt1' -> "
         "'KTime_t ts = KTimeMakeTime(kt1)' -> 'KTime kt2 = KTimeGlobal(ts)'" );

    KTime kt2;
    const KTime * ktp =  KTimeGlobal ( & kt2, ts );

    REQUIRE ( ktp );
    REQUIRE ( kt1 . year==kt2 . year && kt1 . month == kt2 . month );
    REQUIRE ( kt1 . day == kt2.day );
    if ( kt1 . weekday )
        REQUIRE_EQ ( kt1 . weekday, kt2 . weekday );
#if !defined(__SunOS)  &&  !defined(__sun__)
    REQUIRE_EQ ( kt1 . tzoff, kt2 . tzoff );
#endif
    REQUIRE_EQ ( kt1 . hour, kt2 . hour );
    REQUIRE ( kt1 . minute == kt2 . minute && kt1 . second == kt2 . second  );
    REQUIRE_EQ ( kt1 . dst , kt2 . dst );

////////////////////////////////////////////////////////////////////////////////

}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] )
    {   return TestTimeSuite ( argc, argv ); }
}
