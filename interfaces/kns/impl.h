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

#ifndef _h_kns_impl_
#define _h_kns_impl_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_kns_stream_
#include <kns/stream.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
typedef union KStream_vt KStream_vt;


/*--------------------------------------------------------------------------
 * KStream
 *  a virtual stream
 */
struct KStream
{
    const KStream_vt *vt;
    KRefcount refcount;
    uint8_t read_enabled;
    uint8_t write_enabled;
    uint8_t align [ 2 ];
};

#ifndef KSTREAM_IMPL
#define KSTREAM_IMPL struct KStream
#endif

typedef struct KStream_vt_v1 KStream_vt_v1;
struct KStream_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KSTREAM_IMPL *self );
    rc_t ( CC * read ) ( const KSTREAM_IMPL *self,
        void *buffer, size_t bsize, size_t *num_read );
    rc_t ( CC * write ) ( KSTREAM_IMPL *self,
        const void *buffer, size_t size, size_t *num_writ );
    /* end minor version == 0 */

    /* start minor version == 1 */
    rc_t ( CC * timed_read ) ( const KSTREAM_IMPL *self,
        void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm );
    rc_t ( CC * timed_write ) ( KSTREAM_IMPL *self,
        const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm );
    /* end minor version == 1 */
};

union KStream_vt
{
    KStream_vt_v1 v1;
};


/* Init
 *  initialize a newly allocated stream object
 */
KNS_EXTERN rc_t CC KStreamInit ( KStream *self, const KStream_vt *vt,
    const char *classname, const char *strname,
    bool read_enabled, bool write_enabled );

/* Whack
 *  does anything necessary to tear down KStream
 *  but does not delete memory
 */
KNS_EXTERN rc_t CC KStreamWhack ( KStream * self, const char * classname );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_impl_ */
