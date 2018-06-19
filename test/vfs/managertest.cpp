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
* Unit tests for VFSManager interface
*/

#include <ktst/unit_test.hpp>

#include <klib/text.h>
#include <klib/rc.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path.h>

#include <kfg/config.h>
#include <kfg/keystore.h>
#include <kfg/kfg-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <cstdlib>
#include <stdexcept>

#include <string.h>

#include <sysalloc.h>

#include "../../libs/vfs/path-priv.h" /* vpNameOrOID */

TEST_SUITE(VManagerTestSuite);

using namespace std;

#define ALL

const char* pwFileName="pwfile";
const char* password = "password";

class BaseMgrFixture
{
public:
    static const int BufSize = 1024;

protected:
    BaseMgrFixture(const char* password)
        : wd ( 0 )
        , mgr ( 0 )
        , vpath(0)
        , num_read(0)
        , num_writ ( 0 )
    {
        if (KDirectoryNativeDir ( &wd ) != 0)
           throw logic_error("MgrFixture: KDirectoryNativeDir failed");   
    
        if (VFSManagerMake(&mgr) != 0 || mgr == 0)
           throw logic_error("MgrFixture: VFSManagerMake failed");    
        
        // stick a reference to our pwfile into the VFSManager's configuration 
        // (no easy way to do that through putenv that would work on Windows dynamic since
        // vfs.dll would have its own copy of environment)
        const KConfig* kfg = VFSManagerGetConfig(mgr);
        if (!kfg)
           throw logic_error("MgrFixture: VFSManagerGetConfig failed");    
        KConfigNode* node;
        if (KConfigOpenNodeUpdate((KConfig*)kfg, &node, KFG_KRYPTO_PWFILE) != 0)
           throw logic_error("MgrFixture: KConfigOpenNodeUpdate failed");    
           
        if (KConfigNodeWrite(node, pwFileName, strlen(pwFileName)) != 0)
           throw logic_error("MgrFixture: KConfigNodeWrite failed");   
        if (KConfigNodeRelease(node) != 0)
           throw logic_error("MgrFixture: KConfigNodeRelease failed");   

        // make sure pwfile contains the correct password (some tests might update it)
        CreateFile(pwFileName, password);
    }
    ~BaseMgrFixture()
    {
        if (KDirectoryRemove(wd, true, pwFileName))
           throw logic_error("~MgrFixture: KDirectoryRemove failed");   
        if (vpath && VPathRelease(vpath) != 0)
           throw logic_error("~MgrFixture: VPathRelease failed");
        if (mgr && VFSManagerRelease(mgr) != 0)
           throw logic_error("~MgrFixture: VFSManagerRelease failed");
        if (KDirectoryRelease(wd) != 0)
           throw logic_error("~MgrFixture: KDirectoryRelease failed");   
    }
    
    void CreateFile(const string& name, const string& content)
    {
        KFile* f;
        if (KDirectoryCreateFile(wd, &f, false, 0660, kcmInit, name.c_str()) != 0)
           throw logic_error("CreateFile: KDirectoryCreateFile failed");   
        if (KFileWrite(f, 0, content.c_str(), content.size(), &num_writ) != 0 || num_writ != content.size())
           throw logic_error("CreateFile: KDirectoryOpenFileWrite failed");   
        if (KFileWrite(f, content.size(), "\n", 1, &num_writ) != 0 || num_writ != 1)
           throw logic_error("CreateFile: KDirectoryOpenFileWrite failed");   
        if (KFileRelease(f) != 0)
           throw logic_error("CreateFile: KFileRelease failed");   
    }
    string ReadContent(const KFile* file)
    {
        if (KFileReadAll ( file, 0, buf, BufSize, &num_read ) != 0)
           throw logic_error("ReadContent: KFileReadAll failed");   
        return string(buf, num_read);
    }

    KDirectory* wd;
    VFSManager* mgr;
    VPath* vpath;

    // for use in KFileReadAll et al
    char buf[BufSize];
    size_t num_read;
    size_t num_writ;
};

class MgrFixture : protected BaseMgrFixture {
protected:
    MgrFixture () : BaseMgrFixture ( password ) {}
};

class NonAsciiInvalidUnicodeMgrFixture : protected BaseMgrFixture {
static const char* s_password;
protected:
    static string getPassword ( void ) { return s_password; }
    NonAsciiInvalidUnicodeMgrFixture () : BaseMgrFixture ( s_password ) {}
};
const char* NonAsciiInvalidUnicodeMgrFixture::s_password("a\243cdefghtjklm");

