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

#ifndef _hpp_vdb3_kfc_pmemmgr_
#define _hpp_vdb3_kfc_pmemmgr_

#ifndef _hpp_vdb3_kfc_memmgr_
#include <kfc/memmgr.hpp>
#endif

#include <new>

namespace vdb3
{


    /*------------------------------------------------------------------
     * PrimordMemMgr
     *  primordial memory manager
     */
    class PrimordMemMgr : implements MemMgrItf
    {
    public:

        static MemMgr make_primordial ();

        virtual Mem alloc ( const bytes_t & size, bool clear );
        virtual Mem make_const ( const void * ptr, const bytes_t & size );

    protected:

        virtual void * _alloc ( const bytes_t & size, bool clear );
        virtual void * _resize ( void * ptr, const bytes_t & old_size,
            const bytes_t & new_size, bool clear );
        virtual void _free ( void * ptr, const bytes_t & size );

        PrimordMemMgr ( const bytes_t & quota, const bytes_t & avail );
        ~ PrimordMemMgr ();

    private:

        void * operator new ( std :: size_t bytes, void * ptr )
        { return ptr; }
        void operator delete ( void * ptr );

        bytes_t quota;
        atomic_t < U64 > avail;
    };

}

#endif // _hpp_vdb3_kfc_pmemmgr_
