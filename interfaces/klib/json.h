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

#ifndef _h_klib_json_
#define _h_klib_json_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct VNamelist;
struct KDataBuffer;

typedef struct KJsonValue KJsonValue;
typedef struct KJsonObject KJsonObject;
typedef struct KJsonArray KJsonArray;

enum jsType
{
    jsInvalid,
    jsString,
    jsNumber,
    jsBool,
    jsNull,
    jsObject,
    jsArray
};

/*
 * error [ OUT, NULL OK ]
 */
KLIB_EXTERN rc_t CC KJsonValueMake ( KJsonValue ** root, const char * input, char * error, size_t error_size );

KLIB_EXTERN void CC KJsonValueWhack ( KJsonValue * root );

KLIB_EXTERN enum jsType CC KJsonGetValueType ( const KJsonValue * value );

/*
 *  Will work on Uint, Int, Double, returning the original numeric string
*/
KLIB_EXTERN rc_t CC KJsonGetString ( const KJsonValue * node, const char ** value );

KLIB_EXTERN rc_t CC KJsonGetNumber ( const KJsonValue * node, int64_t * value );
KLIB_EXTERN rc_t CC KJsonGetDouble ( const KJsonValue * node, double * value );
KLIB_EXTERN rc_t CC KJsonGetBool ( const KJsonValue * node, bool * value );

KLIB_EXTERN const KJsonObject * CC  KJsonValueToObject ( const KJsonValue * value );
KLIB_EXTERN const KJsonValue * CC   KJsonObjectToValue ( const KJsonObject * object );

KLIB_EXTERN rc_t CC KJsonObjectGetNames ( const KJsonObject * node, struct VNamelist * names );
KLIB_EXTERN const KJsonValue * CC KJsonObjectGetMember ( const KJsonObject * node, const char * name );

KLIB_EXTERN const KJsonArray * CC KJsonValueToArray ( const KJsonValue * value );
KLIB_EXTERN const KJsonValue * CC KJsonArrayToValue ( const KJsonArray * array );

KLIB_EXTERN uint32_t CC KJsonArrayGetLength ( const KJsonArray * node );
KLIB_EXTERN const KJsonValue * CC KJsonArrayGetElement ( const KJsonArray * node, uint32_t index );

/*
 * Print out to a KDataBuffer containing NUL-terminated JSON-formatted string representation of KJsonObject
 * 'output' will be initialized and allocated to grow as much as needed, in 'increment' byte increments.
 * if 'increment' == 0, it will be set to 1024.
 * If successful, the caller is responsible for calling KDataBufferWhack on 'output'.
 * if 'pretty', will be pretty-printed using tabs
 */
KLIB_EXTERN rc_t CC KJsonToJsonString ( const KJsonValue * root, struct KDataBuffer * output, size_t increment, bool pretty );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_json_ */
