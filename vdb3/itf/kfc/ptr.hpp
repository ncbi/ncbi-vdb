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

#ifndef _hpp_vdb3_kfc_ptr_
#define _hpp_vdb3_kfc_ptr_

/*
#include <kfc/except.hpp>
#include <kfc/memory.hpp>
#include <kfc/ref.hpp>
#include <kfc/string.hpp>
*/

namespace vdb3
{

    /*------------------------------------------------------------------
     * exceptions
     */

    class new_xc : vdb3::exception
    {
      public:
        new_xc ( vdb3 :: U32 lineno, const vdb3 :: String & msg ) : logic_err( lineno, msg )
        {}
    }

    XC_DECLARE ( xc_ptr_space_err, logic_err );
    XC_DECLARE ( xc_ptr_size_err, xc_elem_size_err );


    /*------------------------------------------------------------------
     * OpaquePtr
     *  type-opaque pointer
     *  does all the actual work
     */
    class OpaquePtr
    {
    protected:

        // implement read or r/w access to object
        void * access ( const bytes_t & elem_size );
        const void * access ( const bytes_t & elem_size ) const;
        void * access ( index_t offset, const bytes_t & elem_size );
        const void * access ( index_t offset, const bytes_t & elem_size ) const;

        // increment and decrement
        void increment ( index_t offset );
        void decrement ( index_t offset );

        // arithmetic - defined for two pointers on same mem
        index_t difference ( const OpaquePtr & p, const bytes_t & elem_size ) const;

        // comparison
        bool equal ( const OpaquePtr & p ) const;
        bool above ( const OpaquePtr & p ) const;
        bool below ( const OpaquePtr & p ) const;

        // assignment
        void assign ( const OpaquePtr & p );

        // construct from raw memory and element size
        OpaquePtr ( Mem & m, const bytes_t & elem_size );
        OpaquePtr ( const Mem & m, const bytes_t & elem_size );

        // C++
        OpaquePtr ( const OpaquePtr & p );
        OpaquePtr ( const OpaquePtr & p, caps_t reduce );
        ~ OpaquePtr ();

    private:

        Mem m;
        index_t adj;
    };


    /*------------------------------------------------------------------
     * Ptr < T >
     *  a typed pointer
     *  obtains all of its actual behavior from OpaquePtr
     */
    template < class T >
    class Ptr : OpaquePtr
    {
    public:

        // access
        T & operator * ()
        { return * ( T * ) access ( sizeof ( T ) ); }
        const T & operator * () const
        { return * ( const T * ) access ( sizeof ( T ) ); }
        T & operator [] ( index_t idx )
        { return * ( T * ) access ( idx * sizeof ( T ), sizeof ( T ) ); }
        const T & operator [] ( index_t idx ) const
        { return * ( const T * ) access ( idx * sizeof ( T ), sizeof ( T ) ); }

        // increment/decrement - NOT returning ref, no postfix
        void operator ++ ()
        { increment ( sizeof ( T ) ); }
        void operator -- ()
        { decrement ( sizeof ( T ) ); }

        // arithmetic - again not returning ref
        void operator += ( index_t offset )
        { increment ( offset * sizeof ( T ) ); }
        void operator -= ( index_t offset )
        { decrement ( offset * sizeof ( T ) ); }

        // more arithmetic
        Ptr < T > operator + ( index_t offset ) const
        {
            Ptr < T > p ( * this );
            p += offset;
            return p;
        }
        Ptr < T > operator - ( index_t offset ) const
        {
            Ptr < T > p ( * this );
            p -= offset;
            return p;
        }

        // difference - defined for two pointers on same mem
        index_t operator - ( const Ptr < T > & p ) const
        { return difference ( p, sizeof ( T ) ); }

        // equality and relational operators
        bool operator == ( const Ptr < T > & p ) const
        { return equal ( p ); }
        bool operator != ( const Ptr < T > & p ) const
        { return ! equal ( p ); }
        bool operator <= ( const Ptr < T > & p ) const
        { return ! above ( p ); }
        bool operator >= ( const Ptr < T > & p ) const
        { return ! below ( p ); }
        bool operator < ( const Ptr < T > & p ) const
        { return below ( p ); }
        bool operator > ( const Ptr < T > & p ) const
        { return above ( p ); }

        // C++
        Ptr ( const Ptr < T > & p )
            : OpaquePtr ( p ) {}
        void operator = ( const Ptr < T > & p )
            { OpaquePtr :: assign ( p ); }
        Ptr ( const Ptr < T > & p, caps_t reduce )
            : OpaquePtr ( p, reduce ) {}

        // creation from a memory block
        Ptr ( Mem & m )
            : OpaquePtr ( m, sizeof ( T ) ) {}
        Ptr ( const Mem & m )
            : OpaquePtr ( m, sizeof ( T ) ) {}
    };
}
#endif // _hpp_vdb3_kfc_ptr_
