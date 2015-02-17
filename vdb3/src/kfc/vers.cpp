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

#include <kfc/vers.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * vers_t
     */
    vers_t :: vers_t ()
        : val ( 0 )
    {
    }

    vers_t :: vers_t ( U32 maj )
        : val ( maj << 24 )
    {
        FUNC_ENTRY ();
        if ( maj > 255 )
            THROW ( xc_bounds_err, "major version = %u", maj );
    }

    vers_t :: vers_t ( U32 maj, U32 min )
        : val ( ( maj << 24 ) | ( min << 16 ) )
    {
        FUNC_ENTRY ();
        if ( maj > 255 )
            THROW ( xc_bounds_err, "major version = %u", maj );
        if ( min > 255 )
            THROW ( xc_bounds_err, "minor version = %u", min );
    }

    vers_t :: vers_t ( U32 maj, U32 min, U32 rel )
        : val ( ( maj << 24 ) | ( min << 16 ) | ( rel << 8 ) )
    {
        FUNC_ENTRY ();
        if ( maj > 255 )
            THROW ( xc_bounds_err, "major version = %u", maj );
        if ( min > 255 )
            THROW ( xc_bounds_err, "minor version = %u", min );
        if ( rel > 255 )
            THROW ( xc_bounds_err, "release component = %u", rel );
    }

    vers_t :: vers_t ( U32 maj, U32 min, U32 rel, U32 post )
        : val ( ( maj << 24 ) | ( min << 16 ) | ( rel << 8 ) | post )
    {
        FUNC_ENTRY ();
        if ( maj > 255 )
            THROW ( xc_bounds_err, "major version = %u", maj );
        if ( min > 255 )
            THROW ( xc_bounds_err, "minor version = %u", min );
        if ( rel > 255 )
            THROW ( xc_bounds_err, "release component = %u", rel );
        if ( post > 255 )
            THROW ( xc_bounds_err, "post-release component = %u", post );
    }
}
