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

#include <simple/extern.h>

#include "sra-exception.h"
#include "sra-object-priv.h"
#include "sra-string.h"

#include "vdb-mgr.h"

#include <vdb/manager.h>
#include <kdb/manager.h>
#include <klib/rc.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * SRAObject
 */


/* MakeFromSpec
 *  uses "spec" to locate and open object
 *  creates an SRAObject from it
 *  uses static internal manager to allocate return object
 */
LIB_EXPORT SRAObject* CC SRAObjectMakeFromSpec ( SRAException *x, const SRAString *spec )
{
    rc_t rc;
    const VDBMgr *mgr;

    assert ( x != NULL );
    assert ( spec != NULL );
    assert ( spec -> addr != NULL );
    assert ( spec -> size != 0 );

    mgr = VDBMgrMakeRead ( x );
    if ( mgr != NULL )
    {
        /* test the type of thing we have */
        switch ( VDBManagerPathType ( mgr, "%.*s", ( int ) spec -> size, spec -> addr ) & ~ kptAlias )
        {
        case kptNotFound:
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcNotFound );
            break;
        case kptBadPath:
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcInvalid );
            break;

        case kptFile:
            /* we only open SRA files at the moment, but this would be
               the place to catch references to other file types. */
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcIncorrect );
            break;

        case kptDir:
        case kptCharDev:
        case kptBlockDev:
        case kptFIFO:
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcIncorrect );
            break;

        case kptZombieFile:
            /* opening a missing or incomplete file within a kar archive */
            rc = RC ( rcSRA, rcFile, rcOpening, rcFile, rcIncomplete );
            break;

        case kptDataset:
        case kptDatatype:
            /* opening something within hdf5 */
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcIncorrect );
            break;

        case kptDatabase:
            /* opening a cSRA object, possibly PacBio or other
               compound SRA object, or could be WGS, etc. */
            return VDBMgrMakeDatabase ( mgr, x, spec );

        case kptTable:
            /* opening an SRA object or possibly a refseq */
            return VDBMgrMakeTable ( mgr, x, spec );

        case kptIndex:
        case kptColumn:
        case kptMetadata:
            /* these types of objects aren't directly supported under the simple API */
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcUnsupported );
            break;

        case kptPrereleaseTbl:
            /* there are still a very few of these around */
            return VDBMgrMakeTable ( mgr, x, spec );

        default:
            rc = RC ( rcSRA, rcFile, rcOpening, rcPath, rcUnrecognized );
        }

        SRAExceptionError ( x, 0, rc, "failed to open '%.*s'", ( uint32_t ) spec -> size, spec -> addr );
    }

    return NULL;
}

/* MakeConcatenation
 *  creates an SRAObject from the concatenation of two existing objects
 *  uses static internal manager to allocate return object
 */
LIB_EXPORT SRAObject* CC SRAObjectMakeConcatenation ( SRAException *x, const SRAObject *a, const SRAObject *b )
{
    SRAExceptionError ( x, 0, -1, "not yet writ" );
    return NULL;
}
