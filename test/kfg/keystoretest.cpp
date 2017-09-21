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
* Unit tests for KKeyStore interfaces
*/

#include <ktst/unit_test.hpp>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/lockfile.h>

#include <kfg/keystore.h>
#include <kfg/keystore-priv.h>
#include <kfg/config.h>
#include <kfg/kfg-priv.h>

#include <klib/text.h>

#include <kproc/thread.h>

#include <fstream>
#include <stdexcept>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(KeyStoreTestSuite);

TEST_CASE(KeyStoreMake)
{
    KKeyStore* ks = 0;
    REQUIRE_RC(KKeyStoreMake(&ks, NULL));
    REQUIRE_NOT_NULL(ks);
    REQUIRE_RC(KKeyStoreRelease(ks));
}

class KeyStoreFixture
{
public:
    KeyStoreFixture()
    : key(0)
    {
        if (KConfigMake(&kfg, NULL) != 0)
           throw logic_error("KeyStoreFixture: KConfigMake failed");
        if (KKeyStoreMake(&ks, kfg) != 0)
           throw logic_error("KeyStoreFixture: KKeyStoreMake failed");
        if (KDirectoryNativeDir(&wd) != 0)
           throw logic_error("KeyStoreFixture: KDirectoryNativeDir failed");
    }
    ~KeyStoreFixture()
    {
        if (KEncryptionKeyRelease(key) != 0)
           throw logic_error("~KeyStoreFixture: KEncryptionKeyRelease failed");
        if (KDirectoryRelease(wd) != 0)
           throw logic_error("~KeyStoreFixture: KDirectoryRelease failed");
        if (KKeyStoreRelease(ks) != 0)
           throw logic_error("~KeyStoreFixture: KKeyStoreRelease failed");
        if (KConfigRelease(kfg) != 0)
           throw logic_error("~KeyStoreFixture: KConfigRelease failed");
    }
    void KfgUpdateNode(const char* key, const char* value)
    {
        KConfigNode *node;
        if (KConfigOpenNodeUpdate(kfg, &node, key) != 0)
            throw logic_error("KfgUpdateNode: KConfigOpenNodeUpdate failed");
        if (KConfigNodeWrite(node, value, string_measure(value, NULL)) != 0)
            throw logic_error("KfgUpdateNode: KConfigNodeWrite failed");
        if (KConfigNodeRelease(node) != 0)
            throw logic_error("KfgUpdateNode: KConfigNodeRelease failed");
    }    

    KConfig* kfg;
    KKeyStore* ks;
    KDirectory* wd;
    KEncryptionKey* key;
};

// NB: strangely enough, KKeyStoreGetKey ignores its 2nd parameter (obj_name)

