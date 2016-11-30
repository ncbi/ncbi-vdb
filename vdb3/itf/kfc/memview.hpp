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

#ifndef _hpp_vdb3_kfc_memview_
#define _hpp_vdb3_kfc_memview_

#include <kfc/except.hpp>
#include <kfc/refcount.hpp>
#include <kfc/ref.hpp>
#include <kfc/time.hpp>


namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class MemView;
    // interface MemoryItf;


    /*------------------------------------------------------------------
     * exceptions
     */


    /**-----------------------------------------------------------------
     * MemViewItf
     *  memory view interface
     *  Non copying resizable view of an underlying, refcounted contiguous
     *  sequence of characters
     */
    interface MemViewItf : public Refcount // just means struct
    {
      public:
          virtual foo =0;
          // operator ==, !=, <, >, <=, >=, bool (not empty)
          // stream operator<<
          // constructors from memregion, std::string&, ncbi string&
          // operator [], at, front, back, data
          // clear, remove_prefix, remove_suffix, trim
          // compare, find, rfind, find_first_of, find_last_of
          // substr, size(), length(), empty()
          // iterators?


    protected:
    };


    /*------------------------------------------------------------------
     * MemView
     *  Memory view implementation
     */
    class MemView : public Ref < MemViewItf >
    {
    public:
        MemView ();
        MemView ( const MemView & r );
        void operator = ( const MemView & r );
        MemView ( const MemView & r, caps_t reduce );

    private:
    };

}

#endif // _hpp_vdb3_kfc_memview