class NonAsciiValidUnicodeMgrFixture : protected BaseMgrFixture {
static const char* s_password;
protected:
    static string getPassword ( void ) { return s_password; }
    NonAsciiValidUnicodeMgrFixture () : BaseMgrFixture ( s_password ) {}
};
const char* NonAsciiValidUnicodeMgrFixture::s_password("a\xC2\243cdefghtjklm");

#ifdef ALL

TEST_CASE(Make_Basic)
{
    VFSManager* mgr;
    REQUIRE_RC(VFSManagerMake(&mgr));
    REQUIRE_RC(VFSManagerRelease(mgr));
}

/////////////// Accessing encrypted objects
string protectedFileContent = "contents of a single-file object";

FIXTURE_TEST_CASE(OpenFileRead_Decrypt, MgrFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( mgr, &vpath, "./ncbi/protected1/SRR999997.ncbi_enc")); // an encrypted copy of ./ncbi/protected1/SRR999997
    
    // read contents of an encrypted file
    const KFile *f;
    REQUIRE_RC(VFSManagerOpenFileReadDecrypt (mgr, &f, vpath));    
    
    REQUIRE_EQ(protectedFileContent, ReadContent(f));

    REQUIRE_RC(KFileRelease(f));
}
FIXTURE_TEST_CASE(OpenDirRead_Decrypt, MgrFixture)
{
    REQUIRE_RC(VFSManagerMakePath ( mgr, &vpath, "./ncbi_enc")); // this is an encrypted kar archive of directory ./ncbi
    
    const KDirectory *dir;
    REQUIRE_RC(VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt (mgr, wd, &dir, vpath));    
    
    // read contents of a file inside an encrypted archive
    const KFile *f;
    KDirectoryOpenFileRead(dir, &f, "protected1/SRR999997");
    REQUIRE_EQ(protectedFileContent, ReadContent(f));
    REQUIRE_RC(KFileRelease(f));
    
    REQUIRE_RC(KDirectoryRelease(dir));
}

