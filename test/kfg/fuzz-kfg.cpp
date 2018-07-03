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

#include <kfs/directory.h>
#include <kfg/config.h>
#include <kfg/kfg-priv.h>

#include <stdexcept>

using namespace std;

extern "C" int
LLVMFuzzerTestOneInput ( const uint8_t *Data, size_t Size )
{
    KDirectory * wd;
    if ( KDirectoryNativeDir ( & wd ) != 0 )
        throw logic_error("KDirectoryNativeDir failed");
    KConfig * kfg;
    if ( KConfigMake ( & kfg, NULL ) != 0)
        throw logic_error("KConfigMake failed");

    string str ( (const char*)Data, Size );
    // does not matter if parsing fails, we are looking for crashes and leaks
    KConfigParse ( kfg, "", str.c_str() );

    if ( KConfigRelease ( kfg ) != 0 )
        throw logic_error("~KfgFixture: KConfigRelease failed");
    if ( KDirectoryRelease ( wd ) != 0 )
        throw logic_error("~KfgFixture: KDirectoryRelease failed");

    return 0;  // Non-zero return values are reserved for future use.
}

