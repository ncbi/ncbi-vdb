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

typedef struct KRemoveFileTask KRemoveFileTask;
#define KTASK_IMPL KRemoveFileTask

#include <kproc/task.h>
#include <kproc/impl.h>
#include <kfs/extern.h>
#include <kfs/directory.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KRemoveFileTask
 */
struct KRemoveFileTask
{
    KTask dad;
    KDirectory * dir;
    char path [ 1 ];
};

static
rc_t CC KRemoveFileTaskWhack ( KRemoveFileTask *self )
{
    rc_t rc = KDirectoryRelease ( self -> dir );
    KTaskDestroy ( & self -> dad, "KRemoveFileTask" );
    free ( self );
    return rc;
}

static
rc_t CC KRemoveFileTaskExecute ( KRemoveFileTask *self )
{
    return KDirectoryRemove ( self -> dir, true, "%s", self -> path );
}

static
KTask_vt_v1 KRemoveFileTask_vt =
{
    1, 0,
    KRemoveFileTaskWhack,
    KRemoveFileTaskExecute
};

LIB_EXPORT rc_t CC KRemoveFileTaskVMake ( KTask ** task,
    KDirectory * dir, const char * _path, va_list args )
{
    rc_t rc;
    if ( task == NULL )
        rc = RC ( rcFS, rcFunction, rcConstructing, rcParam, rcNull );
    else
    {
        char full [ 4096 ];
        rc = KDirectoryVResolvePath ( dir, true, full, sizeof full, _path, args );
        if ( rc == 0 )
        {
            size_t path_size = string_size ( full );
            KRemoveFileTask *t = malloc ( sizeof * t + path_size );
            if ( t == NULL )
                rc = RC ( rcFS, rcFunction, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KTaskInit ( & t -> dad, ( const KTask_vt * ) & KRemoveFileTask_vt, "KRemoveFileTask", full );
                if ( rc == 0 )
                {
                    rc = KDirectoryAddRef ( t -> dir = dir );
                    if ( rc == 0 )
                    {
                        strcpy ( t -> path, full );
                        * task = & t -> dad;
                        return 0;
                    }
                    
                    KTaskDestroy ( & t -> dad, "KRemoveFileTask" );
                }
                
                free ( t );
            }
        }

        * task = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KRemoveFileTaskMake ( KTask ** task,
    KDirectory * dir, const char * fmt, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, fmt );
    rc = KRemoveFileTaskVMake ( task, dir, fmt, args );
    va_end ( args );

    return rc;
}
