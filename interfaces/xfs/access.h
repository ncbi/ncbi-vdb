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

#ifndef _access_h_
#define _access_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   Since we are going to do AWS image with dbGaP data which
  ))))  will be shared between many users, we need simple way to allow
 ((((   or deny access to whole or part of data. So, that is all about
  ))))
 ((((   XFSAccess interface introduced for these goals
  ))))
 ((((   XFSAccess is a set of Agents. Each agent has unique
  ))))  name and set of policies. Access object is loaded and
 ((((   could be accessible/editable by 'USER WHO STARTED FUSER'
  ))))
 ((((   Right now Accessibility represented by file, where allowed
  ))))  users are listed, each on separate line.
 (((( 
  ))))  The approval methods are requiring 'User' name 'Group' name
 ((((   obtaining those is platform dependent.
  ))))
 ((((   Access object is a Singleton ... and should be treated poperly
  ))))
 ((((*/

/*))
 //  Forwards
((*/
struct XFSAccessAgent;

/*))))
 ((((   Here we are ... Access
  ))))
 ((((*/

XFS_EXTERN rc_t CC XFSAccessInit ( const char * Path );
/*
 *  ProjectId = 0, it get default access list at ~/.ncbi/dbGaP.acl
 *  ProjectId != 0, it get list from ~/.ncbi/dbGaP-####.acl
 *  if both files aren't exits, it creates default Access, which will
 *  be stored
 */
XFS_EXTERN rc_t CC XFSAccessInit4Gap ( uint32_t ProjectId );

XFS_EXTERN rc_t CC XFSAccessDispose ();

/*
 *  Methods for "approving" access
 */
XFS_EXTERN bool CC XFSAccessApprove (
                            const char * User,
                            const char * Group,
                            const char * Path
                            );

/*
 *  Methods for accessing Accessibility Agents
 */
XFS_EXTERN rc_t CC XFSAccessList ( struct KNamelist ** List );
XFS_EXTERN bool CC XFSAccessHas ( const char * AgentName );
XFS_EXTERN rc_t CC XFSAccessGet (
                            const char * AgentName,
                            const struct XFSAccessAgent ** Agent
                            );
    /*  Agent could be NULL ... */
XFS_EXTERN rc_t CC XFSAccessAdd (
                            const char * AgentName,
                            const struct XFSAccessAgent ** Agent
                            );
XFS_EXTERN rc_t CC XFSAccessDel ( const char * AgentName );

    /*
     *  Agent methods. It has only one "approve" method now,
     *  but it will add edit methods later
     */
     /* Group could be NULL */
XFS_EXTERN bool CC XFSAccessAgentApprove (
                            const struct XFSAccessAgent * self,
                            const char * User,
                            const char * Group,
                            const char * Path
                            );


/*))))
 ((((   Here are some special methods
  ))))
 ((((*/
 /*
  * those two are platform dependent and should be defined somewhereelse
  * You are responsible for freeing that memory
  */
XFS_EXTERN rc_t XFSOwnerUserName ( const char ** Name );
XFS_EXTERN rc_t XFSOwnerGroupName ( const char ** Name );
XFS_EXTERN bool XFSDoesUserExist ( const char * Name );

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _access_h_ */