FIXTURE_TEST_CASE(KeyStoreGetKey_TempFile, KeyStoreFixture)
{
    const char tempKey[] = "tempkey from file";
    {
        ofstream f(GetName());
        f << tempKey;
    }

    const KFile* file;
    REQUIRE_RC(KDirectoryOpenFileRead(wd, &file, GetName()));
    
    REQUIRE_RC(KKeyStoreSetTemporaryKeyFromFile(ks, file));
    
    REQUIRE_RC(KKeyStoreGetKey(ks, "boohoo i am ignored here", &key)); 
    REQUIRE_NOT_NULL(key);
    REQUIRE_EQ(string(tempKey), string(key->value.addr, key->value.len));
    
    // now. ask to forget
    REQUIRE_RC(KKeyStoreSetTemporaryKeyFromFile(ks, NULL));
    REQUIRE_RC_FAIL(KKeyStoreGetKey(ks, "boohoo i am ignored here", &key)); 
    
    REQUIRE_RC(KFileRelease(file));
    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

FIXTURE_TEST_CASE(KeyStoreGetKey_Kfg_Default, KeyStoreFixture)
{
    REQUIRE_RC_FAIL(KKeyStoreGetKey(ks, "", &key));
}

FIXTURE_TEST_CASE(KeyStoreGetKey_Kfg, KeyStoreFixture)
{
    const char tempKey[] = "tempkey from file";
    {
        ofstream f(GetName());
        f << tempKey << endl;
    }
    
    KfgUpdateNode(KFG_KRYPTO_PWFILE, GetName());
    
    REQUIRE_RC(KKeyStoreSetConfig(ks, kfg));
    
    REQUIRE_RC(KKeyStoreGetKey(ks, NULL, &key));
    REQUIRE_NOT_NULL(key);
    REQUIRE_EQ(string(tempKey), string(key->value.addr, key->value.len));
    
    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

FIXTURE_TEST_CASE(KeyStoreGetKey_Protected, KeyStoreFixture)
{
    const char tempKey[] = "another tempkey from file";
    {
        ofstream f(GetName());
        f << tempKey << endl;
    }
    
    KfgUpdateNode("/repository/user/protected/dbGaP-2956/root", ".");
    KfgUpdateNode("/repository/user/protected/dbGaP-2956/encryption-key-path", GetName());

    REQUIRE_RC(KKeyStoreSetConfig(ks, kfg));
    
    REQUIRE_RC(KKeyStoreGetKey(ks, "just give us the current repo's key", &key));
    REQUIRE_NOT_NULL(key);
    REQUIRE_EQ(string(tempKey), string(key->value.addr, key->value.len));
    
    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

FIXTURE_TEST_CASE(KeyStoreGetKeyById_Protected, KeyStoreFixture)
{
    const char tempKey[] = "another tempkey from file";
    {
        ofstream f(GetName());
        f << tempKey << endl;
    }
    
    KfgUpdateNode("/repository/user/protected/dbGaP-2956/root", ".");
    KfgUpdateNode("/repository/user/protected/dbGaP-2956/encryption-key-path",
        "wrong file!");
    KfgUpdateNode("/repository/user/protected/dbGaP-2957/root", ".");
    KfgUpdateNode("/repository/user/protected/dbGaP-2957/encryption-key-path",
        GetName());

    REQUIRE_RC(KKeyStoreSetConfig(ks, kfg));
    
    REQUIRE_RC(KKeyStoreGetKeyByProjectId(ks,
        "give us the key for 2957", &key, 2957));
    REQUIRE_NOT_NULL(key);
    REQUIRE_EQ(string(tempKey), string(key->value.addr, key->value.len));
    
    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

//
//  Object Id / Object name bindings
//

class ObjIdBindingFixture : public KeyStoreFixture 
{
public:
    ObjIdBindingFixture()
    {
    }
    ~ObjIdBindingFixture()
    {
        if (bindings.length() != 0 && KDirectoryRemove(wd, true, bindings.c_str()) != 0)
           throw logic_error("ObjIdBindingFixture::TearDown: KDirectoryRemove failed");   
    }
    void SetUp(const string& bindingsFileName)
    {
        bindings = string("./") + bindingsFileName;
        if (KKeyStoreSetBindingsFile(ks, bindings.c_str()) != 0)
           throw logic_error("ObjIdBindingFixture::SetUp: KeyStoreSetBindingsFile failed");   
        KDirectoryRemove(wd, true, bindings.c_str());  // does not have to exist
    }
    
    string bindings;
};

FIXTURE_TEST_CASE(ObjIdRegister, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name1;
    CONST_STRING(&name1, "name1");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1));
    
    String name2;
    CONST_STRING(&name2, "name2");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 2, &name2));
}

FIXTURE_TEST_CASE(ObjIdRegister_Found_Same, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name1;
    CONST_STRING(&name1, "name1");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1));
    
    String name2;
    CONST_STRING(&name2, "name2");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 2, &name2));
    
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1)); // same name, no problem
}

FIXTURE_TEST_CASE(ObjIdRegister_Found_Different, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name1;
    CONST_STRING(&name1, "name1");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1));
    
    String name2;
    CONST_STRING(&name2, "name2");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 2, &name2));
    
    REQUIRE_RC_FAIL(KKeyStoreRegisterObject(ks, 1, &name2)); // name differs
}

