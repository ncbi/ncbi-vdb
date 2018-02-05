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

#ifndef _hdict_h_
#define _hdict_h_

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
 * XFSHashDict - wrapper written over hash table.
 *
 * Okay, I am using B_STree as a container for fast access to data
 * I am using it almost 14 times and every time it is indexed by
 * string ( char * ). So, there is simpele-impele menetation of 
 * look up dictionary with hash table written by Mike
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 // Defines
((*/

typedef void ( CC * XFSHashDictBanana ) ( const void * Value );
typedef void ( CC * XFSHashDictEacher ) (
                                    const char * Key,
                                    const void * Value,
                                    const void * Data
                                    );

/*))
 // Forwards
((*/
struct XFSHashDict;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /*)) If Banana can be NULL, then pointer will be not deleted
     ((*/
XFS_EXTERN rc_t CC XFSHashDictMake (
                            const struct XFSHashDict ** Dict,
                            XFSHashDictBanana Banana
                            );

XFS_EXTERN rc_t CC XFSHashDictDispose (
                            const struct XFSHashDict * self
                            );

XFS_EXTERN bool CC XFSHashDictHas (
                            const struct XFSHashDict * self,
                            const char * Key
                            );

XFS_EXTERN rc_t CC XFSHashDictGet (
                            const struct XFSHashDict * self,
                            const void ** Value,
                            const char * Key
                            );

XFS_EXTERN rc_t CC XFSHashDictAdd (
                            const struct XFSHashDict * self,
                            const void * Value,
                            const char * Key
                            );

XFS_EXTERN rc_t CC XFSHashDictDel (
                            const struct XFSHashDict * self,
                            const char * Key
                            );

XFS_EXTERN rc_t CC XFSHashDictReserve (
                            const struct XFSHashDict * self,
                            size_t NewSize
                            );

XFS_EXTERN rc_t CC XFSHashDictForEach (
                            const struct XFSHashDict * self,
                            XFSHashDictEacher,
                            const void * Data
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSIHashDict;

typedef uint64_t ihashD_t;

typedef void ( CC * XFSIHashDictBanana ) ( const void * Value );
typedef void ( CC * XFSIHashDictEacher ) (
                                    ihashD_t Key,
                                    const void * Value,
                                    const void * Data
                                    );

XFS_EXTERN rc_t CC XFSIHashDictMake (
                            const struct XFSIHashDict ** Table,
                            XFSIHashDictBanana Banana
                            );

XFS_EXTERN rc_t CC XFSIHashDictDispose (
                            const struct XFSIHashDict * self
                            );

XFS_EXTERN bool CC XFSIHashDictHas (
                            const struct XFSIHashDict * self,
                            ihashD_t Key
                            );

XFS_EXTERN rc_t CC XFSIHashDictGet (
                            const struct XFSIHashDict * self,
                            const void ** Value,
                            ihashD_t Key
                            );

XFS_EXTERN rc_t CC XFSIHashDictAdd (
                            const struct XFSIHashDict * self,
                            const void * Value,
                            ihashD_t Key
                            );

XFS_EXTERN rc_t CC XFSIHashDictDel (
                            const struct XFSIHashDict * self,
                            ihashD_t Key
                            );

XFS_EXTERN rc_t CC XFSIHashDictReserve (
                            const struct XFSIHashDict * self,
                            size_t NewSize
                            );

XFS_EXTERN rc_t CC XFSIHashDictForEach (
                            const struct XFSIHashDict * self,
                            XFSIHashDictEacher Eacher,
                            const void * Data
                            );

XFS_EXTERN size_t CC XFSHashDictCount (
                            const struct XFSHashDict * self 
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _hdict_h_ */
