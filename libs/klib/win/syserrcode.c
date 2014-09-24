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

#include <klib/extern.h>
#include "writer-priv.h"
#include <klib/writer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <os-native.h>
#include <Windows.h>

#include <stdio.h>

size_t KWrtFmt_error_code ( char * buffer, size_t buffer_size, uint32_t error_code )
{
    char  * tbuffer;
    char * pc;
    DWORD tbuffer_size;
    size_t z;

    tbuffer_size = FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                   FORMAT_MESSAGE_FROM_SYSTEM | 
                                   FORMAT_MESSAGE_IGNORE_INSERTS, /* dwFlags */
                                   NULL, /* lpSource */
                                   error_code, /* dwMessageId */
                                   0, /* dwLanguageId - we're lazy on language right now */
                                   (LPSTR)&tbuffer,
                                   0, /* nSize */
                                   NULL);/* Arguments */
    if (tbuffer_size == 0)
    {
        rc_t rc;

        rc = string_printf (buffer, buffer_size, &z,
                            "Undefined error: %u", error_code);
        if (rc == 0)
            return z;
        else
        {
            static const char failed[] = "Failure to get Windows error string";
            return string_copy (buffer, buffer_size, failed, sizeof failed);
        }
    }
    else
    {
        for (pc = tbuffer; *pc ; ++pc)
        {
            if (*pc == '\r')
            {
                tbuffer_size = ( DWORD ) ( pc - tbuffer );
                break;
            }
        }
        z = string_copy (buffer, buffer_size, tbuffer, tbuffer_size);

        LocalFree (tbuffer);
    }
    return z;
}
