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

#ifndef _hpp_vdb3_kfc_ref_
#define _hpp_vdb3_kfc_ref_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class Refcount;


    /*------------------------------------------------------------------
     * OpaqueRef
     *  type-opaque reference
     *  does all the actual work
     */
    class OpaqueRef
    {
    protected:

        // ensure at least the required caps
        // throws an exception otherwise
        void test_caps ( caps_t required ) const;

        // dynamic casting support
        static void * cast ( const OpaqueRef & from,
            void * ( * cast_func ) ( Refcount * obj ) );

        // throws an exception describing the cast
        static void bad_cast ( const OpaqueRef & from, const char * func, U32 lineno );

        // create a null reference
        // can only be repaired by assignment
        OpaqueRef ();

        // release the object
        ~ OpaqueRef ();

        // standard initialization and assignment
        OpaqueRef ( const OpaqueRef & r );
        void operator = ( const OpaqueRef & r );

        // reduced capability initialization
        OpaqueRef ( const OpaqueRef & r, caps_t reduce );
        
        // factory initialization
        OpaqueRef ( Refcount * obj, caps_t caps );

    private:

        // the object is reference-counted
        Refcount * obj;

        // we store object capabilities
        caps_t caps;

    };


    /*------------------------------------------------------------------
     * Ref < T >
     *  a typed reference
     *  obtains all of its actual behavior from OpaqueRef
     */
    template < class T >
    class Ref : public OpaqueRef
    {
    public:

        // test for null
        bool null_ref () const
        { return itf == 0; }
        bool operator ! () const
        { return itf == 0; }

    protected:

        T * get_itf ( caps_t required ) const
        {
            test_caps ( required );
            return itf;
        }

        Ref ()
            : itf ( 0 ) {}
        ~ Ref ()
        { itf = 0; }

        Ref ( const Ref < T > & r )
            : OpaqueRef ( r )
            , itf ( r . itf ) {}
        Ref ( const Ref < T > & r, caps_t reduce )
            : OpaqueRef ( r, reduce )
            , itf ( r . itf ) {}
        Ref ( T * obj, caps_t caps )
            : OpaqueRef ( obj, caps )
            , itf ( obj ) {}
        Ref ( Refcount * obj, T * _itf, caps_t caps )
            : OpaqueRef ( obj, caps )
            , itf ( _itf ) {}

        void operator = ( const Ref < T > & r )
        {
            OpaqueRef :: operator = ( r );
            itf = r . itf;
        }

        // dynamic casting
        Ref ( const OpaqueRef & r, void * ( * cast_func ) ( Refcount * obj ) )
            : OpaqueRef ( r )
            , itf ( ( T * ) OpaqueRef :: cast ( r, cast_func ) )
        {
            if ( itf == 0 )
                OpaqueRef :: bad_cast ( r, __PRETTY_FUNCTION__, __LINE__ );
        }

    private:

        T * itf;
    };

}

#endif // _hpp_vdb3_kfc_ref_
