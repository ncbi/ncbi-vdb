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

#ifndef _path_h_
#define _path_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   another and hope more successifull attempt to create
  ))))  good usable interface to a Path
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  Universal method which should load config from resource
 ((*/

struct XFSPath;

XFS_EXTERN rc_t CC XFSPathMake (
                                const struct XFSPath ** Out,
                                bool AddPrecedingSlash,
                                const char * Format,
                                ...
                                );

XFS_EXTERN rc_t CC XFSPathVMake (
                                const struct XFSPath ** Out,
                                bool AddPrecedingSlash,
                                const char * Format,
                                va_list Args
                                );

    /*)) These two methods are requireing KDirectory,
     //  and I am not sure if those should be here ... later
    ((*/
XFS_EXTERN rc_t CC XFSPathMakeAbsolute (
                                const struct XFSPath ** Out,
                                bool AddPrecedingSlash,
                                const char * Format,
                                ...
                                );

XFS_EXTERN rc_t CC XFSPathVMakeAbsolute (
                                const struct XFSPath ** Out,
                                bool AddPrecedingSlash,
                                const char * Format,
                                va_list Args
                                );

XFS_EXTERN rc_t CC XFSPathDup (
                                const struct XFSPath * In,
                                const struct XFSPath ** Out
                                );

XFS_EXTERN rc_t CC XFSPathAddRef ( const struct XFSPath * self );
XFS_EXTERN rc_t CC XFSPathRelease ( const struct XFSPath * self );


XFS_EXTERN rc_t CC XFSPathSet (
                                const struct XFSPath * self,
                                bool AddPrecedingSlash,
                                const char * Format,
                                ...
                                );
XFS_EXTERN rc_t CC XFSPathSetPath (
                                const struct XFSPath * self,
                                const struct XFSPath * Path
                                );
XFS_EXTERN rc_t CC XFSPathAppend (
                                const struct XFSPath * self,
                                const char * Format,
                                ...
                                );
XFS_EXTERN rc_t CC XFSPathAppendPath (
                                const struct XFSPath * self,
                                const struct XFSPath * Path
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN bool CC XFSPathIsAbsolute (
                                const struct XFSPath * self
                                );

XFS_EXTERN uint32_t CC XFSPathPartCount (
                                const struct XFSPath * self
                                );

XFS_EXTERN const char * CC XFSPathPartGet (
                                const struct XFSPath * self,
                                uint32_t Index
                                );

XFS_EXTERN const char * CC XFSPathGet (
                                const struct XFSPath * self
                                );

XFS_EXTERN const char * CC XFSPathOrig (
                                const struct XFSPath * self
                                );

XFS_EXTERN rc_t CC XFSPathParent (
                                const struct XFSPath * self,
                                const struct XFSPath ** Parent
                            );

    /*) Don'd frorget to free Name after calling */
XFS_EXTERN const char *  CC XFSPathName (
                                const struct XFSPath * self
                                );


XFS_EXTERN bool CC XFSPathEqual (
                                const struct XFSPath * Path1,
                                const struct XFSPath * Path2
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*) These methods will allow to compose part of the path.
      | NOTE: there are two positions : From - inclusive, and To -
      |       exclusive
      | NOTE: these methods will allocate string, and caller should
      |       free it.
      (*/
XFS_EXTERN rc_t CC XFSPathSub (
                            const struct XFSPath * self,
                            size_t From,
                            size_t To,
                            const struct XFSPath ** Path
                            );

XFS_EXTERN rc_t CC XFSPathFrom (
                            const struct XFSPath * self,
                            size_t From,
                            const struct XFSPath ** Path
                            );

XFS_EXTERN rc_t CC XFSPathTo (
                            const struct XFSPath * self,
                            size_t To,
                            const struct XFSPath ** Path
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*) That method will check if Path is base path for 'self'. i.e.
      | self could be composed as concatenation of Path and RelPath
      | RelPath could be zero, in that case it will be not returned
      (*/
XFS_EXTERN bool CC XFSPathIsBase (
                                const struct XFSPath * self,
                                const struct XFSPath * Path,
                                const struct XFSPath ** RelPath
                                );
XFS_EXTERN bool CC XFSPathSIsBase (
                                const char * self,
                                const char * Path,
                                const struct XFSPath ** RelPath
                                );

    /*) That method will check if Path is child ( sub ) path for 'self'.
      | i.e. Path could be composed as concatenation of self and RelPath
      | RelPath could be zero, in that case it will be not returned
      (*/
XFS_EXTERN bool CC XFSPathIsChild (
                                const struct XFSPath * self,
                                const struct XFSPath * Path,
                                const struct XFSPath ** RelPath
                                );
XFS_EXTERN bool CC XFSPathSIsChild (
                                const char * self,
                                const char * Path,
                                const struct XFSPath ** RelPath
                                );


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN void CC XFSPathDump ( const struct XFSPath * self );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _path_h_ */
