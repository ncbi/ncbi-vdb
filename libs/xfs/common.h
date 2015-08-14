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

#ifndef _h_common_
#define _h_common_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   This file contains all unsorted common things
  ))))
 ((((*/

/*))    Flavors ... all kind
 ((*/
static const uint32_t _sFlavorLess                = 0;
static const uint32_t _sFlavorOfFoo               = 1;
static const uint32_t _sFlavorOfBogus             = 2;
static const uint32_t _sFlavorOfContNode          = 3;
static const uint32_t _sFlavorOfSimpleContainer   = 4;
static const uint32_t _sFlavorOfKfs               = 5;
static const uint32_t _sFlavorOfReadMe            = 6;
static const uint32_t _sFlavorOfKart              = 7;
static const uint32_t _sFlavorOfKartItem          = 8;
static const uint32_t _sFlavorOfKartCollection    = 9;
static const uint32_t _sFlavorOfHttp              = 10;
static const uint32_t _sFlavorOfTar               = 11;
static const uint32_t _sFlavorOfEncode            = 12;
static const uint32_t _sFlavorOfWorkspace         = 13;
static const uint32_t _sFlavorOfDbGapProject      = 14;
static const uint32_t _sFlavorOfDbGapProjectCache = 15;
static const uint32_t _sFlavorOfDbGapPublicCache  = 16;

/*))    Methods related to all commonly used nodes
 ((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* FileNode and DirNode - implemented in kfs.c                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
XFS_EXTERN rc_t CC XFSFileNodeMake (
                            struct XFSNode ** Node,
                            const char * Path,
                            const char * Name,
                            const char * Perm   /* Could be NULL */
                            );

XFS_EXTERN rc_t CC XFSDirNodeMake (
                            struct XFSNode ** Node,
                            const char * Path,
                            const char * Name,
                            const char * Perm   /* Could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* DocNode - implemented in docnode.c                                */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct XFSDoc;

XFS_EXTERN rc_t CC XFSDocNodeMake (
                            struct XFSNode ** Node,
                            const struct XFSDoc * Doc,
                            const char * Name,
                            const char * Perm       /* Could be NULL */
                            );

XFS_EXTERN rc_t CC XFSDocNodeMakeWithFlavor (
                            struct XFSNode ** Node,
                            const struct XFSDoc * Doc,
                            const char * Name,
                            const char * Perm,      /* Could be NULL */
                            uint32_t Flavor 
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* KartNode - implemented in kart.c                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSKartNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            const char * Path,
                            const char * Perm       /* Could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* KartCollectionNode - implemented in karts.c                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSKartCollectionNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            const char * Path,
                            const char * Perm       /* Could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* TarArchiveNode - implemented in tar.c                             */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSTarArchiveNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            const char * Path,
                            const char * Perm       /* Could be NULL */
                                                    /* Not used nos */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* EncryptedFileNode - implemented in encro.c                        */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSEncryptedFileNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            const char * Path,
                            const char * Passwd,
                            const char * EncType    /* could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* WorkspaceNode - implemented in encws.c                            */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSWorkspaceNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            const char * Path,
                            const char * Passwd,
                            const char * EncType    /* could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* DbGapProjectNode - implemented in karts.c                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSDbGapProjectNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,
                            uint32_t ProjectId,
                            bool ReadOnly,          /* RW if not */
                            const char * KartFilesCommaSeparated,
                            const char * Perm       /* Could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* DbGapCacheNode - implemented in gap_cache.c                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSDbGapCacheNodeMake (
                            struct XFSNode ** Node,
                            uint32_t ProjectId,     /* public if NULL */
                            bool ReadOnly,
                            const char * Name,      /* could be NULL */
                            const char * Perm       /* could be NULL */
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* DbGapKartsNode - implemented in gap_cache.c                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN rc_t CC XFSDbGapKartsNodeMake (
                            struct XFSNode ** Node,
                            const char * Name,      /* NOT NULL */
                            const char * ComSepPaths, /* may be NULL */
                            uint32_t ProjectId,     /* NOT NULL */
                            const char * Perm       /* could be NULL */
                            );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _h_common_ */
