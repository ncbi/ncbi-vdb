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
 *============================================================================
 *
 */

#ifndef _h_kdb_consistency_check_
#define _h_kdb_consistency_check_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_insdc_sra_
#include <insdc/sra.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
/*--------------------------------------------------------------------------
 * forwards
 */
struct KDatabase;
struct KTable;
struct KColumn;


/*--------------------------------------------------------------------------
 * CCReportTypes
 */
enum
{
    ccrpt_Done,
    ccrpt_MD5,
    ccrpt_Blob,
    ccrpt_Index,
    ccrpt_Visit
};


/*--------------------------------------------------------------------------
 * CCReportInfoBlock
 */
typedef struct CCReportInfoBlock CCReportInfoBlock;
struct CCReportInfoBlock
{
    const char *objName;
    uint32_t objId;
    uint32_t objType;
    uint32_t type;

    union
    {
        struct /* ccrb_done_s */
        {
            const char *mesg;
            rc_t rc;
        } done;

        struct ccrb_md5_s
        {
            const char *file;
            rc_t rc;
        } MD5;

        struct ccrb_blob_s
        {
            uint64_t start;
            uint64_t count;

        } blob;

        struct ccrb_index_s
        {
            int64_t start_id;
            uint64_t id_range;
            uint64_t num_keys;
            uint64_t num_rows;
            uint64_t num_holes;

        } index;
        
        struct ccrb_visit_s {
            unsigned depth;
        } visit;
    } info;
};

typedef rc_t ( CC *CCReportFunc ) ( const CCReportInfoBlock *info, void *data );

/* a flag for level parameter */
#define CC_INDEX_ONLY 0x80000000

/*--------------------------------------------------------------------------
 * KDatabase
 */
KDB_EXTERN rc_t CC KDatabaseConsistencyCheck ( struct KDatabase const *self,
    uint32_t depth, uint32_t level, CCReportFunc report, void *data );


/*--------------------------------------------------------------------------
 * KTable
 */
KDB_EXTERN rc_t CC KTableConsistencyCheck ( struct KTable const *self,
    uint32_t depth, uint32_t level, CCReportFunc report, void *data,
    INSDC_SRA_platform_id platform);


/*--------------------------------------------------------------------------
 * KColumn
 */
KDB_EXTERN rc_t CC KColumnConsistencyCheck ( struct KColumn const *self,
    uint32_t level, CCReportInfoBlock *info, CCReportFunc report, void *data );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_consistency_check_ */
