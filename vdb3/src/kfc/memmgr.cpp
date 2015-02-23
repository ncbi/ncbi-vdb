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

#include <kfc/memmgr.hpp>
#include <kfc/memory.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/caps.hpp>
#include <kfc/callstk.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * MemMgrItf
     *  memory manager interface
     */

    // support for C++ new and delete
    void * MemMgrItf :: _new ( size_t bytes )
    {
        FUNC_ENTRY ();

        // allocate new object plus header size
        Refcount * obj = ( Refcount * ) _alloc ( bytes, true );
        obj -> mmgr = ( MemMgrItf * ) duplicate ();
        obj -> obj_size = bytes;

        // return allocation
        return ( void * ) obj;
    }

    void MemMgrItf :: _delete ( void * ptr )
    {
        Refcount * obj;

        if ( ( size_t ) ptr >= sizeof * obj )
        {
            FUNC_ENTRY ();

            // convert pointer back to Refcount
            obj = ( Refcount * ) ptr;

            // recover the original memory manager interface
            MemMgrItf * self = obj -> mmgr;
            assert ( self != 0 );

            // free the memory
            self -> _free ( ( void * ) obj, obj -> obj_size );

            // detach from memmgr
            self -> release ();
        }
    }

    MemMgr MemMgrItf :: make_mmgr_ref ( Refcount * obj, caps_t caps )
    {
        // TBD - can set max capabilities here
        return MemMgr ( obj, this, caps );
    }

    Mem MemMgrItf :: make_mem_ref ( Refcount * obj, MemoryItf * itf, caps_t caps )
    {
        // TBD - can set max capabilities here
        return Mem ( obj, itf, caps );
    }


    /*------------------------------------------------------------------
     * MemMgr
     *  memory manager reference
     */

    // allocate memory 
    Mem MemMgr :: alloc ( const bytes_t & size, bool clear ) const
    {
        FUNC_ENTRY ();
        MemMgrItf * itf = get_itf ( CAP_ALLOC );
        return itf -> alloc ( size, clear );
    }


    // make a block of constant memory
    Mem MemMgr :: make_const ( const void * ptr, const bytes_t & size ) const
    {
        FUNC_ENTRY ();

        if ( ptr == 0 && size != ( U64 ) 0 )
        {
            if ( rsrc == 0 )
                throw "null const memory block";
            THROW ( xc_null_param_err, "null pointer with size %lu", ( U64 ) size );
        }

        MemMgrItf * itf = get_itf ( CAP_ALLOC );
        return itf -> make_const ( ptr, size );
    }

    // C++
    MemMgr :: MemMgr ()
    {
    }

    MemMgr :: MemMgr ( const MemMgr & r )
        : Ref < MemMgrItf > ( r )
    {
    }

    void MemMgr :: operator = ( const MemMgr & r )
    {
        Ref < MemMgrItf > :: operator = ( r );
    }

    MemMgr :: MemMgr ( const MemMgr & r, caps_t reduce )
        : Ref < MemMgrItf > ( r, reduce )
    {
    }

    // support for "new" and "delete" operators
    void * MemMgr :: _new ( size_t bytes ) const
    {
        FUNC_ENTRY ();
        MemMgrItf * itf = get_itf ( CAP_ALLOC );
        return itf -> _new ( bytes );
    }

    void MemMgr :: _delete ( void * ptr ) const
    {
        FUNC_ENTRY ();
        return MemMgrItf :: _delete ( ptr );
    }

    // initialization by MemMgrItf
    MemMgr :: MemMgr ( Refcount * obj, MemMgrItf * itf, caps_t caps )
        : Ref < MemMgrItf > ( obj, itf, caps )
    {
    }


}
