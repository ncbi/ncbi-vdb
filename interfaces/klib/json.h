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

typedef struct VNamelist VNamelist;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KJsonValue KJsonValue;
typedef struct KJsonObject KJsonObject;
typedef struct KJsonArray KJsonArray;

/*
 * error [ OUT, NULL OK ]
 */
rc_t CC KJsonMake ( KJsonObject ** root, const char * input, char * error, size_t error_size );

void CC KJsonWhack ( KJsonObject * root );

rc_t CC KJsonToString ( const KJsonObject * root, char * error, size_t error_size );

bool CC KJsonIsString ( const KJsonValue * value );
bool CC KJsonIsNumber ( const KJsonValue * value );
bool CC KJsonIsObject ( const KJsonValue * value );
bool CC KJsonIsArray ( const KJsonValue * value );
bool CC KJsonIsTrue ( const KJsonValue * value );
bool CC KJsonIsFalse ( const KJsonValue * value );
bool CC KJsonIsNull ( const KJsonValue * value );

rc_t CC KJsonGetString ( const KJsonValue * node, const char ** value );
rc_t CC KJsonGetNumber ( const KJsonValue * node, int64_t * value );
rc_t CC KJsonGetDouble ( const KJsonValue * node, double * value );

const KJsonObject * CC  KJsonValueToObject ( const KJsonValue * value );
const KJsonValue * CC   KJsonObjectToValue ( const KJsonObject * object );

rc_t CC KJsonObjectGetNames ( const KJsonObject * node, VNamelist * names );
const KJsonValue * CC KJsonObjectGetValue ( const KJsonObject * node, const char * name );

const KJsonArray * CC KJsonValueToArray ( const KJsonValue * value );
const KJsonValue * CC KJsonArrayToValue ( const KJsonArray * array );

uint32_t CC KJsonArrayLength ( const KJsonArray * node );
const KJsonValue * CC KJsonArrayGetElement ( const KJsonArray * node, uint32_t index );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_json_ */
