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
* Unit test fixture for the Kfg tests
*/

#include <string>

#include <kfs/directory.h>

struct KConfig;
struct KFile;
struct VPath;
struct KConfigNode;

// test fixture for creation and lookup of kfg files
class KfgFixture
{
public:
    KfgFixture();
    ~KfgFixture();

    void MakeFile(const char* name, const char* contents);

    void LoadFile(const char* name);

    void CreateAndLoad(const std :: string & sname, const char* contents);

    bool GetValue(const char* path, std :: string& value);
    bool ValueMatches(const char* path, const char* value, bool nullAsEmpty=false);
    void UpdateNode(const char* key, const char* value);

    std :: string DirPath(const KDirectory* dir);
    std :: string GetHomeDirectory();
    const KConfigNode* GetNode(const char* path);
    std :: string ReadContent(const std :: string& fileName);

    KDirectory* wd;
    struct KConfig* kfg;
    struct KFile* file;
    struct VPath* path;

    struct KConfigNode* node;

    static const int BufSize = 8192;
    char buf[BufSize];
    size_t num_read;
    size_t num_writ;

    static std :: string apppath; // only gets set for the 1st instance of KConfig; save it here for the corresponding test case
};
