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

#include <kapp/main.h>
#include <kapp/args.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>

#include <assert.h>
#include <stdio.h>

ver_t CC KAppVersion ( void )
{
    return 0;
}

const char UsageDefaultName[] = "append-arg-test";

rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg ( "\n"
                     "Usage:\n"
                     "  %s [Options]\n"
                     "\n"
                     "Summary:\n"
                     "  Test append argument. [-h for help]\n"
                     , progname
        );
}

rc_t CC Usage ( const Args *args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);
    if (rc)
        progname = fullpath = UsageDefaultName;

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard ();
    HelpVersion ( fullpath, KAppVersion () );

    return rc;
}


rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc;
    bool Jojoba = false;

    if ( argc != 2 && argc != 1 ) {
        if ( argc == 3 && strcmp ( argv [ 2 ], "jojoba" ) == 0 ) {
            Jojoba = true;
        }
        else {
            printf ( "INVALID ARGUMENTS\n" );
            UsageSummary ( UsageDefaultName );
            return 22;
        }
    }

    rc = ArgsMakeStandardOptions ( & args );
    if ( rc == 0 ) {
        if ( ! Jojoba ) {
            rc = ArgsAddAppendModeOption ( args );
        }
        if ( rc == 0 ) {
            rc = ArgsParse ( args, argc, argv );
            if ( rc == 0 ) {
                ArgsHandleHelp ( args );

                rc = ArgsHandleAppendMode ( args );
                if ( rc == 0 ) {
                    printf (
                            "APPEND_MODE: %s\n",
                            ( ArgsIsAppendModeSet () ? "Y" : "N" )
                            );
                }
            }
        }

        ArgsWhack ( args );
    }

    return rc;
}
