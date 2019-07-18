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
* Unit tests for klib/log interface
*/

#include <ktst/unit_test.hpp>

#include <klib/log.h>
#include <klib/rc.h>
#include <klib/text.h>

using namespace std;

static const size_t BufSize = 1024;
// implementation of KWrtWriter for testing purposes
rc_t CC TestWrtWriter( void * data, const char * buffer, size_t bufsize, size_t * num_writ )
{
    size_t nw = string_copy((char*)data, BufSize, buffer, bufsize);
    if (num_writ != 0)
        *num_writ = nw;
    return 0;
}


// VDB_1223: logging must perform properly when not initialized from KMane/KMain
// Start using main(), never call KWrtInit() or KLogInit()
// call all logging functions, make sure they do not crash

TEST_SUITE(KLogTestSuite);

// Getters working on an implicitly initialized logging
TEST_CASE(KLog_KLogLevelGet)
{
    REQUIRE_EQ((KLogLevel)klogErr, KLogLevelGet());
}

TEST_CASE(KLog_KLogLevelExplain)
{
    char buf[1024];
    REQUIRE_RC(KLogLevelExplain ( klogErr, buf, sizeof(buf), NULL ));
    REQUIRE_EQ(string("err"), string(buf));
}

TEST_CASE(KLog_KLogGetParamStrings)
{
    REQUIRE_EQ(string("fatal"), string(KLogGetParamStrings()[0]));
}

TEST_CASE(KLog_KLogHandlerGet)
{
    REQUIRE_NOT_NULL(KLogHandlerGet());
    REQUIRE_EQ((KWrtWriter)0, KLogHandlerGet()->writer);
    REQUIRE_NULL(KLogHandlerGet()->data);
}

TEST_CASE(KLog_KLogLibHandlerGet)
{
    REQUIRE_NOT_NULL(KLogLibHandlerGet());
    REQUIRE_EQ((KWrtWriter)0, KLogLibHandlerGet()->writer);
    REQUIRE_NULL(KLogLibHandlerGet()->data);
}

TEST_CASE(KLog_KLogWriterGet)
{
    REQUIRE_EQ((KWrtWriter)0, KLogWriterGet());
}

TEST_CASE(KLog_KLogLibWriterGet)
{
    REQUIRE_EQ((KWrtWriter)0, KLogLibWriterGet());
}

TEST_CASE(KLog_KLogDataGet)
{
    REQUIRE_NULL(KLogDataGet());
}

TEST_CASE(KLog_KLogLibDataGet)
{
    REQUIRE_NULL(KLogLibDataGet());
}

TEST_CASE(KLog_KLogFmtHandlerGet)
{
    REQUIRE_NOT_NULL(KLogFmtHandlerGet());
    REQUIRE_EQ((KFmtWriter)0, KLogFmtHandlerGet()->formatter);
    REQUIRE_NULL(KLogFmtHandlerGet()->data);
}

TEST_CASE(KLog_KLogLibFmtHandlerGet)
{
    REQUIRE_NOT_NULL(KLogLibFmtHandlerGet());
    REQUIRE_EQ((KFmtWriter)0, KLogLibFmtHandlerGet()->formatter);
    REQUIRE_NULL(KLogLibFmtHandlerGet()->data);
}

TEST_CASE(KLog_KLogLibFmtFlagsGet)
{
    KLogFmtFlagsGet();
}

TEST_CASE(KLog_KLogFmtFlagsGet)
{
    KLogLibFmtFlagsGet();
}

TEST_CASE(KLog_KLogFmtWriterGet)
{   
    REQUIRE_NE((KFmtWriter)0, KLogFmtWriterGet());
}

TEST_CASE(KLog_KLogLibFmtWriterGet)
{   
    REQUIRE_NE((KFmtWriter)0, KLogLibFmtWriterGet());
}

TEST_CASE(KLog_KLogFmtDataGet)
{   
    REQUIRE_NULL(KLogFmtDataGet());
}