FIXTURE_TEST_CASE(CreateFile_Encrypt, MgrFixture)
{
    string uri = string("ncbi-file:") + GetName() + "?enc&pwfile=pwfile";
    REQUIRE_RC(VFSManagerMakePath ( mgr, &vpath, uri.c_str())); 
    
    // create with encryption
    KFile* f;
    REQUIRE_RC(VFSManagerCreateFile(mgr, &f, false, 0660, kcmInit, vpath));
    string content="this is supposed to be encrypted";
    REQUIRE_RC(KFileWrite(f, 0, content.c_str(), content.size(), &num_writ));
    REQUIRE_EQ(content.size(), num_writ);
    REQUIRE_RC(KFileRelease(f));
    
    // open as unencrypted - should fail
    KDirectoryOpenFileRead(wd, (const KFile**)&f, GetName());
    REQUIRE_NE(content, ReadContent(f));
    REQUIRE_RC(KFileRelease(f));
    
    // open as encrypted
    REQUIRE_RC(VFSManagerOpenFileReadDecrypt (mgr, (const KFile**)&f, vpath));    
    REQUIRE_EQ(content, ReadContent(f));
    REQUIRE_RC(KFileRelease(f));
    
    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

FIXTURE_TEST_CASE(OpenFileWriteFile_Encrypt_NotEncrypted, MgrFixture)
{   // attempt to open an unencrypted file for encypted writing
    // create unencrypted
    CreateFile(GetName(), "garbage");

    string uri = string("ncbi-file:") + GetName() + "?enc&pwfile=pwfile";
    REQUIRE_RC(VFSManagerMakePath ( mgr, &vpath, uri.c_str())); 

    // open as encrypted - fail
    KFile* f;
    // this will output an error message from KEncFileMakeIntValidSize:
    LOG(ncbi::NK::LogLevel::e_error,
        "Expecting an err. message from KEncFileMakeIntValidSize...\n");
    REQUIRE_RC_FAIL(VFSManagerOpenFileWrite(mgr, &f, false, vpath)); 
    REQUIRE_RC(KFileRelease(f));

    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

FIXTURE_TEST_CASE(OpenFileWriteFile_Encrypt, MgrFixture)
{   // open an encrypted file for encypted writing
    string uri = string("ncbi-file:") + GetName() + "?enc&pwfile=pwfile";
    REQUIRE_RC(VFSManagerMakePath ( mgr, &vpath, uri.c_str())); 

    // create encrypted
    KFile* f;
    REQUIRE_RC(VFSManagerCreateFile(mgr, &f, false, 0660, kcmInit, vpath));
    string content="old encrypted content";
    REQUIRE_RC(KFileWrite(f, 0, content.c_str(), content.size(), &num_writ));
    REQUIRE_EQ(content.size(), num_writ);
    REQUIRE_RC(KFileRelease(f));

    // open, overwrite
    REQUIRE_RC(VFSManagerOpenFileWrite(mgr, &f, false, vpath));
    content="new and shiny and encrypted";
    REQUIRE_RC(KFileWrite(f, 0, content.c_str(), content.size(), &num_writ));
    REQUIRE_EQ(content.size(), num_writ);
    REQUIRE_RC(KFileRelease(f));
    
    // open as unencrypted - fail
    KDirectoryOpenFileRead(wd, (const KFile**)&f, GetName());
    REQUIRE_NE(content, ReadContent(f));
    REQUIRE_RC(KFileRelease(f));
    
    // open as encrypted
    REQUIRE_RC(VFSManagerOpenFileReadDecrypt (mgr, (const KFile**)&f, vpath));    
    REQUIRE_EQ(content, ReadContent(f));
    REQUIRE_RC(KFileRelease(f));

    REQUIRE_RC(KDirectoryRemove(wd, true, GetName()));
}

/////////////// Managing password file
FIXTURE_TEST_CASE(GetKryptoPassword, MgrFixture)
{
    REQUIRE_RC(VFSManagerGetKryptoPassword(mgr, buf, BufSize, &num_read));
    REQUIRE_EQ(string("password"), string(buf, num_read));
}
#endif

/* VDB-3590 */
/* Non-Ascii encryption key - invalid UNICODE : see s_password in fixture */
FIXTURE_TEST_CASE(GetNonAscii1KryptoPassword, NonAsciiInvalidUnicodeMgrFixture)
{
    REQUIRE_RC(VFSManagerGetKryptoPassword(mgr, buf, BufSize, &num_read));
    REQUIRE_EQ(getPassword(), string(buf, num_read));
}
/* Non-Ascii encryption key - valid UNICODE : see s_password in fixture */
FIXTURE_TEST_CASE(GetNonAscii2KryptoPassword, NonAsciiValidUnicodeMgrFixture)
{
    REQUIRE_RC(VFSManagerGetKryptoPassword(mgr, buf, BufSize, &num_read));
    REQUIRE_EQ(getPassword(), string(buf, num_read));
}

#ifdef ALL
FIXTURE_TEST_CASE(UpdateKryptoPassword_NoOutput, MgrFixture)
{
    string newPwd("new_pwd1");
    rc_t rc = VFSManagerUpdateKryptoPassword(mgr, newPwd.c_str(), newPwd.size(), 0, 0);
// directory permissions are not looked at on Windows    
#if !WINDOWS    
    REQUIRE_EQ(rc, RC(rcVFS, rcEncryptionKey, rcUpdating, rcDirectory, rcExcessive));
#else
    REQUIRE_EQ(rc, (rc_t)0);
#endif    

    REQUIRE_RC(VFSManagerGetKryptoPassword(mgr, buf, BufSize, &num_read));
    REQUIRE_EQ(newPwd, string(buf, num_read));
}
FIXTURE_TEST_CASE(UpdateKryptoPassword_Output, MgrFixture)
{
    string newPwd("new_pwd2");
    // VFSManagerUpdateKryptoPassword returns the directory containing the password file,
    // to help instruct user to chmod if permissions are too lax
    rc_t rc = VFSManagerUpdateKryptoPassword(mgr, newPwd.c_str(), newPwd.size(), buf, BufSize);
// directory permissions are not looked at on Windows    
#if !WINDOWS    
    REQUIRE_EQ(rc, RC(rcVFS, rcEncryptionKey, rcUpdating, rcDirectory, rcExcessive));
#else
    REQUIRE_EQ(rc, (rc_t)0);
#endif    
    REQUIRE_EQ(string("."), string(buf));
    
    REQUIRE_RC(VFSManagerGetKryptoPassword(mgr, buf, BufSize, &num_read));
    REQUIRE_EQ(newPwd, string(buf, num_read));
}

//TODO: VFSManagerWGAValidateHack

//
//  Object Id / Object name bindings
//

class ObjIdBindingFixture : public MgrFixture 
{
public:
    static char bindings[1024];
    
    ObjIdBindingFixture()
    {
    }
    ~ObjIdBindingFixture()
    {
        const char* bFile = VFSManagerGetBindingsFile(mgr);
        if (bFile != NULL)
            KDirectoryRemove(wd, true, bFile);
        VFSManagerSetBindingsFile(mgr, NULL);
    }
    void SetUp(const string& bindingsFileName)
    {
        string bFile = string("./") + bindingsFileName;
        if (string_copy(bindings, sizeof(bindings), bFile.c_str(), bFile.length()) != bFile.length())
           throw logic_error("ObjIdBindingFixture::SetUp: string_copy failed");   
        VFSManagerSetBindingsFile(mgr, bindings);
        KDirectoryRemove(wd, true, bindings);
    }
    rc_t Register(uint32_t id, VPath* p) const
    {
        rc_t rc = VFSManagerRegisterObject(mgr, id, p);
        if (VPathRelease(p) != 0)
           throw logic_error("ObjIdBindingFixture::Register: VPathRelease failed");   
        return rc;
    }
    rc_t Register(uint32_t id, const char* uri)
    {
        VPath* p;
        if (VFSManagerMakePath(mgr, &p, uri) != 0)
           throw logic_error("ObjIdBindingFixture::Register: VFSManagerMakePath failed");
        return Register(id, p);
    }
    rc_t GetById(uint32_t id, string& uri)
    {
        VPath* vp;
        rc_t rc = VFSManagerGetObject(mgr, id, &vp);
        if (rc == 0)
        {
            if (vp == 0)
                throw logic_error("ObjIdBindingFixture::GetById: VFSManagerGetObject returned NULL");   
                
            const String* p;
            if (VPathMakeString(vp, &p) != 0 || p == 0)
                throw logic_error("ObjIdBindingFixture::GetById: VPathMakeString failed");   
                
            uri = string(p->addr, p->size);
            
            free((void*)p);
            if (VPathRelease(vp) != 0)
                throw logic_error("ObjIdBindingFixture::GetById: VPathRelease failed");   
        }
        return rc;
    }
    rc_t GetByUri(const string& uri, uint32_t& id)
    {
        VPath* p;
        if (VFSManagerMakePath(mgr, &p, uri.c_str()) != 0)
            throw logic_error("ObjIdBindingFixture::GetByUri: VFSManagerMakePath failed"); 
        rc_t rc = VFSManagerGetObjectId(mgr, p, &id);
        if (VPathRelease(p) != 0)
            throw logic_error("ObjIdBindingFixture::GetById: VPathRelease failed");   
        return rc;
    }
};

char ObjIdBindingFixture::bindings[];

FIXTURE_TEST_CASE(ObjIdRegister, ObjIdBindingFixture)
{
    SetUp(GetName());
    
//  REQUIRE_RC(Register(1, "ncbi-acc:acc1"));
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    REQUIRE_RC(Register(2, "ncbi-file:acc2?tic=22"));
}
FIXTURE_TEST_CASE(ObjIdRegister_Found_Same, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    REQUIRE_RC(Register(2, "ncbi-file:acc2?tic=22"));
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1")); // same name, no problem
}
FIXTURE_TEST_CASE(ObjIdRegister_Found_Different, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    REQUIRE_RC(Register(2, "ncbi-file:acc2?tic=22"));
    REQUIRE_RC_FAIL(Register(1, "ncbi-acc:acc11?tic=1")); // name differs
}
FIXTURE_TEST_CASE(ObjIdById_Found, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(123, "ncbi-file:acc123?tic=3"));
    REQUIRE_RC(Register(12, "ncbi-acc:acc12?tic=2"));
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    
    string uri;
    
    REQUIRE_RC(GetById(123, uri));
    REQUIRE_EQ(uri, string("ncbi-file:acc123?tic=3"));
    
    REQUIRE_RC(GetById(12, uri));
    REQUIRE_EQ(uri, string("ncbi-acc:acc12?tic=2"));
    
    REQUIRE_RC(GetById(1, uri));
    REQUIRE_EQ(uri, string("ncbi-acc:acc1?tic=1"));
}
FIXTURE_TEST_CASE(ObjIdById_NotFound, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(100, "ncbi-acc:acc1?tic=1"));
    REQUIRE_RC(Register(200, "ncbi-file:acc2?tic=1"));
    
    string uri;
    
    REQUIRE_RC_FAIL(GetById(100200, uri));
}
FIXTURE_TEST_CASE(ObjId_DefaultLocation, ObjIdBindingFixture)
{
    VFSManagerSetBindingsFile(mgr, NULL);
    REQUIRE_RC_FAIL(VFSManagerGetObject(mgr, 1, &vpath)); // this will fail but set VFSManagerBindings to default
    const char* bindings = VFSManagerGetBindingsFile(mgr);
    REQUIRE_NOT_NULL(bindings);
    
    // verify default location
    String* home;
    REQUIRE_RC(KConfigReadString(VFSManagerGetConfig(mgr), "NCBI_HOME", &home));
    REQUIRE_NOT_NULL(home);
    REQUIRE_EQ(string(bindings), string(home->addr, home->size) + "/objid.mapping"); 
    StringWhack(home);
}

