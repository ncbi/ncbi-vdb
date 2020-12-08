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
* Unit tests for KRamFile
*/
#include <cstring>
#include <ktst/unit_test.hpp>
#include <kfs/ramfile.h>
#include <kfs/impl.h>
#include <klib/out.h>
#include <kapp/args.h>
#include <kfg/config.h>

using namespace std;

TEST_SUITE(RamFileTestSuite);

TEST_CASE(KRamFileWrite_simple_write)
{
    char input [4 * 1024];

    for (size_t ix = 0; ix < sizeof input;  ++ ix)
        input [ix] = (char)(' ' + (char)(ix & 0x3F));

    char buffer[4 * 1024 + 1]; // extra for NUL for display purposes

    KFile * wfile;


    for (size_t pos = 0; pos < sizeof buffer /* including the +1 */; pos += 100) // skipping a bunch since its working
    {
        for (size_t inc = 1; inc < sizeof (input); inc += 250) // faster since its working
        {
            for (size_t bsize = 0; bsize <= sizeof input + 1; bsize += inc)
            {
                size_t num_writ;

                memset (buffer, 0, sizeof buffer);

                REQUIRE_RC(KRamFileMakeWrite (&wfile, buffer, sizeof buffer - 1));
                REQUIRE_RC(KFileWrite(wfile, pos, input, bsize, &num_writ));
                REQUIRE(((bsize<sizeof(input))?bsize:sizeof(input))==num_writ);
                REQUIRE(memcmp(buffer,input,num_writ) == 0);
                REQUIRE_RC(KFileRelease(wfile));
            }
        }
    }

}


TEST_CASE(KramFileWrite_append_write)
{
    char input [4 * 1024];

    for (size_t ix = 0; ix < sizeof input;  ++ ix)
        input [ix] = (char)(' ' + (char)(ix & 0x3F));

    char buffer[4 * 1024 + 1]; // extra for NUL for display purposes

    KFile * wfile;

    for (size_t pos = 0; pos < sizeof buffer - 1; pos += 200)
    {
        for (size_t bsize = 1; bsize < sizeof buffer - 1; bsize += 200)
        {
            uint64_t tot_writ = 0;
            size_t num_writ;

            REQUIRE_RC(KRamFileMakeWrite (&wfile, buffer, sizeof buffer - 1));
            memset (buffer, 0, sizeof buffer);

            for (size_t off = 0; tot_writ + bsize < sizeof buffer - 1; off += num_writ)
            {
                REQUIRE_RC (KFileWrite(wfile, tot_writ, input + off, bsize, &num_writ));
                REQUIRE(num_writ == bsize);
                tot_writ += num_writ;

                REQUIRE(memcmp(input,buffer,tot_writ) == 0);
            }

            REQUIRE_RC(KFileRelease(wfile));
        }
    }
}

TEST_CASE(KRamFileWrite_oversized_writes)
{
    char input [16 * 1024];

    for (size_t ix = 0; ix < sizeof input;  ++ ix)
        input [ix] = (char)(' ' + (char)(ix & 0x3F));

    char buffer[4 * 1024 + 1];

    KFile * wfile;

    for (size_t max = 1; max <= sizeof buffer - 1; max += 200)
    {
        memset (buffer, 0, sizeof buffer);
        REQUIRE_RC(KRamFileMakeWrite (&wfile, buffer, max));

        for (size_t bsize = (max+1)/2; bsize < sizeof input; bsize+= 150)
        {
            size_t num_writ;

            REQUIRE_RC (KFileWrite(wfile, 0, input, bsize, &num_writ));
            if (bsize > max)
                REQUIRE(num_writ == max);
            else
                REQUIRE(num_writ == bsize);

            REQUIRE(0 == memcmp(buffer, input, num_writ));
            REQUIRE(0 != memcmp(buffer, input, num_writ+1));
        }
        REQUIRE_RC(KFileRelease(wfile));
    }
}


TEST_CASE(KRamFileWrite_shift_right)
{
    char input [16 * 1024];

    for (size_t ix = 0; ix < sizeof input;  ++ ix)
        input [ix] = (char)(' ' + (char)(ix & 0x3F));

    char buffer[4 * 1024];

    size_t num_writ;

    KFile * wfile;

    for (size_t bsize = 1; bsize < sizeof buffer; ++bsize)
    {
        REQUIRE_RC(KRamFileMakeWrite (&wfile, buffer, sizeof buffer - 1));

        REQUIRE_RC(KFileWrite(wfile, 0, input, bsize, &num_writ));
        REQUIRE(bsize = num_writ);

        REQUIRE_RC(KFileRelease(wfile));
    }

}

//////////////////////////////////////////// Main

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

rc_t CC UsageSummary (const char * prog_name)
{
    return 0;
}

rc_t CC Usage ( const Args * args)
{
    return 0;
}

const char UsageDefaultName[] = "test-kfg";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=RamFileTestSuite(argc, argv);
    return rc;
}

}
