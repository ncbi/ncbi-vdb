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

#define XFS_SIZE_4096       4096
#define XFS_SIZE_2048       2048
#define XFS_SIZE_1024       1024
#define XFS_SIZE_512         512 
#define XFS_SIZE_128         128 

/*  That is a part of conspiracy, if You do not like too verbose
 *  programms, comment that part
 */
#define XFS_EXTENDED_LOGGING

#ifdef XFS_EXTENDED_LOGGING
    #define XFSMSG(msg) OUTMSG(msg)
#else   /* XFS_EXTENDED_LOGGING */
    #define XFSMSG(msg)
#endif  /* XFS_EXTENDED_LOGGING */

/*  Another great masterpiece lol
 */
#define XFS_RC(State)   RC(rcFS, rcNoTarg, rcProcessing, rcNoObj, State)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _h_xfs_defs_ */
