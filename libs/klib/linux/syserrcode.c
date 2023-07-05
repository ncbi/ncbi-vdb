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

#if _POSIX_C_SOURCE < 200112L
#define _POSIX_C_SOURCE 200112L
#endif

#if (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
#else
  #if ! _GNU_SOURCE
    /* non-XSI clang? strerror_r is not declared probably */
    #include <stdlib.h> /* size_t */
    char *strerror_r(int errnum, char *buf, size_t buflen);
  #else
    /* strerror_r is declared by the system headers */
  #endif
#endif

#include <errno.h>

#include <klib/extern.h>
#include "writer-priv.h"
#include <klib/writer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <os-native.h> /* for strchrnul on non-linux */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>


#if (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
size_t KWrtFmt_error_code ( char * buffer, size_t buffer_size, uint32_t error_code )
{
    int res = strerror_r ((int)error_code, buffer, buffer_size);

    if ( res == 0 )
    {
        return string_size (buffer);
    }

    if ( res == EINVAL )
    {
        snprintf( buffer, buffer_size, "The value of errnum (%d) is not a valid error number", (int)error_code );
    }
    if ( res == ERANGE )
    {
        snprintf( buffer, buffer_size, "Insufficient storage was supplied to contain the error description string: buffer=%p, buffer_size=%ld", buffer, buffer_size );
    }

    return string_size (buffer);
}
#else
size_t KWrtFmt_error_code ( char * buffer, size_t buffer_size, uint32_t error_code )
{
    char * pc;

    pc = strerror_r ((int)error_code, buffer, buffer_size);

    /* the glibc guys are sorta strange */
    if (pc != buffer)
    {
        string_copy_measure (buffer, buffer_size, pc);
        buffer[buffer_size-1] = '\0';
    }
    return string_size (buffer);
}
#endif
