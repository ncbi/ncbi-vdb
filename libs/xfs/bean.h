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

#ifndef _bean_h_
#define _bean_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There is some interface which allows to access list of objects,
  ))))  which are mostly names and resources. Called it XFSBeans and
 ((((   XFSBeanSack.
  ))))  User allowed to edit BeanSack to use it for dbGaP Cache
 ((((   
  ))))  WARNING: thread unsafe operations, required blocking
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSBeanSack;
struct KNamelist;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSBean                                                           */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSBean;

XFS_EXTERN rc_t CC XFSBeanMake ( 
                                const struct XFSBean ** Bean,
                                const char * Name,
                                const char * Resource,
                                uint64_t Version
                                );

XFS_EXTERN rc_t CC XFSBeanDispose ( const struct XFSBean * self );

XFS_EXTERN rc_t CC XFSBeanAddRef ( const struct XFSBean * self );

XFS_EXTERN rc_t CC XFSBeanRelease ( const struct XFSBean * self );

XFS_EXTERN rc_t CC XFSBeanName ( 
                                const struct XFSBean * self,
                                const char ** Name
                                );

XFS_EXTERN rc_t CC XFSBeanResource ( 
                                const struct XFSBean * self,
                                const char ** Resource
                                );

XFS_EXTERN rc_t CC XFSBeanVersion ( 
                                const struct XFSBean * self,
                                uint64_t * Version
                                );


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSBeanSack                                                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSBeanSack;

    /*)) Default value for InitialVersion is 1, so if You will pass 0
     //  as InitialVersion, it will be set to 1
    ((*/
XFS_EXTERN rc_t CC XFSBeanSackMake (
                                const struct XFSBeanSack ** Sack,
                                uint64_t InitialVersion
                                );

XFS_EXTERN rc_t CC XFSBeanSackDispose (
                                const struct XFSBeanSack * self
                                );

XFS_EXTERN rc_t CC XFSBeanSackAddRef (
                                const struct XFSBeanSack * self
                                );

XFS_EXTERN rc_t CC XFSBeanSackRelease (
                                const struct XFSBeanSack * self
                                );

XFS_EXTERN rc_t CC XFSBeanSackNames (
                                const struct XFSBeanSack * self,
                                const struct KNamelist ** Names
                                );

XFS_EXTERN rc_t CC XFSBeanSackVersion (
                                const struct XFSBeanSack * self,
                                uint64_t * Version
                                );

XFS_EXTERN rc_t CC XFSBeanSackSetVersion (
                                const struct XFSBeanSack * self,
                                uint64_t Version
                                );

        /* Don't forget to Release bean after processing
         */
XFS_EXTERN rc_t CC XFSBeanSackFind (
                                const struct XFSBeanSack * self,
                                const struct XFSBean ** Bean,
                                const char * Name
                                );

        /* Don't forget to Release bean after adding
         */
XFS_EXTERN rc_t CC XFSBeanSackAdd (
                                const struct XFSBeanSack * self,
                                const struct XFSBean * Bean
                                );

XFS_EXTERN rc_t CC XFSBeanSackDel (
                                const struct XFSBeanSack * self,
                                const struct XFSBean * Bean
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSBeanSuite                                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSNode;
struct XFSBeanSackDepot;

struct XFSBeanSuite {
    const union XFSBeanSuite_vt * vt;

    KRefcount refcount;

    const struct XFSBeanSackDepot * depot;
};

struct XFSBeanSuite_vt_v1 {
    uint32_t maj, min;

        /* Destructor */
    rc_t ( CC * dispose ) ( const struct XFSBeanSuite * self );

        /* Renders XFSBean to XFSNode  by it's name */
    rc_t ( CC * render ) (
                        const struct XFSBeanSuite * self, 
                        const char * Name,
                        const struct XFSNode ** Node
                        );

        /* Refresh content of XFSBeanSackDepot */
    rc_t ( CC * refresh ) (
                        const struct XFSBeanSuite * self
                        );

        /* Ulink content of XFSBeanSackDepot */
    rc_t ( CC * unlink ) (
                        const struct XFSBeanSuite * self, 
                        const char * Name
                        );
};

union XFSBeanSuite_vt {
    struct XFSBeanSuite_vt_v1 v1;
};

XFS_EXTERN rc_t CC XFSBeanSuiteInit (
                            const struct XFSBeanSuite * self,
                            const union XFSBeanSuite_vt * VTable
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteDispose (
                            const struct XFSBeanSuite * self
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteAddRef (
                            const struct XFSBeanSuite * self
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteRelease (
                            const struct XFSBeanSuite * self
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteFind (
                            const struct XFSBeanSuite * self,
                            const char * Name,
                            const struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteRefresh (
                            const struct XFSBeanSuite * self
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteUnlink (
                            const struct XFSBeanSuite * self,
                            const char * Name
                            );

    /* Please, release Sack after using
     */
XFS_EXTERN rc_t CC XFSBeanSuiteGetSack (
                            const struct XFSBeanSuite * self,
                            const struct XFSBeanSack ** Sack
                            );

    /* Please, release Sack after setting
     */
XFS_EXTERN rc_t CC XFSBeanSuiteSetSack (
                            const struct XFSBeanSuite * self,
                            const struct XFSBeanSack * Sack
                            );

XFS_EXTERN rc_t CC XFSBeanSuiteVersion (
                            const struct XFSBeanSuite * self,
                            uint64_t * Version
                            );

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _bean_h_ */
