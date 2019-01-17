/*==============================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any rem_striction on its use or reproduction.
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

#ifndef _hpp_jwt_vector_impl_
#define _hpp_jwt_vector_impl_

#include <jwt/jwt-vector.hpp>

#include <cstring>

namespace ncbi
{
    template < typename T > JwtVector < T > :: JwtVector()
    :   m_v ( nullptr ),
        m_elems ( 0 )
    {
    }

    template < typename T > JwtVector < T > :: JwtVector(const JwtVector& p_v)
    :   m_v ( ( T * ) calloc ( p_v .  m_elems, sizeof ( T ) ) ),
        m_elems ( p_v . m_elems )
    {
        //TODO: throw if m_v == nullptr
        for (size_t i = 0; i < m_elems; ++i )
        {
            // a placement ctor does not compile for scalars
            // hope all 0s constitute a valid value for lhs
            m_v [ i ] = p_v [ i ];
        }
    }

    template < typename T > JwtVector < T > :: ~JwtVector()
    {
        clear();
    }

    template < typename T > T& JwtVector < T > :: operator[] (size_t n)
    {
        //NB: undefined behavior if n is out of range
        return m_v[n];
    }

    template < typename T > const T& JwtVector < T > :: operator[] (size_t n) const
    {
        //NB: undefined behavior if n is out of range
        return m_v[n];
    }

    template < typename T > void JwtVector < T > :: push_back (const T& val)
    {
        m_v = ( T * ) realloc ( m_v, sizeof ( T ) * ( m_elems + 1 ) );
        //TODO: throw if m_v == nullptr
        // a placement ctor does not compile for scalars
        // hope all 0s constitute a valid value for lhs
        memset ( m_v + m_elems, 0, sizeof ( T ) );
        m_v [ m_elems ] = val;
        ++m_elems;
    }

    template < typename T > void JwtVector < T > :: clear()
    {
        for (size_t i = 0; i < m_elems; ++i )
        {
           m_v [ i ] . ~T ();
        }
        free ( m_v );
        m_v = nullptr;
        m_elems = 0;
    }

    template < typename T > T& JwtVector < T > :: back()
    {
        //NB: undefined behavior if empty
        return m_v [ m_elems - 1 ];
    }

    template < typename T > const T& JwtVector < T > :: back() const
    {
        //NB: undefined behavior if empty
        return m_v [ m_elems - 1 ];
    }

    template < typename T > void JwtVector < T > :: pop_back()
    {
        //NB: undefined behavior if empty
        m_v [ m_elems - 1 ] . ~T ();
        --m_elems;
    }
}

#endif
