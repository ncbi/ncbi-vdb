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

#ifndef _h_database_priv_
#define _h_database_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_vdb_database_
#include <vdb/database.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef KONST
#define KONST
#endif

#ifndef SKONST
#define SKONST const
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct KDatabase;
struct SDatabase;
struct KMetadata;
struct VDBManager;

/*--------------------------------------------------------------------------
 * VDatabase
 *  interface to physical database
 */
struct VDatabase
{
    /* creating manager */
    struct VDBManager KONST *mgr;

    /* enclosing database - NULL OKAY */
    KONST VDatabase *dad;

    /* schema and db description */
    struct VSchema SKONST *schema;
    struct SDatabase const *sdb;

    /* underlying physical layer database */
    struct KDatabase KONST *kdb;

    /* open database metadata */
    struct KMetadata KONST *meta;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );
#if 0
    /* externally visible names */
    BSTree scope;
#endif
    /* page size for creating columns */
    size_t pgsize;

    /* open references */
    KRefcount refcount;

    /* create and checksum modes for columns */
    uint8_t cmode;
    uint8_t checksum;

    /* open mode */
    bool read_only;
};


/* Whack
 */
rc_t CC VDatabaseWhack ( VDatabase *self );


/* Attach
 * Sever
 *  internal reference management
 */
VDatabase *VDatabaseAttach ( const VDatabase *self );
rc_t VDatabaseSever ( const VDatabase *self );


/* Make
 *  creates an initialized database object
 *  expects either a mgr or a parent db,
 *  enforces schema validity according to some rules,
 */
rc_t VDatabaseMake ( VDatabase **db,
    struct VDBManager const *mgr,
    const VDatabase *dad,
    struct VSchema const *schema );


/* LoadSchema
 *  looks in metadata for stored schema
 */
rc_t VDatabaseLoadSchema ( VDatabase *self );

/* StoreSchema
 *  stores schema definition in metadata
 */
rc_t VDatabaseStoreSchema ( VDatabase *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_database_priv_ */
