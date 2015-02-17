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

#include <kfc/syserr.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/except.hpp>

#if UNIX
#include <string.h>
#else
#error "unsupported target platform"
#endif

namespace vdb3
{

    static
    String tmp_syserr ( int err_code, char * buffer, size_t bsize )
    {
#if UNIX
        // get the error string from libc
        strerror_r ( err_code, buffer, bsize );

        // turn it into a temporary ConstString
        return ConstString ( buffer, strlen ( buffer ) );
#endif
    }

    // returns a system error string
    String SysErr( int err_code )
    {
        // get SysErras a tmp ConstString
        char buffer [ 4096 ];
        String str = tmp_syserr ( err_code, buffer, sizeof buffer );

        // allocate memory for a copy
        Mem m = rsrc -> mmgr . alloc ( str . size (), false );

        // create a string buffer
        StringBuffer sb ( m );

        // copy in the string
        sb . append ( str );

        // now return the safe string
        return sb . to_str ();
    }

    // throws a runtime error
    void ThrowOSErr( U32 lineno, const String & func_name, int err_code )
    {
        String msg;

        try
        {
            // get SysErras a tmp ConstString
            char buffer [ 4096 ];
            String str = tmp_syserr ( err_code, buffer, sizeof buffer );

            // create a string buffer
            StringBuffer sb ( "%s() returned %d: '%s'", & func_name, err_code, & str );

            msg = sb . to_str ();
        }
        catch ( ... )
        {
            msg = CONST_STRING ( "<error creating SysErrstring>" );
        }

        // throw exception
        throw runtime_err ( lineno, msg );
    }

}
