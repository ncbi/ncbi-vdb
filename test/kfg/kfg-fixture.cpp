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
* Unit testiung fuixture for Kfg tests
*/

#include "kfg-fixture.hpp"

#include <stdexcept>
#include <iostream>

#include <kfg/config.h>
#include <vfs/path.h>
#include <kfs/impl.h>

using namespace std;

KfgFixture :: KfgFixture() : wd(0), kfg(0), file(0), path(0), node(0)
{
    if ( KDirectoryNativeDir ( & wd ) != 0 )
        throw std :: logic_error("KfgFixture: KDirectoryNativeDir failed");

    if (KConfigMake ( & kfg, NULL ) != 0)
        throw logic_error("KfgFixture: KConfigMake failed");

    if (apppath.length() == 0) // first call
    {
        if (!GetValue("APPPATH", apppath))
            throw logic_error("KfgFixture: GetValue failed");
    }
}

KfgFixture :: ~KfgFixture()
{
    if ( node != 0 && KConfigNodeRelease(node) )
    {
        cerr << "~KfgFixture: KConfigNodeRelease failed" << endl;
    }

    if ( KConfigRelease ( kfg ) != 0 )
    {
        cerr << "~KfgFixture: KConfigRelease failed" << endl;
    }

    if ( KDirectoryRelease ( wd ) != 0 )
    {
        cerr << "~KfgFixture: KDirectoryRelease failed" << endl;
    }

    if ( KFileRelease( file ) != 0 )
    {
        cerr << "~KfgFixture: KFileRelease failed" << endl;
    }

    if ( path && VPathRelease ( path) != 0 )
    {
        cerr << "~KfgFixture: VPathRelease failed" << endl;
    }
}

void
KfgFixture :: MakeFile(const char* name, const char* contents)
{
    if (KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, name) != 0)
        throw logic_error("MakeFile: KDirectoryCreateFile failed");

    size_t num_writ=0;
    if (KFileWrite(file, 0, contents, strlen(contents), &num_writ) != 0)
        throw logic_error("MakeFile: KFileWrite failed");

    if (KFileRelease(file) != 0)
        throw logic_error("MakeFile: KFileRelease failed");

    file=0;
}

void
KfgFixture :: LoadFile(const char* name)
{
    if (KDirectoryOpenFileRead(wd, (const KFile**)&file, name) != 0)
        throw logic_error("LoadFile: KDirectoryOpenFileRead failed");

    if (KConfigLoadFile ( kfg, name, file) != 0)
        throw logic_error("LoadFile: KConfigLoadFile failed");

    if (KFileRelease(file) != 0)
        throw logic_error("LoadFile: KFileRelease failed");

    file=0;
}

void
KfgFixture :: CreateAndLoad(const string & sname, const char* contents)
{
    const char * name = sname . c_str ();
#ifdef DBG_KFG
    cout << "26 CreateAndLoad(" << name << ")\n";
#endif
    MakeFile(name, contents);
    LoadFile(name);
    // the .kfg is not needed anymore
    if (KDirectoryRemove(wd, true, name) != 0)
        throw logic_error("CreateAndLoad: KDirectoryRemove failed");
#ifdef DBG_KFG
    cout << "32 CreateAndLoad(" << name << ")\n";
#endif
}

bool
KfgFixture :: GetValue(const char* path, string& value)
{
    const KConfigNode *node;
    rc_t rc=KConfigOpenNodeRead(kfg, &node, "%.*s", strlen(path), path);
    if (rc == 0)
    {
        rc = KConfigNodeRead(node, 0, buf, BufSize, &num_read, NULL);
        if (rc != 0)
            throw logic_error("GetValue: KConfigNodeRead failed");
        buf[num_read]=0;
        value=buf;
        return KConfigNodeRelease(node) == 0;
    }
    return false;
}

bool
KfgFixture :: ValueMatches(const char* path, const char* value, bool nullAsEmpty)
{
    if (nullAsEmpty && value == 0)
    {
        value="";
    }
    string v;
    if (GetValue(path, v))
    {
        bool ret=true;
        if (v != string(value, strlen(value)))
        {
            cerr << "ValueMatches mismatch: expected='" << value << "', actual='" << v << "'" << endl;
            ret=false;
        }
        return ret;
    }
    return false;
}

void
KfgFixture :: UpdateNode(const char* key, const char* value)
{
    KConfigNode *node;
    if (KConfigOpenNodeUpdate(kfg, &node, key) != 0)
        throw logic_error("UpdateNode: KConfigOpenNodeUpdate failed");
    if (KConfigNodeWrite(node, value, strlen(value)) != 0)
        throw logic_error("UpdateNode: KConfigNodeWrite failed");
    if (KConfigNodeRelease(node) != 0)
        throw logic_error("UpdateNode: KConfigNodeRelease failed");
}

string
KfgFixture :: DirPath(const KDirectory* dir)
{
    char resolved[4097];
    if (KDirectoryResolvePath(dir, true, resolved, sizeof resolved, ".") == 0)
    {
        return string(resolved);
    }
    else
    {
        return "??";
    }
}

string
KfgFixture :: GetHomeDirectory()
{
    string ret;
    if (getenv("HOME") != NULL)
        ret = getenv("HOME");
    else if (getenv("USERPROFILE") != NULL) // on Windows the value is taken from USERPROFILE
        ret = getenv("USERPROFILE");

    if (KDirectoryResolvePath(wd, true, buf, sizeof(buf), ret.c_str()) != 0)
        throw logic_error("GetHomeDirectory: KDirectoryResolvePath failed");

    return string(buf);
}

const KConfigNode*
KfgFixture :: GetNode(const char* path)
{
    if ( node != 0 && KConfigNodeRelease(node) )
        throw logic_error("GetNode: KConfigNodeRelease failed");
    if (KConfigOpenNodeRead(kfg, (const KConfigNode**)&node, path) != 0)
        throw logic_error("GetNode: KConfigOpenNodeRead failed");
    return node;
}

string
KfgFixture :: ReadContent(const string& fileName)
{
    KFile* f;
    KDirectoryOpenFileRead(wd, (const KFile**)&f, fileName.c_str());

    if (KFileReadAll ( f, 0, buf, BufSize, &num_read ) != 0)
        throw logic_error("ReadContent: KFileReadAll failed");

    KFileRelease(f);
    return string(buf, num_read);
}

string KfgFixture :: apppath;
