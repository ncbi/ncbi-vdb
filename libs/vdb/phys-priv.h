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

#ifndef _h_phys_priv_
#define _h_phys_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_data_buffer_
#include <klib/data-buffer.h>
#endif

#ifndef KONST
#define KONST
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KColumn;
struct KMetadata;
struct KMDataNode;
struct VTypedecl;
struct VSchema;
struct VTable;
struct VCursor;
struct VBlob;
struct SExpression;
struct SPhysMember;
struct VProduction;


/*--------------------------------------------------------------------------
 * VPhysical
 *
 *  "?start_id" is a starting id for column
 *
 *  "?stop_id" is an INCLUSIVE stop id for column, where the apparent
 *  convention was to whine about using intervals at all due to legacy
 *  issues while at the same time using "stop" as a lame attempt at
 *  indicating fully-closed vs. "end" to indicate half-closed intervals.
 */
typedef struct VPhysical VPhysical;
struct VPhysical
{
    /* physical column */
    int64_t kstart_id, kstop_id;
    struct KColumn KONST *kcol;
    struct KMetadata KONST *meta;

    /* static column */
    int64_t sstart_id, sstop_id;
    struct KMDataNode KONST *knode;

    /* owning cursor */
    struct VCursor KONST *curs;

    /* definition from schema */
    struct SPhysMember const *smbr;

    /* encoding expression */
    struct SExpression const *enc;

    /* read production
       provides decompressed data,
       with correct byte-order */
    struct VProduction *out;

    /* output of decompression schema */
    struct VProduction *b2p;

    /* output of compression schema */
    struct VProduction *b2s;

    /* write production
       provides decompressed data
       in native byte-order */
    struct VProduction *in;

    /* cached static row data */
    KDataBuffer srow;

    /* id */
    uint32_t id;

    /* fixed row length */
    uint32_t fixed_len;

    /* synthesize v1 blob header */
    bool no_hdr;

    /* v0 or v1 blobs */
    bool v01;

    /* recorded at create time */
    bool read_only;
};

/* symbol for failed production */
#define FAILED_PHYSICAL ( ( VPhysical* ) 1U )

/* Make
 *  make an empty object
 */
rc_t VPhysicalMake ( VPhysical **phys,
    struct VCursor KONST *curs, struct SPhysMember const *smbr );

/* Whack
 */
void VPhysicalDestroy ( VPhysical *self );
void CC VPhysicalWhack ( void *self, void *ignore );

/* Open
 *  open existing columns
 *  load schema definitions
 */
rc_t VPhysicalOpenWrite ( VPhysical *self,
    struct VSchema *schema, struct VTable const *tbl );
rc_t VPhysicalOpenRead ( VPhysical *self,
    struct VSchema *schema, struct VTable const *tbl );

/* Finish
 *  complete common open procedures
 */
rc_t VPhysicalFinishKColumn ( VPhysical *self,
    struct VSchema *schema, struct SPhysMember const *smbr );
rc_t VPhysicalFinishStatic ( VPhysical *self,
    struct VSchema const *schema, struct SPhysMember const *smbr );

/* LoadMetadata
 *  looks in metadata for stored schema
 *  loads fixed row length
 */
rc_t VPhysicalLoadMetadata ( VPhysical *self,
    struct VTypedecl *td, struct VSchema *schema );

/* Read
 *  get the blob
 */
rc_t VPhysicalRead ( VPhysical *self,
    struct VBlob **vblob, int64_t id, uint32_t cnt, uint32_t elem_bits );
rc_t VPhysicalReadBlob ( VPhysical *self,
    struct VBlob **vblob, int64_t id, uint32_t elem_bits );

/* IsStatic
 *  is this a static column
 */
rc_t VPhysicalIsStatic ( const VPhysical *self, bool *is_static );

/* GetKColumn
 *  try to get a KColumn,
 *  and if that fails, indicate whether the column is static
 */
rc_t VPhysicalGetKColumn ( const VPhysical * self, struct KColumn ** kcol, bool * is_static );

#ifdef __cplusplus
}
#endif

#endif /* _h_phys_priv_ */
