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

#include <kdb/database.h>
#include <kdb/column.h>

#include <klib/symbol.h>

#ifndef KONST
#define KONST
#endif

#define KDATABASE_IMPL KDatabase
#include "database-base.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KDBManager;
struct KDirectory;
struct KMD5SumFmt;


/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system
 */

struct KDatabase
{
    KDatabaseBase dad;

    /* manager reference */
    struct KDBManager KONST *mgr;

    /* if a sub-directory */
    struct KDatabase KONST *parent;

    /* database directory */
    struct KDirectory KONST *dir;

    /* MD5 format object */
    struct KMD5SumFmt *md5;

    /* open references */
    uint32_t opencount;

    /* fits into manager symbol table */
    KSymbol sym;

    /* create and checksum modes for columns */
    KCreateMode cmode;
    KChecksum checksum;

    /* true if database was opened read-only */
    bool read_only;

    /* symbol text space */
    char path [ 1 ];
};

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KDatabase *KDatabaseAttach ( const KDatabase *self );
rc_t KDatabaseSever ( const KDatabase *self );

#ifdef __cplusplus
}
#endif

