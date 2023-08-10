#ifndef _h_schema_priv_
#define _h_schema_priv_

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
* ===========================================================================
* Access to internals of VSchema to allow them to be used in other projects.
*/

#include <klib/text.h> /* String */

#ifdef __cplusplus
extern "C" {
#endif

struct SDatabase;
struct STable;
struct SView;
struct Vector;
struct VSchema;

typedef struct KSymbolNameElm {
    const String * name;
    struct KSymbolNameElm * next;
} KSymbolNameElm;

typedef struct KSymbolName {
    uint32_t version;
    struct KSymbolNameElm * name;
} KSymbolName;

rc_t VSchemaGetDb(const struct VSchema *self, const struct Vector **db);
rc_t VSchemaGetTbl(const struct VSchema *self, const struct Vector **tbl);
rc_t VSchemaGetView(const struct VSchema *self, const struct Vector **view);

rc_t SDatabaseGetDad(const struct SDatabase *self, const struct SDatabase **dd);
rc_t STableGetParents(const struct STable *self, const struct Vector **parents);
rc_t SViewGetParents(const struct SView *self, const struct Vector **parents);

rc_t SDatabaseMakeKSymbolName(const struct SDatabase *self, KSymbolName **sn);
rc_t STableMakeKSymbolName(const struct STable *self, KSymbolName **sn);
rc_t SViewMakeKSymbolName(const struct SView *self, KSymbolName **sn);
rc_t KSymbolNameWhack(KSymbolName *self);

#ifdef __cplusplus
}
#endif

#endif /*  _h_schema_priv_ */
