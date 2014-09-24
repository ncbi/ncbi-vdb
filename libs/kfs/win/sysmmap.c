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

#include <kfs/extern.h>
#include "sysmmap-priv.h"
#include "sysfile-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <WINDOWS.H>


/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */

/* Make
 */
rc_t KMMapMake ( KMMap **mmp )
{
    SYSTEM_INFO sinfo;

    KMMap *mm = calloc ( 1, sizeof * mm );
    if ( mm == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );

    GetSystemInfo ( & sinfo );
    mm -> pg_size = sinfo . dwAllocationGranularity;

    * mmp = mm;

    return 0;
}


/* RWSys
 */
rc_t KMMapRWSys ( KMMap *self, uint64_t pos, size_t size )
{
    DWORD status;

    KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );
    if ( sf == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );

    self -> handle = CreateFileMapping ( sf -> handle, NULL, PAGE_READWRITE, 0, 0, NULL );
    if ( self -> handle != NULL )
    {
        self -> addr = MapViewOfFile ( self -> handle, FILE_MAP_ALL_ACCESS, 
            ( DWORD ) ( pos >> 32 ), ( DWORD ) pos, size );
        if ( self -> addr != NULL )
            return 0;
    }

    switch ( status = GetLastError () )
    {
    /* NEED ERROR CODES - THANKS, REDMOND!! */
    default:
        break;
    }

    return RC ( rcFS, rcMemMap, rcConstructing, rcNoObj, rcUnknown );
}


/* ROSys
 */
rc_t KMMapROSys ( KMMap *self, uint64_t pos, size_t size )
{
    DWORD status;

    KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );
    if ( sf == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );

    self -> handle = CreateFileMapping ( sf -> handle, NULL, PAGE_READONLY, 0, 0, NULL );
    if ( self -> handle != NULL )
    {
        self -> addr = MapViewOfFile ( self -> handle, FILE_MAP_READ, 
            ( DWORD ) ( pos >> 32 ), ( DWORD ) pos, size );
        if ( self -> addr != NULL )
            return 0;
    }

    switch ( status = GetLastError () )
    {
    /* NEED ERROR CODES - THANKS, REDMOND!! */
    default:
        break;
    }

    return RC ( rcFS, rcMemMap, rcConstructing, rcNoObj, rcUnknown );
}


/* Unmap
 *  removes a memory map
 */
rc_t KMMapUnmap ( KMMap *self )
{
    if ( self -> handle != NULL )
    {
        if ( ! CloseHandle ( self -> handle ) )
            return RC ( rcFS, rcMemMap, rcDestroying, rcNoObj, rcUnknown );
        if ( ! UnmapViewOfFile( self -> addr ) )
            return RC ( rcFS, rcMemMap, rcDestroying, rcNoObj, rcUnknown );

        self -> addr = NULL;
        self -> size = 0;
        self -> handle = NULL;
    }

    return 0;
}
