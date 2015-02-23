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
* Unit tests for KRepository interfaces
*/

#include <ktst/unit_test.hpp>

#include <cstring>
#include <stdexcept>

#include <klib/vector.h>

#include <kfs/directory.h>
#include <kfs/impl.h>

#include <kfg/config.h>
#include <kfg/repository.h>

using namespace std;

TEST_SUITE(RepositoryTestSuite);

class RepositoryFixture
{
public:
    RepositoryFixture()
    : repo(0)
    {
        if (KConfigMake(&kfg, NULL) != 0)
            throw logic_error("RepositoryFixture: KConfigMake failed");
        if (KConfigMakeRepositoryMgrRead(kfg, (const KRepositoryMgr**)&mgr) != 0)
            throw logic_error("RepositoryFixture: KConfigMakeRepositoryMgrRead failed");
        VectorInit(&repos, 0, 0); // in order to be able to whack it when we are done, even if not used by the test case
        ClearRepositories();
    }
    ~RepositoryFixture() 
    {
        if (repo && KRepositoryRelease(repo) != 0)
            throw logic_error("~RepositoryFixture: KRepositoryRelease failed");
        if (KRepositoryVectorWhack(&repos) != 0)
            throw logic_error("~RepositoryFixture: KRepositoryVectorWhack failed");
        if (KRepositoryMgrRelease(mgr) != 0)
            throw logic_error("~RepositoryFixture: KRepositoryMgrRelease failed");
        if (KConfigRelease(kfg) != 0)
            throw logic_error("~RepositoryFixture: KConfigRelease failed");
    }
    
    void UpdateNode(const char* key, const char* value)
    {
        KConfigNode *node;
        if (KConfigOpenNodeUpdate(kfg, &node, key) != 0)
            throw logic_error("UpdateNode: KConfigOpenNodeUpdate failed");
        if (KConfigNodeWrite(node, value, strlen(value)) != 0)
            throw logic_error("UpdateNode: KConfigNodeWrite failed");
        if (KConfigNodeRelease(node) != 0)
            throw logic_error("UpdateNode: KConfigNodeRelease failed");
    }    
    
    void ClearRepositories()
    {
        KConfigNode *node;
        if (KConfigOpenNodeUpdate(kfg, &node, "repository") != 0)
            throw logic_error("ClearRepositories: KConfigOpenNodeUpdate failed");
        if (KConfigNodeDropAll(node) != 0)
            throw logic_error("ClearRepositories: KConfigNodeDropAll failed");
        if (KConfigNodeRelease(node) != 0)
            throw logic_error("ClearRepositories: KConfigNodeRelease failed");
    }
    
    bool ValidateRepository(const KRepository* r, KRepCategory cat, KRepSubCategory subcat, const char* name)
    {
        if (KRepositoryCategory( r ) != cat)
            return false;
        if (KRepositorySubCategory( r ) != subcat)
            return false;
        if (KRepositoryName( r, buf, BufSize, &num_writ) )
            return false;
        if (string(buf, num_writ) != string(name))
            return false;
        return true;
    }
    
    KConfig* kfg;
    KRepositoryMgr* mgr;
    KRepositoryVector repos;
    const KRepository* repo;
    
    static const int BufSize = 1024;
    char buf[BufSize];
    size_t num_writ;
};

FIXTURE_TEST_CASE(Mgr_MakeRead, RepositoryFixture)
{
    REQUIRE_NOT_NULL(mgr);
}
FIXTURE_TEST_CASE(Mgr_MakeUpdate, RepositoryFixture)
{
    REQUIRE_RC(KRepositoryMgrRelease(mgr)); // remove the default (read-only) mgr
    mgr = 0;
    REQUIRE_RC(KConfigMakeRepositoryMgrUpdate(kfg, &mgr));
    REQUIRE_NOT_NULL(mgr);
}

// UserRepositories
FIXTURE_TEST_CASE(Mgr_UserRepositories_NoNode, RepositoryFixture)
{
    REQUIRE_RC_FAIL(KRepositoryMgrUserRepositories(mgr, &repos));
}
FIXTURE_TEST_CASE(Mgr_UserRepositoriesEmpty, RepositoryFixture)
{
    KRepositoryVector v; // using a local vector to make sure it is initialized by the call
    UpdateNode("/repository/user", "");
    
    REQUIRE_RC(KRepositoryMgrUserRepositories(mgr, &v));
    REQUIRE_EQ(VectorLength(&v), (uint32_t)0);

    REQUIRE_RC(KRepositoryVectorWhack(&v));
}

FIXTURE_TEST_CASE(Mgr_UserRepositories, RepositoryFixture)
{
    // unsorted
    UpdateNode("/repository/user/main/repo4/", "");
    UpdateNode("/repository/user/bad/repo7/", ""); // should be ignored
    UpdateNode("/repository/user/aux/repo1/", "");
    UpdateNode("/repository/user/protected/repo3/", "");
    UpdateNode("/repository/user/aux/repo6/", "");
    UpdateNode("/repository/user/protected/repo5/", "");
    UpdateNode("/repository/user/main/repo1/", "");

    REQUIRE_RC(KRepositoryMgrUserRepositories(mgr, &repos));
    REQUIRE_EQ(VectorLength(&repos), (uint32_t)6);
    
    // verify the values and that the vector has been sorted on: subcategory(main<aux<protected), name
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 0 ), krepUserCategory, (uint32_t)krepMainSubCategory, "repo1"));
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 5 ), krepUserCategory, (uint32_t)krepProtectedSubCategory, "repo5"));
}

