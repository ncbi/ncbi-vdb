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

#include <kapp/log-xml.h> // XMLLogger
#include <klib/log.h> // PLOGERR
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <fstream> // ifstream
#include <regex>

using std::regex_search;
using std::string;

const XMLLogger* LOGGER(nullptr);
static char XML_LOG[]("tmp.xml");

static rc_t argsHandler(int argc, char* argv[]) {
    char* arg[] = { argv[0], (char*)"-z", XML_LOG };
    int arc(sizeof arg / sizeof arg[0]);

    Args* args(nullptr);
    rc_t rc(ArgsMakeAndHandle(&args, arc, arg,
        1, XMLLogger_Args, XMLLogger_ArgsQty));

    rc_t r2(XMLLogger_Make(&LOGGER, NULL, args));
    if (rc == 0 && r2 != 0)
        rc = r2;

    r2 = ArgsWhack(args);
    if (rc == 0 && r2 != 0)
        rc = r2;

    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER(TestSuite, argsHandler)

TEST_CASE(Test) {
    PLOGERR(klogSys, (klogSys, 0, "123456789a=$(a);23456789=$(bb)",
        "a=%s,bb=%s", "a1abcdefgh", "A2ABCDEFGH")); // 0) nothing truncated

    char a1[](
        "1:PLAIN-NOT-TRUNCATED-3456783012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "1234567810123456782012345678301234567840123456249X"
        );
    LOGERR(klogSys, 0, a1); // 1) 1:PLAIN-NOT-TRUNCATED

    char a2[](
        "2:PLAIN-AND-TRUNCATED-3456783012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "12345678101234567820123456783012345678401234567250X"
        );
    LOGERR(klogSys, 0, a2); // 2) 2:PLAIN-AND-TRUNCATED

    char a3[](
        "3:ARG-NOT-TRUNCATED-123456783012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "12345678101234567820123456783012345678401234567250"
        "123456786012345X"
        );
    // 3) 3:ARG-NOT-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, "$(a)", "a=%s", a3));

    char a5[](
        "5:ARG-IS--TRUNCATED-123456783012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "12345678101234567820123456783012345678401234567250"
        "1234567860123456X"
        );
    // 4) 5:ARG--IS-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, "$(a)", "a=%s", a5));

#define A3 \
        "3:MSG-NOT-TRUNCATED-123456783012345678401234567850"\
        "12345678601234567870123456788012345678901234567100"\
        "12345678101234567820123456783012345678401234567150"\
        "12345678601234567870123456788012345678901234567200"\
        "1234567810123456782012345678301234567840123456249X"
    // 5) 3:MSG-NOT-TRUNCATED 3:ARG-NOT-TRUNCATED
   PLOGERR(klogSys, (klogSys, 0, A3 "$(a)", "a=%s", a3));

#define A4 \
        "4:MSG-IS--TRUNCATED-123456783012345678401234567850"\
        "12345678601234567870123456788012345678901234567100"\
        "12345678101234567820123456783012345678401234567150"\
        "12345678601234567870123456788012345678901234567200"\
        "12345678101234567820123456783012345678401234567250="
    // 6) 4:MSG-IS--TRUNCATED 3:ARG-NOT-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, A4 "$(a)", "a=%s", a3));

    // 7) 4:MSG-IS--TRUNCATED 5:ARG--IS-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, A4 "$(a)", "a=%s", a5));

    char a6[](
        "6:2-chars-ARG-NOT-TRUNCATED-3012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "12345678101234567820123456783012345678401234567250"
        "123456786012345X"
        );
    // 8) 4:MSGS-NOT-TRUNCATED 2-chars-ARGS-NOT-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, A3 "$(a1)" A3 "$(a2)",
        "a1=%s,a2=%s", a6, a6));

#define A5 \
        "5:MSG-IS--TRUNCATED-123456783012345678401234567850"\
        "12345678601234567870123456788012345678901234567100"\
        "12345678101234567820123456783012345678401234567150"\
        "12345678601234567870123456788012345678901234567200"\
        "12345678101234567820123456783012345678401234567250"\
        "12345678601234567870123456788012345678901234567300"\
        "12345678101234567820123456783012345678401234567350"\
        "12345678601234567870123456788012345678901234567400"\
        "12345678101234567820123456783012345678401234567550"
    char a7[](
        "6:3-chars-ARG-IS--TRUNCATED-3012345678401234567850"
        "12345678601234567870123456788012345678901234567100"
        "12345678101234567820123456783012345678401234567150"
        "12345678601234567870123456788012345678901234567200"
        "12345678101234567820123456783012345678401234567250"
        "12345678601234567870123456788012345678901234567300"
        "12345678101234567820123456783012345678401234567350"
        "12345678601234567870123456788012345678901234567400"
        "12345678101234567820123456783012345678401234567450"
        );
    // 9) LONG-MSGS-TRUNCATED LONG-ARGS-TRUNCATED
    PLOGERR(klogSys, (klogSys, 0, A5 "$(ab1)" A5 "$(ab2)",
        "ab1=%s,ab2=%s", a7, a7));

    /*(void)*/XMLLogger_Release(LOGGER);
    LOGGER = nullptr;

    std::ifstream file(XML_LOG);
    string line;

    getline(file, line);
    CHECK_EQ(line, string("<Log>"));

    getline(file, line); // 0) nothing truncated
    string e0("<system app=\"\" message=\"");
    string e(e0 + "123456789a=a1abcdefgh;23456789=A2ABCDEFGH");
    std::regex pattern("\" reason_short.*$");
    std::smatch matches;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 1) 1:PLAIN-NOT-TRUNCATED
    e = e0 + a1;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 2) 2:PLAIN-AND-TRUNCATED
    string truncated("... [ truncated ]");
    e = e0 + string(a2).substr(0, 250) + truncated;
    if (std::regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 3) 3:ARG-NOT-TRUNCATED
    e = e0 + a3;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line);// 4) 3:ARG-IS--TRUNCATED
    e = e0 + string(a5).substr(0, 250) + truncated;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 5) 3:MSG-NOT-TRUNCATED 3:ARG-NOT-TRUNCATED
    e = e0 + A3 + a3;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 6) 3:MSG-IS--TRUNCATED 3:ARG-NOT-TRUNCATED
    e = e0 + string(A4).substr(0, 250) + truncated + a3;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 7) 4:MSG-IS--TRUNCATED 5:ARG--IS-TRUNCATED
    e = e0 + string(A4).substr(0, 250) + truncated;
    e += string(a5).substr(0, 250) + truncated;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 8) 4:MSGS-NOT-TRUNCATED 2-chars-ARGS-NOT-TRUNCATED
    e = e0 + A3 + a6 + A3 + a6;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);

    getline(file, line); // 9) LONG-MSGS-TRUNCATED LONG-ARGS-TRUNCATED
    e = e0 + A3 + a6 + A3 + a6;
    e = e0 + string(A5).substr(0, 250) + truncated;
    e += string(a7).substr(0, 250) + truncated;
    e += string(A5).substr(0, 250) + truncated;
    e += string(a7).substr(0, 250) + truncated;
    if (regex_search(line, matches, pattern))
        e += matches[0];
    CHECK_EQ(line, e);
    
    getline(file, line);
    CHECK_EQ(line, string("</Log>"));

    CHECK(!getline(file, line));
}

int main(int argc, char* argv[]) {
    int r(TestSuite(argc, argv));
    std::remove(XML_LOG);
    return r;
}
