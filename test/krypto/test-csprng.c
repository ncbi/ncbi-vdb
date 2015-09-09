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
#include <krypto/rng.h>

#include <kapp/main.h>
#include <kapp/args.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/status.h>

/* #include <kfs/directory.h> */
/* #include <kfs/file.h> */

/* #include <kxml/xml.h> */

/* #include <string.h> */

/*
 * not intended to be pretty.
 * not intended to be slick.
 * not intended to make tools like valgrind happy.
 *
 * This does not REALLY test the quality of the RNG; it's just
 * a development test and a "did I break it" test.
 *
 * It just spews lines of random bytes.  These could be gathered
 * and analyzed with another tool such as the one from NIST.  Perhaps
 * a bit of change to the output format might be needed for that.
 */

static
rc_t run()
{
    KRng * rng;
    rc_t rc;

    rc = KCSPRngMake (&rng);
    if (rc)
        LOGERR (klogErr, rc, "failed to make RNG");
    else
    {
        uint32_t ix;

        OUTMSG (("KCSPRng with no seed\n"));
        
        for (ix = 0; ix < 10; ++ix)
        {
            uint8_t buff [8];
            uint32_t num_read;

            rc = KRngRead (rng, buff, sizeof (buff), &num_read);
            if (rc)
                PLOGERR (klogErr,
                         (klogErr, rc, "error in read '$u' from KRng",
                          "u=%lu", ix));
            else
            {
                OUTMSG (( "%4.4d: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                          ix,
                          buff[0], buff[1], buff[2], buff[3],
                          buff[4], buff[5], buff[6], buff[7]));
            }
        }

        KRngRelease (rng);

        rc = KCSPRngMake (&rng);
        if (rc)
            LOGERR (klogErr, rc, "failed to make RNG");
        else
        {
            uint32_t ix;


            /* this is using system seed from us */
            rc = KRngSeed (rng);
            if (rc)
                LOGERR (klogErr, rc, "error running KRngSeed");
            else
            {
                OUTMSG (("KCSPRng with seed\n"));
        
                for (ix = 0; ix < 10; ++ix)
                {
                    uint8_t buff [8];
                    uint32_t num_read;

                    rc = KRngRead (rng, buff, sizeof (buff), &num_read);
                    if (rc)
                        PLOGERR (klogErr,
                                 (klogErr, rc, "error in read '$u' from KRng",
                                  "u=%lu", ix));
                    else
                    {
                        OUTMSG (( "%4.4d: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                                  ix,
                                  buff[0], buff[1], buff[2], buff[3],
                                  buff[4], buff[5], buff[6], buff[7]));
                    }
                }
                {
                    uint8_t reseed [1024];

                    reseed[0] = 0xAA;

                    for (ix = 1; ix < sizeof (reseed); ++ix)
                        reseed[ix] = reseed[ix-1] ^ (uint8_t)ix;

                    /* this is using reseed from us */
                    rc = KRngReseed (rng, reseed, sizeof (reseed));
                    if (rc)
                        LOGERR (klogErr, rc, "error running KRngReseed");
                    else
                    {
                        OUTMSG (("KCSPRng with reseed\n"));

                        for (ix = 0; ix < 10; ++ix)
                        {
                            uint8_t buff [8];
                            uint32_t num_read;

                            rc = KRngRead (rng, buff, sizeof (buff), &num_read);
                            if (rc)
                                PLOGERR (klogErr,
                                         (klogErr, rc, "error in read '$u' from KRng",
                                          "u=%lu", ix));
                            else
                            {
                                OUTMSG (( "%4.4d: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                                          ix,
                                          buff[0], buff[1], buff[2], buff[3],
                                          buff[4], buff[5], buff[6], buff[7]));
                            }
                        }
                    }        
                }
            }        
            KRngRelease (rng);
        }
    }
    return 0;
}

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg (
        "\n"
        "Usage:\n"
        "  %s [OPTIONS]\n"
        "\n"
        "Summary:\n"
        "  Test the KCSPRng type.\n",
        progname);
}


const char UsageDefaultName [] = "test-csprng";
rc_t CC Usage (const Args * args)
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcExe, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard();

    HelpVersion (fullpath, KAppVersion());

    return rc;
}


ver_t CC KAppVersion (void)
{
    return 0;
}
rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    rc_t rc;

    rc = ArgsMakeAndHandle (&args, argc, argv, 0);
    if (rc == 0)
    {

        rc = run();
        if (rc)
            LOGERR (klogErr, rc, "Exiting failure");
        else
            STSMSG (0, ("Exiting okay\n"));
    }

    if (rc)
        LOGERR (klogErr, rc, "Exiting status");
    else
        STSMSG (0, ("Exiting status (%R)\n", rc));
    return rc;
}
