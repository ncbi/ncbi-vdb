/*

  vdb3.cmn.log-tst

 */

#include "log.cpp"
#include "logger.cpp"

#include "buffer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "nconv.cpp"
#include "writer.cpp"
#include "txt-writer.cpp"
#include "fmt.cpp"
#include "str-buffer.cpp"
#include "buffmt.cpp"
#include "trace.cpp"
#include "tracer.cpp"
#include "ptracer.cpp"
#include "trace-impl.cpp"
#include "rsrc-trace.cpp"

#include <vdb3/test/gtest.hpp>

#include <map>

#include <time.h>

namespace vdb3
{
    static bool do_print, do_logger, do_logger_write;
    static bool have_message;

    static const LogLevel LOG_DFLT = LOG_ERR;

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
            << tm . tm_hour
            << ':'
            << std :: setw ( 2 )
            << tm . tm_min
            << ':'
            << std :: setw ( 2 )
            << tm . tm_sec
            ;
    }
    
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
    
    struct FakeLogger
        : Refcount
        , LoggerItf
        , LogQueueFactory
    {
        METH ( String getHostname () const noexcept )
        {
            return hostname;
        }
        
        METH ( String getProcname () const noexcept )
        {
            return procname;
        }
        
        METH ( LogQueueId getQueueId ( const String & name ) const )
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

        METH ( LogQueueId makeQueue ( const String & name ) )
        {
            auto it = qmap . find ( name );
            if ( it != qmap . end () )
                return it -> second;

            static std :: atomic < N32 > qids;
            LogQueueId qid = makeQueueId ( name, ++ qids );
            qmap . emplace ( name, qid );

            return qid;
        }

        METH ( void write ( LogLevel lvl, pid_t pid,
            const Timestamp & ts, const CText & msg ) const )
        {
            if ( ! do_logger_write )
                return;
            
            try
            {
                std :: cout
                    << ts
                    << ' '
                    << hostname
                    << ' '
                    << procname
                    << '['
                    << pid
                    << "]: "
                    << lvl
                    << ": "
                    ;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write hostname, procname, pid, priority to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write hostname, procname, pid, priority to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }

            try
            {
                std :: cout << msg;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }
            
            std :: cout << '\n';
        }

        METH ( void write ( const LogQueueId & qid, LogLevel lvl,
            pid_t pid, const Timestamp & ts, const CText & msg ) const )
        {
            if ( ! do_logger_write )
                return;
            
            try
            {
                std :: cout
                    << ts
                    << ' '
                    << hostname
                    << ' '
                    << procname
                    << '['
                    << pid
                    << ':'
                    << qid -> getName ()
                    << "]: "
                    << lvl
                    << ": "
                    ;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write hostname, procname, pid, priority to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write hostname, procname, pid, priority to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }

            try
            {
                std :: cout << msg;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }
            
            std :: cout << '\n';
        }

        METH ( void flush () const )
        {
        }

        FakeLogger () noexcept
            : hostname ( CTextLiteral ( "fake-host" ) )
            , procname ( CTextLiteral ( "fake-proc" ) )
            , pid ( 12345 )
        {
        }

        ~ FakeLogger () noexcept
        {
        }

        // the hostname is captured here
        String hostname;

        // the procname is externally supplied
        String procname;

        // queue names and ids
        std :: map < String, LogQueueId > qmap;

        pid_t pid;
    };
        
    struct FakeLogWriter
        : Refcount
        , TextStreamWriterItf
    {
        // called from BufferedFmt
        // when its message is collapsed
        METH ( CText write ( const CText & msg ) )
        {
            have_message = true;
            if ( do_logger )
            {
                if ( ! qid )
                    logger . write ( lvl, pid, ts, msg );
                else
                    logger . write ( qid, lvl, pid, ts, msg );
            }
            return msg;
        }
        
        // called from BufferedFmt
        // either after every write or ...
        METH ( void flush () )
        {
            logger . flush ();
        }

        FakeLogWriter ( const RsrcTime & rsrc, const Logger & l, LogLevel _lvl, pid_t _pid ) noexcept
            : logger ( l )
            , ts ( rsrc . time . now () )
            , lvl ( _lvl )
            , pid ( _pid )
        {
        }

        FakeLogWriter ( const RsrcTime & rsrc, const Logger & l, const LogQueueId & _qid, LogLevel _lvl, pid_t _pid ) noexcept
            : logger ( l )
            , qid ( _qid )
            , ts ( rsrc . time . now () )
            , lvl ( _lvl )
            , pid ( _pid )
        {
        }

        ~ FakeLogWriter () noexcept
        {
        }

        // this is where we need to transfer data
        Logger logger;

        // an optional LogQueue id
        LogQueueId qid;

        // event timestamp
        Timestamp ts;

        // the priority
        LogLevel lvl;

        // process id
        pid_t pid;
    };
    
    struct FakeLogMgr
        : Refcount
        , LogMgrItf
    {
        METH ( TextStreamWriterRef msg ( const RsrcKfc & rsrc,
            const Logger & logger, LogLevel priority ) )
        {
            return new FakeLogWriter ( rsrc, logger, priority, pid );
        }
        
        METH ( TextStreamWriterRef msg ( const RsrcKfc & rsrc,
            const Logger & logger, const LogQueueId & qid, LogLevel priority ) )
        {
            return new FakeLogWriter ( rsrc, logger, qid, priority, pid );
        }
        
        METH ( LogLevel getThreshold () const noexcept )
        {
            return lvl;
        }
        
        METH ( LogLevel getThreshold ( const LogQueueId & qid ) const noexcept )
        {
            try
            {
                return qlvl [ qid -> getId () - 1 ];
            }
            catch ( ... )
            {
            }
            return LOG_INVALID;
        }
        
        METH ( void setThreshold ( LogLevel _lvl ) )
        {
            if ( _lvl >= LOG_EMERG && _lvl <= LOG_DEBUG )
                lvl = _lvl;
        }
        
        METH ( void setThreshold ( const LogQueueId & qid, LogLevel _lvl ) )
        {
            if ( _lvl >= LOG_EMERG && _lvl <= LOG_DEBUG )
            {
                // make a log-level entry
                N32 id = qid -> getId ();

                // extend our table as required
                if ( ( size_t ) id > qlvl . size () )
                {
                    qlvl . reserve ( id );
                    for ( N32 i = ( N32 ) qlvl . size (); i < id; ++ i )
                        qlvl . push_back ( LOG_INVALID );
                }

                // update existing entry
                assert ( id > 0 );
                qlvl [ id - 1 ] = _lvl;
            }
        }

        METH ( LogMgrRef clone () const )
        {
            return new FakeLogMgr ( self );
        }

        FakeLogMgr ()
            : lvl ( LOG_DFLT )
            , pid ( getpid () )
        {
        }

        FakeLogMgr ( const FakeLogMgr & mgr )
            : lvl ( mgr . lvl )
            , pid ( mgr . pid )
        {
            for ( LogLevel l : mgr . qlvl )
                qlvl . push_back ( l );
        }
        
        virtual ~ FakeLogMgr () noexcept
        {
        }

        std :: vector < LogLevel > qlvl;
        LogLevel lvl;
        pid_t pid;
    };

    class LogTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
            do_print = do_logger = do_logger_write = true;
            have_message = false;
        }

        virtual void TearDown () override
        {
        }

        LogTestFixture ()
            : logger ( new FakeLogger )
            , log ( new FakeLogMgr )
        {
        }

    protected:

        RsrcTrace rsrc;
        Logger logger;
        LogMgr log;

    };
    
    TEST_F ( LogTestFixture, constructor_destructor )
    {
    }
    
    TEST_F ( LogTestFixture, set_log_level_debug )
    {
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        EXPECT_EQ ( log . getThreshold (), LOG_DEBUG );
    }
    
    TEST_F ( LogTestFixture, say_howdy_no_print )
    {
        do_print = false;
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, LOG_INFO,
                [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( LogTestFixture, say_howdy_no_logger )
    {
        do_logger = false;
        EXPECT_EQ ( have_message, false );
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, LOG_INFO,
                [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
        EXPECT_EQ ( have_message, true );
    }
    
    TEST_F ( LogTestFixture, say_howdy_no_logger_write )
    {
        do_logger_write = false;
        EXPECT_EQ ( have_message, false );
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, LOG_INFO,
                [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
        EXPECT_EQ ( have_message, true );
    }
    
    TEST_F ( LogTestFixture, say_howdy )
    {
        EXPECT_EQ ( have_message, false );
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, LOG_INFO, [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
        EXPECT_EQ ( have_message, true );
    }
    
    TEST_F ( LogTestFixture, say_silent_howdy )
    {
        EXPECT_EQ ( have_message, false );
        EXPECT_NO_THROW ( log . setThreshold ( LOG_ERR ) );
        log . msg ( rsrc, logger, LOG_INFO, [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
        EXPECT_EQ ( have_message, false );
    }
    
    TEST_F ( LogTestFixture, create_queue )
    {
        EXPECT_NO_THROW ( LogQueueId qid = logger . makeQueue ( "blark" ) );
    }
    
    TEST_F ( LogTestFixture, say_howdy_to_queue )
    {
        LogQueueId qid = logger . makeQueue ( "blark" );
        EXPECT_EQ ( have_message, false );
        EXPECT_NO_THROW ( log . setThreshold ( LOG_DEBUG ) );
        log . msg ( rsrc, logger, qid, LOG_INFO, [] ( Fmt & fmt )
                {
                    fmt
                        << "how doo to the new queue"
                        << endm
                        ;
                }
            );
        EXPECT_EQ ( have_message, true );
    }
}

extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        CAPTURE_TOOL ( argv [ 0 ] );
        CAPTURE_EARLY_TRACE_LEVEL ( argc, argv );
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
