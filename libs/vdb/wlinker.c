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

#include <vdb/extern.h>

#define TRACK_REFERENCES 0

#include "linker-priv.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


extern VTRANSFACT_DECL ( meta_write );
extern VTRANSFACT_DECL ( meta_attr_write );

/* newly imported things */
extern VTRANSFACT_DECL ( NCBI_csra2_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_cmp_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_cmpb_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_cmpf_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_extract_name_fmt );
extern VTRANSFACT_DECL ( NCBI_SRA_extract_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_phred_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_qual4_encode );
extern VTRANSFACT_DECL ( NCBI_SRA_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_SRA_setRnaFlag );
#if HAVE_ALIGN_STATS_TRIGGER
extern VTRANSFACT_DECL ( NCBI_align_stats_trigger );
extern VTRANSFACT_DECL ( NCBI_seq_stats_trigger );
#endif
extern VTRANSFACT_DECL ( NCBI_refSeq_stats );
extern VTRANSFACT_DECL ( idx_text_insert );
extern VTRANSFACT_DECL ( vdb_bzip );
extern VTRANSFACT_DECL ( vdb_checksum );
extern VTRANSFACT_DECL ( vdb_fzip );
extern VTRANSFACT_DECL ( vdb_rlencode );
extern VTRANSFACT_DECL ( vdb_zip );

/* InitFactories
 */
rc_t VLinkerInitFactories ( VLinker *self, struct KSymTable *tbl, struct SchemaEnv const *env )
{
    static VLinkerIntFactory fact [] =
    {
        { meta_write, "meta:write" },
        { meta_attr_write, "meta:attr:write" },

        { NCBI_csra2_stats_trigger, "NCBI:csra2:stats_trigger" },
        { NCBI_SRA_cmp_stats_trigger, "NCBI:SRA:cmp_stats_trigger" },
        { NCBI_SRA_cmpb_stats_trigger, "NCBI:SRA:cmpb_stats_trigger" },
        { NCBI_SRA_cmpf_stats_trigger, "NCBI:SRA:cmpf_stats_trigger" },
        { NCBI_SRA_extract_name_fmt, "NCBI:SRA:extract_name_fmt" },
        { NCBI_SRA_extract_spot_name, "NCBI:SRA:extract_spot_name" },
        { NCBI_SRA_phred_stats_trigger, "NCBI:SRA:phred_stats_trigger" },
        { NCBI_SRA_qual4_encode, "NCBI:SRA:qual4_encode" },
        { NCBI_SRA_stats_trigger, "NCBI:SRA:stats_trigger" },
        { NCBI_SRA_setRnaFlag, "NCBI:SRA:setRnaFlag" },
#if HAVE_ALIGN_STATS_TRIGGER
        { NCBI_align_stats_trigger, "NCBI:align:stats_trigger" },
        { NCBI_seq_stats_trigger, "NCBI:seq:stats_trigger" },
#endif
        { NCBI_refSeq_stats, "NCBI:refSeq:stats" },
        { idx_text_insert, "idx:text:insert" },
        { vdb_bzip, "vdb:bzip" },
        { vdb_checksum, "vdb:checksum" },
        { vdb_fzip, "vdb:fzip" },
        { vdb_rlencode, "vdb:rlencode" },
        { vdb_zip, "vdb:zip" }
    };

    rc_t rc = VLinkerInitFactoriesRead ( self, tbl, env );
    if ( rc == 0 )
        rc = VLinkerAddFactories ( self, fact, sizeof fact / sizeof fact [ 0 ], tbl, env );
    return rc;
}
