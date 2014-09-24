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

#ifndef _hpp_kproc_barrier_
#define _hpp_kproc_barrier_

#ifndef _h_kproc_barrier_
#include <kproc/barrier.h>
#endif


/*--------------------------------------------------------------------------
 * KBarrier
 *  a thread synchronization device
 *  detains all callers until the required number has been reached
 */
struct KBarrier
{
    /* Make
     *  create a barrier
     *
     *  "count" [ IN ] - the number of threads to block
     */
    static inline rc_t Make ( KBarrier **b, uint32_t count )
        throw ()
    { return KBarrierMake ( b, count ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KBarrierAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KBarrierRelease ( this ); }


    /* Wait
     *  block until the required number of callers has been reached
     */
    inline rc_t Wait ()
        throw ()
    { return KBarrierWait ( this ); }

private:
    KBarrier ();
    ~ KBarrier ();
    KBarrier ( const KBarrier& );
    KBarrier &operator = ( const KBarrier& );
};


#endif // _hpp_kproc_barrier_
