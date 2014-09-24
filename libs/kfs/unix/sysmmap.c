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

#include "sysmmap-priv.h"
#include "sysfile-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>


/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */

/* Make
 */
rc_t KMMapMake ( KMMap **mmp )
{
    KMMap *mm = calloc ( 1, sizeof * mm );
    if ( mm == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );
    
    mm -> pg_size = sysconf ( _SC_PAGE_SIZE );

    * mmp = mm;

    return 0;
}


/* RWSys
 */
rc_t KMMapRWSys ( KMMap *self, uint64_t pos, size_t size )
{
    KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );
    if ( sf == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );

    self -> addr = mmap ( 0, size,
        PROT_READ | PROT_WRITE, MAP_SHARED, sf -> fd, pos );
    if ( self -> addr != ( char* ) MAP_FAILED )
        return 0;

    switch ( errno )
    {
    case EBADF:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFileDesc, rcInvalid );
    case EACCES:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );
    case ETXTBSY:
    case ENODEV:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcInvalid );
    case EAGAIN:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFunction, rcIncomplete );
    case ENOMEM:
        return RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );
    }

    return RC ( rcFS, rcMemMap, rcConstructing, rcNoObj, rcUnknown );
}


/* ROSys
 */
rc_t KMMapROSys ( KMMap *self, uint64_t pos, size_t size )
{
    KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );
    if ( sf == NULL )
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );

    self -> addr = mmap ( 0, size,
        PROT_READ, MAP_SHARED, sf -> fd, pos );
    if ( self -> addr != ( char* ) MAP_FAILED )
        return 0;

    switch ( errno )
    {
    case EBADF:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFileDesc, rcInvalid );
    case EACCES:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );
    case ETXTBSY:
    case ENODEV:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcInvalid );
    case EAGAIN:
        return RC ( rcFS, rcMemMap, rcConstructing, rcFunction, rcIncomplete );
    case ENOMEM:
        return RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );
    }

    return RC ( rcFS, rcMemMap, rcConstructing, rcNoObj, rcUnknown );
}


/* Unmap
 *  removes a memory map
 */
rc_t KMMapUnmap ( KMMap *self )
{
    if ( self -> size != 0 )
    {
        if ( munmap ( self -> addr - self -> addr_adj,
                 self -> size + self -> size_adj ) )
        {
            if ( errno != EINVAL )
                return RC ( rcFS, rcMemMap, rcDestroying, rcNoObj, rcUnknown );
        }

        self -> addr = NULL;
        self -> size = 0;
    }

    return 0;
}
