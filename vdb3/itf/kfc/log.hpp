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

#ifndef _hpp_vdb3_kfc_log_
#define _hpp_vdb3_kfc_log_

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif


namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class Log;
    class Refcount;
    interface LoggerItf;


    /*------------------------------------------------------------------
     * LogLevel
     *  mimic unix syslog
     */
    enum LogLevel
    {
        log_emerg = 1,
        log_alert,
        log_crit,
        log_err,
        log_warning,
        log_notice,
        log_info
    };

    /*------------------------------------------------------------------
     * LoggerItf
     */
    interface LoggerItf
    {
        // perform the formatting of the message
        // transfer it to the task log stream
        virtual void msg ( LogLevel priority, const char * fmt, va_list args ) = 0;

    protected:

        Log make_ref ( Refcount * obj, caps_t caps );

    private:

        static void * cast ( Refcount * obj );

        friend class Log;
    };

    /*------------------------------------------------------------------
     * Log
     *  logging formatter
     *  handles output to the log stream
     */
    class Log : public Ref < LoggerItf >
    {
    public:

        // log level
        LogLevel get_level () const
        { return lvl; }
        void set_level ( LogLevel level );

        // log message
        void msg ( LogLevel priority, const char * fmt, ... ) const;
        void vmsg ( LogLevel priority, const char * fmt, va_list args ) const;

        // C++
        Log ();
        Log ( const Log & r );
        void operator = ( const Log & r );
        Log ( const Log & r, caps_t reduce );
        ~ Log ();

    private:

        // factory
        Log ( Refcount * obj, LoggerItf * itf, caps_t caps );

        LogLevel lvl;

        friend interface LoggerItf;
    };

}

#endif // _hpp_vdb3_kfc_log_
