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

#ifndef _proc_on_h_
#define _proc_on_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * LYRICS:
 *
 * That file contains some functions which should be called on 
 * file system initialisation and destroying
 * 
 * We need that stuff, cuz, for example, I can not start thread before
 * it will daemonize, and ... I don't think that it is smart to 
 * pull the hair from all the code around. So ... introducing
 *
 *      XFSProcOnInit();        : Init ProcOn system
 *      XFSProcOnDispose();     : Dispose ProcOn system
 *
 *      XFSProcOnAddStart();    : Add OnStart proc to system
 *      XFSProcOnAddFinish();    : Add OnFinish proc to system
 *
 * IMPORTANT:
 * functions which set callpack procs are requires 'ProcName', Proc,
 * and pointer to Data. The copy of ProcName will be preserved,
 * and deleted on program exit. Proc itself and Data will be left 
 * unchanged. So, it is Your responsibility to clear these
 * Also, ProcName is not need to be unique ... just for logging purposes
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 // Defines
((*/
typedef rc_t ( CC * XFSPOProc ) ( const void * Data );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSProcOnInit ();
XFS_EXTERN rc_t CC XFSProcOnDispose ();

XFS_EXTERN rc_t CC XFSProcOnAddStart (
                                    const char * ProcName,
                                    XFSPOProc Proc,
                                    const void * ProcData,
                                    bool ExitOnError
                                    );

XFS_EXTERN rc_t CC XFSProcOnAddFinish (
                                    const char * ProcName,
                                    XFSPOProc Proc,
                                    const void * ProcData,
                                    bool ExitOnError
                                    );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _proc_on_h_ */
