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

#ifndef _h_sra_sraschema_
#define _h_sra_sraschema_

#ifndef _h_sra_extern_
#include <sra/extern.h>
#endif

#ifndef _h_klib_callconv_
#include <klib/callconv.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct SRAMgr;
struct VSchema;
struct VDBManager;


/*--------------------------------------------------------------------------
 * SRASchema
 *  a schema object pre-loaded with default SRA schema
 */


/* Make
 *  create an instance of the default SRA schema
 */
SRA_EXTERN rc_t CC SRAMgrMakeSRASchema ( struct SRAMgr const *self, struct VSchema **schema );
SRA_EXTERN rc_t CC VDBManagerMakeSRASchema ( struct VDBManager const *self, struct VSchema **schema );


#ifdef __cplusplus
}
#endif

#endif /* _h_sra_sraschema_ */
