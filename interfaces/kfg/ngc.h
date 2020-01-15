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

#ifndef _h_kfg_ngc_
#define _h_kfg_ngc_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h> /* rc_t */
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KNgcObj KNgcObj;

/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;


KFG_EXTERN rc_t CC KNgcObjMakeFromString ( const KNgcObj **ngc, const char * line );
KFG_EXTERN rc_t CC KNgcObjMakeFromFile ( const KNgcObj **ngc, const struct KFile * src );

/* AddRef
 * Release
 */
KFG_EXTERN rc_t CC KNgcObjAddRef ( const KNgcObj *self );
KFG_EXTERN rc_t CC KNgcObjRelease ( const KNgcObj *self );

KFG_EXTERN rc_t CC KNgcObjPrint ( const KNgcObj *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KNgcObjWriteToFile ( const KNgcObj *self, struct KFile * dst );
KFG_EXTERN rc_t CC KNgcObjWriteKeyToFile ( const KNgcObj *self, struct KFile * dst );

KFG_EXTERN rc_t CC KNgcObjGetProjectId ( const KNgcObj *self, uint32_t * id );
KFG_EXTERN rc_t CC KNgcObjGetProjectName ( const KNgcObj *self, char * buffer, size_t buffer_size, size_t * written );
KFG_EXTERN rc_t CC KNgcObjGetTicket ( const KNgcObj *self,
    char * buffer, size_t buffer_size, size_t * written );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_ngc_ */
