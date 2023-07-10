/*==============================================================================
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
* =========================================================================== */

#pragma once

#ifndef _h_kfc_defs_
#include <kfc/defs.h> /* rc_t */
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct String;

rc_t 
Sign_RSA_SHA256(
    const char * key,
    const char * input,
    const struct String ** output);

rc_t CC GCPAddAuthentication(const struct GCP * cself,
    KClientHttpRequest * req, const char * http_method);

rc_t GetJsonNumMember(const struct KJsonObject *obj, const char * name,
    int64_t * value);
rc_t GetJsonStringMember(const struct KJsonObject *obj, const char * name,
    const char ** value);

rc_t ParseAccessToken( const char * jsonResponse, char ** token, KTime_t * expiration );

/*TODO: use log.h instead, or promote to cloud-priv.h
    (there is a copy in cloud-mgr.c) */
#if 0
#include <stdio.h>
#define TRACE( ... )                                              \
    do { fprintf ( stderr, "%s:%d - ", __func__, __LINE__ );      \
         fprintf ( stderr, __VA_ARGS__ );                         \
         fputc ( '\n', stderr ); } while ( 0 )
#else
#define TRACE( ... ) \
    ( ( void ) 0 )
#endif

#ifdef __cplusplus
}
#endif
