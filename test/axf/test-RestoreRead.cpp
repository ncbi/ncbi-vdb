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

#include <ktst/unit_test.hpp>
#include <klib/log.h>
#include <vdb/manager.h>
#include <vdb/table.h>

extern "C" {
#include "../../libs/axf/restore-read.h"
unsigned RestoreReadShared_getState(unsigned *refSeqs, unsigned *wgs, unsigned *errors, unsigned *activeRefSeqs);
}

#include <chrono>
#include <thread>

TEST_SUITE(RestoreReadSuite);

class RR_Fixture
{
public:
    VDBManager const *mgr;
    std::string const seqid = "NC_000001.11";
    uint8_t read[5000];
    
    RR_Fixture()
    : mgr(0)
    {
        auto const rc = VDBManagerMakeRead(&mgr, 0);
        assert(rc == 0);
    }
    
    RestoreRead *make() {
        rc_t rc = 0;
        RestoreRead *rr = RestoreReadMake(mgr, &rc);
        assert(rc == 0);
        return rr;
    }
    void free(RestoreRead *rr) {
        RestoreReadFree(rr);
    }
    unsigned sequence(RestoreRead *rr, unsigned start) {
        unsigned nread = 0;
        auto const rc = RestoreReadGetSequence(rr, start, 5000, read, seqid.size(), seqid.c_str(), &nread, nullptr);
        assert(rc == 0);
        return nread;
    }
    
    struct State {
        unsigned refSeqs, wgs, errors, activeRefSeqs;
        bool isActive;
    };
    State status() {
        State result = {};
        auto active = RestoreReadShared_getState(&result.refSeqs, &result.wgs, &result.errors, &result.activeRefSeqs);
        result.isActive = active > 0;
        return result;
    }
};

FIXTURE_TEST_CASE(ReadAllAndClose, RR_Fixture)
{
    {
        auto const &rr = make();
        unsigned pos = 0;
        unsigned nread = 0;
        bool done = false;
    
        REQUIRE_EQ(5000u, nread = sequence(rr, pos));
        {
            auto const &state = status();
            REQUIRE(state.isActive);
            REQUIRE_EQ(state.refSeqs, 1u);
            REQUIRE_EQ(state.activeRefSeqs, 1u);
            REQUIRE_EQ(state.wgs, 0u);
            REQUIRE_EQ(state.errors, 0u);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pos += nread;
        while ((nread = sequence(rr, pos)) > 0) {
            pos += nread;
            done = false;
            {
                auto const &state = status();
                if (!state.isActive)
                    break;
                if (state.activeRefSeqs == 0)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            done = true;
        }
        if (!done) {
            LOGMSG(klogDebug, "Continuing without waiting");
            while ((nread = sequence(rr, pos)) > 0) {
                pos += nread;
            }
            for ( ; ; ) {
                auto const &state = status();
                if (!state.isActive)
                    break;
                if (state.activeRefSeqs == 0)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        free(rr);
    }
}

FIXTURE_TEST_CASE(ReadSomeAndClose, RR_Fixture)
{
    {
        auto const &rr = make();
    
        REQUIRE_EQ(5000u, sequence(rr, 0));
        {
            auto const &state = status();
            REQUIRE(state.isActive);
            REQUIRE_EQ(state.refSeqs, 1u);
            REQUIRE_EQ(state.wgs, 0u);
            REQUIRE_EQ(state.errors, 0u);
        }    
    
        free(rr);
    }
    {
        auto const &state = status();
        REQUIRE(!state.isActive);
    }    
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

const char UsageDefaultName[] = "test-RestoreRead";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    // KLogLevelSet(klogDebug);
    rc_t rc = RestoreReadSuite(argc, argv);
    return rc;
}

}
