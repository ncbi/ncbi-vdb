/*

  vdb3.cmn.logger


 */

#include <vdb3/cmn/plogger.hpp>

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include <errno.h>

namespace vdb3
{

    // primordial logger timestamp
    std :: ostream & operator << ( std :: ostream & o, const Timestamp & ts )
    {
        static const char * months [ 12 ] =
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        time_t secs = ( time_t ) ( Z64 ) ts . trunc ();

        struct tm tm;
        localtime_r ( & secs, & tm );

        assert ( tm . tm_mon >= 0 && tm . tm_mon < 12 );

        return o
            << months [ tm . tm_mon ]
            << ' '
            << std :: setw ( 2 )
            << tm . tm_mday
            << ' '
            << std :: setw ( 2 )
            << std :: setfill ( '0' )
            << tm . tm_hour
            << ':'
            << std :: setw ( 2 )
            << std :: setfill ( '0' )
            << tm . tm_min
            << ':'
            << std :: setw ( 2 )
            << std :: setfill ( '0' )
            << tm . tm_sec
            ;
    }

    // primordial logger priority
    std :: ostream & operator << ( std :: ostream & o, LogLevel lvl )
    {
        static const char * names [ 8 ] =
        {
            "EMERG", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"
        };

        if ( ( Z32 ) lvl < 0 || lvl > LOG_DEBUG )
            return o << "<INVALID>";

        return o << names [ lvl ];
    }

    /*=====================================================*
     *                     PLoggerImpl                     *
     *=====================================================*/

    std :: atomic_flag PLoggerImpl :: latch = ATOMIC_FLAG_INIT;

    String PLoggerImpl :: getHostname () const noexcept
    {
        return hostname;
    }

    String PLoggerImpl :: getProcname () const noexcept
    {
        return procname;
    }

    LogQueueId PLoggerImpl :: getQueueId ( const String & name ) const
    {
        auto it = qmap . find ( name );
        if ( it == qmap . end () )
        {
            throw NotFoundException (
                XP ( XLOC )
                << xprob
                << "failed to get queue id"
                << xcause
                << "queue '"
                << name
                << "' does not exist"
                );
        }

        return it -> second;
    }

    LogQueueId PLoggerImpl :: makeQueue ( const String & name )
    {
        auto it = qmap . find ( name );
        if ( it != qmap . end () )
            return it -> second;

        LogQueueId qid = makeQueueId ( name, ++ qids );
        qmap . emplace ( name, qid );

        return qid;
    }

    void PLoggerImpl :: write ( LogLevel lvl, pid_t pid,
        const Timestamp & ts, const CText & msg ) const
    {
        std :: cerr
            << ts
            << ' '
            << hostname
            << ' '
            << tool_name
            << '['
            << pid
            << "]: "
            << lvl
            << ": "
            << msg
            << '\n'
            ;
    }

    void PLoggerImpl :: write ( const LogQueueId & qid, LogLevel lvl,
        pid_t pid, const Timestamp & ts, const CText & msg ) const
    {
        std :: cerr
            << ts
            << ' '
            << hostname
            << ' '
            << tool_name
            << '['
            << pid
            << ':'
            << qid -> getName ()
            << "]: "
            << lvl
            << ": "
            << msg
            << '\n'
            ;
    }

    PLoggerImpl :: PLoggerImpl ( const String & _host, const String & _proc )
        : hostname ( _host )
        , procname ( _proc )
        , qids ( 0 )
    {
        KTRACE ( TRACE_PRG, "acquiring primordial Logger latch" );
        if ( latch . test_and_set () )
        {
            throw PermissionViolation (
                XP ( XLOC, rc_logic_err )
                << "primordial logger is already initialized"
                );
        }

        KTRACE ( TRACE_PRG, "constructing primordial Logger @ 0x%zx", ( size_t ) this );
    }

    PLoggerImpl :: ~ PLoggerImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "destroying primordial Logger @ 0x%zx", ( size_t ) this );

        KTRACE ( TRACE_PRG, "clearing primordial Logger latch" );
        latch . clear ();
    }
}
