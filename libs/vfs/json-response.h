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

#ifndef _h_vfs_json_response_
#define _h_vfs_json_response_

#include <kfc/defs.h> /* rc_t */

typedef struct KSrvRespObj KSrvRespObj;
typedef struct Response4 Response4;

rc_t Response4Make    (      Response4 ** self, const char * input );
rc_t Response4AddRef  ( const Response4 * self );
rc_t Response4Release ( const Response4 * self );

rc_t Response4GetKSrvRespObjCount ( const Response4 * self, uint32_t * n );
rc_t Response4GetKSrvRespObjByIdx ( const Response4 * self, uint32_t i,
                                    const KSrvRespObj ** box );
rc_t Response4GetKSrvRespObjByAcc ( const Response4 * self, const char * acc,
                                    const KSrvRespObj ** box );

#endif /* _h_vfs_json_response_ */
