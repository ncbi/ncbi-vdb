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

#ifndef _h_sra_priv_
#define _h_sra_priv_

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifndef KONST
#define KONST const
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KMetadata;
struct VDBManager;
struct VSchema;
struct VTable;
struct VCursor;
struct VTypedecl;
struct VTypedef;
struct VResolver;
struct SRACache;

#define CSRA_EXT(lite) (lite ? ".lite.sra" : ".sra")
#define SRA_EXT(lite) (lite ? ".lite.sra" : ".sra")

/*--------------------------------------------------------------------------
 * SRAMgr
 */
struct SRAMgr
{
    struct VDBManager KONST *vmgr;
    struct VSchema const *schema;
#if OLD_SRAPATH_MGR
    struct SRAPath volatile *_pmgr;
#else
    struct VResolver volatile *_pmgr;
#endif
    struct SRACache* cache;
    KRefcount refcount;
    KCreateMode mode;
    bool read_only;
};

/* SRAMgrMake
 */
rc_t SRAMgrMake ( SRAMgr **mgr,
    struct VDBManager const *vmgr, struct KDirectory const *wd );

/* Attach
 * Sever
 */
SRAMgr *SRAMgrAttach ( const SRAMgr *self );
rc_t SRAMgrSever ( const SRAMgr *self );


/* AccessSRAPath
 *  returns a new reference to SRAPath
 *  do NOT access "pmgr" directly
 */
struct SRAPath *SRAMgrAccessSRAPath ( const SRAMgr *self );

/*--------------------------------------------------------------------------
 * SRATable
 */
struct SRATable
{
    KONST SRAMgr *mgr;
    struct VTable KONST *vtbl;
    struct VCursor KONST *curs;
    struct KMetadata KONST *meta;

    Vector wcol;

    KRefcount refcount;

    uint32_t metavers;

    KCreateMode mode;
    bool curs_open;
    bool read_only;

    uint64_t spot_count;
    uint64_t base_count;
    int64_t min_spot_id;
    int64_t max_spot_id;
};

/* Whack
 * Destroy
 */
rc_t SRATableWhack ( SRATable *self );
void SRATableDestroy ( SRATable *self );

/* Attach
 * Sever
 */
SRATable *SRATableAttach ( const SRATable *self );
rc_t SRATableSever ( const SRATable *self );

/* SRATableRead
 */
rc_t SRATableRead ( const SRATable *self, spotid_t id,
    uint32_t cid, const void **base, bitsz_t *offset, bitsz_t *size );

/* SRATableColDatatype
 */
rc_t SRATableColDatatype ( const SRATable *self, uint32_t idx,
    struct VTypedecl *type, struct VTypedef *def );

/* SRATableGetIdRange
 */
rc_t SRATableGetIdRange ( const SRATable *self, uint32_t idx,
    spotid_t id, spotid_t *pfirst, spotid_t *last );

rc_t ResolveTablePath ( const SRAMgr *mgr, char *path, size_t psize, const char *spec, va_list args );

rc_t SRATableFillOut ( SRATable *self, bool update );
    
/*--------------------------------------------------------------------------
 * SRAColumn
 */
struct SRAColumn
{
    const SRATable *tbl;
    KRefcount refcount;
    uint32_t idx;
    uint32_t elem_bits;
    bool read_only;
};


/* Whackity whack
 */
rc_t SRAColumnWhack ( SRAColumn *self );


#ifdef __cplusplus
}
#endif

#endif /* _sra_priv_ */
