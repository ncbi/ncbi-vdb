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

#include <kfc/array.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/caps.hpp>
#include <kfc/except.hpp>
#include <kfc/callstk.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * OpaqueArray
     *  represents a container addressable by ordinal
     */

    count_t OpaqueArray :: length () const
    {
        return cnt;
    }

    bytes_t OpaqueArray :: size () const
    {
        return m . bytes;
    }

    void * OpaqueArray :: access ( index_t offset, const bytes_t & elem_size )
    {
        if ( offset < 0 || ( U64 ) offset + elem_size > m . bytes )
        {
            FUNC_ENTRY ();

            bool okay = false;
            if ( offset >= 0 )
            {
                // try to resize the storage
            }

            if ( ! okay )
            {
                THROW ( xc_bounds_err, "memory read/write access of %lu bytes at index %ld"
                        , ( U64 ) elem_size
                        , offset / elem_size
                    );
            }
        }

        return ( void* ) ( ( char* ) m . ptr + offset );
    }

    const void * OpaqueArray :: access ( index_t offset, const bytes_t & elem_size ) const
    {
        if ( offset < 0 || ( U64 ) offset + elem_size > m . bytes )
        {
            FUNC_ENTRY ();
            THROW ( xc_bounds_err, "memory read access of %lu bytes at index %ld"
                    , ( U64 ) elem_size
                    , offset / elem_size
                );
        }

        return ( const void* ) ( ( const char* ) m . ptr + offset );
    }

    void OpaqueArray :: assign ( const OpaqueArray & r )
    {
        FUNC_ENTRY ();
        m = r . m;
        cnt = r . cnt;
    }

    OpaqueArray :: OpaqueArray ()
        : cnt ( 0 )
    {
    }

    OpaqueArray :: OpaqueArray ( const OpaqueArray & r )
        : m ( r . m )
        , cnt ( r . cnt )
    {
    }

    OpaqueArray :: OpaqueArray ( const OpaqueArray & r, caps_t reduce )
        : m ( r . m )
        , cnt ( r . cnt )
    {
    }

    OpaqueArray :: ~ OpaqueArray ()
    {
        cnt = 0;
    }

    OpaqueArray :: OpaqueArray ( Mem & _m, const bytes_t & elem_size )
        : m ( _m )
        , cnt ( 0 )
    {
        FUNC_ENTRY ();
        m . get_itf ( CAP_RDWR );

        if ( elem_size == ( U64 ) 0 )
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        if ( elem_size != ( U64 ) 1 )
        {
            if ( ( U64 ) m . bytes % elem_size != ( U64 ) 0 )
                CONST_THROW ( xc_elem_size_err, "memory block is not an integral multiple of array element size" );
        }
    }

    OpaqueArray :: OpaqueArray ( const Mem & _m, const bytes_t & elem_size )
        : m ( _m )
        , cnt ( 0 )
    {
        FUNC_ENTRY ();
        m . get_itf ( CAP_READ );

        if ( elem_size == ( U64 ) 0 )
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        cnt = ( U64 ) _m . bytes;
        if ( elem_size != ( U64 ) 1 )
        {
            if ( ( U64 ) m . bytes % elem_size != ( U64 ) 0 )
                CONST_THROW ( xc_elem_size_err, "memory block is not an integral multiple of array element size" );
            cnt /= ( U64 ) elem_size;
        }
    }

}
