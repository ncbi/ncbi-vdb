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

#pragma once

#include <kdb/meta.h>
#include <kfs/directory.h>

typedef struct KRMetadata KRMetadata;
#define KMETA_IMPL KRMetadata
#include "meta-base.h"

struct KDatabase;
struct KTable;
struct KRColumn;
struct KDBManager;
struct KMDataNode;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KRMetadata KRMetadata;
struct KRMetadata
{
    KMetadata dad;

    const struct KDirectory *dir;
    const struct KDBManager *mgr;

    /* owner */
    const struct KDatabase *db;
    const struct KTable *tbl;
    const struct KRColumn *col;

    /* root node */
    struct KRMDataNode *root;

    uint32_t vers;
    uint32_t rev;
    bool byteswap;
    char path [ 1 ];
};

rc_t KRMetadataMakeRead ( KRMetadata **metap, const KDirectory *dir, const char *path, uint32_t rev );

#ifdef __cplusplus
}
#endif

