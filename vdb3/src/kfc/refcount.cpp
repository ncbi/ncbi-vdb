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

#include <kfc/refcount.hpp>
#include <kfc/callstk.hpp>
#include <kfc/atomic.hpp>
#include <kfc/except.hpp>
#include <kfc/memory.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/rsrc.hpp>

#ifndef _hpp_vdb3_kfc_memmgr_
#include <stdlib.h>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * Refcount
     *  base for reference-counted objects
     */

    void * Refcount :: operator new ( size_t bytes )
    {
        FUNC_ENTRY ();
#if ! defined _hpp_vdb3_kfc_rsrc_ || ! defined _hpp_vdb3_kfc_memmgr_
        return calloc ( 1, bytes );
#else
        assert ( rsrc != 0 );
        return rsrc -> mmgr . _new ( bytes );
#endif
    }

    void Refcount :: operator delete ( void * ptr )
    {
#ifndef _hpp_vdb3_kfc_memmgr_
        free ( ptr );
#else
        assert ( rsrc != 0 );
        return rsrc -> mmgr . _delete ( ptr );
#endif
    }

    Refcount :: Refcount ()
        : count ( 0 )
    {
    }

    Refcount :: ~ Refcount ()
    {
        count = 0;
    }

    Refcount * Refcount :: duplicate ()
    {
        if ( this != 0 )
        {
            if ( count . inc_and_test () )
            {
                -- count;
                FUNC_ENTRY ();
                CONST_THROW ( xc_references_exhausted_err, "all available references to this object have been exhausted" );
            }
        }
        return this;
    }

    void Refcount :: release ()
    {
        if ( this != 0 )
        {
            // if ( count >= 1 ) count -= 1;
            switch ( count . read_and_sub_ge ( 1, 1 ) )
            {
            case 1:
            {
                FUNC_ENTRY ();
                delete this;
                break;
            }
            case 0:
            {
                FUNC_ENTRY ();
                CONST_THROW ( xc_zombie_object_err, "attempt to release a zombie object" );
            }}
        }
    }

}