FIXTURE_TEST_CASE(ObjIdById_Found, ObjIdBindingFixture)
{
    SetUp(GetName());

    String name123;
    CONST_STRING(&name123, "name123");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 123, &name123));
    
    String name12;
    CONST_STRING(&name12, "name12");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 12, &name12));
    
    String name1;
    CONST_STRING(&name1, "name1");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1));
    
    const String* res;
    
    REQUIRE_RC(KKeyStoreGetObjectName(ks, 123, &res));
    REQUIRE_NOT_NULL(res);
    REQUIRE_EQ(StringCompare(res, &name123), 0);
    StringWhack(res);
    
    REQUIRE_RC(KKeyStoreGetObjectName(ks, 12, &res));
    REQUIRE_NOT_NULL(res);
    REQUIRE_EQ(StringCompare(res, &name12), 0);
    StringWhack(res);
    
    REQUIRE_RC(KKeyStoreGetObjectName(ks, 1, &res));
    REQUIRE_NOT_NULL(res);
    REQUIRE_EQ(StringCompare(res, &name1), 0);
    StringWhack(res);
}

FIXTURE_TEST_CASE(ObjIdById_NotFound, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name100;
    CONST_STRING(&name100, "name100");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 100, &name100));
    
    String name200;
    CONST_STRING(&name200, "name200");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 200, &name200));
    
    const String* res;
    REQUIRE_RC_FAIL(KKeyStoreGetObjectName(ks, 100200, &res));
}

FIXTURE_TEST_CASE(ObjId_DefaultLocation, ObjIdBindingFixture)
{
    const String* res;
    REQUIRE_RC_FAIL(KKeyStoreGetObjectName(ks, 1, &res)); // this will fail but set location to default
    
    // verify default location
    String* home;
    REQUIRE_RC(KConfigReadString(kfg, "NCBI_HOME", &home));
    REQUIRE_NOT_NULL(home);
    
    const char* loc = KKeyStoreGetBindingsFile(ks);
    REQUIRE_NOT_NULL(loc);
    REQUIRE_EQ(string(loc), string(home->addr, home->size) + "/objid.mapping"); 
    StringWhack(home);
}

FIXTURE_TEST_CASE(ObjIdByName_Found, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name11;
    CONST_STRING(&name11, "name11");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 11, &name11));
    
    String name21;
    CONST_STRING(&name21, "name21");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 21, &name21));
    
    String name1;
    CONST_STRING(&name1, "name1");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 1, &name1));
    
    uint32_t id;
    
    REQUIRE_RC(VKKeyStoreGetObjectId(ks, &name11, &id));
    REQUIRE_EQ((uint32_t)11, id);

    REQUIRE_RC(VKKeyStoreGetObjectId(ks, &name21, &id));
    REQUIRE_EQ((uint32_t)21, id);
    
    REQUIRE_RC(VKKeyStoreGetObjectId(ks, &name1, &id));
    REQUIRE_EQ((uint32_t)1, id);
}
FIXTURE_TEST_CASE(ObjIdByName_NotFound, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    String name11;
    CONST_STRING(&name11, "name11");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 11, &name11));
    
    String name21;
    CONST_STRING(&name21, "name21");
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 21, &name21));
    
    String name1;
    CONST_STRING(&name1, "name1");
    
    uint32_t id;
    
    REQUIRE_RC_FAIL(VKKeyStoreGetObjectId(ks, &name1, &id));
}

FIXTURE_TEST_CASE(ObjIdRegister_Lock, ObjIdBindingFixture)
{   // make sure registration fails while the file is locked
    SetUp(GetName());

    KFile* lockedFile;
    REQUIRE_RC(KDirectoryCreateExclusiveAccessFile(wd, &lockedFile, true, 0600, kcmOpen, GetName()));
    
    String name11;
    CONST_STRING(&name11, "name11");
    REQUIRE_RC_FAIL(KKeyStoreRegisterObject(ks, 11, &name11));
    
    REQUIRE_RC(KFileRelease(lockedFile));
    
    // now, will work
    REQUIRE_RC(KKeyStoreRegisterObject(ks, 11, &name11));
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

const char UsageDefaultName[] = "test-keystore";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KeyStoreTestSuite(argc, argv);
    return rc;
}

}
