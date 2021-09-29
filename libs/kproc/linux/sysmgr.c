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

#include <unistd.h>

#if !defined(_GNU_SOURCE)

#define NEED_GET_TID 1

#elif defined(__GLIBC__) && (__GLIBC__ < 2|| (__GLIBC__ == 2 && __GLIBC_MINOR__ < 30))

#define NEED_GET_TID 1

#endif

#if NEED_GET_TID
#include <sys/syscall.h>

#define GET_TID ( syscall ( SYS_gettid ) )

#else

#define GET_TID ( gettid() )

#endif

static __thread bool have_tid, on_main_thread;

/* OnMainThread
 *  returns true if running on main thread
 */
LIB_EXPORT bool CC KProcMgrOnMainThread ( void )
{
    if ( ! have_tid )
    {
        on_main_thread = GET_TID == getpid ();
        have_tid = true;
    }
    return on_main_thread;
}

uint32_t sys_GetPID ( void )
{
    return getpid ();
}


int sys_GetHostName ( char * buffer, size_t buffer_size )
{
    return gethostname( buffer, buffer_size );
}
