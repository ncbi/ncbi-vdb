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

#ifndef _perm_h_
#define _perm_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics: XFSPerm - permissions for filesystem objects
 ||
 ||   The permissions are stored as string in format :
 ||
 ||         rwxrwxrwx[ user[:group[:other]]]
 ||         oooggguuu
 ||
 ||   Where first mandatary part is standard unix permissions string,
 ||   and first three characters are read-write-execute permitions for
 ||   others, second three characters are read-write-execute permissions
 ||   for group, and third three characters are read-write-execute
 ||   permissions for user. The rest of a string is a triplet for
 ||   of names for 'user', 'group' and 'other' ( which is for windows )
 ||
 ||   BTW that primitive is storeing data in char format, no gid or uid
 ||   because it should work well on different platforms. The final 
 ||   transformation to native format will be implemented on each 
 ||   platform.
 ||
 ||   So, righot now there is XFSPerm structure, which contains three
 ||   XFSAuth structures which represents user/group/other
 ||
((*/

/*)))
 ///   And other forwards.
(((*/

struct XFSPerm;
struct XFSAuth;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   XFSPerm make/dispose
 ((*/

/*))
 //    Currently used types
(*/

/*))
 //    Parses permission string and makes XFSPerm 
(*/
XFS_EXTERN rc_t CC XFSPermMake (
                            const char * PermAsString,
                            const struct XFSPerm ** Permissions
                            );

/*))
 //    Disposes XFSPerm
(*/
XFS_EXTERN rc_t CC XFSPermDispose (
                            const struct XFSPerm * self
                            );

/*))
 //    Returns Authority by it's type
(*/
XFS_EXTERN const struct XFSAuth * CC XFSPermAuth (
                            const struct XFSPerm * self,
                            XFSAType Type
                            );


/*))
 //    Generates string which represents XFSPerm object
(*/
XFS_EXTERN rc_t CC XFSPermToString (
                            const struct XFSPerm * self,
                            char * Buffer,
                            size_t BufferSize
                            );

/*))
 //    Some defaults Don't yet know for what ... to make configurable ?
((*/
XFS_EXTERN const char * CC XFSPermDefaultName ( XFSAType Type );
XFS_EXTERN rc_t CC XFSPermSetDefaultName (
                            XFSAType Type,
                            const char * Name
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   XFSAuth
 ((*/

/*))
 //     Returns name of authority, which could be defaulted
((*/
XFS_EXTERN const char * CC XFSAuthName ( const struct XFSAuth * self );

/*))
 //     These three methods will return permissions for authority
((*/
XFS_EXTERN bool CC XFSAuthCanRead ( const struct XFSAuth * self );
XFS_EXTERN bool CC XFSAuthCanWrite ( const struct XFSAuth * self );
XFS_EXTERN bool CC XFSAuthCanExecute ( const struct XFSAuth * self );

/*))
 //     That method will convert XFSPerm to unix like access uint32_t
((*/
XFS_EXTERN rc_t CC XFSPermToNum (
                                    const char * Perm,
                                    uint32_t * Access
                                    );

XFS_EXTERN rc_t CC XFSPermToChar (
                                    uint32_t Access,
                                    char * Buf,
                                    size_t BufSize
                                    );

/*))
 //     Some usefull defaults ... not sure
((*/
XFS_EXTERN const char * CC XFSPermRODefContChar ();
XFS_EXTERN const char * CC XFSPermRODefNodeChar ();
XFS_EXTERN uint32_t CC XFSPermRODefContNum ();
XFS_EXTERN uint32_t CC XFSPermRODefNodeNum ();

XFS_EXTERN const char * CC XFSPermRWDefContChar ();
XFS_EXTERN const char * CC XFSPermRWDefNodeChar ();
XFS_EXTERN uint32_t CC XFSPermRWDefContNum ();
XFS_EXTERN uint32_t CC XFSPermRWDefNodeNum ();

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _perm_h_ */
