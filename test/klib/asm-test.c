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

#include <arch-impl.h>

static
rc_t run ( uint32_t x, uint32_t expected_result )
{
    int32_t lsb = uint32_lsbit(x);
    if (lsb != expected_result)
    {
        KOutMsg ( "uint32_lsbit(%u) failed, expected %u, actual %u\n", x, expected_result, lsb );
        return -1;
    }
    return 0;
}

/* KMain
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc == 0 )
    {
        rc = run (8, 3);
        ArgsWhack ( args );
    }

    return rc;
}
