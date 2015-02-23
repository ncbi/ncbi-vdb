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

#ifndef _hpp_vdb3_kfc_timemgr_
#define _hpp_vdb3_kfc_timemgr_

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class nS_t;
    class TimeMgr;
    class timeout_t;
    class Refcount;
    class timestamp_t;


    /*------------------------------------------------------------------
     * TimeMgrItf
     *  time manager
     */
    interface TimeMgrItf
    {

        virtual timestamp_t cur_time () const = 0;
        void prepare_timeout ( const timeout_t & tm ) const;

    protected:

        static timestamp_t make_timestamp ( const nS_t & nS );
        TimeMgr make_tmmgr_ref ( Refcount * obj, caps_t caps );
    };


    /*------------------------------------------------------------------
     * TimeMgr
     *  time manager reference
     */
    class TimeMgr : public Ref < TimeMgrItf >
    {
    public:

        // return the current time
        timestamp_t cur_time () const;

        // given a timeout, prepare it for use with OS
        // under POSIX, this may convert a time delay to absolute time
        void prepare_timeout ( const timeout_t & tm ) const;

        // C++
        TimeMgr ();
        TimeMgr ( const TimeMgr & r );
        void operator = ( const TimeMgr & r );
        TimeMgr ( const TimeMgr & r, caps_t reduce );

    private:

        // factory
        TimeMgr ( Refcount * obj, TimeMgrItf * itf, caps_t caps );

        friend interface TimeMgrItf;
    };
}

#endif // _hpp_vdb3_kfc_timemgr_