FIXTURE_TEST_CASE(ObjIdByName_Found, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(11, "ncbi-acc:acc11?tic=3"));
    REQUIRE_RC(Register(21, "ncbi-file:acc21?tic=22"));
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    
    uint32_t id;
    
    REQUIRE_RC(GetByUri(string("ncbi-acc:acc11?tic=3"), id));
    REQUIRE_EQ((uint32_t)11, id);

    REQUIRE_RC(GetByUri(string("ncbi-file:acc21?tic=22"), id));
    REQUIRE_EQ((uint32_t)21, id);
    
    REQUIRE_RC(GetByUri(string("ncbi-acc:acc1?tic=1"), id));
    REQUIRE_EQ((uint32_t)1, id);
}
FIXTURE_TEST_CASE(ObjIdByName_NotFound, ObjIdBindingFixture)
{
    SetUp(GetName());
    
    REQUIRE_RC(Register(1, "ncbi-acc:acc1?tic=1"));
    REQUIRE_RC(Register(2, "ncbi-file:acc2?tic=2"));
    
    uint32_t id;
    REQUIRE_RC_FAIL(GetByUri(string("ncbi-acc:acc2?tic=1"), id));
}

FIXTURE_TEST_CASE(DontRegistrer_vpNameOrOID, ObjIdBindingFixture) {
    SetUp(GetName());

    String id;
    CONST_STRING(&id, "1154149");

    String tick;
    CONST_STRING(&tick, "D2BE86BF-CCD4-4114-9C60-FCB5422C64F5");

    VPath* p = NULL;
    REQUIRE_RC(VFSManagerMakePath(mgr, &p, "ncbi-file:%S?tic=%S", &id, &tick));

    REQUIRE(p);
    REQUIRE(p-> path_type == vpNameOrOID);

    REQUIRE_RC_FAIL(Register(1154149, p));
}

