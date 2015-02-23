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

#include <kfc/defs.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>

namespace vdb3
{

    template < class IT, class VT >
    IT integer_t < IT, VT > :: operator / ( const IT & i ) const
    {
        if ( i . val == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return IT ( val / i . val );
    }

    template < class IT, class VT >
    IT integer_t < IT, VT > :: operator % ( const IT & i ) const
    {
        if ( i . val == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return IT ( val % i . val );
    }

    template < class IT, class VT >
    IT & integer_t < IT, VT > :: operator /= ( const IT & i )
    {
        if ( i . val == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }

        val /= i . val;
        return * ( IT * ) this;
    }

    template < class IT, class VT >
    IT & integer_t < IT, VT > :: operator %= ( const IT & i )
    {
        if ( i . val == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }

        val %= i . val;
        return * ( IT * ) this;
    }

#if ALLOW_RAW_CONSTANTS

    template < class IT, class VT >
    IT integer_t < IT, VT > :: operator / ( VT i ) const
    {
        if ( i == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return IT ( val / i );
    }

    template < class IT, class VT >
    IT integer_t < IT, VT > :: operator % ( VT i ) const
    {
        if ( i == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }
        return IT ( val % i );
    }

    template < class IT, class VT >
    IT & integer_t < IT, VT > :: operator /= ( VT i )
    {
        if ( i == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }

        val /= i;
        return * ( IT * ) this;
    }

    template < class IT, class VT >
    IT & integer_t < IT, VT > :: operator %= ( VT i )
    {
        if ( i == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        }

        val %= i;
        return * ( IT * ) this;
    }

#endif

    template class integer_t < bytes_t, U64 >;
    template class integer_t < bitsz_t, U64 >;
    
}
