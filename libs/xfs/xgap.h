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
 * That file contains methods which allow to access dbGaP data
 * 
 * The core of that celebrations stored in hidden structure XFSGap
 * which is not accessible to user, and may be inied/disposed with
 * calling appropriate method: XFSGapInit/XFSGapDispose
 *
 * XFSGap structure has three most important objects, which could be
 * accessed:
 *
 *     Projecs
 *     Karts
 *     GapObjects
 *
 * Project is represented by XFSGapProject structure, instance of which
 * could be retrieved by ProjectId ( integer, greater than zero ). User
 * could refer to PUBLIC data by using ProjectId = XFS_PUBLIC_PROJECT_ID
 * ( 0 ). Using GapProject.
 * user could access following information: repository location, name,
 * description, encryption key and password for project data
 * There are no Init/Dispose methods for GapProject, so Release object
 * after using
 *
 * Kart are represented by XFSGapCart structure, which contains name,
 * and path to physical cart location. It also contains set of KartItem
 * which contains ObjecId ( accessin or object id ) of real item,
 * project Id, which thatt item belongs to. Cart could be accepted
 * by name, and list names could be retrieved from XFSGap instance
 *
 * GapObjects are represented by XFSGapObject, and it contains: name, 
 * ObjectId ( string: accession or id ), remote url, cache path,
 * modified time, size and md5. GapObjecte could be retrieved by name
 * only. There is no way to receive full list of objects :MWAHAHAH:
 *
 * Looks like all those are thread safe, not sure yet :LOL:
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 // Defines
((*/
#define XFS_PUBLIC_PROJECT_ID   0

/*))
 // Forwards
((*/
struct XFSGapProject;
struct XFSGapKart;
struct XFSGapObject;
struct KKey;

/* JOJOBA? struct KKey;  */
/* JOJOBA? struct VPath; */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 ///    XFSGap - hidden marvel LOL
(((*/

/*  You should remember that XFSGap is a singleton, so if You will call
 *  Init() or Dispose() method twice, You will have some problems :LOL:
 *  so, be carefoul
 */
XFS_EXTERN rc_t CC XFSGapInit ();
XFS_EXTERN rc_t CC XFSGapDispose ();

/*)))
 ///    Accessing different data from XFSGap
(((*/

/*
 *  Those methods will initialize pointer to some structures. Please,
 *  don't free them. Use corresponding Release method ... thanks
 */
XFS_EXTERN rc_t CC XFSGapGetProject (
                                const struct XFSGapProject ** Project,
                                uint32_t ProjectId
                                );

XFS_EXTERN bool CC XFSGapHasProject ( uint32_t ProjectId );

    /* Don't forget to free Kartfiles, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapKartfiles ( const char ** Kartfiles );

    /* Don't forget to free Publicfiles, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapPublicfiles ( const char ** PublicFiles );

    /*  The Kart parameter could be NULL, in that case kart will be
     *  loaded, but will not be returned
     *  Note, path is for loading only, karts are differ by name
     */
XFS_EXTERN rc_t CC XFSGapLoadKart (
                                const struct XFSGapKart ** Kart,
                                const char * KartDir,
                                const char * KartName
                                );

XFS_EXTERN rc_t CC XFSGapGetKart (
                                const struct XFSGapKart ** Kart,
                                const char * KartName
                                );

XFS_EXTERN bool CC XFSGapHasKart ( const char * KartName );

        /*  That one will return list of kart names
         */
XFS_EXTERN rc_t CC XFSGapListKarts ( struct KNamelist ** List );

        /*  That one will return list of kart names for ProjectId
         */
XFS_EXTERN rc_t CC XFSGapListKartsForProject (
                                            struct KNamelist ** List,
                                            uint32_t ProjectId
                                            );

XFS_EXTERN rc_t CC XFSGapRefreshKarts ();

        /*  GapObjects are distints by AccessionOrId ...
         */
XFS_EXTERN rc_t CC XFSGapGetObject (
                                const struct XFSGapObject ** Object,
                                const char * AccessionOrId
                                );

XFS_EXTERN bool CC XFSGapHasObject ( const char * AccessionOrId );

    /*  That method will reload all known resources
     */
XFS_EXTERN rc_t CC XFSGapRehash ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSGapProject
((*/

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

    /* No need to free RepositoryName, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryName (
                            const struct XFSGapProject * self,
                            const char ** RepositoryName
                            );

    /* No need to free DisplayName, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryDisplayName (
                            const struct XFSGapProject * self,
                            const char ** DislplayName
                            );

    /* No need to free RepositoryRoot, cuz it is part of a system
     */
XFS_EXTERN rc_t CC XFSGapProjectRepositoryRoot (
                            const struct XFSGapProject * self,
                            const char ** RepositoryRoot
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
                            const char ** Workspace
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSGapKart
((*/
XFS_EXTERN rc_t CC XFSGapKartAddRef ( const struct XFSGapKart * self );

XFS_EXTERN rc_t CC XFSGapKartRelease ( const struct XFSGapKart * self );

XFS_EXTERN rc_t CC XFSGapKartList (
                                const struct XFSGapKart * self,
                                struct KNamelist ** List
                                );

XFS_EXTERN rc_t CC XFSGapKartListForProject (
                                const struct XFSGapKart * self,
                                struct KNamelist ** List,
                                uint32_t ProjectId
                                );

                /*  Returns the name of directory, where kart file is
                 */
XFS_EXTERN rc_t CC XFSGapKartDirectory (
                                const struct XFSGapKart * self,
                                const char ** Dir
                                );

                /*  Returns the name of file, where kart is stored
                 */
XFS_EXTERN rc_t CC XFSGapKartName (
                                const struct XFSGapKart * self,
                                const char ** Name
                                );

                /*  Returns the name of file, where kart is stored
                 */
XFS_EXTERN rc_t CC XFSGapKartPath (
                                const struct XFSGapKart * self,
                                const char ** Path
                                );

XFS_EXTERN bool CC XFSGapKartHasDataForProject (
                                const struct XFSGapKart * self,
                                uint32_t ProjectId
                                );

    /*  It allocates AccessionOrId, so don't forget to free it
     */
XFS_EXTERN rc_t CC XFSGapKartItemAttributes (
                                const struct XFSGapKart * self,
                                uint32_t * ProjectId,
                                char ** AccessionOrId,
                                const char * ItemName
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     XFSGapObject
((*/
XFS_EXTERN rc_t CC XFSGapObjectAddRef (
                                const struct XFSGapObject * self
                                );

XFS_EXTERN rc_t CC XFSGapObjectRelease (
                                const struct XFSGapObject * self
                                );

XFS_EXTERN rc_t CC XFSGapObjectId (
                                const struct XFSGapObject * self,
                                const char ** AccessionOrId
                                );

XFS_EXTERN rc_t CC XFSGapObjectURL (
                                const struct XFSGapObject * self,
                                const char ** Url
                                );

XFS_EXTERN rc_t CC XFSGapObjectLocalPath (
                                const struct XFSGapObject * self,
                                const char ** LocalPath
                                );

XFS_EXTERN rc_t CC XFSGapObjectModTime (
                                const struct XFSGapObject * self,
                                KTime_t * ModTime
                                );

XFS_EXTERN rc_t CC XFSGapObjectSize (
                                const struct XFSGapObject * self,
                                uint64_t * Size
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xgap_h_ */