FIXTURE_TEST_CASE(Mgr_SiteRepositories, RepositoryFixture)
{
    // unsorted
    UpdateNode("/repository/site/main/repo4/", "");
    UpdateNode("/repository/site/bad/repo7/", ""); // should be ignored
    UpdateNode("/repository/site/aux/repo1/", "");
    UpdateNode("/repository/site/protected/repo3/", "");
    UpdateNode("/repository/site/aux/repo6/", "");
    UpdateNode("/repository/site/protected/repo5/", "");
    UpdateNode("/repository/site/main/repo1/", "");

    REQUIRE_RC(KRepositoryMgrSiteRepositories(mgr, &repos));
    REQUIRE_EQ(VectorLength(&repos), (uint32_t)6);
    
    // verify the values and that the vector has been sorted on: subcategory(main<aux<protected), name
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 0 ), krepSiteCategory, (uint32_t)krepMainSubCategory, "repo1"));
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 5 ), krepSiteCategory, (uint32_t)krepProtectedSubCategory, "repo5"));
}

FIXTURE_TEST_CASE(Mgr_RemoteRepositories, RepositoryFixture)
{
    // unsorted
    UpdateNode("/repository/remote/main/repo4/", "");
    UpdateNode("/repository/remote/bad/repo7/", ""); // should be ignored
    UpdateNode("/repository/remote/aux/repo1/", "");
    UpdateNode("/repository/remote/protected/repo3/", "");
    UpdateNode("/repository/remote/aux/repo6/", "");
    UpdateNode("/repository/remote/protected/repo5/", "");
    UpdateNode("/repository/remote/main/repo1/", "");

    REQUIRE_RC(KRepositoryMgrRemoteRepositories(mgr, &repos));
    REQUIRE_EQ(VectorLength(&repos), (uint32_t)6);
    
    // verify the values and that the vector has been sorted on: subcategory(main<aux<protected), name
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 0 ), krepRemoteCategory, (uint32_t)krepMainSubCategory, "repo1"));
    REQUIRE(ValidateRepository(( const KRepository* ) VectorGet ( & repos, 5 ), krepRemoteCategory, (uint32_t)krepProtectedSubCategory, "repo5"));
}

FIXTURE_TEST_CASE(Mgr_GetProtectedRepository_OldSpelling, RepositoryFixture)
{
    // only repositories from /repository/user/protected/ are looked at
    UpdateNode("/repository/user/protected/dbGap-123/", "");
    UpdateNode("/repository/user/main/dbGap-321/", ""); 
    UpdateNode("/repository/site/protected/dbGap-321/", "");
    UpdateNode("/repository/user/protected/dbGap-321/", "");
    
    REQUIRE_RC(KRepositoryMgrGetProtectedRepository(mgr, 321, &repo));
    REQUIRE(ValidateRepository(repo, krepUserCategory, (uint32_t)krepProtectedSubCategory, "dbGap-321"));
}
FIXTURE_TEST_CASE(Mgr_GetProtectedRepository, RepositoryFixture)
{
    // only repositories from /repository/user/protected/ are looked at
    UpdateNode("/repository/user/protected/dbGaP-123/", "");
    UpdateNode("/repository/user/main/dbGaP-321/", ""); 
    UpdateNode("/repository/site/protected/dbGaP-321/", "");
    UpdateNode("/repository/user/protected/dbGaP-321/", "");
    
    REQUIRE_RC(KRepositoryMgrGetProtectedRepository(mgr, 321, &repo));
    REQUIRE(ValidateRepository(repo, krepUserCategory, (uint32_t)krepProtectedSubCategory, "dbGaP-321"));
}

//TODO: KRepositoryMgrCurrentProtectedRepository

// remote repository access
FIXTURE_TEST_CASE(Mgr_RemoteOnSingle, RepositoryFixture)
{
    UpdateNode ( "/repository/remote/main/repo4/", "" );
    REQUIRE ( KRepositoryMgrHasRemoteAccess ( mgr ) );
}
FIXTURE_TEST_CASE(Mgr_RemoteOnMultiple, RepositoryFixture)
{
    UpdateNode ( "/repository/remote/main/repo4/", "" );
    UpdateNode ( "/repository/remote/aux/repo1/disabled", "true" );
    REQUIRE ( KRepositoryMgrHasRemoteAccess ( mgr ) );
}
FIXTURE_TEST_CASE(Mgr_RemoteOff, RepositoryFixture)
{
    UpdateNode ( "/repository/remote/main/repo4/disabled", "true" );
    UpdateNode ( "/repository/remote/aux/repo1/disabled", "true" );

    REQUIRE ( ! KRepositoryMgrHasRemoteAccess ( mgr ) );
}


//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-kfg";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=RepositoryTestSuite(argc, argv);
    return rc;
}

}
