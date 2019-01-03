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

#include <kns/manager.h> // KNSManagerRelease
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vfs/manager.h> // VFSManagerMake

#include "../../libs/kns/kns_manager-singleton.h" // USE_SINGLETON

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

TEST_SUITE(KNSManagerSingletonTestSuite);

TEST_CASE(VDB_2877) {
    rc_t rc = 0;

    KNSManager *mgr = NULL;
    KNSManager *mgr2 = NULL;

    REQUIRE_EQ(mgr, mgr2); 

    REQUIRE_RC(KNSManagerMake(&mgr)); 
    REQUIRE(mgr); 
    REQUIRE_NE(mgr, mgr2); 

    VFSManager *vmgr = NULL;
    REQUIRE_RC(VFSManagerMake(&vmgr));
    REQUIRE(vmgr); 

    REQUIRE_RC(VFSManagerGetKNSMgr(vmgr, &mgr2));

#if USE_SINGLETON
    REQUIRE_EQ(mgr, mgr2);
#else
    REQUIRE_NE(mgr, mgr2);
#endif

    RELEASE(KNSManager, mgr2);

    RELEASE(VFSManager, vmgr);

    RELEASE(KNSManager, mgr);
    REQUIRE_EQ(mgr, mgr2); 

    REQUIRE(!rc);
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char *argv [] )
    {    return KNSManagerSingletonTestSuite(argc, argv); }
}
