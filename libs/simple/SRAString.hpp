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

#ifndef _hpp_simple_SRAString_
#define _hpp_simple_SRAString_

#ifndef _h_simple_SRAString_
#include "sra-string.h"
#endif

#ifndef _hpp_simple_sra_
#include <simple/sra.hpp>
#endif


namespace sra
{

    /*----------------------------------------------------------------------
     * String
     */

    /* Make
     */
    inline
    String StringMake ( :: SRAString & sra_str )
    {
        String cpp_str ( sra_str . addr, sra_str . size );
        :: SRAStringWhack ( & sra_str );
        return cpp_str;
    }

    /* Append
     */
    inline
    void StringAppend ( String & cpp_str, :: SRAString & sra_str )
    {
        cpp_str . append ( sra_str . addr, sra_str . size );
        :: SRAStringWhack ( & sra_str );
    }

}

#endif /* _hpp_simple_SRAString_ */