FIXTURE_TEST_CASE(RegistrerGoodPath, ObjIdBindingFixture) {
    SetUp(GetName());

    String id;
    CONST_STRING(&id, "name.ext");

    String tick;
    CONST_STRING(&tick, "D2BE86BF-CCD4-4114-9C60-FCB5422C64F5");

    VPath* p = NULL;
    REQUIRE_RC(VFSManagerMakePath(mgr, &p, "ncbi-file:%S?tic=%S", &id, &tick));

    REQUIRE(p);

    REQUIRE_RC(Register(1154149, p));
}

FIXTURE_TEST_CASE(DontRegistrer_BadScheme, ObjIdBindingFixture) {
    SetUp(GetName());

    String id;
    CONST_STRING(&id, "name.ext");

    String tick;
    CONST_STRING(&tick, "D2BE86BF-CCD4-4114-9C60-FCB5422C64F5");

    VPath* p = NULL;
    REQUIRE_RC(VFSManagerMakePath(mgr, &p, "file:%S?tic=%S", &id, &tick));

    REQUIRE(p);

    REQUIRE_RC_FAIL(Register(1154149, p));
}

FIXTURE_TEST_CASE(DontRegistrer_NoQuery, ObjIdBindingFixture) {
    SetUp(GetName());

    String id;
    CONST_STRING(&id, "name.ext");

    VPath* p = NULL;
    REQUIRE_RC(VFSManagerMakePath(mgr, &p, "ncbi-file:%S", &id));

    REQUIRE(p);

    REQUIRE_RC_FAIL(Register(1154149, p));
}

FIXTURE_TEST_CASE(RegistrerGoodAccPath, ObjIdBindingFixture) {
    SetUp(GetName());

    String id;
    CONST_STRING(&id, "ACC123");

    String tick;
    CONST_STRING(&tick, "D2BE86BF-CCD4-4114-9C60-FCB5422C64F5");

    VPath* p = NULL;
    REQUIRE_RC(VFSManagerMakePath(mgr, &p, "ncbi-acc:%S?tic=%S", &id, &tick));

    REQUIRE(p);

    REQUIRE_RC(Register(1154149, p));
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
const char UsageDefaultName[] = "test-manager";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VManagerTestSuite(argc, argv);
    return rc;
}

}
