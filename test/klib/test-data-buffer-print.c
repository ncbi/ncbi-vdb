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
#include <klib/printf.h>
#include <klib/data-buffer.h>
#include <klib/symbol.h>
#include <klib/text.h>
#include <klib/time.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <kfg/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <va_copy.h>
#include <time.h>

static
rc_t run ( const char *progname )
{
    rc_t rc = 0;
    
    char const *const cicero = "Sed ut perspiciatis, unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt, explicabo. Nemo enim ipsam voluptatem, quia voluptas sit, aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos, qui ratione voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum, quia dolor sit amet consectetur adipisci[ng]velit, sed quia non-numquam [do] eius modi tempora inci[di]dunt, ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum[d] exercitationem ullam corporis suscipitlaboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit, qui inea voluptate velit esse, quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla pariatur?";
    size_t const orig_length = strlen(cicero);
    int const replicount = (5000 + orig_length - 1) / orig_length; /* allocation is in 4k chunks; pick a repeat count so that the buffer has to grow at least once */
    size_t const final_length = (orig_length + 1) * replicount + 1;
    KDataBuffer buffer;
    int i;
    
    memset(&buffer, 0, sizeof buffer);
    for (i = 0; i < replicount; ++i) {
        rc = KDataBufferPrintf(&buffer, "%s", cicero);
        if (rc) return rc;
        if (((char const *)buffer.base)[buffer.elem_count - 1] != '\0') {
            rc = RC ( rcExe, rcString, rcFormatting, rcData, rcIncorrect );

            /* report discrepancy */
            pLogErr ( klogErr, rc, "KDataBufferPrintf result is not nul terminated, or length is incorrect", "");
            return rc;
        }
        if (i == 0) {
            if (buffer.elem_count != orig_length + 1) {
                rc = RC ( rcExe, rcString, rcFormatting, rcData, rcIncorrect );

                /* report discrepancy */
                pLogErr ( klogErr, rc, "KDataBufferPrintf result differs from expected; length = $(length), expected $(orig)", "length=%u,orig=%u", (unsigned)buffer.elem_count, (unsigned)orig_length + 1);
                return rc;
            }
        }
        rc = KDataBufferResize(&buffer, buffer.elem_count + 1);
        if (rc == 0) {
            char *base = buffer.base;
            base[buffer.elem_count - 2] = '\n';
            base[buffer.elem_count - 1] = '\0';
        }
        else
            return rc;
    }
    if (buffer.elem_count != final_length) {
        rc = RC ( rcExe, rcString, rcFormatting, rcData, rcIncorrect );

        /* report discrepancy */
        pLogErr ( klogErr, rc, "KDataBufferPrintf result differs from expected; length = $(length), expected $(orig)", "length=%u,orig=%u", (unsigned)buffer.elem_count, (unsigned)final_length);
        return rc;
    }
    KDataBufferWhack(&buffer);
    return rc;
}


/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release 
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void )
{
    return 0;
}


/* Usage
 *  This function is called when the command line argument
 *  handling sees -? -h or --help
 */
rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg (
        "\n"
        "Usage:\n"
        "  %s [Options]\n"
        "\n"
        "Summary:\n"
        "  Simple test of printf.\n"
        , progname );
}

const char UsageDefaultName[] = "time-data-buffer-print";

rc_t CC Usage ( const Args *args )
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

    HelpVersion (fullpath, KAppVersion());

    return rc;
}

    
/* KMain
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc == 0 )
    {
        KConfigDisableUserSettings();
        rc = run ( argv [ 0 ] );
        ArgsWhack ( args );
    }

    return rc;
}
