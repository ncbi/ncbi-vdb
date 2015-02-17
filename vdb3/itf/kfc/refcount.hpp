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

#ifndef _hpp_vdb3_kfc_refcount_
#define _hpp_vdb3_kfc_refcount_

#ifndef _hpp_vdb3_kfc_atomic_
#include <kfc/atomic.hpp>
#endif

#include <new>

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    interface MemMgrItf;


    /*------------------------------------------------------------------
     * Refcount
     *  base for reference-counted objects
     */
    class Refcount
    {
    public:

        // interface to current MemMgr
        void * operator new ( std :: size_t bytes );

        // return the count at any given moment
        // if the object can be shared between threads,
        // only a count of 1 is meaningful
        count_t getCount () const
        { return count_t ( ( U64 ) count ); }

    protected:

        // interface to embedded MemMgr
        void operator delete ( void * ptr );

        Refcount ();
        virtual ~ Refcount ();

    private:

        // reference count manipulation
        // null-safe
        Refcount * duplicate ();
        void release ();

        // memory manager that made this object
        MemMgrItf * mmgr;

        // object size
        size_t obj_size;

        // reference count
        atomic_t < U64 > count;

        friend class PrimordMemMgr;
        friend class OpaqueRef;
        friend interface MemoryItf;
        friend interface MemMgrItf;
    };
    
}

#endif // _hpp_vdb3_kfc_refcount_
