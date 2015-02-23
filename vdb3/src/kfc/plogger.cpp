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

#include "plogger.hpp"
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/string.hpp>
#include <kfc/caps.hpp>

#if UNIX
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#else
#error "unsupported target platform"
#endif

namespace vdb3
{
    Log plogger_t :: make ( const char * ident )
    {
        FUNC_ENTRY ();

        // going to record a "hostname procname[pid]"
        StringBuffer hostproc;

        // create a newline
        ConstString newline ( "\n", 1 );
#if UNIX
        // hostname
        char namebuf [ 512 ];
        int status = gethostname ( namebuf, sizeof namebuf );
        if ( status != 0 )
        {
            strncpy ( namebuf, "<HOST-UNKNOWN>", sizeof namebuf );
            status = 0;
        }
        ConstString hostname ( namebuf, strlen ( namebuf ) );

        // process id
        pid_t pid = getpid ();

        // process name
        if ( ident == 0 || ident [ 0 ] == 0 )
            hostproc . append ( "%s[%d]", & hostname, pid );
        else
        {
            const char * leaf = strrchr ( ident, '/' );
            if ( leaf ++ == 0 )
                leaf = ident;
            ConstString procname ( leaf, strlen ( leaf ) );

            hostproc . append ( "%s %s[%d]", & hostname, & procname, pid );
        }
#endif

        plogger_t * obj = new plogger_t ( hostproc . to_str (), newline );
        return obj ->  make_ref ( obj, CAP_WRITE );
    }

    void plogger_t :: msg ( LogLevel priority, const char * fmt, va_list args )
    {
        // guard against /dev/null
        if ( ! rsrc -> err )
            return;

        FUNC_ENTRY ();

        static const char * months [ 12 ] =
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

#if UNIX
        // timestamp
        // TBD - this should be entirely through timemgr
#warning "timestamp should be via TimeMgrItf"
        time_t secs = time ( 0 );
        struct tm tm = * localtime ( & secs );

        // create month string
        assert ( tm . tm_mon >= 0 && tm . tm_mon < 12 );
        ConstString month ( months [ tm . tm_mon ], 3 );

        // timestamp: 'Sep 25 10:49:18'
        // print timestamp, host&process[pid]
        StringBuffer buffer ( "%s %2d %02d:%02d:%02d %s: "
                              , & month
                              , tm . tm_mday
                              , tm . tm_hour
                              , tm . tm_min
                              , tm . tm_sec
                              , & hostproc
            );
#endif
    
        // print message
        if ( fmt != 0 && fmt [ 0 ] != 0 )
            buffer . vappend ( fmt, args );

        // trim off trailing white-space
        buffer . trim ();

        // end with newline
        buffer += newline;

        // extract memory from string
        String str = buffer . to_str ();

        // copy to log stream
        rsrc -> err . write_all ( str . to_mem () );
    }

    plogger_t :: plogger_t ( const String & ident, const String & nl )
        : hostproc ( ident )
        , newline ( nl )
    {
    }

    plogger_t :: ~ plogger_t ()
    {
    }
}
