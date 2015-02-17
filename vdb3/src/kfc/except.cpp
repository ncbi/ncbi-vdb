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

#define SRC_LOC_DEFINED
#include <kfc/except.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/array.hpp>
#include <kfc/ptr.hpp>

#include <string.h>
#include <ctype.h>

namespace vdb3
{
    struct func_loc_t : SrcLoc
    {
        const char * mod;
        const char * file;
        const char * ext;
        const char * func;

        void operator = ( const CallStk & stkfrm )
        {
            mod = stkfrm . src . mod;
            file = stkfrm . src . file;
            ext = stkfrm . src . ext;
            func = ( const char * ) stkfrm . func;
        }
    };

#ifdef _hpp_vdb3_kfc_string_
    static
    String extract_method_name ( const char * pretty_func )
    {
        U64 i;
        for ( i = 0; pretty_func [ i ] != 0; ++ i )
        {
            int ch = pretty_func [ i ];
            if ( isalpha ( ch ) || ch == '_' )
            {
                U64 start = i;
                for ( ++ i; pretty_func [ i ] != 0; ++ i )
                {
                    ch = pretty_func [ i ];
                    if ( ! isalnum ( ch ) && ch != '_' )
                    {
                        if ( ch != ':' || pretty_func [ i + 1 ] != ':' )
                            break;
                        if ( isdigit ( pretty_func [ i + 2 ] ) )
                            break;
                        ++ i;
                    }
                }

                if ( ch == '(' )
                    return ConstString ( & pretty_func [ start ], i - start );
                if ( ch == 0 )
                    -- i;
            }
        }
        return ConstString ( pretty_func, i );
    }

    String exception :: where () const
    {
        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        if ( rsrc == 0 )
            return CONST_STRING ( "<no-mem>" );

        const Array < func_loc_t > a = stk;
        const func_loc_t & loc = a [ 0 ];

        ConstString m ( loc . mod, strlen ( loc . mod ) );
        ConstString f ( loc . file, strlen ( loc . file ) );
        ConstString x ( loc . ext, strlen ( loc . ext ) );
        String fn ( extract_method_name ( loc . func ) );
        //const_str_t fn ( loc . func, strlen ( loc . func ) );

        StringBuffer sb ( "%s/%s.%s:%u %s", & m, & f, & x, lineno, & fn );
        return sb . to_str ();
    }

    String exception :: stack_trace () const
    {
        count_t depth = stk . size () / sizeof ( func_loc_t );

        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        if ( rsrc == 0 )
            return CONST_STRING ( "<no-mem>" );

        StringBuffer sb;
        Array < const func_loc_t > a = stk;

        for ( count_t i = 0; i < depth; ++ i )
        {
            const func_loc_t & loc = a [ i ];

            ConstString m ( loc . mod, strlen ( loc . mod ) );
            ConstString f ( loc . file, strlen ( loc . file ) );
            ConstString x ( loc . ext, strlen ( loc . ext ) );
            String fn ( extract_method_name ( loc . func ) );
            //const_str_t fn ( loc . func, strlen ( loc . func ) );

            if ( i == 0 )
                sb . append ( "  %s/%s.%s:%u %s\n", & m, & f, & x, lineno, & fn );
            else
                sb . append ( "  %s/%s.%s %s\n", & m, & f, & x, & fn );
        }

        // this is optional...
        sb . trim_eoln ();

        return sb . to_str ();
    }

    String exception :: module () const
    {
        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        const Array < func_loc_t > a = stk;
        const char * text = a [ 0 ] . mod;

        return ConstString ( text, strlen ( text ) );
    }

    String exception :: file () const
    {
        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        const Array < func_loc_t > a = stk;
        const char * text = a [ 0 ] . file;

        return ConstString ( text, strlen ( text ) );
    }

    String exception :: ext () const
    {
        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        const Array < func_loc_t > a = stk;
        const char * text = a [ 0 ] . ext;

        return ConstString ( text, strlen ( text ) );
    }

    String exception :: func () const
    {
        if ( stk . size () == ( U64 ) 0 )
            return CONST_STRING ( "<no-stack>" );

        const Array < func_loc_t > a = stk;
        const char * text = a [ 0 ] . func;

        return ConstString ( text, strlen ( text ) );
    }
#endif

    exception :: ~ exception ()
    {
    }

#ifndef _hpp_vdb3_kfc_string_
    exception :: exception ( U32 line, const char * m )
        : msg ( m )
        , lineno ( line )
    {
    }
#else
    exception :: exception ( U32 line, const String & m )
        : msg ( m )
        , lineno ( line )
    {
        if ( rsrc != 0 && callstk != 0 )
        {
            size_t frm_bytes = callstk -> depth * sizeof ( func_loc_t );
            if ( callstk -> depth != 0 ) try
            {
                Mem stk_tmp = rsrc -> mmgr . alloc ( frm_bytes, true );
                Ptr < func_loc_t > p = stk_tmp;
                for ( const CallStk * s = callstk; s -> depth != 0; ++ p, s = s -> caller )
                    * p = * s;
                stk = stk_tmp;
            }
            catch ( ... )
            {
            }
        }
    }
#endif

}
