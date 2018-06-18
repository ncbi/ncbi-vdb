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

#include <kproc/extern.h>
#include <kproc/procmgr.h>

/* OnMainThread
 *  returns true if running on main thread
 */
LIB_EXPORT bool CC KProcMgrOnMainThread ( void )
{
    /* don't know how to do this on Winders */
    return false;
}

uint32_t sys_GetPID ( void )
{
/* Enable Windows-32 bits build */
#if defined(_WIN32)
    return 0;
#else
    return ( uint32_t) GetCurrentProcessId();
#endif
}

int sys_GetHostName ( char * buffer, size_t buffer_size )
{
/* Enable Windows-32 bits build */
#if defined(_WIN32)
    if ( buffer_size > 0 && buffer != NULL )
        buffer [ 0 ] = '\0';
    return 0;
#else
    return gethostname( buffer, buffer_size );
#endif
}
