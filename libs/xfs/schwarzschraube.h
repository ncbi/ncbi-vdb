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

#ifndef _schwarzschraube_
#define _schwarzschraube_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*) Fowards, and others
 (*/

/*))))
  ||||   Unsorted methods which usage was approved by usage
  ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct String;

/*)))   Dupping string, and if NULL is Src, it will be passed to Dst
 (((*/
XFS_EXTERN rc_t CC XFS_StrDup ( const char * Src,  const char ** Dst );

/*)))   Dupping String, and if NULL is Src, it will be passed to Dst
 (((*/
XFS_EXTERN rc_t CC XFS_SStrDup (
                                const struct String * Src,
                                const char ** Dst
                                );

/*)))   Checking if string ends with string
 (((*/
XFS_EXTERN rc_t CC XFS_StrEndsWith (
                                const char * Str, const char * End
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Sometimes I need VFSManager, and from different places, and
  |||   it is very costly to make on Windows platform
  (((*/
struct VFSManager;

XFS_EXTERN rc_t CC XFS_VfsManagerInit ( );
XFS_EXTERN rc_t CC XFS_VfsManagerDispose ( );

XFS_EXTERN const struct VFSManager * CC XFS_VfsManager ( );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Sometimes I need KNSManager, and from different places, and
  |||   it is costly too
  (((*/
struct KNSManager;

XFS_EXTERN rc_t CC XFS_KnsManagerInit ( );
XFS_EXTERN rc_t CC XFS_KnsManagerDispose ( );

XFS_EXTERN const struct KNSManager * CC XFS_KnsManager ( );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Just resolving absolute/relative path. Need it sometime
  |||
  (((*/
XFS_EXTERN rc_t CC XFS_ResolvePath (
                                bool Absolute,
                                char * Resolved,
                                size_t ResolvedSize,
                                const char * Format,
                                ...
                                );
XFS_EXTERN rc_t CC XFS_VResolvePath (
                                bool Absolute,
                                char * Resolved,
                                size_t ResolvedSize,
                                const char * Format,
                                va_list Args
                                );

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _schwarzschraube_ */
