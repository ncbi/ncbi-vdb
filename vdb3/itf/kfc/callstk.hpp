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

#ifndef _hpp_vdb3_kfc_callstk_
#define _hpp_vdb3_kfc_callstk_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards and externs
     */

    class CallStk;
    extern __thread CallStk const * callstk;


    /*------------------------------------------------------------------
     * SrcLoc
     *  a structure to identify source file
     */
    struct SrcLoc
    {
        const char * mod;
        const char * file;
        const char * ext;
    };

    // an automatic mechanism for
    // creating a single static object per source
#ifndef SRC_LOC_DEFINED
#define SRC_LOC_DEFINED
    static SrcLoc s_src_loc = { __mod__, __file__, __fext__ };
#endif


    /*------------------------------------------------------------------
     * CallStk
     *  identifies location of current frame
     *  chained to caller
     */
    class CallStk
    {
    public:

        // when caller == 0, represents top frame of stack
        const CallStk * caller;

        // stack depth
        // declared "volatile" to keep optimizer from pruning
        count_t depth;

        // function location
        const SrcLoc & src;
        const char volatile * func;

        // stacking constructor/destructor
        CallStk ( const SrcLoc & sloc, const char * fname )
            : caller ( callstk )
            , depth ( callstk -> depth + 1 )
            , src ( sloc )
            , func ( fname )
        {
            callstk = this;
        }

        ~ CallStk ()
        {
            callstk = caller;
        }


    protected:

        // special per-thread constructor
        CallStk ( const SrcLoc & sloc );

    };


    /*------------------------------------------------------------------
     * FUNC_ENTRY
     *  macro to create stack frame linked with caller
     *  uses GCC __PRETTY_FUNCTION__ to produce function signature
     */
#define FUNC_ENTRY()                                                                  \
    vdb3 :: CallStk local_stack_frame ( vdb3 :: s_src_loc, __PRETTY_FUNCTION__ )
}

#endif // _hpp_vdb3_kfc_callstk_
