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
#ifndef _h_fastq_fq_dump_
#define _h_fastq_fq_dump_

#ifndef _h_fastq_extern_
#include <fastq/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * FastqDumper
 *  an object capable of producing fastq
 */
typedef struct FastqDumper FastqDumper;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
FASTQ_EXTERN rc_t CC FastqDumperAddRef ( const FastqDumper *self );
FASTQ_EXTERN rc_t CC FastqDumperRelease ( const FastqDumper *self );


/* FastqDumperMake
 *  make an empty dumper
 */
FASTQ_EXTERN rc_t CC FastqDumperMake ( FastqDumper **fastq );


/* FastqDumperAddSource
 *  adds an accession/filename/url to be processed
 */
FASTQ_EXTERN rc_t CC FastqDumperAddSource ( FastqDumper *self, const char * name );


/* FastqDumperEnableSpotSplitting
 *  spot-splitting is OFF per default
 */
FASTQ_EXTERN rc_t CC FastqDumperEnableSpotSplit ( FastqDumper *self );


/* FastqDumperEnableClipping
 *  clipping is OFF per default
 */
FASTQ_EXTERN rc_t CC FastqDumperEnableClipping ( FastqDumper *self );


/* FastqDumperAddRowRange
 * default is processing all rows in the source(s)
 */
FASTQ_EXTERN rc_t CC FastqDumperAddRowRange ( FastqDumper *self, int64_t range_start, uint64_t count );


/* FastqDumperEstimate
 *  estimates ( because of filtering ) the number of fastq-records
 *  for gui-tools to setup a progress-bar
 */
FASTQ_EXTERN rc_t CC FastqDumperEstimate ( const FastqDumper *self, uint64_t * count );


/* FastqDumperSetCallback
 *  the function will be called for every "fastq-record"
 */
FASTQ_EXTERN rc_t CC FastqDumperSetCallback ( FastqDumper *self, 
    void *data, void ( CC * on_record ) ( uint64_t nr, uint32_t length,
        const char * seq_id, const char *name,
        const char * read, const char * quality ) );


/* FastqDumperPerform
 *  performs the requested operations
 */
FASTQ_EXTERN rc_t CC FastqDumperPerform ( const FastqDumper *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_fastq_fq_dump_ */
