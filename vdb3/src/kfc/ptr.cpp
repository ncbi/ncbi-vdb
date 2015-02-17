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

#include <kfc/ptr.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/caps.hpp>
#include <kfc/except.hpp>
#include <kfc/callstk.hpp>

namespace vdb3
{

    // implement read or r/w access to object
    void * OpaquePtr :: access ( const bytes_t & elem_size )
    {
        if ( adj < ( I64 ) 0 || elem_size + ( U64 ) adj > m . bytes )
        {
            FUNC_ENTRY ();
            THROW ( xc_bounds_err, "read/write memory access of %lu bytes", ( U64 ) elem_size );
        }

        return ( void* ) ( ( char* ) m . ptr + adj );
    }

    const void * OpaquePtr :: access ( const bytes_t & elem_size ) const
    {
        if ( adj < ( I64 ) 0 || elem_size + ( U64 ) adj > m . bytes )
        {
            FUNC_ENTRY ();
            THROW ( xc_bounds_err, "read memory access of %lu bytes", ( U64 ) elem_size );
        }

        return ( const void* ) ( ( const char* ) m . ptr + adj );
    }

    void * OpaquePtr :: access ( index_t offset, const bytes_t & elem_size )
    {
        index_t new_adj ( adj + offset );
        if ( new_adj < ( I64 ) 0 || elem_size + ( U64 ) new_adj > m . bytes )
        {
            FUNC_ENTRY ();
            THROW ( xc_bounds_err, "read/write memory access of %lu bytes", ( U64 ) elem_size );
        }

        return ( void* ) ( ( char* ) m . ptr + new_adj );
    }

    const void * OpaquePtr :: access ( index_t offset, const bytes_t & elem_size ) const
    {
        index_t new_adj ( adj + offset );
        if ( new_adj < ( I64 ) 0 || elem_size + ( U64 ) new_adj > m . bytes )
        {
            FUNC_ENTRY ();
            THROW ( xc_bounds_err, "read memory access of %lu bytes", ( U64 ) elem_size );
        }

        return ( const void* ) ( ( const char* ) m . ptr + new_adj );
    }

    // increment and decrement
    void OpaquePtr :: increment ( index_t offset )
    {
        adj += offset;
    }

    void OpaquePtr :: decrement ( index_t offset )
    {
        adj -= offset;
    }

    // difference - defined for two pointers on same mem
    index_t OpaquePtr :: difference ( const OpaquePtr & p, const bytes_t & elem_size ) const
    {
        FUNC_ENTRY ();

        // test if both mem blocks are identical
        if ( m . ptr == p . m . ptr &&
             m . bytes == p . m . bytes )
        {
            if ( elem_size == ( U64 ) 0 )
                CONST_THROW ( xc_div_zero_err, "divide by zero" );
            return index_t ( ( ( I64 ) adj - p . adj ) / ( U64 ) elem_size );
        }

        // have to derive overlap, and both pointers must be within
        const char * l1 = ( const char* ) m . ptr;
        const char * l2 = ( const char* ) p . m . ptr;
        const char * r1 = l1 + m . bytes;
        const char * r2 = l2 + p . m . bytes;
        const char * p1 = l1 + adj;
        const char * p2 = l2 + p . adj;

        const char * left = ( l1 >= l2 ) ? l1 : l2;
        const char * right = ( r1 <= r2 ) ? r1 : r2;
        if ( left >= right )
            CONST_THROW ( xc_ptr_space_err, "pointer difference between disjoint memory spaces" );

        if ( p1 < left || p1 > right ||
             p2 < left || p2 > right )
            CONST_THROW ( xc_ptr_space_err, "pointer difference between disjoint memory spaces" );

        // now, the pointers can be compared
        if ( elem_size == ( U64 ) 0 )
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        return index_t ( ( p1 - p2 ) / ( U64 ) elem_size );
    }

    // comparison
    bool OpaquePtr :: equal ( const OpaquePtr & p ) const
    {
        FUNC_ENTRY ();
        return difference ( p, 1 ) == ( I64 ) 0;
    }

    bool OpaquePtr :: above ( const OpaquePtr & p ) const
    {
        FUNC_ENTRY ();
        return difference ( p, 1 ) > ( I64 ) 0;
    }

    bool OpaquePtr :: below ( const OpaquePtr & p ) const
    {
        FUNC_ENTRY ();
        return difference ( p, 1 ) < ( I64 ) 0;
    }

    // assignment
    void OpaquePtr :: assign ( const OpaquePtr & p )
    {
        m = p . m;
        adj = p . adj;
    }

    OpaquePtr :: OpaquePtr ( Mem & _m, const bytes_t & elem_size )
        : m ( _m )
        , adj ( 0 )
    {
        FUNC_ENTRY ();
        m . get_itf ( CAP_RDWR );

        if ( elem_size == ( U64 ) 0 )
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        if ( elem_size != ( U64 ) 1 )
        {
            if ( ( U64 ) m . bytes % elem_size != ( U64 ) 0 )
                CONST_THROW ( xc_ptr_size_err, "memory block is not an integral multiple of pointer element size" );
        }
    }

    OpaquePtr :: OpaquePtr ( const Mem & _m, const bytes_t & elem_size )
        : m ( _m )
        , adj ( 0 )
    {
        FUNC_ENTRY ();
        m . get_itf ( CAP_READ );

        if ( elem_size == ( U64 ) 0 )
            CONST_THROW ( xc_div_zero_err, "divide by zero" );
        if ( elem_size != ( U64 ) 1 )
        {
            if ( ( U64 ) m . bytes % elem_size != ( U64 ) 0 )
                CONST_THROW ( xc_ptr_size_err, "memory block is not an integral multiple of pointer element size" );
        }
    }

    OpaquePtr :: OpaquePtr ( const OpaquePtr & p )
        : m ( p . m )
        , adj ( p . adj )
    {
    }

    OpaquePtr :: OpaquePtr ( const OpaquePtr & p, caps_t reduce )
        : m ( p . m, reduce )
        , adj ( p . adj )
    {
    }

    OpaquePtr :: ~ OpaquePtr ()
    {
        adj = 0;
    }

}
