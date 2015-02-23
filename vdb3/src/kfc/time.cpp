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

#include <kfc/time.hpp>
#include <kfc/timemgr.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/callstk.hpp>

namespace vdb3
{

    template < class T >
    I64 round ( const T & tm, I64 d )
    {
        I64 val = tm;
        if ( val >= 0 )
            return ( val + ( d / 2 ) ) / d;
        return ( val - ( d / 2 ) ) / d;
    }


    /*------------------------------------------------------------------
     * tm_base_t
     */

    template < class T >
    T tm_base_t < T > :: operator / ( I64 d ) const
    {
        if ( d == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return T ( val / d );
    }

    template < class T >
    T tm_base_t < T > :: operator % ( I64 d ) const
    {
        if ( d == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return T ( val % d );
    }

    template < class T >
    T & tm_base_t < T > :: operator /= ( I64 d )
    {
        if ( d == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        val /= d;
        return * ( T * ) this;
    }

    template < class T >
    T & tm_base_t < T > :: operator %= ( I64 d )
    {
        if ( d == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        val %= d;
        return * ( T * ) this;
    }


    /*------------------------------------------------------------------
     * nS_t
     *  represents nano-seconds
     */

    nS_t :: nS_t ( const uS_t & tm )
        : tm_base_t < nS_t > ( ( I64 ) tm * 1000 )
    {
    }

    void nS_t :: operator = ( const uS_t & tm )
    {
        val = ( I64 ) tm * 1000;
    }

    nS_t :: nS_t ( const mS_t & tm )
        : tm_base_t < nS_t > ( ( I64 ) tm * 1000000 )
    {
    }

    void nS_t :: operator = ( const mS_t & tm )
    {
        val = ( I64 ) tm * 1000000;
    }

    nS_t :: nS_t ( const seconds_t & tm )
        : tm_base_t < nS_t > ( ( I64 ) tm * 1000000000 )
    {
    }

    void nS_t :: operator = ( const seconds_t & tm )
    {
        val = ( I64 ) tm * 1000000000;
    }



    /*------------------------------------------------------------------
     * uS_t
     *  represents micro-seconds
     */

    uS_t :: uS_t ( const nS_t & tm )
        : tm_base_t < uS_t > ( round ( tm, 1000 ) )
    {
    }

    void uS_t :: operator = ( const nS_t & tm )
    {
        val = round ( tm, 1000 );
    }

    uS_t :: uS_t ( const mS_t & tm )
        : tm_base_t < uS_t > ( ( I64 ) tm * 1000 )
    {
    }

    void uS_t :: operator = ( const mS_t & tm )
    {
        val = ( I64 ) tm * 1000;
    }

    uS_t :: uS_t ( const seconds_t & tm )
        : tm_base_t < uS_t > ( ( I64 ) tm * 1000000 )
    {
    }

    void uS_t :: operator = ( const seconds_t & tm )
    {
        val = ( I64 ) tm * 1000000;
    }



    /*------------------------------------------------------------------
     * mS_t
     *  represents milli-seconds
     */
    mS_t :: mS_t ( const nS_t & tm )
        : tm_base_t < mS_t > ( round ( tm, 1000000 ) )
    {
    }

    void mS_t :: operator = ( const nS_t & tm )
    {
        val = round ( tm, 1000000 );
    }

    mS_t :: mS_t ( const uS_t & tm )
        : tm_base_t < mS_t > ( round ( tm, 1000 ) )
    {
    }

    void mS_t :: operator = ( const uS_t & tm )
    {
        val = round ( tm, 1000 );
    }

    mS_t :: mS_t ( const seconds_t & tm )
        : tm_base_t < mS_t > ( ( I64 ) tm * 1000 )
    {
    }

    void mS_t :: operator = ( const seconds_t & tm )
    {
        val = ( I64 ) tm * 1000;
    }



    /*------------------------------------------------------------------
     * seconds_t
     *  represents whole seconds
     */
    seconds_t :: seconds_t ( const nS_t & tm )
        : tm_base_t < seconds_t > ( round ( tm, 1000000000 ) )
    {
    }

    void seconds_t :: operator = ( const nS_t & tm )
    {
        val = round ( tm, 1000000000 );
    }

    seconds_t :: seconds_t ( const uS_t & tm )
        : tm_base_t < seconds_t > ( round ( tm, 1000000 ) )
    {
    }

    void seconds_t :: operator = ( const uS_t & tm )
    {
        val = round ( tm, 1000000 );
    }

    seconds_t :: seconds_t ( const mS_t & tm )
        : tm_base_t < seconds_t > ( round ( tm, 1000 ) )
    {
    }

    void seconds_t :: operator = ( const mS_t & tm )
    {
        val = round ( tm, 1000 );
    }


    /*------------------------------------------------------------------
     * timeout_t
     *  represents a time period in mS for triggering events
     *  NB - this class has OS manifest dependencies
     */

    timeout_t :: timeout_t ( const nS_t & _delay )
        : delay ( _delay )
#if UNIX
        , prepared ( 0 )
#endif
    {
    }

    timeout_t :: ~ timeout_t ()
    {
#if UNIX
        prepared = 0;
#endif
    }

    timeout_t :: timeout_t ( const timeout_t & tm )
        : delay ( tm . delay )
#if UNIX
        , to ( tm . to )
        , prepared ( tm . prepared )
#endif
    {
    }

    void timeout_t :: operator = ( const timeout_t & tm )
    {
        delay = tm . delay;
#if UNIX
        to = tm . to;
        prepared = tm . prepared;
#endif
    }

    void timeout_t :: prepare () const
    {
#if UNIX
        // only POSIX timeouts need preparation
        if ( prepared . test_and_set ( 0, 1 ) == 0 )
        {
            rsrc -> tmmgr . prepare_timeout ( * this );
        }
#endif
    }
}
