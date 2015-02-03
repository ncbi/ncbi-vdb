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

#ifndef _doc_h_
#define _doc_h_

#include <klib/refcount.h>

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Forwards and others
 */
struct KRefcount;
struct KFile;

struct XFSDoc;
union XFSDoc_vt;

/******************************************************************
 * Another awkward structure which should represent memory buffer
 * with file interface. Will be used for implementing virtual files
 * like README.txt, etc ...
 * There are two general interfaces :
 *      XFSDoc - buffer
 *      XFSDocFile - file
 * Also, we are providing some generic TEXT Doc functionality
 ******************************************************************/

/*))    Just a buffer interface and some methods
 ((*/
struct XFSDoc_vt_v1 {
        /* version for pension */
    uint32_t maj;
    uint32_t min;

    rc_t ( CC * dispose ) ( struct XFSDoc * self );

    rc_t ( CC * get ) ( const struct XFSDoc * self, const char ** B );
    rc_t ( CC * size ) ( const struct XFSDoc * self, uint64_t * S ); 
};

union XFSDoc_vt {
    struct XFSDoc_vt_v1 v1;
};

XFS_EXTERN rc_t CC XFSDocInit (
                            const struct XFSDoc * self,
                            const union XFSDoc_vt * VT
                            );
XFS_EXTERN rc_t CC XFSDocDispose (
                            const struct XFSDoc * self
                            );
XFS_EXTERN rc_t CC XFSDocAddRef (
                            const struct XFSDoc * self
                            );
XFS_EXTERN rc_t CC XFSDocRelease (
                            const struct XFSDoc * self
                            );
XFS_EXTERN rc_t CC XFSDocGet (
                            const struct XFSDoc * self,
                            const char ** B
                            );
XFS_EXTERN rc_t CC XFSDocSize (
                            const struct XFSDoc * self,
                            uint64_t * Size
                            );

/*))    Just a file to make. Name is OK to be NULL
 ((*/
XFS_EXTERN rc_t CC XFSDocFileMake (
                            const char * Name,      /* Ok to be NULL */
                            const struct XFSDoc * Doc,
                            struct KFile ** File
                            );

/*))    Usefull TextDoc interface
 ((*/
XFS_EXTERN rc_t CC XFSTextDocMake (
                            struct XFSDoc ** TextDoc
                            );
XFS_EXTERN rc_t CC XFSTextDocAppend (
                            struct XFSDoc * self,
                            const char * Fmt,
                            ...
                            );
XFS_EXTERN rc_t CC XFSTextDocVAppend (
                            struct XFSDoc * self,
                            const char * Fmt,
                            va_list args
                            );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _doc_h_ */