TEST_CASE(KLog_KLogLibFmtDataGet)
{   
    REQUIRE_NULL(KLogLibFmtDataGet());
}

// Writers (just make sure they do not crash)
TEST_CASE(KLog_LogMsg)
{   
    REQUIRE_RC(LogMsg(klogFatal, "test"));
}

TEST_CASE(KLog_LogLibMsg)
{   
    REQUIRE_RC(LogLibMsg(klogFatal, "test"));
}

TEST_CASE(KLog_pLogMsg)
{   
    REQUIRE_RC(pLogMsg(klogFatal, "test $(a) $(b)\n", "a=%s,b=%d", "kaboom", 10));
}

TEST_CASE(KLog_vLogMsg)
{   
    va_list l;
    va_end(l);
    REQUIRE_RC(vLogMsg(klogFatal, "test $(a) $(b)\n", "a=%s,b=%d", l));
}

TEST_CASE(KLog_pLogLibMsg)
{   
    REQUIRE_RC(pLogLibMsg(klogFatal, "test $(a) $(b)\n", "a=%s,b=%d", "kaboom", 10));
}

TEST_CASE(KLog_vLogLibMsg)
{   
    va_list l;
    va_end(l);
    REQUIRE_RC(vLogLibMsg(klogFatal, "test $(a) $(b)\n", "a=%s,b=%d", l));
}

