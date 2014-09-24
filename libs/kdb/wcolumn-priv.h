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

#ifndef _h_column_priv_
#define _h_column_priv_

#ifndef _h_kdb_column_
#include <kdb/column.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_coldata_priv_
#include "wcoldata-priv.h"
#endif

#ifndef _h_colidx_priv_
#include "wcolidx-priv.h"
#endif

#include <klib/symbol.h>
#include <kfs/file.h>
#include <kfs/md5.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct KDBManager;
struct KDirectory;
struct KMD5SumFmt;


/*--------------------------------------------------------------------------
 * KColumn
 */
struct KColumn
{
    struct KTable *tbl;
    struct KDBManager *mgr;
    struct KDirectory *dir;

    KMD5SumFmt *md5;

    KColumnIdx idx;
    KColumnData df;

    KRefcount refcount;
    uint32_t opencount;
    uint32_t commit_freq;
    uint32_t csbytes;
    int32_t checksum;
    bool read_only;

    KSymbol sym;

    char path [ 1 ];
};

/* Attach
 * Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
KColumn *KColumnAttach ( const KColumn *self );
rc_t KColumnSever ( const KColumn *self );

/* Cmp
 * Sort
 */
int KColumnCmp ( const void *item, struct BSTNode const *n );
int KColumnSort ( struct BSTNode const *item, struct BSTNode const *n );


rc_t KColumnFileCreate ( KFile ** ppf, KMD5File ** ppfmd5, KDirectory * dir, 
			 KMD5SumFmt * md5, KCreateMode mode,
			 bool append, const char * name);
rc_t KColumnFileOpenUpdate ( KFile ** ppf, KMD5File ** ppfmd5, KDirectory * dir, 
			     KMD5SumFmt * md5, bool append,
			     const char * name);

#ifdef __cplusplus
}
#endif

#endif /* _h_column_priv_ */
