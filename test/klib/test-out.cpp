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
* Unit tests for klib/out interface
*/

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>
#include <klib/text.h>

#include <cstring>

using namespace std;

// VDB_1223: logging must perform properly when not initialized from KMane/KMain
// Start using main(), never call KWrtInit() or KLogInit()
// call all logging functions, make sure they do not crash

TEST_SUITE(KOutTestSuite);

// Getters working on an implicitly initialized logging
TEST_CASE(Out_KOutHandlerGet)
{
    REQUIRE_NOT_NULL(KOutHandlerGet());
}

TEST_CASE(Out_KOutWriterGet)
{
    REQUIRE_EQ((KWrtWriter)0, KOutWriterGet());
}

TEST_CASE(Out_KOutDataGet)
{
    REQUIRE_NULL(KOutDataGet());
}

// Writers (just make sure they do not crash)
TEST_CASE(Out_KOutMsg)
{
    REQUIRE_RC(KOutMsg("i'm a %s out message", "little"));
}

// Setters
// TODO:
// KOutHandlerSet 
// KOutHandlerSetStdOut 
// KOutHandlerSetStdErr 

// VDB-1352: short-circuit certain formats in KMsgOut
rc_t CC writerFn ( void * self, const char * buffer, size_t bufsize, size_t * num_writ )
{
    string& res = *(string*)self;
    res += string(buffer, bufsize);
    *num_writ = bufsize;
    return 0; 
}

TEST_CASE(KOutMsgShortcut_s)
{
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    REQUIRE_RC(KOutMsg("%s", "some%string"));
    REQUIRE_EQ(output, string("some%string"));
}

TEST_CASE(KOutMsgShortcut_s_sized)
{   // when precision is less than the NUL-terminated string size, output according to precision
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    REQUIRE_RC(KOutMsg("%.*s", 5, "some%string"));
    REQUIRE_EQ(output, string("some%"));
}
TEST_CASE(KOutMsgShortcut_s_sized_extra_size)
{   // when precision is greater than the NUL-terminated string size, output according to precision
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    REQUIRE_RC(KOutMsg("%.*s", 15, "some%string\0ext"));
    REQUIRE_EQ(memcmp("some%string\0ext", output.c_str(), 15), 0);
}

TEST_CASE(KOutMsgShortcut_S)
{
    string output;
    String s;
    CONST_STRING( &s, "some%string" );
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    REQUIRE_RC(KOutMsg("%S", &s));
    REQUIRE_EQ(output, string("some%string"));
}

TEST_CASE(KOutMsgShortcut_c_ascii)
{
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    char ch = 'h';
    REQUIRE_RC(KOutMsg("%c", ch));
    REQUIRE_EQ(output, string(&ch, 1));
}
TEST_CASE(KOutMsgShortcut_c_utf8)
{
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    uint32_t ch = 163; // pound sign
    REQUIRE_RC(KOutMsg("%c", ch));
    REQUIRE_EQ(output[0], '\302');
    REQUIRE_EQ(output[1], '\243');
}

TEST_CASE(KOutMsgShortcut_no_shortcut)
{
    string output;
    REQUIRE_RC(KOutHandlerSet(writerFn, &output));
    REQUIRE_RC(KOutMsg("%s\n", "str"));
    REQUIRE_EQ(output, string("str\n"));
}


//////////////////////////////////////////////////// Main
extern "C"
{
#ifdef WINDOWS
#define main wmain
#endif
int main ( int argc, char *argv [] )
{
    rc_t rc=KOutTestSuite(argc, argv);
    return rc;
}

}
