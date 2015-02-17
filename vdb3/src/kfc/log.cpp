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

#include <kfc/log.hpp>
#include <kfc/refcount.hpp>
#include <kfc/callstk.hpp>
#include <kfc/caps.hpp>

namespace vdb3
{

    // not valid for setting level, used internally
    const LogLevel log_disabled = ( LogLevel ) 0;
    const LogLevel dflt_log_level = log_info;

    /*------------------------------------------------------------------
     * LoggerItf
     */

    Log LoggerItf :: make_ref ( Refcount * obj, caps_t caps )
    {
        return Log ( obj, this, caps );
    }

    void * LoggerItf :: cast ( Refcount * obj )
    {
        return ( void * )  dynamic_cast < LoggerItf * > ( obj );
    }


    /*------------------------------------------------------------------
     * Log
     *  logging formatter
     *  handles output to the log stream
     */

    void Log :: set_level ( LogLevel level )
    {
        FUNC_ENTRY ();
        test_caps ( CAP_PROP_WRITE );

        if ( level >= log_emerg && level <= log_info )
            lvl = level;
    }

    // log message
    void Log :: msg ( LogLevel priority, const char * fmt, ... ) const
    {
        if ( priority <= lvl )
        {
            FUNC_ENTRY ();

            va_list args;
            va_start ( args, fmt );

            try
            {
                LoggerItf * logger = get_itf ( CAP_WRITE );
                logger -> msg ( priority, fmt, args );
            }
            catch ( ... )
            {
                va_end ( args );
                throw;
            }

            va_end ( args );
        }
    }

    void Log :: vmsg ( LogLevel priority, const char * fmt, va_list args ) const
    {
        if ( priority <= lvl )
        {
            FUNC_ENTRY ();

            LoggerItf * logger = get_itf ( CAP_WRITE );
            logger -> msg ( priority, fmt, args );
        }
    }

    // C++
    Log :: Log ()
        : lvl ( log_disabled )
    {
    }

    Log :: Log ( const Log & r )
        : Ref < LoggerItf > ( r )
        , lvl ( r . lvl )
    {
    }

    void Log :: operator = ( const Log & r )
    {
        FUNC_ENTRY ();
        Ref < LoggerItf > :: operator = ( r );
        lvl = r . lvl;
    }

    Log :: Log ( const Log & r, caps_t reduce )
        : Ref < LoggerItf > ( r, reduce )
        , lvl ( r . lvl )
    {
    }

    // factory
    Log :: Log ( Refcount * obj, LoggerItf * itf, caps_t caps )
        : Ref < LoggerItf > ( obj, itf, caps )
        , lvl ( dflt_log_level )
    {
    }

    Log :: ~ Log ()
    {
        lvl = log_disabled;
    }

}
