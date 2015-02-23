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

#ifndef _hpp_vdb3_kfc_array_
#define _hpp_vdb3_kfc_array_

#ifndef _hpp_vdb3_kfc_memory_
#include <kfc/memory.hpp>
#endif

#ifndef _hpp_vdb3_kfc_except_
#include <kfc/except.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */


    /*------------------------------------------------------------------
     * OpaqueArray
     *  represents a container addressable by ordinal
     */
    class OpaqueArray
    {
    public:

        // the number of elements actively stored
        count_t length () const;

    protected:

        // the storage capacity
        bytes_t size () const;

        // implement read or r/w access to object
        void * access ( index_t offset, const bytes_t & elem_size );
        const void * access ( index_t offset, const bytes_t & elem_size ) const;

        // assignment
        void assign ( const OpaqueArray & r );

        // C++
        OpaqueArray ();
        OpaqueArray ( const OpaqueArray & r );
        OpaqueArray ( const OpaqueArray & r, caps_t reduce );
        ~ OpaqueArray ();

        // factory
        OpaqueArray ( Mem & m, const bytes_t & elem_size );
        OpaqueArray ( const Mem & m, const bytes_t & elem_size );

    private:

        Mem m;
        count_t cnt;
    };



    /*------------------------------------------------------------------
     * Array < T >
     *  represents a container addressable by ordinal index
     */
    template < class T >
    class Array : public OpaqueArray
    {
    public:

        // element size
        bytes_t elem_size () const
        { return bytes_t (  sizeof ( T ) ); }

        // array capacity
        count_t capacity () const
        { return OpaqueArray :: size () / sizeof ( T ); }

        // access
        T & operator [] ( index_t idx )
        { return * ( T * ) access ( idx * sizeof ( T ), sizeof ( T ) ); }
        const T & operator [] ( index_t idx ) const
        { return * ( const T * ) access ( idx * sizeof ( T ), sizeof ( T ) ); }

        // C++
        Array () {}
        Array ( const Array < T > & a )
            : OpaqueArray ( a ) {}
        void operator = ( const Array < T > & a )
            { OpaqueArray :: assign ( a ); }
        Array ( const Array < T > & a, caps_t reduce )
            : OpaqueArray ( a, reduce ) {}

        // creation from a memory block
        Array ( Mem & m )
            : OpaqueArray ( m, sizeof ( T ) ) {}
        Array ( const Mem & m )
            : OpaqueArray ( m, sizeof ( T ) ) {}
    };
}

#endif // _hpp_vdb3_kfc_array_