TEST_CASE(KLog_LogErr)
{   
    REQUIRE_RC(LogErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad stuff"));
}

TEST_CASE(KLog_LogLibErr)
{   
    REQUIRE_RC(LogLibErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad stuff"));
}

TEST_CASE(KLog_pLogErr)
{   
    REQUIRE_RC(pLogErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad $(s)", "s=%s", "stuff"));
}

TEST_CASE(KLog_vLogErr)
{   
    va_list l;
    va_end(l);
    REQUIRE_RC(vLogErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad $(s)", "s=%s", l));
}

TEST_CASE(KLog_pLogLibErr)
{   
    REQUIRE_RC(pLogLibErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad $(s)", "s=%s", "stuff"));
}

TEST_CASE(KLog_vLogLibErr)
{   
    va_list l;
    va_end(l);
    REQUIRE_RC(vLogLibErr(klogFatal, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted), "bad $(s)", "s=%s", l));
}

// Setters
TEST_CASE(KLog_KLogLevelSet)
{
    REQUIRE_RC(KLogLevelSet(klogInt));
    REQUIRE_EQ((KLogLevel)klogInt, KLogLevelGet());
}

TEST_CASE(KLog_KLogLevelAdjust)
{
    REQUIRE_RC(KLogLevelSet(klogInt));
    KLogLevelAdjust(2);
    REQUIRE_EQ((KLogLevel)klogWarn, KLogLevelGet());
}

TEST_CASE(KLog_KLogLastErrorCode)
{   // also KLogLastErrorCode
    rc_t rc = RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted);
    KLogLastErrorCodeSet(klogErr, rc);
    REQUIRE_EQ(rc, KLogLastErrorCode());
}

TEST_CASE(KLog_KLogLastErrorCodeReset)
{
    rc_t rc = RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted);
    KLogLastErrorCodeSet(klogErr, rc);
    KLogLastErrorCodeReset();
    REQUIRE_EQ((rc_t)0, KLogLastErrorCode());
}


TEST_CASE(KLog_KLogHandlerSet)
{
    char buf[BufSize];
    REQUIRE_RC(KLogHandlerSet(TestWrtWriter, buf));
    REQUIRE_NOT_NULL(KLogHandlerGet());
    REQUIRE_EQ((KWrtWriter)TestWrtWriter, KLogHandlerGet()->writer);
    REQUIRE_EQ((void*)buf, KLogHandlerGet()->data);
    REQUIRE_RC(KLogHandlerSet(0, 0));
}

TEST_CASE(KLog_KLogLibHandlerSet)
{
    char buf[BufSize];
    REQUIRE_RC(KLogLibHandlerSet(TestWrtWriter, buf));
    REQUIRE_NOT_NULL(KLogLibHandlerGet());
    REQUIRE_EQ((KWrtWriter)TestWrtWriter, KLogLibHandlerGet()->writer);
    REQUIRE_EQ((void*)buf, KLogLibHandlerGet()->data);
    REQUIRE_RC(KLogLibHandlerSet(0, 0));
}

TEST_CASE(KLog_KLogHandlerSetStdOut)
{
    REQUIRE_RC(KLogHandlerSetStdOut());
}
TEST_CASE(KLog_KLogLibHandlerSetStdOut)
{
    REQUIRE_RC(KLogLibHandlerSetStdOut());
}
TEST_CASE(KLog_KLogHandlerSetStdErr)
{
    REQUIRE_RC(KLogHandlerSetStdErr());
}
TEST_CASE(KLog_KLogLibHandlerSetStdErr)
{
    REQUIRE_RC(KLogLibHandlerSetStdErr());
}

TEST_CASE(UninitailizedCrachTest) {
    REQUIRE_RC_FAIL(LOGMSG
        (klogErr, "BANG! YOU SHOULD NOT SEE IT: Logger was not initialized"));
    REQUIRE_RC(KWrtInit("app", 0x01020003));
    REQUIRE_RC(
        PLOGMSG(klogWarn, (klogWarn, "Now you see $(a)", "a=%s", "a warning")));
}

TEST_CASE ( rcBufferrcInsufficientAfterprep_v_args ) {
    REQUIRE_RC ( KWrtInit ( "fastq-dump", 0x02070000 ) );
    REQUIRE_RC ( KLogLibHandlerSetStdErr () );

    rc_t rc = SILENT_RC ( rcNS, rcFile, rcOpening, rcFunction, rcUnsupported );

    size_t s ( 1969 );
    string u ( s + 1, '<' );
    u += "0123456789ABCDEFGHIJKLMNOPQRST";
    string v ( s, '>' );
    v += "56789ABCDEFGHIJKLMNOPRSTUVWXYZ";

    REQUIRE_RC ( pLogLibErr ( klogErr, rc, "This message "
        "used to produce a log failure after error prep_v_args call: "
        "error with http open '$(U) * $(V)'",
        "U=%s,V=%s", u.c_str (), v.c_str () ) );
}

TEST_CASE ( rcBufferrcInsufficientInprep_v_argsstring_vprintf ) {
    REQUIRE_RC ( KWrtInit ( "fastq-dump", 0x02070000 ) );
    REQUIRE_RC ( KLogLibHandlerSetStdErr () );

    rc_t rc = SILENT_RC ( rcNS, rcFile, rcOpening, rcFunction, rcUnsupported );

    size_t s ( 2025 );
    string u ( s + 1, '<' );
    u += "0123456789ABCDEFGHIJ";
    string v ( s, '>' );
    v += "FGHIJKLMNOPRSTUVWXYZ";

    REQUIRE_RC ( pLogLibErr ( klogErr, rc, "This message "
        "used to produce a log failure inside of prep_v_args call: "
        "error with http open '$(U) * $(V)'",
        "U=%s,V=%s", u.c_str (), v.c_str () ) );
}

//TODO:
// KLogFmtFlagsSet    
// KLogLibFmtFlagsSet 
// KLogFmtHandlerSet   
// KLogLibFmtHandlerSet
// KLogFmtHandlerSetDefault
// KLogLibFmtHandlerSetDefault
// KLogInit

//////////////////////////////////////////////////// Main
extern "C"
{
#ifdef WINDOWS
#define main wmain
#endif
int main ( int argc, char *argv [] )
{
    rc_t rc=KLogTestSuite(argc, argv);
    return rc;
}

}
