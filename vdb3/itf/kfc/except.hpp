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

#ifndef _hpp_vdb3_kfc_except_
#define _hpp_vdb3_kfc_except_

/*------------------------------------------------------------------
 * XC_DECLARE
 *  provides a simple means of declaring a new exception type
 */
//#ifdef _hpp_vdb3_kfc_string_
#ifdef __FILE__
#define XC_DECLARE ( new_xc, parent_xc )                            \
    struct new_xc : parent_xc                                      \
    { new_xc ( vdb3 :: U32 lineno, const char * msg )              \
          : parent_xc ( lineno, msg ) {} }
#else
#define XC_DECLARE ( new_xc, parent_xc )                            \
    struct new_xc : parent_xc                                      \
    { new_xc ( vdb3 :: U32 lineno, const vdb3 :: String & msg )    \
          : parent_xc ( lineno, msg ) {} }
#endif

#include <kfc/defs.hpp>
#include <kfc/string.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * exception
     *  captures the type of exception and a description
     */
    class exception
    {
    public:

#ifndef _hpp_vdb3_kfc_string_
        const char * what () const
        { return msg; }
#else
        // report basically what went wrong into a string
        const String & what () const
        { return msg; }

        // report where the exception occurred
        // assembles module, file, line and function
        String where () const;

        // report entire stack trace
        String stack_trace () const;

        // return the constant module name
        String module () const;

        // return the constant file name
        String file () const;

        // return the constant file extension
        String ext () const;

        // return the constant function name
        String func () const;
#endif

        // return the line number where exception was thrown
        U32 line () const
        { return lineno; }

        // public destructor
        ~ exception ();

    protected:

#ifdef _hpp_vdb3_kfc_string_
        exception ( U32 lineno, const String & msg );
#else
        exception ( U32 lineno, const char * msg );
#endif

#ifdef _hpp_vdb3_kfc_string_
        String msg;
#else
        const char * msg;
#endif
#ifdef _hpp_vdb3_kfc_memory_
        Mem stk;
#endif
        U32 lineno;
    };


    XC_DECLARE ( logic_err, exception );
    XC_DECLARE ( runtime_err, exception );
    XC_DECLARE ( usage_err, exception );
    XC_DECLARE ( abuse_err, exception );
    XC_DECLARE ( xc_internal_err, logic_err );
    XC_DECLARE ( xc_unimplemented_err, xc_internal_err );
    XC_DECLARE ( xc_bounds_err, logic_err );
    XC_DECLARE ( xc_div_zero_err, logic_err );
    XC_DECLARE ( xc_caps_violation_err, logic_err );
    XC_DECLARE ( xc_caps_over_extended_err, xc_internal_err );
    XC_DECLARE ( xc_rcaps_violation_err, logic_err );
    XC_DECLARE ( xc_program_state_violation, xc_internal_err );
    XC_DECLARE ( xc_bad_fmt_err, logic_err );
    XC_DECLARE ( xc_param_err, logic_err );
    XC_DECLARE ( xc_null_param_err, xc_param_err );
    XC_DECLARE ( xc_null_self_err, logic_err );
    XC_DECLARE ( xc_unsupported_interface_err, logic_err );
    XC_DECLARE ( xc_references_exhausted_err, runtime_err );
    XC_DECLARE ( xc_zombie_object_err, logic_err );
    XC_DECLARE ( xc_elem_size_err, logic_err );
    XC_DECLARE ( xc_no_mem, runtime_err );
    XC_DECLARE ( xc_mem_quota, xc_no_mem );


    /*------------------------------------------------------------------
     * THROW
     *  throws an exception of the given type
     *  allocates a string to copy data
     *  provides line number for xc_loc_t
     */
#ifdef _hpp_vdb3_kfc_string_
#define THROW( xc, ... )                                     \
    throw xc ( __LINE__,                                     \
        vdb3 :: StringBuffer ( __VA_ARGS__ ) . to_str () )
#else
#define THROW( xc, fmt, ... )                                \
    throw xc ( __LINE__, fmt )
#endif

    /*------------------------------------------------------------------
     * CONST_THROW
     *  throws an exception of the given type
     *  embeds a string constant with no allocation
     *  provides line number for xc_loc_t
     */
#ifdef _hpp_vdb3_kfc_string_
#define CONST_THROW( xc, msg )                  \
    throw xc ( __LINE__, CONST_STRING ( msg ) )
#else
#define CONST_THROW( xc, msg )                  \
    throw xc ( __LINE__, msg )
#endif
}

#endif // _hpp_vdb3_kfc_except_
