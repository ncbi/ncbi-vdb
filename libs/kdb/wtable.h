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

#include <kdb/table.h>
#include <klib/container.h>
#include <klib/refcount.h>

#include <klib/symbol.h>
#include <kfs/md5.h>

#include "table-base.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDatabase;
struct KDBManager;
struct KDirectory;

/*--------------------------------------------------------------------------
 * KTable
 *  represents a table
 *  normally implemented as a directory
 *  but may be a single archive file
 *  in either executable or streamable format
 */
typedef struct KWTable KWTable;
struct KWTable
{
    KTable dad;

    struct KDirectory *dir;
    struct KDBManager *mgr;
    struct KDatabase *db;
    struct KMD5SumFmt *md5;

    uint32_t opencount;
    bool use_md5;
    bool read_only;
    bool prerelease;
    uint8_t align [ 5 ];

    KSymbol sym;

    char path [ 1 ];
};

rc_t KWTableMake ( KWTable **tblp, const KDirectory *dir, const char *path, KMD5SumFmt * md5, bool read_only );

/* check a disk-resident column for needing re-indexing */
bool KWTableColumnNeedsReindex ( KWTable *self, const char *colname );


#ifdef __cplusplus
}
#endif
