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

#ifndef _hpp_vdb3_kfc_time_
#define _hpp_vdb3_kfc_time_

#ifndef _hpp_vdb3_kfc_atomic_
#include <kfc/atomic.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class nS_t;
    class uS_t;
    class mS_t;
    class seconds_t;


    /*------------------------------------------------------------------
     * tm_base_t
     *  basic inline operations for time quantities
     */
    template < class T >
    class tm_base_t
    {
    public:

        // conversion to raw number
        operator I64 () const
        { return val; }

        // arithmetic
        T operator + ( const T & tm ) const
        { return T ( val + tm . val ); }
        T operator - ( const T & tm ) const
        { return T ( val - tm . val ); }
        T operator * ( I64 m ) const
        { return T ( val * m ); }
        T operator / ( I64 d ) const;
        T operator % ( I64 d ) const;

        // updating arithmetic
        T & operator += ( const T & tm )
        { val += tm . val; return * ( T * ) this; }
        T & operator -= ( const T & tm )
        { val -= tm . val; return * ( T * ) this; }
        T & operator *= ( I64 m )
        { val *= m; return * ( T * ) this; }
        T & operator /= ( I64 d );
        T & operator %= ( I64 d );

        // equality and relational operators
        bool operator == ( const T & tm ) const
        { return val == tm . val; }
        bool operator != ( const T & tm ) const
        { return val != tm . val; }
        bool operator <= ( const T & tm ) const
        { return val <= tm . val; }
        bool operator >= ( const T & tm ) const
        { return val >= tm . val; }
        bool operator < ( const T & tm ) const
        { return val < tm . val; }
        bool operator > ( const T & tm ) const
        { return val > tm . val; }

    protected:

        // constructors
        tm_base_t ( I64 v )
            : val ( v ) {}

        // for good measure
        ~ tm_base_t ()
        { val = 0; }

        I64 val;
        
    };


    /*------------------------------------------------------------------
     * nS_t
     *  represents nano-seconds
     */
    class nS_t : public tm_base_t < nS_t >
    {
    public:


        // declare uninitialized
        nS_t () : tm_base_t < nS_t > ( 0 ) {}

        // create from raw integer
        nS_t ( I64 tm ) : tm_base_t < nS_t > ( tm ) {}
        void operator = ( I64 tm ) { val = tm; }

        // create from another nS_t
        nS_t ( const nS_t & tm ) : tm_base_t < nS_t > ( tm . val ) {}
        void operator = ( const nS_t & tm ) { val = tm . val; }

        // create from uS_t
        nS_t ( const uS_t & tm );
        void operator = ( const uS_t & tm );

        // create from mS_t
        nS_t ( const mS_t & tm );
        void operator = ( const mS_t & tm );

        // create from seconds
        nS_t ( const seconds_t & tm );
        void operator = ( const seconds_t & tm );
    };


    /*------------------------------------------------------------------
     * uS_t
     *  represents micro-seconds
     */
    class uS_t : public tm_base_t < uS_t >
    {
    public:


        // declare uninitialized
        uS_t () : tm_base_t < uS_t > ( 0 ) {}

        // create from raw integer
        uS_t ( I64 tm ) : tm_base_t < uS_t > ( tm ) {}
        void operator = ( I64 tm ) { val = tm; }

        // create from another uS_t
        uS_t ( const uS_t & tm ) : tm_base_t < uS_t > ( tm . val ) {}
        void operator = ( const uS_t & tm ) { val = tm . val; }

        // create from nS_t
        uS_t ( const nS_t & tm );
        void operator = ( const nS_t & tm );

        // create from mS_t
        uS_t ( const mS_t & tm );
        void operator = ( const mS_t & tm );

        // create from seconds
        uS_t ( const seconds_t & tm );
        void operator = ( const seconds_t & tm );
    };


    /*------------------------------------------------------------------
     * mS_t
     *  represents milli-seconds
     */
    class mS_t : public tm_base_t < mS_t >
    {
    public:


        // declare uninitialized
        mS_t () : tm_base_t < mS_t > ( 0 ) {}

        // create from raw integer
        mS_t ( I64 tm ) : tm_base_t < mS_t > ( tm ) {}
        void operator = ( I64 tm ) { val = tm; }

        // create from another mS_t
        mS_t ( const mS_t & tm ) : tm_base_t < mS_t > ( tm . val ) {}
        void operator = ( const mS_t & tm ) { val = tm . val; }

        // create from nS_t
        mS_t ( const nS_t & tm );
        void operator = ( const nS_t & tm );

        // create from uS_t
        mS_t ( const uS_t & tm );
        void operator = ( const uS_t & tm );

        // create from seconds
        mS_t ( const seconds_t & tm );
        void operator = ( const seconds_t & tm );
    };


    /*------------------------------------------------------------------
     * seconds_t
     *  represents whole seconds
     */
    class seconds_t : public tm_base_t < seconds_t >
    {
    public:


        // declare uninitialized
        seconds_t () : tm_base_t < seconds_t > ( 0 ) {}

        // create from raw integer
        seconds_t ( I64 tm ) : tm_base_t < seconds_t > ( tm ) {}
        void operator = ( I64 tm ) { val = tm; }

        // create from another seconds_t
        seconds_t ( const seconds_t & tm ) : tm_base_t < seconds_t > ( tm . val ) {}
        void operator = ( const seconds_t & tm ) { val = tm . val; }

        // create from nS_t
        seconds_t ( const nS_t & tm );
        void operator = ( const nS_t & tm );

        // create from uS_t
        seconds_t ( const uS_t & tm );
        void operator = ( const uS_t & tm );

        // create from mS_t
        seconds_t ( const mS_t & tm );
        void operator = ( const mS_t & tm );
    };


    /*------------------------------------------------------------------
     * timestamp_t
     *  
     */
    class timestamp_t : public nS_t
    {
    public:

        timestamp_t () {}
        timestamp_t ( const timestamp_t & tm )
            : nS_t ( tm ) {}
        void operator = ( const timestamp_t & tm )
        { nS_t :: operator = ( tm ); }

    private:

        timestamp_t ( const nS_t & tm ) : nS_t ( tm ) {}

        friend interface TimeMgrItf;
    };


    /*------------------------------------------------------------------
     * timeout_t
     *  represents a time period in mS for triggering events
     *  NB - this class has OS manifest dependencies
     */
    class timeout_t
    {
    public:

        // constructor to capture timeout delay
        timeout_t ( const nS_t & delay );

        // public destructor
        ~ timeout_t ();

        // initialization and copy
        timeout_t ( const timeout_t & tm );
        void operator = ( const timeout_t & tm );

    private:

        // called to prepare "to" for POSIX
        void prepare () const;

        nS_t delay;
#if UNIX
        mutable timestamp_t to;
        mutable atomic_t < U32 > prepared;
#endif
        friend interface TimeMgrItf;
    };
}

#endif // _hpp_vdb3_kfc_time_
