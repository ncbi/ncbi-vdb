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

#ifndef _apath_h_
#define _apath_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   another and hope more successifull attempt to create
  ))))  good usable interface to a Pfad
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  Universal method which should load config from resource
 ((*/

struct XFSPfad;

XFS_EXTERN rc_t CC XFSPfadMake (
                                const struct XFSPfad ** Out,
                                const char * Format,
                                ...
                                );

XFS_EXTERN rc_t CC XFSPfadVMake (
                                const struct XFSPfad ** Out,
                                const char * Format,
                                va_list Args
                                );

    /*)) These two methods are requireing KDirectory,
     //  and I am not sure if those should be here ... later
    ((*/
XFS_EXTERN rc_t CC XFSPfadMakeAbsolute (
                                const struct XFSPfad ** Out,
                                const char * Format,
                                ...
                                );

XFS_EXTERN rc_t CC XFSPfadVMakeAbsolute (
                                const struct XFSPfad ** Out,
                                const char * Format,
                                va_list Args
                                );

XFS_EXTERN rc_t CC XFSPfadDup (
                                const struct XFSPfad * In,
                                const struct XFSPfad ** Out
                                );

XFS_EXTERN rc_t CC XFSPfadAddRef ( const struct XFSPfad * self );
XFS_EXTERN rc_t CC XFSPfadRelease ( const struct XFSPfad * self );


XFS_EXTERN rc_t CC XFSPfadSet (
                                const struct XFSPfad * self,
                                const char * Format,
                                ...
                                );
XFS_EXTERN rc_t CC XFSPfadSetPfad (
                                const struct XFSPfad * self,
                                const struct XFSPfad * Pfad
                                );
XFS_EXTERN rc_t CC XFSPfadAppend (
                                const struct XFSPfad * self,
                                const char * Format,
                                ...
                                );
XFS_EXTERN rc_t CC XFSPfadAppendPfad (
                                const struct XFSPfad * self,
                                const struct XFSPfad * Pfad
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN bool CC XFSPfadIsAbsolute (
                                const struct XFSPfad * self
                                );

XFS_EXTERN uint32_t CC XFSPfadPartCount (
                                const struct XFSPfad * self
                                );

XFS_EXTERN const char * CC XFSPfadPartGet (
                                const struct XFSPfad * self,
                                uint32_t Index
                                );

XFS_EXTERN const char * CC XFSPfadGet (
                                const struct XFSPfad * self
                                );

XFS_EXTERN const char * CC XFSPfadOrig (
                                const struct XFSPfad * self
                                );

XFS_EXTERN rc_t CC XFSPfadParent (
                                const struct XFSPfad * self,
                                const struct XFSPfad ** Parent
                            );

    /*) Don'd frorget to free Name after calling */
XFS_EXTERN const char *  CC XFSPfadName (
                                const struct XFSPfad * self
                                );


XFS_EXTERN bool CC XFSPfadEqual (
                                const struct XFSPfad * Pfad1,
                                const struct XFSPfad * Pfad2
                                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*) These methods will allow to compose part of the path.
      | NOTE: there are two positions : From - inclusive, and To -
      |       exclusive
      | NOTE: these methods will allocate string, and caller should
      |       free it.
      (*/
XFS_EXTERN rc_t CC XFSPfadSub (
                            const struct XFSPfad * self,
                            size_t From,
                            size_t To,
                            const struct XFSPfad ** Pfad
                            );

XFS_EXTERN rc_t CC XFSPfadFrom (
                            const struct XFSPfad * self,
                            size_t From,
                            const struct XFSPfad ** Pfad
                            );

XFS_EXTERN rc_t CC XFSPfadTo (
                            const struct XFSPfad * self,
                            size_t To,
                            const struct XFSPfad ** Pfad
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*) That method will check if Pfad is base path for 'self'. i.e.
      | self could be composed as concatenation of Pfad and RelPfad
      | RelPfad could be zero, in that case it will be not returned
      (*/
XFS_EXTERN bool CC XFSPfadIsBase (
                                const struct XFSPfad * self,
                                const struct XFSPfad * Pfad,
                                const struct XFSPfad ** RelPfad
                                );
XFS_EXTERN bool CC XFSPfadSIsBase (
                                const char * self,
                                const char * Pfad,
                                const struct XFSPfad ** RelPfad
                                );

    /*) That method will check if Pfad is child ( sub ) path for 'self'.
      | i.e. Pfad could be composed as concatenation of self and RelPfad
      | RelPfad could be zero, in that case it will be not returned
      (*/
XFS_EXTERN bool CC XFSPfadIsChild (
                                const struct XFSPfad * self,
                                const struct XFSPfad * Pfad,
                                const struct XFSPfad ** RelPfad
                                );
XFS_EXTERN bool CC XFSPfadSIsChild (
                                const char * self,
                                const char * Pfad,
                                const struct XFSPfad ** RelPfad
                                );


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

XFS_EXTERN void CC XFSPfadDump ( const struct XFSPfad * self );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _apath_h_ */
