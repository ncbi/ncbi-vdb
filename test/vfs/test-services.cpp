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
* ==============================================================================
*
* tests of names service
*/

#include <ktst/unit_test.hpp> /* KMain */

#include <vfs/services.h> /* KServiceRelease */

TEST_SUITE ( TestServices );

TEST_CASE ( TestKServiceAddId ) {
    KService * s = NULL;

    REQUIRE_RC_FAIL ( KServiceAddId ( s, "0" ) );

    REQUIRE_RC ( KServiceMake ( & s ) );

    REQUIRE_RC_FAIL ( KServiceAddId ( s, NULL ) );
    REQUIRE_RC_FAIL ( KServiceAddId ( s, "" ) );

    for ( int i = 0; i < 512; ++ i )
        REQUIRE_RC ( KServiceAddId ( s, "0" ) );
    REQUIRE_RC ( KServiceAddId ( s, "0" ) );
    for ( int i = 0; i < 512; ++ i )
        REQUIRE_RC ( KServiceAddId ( s, "0" ) );

    REQUIRE_RC ( KServiceRelease ( s ) );
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        return TestServices ( argc, argv );
    }
}
