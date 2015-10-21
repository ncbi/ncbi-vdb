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

#ifndef _xgap_h_
#define _xgap_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some thing which suppose to be a temporary stuff
  ))))  of viewer for dbGaP projects
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * LYRICS:
 *
 * That file contains methods which allow to access to dbGaP project
 * data, as for :
 * 
 *      Directories
 *      Files
 *      Passwords
 *
 * XFSGapProject - dbGap Project defined by unique ProjectId (uint32_t)
 *    1) does not have Make/Dispose methods - should be created by
 *       Gap's FindOrCreate method, and Released after use
 *    2) provides methods: Root directory, Key for ProjectId
 *    3) provides access to Project Data by Accession or ObjectId
 *       ( opens and decrypts files )
 *    3) thread safe
 *    4) struct itself hidden
 *
 * XFSGap - retrieves or creates dbGaP Project by ProjectId
 *    1) provides access to dbGaP Project data by ProjectID
 *    3) thread safe.
 *
 * There is special ProjectId = XFS_PUBLIC_PROJECT_ID ( 0 ), which is
 * for 'public' data
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     Forwards
((*/
struct XFSGapProject;
struct KKey;
struct VPath;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     XFSGapInit/XFSGapDispose
 \\     We are keepin' single instance of depot per program. These 
 //     methods should be called at the program begining and end.
((*/
XFS_EXTERN rc_t CC XFSGapInit ();
XFS_EXTERN rc_t CC XFSGapDispose ();

    /*  Don't know if we need that method. Added for a sake of sake
     *  Return true if Project with such ID exists
     */
XFS_EXTERN bool CC XFSGapHas ( uint32_t ProjectId );

    /*  Usint method FindOrCreate, to avoid collisions from different
     *  creative people :lol:
     */
XFS_EXTERN rc_t CC XFSGapFindOrCreate (
                                uint32_t ProjectId,
                                const struct XFSGapProject ** Project
                                );

    /*  Don't know if we need that method. Added for a sake of sake
     */
XFS_EXTERN rc_t CC XFSGapRemove ( uint32_t ProjectId );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSGapProject
((*/

#define XFS_PUBLIC_PROJECT_ID   0

XFS_EXTERN rc_t CC XFSGapProjectAddRef (
                            const struct XFSGapProject * self
                            );
XFS_EXTERN rc_t CC XFSGapProjectRelease (
                            const struct XFSGapProject * self
                            );

XFS_EXTERN bool CC XFSGapPfojectIsValid (
                            const struct XFSGapProject * self
                            );

XFS_EXTERN rc_t CC XFSGapProjectId (
                            const struct XFSGapProject * self,
                            uint32_t * Id
                            );

    /* Both Remote and Cache could be NULL, so no data will be returned
     * To dispose pahts, You should release them after using
     */
XFS_EXTERN rc_t CC XFSGapProjectLocateAccession (
                            const struct XFSGapProject * self,
                            const char * Accession,
                            const struct VPath ** Remote,
                            const struct VPath ** Cache
                            );

    /* Both Remote and Cache could be NULL, so no data will be returned
     * To dispose pahts, You should release them after using
     */
XFS_EXTERN rc_t CC XFSGapProjectLocateObject (
                            const struct XFSGapProject * self,
                            uint32_t ObjectId,
                            const struct VPath ** Remote,
                            const struct VPath ** Cache
                            );

    /*  Both Remote and Cache could be NULL, and don't forget to free
     *  them 
     */
XFS_EXTERN rc_t CC XFSGapProjectAccessionUrlAndPath (
                            const struct XFSGapProject * self,
                            const char * Accession,
                            const char ** RemoteUrl,
                            const char ** CachePath
                            );

    /*  Both Remote and Cache could be NULL, and don't forget to free
     *  them 
     */
XFS_EXTERN rc_t CC XFSGapProjectObjectUrlAndPath (
                            const struct XFSGapProject * self,
                            uint32_t ObjectId,
                            const char ** RemoteUrl,
                            const char ** CachePath
                            );

    /* Don't forget to free RepositoryName, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryName (
                            const struct XFSGapProject * self,
                            char ** RepositoryName
                            );

    /* Don't forget to free DislplayName, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryDisplayName (
                            const struct XFSGapProject * self,
                            char ** DislplayName
                            );

    /* Don't forget to free RepositoryRoot, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryRoot (
                            const struct XFSGapProject * self,
                            char ** RepositoryRoot
                            );

    /* No need to free Key, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectEncriptionKey (
                            const struct XFSGapProject * self,
                            const struct KKey ** EncriptionKey
                            );

    /* No need to free Password, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectPassword (
                            const struct XFSGapProject * self,
                            const char ** Password,
                            size_t * PasswordSize
                            );

    /* Don't forget to free Workspace, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectWorkspace (
                            const struct XFSGapProject * self,
                            char ** Workspace
                            );

/*)))   Those do not need ProjectId
 (((*/
    /* Don't forget to free Kartfiles, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapKartfiles ( char ** Kartfiles );

    /* Don't forget to free Publicfiles, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapPublicfiles ( char ** PublicFiles );

/*)))   For those, who are lame. Apparently, there should be a
 |||    method in kfg/config.h for that ... but ...
(((*/
    /* Don't forget to free UserHome, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapUserHome ( char ** UserHome );

    /* Don't forget to free UserConfigDir, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapUserConfigDir ( char ** UserConfigDir );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xgap_h_ */
