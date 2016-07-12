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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <klib/rc.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <klib/namelist.h>
#include <klib/log.h>
#include <klib/out.h>
#include <kapp/main.h>
#include <kapp/args.h>

#include <assert.h>

#if 0
void add_to_file (KFile * file, KFile * md5, uint64_t * pos)
{
    rc_t	rc;
    uint64_t	word;
    size_t	written;

    word = *pos;

    rc = KFileWrite (file, word, &word, sizeof (word), &written);

    assert (rc == 0);

    pos = word + written;
}
#endif

rc_t	runtest (KDirectory * kdir,const char *  filename)
{
    uint64_t		iteration = 0;
    size_t		written;
    KFile *		file;
    KFile *		md5sumfile;
    KMD5File *  md5file;
    KMD5SumFmt *	md5sumfmt;
    char *		md5filename;
    rc_t		rc = 0;
    char		c;
    char		buff [16];

    OUTMSG (("allocate memory for name %s.md5\n", filename));
    md5filename = malloc (strlen(filename) + 5);
    if (md5filename == NULL)
    {
        OUTMSG (("failure to allocate for md5 filename\n"));
        return -1;
    }
    strcpy (md5filename, filename);
    strcat (md5filename, ".md5");
    OUTMSG (("create md5filename is %s\n",md5filename));


    OUTMSG (("create %s\n", filename));
    rc = KDirectoryCreateFile (kdir, &file, true, 0777, kcmInit, filename);
    if (rc != 0)
    {
        OUTMSG (("failure to open %s\n", filename));
        return rc;
    }

    rc = KDirectoryCreateFile (kdir, &md5sumfile, true, 0777, kcmInit, md5filename);
    if (rc != 0)
    {
        OUTMSG (("failure to create %s\n", md5filename));
        return rc;
    }

    OUTMSG (("make KMD5SumFmtUpdate\n"));
    rc = KMD5SumFmtMakeUpdate (&md5sumfmt, md5sumfile);
    if (rc != 0)
    {
        OUTMSG (("unable to make KMD5SumFmtUpdate %d\n", rc));
        return rc;
    }

    OUTMSG (("make Md5 for write\n"));
    rc = KMD5FileMakeAppend ( & md5file, file, md5sumfmt, filename );
    if (rc != 0)
    {
        OUTMSG (("unable to create KFileMakeMd5Append %d\n", rc));
        return rc;
    }
    file = KMD5FileToKFile ( md5file );
    
    c = ' ';
    memset (buff, c, sizeof (buff));
    OUTMSG (("write to file %lu\n", iteration));
    rc = KFileWrite (file, iteration * sizeof (buff), &buff, sizeof (buff), &written);
    if (rc != 0)
    {
        OUTMSG (("unable to write first iteration\n"));
        return rc;
    }

    OUTMSG (("%c %lu\n", buff[0], written));

    /* close all files */
    rc = KFileRelease (file);
    if (rc != 0)
    {
        OUTMSG (("unable to close md5file %d\n", rc));
        return rc;
    }
    rc = KMD5SumFmtRelease (md5sumfmt);
    if (rc != 0)
    {
        OUTMSG (("unable to close md5 formatter: %d\n", rc));
        return rc;
    }
    
#if 1
    for (iteration = 1; iteration < 1024; ++iteration)
    {
        rc = KDirectoryOpenFileWrite (kdir, &file, true, filename);
        if (rc)
            PLOGERR (klogFatal, (klogFatal, rc, "Couldn't open %s", filename));

        else
        {
            rc = KDirectoryOpenFileWrite (kdir, &md5sumfile, true, md5filename);
            if (rc)
                PLOGERR (klogFatal, (klogFatal, rc, "Couldn't open %s", md5filename));

            else
            {
                rc = KMD5SumFmtMakeUpdate (&md5sumfmt, md5sumfile);
                if (rc)
                    LOGERR (klogFatal, rc, "Couldn't make md5sumfmt");

                else
                {
                    rc = KMD5FileMakeAppend (&md5file, file, md5sumfmt, filename);
                    if (rc)
                        LOGERR (klogFatal, rc, "unable to make KFileMakeMd5Append");

                    else
                    {
                        c = (c == '~') ? ' ' : c+1;

                        memset (buff, c, sizeof (buff));
                        OUTMSG (("%c\n",buff[0]));

                        file = KMD5FileToKFile ( md5file );
                        rc = KFileWrite (file, iteration * sizeof (buff), &buff, sizeof (buff), &written);
                        if (rc)
                            LOGERR (klogFatal, rc, "failed to write");
                        assert (written == sizeof (buff));
                        KFileRelease (KMD5FileToKFile (md5file));
                    }
                    KMD5SumFmtRelease (md5sumfmt);
                }
            }
/*             rc = KFileRelease (file); */
        }
    }
#endif
    return rc;
}

uint32_t CC KAppVersion ( void )
{
    return  0;
}

const char UsageDefaultName[] = "md5appendtest";

rc_t CC UsageSummary (const char * name)
{
    return KOutMsg (
        "Usage:\n"
        " %s <file>\n"
        "\n"
        "    run a test of the KMD5File in append mode\n"
        "\n", name);
}


rc_t CC Usage ( const Args * args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);
    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard();
 
    return rc;
}


rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    rc_t rc;

    rc = ArgsMakeAndHandle (&args, argc, argv, 0);
    if (rc == 0)
    {
        uint32_t pcount;
        const char * filename;
        KDirectory * kdir;

        do
        {
            rc = ArgsParamCount (args, &pcount);
            if (rc)
                break;
            if (pcount != 1)
            {
                MiniUsage (args);
                break;
            }

            rc = ArgsParamValue (args, 0, (const void **)&filename);
            if (rc)
                break;

            /* -----
             * Mount the native filesystem as root
             */
            rc = KDirectoryNativeDir(&kdir);
            if (rc)
                break;

            rc = runtest (kdir, filename);
            KDirectoryRelease (kdir);
        } while (0);
    }
    return rc;
}




