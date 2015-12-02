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

#ifndef _h_xfs_defs_
#define _h_xfs_defs_

#include <klib/rc.h>

#ifndef _h_xfs_extern_
#include <xfs/extern.h>
#endif /* _h_xfs_extern_ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#define XFS_SIZE_8192       8192
#define XFS_SIZE_4096       4096
#define XFS_SIZE_2048       2048
#define XFS_SIZE_1024       1024
#define XFS_SIZE_512         512 
#define XFS_SIZE_128         128 
#define XFS_SIZE_64           64

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*  Another great masterpiece lol
 */
#define XFS_RC(State)   RC(rcFS, rcNoTarg, rcProcessing, rcNoObj, State)

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /* Something to do :
     *      XFS_CA   - XFS Check Arg
     *      XFS_CAN  - XFS Check Arg Null
     *      XFS_CSA  - XFS Check Set Arg
     *      XFS_CSAN - XFS Check Set Arg Null
     */

#define XFS_CA(Var,Value)    if ( ( Var ) == Value ) { return XFS_RC ( rcInvalid ); }

#define XFS_CAN(Var)         if ( ( Var ) == NULL ) { return XFS_RC ( rcNull ); }

#define XFS_CSA(Var,Value)   if ( ( Var ) != NULL ) { * ( Var ) = Value; }

#define XFS_CSAN(Var)        if ( ( Var ) != NULL ) { * ( Var ) = NULL; }

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*\
 *  That enum is used for opening files mode only. I'll put it here
\*/
typedef enum XFSNMode {
    kxfsNone  = 00,
    kxfsRead  = 01,
    kxfsWrite = 02,
    kxfsReadWrite = kxfsRead | kxfsWrite
} XFSNMode;

/*\
 *  All types of file XFS recognises as good standing
\*/
typedef enum XFSNType {
    kxfsNotFound = 0,
    kxfsBadPath,
    kxfsFile,
    kxfsDir,
    kxfsLink
} XFSNType;

/*\
 *  All authority types recognised by XFS on good will
\*/
typedef enum XFSAType {
    kxfsUser = 0,
    kxfsGroup,
    kxfsOther
} XFSAType;

/*\
 *  Wery useful enum used for describing state of object
\*/
typedef enum XFSStatus {
    kxfsInvalid = 0,
    kxfsReady,
    kxfsComplete,
    kxfsGood = kxfsComplete,    /* kinda Good and Complete are same */
    kxfsBroken
} XFSStatus;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _h_xfs_defs_ */
