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

#ifndef _h_kns_adapt_
#define _h_kns_adapt_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KStream;


/*--------------------------------------------------------------------------
 * KStream
 *  adapters between KFile and KStream
 */

/* FromKFilePair
 *  create a KStream from a pair of KFiles
 *  maintains a "pos" marker for input and output files
 *
 *  "strm" [ OUT ] - resultant KStream
 *
 *  "read" [ IN, NULL OKAY ] - file to use for stream reading
 *
 *  "write" [ IN, NULL OKAY ] - file to use for stream writing
 *
 * NB - EITHER "read" or "write" may be NULL, but not both.
 */
KNS_EXTERN rc_t CC KStreamFromKFilePair ( struct KStream **strm,
    struct KFile const *read, struct KFile *write );

/* FromKStreamPair
 *  create a KStream from a pair of KStreams
 *
 *  "strm" [ OUT ] - resultant KStream
 *
 *  "read" [ IN, NULL OKAY ] - stream to use for input
 *
 *  "write" [ IN, NULL OKAY ] - stream to use for output
 *
 * NB - EITHER "read" or "write" may be NULL, but not both.
 */
KNS_EXTERN rc_t CC KStreamFromKStreamPair ( struct KStream **strm,
    struct KStream const *read, struct KStream *write );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_adapt_ */
