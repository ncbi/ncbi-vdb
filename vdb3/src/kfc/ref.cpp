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

#include <kfc/ref.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>
#include <kfc/refcount.hpp>

#include <typeinfo>
#include <string.h>

namespace vdb3
{

    // construct a src_loc for hpp
    static SrcLoc hpp_src_loc =
    {
        __mod__, __file__, "hpp"
    };

    static inline
    size_t safe_strlen ( const char * str )
    {
        if ( str != 0 )
            return strlen ( str );
        return 0;
    }

    /*------------------------------------------------------------------
     * OpaqueRef
     *  type-opaque reference
     *  does all the actual work
     */

    void OpaqueRef :: test_caps ( caps_t required ) const
    {
        // test caps against required
        if ( ( caps & required ) != required )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_caps_violation_err, "insufficient capabilities for this operation" );
        }
    }

    void * OpaqueRef :: cast ( const OpaqueRef & from,
        void * ( * cast_func ) ( Refcount * obj ) )
    {
        FUNC_ENTRY ();
        assert ( cast_func != 0 );
        return ( * cast_func ) ( from . obj );
    }

    // throws an exception describing the cast
    void OpaqueRef :: bad_cast ( const OpaqueRef & from, const char * func, U32 lineno )
    {
        if ( from . obj != 0 )
        {
            // push hpp onto call stack
            CallStk hpp ( hpp_src_loc, func );
            ConstString func_str ( func, safe_strlen ( func ) );

            // get actual type of object
            const char * obj_class = typeid ( * from . obj ) . name ();
            ConstString obj_class_str ( obj_class, safe_strlen ( obj_class ) );

            // create the message string
            StringBuffer msg ( "constructor '%s' failed to cast "
                               "from implementation type '%s'",
                               & func_str, & obj_class_str );

            // now throw the exception
            throw xc_unsupported_interface_err ( lineno, msg . to_str () );
        }
    }

    OpaqueRef :: OpaqueRef ()
        : obj ( 0 )
        , caps ( 0 )
    {
    }

    OpaqueRef :: ~ OpaqueRef ()
    {
        obj -> release ();
        obj = 0;
        caps = 0;
    }

    OpaqueRef :: OpaqueRef ( const OpaqueRef & r )
        : obj ( r . obj -> duplicate () )
        , caps ( r . caps )
    {
    }

    void OpaqueRef :: operator = ( const OpaqueRef & r )
    {
        if ( obj != r . obj )
        {
            Refcount * dup = r . obj -> duplicate ();
            obj -> release ();
            obj = dup;
        }
        caps = r . caps;
    }

    OpaqueRef :: OpaqueRef ( const OpaqueRef & r, caps_t reduce )
        : obj ( r . obj -> duplicate () )
        , caps ( r . caps & ~ reduce )
    {
    }

    OpaqueRef :: OpaqueRef ( Refcount * o, caps_t c )
        : obj ( o -> duplicate () )
        , caps ( c )
    {
    }
}
