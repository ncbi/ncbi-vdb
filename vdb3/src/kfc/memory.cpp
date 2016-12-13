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

#include <kfc/memory.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/caps.hpp>

#include <string.h>

namespace vdb3
{

    /*------------------------------------------------------------------
     * MemoryItf
     *  an interface representing a block of storage
     */

    MemMgrItf * MemoryItf :: get_mmgr () const
    {
        return mmgr;
    }

    void * MemoryItf :: cast ( Refcount * obj )
    {
        return ( void * ) dynamic_cast < MemoryItf * > ( obj );
    }


    /*------------------------------------------------------------------
     * Mem
     *  an object representing a range of address space
     */

    bytes_t Mem :: size () const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_PROP_READ );
        return bytes;
    }

    index_t Mem :: find_first ( U8 byte ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_READ );

        void * loc = memchr ( ptr, byte, bytes );
        if ( loc != 0 )
            return ( char * ) loc - ( char * ) ptr;

        return ( I64 ) -1;
    }

    index_t Mem :: find_first ( U8 byte, index_t offset ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_READ );

        if ( offset < ( I64 ) 0 || ( U64 ) offset >= bytes )
            return -1;

        const char * p = ( const char * ) ptr;
        void * loc = memchr ( & p [ offset ], byte, bytes - ( U64 ) offset );
        if ( loc != 0 )
            return ( const char * ) loc - p;

        return ( I64 ) -1;
    }

    index_t Mem :: find_first ( U8 byte, index_t offset, const bytes_t & _sz ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_READ );

        size_t sz = _sz;

        if ( offset < ( I64 ) 0 || ( U64 ) offset >= bytes )
            return -1;

        if ( sz + offset > bytes )
            sz = bytes - ( U64 ) offset;

        const char * p = ( const char * ) ptr;
        void * loc = memchr ( & p [ offset ], byte, sz );
        if ( loc != 0 )
            return ( const char * ) loc - p;

        return ( I64 ) -1;
    }

    Mem Mem :: subrange ( index_t offset ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_SUBRANGE );

        Mem m ( * this );
        if ( offset < ( I64 ) 0 || ( U64 ) offset >= bytes )
        {
            m . ptr = ( void* ) & ( ( char* ) ptr ) [ bytes ];
            m . bytes = ( U64 ) 0;
        }
        else
        {
            m . ptr = ( void* ) & ( ( char* ) ptr ) [ offset ];
            m . bytes = ( U64 ) bytes - ( U64 ) offset;
        }

        return m;
    }

    Mem Mem :: subrange ( index_t offset, const bytes_t & _sz ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_SUBRANGE );

        U64 sz = _sz;

        Mem m ( * this );

        if ( offset < ( I64 ) 0 || ( U64 ) offset >= bytes )
        {
            m . ptr = ( void* ) & ( ( char* ) ptr ) [ bytes ];
            m . bytes = ( U64 ) 0;
        }
        else
        {
            U64 b = ( U64 ) bytes - ( U64 ) offset;
            if ( sz > b )
                sz = b;

            m . ptr = ( void* ) & ( ( char* ) ptr ) [ offset ];
            m . bytes = sz;
        }

        return m;
    }

    void Mem :: resize ( const bytes_t & sz, bool clear )
    {
        FUNC_ENTRY ();
        test_caps ( CAP_RESIZE );

        // early cheap detection of noop
        if ( sz == bytes )
            return;

        // a null ref can be resized to allocate
        if ( ptr == 0 )
        {
            assert ( bytes == ( U64 ) 0 );
#ifdef _hpp_vdb3_rsrc_
            * this = rsrc -> mmgr . alloc ( sz, clear );
#else
            CONST_THROW ( xc_unimplemented_err, "unimplemented" );
#endif
        }
        else
        {
            assert ( bytes != ( U64 ) 0 );

            MemoryItf * itf = get_itf ( CAP_RESIZE );

            // there are cases when the obj can be null
            if ( itf == 0 )
            {
#ifdef _hpp_vdb3_rsrc_
                // ref represents constant data, not heap data
                Mem tmp = rsrc -> mmgr . alloc ( sz, false );

                // copy in data from self
                bytes_t num_writ = tmp . copy ( bytes, 0, * this, 0 );
                if ( bytes >= sz )
                    assert ( num_writ == sz );
                else
                {
                    assert ( num_writ == bytes );
                    if ( clear )
                    {
                        bytes_t num_zeroed = tmp . fill ( ( U64 ) sz - bytes, ( I64 ) ( U64 ) num_writ, 0 );
                        assert ( num_writ + num_zeroed == sz );
                    }
                }

                * this = tmp;
#else
                CONST_THROW ( xc_unimplemented_err, "unimplemented" );
#endif
            }
            else
            {
                // normal case
                itf -> resize ( sz, clear );
                ptr = itf -> get_mapped_memory ( & bytes );
            }
        }
    }

    bytes_t Mem :: copy ( const bytes_t & _amount,
        index_t dst_offset, const Mem & src, index_t src_offset )
    {
        FUNC_ENTRY ();
        test_caps ( CAP_WRITE );
        src . test_caps ( CAP_READ );

        U64 amount = _amount;

        if ( dst_offset < 0 || ( U64 ) dst_offset >= bytes ||
             src_offset < 0 || ( U64 ) src_offset >= src . bytes )
            return bytes_t ( 0 );

        if ( amount + dst_offset > bytes )
            amount = ( U64 ) bytes - dst_offset;
        if ( amount + src_offset > src . bytes )
            amount = ( U64 ) src . bytes - src_offset;

        if ( amount != 0 )
        {
            char * dp = ( char * ) ptr;
            const char * sp = ( const char * ) src . ptr;
            memmove ( & dp [ dst_offset ], & sp [ src_offset ], ( size_t ) amount );
        }

        return bytes_t ( amount );
    }

    CopyUntilRslt Mem :: copy_until ( const bytes_t & _amount,
        U8 stop_byte, index_t dst_offset, const Mem & src, index_t src_offset )
    {
        FUNC_ENTRY ();
        test_caps ( CAP_WRITE );
        src . test_caps ( CAP_READ );

        U64 amount = _amount;
        bool found = false;

        if ( dst_offset < 0 || ( U64 ) dst_offset >= bytes ||
             src_offset < 0 || ( U64 ) src_offset >= src . bytes )
            return CopyUntilRslt ( 0, false );

        if ( amount + dst_offset > bytes )
            amount = ( U64 ) bytes - dst_offset;
        if ( amount + src_offset > src . bytes )
            amount = ( U64 ) src . bytes - src_offset;

        if ( amount != 0 )
        {
            U8 * dp = ( U8 * ) ptr + dst_offset;
            const U8 * sp = ( const U8 * ) src . ptr + src_offset;
            for ( U64 i = 0; i < amount; ++ i )
            {
                if ( ( dp [ i ] = sp [ i ] ) == stop_byte )
                {
                    amount = i + 1;
                    found = true;
                    break;
                }
            }
        }

        return CopyUntilRslt ( amount, found );
    }

    bytes_t Mem :: fill ( count_t repeat, index_t offset, U8 byte )
    {
        FUNC_ENTRY ();
        test_caps ( CAP_WRITE );

        if ( offset < 0 || ( U64 ) offset >= bytes )
            return bytes_t ( 0 );

        if ( repeat + offset > bytes )
            repeat = ( U64 ) bytes - offset;

        if ( repeat != 0 )
        {
            char * p = ( char * ) ptr;
            memset ( & p [ offset ], byte, ( size_t ) repeat );
        }

        return bytes_t ( repeat );
    }

    bool Mem :: operator == ( const Mem & m ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_READ );
        m . test_caps ( CAP_READ );

        if ( bytes == m . bytes )
        {
            if ( ptr == m . ptr )
                return true;
            return memcmp ( ptr, m . ptr, bytes ) == 0;
        }

        return false;
    }

    bool Mem :: operator != ( const Mem & m ) const
    {
        FUNC_ENTRY ();
        test_caps ( CAP_READ );
        m . test_caps ( CAP_READ );

        if ( bytes == m . bytes )
        {
            if ( ptr == m . ptr )
                return false;
            return memcmp ( ptr, m . ptr, bytes ) != 0;
        }

        return true;
    }

    Mem :: Mem ()
        : ptr ( 0 )
        , bytes ( 0 )
    {
    }

    Mem :: ~ Mem ()
    {
        ptr = 0;
        bytes = 0;
    }

    Mem :: Mem ( const Mem & r )
        : Ref < MemoryItf > ( r )
        , ptr ( r . ptr )
        , bytes ( r . bytes )
    {
    }

    void Mem :: operator = ( const Mem & r )
    {
        Ref < MemoryItf > :: operator = ( r );
        ptr = r . ptr;
        bytes = r . bytes;
    }

    Mem :: Mem ( const Mem & r, caps_t reduce )
        : Ref < MemoryItf > ( r, reduce )
        , ptr ( r . ptr )
        , bytes ( r . bytes )
    {
    }

    Mem :: Mem ( Refcount * obj, MemoryItf * itf, caps_t caps )
        : Ref < MemoryItf > ( obj, itf, caps )
        , ptr ( 0 )
        , bytes ( 0 )
    {
        if ( itf != 0 )
            ptr = itf -> get_mapped_memory ( & bytes );
    }

#if 0
    Mem :: Mem ( const void * _ptr, const bytes_t & sz )
        : Ref < MemoryItf > ( 0, CAP_READ | CAP_PROP_READ | CAP_CAST )
        , ptr ( ( void * ) _ptr )
        , bytes ( sz )
    {
        if ( _ptr == 0 )
            bytes = ( U64 ) 0;
    }
#endif
}
