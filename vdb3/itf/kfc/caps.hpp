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

#ifndef _hpp_vdb3_kfc_caps_
#define _hpp_vdb3_kfc_caps_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

namespace vdb3
{
    // access object data
    const caps_t CAP_READ           = ( 1 <<  0 );
    const caps_t CAP_WRITE          = ( 1 <<  1 );
    const caps_t CAP_RDWR           = CAP_READ | CAP_WRITE;

    // access a property
    const caps_t CAP_PROP_READ      = ( 1 <<  2 );
    const caps_t CAP_PROP_WRITE     = ( 1 <<  3 );
    const caps_t CAP_PROP_RDWR      = CAP_PROP_READ | CAP_PROP_WRITE;

    // resize memory
    const caps_t CAP_RESIZE         = ( 1 <<  4 );

    // subrange memory
    const caps_t CAP_SUBRANGE       = ( 1 <<  5 );

    // specific rights
    const caps_t CAP_CAST           = ( 1 <<  6 );
    const caps_t CAP_ALLOC          = ( 1 <<  7 );
    const caps_t CAP_EXECUTE        = ( 1 <<  8 );
    const caps_t CAP_SUSPEND        = ( 1 <<  9 );
}

#endif // _hpp_vdb3_kfc_caps_
